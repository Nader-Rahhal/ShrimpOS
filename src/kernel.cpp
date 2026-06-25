#include "color.h"
#include "framebuffer.h"
#include "memmap.h"
#include "util.cpp"
#include "pmm.h"

extern "C" { extern uint8_t _binary_font_psf_start[]; }

extern "C" __attribute__((section(".text.kmain"))) void kmain(FrameBuffer* fb, MemMap* mm)
{ 
    fb->set_background(Color::WHITE);
    fb->set_font(_binary_font_psf_start);

    const char* msg = "Hello from ShrimpOS";
    uint32_t start_x = (fb->get_width() / 8 - strlen(msg)) / 2;
    uint32_t start_y = (fb->get_height() / 16) / 2;
    fb->draw_string(msg, start_x, start_y, Color::BLACK);

    PMM pmm(mm);
    uint32_t row = start_y + 2;
    char buf[24];

    fb->draw_string("bitmap @ 0x", 0, row, Color::BLACK);
    u64_to_hex((uint64_t)pmm.get_bitmap(), buf);
    fb->draw_string(buf, 11, row++, Color::BLACK);

    uint64_t bitmap_pages = (mm->usable_memory_pages() + 7) / 8;
    bitmap_pages = (bitmap_pages + 4095) / 4096;
    fb->draw_string("bitmap pages: ", 0, row, Color::BLACK);
    u64_to_str(bitmap_pages, buf);
    fb->draw_string(buf, 14, row++, Color::BLACK);

    fb->draw_string("total pages:  ", 0, row, Color::BLACK);
    u64_to_str(mm->usable_memory_pages(), buf);
    fb->draw_string(buf, 14, row++, Color::BLACK);
    row++;

    uint64_t a, b, c, d;

    pmm.get_page(a);
    pmm.get_page(b);
    pmm.get_page(c);

    fb->draw_string("alloc a: 0x", 0, row, Color::BLACK);
    u64_to_hex(a, buf); fb->draw_string(buf, 11, row++, Color::BLACK);

    fb->draw_string("alloc b: 0x", 0, row, Color::BLACK);
    u64_to_hex(b, buf); fb->draw_string(buf, 11, row++, Color::BLACK);

    fb->draw_string("alloc c: 0x", 0, row, Color::BLACK);
    u64_to_hex(c, buf); fb->draw_string(buf, 11, row++, Color::BLACK);

    pmm.free_page(b);
    fb->draw_string("freed  b", 0, row++, Color::BLACK);

    PMM_STATUS status = pmm.alloc_pages(1, d);
    fb->draw_string("alloc d: 0x", 0, row, Color::BLACK);
    u64_to_hex(d, buf); fb->draw_string(buf, 11, row++, Color::BLACK);

    if (status == PMM_STATUS::ALLOC_FAIL) {
        fb->draw_string("FAIL: alloc_pages failed", 0, row++, Color::BLACK);
    } else if (d == b) {
        fb->draw_string("PASS: d == b", 0, row++, Color::BLACK);
    } else {
        fb->draw_string("FAIL: d != b", 0, row++, Color::BLACK);
    }

    for (;;) __asm__ volatile ("hlt");
}