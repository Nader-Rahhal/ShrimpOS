#pragma once
#include "util.h"

#define KPANIC(msg)                          \
    do {                                     \
        serial_puts("\r\nKERNEL PANIC: ");   \
        serial_puts(msg);                    \
        serial_puts("\r\n");                 \
        for (;;) __asm__ volatile ("hlt");   \
    } while (0)
