import os
import sys
import mmap
import struct
import time
import argparse
import subprocess

import targets

# Memory Configuration
LOADER_BASE  = 0x00
AMP_BASE     = 0x00
EXEC_BASE    = 0x00

LAUNCH_MAGIC = 0x1337C910

# AMP States
AMP_STATE_IDLE = 0
AMP_STATE_RUN  = 1
AMP_STATE_DONE = 2
AMP_STATE_LOADER_OK = 0xF


# Variables
target = None


# Functions

def format_hart_state(value: int) -> str:
    states = {
        AMP_STATE_IDLE: "IDLE",
        AMP_STATE_RUN: "RUN",
        AMP_STATE_DONE: "DONE",
        AMP_STATE_LOADER_OK: "LOADER OK",
    }
    return states.get(value, f"0x{value:x}")


def compile_and_load_loader(fd, loader_base, amp_base):
    print("[*] Compiling loader.c ...")
    try:
        subprocess.run(["gcc", "-O2", "-nostdlib", "-T", "linker_loader.ld", "loader.c", "-o", "loader.elf"], check=True)
        subprocess.run(["objcopy", "-O", "binary", "loader.elf", "loader.bin"], check=True)

    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Loader compilation failed: {e}")
        sys.exit(1)

    with open("loader.bin", "rb") as f:
        loader_binary = f.read()

    print(f"[+] Flashing loader to 0x{loader_base:08X}...")
    loader_mem = mmap.mmap(fd, targets.LOADER_SIZE, flags=mmap.MAP_SHARED,
                           prot=mmap.PROT_WRITE | mmap.PROT_READ,
                           offset=loader_base)
    loader_mem.seek(0)
    loader_mem.write(loader_binary)
    loader_mem.close()

    # Remove the compiled files
    try:
        subprocess.run(["rm", "loader.elf"], check=True)
        subprocess.run(["rm", "loader.bin"], check=True)

    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Payload removal failed: {e}")

    amp_mem = mmap.mmap(fd, targets.AMP_SIZE, flags=mmap.MAP_SHARED,
                        prot=mmap.PROT_WRITE | mmap.PROT_READ, offset=amp_base)

    print("[*] Sending hardware launch triggers to Harts (from 1 to 4)...")
    magic_pack = struct.pack('<I', LAUNCH_MAGIC)
    amp_mem.seek(0xA0)
    amp_mem.write(magic_pack)
    amp_mem.seek(0xB0)
    amp_mem.write(magic_pack)
    amp_mem.seek(0xC0)
    amp_mem.write(magic_pack)
    amp_mem.seek(0xD0)
    amp_mem.write(magic_pack)
    amp_mem.close()
    print("[+] All Harts deployed and running loader daemon.")

def load_executable(fd, file, hart_id, exec_base):
    # Calculate unique base address for this specific core
    core_exec_base = exec_base + ((hart_id - 1) * targets.EXEC_SIZE)

    with open(file, "rb") as f:
        payload_binary = f.read()

    # check alignment restrictions to avoid SEGFAULTs
    missing_bytes = len(payload_binary) % 8
    if missing_bytes != 0:
        payload_binary += b'\x00' * (8 - missing_bytes)


    print(f"[*] Flashing {len(payload_binary)} bytes to:\n"
          + f"- 0x{core_exec_base:08X}\n")

    exec_mem = mmap.mmap(fd, targets.EXEC_SIZE, flags=mmap.MAP_SHARED,
                         prot=mmap.PROT_WRITE | mmap.PROT_READ,
                         offset=core_exec_base)
    exec_mem.seek(0)
    exec_mem.write(payload_binary)
    exec_mem.close()

def trigger_cores(fd, target_info, only_one_hart=False, single_hart=0,
                  slots=None):
    _,amp_base,_,_ = targets.mem_region_bases(target_info)
    # Trigger the specific core
    amp_mem = mmap.mmap(
            fd, targets.AMP_SIZE, flags=mmap.MAP_SHARED,
            prot=mmap.PROT_WRITE | mmap.PROT_READ, offset=amp_base
    )

    if slots is None:
        slots = range(1, target_info.harts + 1)

    hart_ids = {
        slot: getattr(target_info, f"hart{slot}")
        for slot in slots
    }

    if not only_one_hart:
        for hart_nr, hart_id in hart_ids.items():
            if amp_mem[hart_nr] != AMP_STATE_IDLE:
                print(f"[!] Warning: Core {hart_id} is not IDLE! "+
                      "Overriding state anyway...")

        str_list_harts = ", ".join(
                    str(hart) for hart in hart_ids.values()
                )

        print(f"[*] Sending RUN command to Cores {str_list_harts}...")
        for hart_nr, hart_id in hart_ids.items():
            amp_mem[hart_nr] = AMP_STATE_RUN
        amp_mem.close()
        print(f"[+] Cores {str_list_harts} are now executing their payload.")
    else:
        hart_id = hart_ids.get(single_hart)
        if hart_id is None:
            raise RuntimeError("Invalid hart number")
        if amp_mem[single_hart] != AMP_STATE_IDLE:
            print(f"[!] Warning: Core {hart_id} is not IDLE! "+
                  "Overriding state anyway...")

        print(f"[*] Sending RUN command to Core {hart_id}...")
        amp_mem[single_hart] = AMP_STATE_RUN
        amp_mem.close()
        print(f"[+] Core {single_hart} is now executing its payload.")


def get_payload_base_addr(hart_id, exec_base):
    if hart_id == 1:
        return f"0x{exec_base:X}"
    if hart_id == 2:
        return f"0x{exec_base+100000:X}"
    if hart_id == 3:
        return f"0x{exec_base+200000:X}"
    if hart_id == 4:
        return f"0x{exec_base+300000:X}"

def monitor_harts(fd, core_info, only_one_hart=False, single_hart=0):
    _,amp_base,_,_ = targets.mem_region_bases(core_info)
    amp_mem = mmap.mmap(fd, targets.AMP_SIZE, flags=mmap.MAP_SHARED,
                        prot=mmap.PROT_WRITE | mmap.PROT_READ, offset=amp_base)

    print("\n[*] Monitoring Hart States (Press Ctrl+C to exit)...")
    try:
        if not only_one_hart:
            if core_info.harts == 2:
                while True:
                    h1 = amp_mem[1]
                    h2 = amp_mem[2]
                    print(
                        f"\rHart {core_info.hart1}: {format_hart_state(h1)} "+
                        f"| Hart {core_info.hart2}: {format_hart_state(h2)}",
                        end=""
                    )
                    time.sleep(0.1)
            else:
                while True:
                    h1 = amp_mem[1]
                    h2 = amp_mem[2]
                    h3 = amp_mem[3]
                    h4 = amp_mem[4]
                    print(
                        f"\rHart {core_info.hart1}: {format_hart_state(h1)} "+
                        f"| Hart {core_info.hart2}: {format_hart_state(h2)} "+
                        f"| Hart {core_info.hart3}: {format_hart_state(h3)} "+
                        f"| Hart {core_info.hart4}: {format_hart_state(h4)}",
                        end=""
                    )
                    time.sleep(0.1)
        else:
            hart_ids = {(i+1): getattr(core_info, "hart"+str(i+1))
                       for i in range(core_info.harts)}
            hart_id = hart_ids.get(single_hart)
            while True:
                h1 = amp_mem[single_hart]
                print(
                    f"\rHart {hart_id}: {format_hart_state(h1)} ",
                    end=""
                )
                time.sleep(0.1)

    except KeyboardInterrupt:
        print("\n[*] Exiting monitor.")

    finally:
        amp_mem.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compile and load payload "+
                                     "loader to memory")
    parser.add_argument(
            "--hart",
            type=int,
            help="hart to run payload with"
    )
    parser.add_argument(
            "--target",
            type=str,
            choices= targets.target_choices,
            help="adapt payload loader to target"
    )
    parser.add_argument(
            "--loader",
            action="store_true",
            help="Compile and boot the loader on Harts 1-3"
    )
    parser.add_argument(
            "--payload",
            action="append",
            type=str,
            metavar="FILE.bin",
            help="Binary files to run" +
                 " (specify twice; local binary then remote binary)"
    )
    args = parser.parse_args()

    #fd = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
    fd = os.open("/dev/mem", os.O_RDWR)

    proc_info = None
    if args.target:
        proc_info = getattr(targets, args.target+"_info")
        #global LOADER_BASE, AMP_BASE, EXEC_BASE
        LOADER_BASE,AMP_BASE,_,EXEC_BASE = targets.mem_region_bases(proc_info)
    else:
        parser.error("--target needs to be specified.")


    if args.hart != None and not args.payload:
        print("/!\\ --core needs a payload to launch /!\\")
        monitor_harts(fd, None, True, args.hart)
        os.close(fd)
        exit(0)
    elif args.hart != None:  # core can be 0
        load_executable(fd, args.payload[0], args.hart, EXEC_BASE)
        trigger_cores(fd, proc_info, True, args.hart)
        monitor_harts(fd, proc_info, True, args.hart)
        os.close(fd)
        exit(0)


    if args.loader:
        compile_and_load_loader(fd, LOADER_BASE, AMP_BASE)

    # allow to ensure the loader loading was successfully performed,
    # before manually running with the payload argument
    if args.payload and not args.loader:
        if len(args.payload) == 1:
            payload = args.payload[0]
            load_executable(fd, payload, 1, EXEC_BASE)
            load_executable(fd, payload, 2, EXEC_BASE)
            if proc_info.harts > 2:
                load_executable(fd, payload, 3, EXEC_BASE)
                load_executable(fd, payload, 4, EXEC_BASE)
        else:
            for i in range(0, proc_info.harts):
                load_executable(fd, args.payload[i], i+1, EXEC_BASE)
        trigger_cores(fd, proc_info)

    monitor_harts(fd, proc_info)
    os.close(fd)


