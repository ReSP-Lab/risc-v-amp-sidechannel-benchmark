# AMP Setup for the BeagleV Fire SBC

## Ubuntu

In our setup, we use the [Ubuntu 23.04 image from the SBC provider](https://www.beagleboard.org/distros/beaglev-fire-ubuntu-2023-11-21)
which can be found with the [Latest Software Images from BeagleBoard](https://www.beagleboard.org/distros).

**Note:** while the Kernel version from `2023-10-19` was removed from their
list, which was originally used for the Linux setup, only minor changes should
appear in the version from `2023-11-21`.


## Setting Up AMP Environment

Dump the content of the SBC Image Tree blob:

```
dumpimage -l /boot/firmware/beaglev_fire.itb
```

You should get an output similar to the one below.
```
FIT description: U-Boot fitImage for the BeagleV-Fire
Created:         ...
 Image 0 (kernel)
  Description:  Linux kernel
  ...
  Load Address: 0x80200000
  Entry Point:  0x80200000
  ...
 Image 1 (base_fdt)
  Description:  Flattened Device Tree blob
  ...
  Load Address: 0x8a000000
  ...
 Default Configuration: 'kernel_dtb'
 Configuration 0 (kernel_dtb)
  Description:  1 Linux kernel, FDT blob
  Kernel:       kernel
  FDT:          base_fdt
 Configuration 1 (base_dtb)
  Description:  Base FDT blob for BeagleV-Fire board
  Kernel:       unavailable
  FDT:          base_fdt
```

In the affirmative, extract the Kernel image we will re-use later.

```
dumpimage -T flat_dt -p 0 -o kernel.bin /boot/firmware/beaglev_fire.itb
```

Make a backup of the original Image Tree Blob.

```
cp /boot/firmware/beaglev_fire.itb beaglev_fire.itb.bak
```


### Device Tree Source

Compile the [provided device tree source](./base_fdt.dts) containing
information about reserved memory regions and disabling AMP CPUs.

```
dtc -I dts -O dtb -o base_fdt.dtb base_fdt.dts
```

### Build and Flash the Image

Make the image from the [Image Tree Source file](./beaglev_fire.its)
provided in this folder and flash it to the right partition. The
`beaglev_fire.its` file indicates to re-use the Kernel image we extracted
earlier and the previously generated `base_fdt.dtb` file.

```
mkimage -f beaglev_fire.its beaglev_fire.itb
sudo cp beaglev_fire.itb /boot/firmware/beaglev_fire.itb
```

Now, restart the SBC and verify the memory region was correctly reserved and
the CPUs were indeed disabled before moving to the next step.
The verification is important since the firmware we will flash needs those to
work as expected.


## Adapt the Firmware

The BeagleV Fire relies on the
[Hart Software Services (HSS)](https://github.com/polarfire-soc/hart-software-services/),
making the AMP setup easier to implement. We will however use the same
[AMP](https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/applications-and-demos/asymmetric-multiprocessing/amp.md)
setup than we did for the other SBCs, not relying on Remoteproc and other
available tools.

To modify the firmware, it will be first necessary to create a new
U-Boot binary. Clone the associated repository:

```
git clone https://github.com/beagleboard/BeagleV-Fire-ubuntu.git
cd BeagleV-Fire-ubuntu
git clone --depth 1 --branch 'linux4microchip+fpga-2025.10'     https://github.com/linux4microchip/u-boot-mchp.git u-boot
```

Generate the binary (modify the `CC` toolchain prefix if necessary) and copy
it in the AMP BeagleV Fire folder:
```
CC=riscv64-unknown-linux-gnu- ./03_build_u-boot.sh
cp deploy/u-boot.bin [FOLDER-PATH]
```

Then, it will be necessary to download the
[HSS payload generator](https://github.com/polarfire-soc/hart-software-services/releases)
tool on your development machine. It can then be used to generate the new HSS
payload with the provided `compile.sh` script, as follows:

```
HSSGEN=[PATH-TO-HSS-PAYLOAD-GENERATOR-BIN] sh compile.sh
```

The generated `amp-payload.bin` has to be transferred to the SBC.

Before flashing the payload into the HSS partition, this partition is the
U-Boot partition referenced [here](https://github.com/beagleboard/BeagleV-Fire-ubuntu/blob/main/deploy/genimage.cfg).
To ensure the flashing process is performed on the right device, verify which
partition to flash with `lsblk -o NAME,SIZE,PARTTYPE,PARTLABEL,MOUNTPOINTS`.
In our case, the `/dev/mmcblk0p1` block device (*verify before using on your
SBC*) had the right `PARTTYPE`/`PARTLABEL`:

```
$ lsblk -o NAME,SIZE,PARTTYPE,PARTLABEL,MOUNTPOINTS
NAME          SIZE PARTTYPE                             PARTLABEL MOUNTPOINTS
mmcblk0        ...
├─mmcblk0p1    ... 21686148-6449-6e6f-744e-656564454649 uboot
├─mmcblk0p2    ... .................................... kernel    /boot/firmware
└─mmcblk0p3    ... .................................... root      /
mmcblk0boot0   ...
mmcblk0boot1   ...
```

To flash the payload, use the following command:

```
sudo dd if=amp-payload.bin of=[/dev/mmcblk0p1] bs=1M conv=fsync status=progress
```



