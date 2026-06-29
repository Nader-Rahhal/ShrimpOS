#include "util.h"

uint32_t strlen(const char* s) {
    uint32_t n = 0;
    while (s[n]) n++;
    return n;
}

void u64_to_str(uint64_t n, char* buf) {
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[20];
    int i = 0;
    while (n) { tmp[i++] = '0' + (n % 10); n /= 10; }
    for (int j = 0; j < i; j++) buf[j] = tmp[i - 1 - j];
    buf[i] = '\0';
}

void u64_to_hex(uint64_t val, char* buf) {
    const char* digits = "0123456789ABCDEF";
    for (int i = 15; i >= 0; i--) {
        buf[i] = digits[val & 0xF];
        val >>= 4;
    }
    buf[16] = '\0';
}

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void io_wait() {
    outb(0x80, 0);
}

void serial_putc(char c) {
    while (!(inb(0x3F8 + 5) & 0x20));
    outb(0x3F8, c);
}

void serial_puts(const char* s) {
    while (*s) serial_putc(*s++);
}

void serial_puthex(uint64_t val) {
    serial_puts("0x");
    char buf[17];
    u64_to_hex(val, buf);
    serial_puts(buf);
}

void serial_putdec(uint64_t val) {
    char buf[21];
    u64_to_str(val, buf);
    serial_puts(buf);
}
