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

#ifndef __BPI_RV2_H
#define __BPI_RV2_H

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
#define L2_CACHE_SIZE   (256*1024)
#define L2_CACHE_SETS   256
#define L2_ASSOC        (L2_CACHE_SIZE/(L2_CACHE_SETS*CACHE_LINE))
#define NO_FLUSH
#define NO_L3_CACHE

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

#define MULTICORE


/* Board-specific inline instructions */
__attribute__((always_inline))
static inline void flush(char* char_ptr)
{
    // TODO: define it the processor has a custom flush instruction
}


static inline void platform_init()
{
    // no initialization required
}

static inline void publish(char* p)
{
    // no publication procedure required
}

static inline void publish_range(char* base, char* end)
{
    // no completion required
}


#endif
