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

// TODO: adapt the macro
#ifndef __TEMPLATE_BOARD_H
#define __TEMPLATE_BOARD_H

#include <stdint.h>

/* CONSTANTS */

/* Board-specific constants */
// TODO: set the values
#define CACHE_LINE      64 // 64B cache block - change if needed
#define L1D_CACHE_SIZE  ...
#define L1D_ASSOC       ...
#define L1I_CACHE_SIZE  ...
#define L1I_ASSOC       ...
#ifndef I_CACHE_ANALYSIS
#define L1_CACHE_SIZE   L1D_CACHE_SIZE
#define L1_ASSOC        L1D_ASSOC
#else
#define L1_CACHE_SIZE   L1I_CACHE_SIZE
#define L1_ASSOC        L1I_ASSOC
#endif // !I_CACHE_ANALYSIS
#define L2_CACHE_SIZE   ...
#define L2_ASSOC        ...

//#define NO_FLUSH // TODO: uncomment if it has no custom instruction
//#define NO_L3_CACHE // TODO: uncomment if no L3 cache is present
// e.g.:  (TODO: adapt if necessary or remove)
#ifndef AMP_SETUP
#define NO_FLUSH
#else
#ifdef I_CACHE_ANALYSIS
#define NO_FLUSH
//#define L1_FLUSH_INSN  // L1i specific flush instruction
#else
#define NO_LLC_FLUSH  // L1d cache flush instruction exists
#define L1_FLUSH_INSN  // L1d specific flush instruction
#define L2_FLUSH_INSN  // L2 only flush instruction
#endif // I_CACHE_ANALYSIS
#endif // !AMP_SETUP
#define NO_L3_CACHE
#define NO_RDTIME

// NOTE: you can define specific requirements depending on your platform
//#ifndef AMP_SETUP
//#define NO_FLUSH
//#else
//#ifndef I_CACHE_ANALYSIS
//#define NO_LLC_FLUSH  // L1d cache flush instruction exists
//#else
//#define NO_FLUSH
//#endif // !I_CACHE_ANALYSIS
//#endif // !AMP_SETUP

/* Eviction strategy parameters */
// TODO: adapt to the strategy which works on the processor
// TODO: comment levels which do not exist
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
#define L3_C    1
#define L3_D    1
#define L3_L    1
#define L3_S    L3_ASSOC

/* CPU definitions */
// TODO: CPU2 should be identical to CPU1 if no L3 exists or L2 is not shared between at least two cores
// TODO: REMOTE_CPU1 has to be set to a core independent from LOCAL_CPU1
#define LOCAL_CPU1  0
#define LOCAL_CPU2  0
#define REMOTE_CPU1 2
#define REMOTE_CPU2 2
#define REST_CPU    1

#define LOCAL_HART1  1
#define LOCAL_HART2  1
#define REMOTE_HART1 2
#define REMOTE_HART2 2

// TODO: comment if single core
#define MULTICORE

// TODO: uncomment if rdcycle is not available to user mode
//#ifndef AMP_SETUP
//#define ENABLE_HPC // make rdcycle available to user mode through HPC
//#endif // !AMP_SETUP


/* Board-specific inline instructions */
__attribute__((always_inline))
static inline void flush(char* char_ptr)
{
    // TODO: define if the processor has a custom flush instruction
}

/* Board specific L1 and L2 flush instructions */
__attribute__((always_inline))
static inline void l1_flush(char* char_ptr)
{ // TODO: define (optional)
    fence_iorwiorw();
    ...(char_ptr+0);  // TODO: replace "..."
#if ON_MULTIPLE_LINES
    fence_iorwiorw();
    ...(char_ptr+64);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+448);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+256);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+384);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+320);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+192);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+128);  // TODO: replace "..."
#endif // ON_MULTIPLE_LINES
    fence_iorwiorw();
}

__attribute__((always_inline))
static inline void l2_flush(char* char_ptr)
{// TODO: define (optional)
    fence_iorwiorw();
    ...(char_ptr+0);  // TODO: replace "..."
#if ON_MULTIPLE_LINES
    fence_iorwiorw();
    ...(char_ptr+64);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+448);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+256);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+384);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+320);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+192);  // TODO: replace "..."
    fence_iorwiorw();
    ...(char_ptr+128);  // TODO: replace "..."
#endif // ON_MULTIPLE_LINES
    fence_iorwiorw();
}


static inline void platform_init()
{
    // TODO: define if the platform requires specific initialization steps
}

static inline void publish(char* p)
{
    /*
    asm __volatile__("fence iorw, iorw");
    // TODO: define if the platform requires specific publication procedure
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
        // TODO: define if the platform requires a publication procedure
    }
    */
    asm __volatile__("fence iorw, iorw");
}




#endif
