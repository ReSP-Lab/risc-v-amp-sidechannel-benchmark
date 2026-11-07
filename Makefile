GCC=gcc
# cross toolchain
ifndef CROSS_TC
CROSS_TC=riscv64-unknown-linux-gnu-
endif

IMPLEM_CFLAGS=-O0 -DON_MULTIPLE_LINES=1
COMPLEM_CFLAGS=-march=rv64gc_zifencei$(EXT_FLAGS)
CFLAGS=
ifeq "$(SETUP)" "AMP"
	COMPLEM_CFLAGS+=-DAMP_SETUP -mcmodel=medany
	CFLAGS+=$(COMPLEM_CFLAGS) $(IMPLEM_CFLAGS)
else
	CFLAGS+=$(COMPLEM_CFLAGS) $(IMPLEM_CFLAGS)
	CFLAGS+=-DMMAP_ARRAY
endif

#CFLAGS=-O0
INCLUDE=-Iinclude/
EVICT_SRC=src/evict.c src/eviction_evaluation.c

HIST_SRC=src/evict.c src/timing_histogram_utils.c src/rng.c
ifeq "$(SETUP)" "AMP"
	HIST_SRC+=src/amp_setup/stdlib_substitute.c
	HIST_SRC+=src/amp_setup/timing_histogram_implem.c
else
	HIST_SRC+=src/linux_setup/timing_histogram_implem.c
endif
HIST_SRC+=src/timing_histogram.c

BENCH_SRC=src/generate_compilation_commands.c

ifndef TESTS
TESTS=1
endif
ifndef SAMPLES
SAMPLES=5000
endif

L1D_SFX=_smp
L1I_SFX=_smp_l1icache
ifeq "$(SETUP)" "AMP"
	L1D_SFX=_amp
	L1I_SFX=_amp_l1icache
endif

BENCH_AMP_GEN_FLAG=
BENCH_AMP_FLAG=
ifeq "$(SETUP)" "AMP"
	BENCH_AMP_GEN_FLAG+=-DAMP_SETUP
	BENCH_AMP_FLAG+=-a
endif



evict:
	mkdir -p eviction_output
	$(CROSS_TC)gcc $(INCLUDE) $(CFLAGS) $(EVICT_SRC) -o eviction_histogram

evict-unmatched: TARGET=-DBOARD=UNMATCHED
evict-unmatched:
	mkdir -p eviction_output
	$(CROSS_TC)gcc $(INCLUDE) $(CFLAGS) $(EVICT_SRC) $(TARGET) -o eviction_histogram


# template
#histogram-template: TARGET=-DBOARD=
#histogram-template: histogram
# 	@echo "Histogram binary compiled for [...]"

histogram-unmatched: TARGET=-DBOARD=UNMATCHED
histogram-unmatched: histogram
	@echo "Histogram binary compiled for HiFive Unmatched."

histogram-ahead: TARGET=-DBOARD=BV_AHEAD
histogram-ahead: histogram
	@echo "Histogram binary compiled for BeagleV-Ahead."

histogram-fire: TARGET=-DBOARD=BV_FIRE
histogram-fire: histogram
	@echo "Histogram binary compiled for BeagleV-Fire."

histogram-f3: TARGET=-DBOARD=BPI_F3
histogram-f3: histogram
	@echo "Histogram binary compiled for Banana Pi BPI-F3."

histogram-rv2: TARGET=-DBOARD=BPI_RV2
histogram-rv2: histogram
	@echo "Histogram binary compiled for Banana Pi BPI-RV2."

histogram:
	mkdir -p histogram_output
ifeq "$(SETUP)" "AMP"
	$(CROSS_TC)gcc $(CFLAGS) $(TARGET) -nostdlib \
		-T amp-setup/linker_payload.ld $(INCLUDE) $(HIST_SRC) -o payload.elf
	$(CROSS_TC)objcopy -O binary payload.elf timing-payload.bin
	$(CROSS_TC)objdump -S payload.elf > timing-payload.s
	$(CROSS_TC)gcc $(CFLAGS) $(TARGET) -DI_CACHE_ANALYSIS -nostdlib \
		-T amp-setup/linker_payload.ld $(INCLUDE) $(HIST_SRC) -o payload.elf
	$(CROSS_TC)objcopy -O binary payload.elf timing-icache-payload.bin
	$(CROSS_TC)objdump -S payload.elf > timing-icache-payload.s
	rm payload.elf
else
	$(CROSS_TC)gcc $(INCLUDE) $(CFLAGS) $(HIST_SRC) $(TARGET) -o timing_histogram
	$(CROSS_TC)gcc $(INCLUDE) $(CFLAGS) -DI_CACHE_ANALYSIS \
		$(HIST_SRC) $(TARGET) -o timing_icache_histogram
endif


# template
#benchmark-template: TARGET=
#	@echo "Benchmark compilation done for [...]."

benchmark-unmatched: TARGET=UNMATCHED
benchmark-unmatched: benchmark-l1d benchmark-l1i
	@echo "Benchmark compilation done for HiFive Unmatched."

benchmark-ahead: TARGET=BV_AHEAD
benchmark-ahead: benchmark-l1d benchmark-l1i
	@echo "Benchmark compilation done for BeagleV-Ahead."
	
benchmark-fire: TARGET=BV_FIRE
benchmark-fire: benchmark-l1d benchmark-l1i
	@echo "Benchmark compilation done for BeagleV-Fire."

benchmark-f3: TARGET=BPI_F3
benchmark-f3: benchmark-l1d benchmark-l1i
	@echo "Benchmark compilation done for Banana Pi BPI-F3."

benchmark-rv2: TARGET=BPI_RV2
benchmark-rv2: benchmark-l1d benchmark-l1i
	@echo "Benchmark compilation done for Banana Pi BPI-RV2."


# This recipe will generate compilation commands, apply them using target
# symbol and generate binaries for the target using cross-compilation
benchmark-l1d:
	mkdir -p benchmark_output
	$(GCC) $(INCLUDE) $(IMPLEM_CFLAGS) $(BENCH_SRC) \
			$(BENCH_AMP_GEN_FLAG) -DBOARD=$(TARGET) \
			-o generate_compilation_commands_$(TARGET)
	./generate_compilation_commands_$(TARGET) -m $(TARGET) \
			-n $(TESTS) -s $(SAMPLES) \
			-g $(CROSS_TC) -f "$(COMPLEM_CFLAGS)" $(BENCH_AMP_FLAG)
	rm generate_compilation_commands_$(TARGET)
	sh benchmark_scripts/$(TARGET)_NumSamples$(SAMPLES)_NumTest$(TESTS)_MaxCycle8000.sh
	rm -Rf benchmark_output/$(TARGET)$(L1D_SFX)_s$(SAMPLES)-n$(TESTS)
	rm -Rf benchmark_output/$(TARGET)$(L1D_SFX)/objects
	rm -Rf benchmark_output/$(TARGET)$(L1D_SFX)_s$(SAMPLES)-n$(TESTS)
	mv benchmark_output/$(TARGET)$(L1D_SFX) \
		benchmark_output/$(TARGET)$(L1D_SFX)_s$(SAMPLES)-n$(TESTS)
	@echo "Benchmark path: benchmark_output/$(TARGET)$(L1D_SFX)_s$(SAMPLES)-n$(TESTS)."


benchmark-l1i:
	mkdir -p benchmark_output
	$(GCC) $(INCLUDE) $(IMPLEM_CFLAGS) $(BENCH_SRC) \
			$(BENCH_AMP_GEN_FLAG) -DI_CACHE_ANALYSIS -DBOARD=$(TARGET) \
			-o generate_compilation_commands_$(TARGET)_l1icache
	./generate_compilation_commands_$(TARGET)_l1icache -m $(TARGET) \
			-n $(TESTS) -s $(SAMPLES) \
			-g $(CROSS_TC) -f "$(COMPLEM_CFLAGS)" $(BENCH_AMP_FLAG)
	rm generate_compilation_commands_$(TARGET)_l1icache
	sh benchmark_scripts/$(TARGET)_NumSamples$(SAMPLES)_NumTest$(TESTS)_MaxCycle8000_l1icache.sh
	rm -Rf benchmark_output/$(TARGET)$(L1I_SFX)_s$(SAMPLES)-n$(TESTS)
	rm -Rf benchmark_output/$(TARGET)$(L1I_SFX)/objects
	rm -Rf benchmark_output/$(TARGET)$(L1I_SFX)_s$(SAMPLES)-n$(TESTS)
	mv benchmark_output/$(TARGET)$(L1I_SFX) \
		benchmark_output/$(TARGET)$(L1I_SFX)_s$(SAMPLES)-n$(TESTS)
	@echo "Benchmark path: benchmark_output/$(TARGET)$(L1I_SFX)_s$(SAMPLES)-n$(TESTS)."

clean:
	rm -Rf benchmark_output/*/
	rm -f benchmark_scripts/*.sh
	rm -f generate_compilation_commands_*
	rm -f timing_*histogram
