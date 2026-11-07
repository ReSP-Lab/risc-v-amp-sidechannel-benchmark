/*
* This file is the header file to define the BeagleV Fire board macros
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

#ifndef __BVFIRE_BOARD_H
#define __BVFIRE_BOARD_H

#include <stdint.h>

/* CONSTANTS */

/* Board-specific constants */
#define CACHE_LINE      64 // 64B cache block - change if needed
#define L1D_CACHE_SIZE  (32*1024)
#define L1D_ASSOC       8
#define L1I_CACHE_SIZE  (32*1024)
#define L1I_ASSOC       8
#ifndef I_CACHE_ANALYSIS
#define L1_CACHE_SIZE   L1D_CACHE_SIZE
#define L1_ASSOC        L1D_ASSOC
#else
#define L1_CACHE_SIZE   L1I_CACHE_SIZE
#define L1_ASSOC        L1I_ASSOC
#endif // !I_CACHE
#define L2_CACHE_SIZE   (2*1024*1024)
#define L2_ASSOC        16

#ifndef AMP_SETUP
#define NO_FLUSH
#endif /* !AMP_SETUP */
#define NO_L3_CACHE
#define NO_RDTIME

/* Eviction strategy parameters */
#define L1D_S   (L1D_ASSOC+2)
#define L1D_D   (L1D_S-1)
#define L1D_C   4
#define L1D_L   1
#define L1I_S   (L1I_ASSOC+2)
#define L1I_D   (L1I_S-1)
#define L1I_C   4
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
#define L2_C    4
#define L2_S    (L2_ASSOC+4)
#define L2_D    (L2_S-1)
#define L2_L    1

/* CPU definitions */
#define LOCAL_CPU1  0
#define LOCAL_CPU2  0
#define REMOTE_CPU1 2
#define REMOTE_CPU2 2
#define REST_CPU    1

#define LOCAL_HART1  2
#define LOCAL_HART2  2
#define REMOTE_HART1 3
#define REMOTE_HART2 3

#define MULTICORE

#define AMP_BASE                0x82000000


/* Board-specific inline instructions */
__attribute__((always_inline))
static inline void flush(char* char_ptr)
{
    /* This Flush64 instruction is only available in M-mode.
     * To flush, you have to write the target address in the dedicated
     * register in memory. Memory address of this register is given by:
     *  - L2 cache controller memory map base address (0x0201_0000)
     *  - Flush64 register offset (0x200)
     */
    const uint64_t* flush64_addr = (uint64_t*) 0x02010200;  // 0x0201_0200
    asm __volatile__(
	    "sd %[c],    0(%[flush64_reg]) \n"
        :
        : [c] "r" (char_ptr), [flush64_reg] "r" (flush64_addr)
        : "a7", "memory"
    );
}


static inline void platform_init()
{
    // no initialization required
}

static inline void publish(char* p)
{
    /*
    asm __volatile__("fence iorw, iorw");
    flush(p);
    */
    asm __volatile__("fence iorw, iorw");
}

static inline void publish_range(char* base, char* end)
{
    /*
    if ((uint64_t) base >= (uint64_t) end)
        return;
    char* new_base = (char*) ((uint64_t) base - (uint64_t) base % CACHE_LINE);

    asm __volatile__("fence iorw, iorw");
    for (char* p = new_base; p < end; p+= CACHE_LINE)
    {
        flush(p);
    }
    */
    asm __volatile__("fence iorw, iorw");
}

#endif
