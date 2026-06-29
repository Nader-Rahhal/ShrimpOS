#pragma once
#include <stdint.h>
#include "acpi.h"

struct __attribute__((packed)) IOAPICRecord {
    uint8_t  type;      // 1
    uint8_t  length;    // 12
    uint8_t  id;
    uint8_t  reserved;
    uint32_t address;   // MMIO base
    uint32_t gsi_base;  // first GSI this I/O APIC handles
};

struct __attribute__((packed)) ISORecord {
    uint8_t  type;   // 2
    uint8_t  length; // 10
    uint8_t  bus;    // 0 = ISA
    uint8_t  irq;    // ISA IRQ number
    uint32_t gsi;    // GSI it maps to
    uint16_t flags;  // bits[1:0]=polarity, bits[3:2]=trigger mode
};

#define LAPIC_ID   0x020
#define LAPIC_TPR  0x080
#define LAPIC_EOI  0x0B0
#define LAPIC_SVR  0x0F0

static volatile uint32_t* lapic_ptr;

static inline uint32_t lapic_read(uint32_t reg) {
    return lapic_ptr[reg >> 2];
}

static inline void lapic_write(uint32_t reg, uint32_t val) {
    lapic_ptr[reg >> 2] = val;
}

inline void lapic_eoi() {
    lapic_write(LAPIC_EOI, 0);
}

static inline uint32_t ioapic_read(volatile uint32_t* base, uint8_t reg) {
    base[0] = reg;
    return base[4]; // IOWIN is at offset 0x10 = 4 uint32_ts away
}

static inline void ioapic_write(volatile uint32_t* base, uint8_t reg, uint32_t val) {
    base[0] = reg;
    base[4] = val;
}

static void ioapic_redirect(volatile uint32_t* ioapic, uint32_t slot, uint8_t vector,
                            uint8_t dest, bool active_low, bool level) {
    uint32_t lo = (uint32_t)vector
                | (active_low ? (1u << 13) : 0u)
                | (level      ? (1u << 15) : 0u);
    ioapic_write(ioapic, 0x10 + 2 * slot, lo);
    ioapic_write(ioapic, 0x11 + 2 * slot, (uint32_t)dest << 24);
}

inline void apic_init(MADT* madt) {
    // start with ISA defaults: identity-mapped GSI, active-high, edge-triggered
    uint32_t irq_to_gsi[16];
    bool     active_low[16];
    bool     level_trig[16];
    for (int i = 0; i < 16; i++) {
        irq_to_gsi[i] = i;
        active_low[i] = false;
        level_trig[i] = false;
    }

    volatile uint32_t* ioapic = nullptr;
    uint32_t           gsi_base = 0;

    uint8_t* ptr = (uint8_t*)madt + sizeof(MADT);
    uint8_t* end = (uint8_t*)madt + madt->Header.Length;

    while (ptr < end) {
        uint8_t type = ptr[0], len = ptr[1];
        if (type == 1) {
            auto* r  = (IOAPICRecord*)ptr;
            ioapic   = (volatile uint32_t*)(uintptr_t)r->address;
            gsi_base = r->gsi_base;
        } else if (type == 2) {
            auto* r = (ISORecord*)ptr;
            if (r->bus == 0 && r->irq < 16) {
                irq_to_gsi[r->irq] = r->gsi;
                active_low[r->irq] = (r->flags & 0x3) == 0x3;
                level_trig[r->irq] = ((r->flags >> 2) & 0x3) == 0x3;
            }
        }
        ptr += len;
    }

    lapic_ptr = (volatile uint32_t*)(uintptr_t)madt->LocalApicAddress;
    lapic_write(LAPIC_TPR, 0);      // accept all interrupt priorities
    lapic_write(LAPIC_SVR, 0x1FF); // enable LAPIC; spurious → vector 255

    uint8_t lapic_id = (lapic_read(LAPIC_ID) >> 24) & 0xFF;

    // route IRQ 1 (PS/2 keyboard) → vector 33
    uint32_t gsi  = irq_to_gsi[1];
    uint32_t slot = gsi - gsi_base;
    ioapic_redirect(ioapic, slot, 33, lapic_id, active_low[1], level_trig[1]);
}
