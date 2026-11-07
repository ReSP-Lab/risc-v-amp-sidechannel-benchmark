#include <stdint.h>

#define CACHE_LINE      64 // 64B cache block
#define L2_CACHE_SIZE   // TODO: fill
// TODO: you may comment out the one which is not provided to you
#define L2_ASSOC        // TODO: fill
#define L2_CACHE_SETS   // TODO: fill

#if defined(L2_CACHE_SIZE) && defined(L2_ASSOC)
#ifndef L2_CACHE_SETS
#define L2_CACHE_SETS   L2_CACHE_SIZE/(L2_ASSOC*CACHE_LINE)
#endif // !L2_CACHE_SETS
#elif defined(L2_CACHE_SIZE) && defined(L2_CACHE_SETS)
#ifndef L2_ASSOC
#define L2_ASSOC        L2_CACHE_SIZE/(L2_CACHE_SETS*CACHE_LINE)
#endif // !L2_ASSOC
#endif
#define L2_TOTAL_WAYS   L2_CACHE_SETS*L2_ASSOC
#define L2_STRIDE       L2_CACHE_SETS*CACHE_LINE


#define SIZE_GT_LLC     8*L2_CACHE_SIZE

#define AMP_BASE        // TODO: fill with your AMP_BASE

#define CSR_K1_MSETUP   0x7C0  // example of platform-specific CSR

// helper to read the hart ID (M-Mode CSR)
static inline uint64_t current_hart_id(void) {
    uint64_t hart_id;
    __asm__ __volatile__("csrr %0, mhartid" : "=r"(hart_id));
    return hart_id;
}

// helper to read the machine status register (M-Mode CSR)
static inline uint64_t read_mstatus(void) {
    uint64_t mstatus;
    __asm__ __volatile__("csrr %0, mstatus" : "=r"(mstatus));
    return mstatus;
}

// use `csrs` to `csr_set`
static inline uint64_t read_msetup(void)
{
    uint64_t x;
    asm volatile("csrr %0, %1" : "=r"(x) : "i" (CSR_K1_MSETUP) : "memory");
    return x;
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
static inline void fence_iorwiorw()
{
    asm __volatile__(
        "fence iorw, iorw \n"
    );
}

__attribute__((always_inline))
static inline void read(uint8_t* char_ptr)
{
    asm __volatile__ (
        "ld t0,   0(%[c])    \n"
        :
        : [c] "r" (char_ptr)
        : "t0", "memory"
    );

}

__attribute__((always_inline))
static inline void flush(uint8_t* char_ptr)
{
//    asm __volatile__(
//        ... // TODO: fill yourself
//        :
//        : [c] "r" (char_ptr)
//        : "memory"
//    );
}

/*
 * additional custom functions
 */

// example: K1 custom function to test
static inline void l1_clear_inval_flush(void *p)
{
    asm volatile ("cbo.flush" " 0(%0)" : : "rK"(p): "memory");
}


/* the payload */
__attribute__((section(".text.main")))
void main(void) {
    uint64_t id = curret_hart_id();

    /* something like this may be necessary depending on your firmware
     * modification */
    uint64_t id_offset = 1;
    if (id == 5) id_offset = 2;
    if (id == 1) id_offset = 3;
    if (id == 2) id_offset = 4;

    // calculate unique workspace address based on core ID
    volatile uint32_t* workspace = (volatile uint32_t *) (AMP_BASE
                                            + 0x700000
                                            + ((id_offset - 1) * 0x100000));
    // set addresses for test variables in workspace
    volatile uint64_t* t1 = (uint64_t*) ((uint8_t*) workspace+0x30);
    volatile uint64_t* t2 = (uint64_t*) ((uint8_t*) workspace+0x38);
    volatile uint64_t* debug = (uint64_t*) ((uint8_t*) workspace+0x40);
    volatile uint64_t* timing = (uint64_t*) ((uint8_t*) workspace+0x80);
    volatile uint8_t* test_addr = (uint8_t*) workspace + 0x1000;
    // set a probe to test evicted cache line access latency; we try access to
    // cycle and time counters, and their accuracy
    volatile uint8_t* probe = (uint8_t*) test_addr + L2_STRIDE;


    // read M-Mode specific registers:
    // if we were in S-Mode (e.g., Linux, with limited privilege mode),
    // the processor would trap and die right here and flag should not reach
    // its second value assignment
    // (flag is located in AMP buffer, not in workspace)
    volatile uint64_t* flag = (volatile uint64_t*) (AMP_BASE + 0x1000e8);
    *flag = 0x01020304;
    uint64_t mstat = read_mstatus();
    *flag = 0x05060708;

    // RISC-V Privilege Levels:
    // 0 = User (U-Mode)
    // 1 = Supervisor (S-Mode)
    // 3 = Machine (M-Mode)

    asm volatile("csrr %0, mcountinhibit" : "=r"(debug[0]));
    asm volatile("csrr %0, mcycle" : "=r"(debug[1]));
    debug[2] = rdcycle();
    debug[3] = read_msetup();

    /* before using the cache, ensure you perform some of the required warm or
     * cold boot initialization operations; below are given two operations
     * required for Banana Pi BPI-F3
     * 

    // enable L2 cache snooping
    asm volatile(
    "csrs 0x7F0, %0"
    :
    : "r"(1UL << (id % 4))
    : "memory");

    // enable L1 caches
    asm volatile(
    "csrs 0x7C0, %0"
    :
    : "r"(0x10073)
    : "memory");
    debug[3] = read_msetup();
     */


    for (uint64_t i = 0; i < 16; i++)
    {
        fence_iorwiorw();
        *(volatile uint8_t *) test_addr;
        fence_iorwiorw();
        *t1 = rdcycle();
        fence_iorwiorw();
        *(volatile uint8_t *) test_addr;
        fence_iorwiorw();
        *t2 = rdcycle();
        fence_iorwiorw();

        *(timing+i) = *t2-*t1;
        *flag = i;
    }
    for (uint64_t i = 16; i < 32; i++)
    {
        fence_iorwiorw();
        *(volatile uint8_t *) test_addr;
        fence_iorwiorw();
        /* test your flush function here */
        //flush(test_addr);
        /* or */
        //l1_clear_inval_flush(test_addr);
        /* or just attempt eviction on L2 using the most simple strategy */
        for (uint64_t j = 0; j < 2*L2_ASSOC; j++)
        {
            *(volatile uint8_t *) (probe+j*L2_STRIDE);
        }
        fence_iorwiorw();
        *t1 = rdcycle();
        fence_iorwiorw();
        *(volatile uint8_t *) test_addr;
        fence_iorwiorw();
        *t2 = rdcycle();
        fence_iorwiorw();
        *(timing+i) = *t2-*t1;

        *flag = i;
    }

    // write magics to physical memory indicating we reached end of execution
    workspace[0] = 0xDEADBEEF;           // Magic header to prove execution
    workspace[1] = (uint32_t)id;         // Core ID that executed this
    workspace[2] = (uint32_t)mstat;      // Lower 32 bits of mstatus register
    workspace[3] = 0xCAFEBABE;           // Magic footer


    return;
}
