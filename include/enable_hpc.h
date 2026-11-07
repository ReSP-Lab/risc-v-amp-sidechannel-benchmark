#ifndef __ENABLE_HPC_H
#define __ENABLE_HPC_H

#include <stdio.h>
#include <stdlib.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>

struct perf_struct {
    struct perf_event_attr pe;
    long long t1;
    long long t2;
    int fd;
};
struct perf_struct* ps;

static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;
    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                    group_fd, flags);
    return ret;
}

int init_perf_struct()
{
    ps = mmap(0, sizeof(struct perf_struct), PROT_READ|PROT_WRITE,
            MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (!ps) {
        perror("mmap failed for ps");
        return EXIT_FAILURE;
    }
    memset(&(ps->pe), 0, sizeof(struct perf_event_attr));
    (ps->pe).type = PERF_TYPE_HARDWARE;
    (ps->pe).size = sizeof(struct perf_event_attr);
    (ps->pe).config = PERF_COUNT_HW_CPU_CYCLES;
    (ps->pe).disabled = 1;
    (ps->pe).exclude_kernel = 1;
    // Don't count hypervisor events.
    (ps->pe).exclude_hv = 1;

    return EXIT_SUCCESS;
}

void set_perf_event()
{
    (ps->fd) = perf_event_open(&(ps->pe), 0, -1, -1, 0);
    if ((ps->fd) == -1) {
        fprintf(stderr, "Error opening leader %llx\n", (ps->pe).config);
        exit(EXIT_FAILURE);
    }
    // enable HPCs through perf
    ioctl((ps->fd), PERF_EVENT_IOC_RESET, 0);
    ioctl((ps->fd), PERF_EVENT_IOC_ENABLE, 0);
}

void clean_perf_struct()
{
    // disable HPCs through perf
    ioctl((ps->fd), PERF_EVENT_IOC_DISABLE, 0);
    munmap(ps, sizeof(struct perf_struct));
}

#endif // !__ENABLE_HPC_H
