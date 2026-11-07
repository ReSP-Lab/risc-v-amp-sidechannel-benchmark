# AMP Setup for the Bananapi F3 SBC

## Armbian

In our setup, we used an Armbian image generated from Armbian GitHub
repository.

We installed required tools.
```
sudo apt install -y u-boot-tools build-essential device-tree-compiler git
```

## Adapt the Firmware

### OpenSBI

We now clone the OpenSBI source code used to alter our firmware.

```
git clone https://github.com/cyyself/opensbi.git -b k1-opensbi
git checkout 717bcd874f37df9e37fa2cee7a4bebb219c7cb35
```

Then, we copy our modified [sbi_init.c](./sbi_init.c) file into
`lib/sbi/sbi_init.c`, and we recompile OpenSBI using the following commands:

```
make clean
# unselect every platform except Spacemit K1x support ("board evb")
make PLATFORM=generic menuconfig
make PLATFORM=generic
```

We are interested by the `build/platform/generic/firmware/fw_dynamic.itb`
binary file; use it to replace the file in dedicated firmware
partition (consult this [cyyself's Gist](https://gist.github.com/cyyself/a07096e6e99c949ed13f8fa16d884402) on the topic).

```
# perform a backup
sudo dd if=/dev/mmcblk0 of=mmcblk0.bin bs=512 skip=1280 count=768
# export backup in your preferred manner (e.g., scp)
# [...]
# update the firmware
sudo dd if=build/platform/generic/firmware/fw_dynamic.itb of=/dev/mmcblk0 bs=512 seek=1280
```


### Device Tree Source

On the SBC, make a backup of the device tree blob.

```
cp /boot/dtb-6.6.99-legacy-spacemit/spacemit/k1-bananapi-f3.dtb k1-bananapi-f3.dtb.bak
```

Compile the [provided device tree source](./k1-bananapi-f3.dts) containing
information about reserved memory regions and disabling AMP-dedicated CPUs.

```
dtc -I dts -O dtb -o k1-bananapi-f3.dtb k1-bananapi-f3.dts
```

Use the new device tree blob as a replacement for
`/boot/dtb-6.6.99-legacy-spacemit/spacemit/k1-bananapi-f3.dtb`, and restore
original permissions:

```
sudo cp k1-bananapi-f3.dtb /boot/dtb-6.6.99-legacy-spacemit/spacemit/
sudo chown root /boot/dtb-6.6.99-legacy-spacemit/spacemit/k1-bananapi-f3.dtb
sudo chgrp root /boot/dtb-6.6.99-legacy-spacemit/spacemit/k1-bananapi-f3.dtb
sudo chmod 644 /boot/dtb-6.6.99-legacy-spacemit/spacemit/k1-bananapi-f3.dtb
```

### Apply Changes

Reboot to apply changes and verify they were correctly applied (see "Setup
Testing" in [parent directory README](../README.md)).
