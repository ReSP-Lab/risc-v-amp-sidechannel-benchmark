#ifndef __TIMING_HISTOGRAM_IMPLEM_H
#define __TIMING_HISTOGRAM_IMPLEM_H

#define _GNU_SOURCE

#include <string.h>
#include <sys/mman.h>
//#include <sys/syscall.h>
#include <sched.h>
//#include <dlfcn.h>

#include "timing_histogram_utils.h"

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

/* Required for `munmap` with huge pages, as indicated by the
 * "Huge page (Huge TLB) mappings" subsection, in
 * https://man7.org/linux/man-pages/man2/mmap.2.html */
#define PROBE_PAGE_BYTES (2ULL * 1024ULL * 1024ULL)
#define PROBE_MAP_BYTES \
    ((((SIZE_GT_LLC) + PROBE_PAGE_BYTES - 1ULL) / PROBE_PAGE_BYTES) \
     * PROBE_PAGE_BYTES)


/* FUNCTIONS */

int linux_main(int argc, char *argv[]);


#endif // !__TIMING_HISTOGRAM_IMPLEM_H
