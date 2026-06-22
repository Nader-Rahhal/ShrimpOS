#include "framebuffer.h"

#define COM1 0x3F8

static inline void outb(unsigned short port, unsigned char val)
{
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static void serial_init(void)
{
    outb(COM1 + 1, 0x00); /* disable interrupts */
    outb(COM1 + 3, 0x80); /* enable DLAB */
    outb(COM1 + 0, 0x03); /* 38400 baud */
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03); /* 8N1 */
    outb(COM1 + 2, 0xC7); /* enable FIFO */
}

static void serial_putc(char c)
{
    while (!(inb(COM1 + 5) & 0x20))
        ;
    outb(COM1, c);
}

static void serial_puts(const char *s)
{
    for (; *s; s++) {
        if (*s == '\n')
            serial_putc('\r');
        serial_putc(*s);
    }
}

__attribute__((section(".text.kmain")))
void kmain(FrameBuffer* fb)
{
    serial_init();
    serial_puts("KERNEL OK\n");

    uint32_t blue = 0x000000FF; /* BGRX: B=0xFF at byte 0 */

    for (uint32_t y = 0; y < fb->height; y++) {
        for (uint32_t x = 0; x < fb->width; x++) {
            uint32_t* pixel = (uint32_t*)(fb->base + y * fb->pitch + x * 4);
            *pixel = blue;
        }
    }

    for (;;)
        __asm__ volatile ("hlt");
}
