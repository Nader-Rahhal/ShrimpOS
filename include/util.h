#pragma once
#include <stdint.h>

uint32_t strlen(const char* s);
void     u64_to_str(uint64_t n, char* buf);
void     u64_to_hex(uint64_t val, char* buf);

void    outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void    io_wait();

void serial_putc(char c);
void serial_puts(const char* s);
void serial_puthex(uint64_t val);
void serial_putdec(uint64_t val);
