
#pragma once
#include <stdint.h>
// this holds the address to the GDT
// this is loaded using the LGDT instr, whose arg is a pointer to the GDTR
struct __attribute__((packed)) GDTR {
    uint16_t size; // size of GDT
    uint64_t offset; // linear address of GDT
};

struct __attribute__((packed)) SegmentDescriptor {
    uint16_t limit_low;   // Limit bits 15:0
    uint16_t base_low;    // Base bits 15:0
    uint8_t  base_mid;    // Base bits 23:16
    uint8_t  access;      // Access byte
    uint8_t  limit_flags; // Limit bits 19:16 (low nibble) + Flags (high nibble)
    uint8_t  base_high;   // Base bits 31:24
};

struct __attribute__((packed)) SystemSegmentDescriptor {
    uint16_t limit_low;   // Limit bits 15:0
    uint16_t base_low;    // Base bits 15:0
    uint8_t  base_mid;    // Base bits 23:16
    uint8_t  access;      // Access byte
    uint8_t  limit_flags; // Limit bits 19:16 (low nibble) + Flags (high nibble)
    uint8_t  base_high;   // Base bits 31:24
    uint32_t base_upper;  // Base bits 63:32
    uint32_t reserved;
};

struct __attribute__((packed)) GDT {
    SegmentDescriptor null_descriptor;
    SegmentDescriptor kernel_code_descriptor;
    SegmentDescriptor kernel_data_descriptor;
    SegmentDescriptor user_code_descriptor;
    SegmentDescriptor user_data_descriptor;
    SystemSegmentDescriptor tss_descriptor;
};