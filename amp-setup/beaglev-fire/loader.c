#include <stdint.h>

/* macros from `opensbi/include/sbi/riscv_encoding.h` */
#define CSR_FCSR                        0x003
#define CSR_MSTATUS                     0x300
#define CSR_MIE                         0x304
#define CSR_MTVEC                       0x305
#define CSR_MEPC                        0x341
#define CSR_MCAUSE                      0x342
#define CSR_MTVAL                       0x343
#define MSTATUS_FS                      0x00006000UL

/* macros from `opensbi/include/sbi/riscv_asm.h`*/
//#define __ASM_STR(x)    x
#define __ASM_STR(x)    #x

#define csr_set(csr, val)                                          \
        ({                                                         \
                register unsigned long __v = (unsigned long)(val); \
                __asm__ __volatile__("csrs " __ASM_STR(csr) ", %0" \
                                     :                             \
                                     : "rK"(__v)                   \
                                     : "memory");                  \
        })

#define csr_write(csr, val)                                        \
        ({                                                         \
                register unsigned long __v = (unsigned long)(val); \
                __asm__ __volatile__("csrw " __ASM_STR(csr) ", %0" \
                                     :                             \
                                     : "rK"(__v)                   \
                                     : "memory");                  \
        })

#define csr_read(csr)                                           \
        ({                                                      \
                register unsigned long __v;                     \
                __asm__ __volatile__("csrr %0, " __ASM_STR(csr) \
                                     : "=r"(__v)                \
                                     :                          \
                                     : "memory");               \
                __v;                                            \
        })


/* current loader implementation */
#define AMP_STATE_IDLE 0xF
#define AMP_STATE_RUN  0x1
#define AMP_STATE_DONE 0x2

typedef void (*executable_t)(void);

// Inline assembly to extract the executing core's hardware ID
static inline uint64_t get_hart_id(void) {
    uint64_t hart_id;
    __asm__ __volatile__("csrr %0, mhartid" : "=r"(hart_id));
    return hart_id;
}

static void dbg_mmode_trap(void)
{
    volatile uint64_t *dbg =
    (volatile uint64_t *)0x821000E0;

    dbg[0] = 0xDEC0AD0B;  // 0x0badc0de

    dbg[1] = csr_read(CSR_MCAUSE);
    dbg[2] = csr_read(CSR_MEPC);
    dbg[3] = csr_read(CSR_MTVAL);

    while (1)
;
}

__attribute__((section(".text.main")))
__attribute__((used))
void loader_main(void) {
    uint64_t hartid = get_hart_id();

    // Safety check: Only Harts 1 and 2 should execute this logic
    if (hartid != 2 && hartid != 3) {
        while (1) { __asm__ __volatile__("wfi"); }
    }
    uint8_t hart_offset = 1;
    if (hartid == 3) hart_offset = 2;

    volatile uint8_t *checkbox = (volatile uint8_t *)0x82100080;
    checkbox[hart_offset-1] = 0x01;  // track step


    // M-mode trap handler
    csr_write(CSR_MTVEC, (unsigned long)dbg_mmode_trap);

    csr_set(CSR_MSTATUS, MSTATUS_FS);
    csr_write(CSR_FCSR, 0);

    checkbox[hartid-1] = 0x12;  // track step

    // disable interrupts globally for this hart
    csr_write(CSR_MIE, 0);  // TODO: remove?

    checkbox[hartid-1] = 0x23;  // track step


    /* Dynamically calculate state address:
     * ID 1 -> 0x82100001
     * ID 2 -> 0x82100002
     */
    volatile uint8_t *amp_state = (volatile uint8_t *)(0x82100000+hart_offset);

    /* 0x100000 is 1 Megabyte in Hex.
     * Core 1 -> 0x82300000
     * Core 2 -> 0x82400000
     */
    executable_t payload = (executable_t)(0x82300000
                                          + ((hart_offset - 1) * 0x100000));

    checkbox[hartid-1] = 0x34;  // track step

    // Initialize this core's state to IDLE
    *amp_state = AMP_STATE_IDLE;

    while (1) {
        if (*amp_state == AMP_STATE_RUN) {
            // Flush instruction cache before jumping to new code
            __asm__ __volatile__("fence.i");

            checkbox[hartid-1] = 0x45;  // track step

            // Execute the payload
            payload();

            checkbox[hartid-1] = 0x56;  // track step

            // Mark this specific core as DONE
            *amp_state = AMP_STATE_DONE;
        }

        // Prevent core from melting the bus while polling
        __asm__ __volatile__("nop");
    }
}
