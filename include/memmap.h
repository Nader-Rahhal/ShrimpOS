#pragma once
#include <stdint.h>

struct MemDescriptor {
    uint32_t type;
    void* physAddr;
    void* virtAddr;
    uint64_t numPages;
    uint64_t attributes;
};

class MemMap {
public:
    MemMap(void* map, uint64_t mapSize, uint64_t mapDescSize)
    : map(map), mapSize(mapSize), mapDescSize(mapDescSize) {}

    uint64_t usable_memory_mb() const {
        uint64_t pages = 0;
        uint8_t* entry = (uint8_t*)map;
        for (uint64_t i = 0; i < num_descriptors(); i++, entry += mapDescSize){
            MemDescriptor* desc = (MemDescriptor*)entry;
            if (desc->type == 7){
                pages += desc->numPages;
            }
        }
        return pages * 4096 / (1024 * 1024);
    }

    uint64_t usable_memory_bytes() const {
        uint64_t pages = 0;
        uint8_t* entry = (uint8_t*)map;
        for (uint64_t i = 0; i < num_descriptors(); i++, entry += mapDescSize) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if (desc->type == 7)
                pages += desc->numPages;
        }
        return pages * 4096;
    }

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
