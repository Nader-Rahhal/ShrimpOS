#include "color.h"
#include "framebuffer.h"
#include "util.cpp"

// we are using PSF 1 so each character is 16 rows, each row being a byte
extern "C" { extern uint8_t _binary_font_psf_start[]; }

extern "C" __attribute__((section(".text.kmain"))) void kmain(FrameBuffer* fb)
{ 

    fb->set_background(Color::WHITE);

    const char* msg = "Hello from ShrimpOS";
    uint32_t start_x = (fb->get_width() / 8 - strlen(msg)) / 2;
    uint32_t start_y = (fb->get_height() / 16) / 2;

    fb->set_font(_binary_font_psf_start);
    fb->draw_string(msg, start_x, start_y, Color::BLACK);

    fb->draw_line(100, 20, fb->get_width(), 10, Color::BLACK);

    for (;;)
        __asm__ volatile ("hlt");
}
