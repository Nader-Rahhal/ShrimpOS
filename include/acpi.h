#pragma once
#include <stdint.h>

struct __attribute__((packed)) ACPISDTHeader {
    char     Signature[4];
    uint32_t Length;
    uint8_t  Revision;
    uint8_t  Checksum;
    char     OEMID[6];
    char     OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};

struct __attribute__((packed)) MADT {
    ACPISDTHeader Header;
    uint32_t      LocalApicAddress;
    uint32_t      Flags;
};

struct ICHeader {
    uint8_t Type;
    uint8_t Length;
};
