#ifndef __STDLIB_SUBSTITUTE_H
#define __STDLIB_SUBSTITUTE_H

#include <stdint.h>
#include <stddef.h>
#include "asm_functions.h"

uint64_t time(uint64_t* time_ptr);

void *memset(void* ptr, int byte, size_t number);

#endif // !__STDLIB_SUBSTITUTE_H
