/*
* This file is a template header file to define target board macros
*
* Copyright (C) 2024-2025 Cédrick Austa <cedrick.austa@ulb.be>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*
*/

#ifndef __BPI_F3_H
#define __BPI_F3_H

#include <stdint.h>

/* CONSTANTS */

/* Board-specific constants */
#define CACHE_LINE      64 // 64B cache block - change if needed
#define L1D_CACHE_SIZE  (32*1024)
#define L1D_CACHE_SETS  128
#define L1D_ASSOC       (L1D_CACHE_SIZE/(L1D_CACHE_SETS*CACHE_LINE))
#define L1I_CACHE_SIZE  (32*1024)
#define L1I_CACHE_SETS  128
#define L1I_ASSOC       (L1I_CACHE_SIZE/(L1I_CACHE_SETS*CACHE_LINE))
#ifndef I_CACHE_ANALYSIS
#define L1_CACHE_SIZE   L1D_CACHE_SIZE
#define L1_ASSOC        L1D_ASSOC
#define L1_CACHE_SETS   L1D_CACHE_SETS
#else
#define L1_CACHE_SIZE   L1I_CACHE_SIZE
#define L1_ASSOC        L1I_ASSOC
#define L1_CACHE_SETS   L1I_CACHE_SETS
#endif // !I_CACHE_ANALYSIS
#define L2_CACHE_SIZE   (512*1024)
#define L2_CACHE_SETS   512
#define L2_ASSOC        (L2_CACHE_SIZE/(L2_CACHE_SETS*CACHE_LINE))

#ifndef AMP_SETUP
#define NO_FLUSH
#else
#ifdef I_CACHE_ANALYSIS
/* flush instruction is not dedicated to instructions,
* but the measurement is still interesting to illustrate it */
#define NO_LLC_FLUSH  // NOTE: uncomment to illustrate
//#define NO_FLUSH
#endif // I_CACHE_ANALYSIS
#endif // !AMP_SETUP
#define NO_L3_CACHE
#define NO_RDTIME

/* Eviction strategy parameters */
#define L1D_S   (L1D_ASSOC+2)
#define L1D_D   (L1D_S-1)
#define L1D_C   4
#define L1D_L   1
#define L1I_S   L1I_ASSOC
#define L1I_D   1
#define L1I_C   1
#define L1I_L   1
#ifndef I_CACHE_ANALYSIS
#define L1_S    L1D_S
#define L1_D    L1D_D
#define L1_C    L1D_C
#define L1_L    L1D_L
#else
#define L1_S    L1I_S
#define L1_D    L1I_D
#define L1_C    L1I_C
#define L1_L    L1I_L
#endif // !I_CACHE
#define L2_S    L2_ASSOC
#define L2_C    1
#define L2_D    1
#define L2_L    1

/* CPU definitions */
#define LOCAL_CPU1  4
#define LOCAL_CPU2  4
#define REMOTE_CPU1 5
#define REMOTE_CPU2 5
#ifdef REMOTE_CLUSTER  // TODO: use
#undef REMOTE_CPU1
#undef REMOTE_CPU2
#define REMOTE_CPU1 1
#define REMOTE_CPU2 1
#endif // REMOTE_CLUSTER
#define REST_CPU    2

#define LOCAL_HART1  4
#define LOCAL_HART2  4
#define REMOTE_HART1 5
#define REMOTE_HART2 5
#ifdef REMOTE_CLUSTER  // TODO: use
#undef REMOTE_HART1
#undef REMOTE_HART2
#define REMOTE_HART1 1
#define REMOTE_HART2 1
#endif // REMOTE_CLUSTER

#define MULTICORE
#ifndef AMP_SETUP
#define ENABLE_HPC // rdcycle not available to user mode
#endif // !AMP_SETUP

#define AMP_BASE                0x132000000


/* K1x CUSTOM REGISTERS */

// see: https://github.com/riscv-software-src/opensbi/blob/master/platform/generic/include/spacemit/k1.h
#define CSR_MSETUP              0x7C0
#define CSR_ML2SETUP            0x7F0

#define BIT(__n)                (1UL << (__n))
/* dcache enable */
#define MSETUP_DE               BIT(0)
/* icache enable */
#define MSETUP_IE               BIT(1)
/* branch prediction enable */
#define MSETUP_BPE              BIT(4)
/* prefetch functionality enable */
#define MSETUP_PFE              BIT(5)
/* misaligned memory access enable */
#define MSETUP_MME              BIT(6)
/* ECC enable */
#define MSETUP_ECCE             BIT(16)


/* enable all previous features (value: 0x10073) */
#define MSETUP_ALL              (MSETUP_DE | MSETUP_IE | MSETUP_BPE | \
                                 MSETUP_PFE | MSETUP_MME | MSETUP_ECCE)



static inline void clear_flush(void *p)
{
    asm volatile ("cbo.clean" " 0(%0)" : : "rK"(p): "memory");
}

static inline void inval_flush(void *p)
{
    asm volatile ("cbo.inval" " 0(%0)" : : "rK"(p): "memory");
}

static inline void clear_inval_flush(void *p)
{
    asm volatile ("cbo.flush" " 0(%0)" : : "rK"(p): "memory");
}


/* Board-specific inline instructions */
__attribute__((always_inline))
static inline void flush(char* char_ptr)
{
#ifndef AMP_SETUP
#else
    //clear_flush(char_ptr);
    //inval_flush(char_ptr);
    clear_inval_flush(char_ptr);
#endif
}

static inline void platform_init()
{
    uint64_t hartid;
    __asm__ __volatile__("csrr %0, mhartid" : "=r"(hartid));

    // cci initialization is performed per cluster, not required in our setup

    /* enable cache snooping
     * see: https://www.eevblog.com/forum/embedded-computing/bit-brick-k1/
     * see: `spacemit_cold_boot_allowed` in
     *      "platform/generic/spacemit/spacemit_k1.c"
     *      called from `.cold_boot_allowed` registered at the end of
     *      "platform/generic/spacemit/spacemit_k1.c"
     *      called from `sbi_platform_cold_boot_allowed` in
     *      "include/sbi/sbi_platform.h"
     *      called from `sbi_init` in OpenSBI
     * CSR_ML2SETUP = 0x7F0
     *
     */
    asm volatile(
        "csrs %0, %1"
        :
        : "i" (CSR_ML2SETUP), "r"(1UL << (hartid % 4))
        : "memory"
    );

    /* enable optimization features (L1d & L1i caches, prefetcher, etc.)
     * see: `csi_enable_dcache` in "include/sbi_utils/cache/cacheflush.h"
     *      called from `spacemit_k1_early_init` registered at the end of
     *      "platform/generic/spacemit/spacemit_k1.c"
     *      called from `sbi_platform_early_init`in
     *      "include/sbi/sbi_platform.h"
     *      called from `init_coldboot` in current file
     *      called from `sbi_init` in OpenSBI
     * CSR_MSETUP = 0x7C0
     */
    asm volatile(
        "csrs %0, %1"
        :
        : "i" (CSR_MSETUP), "r"(MSETUP_ALL)
        : "memory"
    );

#ifdef NO_L1_CACHE
    asm volatile(
        "csrc %0, %1"
        :
        : "i" (CSR_MSETUP), "r"(MSETUP_PFE)
        : "memory"
    );
    asm volatile(  /* full L1d cache flush */
        "fence iorw, iorw\n\t"
        "csrwi 0x7c2, 3\n\t"
        "fence iorw, iorw"
        :
        :
        : "memory"
    );
    asm volatile(
        "csrc %0, %1"
        :
        : "i" (CSR_MSETUP), "r"(MSETUP_DE)
        : "memory"
    );
#endif

#ifdef NO_PREFETCHING
    // clear CSR indicated bits
    asm volatile(
        "csrc %0, %1"
        :
        : "i" (CSR_MSETUP), "r"(MSETUP_PFE)  // disable prefetching
        : "memory"
    );
#endif
}

static inline void publish(char* p)
{
    asm __volatile__("fence iorw, iorw");
    /* based on: https://github.com/spacemit-com/opensbi/blob/fe3b1fbb9a216d23a70988ee516ee60771417a7e/lib/sbi/sbi_hsm.c#L185C2-L185C4 */
    /*
    clear_inval_flush(p);
    asm __volatile__("fence iorw, iorw");
    */
}

static inline void publish_range(char* base, char* end)
{ /* for this SBC, it was not required to flush AMP output data */
    /*
    if ((uint64_t) base >= (uint64_t) end)
        return;
    char* new_base = (char*) ((uint64_t) base - (uint64_t) base % CACHE_LINE);

    asm __volatile__("fence iorw, iorw");
    for (char* p = new_base; p < end; p+= CACHE_LINE)
    {
        clear_inval_flush(p);
    }
    */
    asm __volatile__("fence iorw, iorw");
}

#endif
