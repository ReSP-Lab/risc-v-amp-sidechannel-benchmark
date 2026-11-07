import re
import os
import time
import mmap
import argparse

import targets
import collector
import controller

def main(target_name, proc_info, directory, payload_list, output_directory,
         active_slots):
    #fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
    fd = os.open("/dev/mem", os.O_RDWR)
    amp_mem, data_mem = collector.map_memory(fd, proc_info)
    mem = (amp_mem, data_mem)
    _,_,_,exec_base = targets.mem_region_bases(proc_info)

    if output_directory:
        if not os.path.exists(output_directory):
            os.makedirs(output_directory)

        histo_dir = os.path.join(output_directory, "benchmark-histograms")
        if not os.path.exists(histo_dir):
            os.makedirs(histo_dir)


    print("[*] Waiting for execution of benchmarks to complete...")
    for payload in payload_list:
        print(f"[*] Running benchmark {payload}...")
        payload_path = os.path.join(directory, payload)
        for slot in active_slots:
            controller.load_executable(fd, payload_path, slot, exec_base)
        controller.trigger_cores(fd, proc_info, slots=active_slots)

        print("\n[*] Waiting for harts to complete their task...")
        while not all(
            amp_mem[slot] == controller.AMP_STATE_DONE
            for slot in active_slots
        ):
            time.sleep(0.1)
        print("\n[+] Harts completed their task.")

        if output_directory:
            output_file = os.path.join(histo_dir, f"{payload}__histogram.csv")
        else:
            output_file = ".benchmark.tmp"
            output_directory = ""
        with open(output_file, "w") as fp_histo:
            collector.read_benchmark_results(fd, target_name, fp_histo, mem,
                                             payload, output_directory)
        print(f"Finished {payload}")

    print("[+] Execution completed.")
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
            "--directory",
            type=str,
            help="string providing directory where payloads are stored"
    )
    parser.add_argument(
            "--output-directory",
            type=str,
            help="string providing directory where payloads are stored"
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

    if args.remote_slot is not None and \
            args.target not in targets.alternative_slot_targets:
        parser.error(f"--remote-slot not supported for {args.target}")

    active_slots = (
        (1, args.remote_slot)
        if args.remote_slot is not None
        else tuple(range(1, proc_info.harts + 1))
    )

    sanity_checker = re.compile('.*AccOne.*AccTwo.*AccThr.*')
    payload_list = []
    if args.directory:
        for (_, _, filenames) in os.walk(f"{args.directory}/"):
            payload_list.extend(filenames)
            break  # do not look down in the tree stop
        payload_list = [_ for _ in payload_list if sanity_checker.match(_)]
        payload_list.sort()
        if not payload_list:
            parser.error("Provided directory does not contain benchmark "+
                         "binaries")
    else:
        parser.error("--directory needs to be specified.")

    main(args.target, proc_info, args.directory, payload_list,
         args.output_directory, active_slots)

