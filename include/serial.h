#pragma once
#include <stdint.h>

class Console {
public:
    Console(){
        init();
    }

    void print(const char* s) {
        while (*s) print_char(*s++);
    }

    void print(uint64_t val) {
        if (val == 0) {
            print_char('0');
            return;
        }
        char buf[20];
        int i = 0;
        while (val > 0) {
            buf[i++] = '0' + (val % 10);
            val /= 10;
        }
        while (i-- > 0) print_char(buf[i]);
    }

private:

    void init(){
        outb(0x3F8 + 1, 0x00);
        outb(0x3F8 + 3, 0x80);
        outb(0x3F8 + 0, 0x01);
        outb(0x3F8 + 1, 0x00);
        outb(0x3F8 + 3, 0x03);
        outb(0x3F8 + 2, 0xC7);
    }

    void print_char(char c) {
        while (!(inb(0x3F8 + 5) & 0x20));
        outb(0x3F8, c);
    }

    uint8_t inb(uint16_t port) {
        uint8_t val;
        __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
        return val;
    }

    void outb(uint16_t port, uint8_t val) {
        __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
    }

    
};