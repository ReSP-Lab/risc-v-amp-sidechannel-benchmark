# AMP Setup for the HiFive Unmatched SBC

## Ubuntu

In our setup, we use the [pre-installed server image with Ubuntu 24.04.3](https://cdimage.ubuntu.com/releases/noble/release/ubuntu-24.04.3-preinstalled-server-riscv64+unmatched.img.xz) 
which can be found on the [SBC-dedicated Ubuntu hardware support](https://ubuntu.com/hardware/docs/boards/how-to/ubuntu_supported/sifive-hifive-unmatched/).

We disable unattended upgrades to avoid having unexpected changes on our
installation.
```
sudo systemctl stop unattended-upgrades.service 
sudo systemctl disable unattended-upgrades.service
```

Then, we install required tools.
```
sudo apt install make git gcc device-tree-compiler
```

## Setting Up the Boot Partition

Create a working directory for the following subsections (referred below as
`WD`) and move into it.

Check your block device information with `lsblk`, you should find the
`mmcblk0p14` block.
Get the image from that partition and dump its content with

```
sudo dd if=/dev/mmcblk0p14 of=old-mmcblk0p14.itb
dumpimage -l old-mmcblk0p14.itb
```

You should get an output similar to the one below.
```
FIT description: Configuration to load OpenSBI before U-Boot
Created:         ...
 Image 0 (uboot)
  Description:  U-Boot
  ...
 Image 1 (opensbi)
  Description:  OpenSBI fw_dynamic Firmware
  ...
 Image 2 (fdt-1)
  Description:  hifive-unmatched-a00
  ...
 Default Configuration: 'conf-1'
 Configuration 0 (conf-1)
  Description:  hifive-unmatched-a00
  Kernel:       unavailable
  Firmware:     opensbi
  FDT:          fdt-1
  Loadables:    uboot
```

In the affirmative, extract the U-Boot image we will re-use later.

```
dumpimage -T flat_dt -p 0 -o u-boot.bin old-mmcblk0p14.itb
```

### OpenSBI

We now clone the OpenSBI source code used to alter our firmware.

```
git clone https://github.com/riscv/opensbi.git && cd opensbi
git checkout 337c23dd66b821ac04a0f7bea313e7e7b30ecc49
```

We first visit the `Makefile` file and look for the `-Werror` flag we should
remove.

Then, we copy our modified [sbi_init.c](./sbi_init.c) file into
`lib/sbi/sbi_init.c`, and we recompile OpenSBI as instructed by
[Das U-Boot documentation](https://docs.u-boot.org/en/v2025.04/board/sifive/unmatched.html).

```
make clean
make PLATFORM=generic
```

We are interested by the `build/platform/generic/firmware/fw_dynamic.bin`
binary file, **which should be copied in our previous working directory `WD`**.

### Device Tree Source

Compile the [provided device tree source](./fdt-1.dts) containing information
about reserved memory regions.

```
dtc -I dts -O dtb -o fdt-1.dtb fdt-1.dts
```


### Build and Flash the Image

Make the image from the Image Tree Source file and flash it to the right
partition. The `mmcblk0p14.its` file indicates to re-use the U-Boot image we
extracted earlier.

```
mkimage -f mmcblk0p14.its -E mmcblk0p14.itb
sudo dd if=mmcblk0p14.itb of=/dev/mmcblk0p14 bs=1M oflag=direct status=progress
```

### Apply Changes

Reboot to apply changes and verify they were correctly applied (see "Setup
Testing" in [parent directory README](../README.md)).
