#include "amp_setup/stdlib_substitute.h"


uint64_t time(uint64_t* time_ptr)
{
#ifndef NO_RDTIME
    uint64_t t = rdtime();
#else
    uint64_t t = rdcycle();
#endif /* ifndef NO_RDTIME */
    if (time_ptr != NULL)
        *time_ptr = t;

    return t;
}

void *memset(void* ptr, int byte, size_t number)
{
    for (size_t i = 0; i < number; i++)
    {
        *( (char*) ptr + i ) = (char) byte;
    }
    return ptr;
}
