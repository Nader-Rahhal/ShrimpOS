#include "color.h"
#include "framebuffer.h"
#include "mmap.h"
#include "util.cpp"
#include "pmm.h"
#include "gdt.h"
#include "tss.h"
#include "image.h"

extern "C" { extern uint8_t _binary_font_psf_start[]; }


// how many entries does a GDT have?
// 1 null, kernel mode code seg, kernel data seg, user code seg, user data seg, task state seg


extern "C" __attribute__((section(".text.kmain"))) void kmain(FrameBuffer* fb, MMap* mm)
{ 
    PMM pmm(mm);

    fb->set_background(Color::WHITE);
    fb->set_font(_binary_font_psf_start);

    const char* msg = "Hello from ShrimpOS";
    uint32_t start_x = (fb->get_width() / 8 - strlen(msg)) / 2;
    uint32_t start_y = (fb->get_height() / 16) / 2;
    fb->draw_string(msg, start_x, start_y, Color::BLACK);

    RGB* image = (RGB*)flag_raw;
    fb->draw_image(image, 0, 0, 640, 427);

    struct SegmentDescriptor null_descriptor;
    null_descriptor.limit_low = 0;
    null_descriptor.base_low = 0;
    null_descriptor.base_mid = 0;
    null_descriptor.access = 0;
    null_descriptor.limit_flags = 0;
    null_descriptor.base_high = 0;

    struct SegmentDescriptor kernel_code_descriptor;
    kernel_code_descriptor.limit_low = 0xFFFF;
    kernel_code_descriptor.base_low = 0;
    kernel_code_descriptor.base_mid = 0;
    kernel_code_descriptor.access = 0x9A;
    kernel_code_descriptor.limit_flags = 0xAF;
    kernel_code_descriptor.base_high = 0;

    struct SegmentDescriptor kernel_data_descriptor;
    kernel_data_descriptor.limit_low = 0xFFFF;
    kernel_data_descriptor.base_low = 0;
    kernel_data_descriptor.base_mid = 0;
    kernel_data_descriptor.access = 0x92;
    kernel_data_descriptor.limit_flags = 0xCF;
    kernel_data_descriptor.base_high = 0;

    struct SegmentDescriptor user_code_descriptor;
    user_code_descriptor.limit_low = 0xFFFF;
    user_code_descriptor.base_low = 0;
    user_code_descriptor.base_mid = 0;
    user_code_descriptor.access = 0xF2;
    user_code_descriptor.limit_flags = 0xCF;
    user_code_descriptor.base_high = 0;

    struct SegmentDescriptor user_data_descriptor;
    user_data_descriptor.limit_low = 0xFFFF;
    user_data_descriptor.base_low = 0;
    user_data_descriptor.base_mid = 0;
    user_data_descriptor.access = 0xFA;
    user_data_descriptor.limit_flags = 0xAF;
    user_data_descriptor.base_high = 0;

    static struct TSS tss;
    
    //void* kernel_stack = pmm.alloc_pages(4);
    //tss.rsp0 = (uint64_t)kernel_stack + 4096 * 4;


    // make tss struct
    uint64_t tss_base = (uint64_t)&tss;

    struct SystemSegmentDescriptor tss_descriptor;
    tss_descriptor.limit_low   = sizeof(tss) - 1;
    tss_descriptor.base_low    = (tss_base >>  0) & 0xFFFF;
    tss_descriptor.base_mid    = (tss_base >> 16) & 0xFF;
    tss_descriptor.access      = 0x89;
    tss_descriptor.limit_flags = 0x00;
    tss_descriptor.base_high   = (tss_base >> 24) & 0xFF;
    tss_descriptor.base_upper  = (tss_base >> 32) & 0xFFFFFFFF;
    tss_descriptor.reserved    = 0;

    static struct GDT gdt;
    gdt.null_descriptor = null_descriptor;
    gdt.kernel_code_descriptor = kernel_code_descriptor;
    gdt.kernel_data_descriptor = kernel_data_descriptor;
    gdt.user_code_descriptor = user_code_descriptor;
    gdt.user_data_descriptor = user_data_descriptor;
    gdt.tss_descriptor = tss_descriptor;

    struct GDTR gdtr;
    gdtr.offset = (uint64_t)&gdt;
    gdtr.size = sizeof(gdt) - 1;

    __asm__ volatile (
        "lgdt %0\n"
        "push $0x08\n"          // kernel code selector
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        "mov $0x10, %%ax\n"     // kernel data selector
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        :
        : "m"(gdtr)
        : "rax", "memory"
    );


    for (;;) __asm__ volatile ("hlt");
}