#include <stdint.h>

uint32_t strlen(const char* s) {
    uint32_t n = 0;
    while (s[n]) n++;
    return n;
}