/** Handler header file */
#ifndef EX_C_HANDLERS_H
#define EX_C_HANDLERS_H
#include "lib.h"
#include "multiboot.h"
#include "x86_desc.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "scheduling.h"
#include "pcb.h"
#include "multiple_terminals.h"

extern void ex_c_handler_0(void); // Divide by zero
extern void ex_c_handler_1(void); // Debug
extern void ex_c_handler_2(void); // Non-maskable Interrupt
extern void ex_c_handler_3(void); // Breakpoint
extern void ex_c_handler_4(void); // Overflow
extern void ex_c_handler_5(void); // Bound Range Exceeded
extern void ex_c_handler_6(void); // Invalid Opcode
extern void ex_c_handler_7(void); // Device Not Available
extern void ex_c_handler_8(void); // Double Fault
extern void ex_c_handler_9(void); // Coprocessor Segment Overrun
extern void ex_c_handler_10(void); // Invalid TSS

extern void ex_c_handler_11(void); // Segment Not Present
extern void ex_c_handler_12(void); // Stack-Segment Fault
extern void ex_c_handler_13(void); // General Protection Fault
extern void ex_c_handler_14(void); // Page Fault
extern void ex_c_handler_16(void); // x87 Floating-Point Exception
extern void ex_c_handler_17(void); // Alignment Check
extern void ex_c_handler_18(void); // Machine Check
extern void ex_c_handler_19(void); // SIMD Floating-Point Exception
extern void ex_c_handler_20(void); // Virtualization Exception
extern void ex_c_handler_21(void); // Control Protection Exception

extern void ex_c_handler_28(void); // Hypervisor Injection Exception
extern void ex_c_handler_29(void); // VMM Communication Exception
extern void ex_c_handler_30(void); // Security Exception

extern void ex_c_handler_32(void); // PIT Interrupt
extern void ex_c_handler_33(void); // Keyboard Interrupt
extern void ex_c_handler_40(void); // RTC Interrupt

extern void ex_c_handler_128(void); // System Call
#endif
