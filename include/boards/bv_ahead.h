/*
* This file is the header file to define the BeagleV Ahead board macros
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

#ifndef __BVAHEAD_BOARD_H
#define __BVAHEAD_BOARD_H

#include <stdint.h>

/* CONSTANTS */

/* Board-specific constants */
#define CACHE_LINE      64 // 64B cache block
#define L1D_CACHE_SIZE  (64*1024)
#define L1D_ASSOC       2
#define L1I_CACHE_SIZE  (32*1024)
#define L1I_ASSOC       2
#ifndef I_CACHE_ANALYSIS
#define L1_CACHE_SIZE   L1D_CACHE_SIZE
#define L1_ASSOC        L1D_ASSOC
#else
#define L1_CACHE_SIZE   L1I_CACHE_SIZE
#define L1_ASSOC        L1I_ASSOC
#endif // !I_CACHE_ANALYSIS
#define L2_CACHE_SIZE   (1*1024*1024)
#define L2_ASSOC        16
#define NO_L3_CACHE

#if defined(I_CACHE_ANALYSIS) && \
    (defined(ONLY_L1_FLUSH) || defined(ONLY_L2_FLUSH))
#define NO_LLC_FLUSH
#define L1_FLUSH_INSN
#define L2_FLUSH_INSN
#endif // if defined(I_CACHE_ANALYSIS) && \
    (defined(ONLY_L1_FLUSH) || defined(ONLY_L2_FLUSH))

/* Eviction strategy parameters */
#define L1D_C   1
#define L1D_D   1
#define L1D_L   1
#define L1D_S   L1D_ASSOC
#define L1I_C   1
#define L1I_D   1
#define L1I_L   1
#define L1I_S   (L1I_ASSOC+1)
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
#define L2_C    1
#define L2_D    1
#define L2_L    1
#define L2_S    L2_ASSOC

/* CPU definitions */
#define LOCAL_CPU1  0
#define LOCAL_CPU2  0
#define REMOTE_CPU1 2
#define REMOTE_CPU2 2
#define REST_CPU    1

#define LOCAL_HART1  1
#define LOCAL_HART2  1
#define REMOTE_HART1 2
#define REMOTE_HART2 2

#define MULTICORE

#define AMP_BASE                0x82000000


/* Board-specific inline instructions */
__attribute__((always_inline))
static inline void iflush(char* char_ptr)
{
    asm __volatile__(
        "xor a7, a7, a7\n"
        "add a7, a7, %0\n"
        ".long 0x308800b" // ICACHE.IVA a7
        :
        : "r"(char_ptr)
        : "a7","memory");
}
__attribute__((always_inline))
static inline void dflush(char* char_ptr)  /* L1d cache and L2 cache flush */
{
    asm __volatile__(
        "addi a7, %[c], 0\n"
        ".long 0x278800b \n"
        :
        : [c] "r" (char_ptr)
        : "a7", "memory"
    );
}

__attribute__((always_inline))
static inline void l1_flush(char* char_ptr)
{
#ifdef I_CACHE_ANALYSIS
    iflush(char_ptr);
#endif // I_CACHE_ANALYSIS
}

__attribute__((always_inline))
static inline void l2_flush(char* char_ptr)
{
    dflush(char_ptr);
}


__attribute__((always_inline))
static inline void flush(char* char_ptr)
{
#if defined(I_CACHE_ANALYSIS) && !defined(ONLY_L2_FLUSH)
    iflush(char_ptr);  // remove from L1i cache before flushing from L2 cache
#endif // I_CACHE_ANALYSIS

#if defined(I_CACHE_ANALYSIS) && defined(ONLY_L1_FLUSH)
    // no L2 flush
#else
    dflush(char_ptr);  // only one use is sufficient for TH1520
#endif // if defined(I_CACHE_ANALYSIS) && defined(ONLY_L1_FLUSH)
}

static inline void platform_init()
{
    // no initialization required
}

static inline void publish(char* p)
{
    /*
    asm __volatile__("fence iorw, iorw");
    dflush(p);
    */
    asm __volatile__("fence iorw, iorw");
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
        dflush(p);
    }
    */
    asm __volatile__("fence iorw, iorw");
}



#endif // !__BVAHEAD_BOARD_H
