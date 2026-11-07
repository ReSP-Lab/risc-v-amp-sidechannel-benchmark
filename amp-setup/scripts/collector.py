from collections import namedtuple
import stats
import targets

import os
import sys
import mmap
import struct
import time
import argparse
import subprocess


# Constants
UINT32_SIZE = 4
INT64_SIZE  = 8
UINT64_SIZE = 8
DOUBLE_SIZE = 8

# "Timing type histograms"-related constants
L1_CLEAN = 0
L2_CLEAN = 1
L3_CLEAN = 2
REMOTE_L1_CLEAN = 3
REMOTE_L2_CLEAN = 4
REMOTE_L3_CLEAN = 5
L1_DIRTY = 6
L2_DIRTY = 7
L3_DIRTY = 8
REMOTE_L1_DIRTY = 9
REMOTE_L2_DIRTY = 10
REMOTE_L3_DIRTY = 11
DRAM = 12
L1_REMOTE_L1_CLEAN = 13
L1_REMOTE_L2_CLEAN = 14
L1_REMOTE_L3_CLEAN = 15
L2_REMOTE_L1_CLEAN = 16
L2_REMOTE_L2_CLEAN = 17
L2_REMOTE_L3_CLEAN = 18
L3_REMOTE_L1_CLEAN = 19
L3_REMOTE_L2_CLEAN = 20
L3_REMOTE_L3_CLEAN = 21
CALIBRE_MAPPING = {
        L1_CLEAN: "L1_CLEAN",
        L2_CLEAN: "L2_CLEAN",
        L3_CLEAN: "L3_CLEAN",
        REMOTE_L1_CLEAN: "REMOTE_L1_CLEAN",
        REMOTE_L2_CLEAN: "REMOTE_L2_CLEAN",
        REMOTE_L3_CLEAN: "REMOTE_L3_CLEAN",
        L1_DIRTY: "L1_DIRTY",
        L2_DIRTY: "L2_DIRTY",
        L3_DIRTY: "L3_DIRTY",
        REMOTE_L1_DIRTY: "REMOTE_L1_DIRTY",
        REMOTE_L2_DIRTY: "REMOTE_L2_DIRTY",
        REMOTE_L3_DIRTY: "REMOTE_L3_DIRTY",
        DRAM: "DRAM",
        L1_REMOTE_L1_CLEAN: "L1_REMOTE_L1_CLEAN",
        L1_REMOTE_L2_CLEAN: "L1_REMOTE_L2_CLEAN",
        L1_REMOTE_L3_CLEAN: "L1_REMOTE_L3_CLEAN",
        L2_REMOTE_L1_CLEAN: "L2_REMOTE_L1_CLEAN",
        L2_REMOTE_L2_CLEAN: "L2_REMOTE_L2_CLEAN",
        L2_REMOTE_L3_CLEAN: "L2_REMOTE_L3_CLEAN",
        L3_REMOTE_L1_CLEAN: "L3_REMOTE_L1_CLEAN",
        L3_REMOTE_L2_CLEAN: "L3_REMOTE_L2_CLEAN",
        L3_REMOTE_L3_CLEAN: "L3_REMOTE_L3_CLEAN",
}
NUM_CALIBRE_SINGLE = 22
READ = 0
WRITE = 1
FLUSH = 2
TIMING_MAPPING = {
        READ: "READ",
        WRITE: "WRITE",
        FLUSH: "FLUSH"
}
CYCLES_STR = "Cycles"
MAX_NUM_CALIBRE = 3*NUM_CALIBRE_SINGLE

# Benchmark-related constants
WT_PROB = 0.0005

PROBE_SIZE = 3

STATS_SENT = 0x55
UNSUPPORTED = 222
IGNORED = 111

TYPE_MATCH = [
    "ATT","VIC",
    "A","A_ALIAS","D","U",
    "INV","A_INV","D_INV","U_INV","A_ALIAS_INV",
    "ERROR1", "ERROR2", "STAR"
]

# Class (benchmark-related results)
class Experiment:
        __slots__ = ("target", "num_test", "num_samples", "max_cycle",
                     "vul_num", "pattern",
                     "histogram_rows", "histogram_row_size", "histogram_size",
                     "arr_rows", "arr_row_size", "arr_size")

        def __init__(self, target, num_test, num_samples, max_cycle,
                     vul_num, pattern):
            self.target = target
            self.num_test = num_test
            self.num_samples = num_samples
            self.max_cycle = max_cycle
            self.vul_num = vul_num
            self.pattern = pattern
            self.histogram_rows = PROBE_SIZE
            self.histogram_row_size = max_cycle
            self.histogram_size = self.histogram_rows * self.histogram_row_size
            self.arr_rows = 6
            self.arr_row_size = num_samples
            #self.arr_row_size = num_samples * (num_test//2)
            self.arr_size = self.arr_rows * self.arr_row_size

        def __str__(self,):
            return f"Target: {self.target}\n"+\
                    f"num_test: {self.num_test}, "+\
                    f"num_samples: {self.num_samples}, "+\
                    f"max_cycle: {self.max_cycle}\n"+\
                    f"vul_num: {self.vul_num}\n"+\
                    f"histogram_size: {self.histogram_size}, "+\
                    f"arr_row_size: {self.arr_row_size}, "+\
                    f"arr_size: {self.arr_size}\n"




# Common functions

def map_amp(fd, amp_base):
    amp_mem = mmap.mmap(fd, targets.AMP_SIZE, flags=mmap.MAP_SHARED,
                        prot=mmap.PROT_WRITE | mmap.PROT_READ,
                        offset=amp_base)
    return amp_mem

def map_data(fd, output_data_base):
    data_mem = mmap.mmap(fd, targets.OUTPUT_DATA_SIZE, flags=mmap.MAP_SHARED,
                         prot=mmap.PROT_WRITE | mmap.PROT_READ,
                         offset=output_data_base)
    return data_mem

def map_memory(fd, proc_info):
    _, amp_base, output_data_base, _ = targets.mem_region_bases(proc_info)
    return map_amp(fd, amp_base), map_data(fd, output_data_base)


# "Timing type histograms"-related functions

def read_histograms(fd, name_arg, i_cache_analysis):
    proc_info = getattr(targets, name_arg+"_info")
    amp_mem, data_mem = map_memory(fd, proc_info)
    print("[*] Collect results from memory...")

    amp_mem.seek(0x120)
    num_calibre = amp_mem.read(1)
    num_calibre = struct.unpack('<B', num_calibre)[0]
    print(f"num_calibre: {num_calibre}")
    num_types = num_calibre//NUM_CALIBRE_SINGLE
    print(f"num_types: {num_types}")
    amp_mem.seek(0x128)
    max_cycle = amp_mem.read(UINT32_SIZE)
    max_cycle = struct.unpack('<I', max_cycle)[0]

    histogram_size = num_calibre*max_cycle
    coarse_histogram_size = int(num_calibre*max_cycle/10)

    hist_offset = 0x00
    coarse_hist_offset = hist_offset + histogram_size*UINT32_SIZE
    min_time_cycle_arr_offset = coarse_hist_offset + \
                                coarse_histogram_size*UINT32_SIZE
    min_time_freq_arr_offset = min_time_cycle_arr_offset + \
                               num_calibre*UINT32_SIZE
    ave_time_cycle_arr_offset = min_time_freq_arr_offset + \
                                num_calibre*UINT32_SIZE

    # Cache type string
    cs = "dcache"
    if i_cache_analysis:
        cs = "icache"

    # Write into histogram file
    with open(f"histogram_output/amp_histogram_{cs}_{name_arg}.csv", "w") as f:
        line = []
        for i in range(num_types):
            if (i_cache_analysis and num_types == 2 and i == 1):
                # RISC-V cores often have non-coherent L1i caches:
                # no write operation will be performed -> i== 1 means FLUSH op.
                _timing_type = [TIMING_MAPPING[FLUSH]
                                for _ in range(NUM_CALIBRE_SINGLE)]
            else:
                _timing_type = [TIMING_MAPPING[i]
                                for _ in range(NUM_CALIBRE_SINGLE)]
            line = [ *line, *_timing_type]
        line = ","+",".join(line)
        f.write(f"{line}\n")

        line = [CYCLES_STR]
        for i in range(num_types):
            _calibre = [c_str for _, c_str in CALIBRE_MAPPING.items()]
            line = [ *line, *_calibre]
        line = ",".join(line)
        f.write(f"{line}\n")

        data_mem.seek(hist_offset)
        for i in range(max_cycle):
            line = [str(i)]
            _row = data_mem.read(num_calibre*UINT32_SIZE)
            _row = struct.unpack(f'<{num_calibre}I', _row)
            _row = [str(_) for _ in _row]
            line = [*line, *_row]
            line = ",".join(line)
            f.write(f"{line}\n")


    # Write into coarse histogram file
    with open(f"histogram_output/amp_coarse_histogram_{cs}_{name_arg}.csv", "w") as f:
        line = []
        for i in range(num_types):
            if (i_cache_analysis and num_types == 2 and i == 1):
                # RISC-V cores often have non-coherent L1i caches:
                # no write operation will be performed -> i== 1 means FLUSH op.
                _timing_type = [TIMING_MAPPING[FLUSH]
                                for _ in range(NUM_CALIBRE_SINGLE)]
            else:
                _timing_type = [TIMING_MAPPING[i]
                                for _ in range(NUM_CALIBRE_SINGLE)]
            line = [ *line, *_timing_type]
        line = ","+",".join(line)
        f.write(f"{line}\n")

        line = [CYCLES_STR]
        for i in range(num_types):
            _calibre = [c_str for _, c_str in CALIBRE_MAPPING.items()]
            line = [ *line, *_calibre]
        line = ",".join(line)
        f.write(f"{line}\n")

        data_mem.seek(coarse_hist_offset)
        for i in range(max_cycle//10):
            line = [str(i*10)]
            _row = data_mem.read(num_calibre*UINT32_SIZE)
            _row = struct.unpack(f'<{num_calibre}I', _row)
            _row = [str(_) for _ in _row]
            line = [*line, *_row]
            line = ",".join(line)
            f.write(f"{line}\n")


    # Provide summary information through standard output
    data_mem.seek(min_time_cycle_arr_offset)
    cycle = data_mem.read(num_calibre*UINT32_SIZE)
    values = struct.unpack(f'<{num_calibre}I', cycle)
    values = [str(_) for _ in values]
    val = ", ".join(values)
    print(f"max. frequency latencies: {val}")
    data_mem.seek(ave_time_cycle_arr_offset)
    ave = data_mem.read(num_calibre*UINT64_SIZE)
    values = struct.unpack(f'<{num_calibre}Q', ave)
    values = [str(_) for _ in values]
    val = ", ".join(values)
    print(f"average frequency latencies: {val}")

    amp_mem.close(); data_mem.close()
    print("[+] Result collection done.")



# Benchmark-related functions

def set_experiment(target, num_test, num_samples, max_cycle, vul_num, pattern):
    return Experiment(target, num_test, num_samples, max_cycle, vul_num, pattern)

def no_postprocessing(fp, fp_res, case, plain, file_name):
    if case == UNSUPPORTED:
        if not plain:
            fp.write(f"\t\t\t\t\t\t\t\t\t\t\t\t\t\t{file_name} Unsupported \n")
            # case that should be skipped (e.g.: no flush support)
            # -> will be considered in IF_PLAIN case
        else:
            fp.write(f"{file_name} Unsupported \n")
            # case that should be skipped (e.g.: no flush support)
            fp_res.write("222\n")
        return True
    elif case == IGNORED:
        if not plain:
            fp.write(f"\t\t\t\t\t\t\t\t\t\t\t\t\t\t{file_name} Ignored \n")
            # case is undesired: local or remote CPUs are identical
            # -> will be considered in IF_PLAIN case
        else:
            fp.write(f"{file_name} Ignored \n")
            # case is undesired: local or remote CPUs are identical
            fp_res.write("111\n")
        return True
    elif case == STATS_SENT:
        return False
    else:
        raise RuntimeError("Invalid outcome.")


def print_m10(fp, plain, sum_v, counter):
    if not plain:
        for i in range(0, PROBE_SIZE):
            mean = sum_v[i] / counter[i] if counter[i] else float("nan")
            fp.write(f"{mean}\t")

def step_operation(step, vul_name, write, plain, i_cache_analysis):
    if step == 2 and plain:
        return "SKIPPED"

    if vul_name in ("A", "A_ALIAS", "D", "U"):
        if write:
            return "WRITE_ACCESS"
        return "EXECUTE_ACCESS" if i_cache_analysis else "READ_ACCESS"

    if vul_name in ("INV", "A_INV", "D_INV", "U_INV", "A_ALIAS_INV"):
        return "WRITE_INVALIDATION" if write else "FLUSH_INVALIDATION"

    if vul_name in ("PRE", "STAR"):
        return "DYNAMIC"

    return "UNKNOWN"

def process_results(fp, fp_res, fp_csv, fp_histo, histogram, arr, file_name,
                    exp, entities, vuls, writes, affinity_rel,
                    i_cache_analysis, plain, u_last_step):

    # - CSV output -
    fields = [exp.vul_num, exp.pattern, plain, affinity_rel, *writes]
    for step in range(3):
        entity = TYPE_MATCH[entities[step]]
        vul = TYPE_MATCH[vuls[step]]
        fields.extend((
            f"{entity}_{vul}",
            step_operation(step + 1, vul, writes[step], plain,
                           i_cache_analysis),
        ))

    if fp_csv.tell() == 0:
        fp_csv.write(
            "vul,pattern,plain,affinity_rel,write1,write2,write3,"
            + "step1,op1,step2,op2,step3,op3,u_last_step,sample,"
            + "a_sum_cycles,a_delta_cycles,"
            + "a_alias_sum_cycles,a_alias_delta_cycles,"
            + "NIB_sum_cycles,NIB_delta_cycles\n"
        )
    for i in range(exp.arr_row_size):
        row = (
            *fields, u_last_step, i,
            arr[0][i], arr[0 + PROBE_SIZE][i],
            arr[1][i], arr[1 + PROBE_SIZE][i],
            arr[2][i], arr[2 + PROBE_SIZE][i],
        )
        fp_csv.write(",".join(map(str, row)) + "\n")

    # - histogram output -
    if fp_histo:
        fp_histo.write("cycles\ta\ta_alias\tNIB\n");
    else:
        print("cycles\ta\ta_alias\tNIB\n");

    for i in range(exp.max_cycle):
        if fp_histo:
            fp_histo.write(f"{i}\t{histogram[0][i]}"+
                  f"\t{histogram[1][i]}\t{histogram[2][i]}\n")
        else:
            print(f"{i}\t{histogram[0][i]}"+
                  f"\t{histogram[1][i]}\t{histogram[2][i]}\n")

    # - original outputs -
    # start calculate the p value of mapped and not mapped data
    pvalue_a_a_alias = stats.p_value(
            arr[0], exp.arr_row_size,
            arr[1], exp.arr_row_size
    )
    pvalue_a_NIB = stats.p_value(
            arr[0], exp.arr_row_size,
            arr[2], exp.arr_row_size
    )
    pvalue_a_alias_NIB = stats.p_value(
            arr[1], exp.arr_row_size,
            arr[2], exp.arr_row_size
    )

    pvalue_a_a_alias_2 = stats.p_value(
            arr[3], exp.arr_row_size,
            arr[4], exp.arr_row_size
    )
    pvalue_a_NIB_2 = stats.p_value(
            arr[3], exp.arr_row_size,
            arr[5], exp.arr_row_size
    )
    pvalue_a_alias_NIB_2 = stats.p_value(
            arr[4], exp.arr_row_size,
            arr[5], exp.arr_row_size
    )

    entity1 = entities[0]
    entity2 = entities[1]
    entity3 = entities[2]
    vul1 = vuls[0]
    vul2 = vuls[1]
    vul3 = vuls[2]

    fp.write(f"\n{exp.pattern}\t{TYPE_MATCH[entity1]}_{TYPE_MATCH[vul1]}"+
             f"\t{TYPE_MATCH[entity2]}_{TYPE_MATCH[vul2]}"+
             f"\t{TYPE_MATCH[entity3]}_{TYPE_MATCH[vul3]}\n")
    fp.write(f"\n{pvalue_a_a_alias}\t{pvalue_a_NIB}\t{pvalue_a_alias_NIB}"+
             f"\t{pvalue_a_a_alias_2}\t{pvalue_a_NIB_2}\t"+
             f"{pvalue_a_alias_NIB_2}\n")

    # first, determine whether an attacker can observe different
    # timings depending on the address pointed by Vu
    welch_cond1 = (pvalue_a_a_alias<WT_PROB and pvalue_a_NIB<WT_PROB) \
        or (pvalue_a_a_alias<WT_PROB and pvalue_a_alias_NIB<WT_PROB) \
        or (pvalue_a_alias_NIB<WT_PROB and pvalue_a_NIB<WT_PROB)
    # then, determine if there already is a timing difference
    # between candidates for cache hits (second access)
    welch_cond2 = (pvalue_a_a_alias_2<WT_PROB and pvalue_a_NIB_2<WT_PROB) \
        or (pvalue_a_a_alias_2<WT_PROB and pvalue_a_alias_NIB_2<WT_PROB) \
        or (pvalue_a_alias_NIB_2<WT_PROB and pvalue_a_NIB_2<WT_PROB)

    # If the first condition is satisfied AND
    # (either the last step is not a U step
    #  OR the second condition is satisfied)
    # -> success
    #
    if welch_cond1 and ((not u_last_step) or welch_cond2):
        if not plain:
            fp.write(f"\t\t\t\t\t\t\t\t\t\t\t\t\t\t{file_name} Succeed \n")
            fp_res.write("1")
        else:
            fp.write(f"{file_name} Succeeds \n\n")
            fp_res.write("1\n")
    #
    # Otherwise -> fail
    #
    else:
        if not plain:
            fp.write(f"\t\t\t\t\t\t\t\t\t\t\t\t\t\t{file_name} Fails \n")
            fp_res.write("0");
        else:
            fp.write(f"{file_name} Fails \n\n")
            fp_res.write("0\n")


def delimit_vulnerability(fp_res, boundary_reached):
    # NOTE: i.e. when plain and the main 4 "for" loops reach their boundary
    if boundary_reached:
        fp_res.write("\n");


def generate_binary_output(fp, fp_res, fp_csv, fp_histo, file_name,
                           exp, histogram, arr, entities, vuls, writes,
                           affinity_rel, i_cache_analysis, sum_v, counter,
                           case, plain, u_last_step, boundary_reached):
    if not no_postprocessing(fp, fp_res, case, plain, file_name):
        process_results(fp, fp_res, fp_csv, fp_histo, histogram, arr,
                        file_name, exp, entities, vuls, writes, affinity_rel,
                        i_cache_analysis, plain, u_last_step)
        print_m10(fp, plain, sum_v, counter)

    delimit_vulnerability(fp_res, boundary_reached)



def read_benchmark_results(fd, name_arg, fp_histo=None, mem=None,
                           file_name=None, output_directory=""):
    to_close = False
    if mem != None:
        amp_mem, data_mem = mem
    else:
        proc_info = getattr(targets, name_arg+"_info")
        amp_mem, data_mem = map_memory(fd, proc_info)
        to_close = True
    print("[*] Collect results from memory...")

    while True:
        amp_mem.seek(0x134)  # data_ready offset
        data_ready = struct.unpack('<B', amp_mem.read(1))[0]
        if data_ready:
            break

    amp_mem.seek(0x128)  # entity offset
    entities = struct.unpack('<3B', amp_mem.read(3))  # 1 per step
    amp_mem.seek(0x12c)  # vul offset
    vuls = struct.unpack('<3B', amp_mem.read(3))  # 1 per step
    amp_mem.seek(0x130)  # outcome offset
    outcome = struct.unpack('<B', amp_mem.read(1))[0]
    # 1 byte away, plain offset
    plain = struct.unpack('<B', amp_mem.read(1))[0]
    # 1 byte away, u_last_step offset
    u_last_step = struct.unpack('<B', amp_mem.read(1))[0]
    # 1 byte away, boundary_reached offset
    boundary_reached = struct.unpack('<B', amp_mem.read(1))[0]
    amp_mem.seek(0x140)  # pattern offset
    pattern = struct.unpack('<I', amp_mem.read(UINT32_SIZE))[0]
    amp_mem.seek(0x144)  # vul_num offset
    vul_num = struct.unpack('<I', amp_mem.read(UINT32_SIZE))[0]
    amp_mem.seek(0x148)  # num_samples offset
    num_samples = struct.unpack('<I', amp_mem.read(UINT32_SIZE))[0]
    amp_mem.seek(0x150)  # num_test offset
    num_test = struct.unpack('<I', amp_mem.read(UINT32_SIZE))[0]
    # 4 bytes away, max_cycle offset
    max_cycle = struct.unpack('<I', amp_mem.read(UINT32_SIZE))[0]
    # 4 bytes away, sum offset
    sum_offset = struct.unpack('<Q', amp_mem.read(UINT64_SIZE))[0]
    # 8 bytes away, m10 offset
    m10_offset = struct.unpack('<Q', amp_mem.read(UINT64_SIZE))[0]
    amp_mem.seek(0x168)
    write1, write2, write3, affinity_rel, i_cache_analysis = struct.unpack(
            "<5B", amp_mem.read(8)[0:5]  # to avoid potential segfaults
    )
    writes = (write1, write2, write3)

    experiment = set_experiment(name_arg, num_test, num_samples, max_cycle,
                                vul_num, pattern)
    print(experiment)

    data_mem.seek(0x0)
    histogram = []
    for _ in range(experiment.histogram_rows):
        row = struct.unpack(
                f'<{experiment.histogram_row_size}I',
                data_mem.read(UINT32_SIZE*experiment.histogram_row_size)
        )
        histogram = [*histogram, row]

    data_mem.seek(UINT32_SIZE*experiment.histogram_size)
    arr = []
    for _ in range(experiment.arr_rows):
        row = struct.unpack(
                f'<{experiment.arr_row_size}q',
                data_mem.read(INT64_SIZE*experiment.arr_row_size)
        )
        arr = [*arr, row]
    data_mem.seek(sum_offset)
    sum_v = struct.unpack(f'<3Q', data_mem.read(UINT64_SIZE*PROBE_SIZE))
    counter = struct.unpack(f'<3Q', data_mem.read(UINT64_SIZE*PROBE_SIZE))
    data_mem.seek(m10_offset)
    m10 = struct.unpack(f'<3d', data_mem.read(DOUBLE_SIZE*PROBE_SIZE))

    if not file_name:
        file_name = name_arg

    output_name = os.path.join(output_directory, f"{name_arg}.output")
    res_name = os.path.join(output_directory, f"{name_arg}.res")
    csv_name = os.path.join(output_directory, f"{name_arg}.csv")
    with open(output_name, "a") as fp, open(res_name, "a") as fp_res, \
            open(csv_name, "a") as fp_csv:
        generate_binary_output(
                fp, fp_res, fp_csv, fp_histo, file_name, experiment,
                histogram, arr, entities, vuls, writes,
                affinity_rel, i_cache_analysis,
                sum_v, counter, outcome, plain, u_last_step, boundary_reached
        )
        fp.flush()
        os.fsync(fp.fileno())
        fp_res.flush()
        os.fsync(fp_res.fileno())
        fp_csv.flush()
        os.fsync(fp_csv.fileno())

    if to_close:
        amp_mem.close(); data_mem.close()
    print("[+] Result collection done.")



# Main section of the collector
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Collect data from memory")
    parser.add_argument(
            "target_name",
            type=str,
            help="string to include in the name of the output file"
    )
    parser.add_argument(
            "--histogram",
            action="store_true",
            help="adapt payload loader to target"
    )
    parser.add_argument(
            "--i-cache-analysis",
            action="store_true",
            help="perform instruction cache analysis (with --histogram)"
    )
    parser.add_argument(
            "--benchmark",
            action="store_true",
            help="Compile and boot the loader on Harts 1-3"
    )
    args = parser.parse_args()

    #fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
    fd = os.open("/dev/mem", os.O_RDWR)

    if not args.target_name:
        parser.error("A target name is required.")

    if args.histogram:
        read_histograms(fd, args.target_name, args.i_cache_analysis)

    if args.benchmark:
        #with open("fp_histo", "w") as fp_histo:
        #    read_benchmark_results(fd, args.target_name, fp_histo)
        read_benchmark_results(fd, args.target_name)

    os.close(fd)


