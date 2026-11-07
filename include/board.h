/*
* This file is the header file for identifying target board macros to use
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

#ifndef __BOARD_H
#define __BOARD_H

/* CONSTANTS */

#define _TARGET(X)          BOARD_##X
#define TARGET(X)           _TARGET(X)
#define BOARD_UNMATCHED     1
#define BOARD_BV_AHEAD      2
#define BOARD_BV_FIRE       3
#define BOARD_BPI_F3        4
#define BOARD_BPI_RV2       5


#ifndef BOARD
    #error "No board defined. Use '-DBOARD=...' to define it."
#endif /* ifndef BOARD */

/* Board-specific constants */
#if TARGET(BOARD) == BOARD_UNMATCHED
#include "boards/unmatched.h"

#elif TARGET(BOARD) == BOARD_BV_AHEAD
#include "boards/bv_ahead.h"

#elif TARGET(BOARD) == BOARD_BV_FIRE
#include "boards/bv_fire.h"

#elif TARGET(BOARD) == BOARD_BPI_F3
#include "boards/bpi_f3.h"

#elif TARGET(BOARD) == BOARD_BPI_RV2
#include "boards/bpi_rv2.h"

//#elif TARGET(BOARD) == BOARD_ESP32_C6
    //#define NO_L2_CACHE
    //#define SINGLE_CORE

#else
    #error "Not matching any board."
    // unsupported board: provide the constants to the compiler
#endif


/* Cache-related information:
 * - either defined by the user at compilation time,
 * - or defined by default.
 */
#ifndef CACHE_LINE
#define CACHE_LINE 0x40 // 64B cache block
#endif

#if defined(L1_CACHE_SIZE) && defined(L1_ASSOC)
#ifndef L1_CACHE_SETS
#define L1_CACHE_SETS (L1_CACHE_SIZE/(L1_ASSOC*CACHE_LINE))
#endif // !L1_CACHE_SETS
#endif


#if defined(L2_CACHE_SIZE) && defined(L2_ASSOC)
#ifndef L2_CACHE_SETS
#define L2_CACHE_SETS (L2_CACHE_SIZE/(L2_ASSOC*CACHE_LINE))
#endif // !L2_CACHE_SETS
#endif


#if defined(L3_CACHE_SIZE) && defined(L3_ASSOC)
#ifndef L3_CACHE_SETS
#define L3_CACHE_SETS (L3_CACHE_SIZE/(L3_ASSOC*CACHE_LINE))
#endif // !L3_CACHE_SETS
#endif

#ifdef NO_L2_CACHE
#ifndef NO_L3_CACHE
#define NO_L3_CACHE
#endif /* ifndef NO_L3_CACHE */
#endif /* ifdef NO_L2_CACHE */

#define SIZE_GT_FACTOR  4
#ifdef NO_L2_CACHE
#ifdef L1_CACHE_SIZE
#define SIZE_GT_LLC   (SIZE_GT_FACTOR*L1_CACHE_SIZE)
#endif /* ifdef L1_CACHE_SIZE */
#elif defined(NO_L3_CACHE)
#ifdef L2_CACHE_SIZE
#define SIZE_GT_LLC   (SIZE_GT_FACTOR*L2_CACHE_SIZE)
#endif /* ifdef L2_CACHE_SIZE */
#else /* !NO_L2_CACHE && !NO_L3_CACHE */
#if defined(L3_CACHE_SIZE)
#define SIZE_GT_LLC   (SIZE_GT_FACTOR*L3_CACHE_SIZE)
#endif /* ifdef L3_CACHE_SIZE */
#endif /* ifdef NO_L2_CACHE */

#define L1_TOTAL_WAYS   (L1_CACHE_SETS*L1_ASSOC)
#ifndef NO_L2_CACHE
#define L2_TOTAL_WAYS   (L2_CACHE_SETS*L2_ASSOC)
#endif /* ifndef NO_L2_CACHE */

#define L1_STRIDE       (L1_CACHE_SETS*CACHE_LINE)
#ifndef NO_L2_CACHE
#define L2_STRIDE       (L2_CACHE_SETS*CACHE_LINE)
#endif /* ifndef NO_L2_CACHE */
#ifndef NO_L3_CACHE
#define L3_STRIDE       (L3_CACHE_SETS*CACHE_LINE)
#endif /* ifndef NO_L3_CACHE */

#ifndef NO_L3_CACHE
#define LLC_STRIDE      L3_STRIDE
#elif !defined(NO_L2_CACHE)
#define LLC_STRIDE      L2_STRIDE
#else
#define LLC_STRIDE      L1_STRIDE
#endif // !NO_L3_CACHE


#define L1 0
#define L2 1
#define L3 2
#ifndef NO_L3_CACHE
#define LLC L3
#define LLC_C   L3_C
#define LLC_D   L3_D
#define LLC_L   L3_L
#define LLC_S   L3_S
#elif !defined(NO_L2_CACHE)
#define LLC L2
#define LLC_C   L2_C
#define LLC_D   L2_D
#define LLC_L   L2_L
#define LLC_S   L2_S
#else
#define LLC L1
#define LLC_C   L1_C
#define LLC_D   L1_D
#define LLC_L   L1_L
#define LLC_S   L1_S
#endif /* ifndef  NO_L3_CACHE */


#if LOCAL_CPU1 == LOCAL_CPU2 || REMOTE_CPU1 == REMOTE_CPU2
#define DIFF_CORE_ONLY  1
#else
#define DIFF_CORE_ONLY  2
#endif /* if LOCAL/REMOTE_CPU1 == LOCAL/REMOTE_CPU2 */

#endif // !__BOARD_H
