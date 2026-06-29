#pragma once
#include <stdint.h>
#include "gdt.h"

extern void* isr_stub_table[];

struct __attribute__((packed)) IDTEntry {
    uint16_t offset_low;   // bits 0-15 of handler function address
    uint16_t selector;     // code segment selector in GDT or LDT
    uint8_t ist;           // bits 0-2 hold Interrupt Stack Table offset, rest of bits zero.
    uint8_t attributes;     // type and attributes
    uint16_t offset_mid;   // bits 16-31 of handler function address
    uint32_t offset_high;  // bits 32-63 of handler function address
    uint32_t zero;         // reserved, set to zero
};

struct __attribute__((aligned(16))) IDT {
    IDTEntry entries[256];
};

struct __attribute__((packed)) IDTR {
    uint16_t limit;  // size of the IDT in bytes - 1
    uint64_t base;   // address of the first element in the IDT
};

struct __attribute__((packed)) InterruptFrame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector;
    uint64_t error_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

void idt_set_descriptor(struct IDT* idt, uint8_t vector, void* isr, uint8_t flags){
    IDTEntry* descriptor = &idt->entries[vector];
    descriptor->offset_low        = (uint64_t)isr & 0xFFFF;
    descriptor->selector      = GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->offset_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->offset_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->zero       = 0;
}

inline void idt_init() {
    static struct IDT idt;
    static struct IDTR idtr;

    idtr.base  = (uint64_t)&idt;
    idtr.limit = (uint16_t)sizeof(IDT) - 1;

    for (int vector = 0; vector < 256; vector++) {
        idt_set_descriptor(&idt, vector, isr_stub_table[vector], 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

inline void enable_interrupts() {
    __asm__ volatile ("sti");
}
