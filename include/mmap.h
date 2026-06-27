#pragma once
#include <stdint.h>

enum class MemType {
    RESERVED = 0,
    LOADER_CODE = 1,
    LOADER_DATA = 2,
    BOOT_SERVICES_CODE = 3,
    BOOT_SERVICES_DATA = 4,
    RUNTIME_SERVICES_CODE = 5,
    RUNTIME_SERVICES_DATA = 6,
    CONVENTIONAL_MEMORY = 7,
    UNUSABLE_MEMORY = 8,
    ACPI_RECLAIM_MEMORY = 9,
    ACPI_MEMORY_NVS = 10,
    MEMORY_MAPPED_IO = 11,
    MEMORY_MAPPED_IO_PORT_SPACE = 12,
    PAL_CODE = 13
};

struct MemDescriptor {
    uint32_t type;
    void* physAddr;
    void* virtAddr;
    uint64_t numPages;
    uint64_t attributes;
};

class MMap {
public:
    MMap(void* map, uint64_t mapSize, uint64_t mapDescSize)
    : map(map), mapSize(mapSize), mapDescSize(mapDescSize) {}

    uint64_t usable_memory_pages() const {
        uint64_t pages = 0;
        uint8_t* entry = (uint8_t*)map;
        for (uint64_t i = 0; i < num_descriptors(); i++, entry += mapDescSize) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if (desc->type == 7)
                pages += desc->numPages;
        }
        return pages;
    }

    void*    get_map()       const { return map; }
    uint64_t get_desc_size() const { return mapDescSize; }
    uint64_t num_descriptors() const { return mapSize / mapDescSize; }


private:
    void*    map;
    uint64_t mapSize;
    uint64_t mapDescSize;

};
