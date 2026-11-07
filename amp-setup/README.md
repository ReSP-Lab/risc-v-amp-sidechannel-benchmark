# AMP Setup

This folder describes the steps to setup each single board computer (SBC) we
used for our experiments.


## Description

The goal with our AMP setup is to be able to execute machine mode instructions
on our SBCs in the attempt to identify all different cache-based timing types
which might be exploited on each of them and to identify cache-based
vulnerabilities in three steps present on each of them.

To achieve our goal, we first want to have at least one hart running Linux and
at least two harts executing a dynamic payload loader to run our benchmarks.
The dynamic payload loader is executed on target harts by modifying the OpenSBI
source code.

Then, to load payloads, to synchronize and to communicate data between harts,
we need to reserve a memory region. We reserve this memory region by modifying
the device tree source of the SBC. In this memory region, we need enough space
for:
- a region dedicated to the payload loader binary;
- a region dedicated to debug and synchronizations flags;
- a region dedicated to each hart payload binary;
- a region dedicated to data cache block access/eviction or instruction cache
  (ROP-based) access/eviction.

Memory mapping will be as follows:

```
#######################################################
#                                                     #
#  96 MB - Reserved     0x[X]2000000 - 0x[X]7FFFFFF   #
#  ---------------                                    #
#                                                     #
#    1 MB - loader                                    #
#        0x82000000 - 0x820FFFFF                      #
#    2 MB - AMP buffer memory                         #
#        0x82100000 - 0x822FFFFF                      #
#            0x8210000[1-4] : state hart 1-4          #
#            0x821000[A-D]0 : launch loader hart 1-4  #
#    4 MB - payload executable                        #
#        0x82300000 - 0x825FFFFF                      #
#            0x82300000 - 0x823FFFFF : hart 1 payload #
#            0x82400000 - 0x824FFFFF : hart 2 payload #
#            0x82500000 - 0x825FFFFF : hart 3 payload #
#            0x82600000 - 0x826FFFFF : hart 4 payload #
#    9 MB - memory                                    #
#        0x82700000 - 0x82FFFFFF                      #
#    16 MB - probe region                             #
#        0x83000000 - 0x83FFFFFF                      #
#    64 MB - output data region                       #
#        0x84000000 - 0x87FFFFFF                      #
#            - histograms & other output data         #
#                                                     #
#######################################################
```

## Setup Testing

To ensure the SBC is correctly set up, first check the required memory regions
were correctly reserved with `sudo cat /proc/iomem`.

Then, verify Linux has not access to the hart we need for our benchmarks by
listing online CPU with `lscpu` and by verifying hart IDs of online CPUs with
`cat /proc/cpuinfo`.

## Debugging and Functions/Counters Testing

To test platform-specific functions or counters, you can modify and use the
[dedicated C file](./m-mode-and-timing_setup-test.c).
To compile it, use the following command (by eventually adding additional
architecture or more classical compilation flags, or by modifying the ones
provided below) to get the `payload.bin` to use:
```
[CROSS-CMPLR-PREFIX]gcc -march=rv64gc_zifencei -mcmodel=medany -nostdlib \
        -T linker_payload.ld m-mode-and-timing_setup-test.c -o payload.elf
[CROSS-CMPLR-PREFIX]objcopy -O binary payload.elf payload.bin
```

To upload the payload to memory, you can use the
[controller](./scripts/controller.py) script which requires you to add your
board-specific metadata to the [targets](./scripts/targets.py) file.
You would then be able to use it as follows (`hart` being between `1` and the
`harts` value you defined for your board):
```
sudo python3 controller.py --target [target] --hart [hart] \
        --payload payload.bin
```

At any time, you can consult a memory region from the reserved memory using
[show_mem](./scripts/show_mem.sh) script with
`./show_mem.sh [DESIRED_AMP_REGION_BASE] [OFFSET]`.


