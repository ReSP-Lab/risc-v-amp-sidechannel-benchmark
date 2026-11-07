# AMP Setup for the BeagleV Ahead SBC

## Ubuntu

In our setup, we use the [Ubuntu 23.04 image from the SBC provider](https://files.beagle.cc/file/beagleboard-public-2021/images/xuantie-ubuntu-23.04-20230705.zip)
which can be found with the [Latest Software Images from BeagleBoard](https://www.beagleboard.org/distros).


## Adapt the Firmware

### OpenSBI

We now clone the OpenSBI source code used to alter our firmware.

```
git clone https://github.com/beagleboard/beaglev-ahead-opensbi.git && cd beaglev-ahead-opensbi
git checkout 031dc5f8a725f3dedd32f6fafd5ea7b4db57f303
```

We first visit the `Makefile` file and look for the `-Werror` flag we should
remove and for the `-march=$(PLATFORM_RISCV_ISA)` we update with
`-march=$(PLATFORM_RISCV_ISA)_zifencei`.

Then, we copy our modified [sbi_init.c](./sbi_init.c) file into
`lib/sbi/sbi_init.c`, and we recompile OpenSBI using the following commands:

```
make clean
export CROSS_COMPILE=riscv64-unknown-elf-
make PLATFORM=generic FW_PIC=n
```

We are interested by the `build/platform/generic/firmware/fw_dynamic.bin`
binary file; use it to replace the original `/boot/firmware/fw_dynamic.bin`
file and restore original permissions:

```
sudo chown root fw_dynamic.bin
sudo chgrp root fw_dynamic.bin
sudo chmod 755 fw_dynamic.bin
```

### Device Tree Source

On the SBC, make a backup of the device tree blob.

```
cp /boot/firmware/light-beagle.dtb light-beagle.dtb.bak
```

Compile the [provided device tree source](./light-beagle.dts) containing
information about reserved memory regions and disabling AMP-dedicated CPUs.

```
dtc -I dts -O dtb -o light-beagle.dtb light-beagle.dts
```

Use the new device tree blob as a replacement for
`/boot/firmware/light-beagle.dtb`, and restore original permissions:

```
sudo chown root light-beagle.dtb
sudo chgrp root light-beagle.dtb
sudo chmod 644 light-beagle.dtb
```

### Apply Changes

Reboot to apply changes and verify they were correctly applied (see "Setup
Testing" in [parent directory README](../README.md)).
