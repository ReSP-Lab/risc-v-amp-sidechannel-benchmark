#ifndef __AMP_SETUP_H
#define __AMP_SETUP_H

#include "board.h"

#define AMP_CHAR_BYTES          1ULL
#define AMP_U32_BYTES           4ULL
#define AMP_U64_BYTES           8ULL
#define AMP_PTR_BYTES           8ULL
#define AMP_ALIGN8(n)           (((n) + 7ULL) & ~7ULL)

#if (AMP_BASE % AMP_U64_BYTES) != 0
#error "AMP output base must be 8-byte aligned"
#endif

#define LOADER_BASE             (AMP_BASE + 0x00)
#define LOADER_SIZE             0x00100000  // 1MB

#define AMP_BUFFER_BASE         (AMP_BASE + 0x00100000)
#define AMP_BUFFER_SIZE         0x00200000  // 2MB
//#define AMP_BUFFER_SIZE         0x00020000  // 128kB
//#define OUTPUT_DATA_BASE        (AMP_BASE + 0x00120000)
//#define OUTPUT_DATA_SIZE        0x001e0000  // 1.875MB
#define OUTPUT_DATA_BASE        (AMP_BASE + 0x02000000)
#define OUTPUT_DATA_SIZE        0x04000000  // 64MB

#define PAYLOAD_STRIDE          0x00100000  // 1MB stride (and size)
#define HART1_PAYLOAD_BASE      (AMP_BASE + 0x00300000)
#define HART2_PAYLOAD_BASE      (AMP_BASE + 0x00400000)
#define HART3_PAYLOAD_BASE      (AMP_BASE + 0x00500000)
#define HART4_PAYLOAD_BASE      (AMP_BASE + 0x00600000)

#define POINTERS_BASE           (AMP_BASE + 0x00700000)
#define POINTERS_SIZE           0x00900000  // 9MB

#define PROBE_REGION_BASE       (AMP_BASE + 0x01000000)
#define PROBE_REGION_SIZE       0x01000000  // 16MB


//#define TOTAL_RESERVED_SIZE     0x02000000  // 32MB
#define TOTAL_RESERVED_SIZE     0x06000000  // 96MB


#endif // !__AMP_SETUP_H
