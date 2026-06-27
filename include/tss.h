#pragma once

struct __attribute__((packed)) TSS {
    uint32_t reserved0;       // 0x00
    uint64_t rsp0;            // 0x04
    uint64_t rsp1;            // 0x0C
    uint64_t rsp2;            // 0x14
    uint32_t reserved1;       // 0x1C
    uint32_t reserved2;       // 0x20
    uint64_t ist1;            // 0x24
    uint64_t ist2;            // 0x2C
    uint64_t ist3;            // 0x34
    uint64_t ist4;            // 0x3C
    uint64_t ist5;            // 0x44
    uint64_t ist6;            // 0x4C
    uint64_t ist7;            // 0x54
    uint32_t reserved3;       // 0x5C
    uint32_t reserved4;       // 0x60
    uint16_t reserved5;       // 0x64
    uint16_t iopb_offset;     // 0x66
};