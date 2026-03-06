GCC=gcc
CROSS_GCC=riscv64-unknown-linux-gnu-gcc
CFLAGS=-O0 -DON_MULTIPLE_LINES=1 -DMMAP_ARRAY
#CFLAGS=-O0
INCLUDE=-Iinclude/
EVICT_SRC=src/evict.c src/eviction_evaluation.c
HIST_SRC=src/evict.c src/timing_histogram.c
BENCH_SRC=src/generate_compilation_commands.c


evict:
	mkdir -p eviction_output
	$(CROSS_GCC) $(INCLUDE) $(CFLAGS) $(EVICT_SRC) -o eviction_histogram

evict-unmatched: TARGET=-DBOARD=UNMATCHED
evict-unmatched:
	mkdir -p eviction_output
	$(CROSS_GCC) $(INCLUDE) $(CFLAGS) $(EVICT_SRC) $(TARGET) -o eviction_histogram


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

histogram:
	mkdir -p histogram_output
	$(CROSS_GCC) $(INCLUDE) $(CFLAGS) $(HIST_SRC) $(TARGET) -o timing_histogram


# template
#benchmark-template: TARGET=
#	@echo "Benchmark compilation done for [...]."

benchmark-unmatched: TARGET=UNMATCHED
benchmark-unmatched: benchmark
	@echo "Benchmark compilation done for HiFive Unmatched."

benchmark-ahead: TARGET=BV_AHEAD
benchmark-ahead: benchmark
	@echo "Benchmark compilation done for BeagleV-Ahead."
	
benchmark-fire: TARGET=BV_FIRE
benchmark-fire: benchmark
	@echo "Benchmark compilation done for BeagleV-Fire."

# This recipe will generate compilation commands, apply them using target
# symbol and generate binaries for the target using cross-compilation
benchmark:
	mkdir -p benchmark_output
	$(GCC) $(INCLUDE) $(CFLAGS) $(BENCH_SRC) -DBOARD=$(TARGET) -o generate_compilation_commands_$(TARGET)
	#./generate_compilation_commands_$(TARGET) -m $(TARGET)
	# for cross-platform, uncomment:
	./generate_compilation_commands_$(TARGET) -m $(TARGET) -g $(CROSS_GCC)
	sh benchmark_scripts/$(TARGET)_EachRun30_NumTest10_MaxCycle8000.sh

clean:
	rm -Rf benchmark_output/*/
	rm -f benchmark_scripts/*.sh
	rm -f generate_compilation_commands_*
	rm -f timing_histogram
