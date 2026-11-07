#!/bin/bash

#set -xe
#set -e

TARGET=
INPUT_DIR=
NUM_TESTS=
NUM_SAMPLES=

LOOP_INIT=0
LOOP_SIZE=14
I_CACHE_ANALYSIS=0

while [[ $# -gt 0 ]]; do
    case $1 in
       -t|--target) TARGET=$2; shift;;
       -i|--input-dir) INPUT_DIR=$2; shift;;
       -n|--num-tests) NUM_TESTS=$2; shift;;
       -s|--samples) NUM_SAMPLES=$2; shift;;
       -li|--loop-init) LOOP_INIT=$2; shift;;
       -ls|--loop-size) LOOP_SIZE=$2; shift;;
       -ia|--i-cache-analysis) I_CACHE_ANALYSIS=1; shift;;
       *) echo "Unknown parameter: $1"; exit 1;;
    esac
    shift
done

OUTPUT_DIR=$TARGET-s$NUM_SAMPLES-n$NUM_TESTS
if [ $I_CACHE_ANALYSIS -eq 1 ]
then
    OUTPUT_DIR="$OUTPUT_DIR-icache"
fi

#for i in $(seq 0 $LOOP_SIZE)
for ((i=LOOP_INIT; i<=LOOP_SIZE; i++))
do
    #OUTPUT_DIR_I=$OUTPUT_DIR/$TARGET$i
    OUTPUT_DIR_I=$OUTPUT_DIR/
    python3 benchmark-runner.py --target $TARGET \
        --directory $INPUT_DIR --output-directory $OUTPUT_DIR_I > log.txt
        #--directory $INPUT_DIR > /dev/null
    mv $OUTPUT_DIR/$TARGET.res $OUTPUT_DIR/$TARGET$i.res
    mv $OUTPUT_DIR/$TARGET.output $OUTPUT_DIR/$TARGET$i.output
    mv $OUTPUT_DIR/$TARGET.csv $OUTPUT_DIR/$TARGET$i.csv
    gzip $OUTPUT_DIR/$TARGET$i.csv
done
