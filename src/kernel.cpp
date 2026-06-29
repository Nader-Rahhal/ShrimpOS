#include "color.h"
#include "framebuffer.h"
#include "mmap.h"
#include "acpi.h"
#include "apic.h"
#include "util.h"
#include "pmm.h"
#include "gdt.h"
#include "idt.h"
#include "panic.h"
#include "pic.h"
#include "exception.h"

extern "C" { extern uint8_t _binary_font_psf_start[]; }


static const char hex_chars[] = "0123456789ABCDEF";

static void serial_hex(uint8_t val) {
    char buf[3] = { hex_chars[val >> 4], hex_chars[val & 0xF], 0 };
    serial_puts(buf);
}

extern "C" void interrupt_handler(InterruptFrame* frame) {
    if (frame->vector < 32) {
        exception_handler(frame);
        return;
    }

    if (frame->vector == 255) {
        // LAPIC spurious interrupt — do NOT send EOI
        return;
    }

    if (frame->vector == 33) {
        uint8_t scancode = inb(0x60);
        serial_puts("key: 0x");
        serial_hex(scancode);
        serial_puts("\r\n");
    }

    lapic_eoi();
}


extern "C" __attribute__((section(".text.kmain"))) void kmain(FrameBuffer* fb, MMap* mm, MADT* madt)
{ 

    Console console;
    PMM pmm(mm);

    disable_pic();

    fb->set_background(Color::WHITE);
    fb->set_font(_binary_font_psf_start);

    const char* msg = "Hello from ShrimpOS";
    uint32_t start_x = (fb->get_width() / 8 - strlen(msg)) / 2;
    uint32_t start_y = (fb->get_height() / 16) / 2;
    fb->draw_string(msg, start_x, start_y, Color::BLACK);

    gdt_init();
    console.print("GDT loaded\r\n");

    idt_init();
    console.print("IDT loaded\r\n");

    apic_init(madt);
    console.print("APIC initialized\r\n");

    enable_interrupts();

    console.print("Interrupts enabled\r\n");


    for (;;) __asm__ volatile ("hlt");
}