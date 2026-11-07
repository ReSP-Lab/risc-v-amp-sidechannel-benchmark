/*
* This file defines assembly functions for memory operations
*
* Copyright (C) 2024-2025 Cédrick Austa <cedrick.austa@ulb.be>
* Copyright (C) 2020 Shuwen Deng <shuwen.deng@yale.edu>
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

#ifndef __ASM_FUNCTIONS_H
#define __ASM_FUNCTIONS_H

#include <stdint.h>
#include "board.h"

#ifndef ON_MULTIPLE_LINES
#define ON_MULTIPLE_LINES 0
#endif // !ON_MULTIPLE_LINES


__attribute__((always_inline))
static inline uint64_t get_hartid()
{
    uint64_t hart_id;
    __asm__ __volatile__("csrr %0, mhartid" : "=r"(hart_id));
    return hart_id;
}

__attribute__((always_inline))
static inline void fence_rwrw()
{
    asm __volatile__(
        "fence rw, rw \n"// mfence
    );
}

__attribute__((always_inline))
static inline void fence_iorwiorw()
{
    asm __volatile__(
        "fence iorw, iorw \n"// mfence
    );
}

__attribute__((always_inline))
static inline void fence_rrw()
{
    asm __volatile__(
        "fence r, rw \n"// lfence
    );
}

__attribute__((always_inline))
static inline void fence_ww()
{
    asm __volatile__(
        "fence w, w \n"// sfence
    );
}


__attribute__((always_inline))
static inline void fencei()
{
    asm __volatile__ (
            "fence.i\n"
    );
}

__attribute__((always_inline))
static inline uint64_t rdcycle()
{
    uint64_t time;
    asm __volatile__ (
        "rdcycle %[t]       \n"
        : [t] "=r" (time)
    );
    return time;
}

__attribute__((always_inline))
static inline uint64_t rdtime()
{
    uint64_t time;
    asm __volatile__ (
        "rdtime %[t]       \n"
        : [t] "=r" (time)
    );
    return time;
}

__attribute__((always_inline))
static inline void jmp_isn_addr(char* char_ptr)
{
    asm __volatile__ (
        "mv t0, %0 \n"
        "jalr ra, t0 \n"
        :
        : "r" (char_ptr)
        : "t0", "ra"
    );
}

__attribute__((always_inline))
static inline uint64_t jmp_timing(char* char_ptr)
{
    uint64_t t1, t2;
    uint64_t time;
    //
    fence_iorwiorw();
    t1 = rdcycle();
    fence_iorwiorw();
    jmp_isn_addr(char_ptr+0);
#if ON_MULTIPLE_LINES
    fence_rrw();
    jmp_isn_addr(char_ptr+64);
    fence_rrw();
    jmp_isn_addr(char_ptr+448);
    fence_rrw();
    jmp_isn_addr(char_ptr+256);
    fence_rrw();
    jmp_isn_addr(char_ptr+384);
    fence_rrw();
    jmp_isn_addr(char_ptr+320);
    fence_rrw();
    jmp_isn_addr(char_ptr+192);
    fence_rrw();
    jmp_isn_addr(char_ptr+128);
#endif // ON_MULTIPLE_LINES
    fence_iorwiorw();
    t2 = rdcycle();
    time = t2-t1;
    //
    /*
    asm __volatile__ (
        "fence iorw, iorw   \n"
        "rdcycle t1         \n"
        "fence ior, iorw    \n"
        "jalr ra, 0(%[p])   \n"
#if ON_MULTIPLE_LINES
        "fence r, rw        \n"
        "jalr ra, 64(%[p])  \n"
        "fence r, rw        \n"
        "jalr ra, 448(%[p]) \n"
        "fence r, rw        \n"
        "jalr ra, 256(%[p]) \n"
        "fence r, rw        \n"
        "jalr ra, 384(%[p]) \n"
        "fence r, rw        \n"
        "jalr ra, 320(%[p]) \n"
        "fence r, rw        \n"
        "jalr ra, 192(%[p]) \n"
        "fence r, rw        \n"
        "jalr ra, 128(%[p]) \n"
#endif // ON_MULTIPLE_LINES
        "fence ior, iorw    \n"
        "rdcycle t2         \n"
        "sub %[t], t2, t1   \n"
        : [t] "=r" (time)
        : [p] "r" (char_ptr)
        : "t1", "t2", "ra", "memory"
    );
    */
    return time;
}

__attribute__((always_inline))
static inline void jmp_mfenced(char* char_ptr)
{
    fence_rrw();
    jmp_isn_addr(char_ptr+0);
    fence_rrw();
#if ON_MULTIPLE_LINES
    jmp_isn_addr(char_ptr+64);
    fence_rrw();
    jmp_isn_addr(char_ptr+448);
    fence_rrw();
    jmp_isn_addr(char_ptr+256);
    fence_rrw();
    jmp_isn_addr(char_ptr+384);
    fence_rrw();
    jmp_isn_addr(char_ptr+320);
    fence_rrw();
    jmp_isn_addr(char_ptr+192);
    fence_rrw();
    jmp_isn_addr(char_ptr+128);
    fence_rrw();
#endif // ON_MULTIPLE_LINES
}

__attribute__((always_inline))
static inline void jmp(char* char_ptr)
{
    fence_rrw();
    jmp_isn_addr(char_ptr+0);
#if ON_MULTIPLE_LINES
    jmp_isn_addr(char_ptr+64);
    jmp_isn_addr(char_ptr+448);
    jmp_isn_addr(char_ptr+256);
    jmp_isn_addr(char_ptr+384);
    jmp_isn_addr(char_ptr+320);
    jmp_isn_addr(char_ptr+192);
    jmp_isn_addr(char_ptr+128);
    fence_rrw();
#endif // ON_MULTIPLE_LINES
}


__attribute__((always_inline))
static inline uint64_t mread_timing(char* char_ptr)
{
    uint64_t time = 0;
    asm __volatile__ (
        "fence iorw, iorw         \n"
        "rdcycle t1          \n"
        "fence ior, rw         \n"
        "ld t0,   0(%[c])    \n"
#if ON_MULTIPLE_LINES
        "fence r, rw         \n"
        "ld t0,  64(%[c])    \n"
        "fence r, rw         \n"
        "ld t0, 448(%[c])    \n"
        "fence r, rw         \n"
        "ld t0, 256(%[c])    \n"
        "fence r, rw         \n"
        "ld t0, 384(%[c])    \n"
        "fence r, rw         \n"
        "ld t0, 320(%[c])    \n"
        "fence r, rw         \n"
        "ld t0, 192(%[c])    \n"
        "fence r, rw         \n"
        "ld t0, 128(%[c])    \n"
#endif // ON_MULTIPLE_LINES
        "fence r, iorw         \n"
        "rdcycle t2          \n"
        "sub %[t], t2, t1    \n"
        : [t] "=r" (time)
        : [c] "r" (char_ptr)
        : "t0", "t1", "t2", "memory"
    );
    return time;
}

__attribute__((always_inline))
static inline void mread_mfenced(char* char_ptr)
{
    asm __volatile__ (
        "fence rw, rw         \n"
        "ld t0,   0(%[c])     \n"
        "fence rw, rw         \n"
#if ON_MULTIPLE_LINES
        "ld t0,  64(%[c])     \n"
        "fence rw, rw         \n"
        "ld t0, 448(%[c])     \n"
        "fence rw, rw         \n"
        "ld t0, 256(%[c])     \n"
        "fence rw, rw         \n"
        "ld t0, 384(%[c])     \n"
        "fence rw, rw         \n"
        "ld t0, 320(%[c])     \n"
        "fence rw, rw         \n"
        "ld t0, 192(%[c])     \n"
        "fence rw, rw         \n"
        "ld t0, 128(%[c])     \n"
        "fence rw, rw         \n"
#endif // ON_MULTIPLE_LINES
        :
        : [c] "r" (char_ptr)
        : "t0", "memory"
    );
}

__attribute__((always_inline))
static inline void mread(char* char_ptr)
{
    asm __volatile__ (
        "fence r, rw         \n"
        "ld t0,   0(%[c])    \n"
#if ON_MULTIPLE_LINES
        "ld t0,  64(%[c])    \n"
        "ld t0, 448(%[c])    \n"
        "ld t0, 256(%[c])    \n"
        "ld t0, 384(%[c])    \n"
        "ld t0, 320(%[c])    \n"
        "ld t0, 192(%[c])    \n"
        "ld t0, 128(%[c])    \n"
#endif // ON_MULTIPLE_LINES
        "fence r, rw         \n"
        :
        : [c] "r" (char_ptr)
        : "t0", "memory"
    );
}


__attribute__((always_inline))
static inline uint64_t mwrite_timing(char* char_ptr)
{
    uint64_t time = 0;
    asm __volatile__ (
        "fence iorw, iorw           \n"
        "rdcycle t1         \n"
        "fence iorw, rw           \n"
        "sd %[c],    0(%[c])    \n"
#if ON_MULTIPLE_LINES
        "fence rw, rw           \n"
        "sd %[c],   64(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  448(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  256(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  384(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  320(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  192(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  128(%[c])    \n"
#endif // ON_MULTIPLE_LINES
        "fence rw, iorw           \n"
        "rdcycle t2         \n"
        "sub %[t], t2, t1   \n"
        : [t] "=r" (time)
        : [c] "r" (char_ptr)
        : "t1", "t2", "memory"
    );
    return time;
}

__attribute__((always_inline))
static inline void mwrite_mfenced(char* char_ptr)
{
    asm __volatile__ (
        "fence rw, rw           \n"
        "sd %[c],    0(%[c])    \n"
        "fence rw, rw           \n"
#if ON_MULTIPLE_LINES
        "sd %[c],   64(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  448(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  256(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  384(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  320(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  192(%[c])    \n"
        "fence rw, rw           \n"
        "sd %[c],  128(%[c])    \n"
        "fence rw, rw           \n"
#endif // ON_MULTIPLE_LINES
        :
        : [c] "r" (char_ptr)
        : "memory"
    );
}

__attribute__((always_inline))
static inline void mwrite_dirty(char* char_ptr)
{
    asm __volatile__ (
        "fence w, w             \n"
        "sd %[c],     (%[c])    \n"
#if ON_MULTIPLE_LINES
        "fence w, w             \n"
        "sd %[c],   64(%[c])    \n"
        "fence w, w             \n"
        "sd %[c],  448(%[c])    \n"
        "fence w, w             \n"
        "sd %[c],  256(%[c])    \n"
        "fence w, w             \n"
        "sd %[c],  384(%[c])    \n"
        "fence w, w             \n"
        "sd %[c],  320(%[c])    \n"
        "fence w, w             \n"
        "sd %[c],  192(%[c])    \n"
        "fence w, w             \n"
        "sd %[c],  128(%[c])    \n"
#endif // ON_MULTIPLE_LINES
        "fence w, w             \n"
        :
        : [c] "r" (char_ptr)
        : "memory"
    );
}



__attribute__((always_inline))
static inline uint64_t mflush_timing(char* char_ptr)
{
    uint64_t t1, t2;
    uint64_t time;
#ifndef NO_FLUSH
    fence_iorwiorw();
    t1 = rdcycle();
    fence_iorwiorw();
    flush(char_ptr+0);
#if ON_MULTIPLE_LINES
    fence_iorwiorw();
    flush(char_ptr+64);
    fence_iorwiorw();
    flush(char_ptr+448);
    fence_iorwiorw();
    flush(char_ptr+256);
    fence_iorwiorw();
    flush(char_ptr+384);
    fence_iorwiorw();
    flush(char_ptr+320);
    fence_iorwiorw();
    flush(char_ptr+192);
    fence_iorwiorw();
    flush(char_ptr+128);
#endif // ON_MULTIPLE_LINES
    fence_iorwiorw();
    t2 = rdcycle();
    time = t2-t1;
#endif // !NO_FLUSH
    return time;
}

__attribute__((always_inline))
static inline void mflush_mfenced(char* char_ptr)
{
#ifndef NO_FLUSH
    fence_iorwiorw();
    flush(char_ptr+0);
    fence_iorwiorw();
#if ON_MULTIPLE_LINES
    flush(char_ptr+64);
    fence_iorwiorw();
    flush(char_ptr+448);
    fence_iorwiorw();
    flush(char_ptr+256);
    fence_iorwiorw();
    flush(char_ptr+384);
    fence_iorwiorw();
    flush(char_ptr+320);
    fence_iorwiorw();
    flush(char_ptr+192);
    fence_iorwiorw();
    flush(char_ptr+128);
    fence_iorwiorw();
#endif // ON_MULTIPLE_LINES
#endif // !NO_FLUSH
}


__attribute__((always_inline))
static inline void mflush(char* char_ptr)
{
#ifndef NO_FLUSH
    fence_iorwiorw();
    flush(char_ptr+0);
#if ON_MULTIPLE_LINES
    flush(char_ptr+64);
    flush(char_ptr+448);
    flush(char_ptr+256);
    flush(char_ptr+384);
    flush(char_ptr+320);
    flush(char_ptr+192);
    flush(char_ptr+128);
#endif // ON_MULTIPLE_LINES
    fence_iorwiorw();
#endif // !NO_FLUSH
}

#endif // !__ASM_FUNCTIONS_H
