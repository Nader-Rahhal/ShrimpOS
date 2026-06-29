#pragma once
#include "util.h"

inline void disable_pic() {
    // reinitialize PIC so its vectors don't overlap CPU exceptions,
    // then mask every line so no IRQs get through
    outb(0x20, 0x11);  io_wait();  // ICW1: start init
    outb(0xA0, 0x11);  io_wait();
    outb(0x21, 0x20);  io_wait();  // ICW2: master base vector 0x20
    outb(0xA1, 0x28);  io_wait();  // ICW2: slave base vector 0x28
    outb(0x21, 0x04);  io_wait();  // ICW3: slave on IRQ2
    outb(0xA1, 0x02);  io_wait();  // ICW3: slave cascade identity
    outb(0x21, 0x01);  io_wait();  // ICW4: 8086 mode
    outb(0xA1, 0x01);  io_wait();
    outb(0x21, 0xFF);              // mask all master IRQs
    outb(0xA1, 0xFF);              // mask all slave IRQs
}
