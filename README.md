# Extending Cache-Timing Side-Channel Assessment on RISC-V with Asymmetric Multiprocessing

## Related works

### Austa's benchmark

This repository contains an adapted version of the ported benchmark suite
proposed by 
[Austa's published article from ESORICS 2025](https://doi.org/10.1007/978-3-032-07894-0_2):

> Austa, C., Mühlberg, J.T., Dricot, JM. (2026). Systematic Assessment of Cache
> Timing Vulnerabilities on RISC-V Processors. In: Nicomette, V., Benzekri, A.,
> Boulahia-Cuppens, N., Vaidya, J. (eds) Computer Security – ESORICS 2025.
> ESORICS 2025. Lecture Notes in Computer Science, vol 16055. Springer, Cham.

### Deng's original work

Our benchmark suite is a refactoring and a translation for the RISC-V ISA from
the following work:

> Shuwen Deng, Wenjie Xiong, and Jakub Szefer, "A Benchmark Suite for Evaluating
> Caches’ Vulnerability to Timing Attacks", in Proceedings of the International
> Conference on Architectural Support for Programming Languages and Operating
> Systems (ASPLOS), March 2020.


## Reproducibility: Images used with experiments

BeagleV Ahead (`target_symbol=BV_AHEAD`):
- BeagleBoard.org Ubuntu 23.04 Xfce Image 2023-07-05
- Linux 5.10.113-g52fbe8443ea1-dirty riscv64

BeagleV Fire (`target_symbol=BV_FIRE`):
- BeagleBoard.org Ubuntu 23.04 Console Image 2023-10-19
- Linux 6.1.33-linux4microchip+fpga-2023.06-20231019+ riscv64

HiFive Unmatched (`target_symbol=UNMATCHED`):
- Ubuntu Jammy Jellyfish 22.04 (development branch)
- Linux 5.15.0-1004-generic riscv64

Banana Pi BPI-F3 (`target_symbol=BPI_F3`) (image built from
[Armbian's Build repository](https://github.com/armbian/build/), with huge page
support):
- Armbian-unofficial 26.02.0-trunk trixie
- Linux 6.6.99-legacy-spacemit

Banana Pi BPI-RV2 (`target_symbol=BPI_RV2`):
- OpenWrt 25.12.0 r32713-f919e7899d
- Linux OpenWrt 6.12.71


## Usage

The benchmark suite is composed of two main components from the previous
original works which were modified for our purpose:
1. an evaluation of the different timing types existing on the targets;
2. an evaluation of the different vulnerabilities, out of the 88 strong
    vulnerabilities identified in the original work, present on the targets.

### Setup

Before using the benchmark suite, ensure to add the target support by:
1. adding a dedicated board file into the [boards folder](./include/boards/),
    based on the [template.h](./include/boards/template.h) file;
2. adequately modifying the [board.h](./include/board.h) file;
3. adapting the [Makefile](./Makefile).

Edit the `CROSS_TC` variable from the [Makefile](./Makefile) to
specify the (path and) prefix of your toolchain (`[GCC-PATH]/[TC-PREFIX]]`),
including your RISC-V GCC
compiler.

When using Python for result processing scripts, use the [requirements.txt](./requirements.txt) file with Python 3.12.
We recommend the use of [pyenv](https://github.com/pyenv/pyenv/).


#### Linux-based setup

On the target board, enable support for memory mapping using huge pages:
```
# verify the support
cat /proc/meminfo | grep -i page
# verify if feature is already enabled (!=0)
cat /proc/sys/vm/nr_hugepages
# otherwise, enable it
echo 128 > /proc/sys/vm/nr_hugepages
```

To enable hardware performance counters to user mode, if disabled:
```
# verify the value to check if disabled (== 0)
cat /proc/sys/kernel/perf_user_access
# if disabled, enable it
echo 2 > /proc/sys/kernel/perf_user_access
```

To adapt cache configuration to your target, first consult the related
documentation, then use `getconf -a | grep -i cache` if information is missing.


#### AMP-based setup

For each board you use, follow instructions in the
[AMP-dedicated README](./amp-setup/README.md) file and in the AMP
board-specific README file. As an overview, you should:
1. update the firmware (e.g., OpenSBI);
2. update the device tree blob;
3. copy all Python scripts and Bash scripts from the
   [dedicated AMP-setup script](./amp-setup/scripts/) directory to the board;
4. test the setup.

Both to test your setup and to use this benchmark suite, you should add AMP
metadata used for the experiments in the dedicated board file in
[include/boards/](./include/boards/) folder and add similar metadata to the
[targets.py](./amp-setup/scripts/targets.py) Python script.


### Linux-based experiments

#### Evaluation of timing types

To evaluate the different timing types, you have to:
1. run the related Makefile recipe (e.g., `make histogram-unmatched`);
2. upload the binaries to the board (if it is not build on the target);
3. (optional, depends on 2.) run `mkdir -p histogram_output`;
4. run `timing_histogram [dcache_output_name]` and `timing_icache_histogram
   [icache_output_name]`;
5. download the output folder;
6. plot the results with `python scripts/plot_histogram.py [input_file]
   [output_file]`.


#### Evaluation of vulnerabilities

To evaluate the existing vulnerabilities, you have to:
1. run the related Makefile recipe (e.g., `make benchmark-unmatched`);
2. upload the `benchmark_output` and `scripts` folders to the board (if
   binaries were not build on the target, **strongly recommended**);
3. run `./scripts/run_benchmark.sh [input_folder] [output_folder] [output_file_name] > tmp2.txt &`;
4. (optional) run `disown -h` (useful to avoid waiting for the end of
   execution, which can be monitored with `ps aux | grep -i run_benchmark` or
   `ps -p [pid]` if you wrote down the PID given from 4.);
5. download the output folder;
6. adapt the CPU list and result paths in `scripts/plot_benchmark.py` and plot
   the results with `python scripts/plot_benchmark.py`.


### AMP-based experiments

#### Evaluation of timing types

To evaluate the different timing types, you have to:
1. run the related Makefile recipe with the AMP setup and indicating additional compilation flags if necessary, e.g., for the Banana Pi BPI-F3:
```
CROSS_TC=riscv64-unknown-linux-gnu- SETUP=AMP EXT_FLAGS=_zicbom make histogram-f3
```
2. upload both generated "timing" payload binaries to the board;
3. run `mkdir -p histogram_output`;
4. run the histogram-related binaries as follows (for `target` argument, look
   into the previously mentioned `targets.py` file):
```
# run payload for L1d cache and collect histogram
sudo python3 histogram-runner.py --target [target] --payload [dcache-payload]

# run payload for L1i cache and collect histogram
sudo python3 histogram-runner.py --target [target] --payload [icache-payload] \
        --i-cache-analysis
```
5. download the output folder;
6. plot the results with `python scripts/plot_histogram.py [input_file]
   [output_file]`.
7. you can eventually perform comparison with SMP results using optional
   arguments, illustrated in the following example:
```
python3 scripts/plot_histogram.py results/histogram-outputs/amp_histogram_dcache_bpi_f3.csv figures/histograms/x60-histogram_dcache_cmp --overlay results/histogram-outputs/no-prefetching/amp_histogram_dcache_bpi_f3.csv --overlay2 results/histogram-outputs/smp_histogram_dcache_bpi_f3.csv --primary "AMP - No Prefetching" --secondary "AMP - Prefetching" --tertiary "SMP"
```



#### Evaluation of vulnerabilities

To evaluate the existing vulnerabilities, you have to:
1. run the related Makefile recipe with the AMP setup and indicating additional compilation flags if necessary, e.g.:
```
SETUP=AMP EXT_FLAGS=_zicbom make benchmark-f3
```
2. upload the folder containing new generated binaries;
3. run the benchmark on the board
```
mkdir -p [output-folder]
sudo python3 benchmark-runner.py --target [target] --directory [binary-folder]\
        --output-directory [output-folder] > log.txt &
disown -h  # allow you to exit the session
```
4. download the output folder;
5. adapt the CPU list and result paths in `scripts/plot_benchmark.py` and plot
   the results with `python scripts/plot_benchmark.py`.

To repeat the benchmark execution, use the
[loop-benchmark.sh](./amp-setup/scripts/loop-benchmark.sh) Bash script.
```
sudo -v; sudo ./loop-benchmark.sh -t [target-name] -i [path-to-amp-binaries] -n [test-count] -s [sample-count] -li [lowest_suffix_index] -ls [higher_suffix_index] &
disown -h  # allow to leave session and keep benchmarks running
```

As an example, for the Unmatched board, for 1 test and 5000 samples in the
benchmark:
```
# case: L1d cache analysis
sudo -v
sudo ./loop-benchmark.sh -t unmatched -i UNMATCHED_amp_[exp-suffix] \
        -n 1 -s 5000 -li 0 -ls 14 &
# OR
# case: L1i cache analysis
sudo -v
sudo ./loop-benchmark.sh -t unmatched -i UNMATCHED_amp_l1icache_[exp-suffix] \
        -n 1 -s 5000 -li 0 -ls 14 -ia &
# then, to be able to exit the session
disown -h
```

The `loop-benchmark.sh` script allows you to reproduce benchmark results to
evaluate stability of your results. The complete arrays used to determine
vulnerabilities present on the device under test are present in `gzip`
compressed CSV files.

### Note about Banana Pi BPI-F3


On the Banana Pi BPI-F3, two clusters are available. You can assign the
remote hart to this cluster by using the `REMOTE_CLUSTER` macro:
```
[SETUP=AMP] EXT_FLAGS="_zicbom -DREMOTE_CLUSTER" make [histogram/benchmark]-f3
```

Then, experiments are run as usual on Linux setup or using the following
command on AMP setup:
```
sudo python3 histogram-runner.py --target bpi_f3 --payload timing[-icache]-payload.bin [--i-cache-analysis] --remote-slot 3
```

