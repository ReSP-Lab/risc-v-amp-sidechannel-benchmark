import re
import os
import time
import mmap
import argparse

import targets
import collector
import controller

def main(target_name, proc_info, payload, i_cache_analysis, active_slots):
    #fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
    fd = os.open("/dev/mem", os.O_RDWR)
    amp_mem, data_mem = collector.map_memory(fd, proc_info)
    mem = (amp_mem, data_mem)
    _,_,_,exec_base = targets.mem_region_bases(proc_info)

    histo_dir = "histogram_output"
    if not os.path.exists(histo_dir):
        os.makedirs(histo_dir)


    print(f"[*] Loading {payload} histogram payload...")
    for slot in active_slots:
        controller.load_executable(fd, payload, slot, exec_base)

    controller.trigger_cores(fd, proc_info, slots=active_slots)

    print("\n[*] Waiting for harts to complete their task...")
    while not all(
        amp_mem[slot] == controller.AMP_STATE_DONE
        for slot in active_slots
    ):
            time.sleep(0.1)
    print("\n[+] Harts completed their task.")

    print(f"[*] Collecting results...")

    collector.read_histograms(fd, target_name, i_cache_analysis)

    print("[+] Result collection completed.")
    amp_mem.close(); data_mem.close()
    os.close(fd)



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run benchmark and collect "+
                                     "results from memory")
    parser.add_argument(
            "--target",
            type=str,
            choices= targets.target_choices,
            help="string to adapt payload loader to target"
    )
    parser.add_argument(
            "--payload",
            type=str,
            metavar="FILE.bin",
            help="Binary file to run"
    )
    parser.add_argument(
            "--i-cache-analysis",
            action="store_true",
            help="perform instruction cache analysis"
    )
    parser.add_argument(
            "--remote-slot",
            type=int,
            choices=(2, 3, 4),
            default=None,
            help="remote firmware slot (default: 2)",
    )
    args = parser.parse_args()

    if args.target:
        proc_info = getattr(targets, args.target+"_info")
        if not proc_info:
            parser.error("Invalid value for --target")
    else:
        parser.error("--target needs to be specified.")

    if not args.payload:
        parser.error("--payload needs to be specified")

    if args.remote_slot is not None and \
            args.target not in targets.alternative_slot_targets:
        parser.error(f"--remote-slot not supported for {args.target}")

    active_slots = (
        (1, args.remote_slot)
        if args.remote_slot is not None
        else tuple(range(1, proc_info.harts + 1))
    )

    main(args.target, proc_info, args.payload, args.i_cache_analysis,
         active_slots)

