#ifndef __BENCHMARK_IMPLEM_H
#define __BENCHMARK_IMPLEM_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#include <string.h>
#include <sys/mman.h>
//#include <sys/syscall.h>
#include <sched.h>
//#include <dlfcn.h>
//

#include "benchmark_utils.h"
#include "stats.h"

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

/* Required for `munmap` with huge pages, as indicated by the
 * "Huge page (Huge TLB) mappings" subsection, in
 * https://man7.org/linux/man-pages/man2/mmap.2.html */
#define PROBE_PAGE_BYTES (2ULL * 1024ULL * 1024ULL)
#define PROBE_MAP_BYTES \
    (((MAX_ARRAY_SIZE + PROBE_PAGE_BYTES - 1ULL) / PROBE_PAGE_BYTES) \
     * PROBE_PAGE_BYTES)


/* FUNCTIONS */

int linux_main(int argc, char *argv[]);


#endif // !__BENCHMARK_IMPLEM_H
