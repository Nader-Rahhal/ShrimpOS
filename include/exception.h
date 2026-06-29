#pragma once
#include "idt.h"
#include "panic.h"


enum class ExceptionResult {
    PANIC,
    RESUME
};

static const char* exception_names[] = {
    "Divide Error",        "Debug",                "NMI",                   "Breakpoint",
    "Overflow",            "Bound Range",          "Invalid Opcode",        "Device Not Available",
    "Double Fault",        "Coprocessor Overrun",  "Invalid TSS",           "Segment Not Present",
    "Stack Fault",         "General Protection",   "Page Fault",            "Reserved",
    "x87 FP Error",        "Alignment Check",      "Machine Check",         "SIMD FP Error",
    "Virtualization",      "Control Protection",   "Reserved",              "Reserved",
    "Reserved",            "Reserved",             "Reserved",              "Reserved",
    "Hypervisor Injection","VMM Communication",    "Security",              "Reserved",
};

ExceptionResult handle_divide_error(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_debug(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_nmi(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_breakpoint(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_overflow(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_bound_range(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_invalid_opcode(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_device_not_available(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_double_fault(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_invalid_tss(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_segment_not_present(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_stack_fault(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_general_protection(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_page_fault(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_x87_fp_error(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_alignment_check(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_machine_check(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_simd_fp_error(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_virtualization(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_control_protection(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_hypervisor_injection(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_vmm_communication(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_security(InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}

ExceptionResult handle_exception(InterruptFrame* frame) {
    switch (frame->vector) {
        case 0:  return handle_divide_error(frame);
        case 1:  return handle_debug(frame);
        case 2:  return handle_nmi(frame);
        case 3:  return handle_breakpoint(frame);
        case 4:  return handle_overflow(frame);
        case 5:  return handle_bound_range(frame);
        case 6:  return handle_invalid_opcode(frame);
        case 7:  return handle_device_not_available(frame);
        case 8:  return handle_double_fault(frame);
        case 10: return handle_invalid_tss(frame);
        case 11: return handle_segment_not_present(frame);
        case 12: return handle_stack_fault(frame);
        case 13: return handle_general_protection(frame);
        case 14: return handle_page_fault(frame);
        case 16: return handle_x87_fp_error(frame);
        case 17: return handle_alignment_check(frame);
        case 18: return handle_machine_check(frame);
        case 19: return handle_simd_fp_error(frame);
        case 20: return handle_virtualization(frame);
        case 21: return handle_control_protection(frame);
        case 28: return handle_hypervisor_injection(frame);
        case 29: return handle_vmm_communication(frame);
        case 30: return handle_security(frame);
        default: return ExceptionResult::PANIC;
    }
}

void divide_by_zero_exception_test(){
    __asm__ volatile ("xor %%ecx, %%ecx; div %%ecx" ::: "eax", "edx", "ecx");
}

inline void exception_handler(InterruptFrame* frame) {
    ExceptionResult result = handle_exception(frame);
    if (result == ExceptionResult::PANIC)
        KPANIC(exception_names[frame->vector]);
}