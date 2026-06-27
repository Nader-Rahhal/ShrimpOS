#pragma once
#include <stdint.h>
#include "mmap.h"

enum class PMM_STATUS {
    ALLOC_SUCCESS,
    ALLOC_FAIL,
    DEALLOC_SUCCESS,
    DEALLOC_FAIL
};

class PMM {
public:
    PMM(MMap* mmap) : mmap(mmap) {
        total_pages = mmap->usable_memory_pages();
        uint64_t bitmap_bytes = (total_pages + 7) / 8;

        uint8_t* entry = (uint8_t*)mmap->get_map();
        for (uint64_t i = 0; i < mmap->num_descriptors(); i++, entry += mmap->get_desc_size()) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if ((MemType)desc->type == MemType::CONVENTIONAL_MEMORY && desc->numPages * 4096 >= bitmap_bytes) {
                bitmap = (uint8_t*)desc->physAddr;
                break;
            }
        }

        for (uint64_t i = 0; i < bitmap_bytes; i++)
            bitmap[i] = 0xFF;

        uint64_t logical = 0;
        entry = (uint8_t*)mmap->get_map();
        for (uint64_t i = 0; i < mmap->num_descriptors(); i++, entry += mmap->get_desc_size()) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if ((MemType)desc->type != MemType::CONVENTIONAL_MEMORY) continue;
            for (uint64_t p = 0; p < desc->numPages; p++, logical++)
                clear_bit(logical);
        }

        uint64_t bitmap_addr = (uint64_t)bitmap;
        uint64_t bitmap_page_count = (bitmap_bytes + 4095) / 4096;
        logical = 0;
        entry = (uint8_t*)mmap->get_map();
        for (uint64_t i = 0; i < mmap->num_descriptors(); i++, entry += mmap->get_desc_size()) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if ((MemType)desc->type != MemType::CONVENTIONAL_MEMORY) continue;
            uint64_t base = (uint64_t)desc->physAddr;
            uint64_t size = desc->numPages * 4096;
            if (bitmap_addr >= base && bitmap_addr < base + size) {
                uint64_t start = logical + (bitmap_addr - base) / 4096;
                for (uint64_t p = 0; p < bitmap_page_count; p++)
                    set_bit(start + p);
                break;
            }
            logical += desc->numPages;
        }
    }

    PMM_STATUS alloc_pages(uint64_t count, uint64_t& addr_out) {
        uint64_t logical = 0;
        uint8_t* entry = (uint8_t*)mmap->get_map();
        for (uint64_t i = 0; i < mmap->num_descriptors(); i++, entry += mmap->get_desc_size()) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if ((MemType)desc->type != MemType::CONVENTIONAL_MEMORY) continue;

            for (uint64_t p = 0; p < desc->numPages; p++, logical++) {
                if (p + count > desc->numPages) break;

                bool found = true;
                for (uint64_t k = 0; k < count; k++) {
                    if (test_bit(logical + k)) { found = false; break; }
                }

                if (found) {
                    for (uint64_t k = 0; k < count; k++)
                        set_bit(logical + k);
                    addr_out = (uint64_t)desc->physAddr + p * 4096;
                    return PMM_STATUS::ALLOC_SUCCESS;
                }
            }
        }
        addr_out = 0;
        return PMM_STATUS::ALLOC_FAIL;
    }

    PMM_STATUS alloc_page(uint64_t& addr_out) {
        uint64_t logical = 0;
        uint8_t* entry = (uint8_t*)mmap->get_map();
        for (uint64_t i = 0; i < mmap->num_descriptors(); i++, entry += mmap->get_desc_size()) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if ((MemType)desc->type != MemType::CONVENTIONAL_MEMORY) continue;
            for (uint64_t p = 0; p < desc->numPages; p++, logical++) {
                if (!test_bit(logical)) {
                    set_bit(logical);
                    addr_out = (uint64_t)desc->physAddr + p * 4096;
                    return PMM_STATUS::DEALLOC_SUCCESS;
                }
            }
        }
        addr_out = 0;
        return PMM_STATUS::DEALLOC_FAIL;
    }

    PMM_STATUS dealloc_pages(uint64_t address, uint64_t count) {
        for (uint64_t i = 0; i < count; i++)
            dealloc_page(address + i * 4096);
        return PMM_STATUS::DEALLOC_SUCCESS;
    }

    PMM_STATUS dealloc_page(uint64_t address) {
        uint64_t logical = 0;
        uint8_t* entry = (uint8_t*)mmap->get_map();
        for (uint64_t i = 0; i < mmap->num_descriptors(); i++, entry += mmap->get_desc_size()) {
            MemDescriptor* desc = (MemDescriptor*)entry;
            if ((MemType)desc->type != MemType::CONVENTIONAL_MEMORY) continue;
            uint64_t base = (uint64_t)desc->physAddr;
            uint64_t size = desc->numPages * 4096;
            if (address >= base && address < base + size) {
                logical += (address - base) / 4096;
                clear_bit(logical);
                return PMM_STATUS::DEALLOC_SUCCESS;
            }
            logical += desc->numPages;
        }
        return PMM_STATUS::DEALLOC_FAIL;
    }

    uint8_t* get_bitmap() const { return bitmap; }
    uint64_t get_total_pages() const { return total_pages; }

private:
    MMap*  mmap;
    uint8_t* bitmap      = nullptr;
    uint64_t total_pages = 0;

    void set_bit(uint64_t idx)    { bitmap[idx / 8] |=  (1 << (idx % 8)); }
    void clear_bit(uint64_t idx)  { bitmap[idx / 8] &= ~(1 << (idx % 8)); }
    bool test_bit(uint64_t idx)   { return bitmap[idx / 8] & (1 << (idx % 8)); }
};