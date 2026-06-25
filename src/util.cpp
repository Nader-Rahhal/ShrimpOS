#include <stdint.h>

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