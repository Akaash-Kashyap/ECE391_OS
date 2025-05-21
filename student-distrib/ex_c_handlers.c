/** Exception/Interrupt handlers */
#include "ex_c_handlers.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "keyboard.h"
#include "system_calls.h"



// extern void ex_c_handler_0(void); // Divide by zero
void ex_c_handler_0(){
    clear();
    printf("Error 0: Divide by zero\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_1(void); // Debug
void ex_c_handler_1(){
    clear();
    printf("Error 1: Debug\n");
    // goto execute_done;
    halt(1);
}
// extern void ex_c_handler_2(void); // Non-maskable Interrupt
void ex_c_handler_2(){
    clear();
    printf("Error 2: Non-maskable Interrupt\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_3(void); // Breakpoint
void ex_c_handler_3(){
    clear();
    printf("Error 3: Breakpoint\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_4(void); // Overflow
void ex_c_handler_4(){
    clear();
    printf("Error 4: Overflow\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_5(void); // Bound Range Exceeded
void ex_c_handler_5(){
    clear();
    printf("Error 5: Bound Range Exceeded\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_6(void); // Invalid Opcode
void ex_c_handler_6(){
    clear();
    printf("Error 6: Invalid Opcode\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_7(void); // Device Not Available
void ex_c_handler_7(){
    clear();
    printf("Error 7: Device Not Available\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_8(void); // Double Fault
void ex_c_handler_8(){
    clear();
    printf("Error 8: Double Fault\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_9(void); // Coprocessor Segment Overrun
void ex_c_handler_9(){
    clear();
    printf("Error 9: Coprocessor Segment Overrun\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_10(void); // Invalid TSS
void ex_c_handler_10(){
    clear();
    printf("Error 10: Invalid TSS\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_11(void); // Segment Not Present
void ex_c_handler_11(){
    clear();
    printf("Error 11: Segment Not Present\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_12(void); // Stack-Segment Fault
void ex_c_handler_12(){
    clear();
    printf("Error 12: Stack-Segment Fault\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_13(void); // General Protection Fault
void ex_c_handler_13(){
    clear();
    printf("Error 13: General Protection Fault\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_14(void); // Page Fault
void ex_c_handler_14(){
    //clear();
    printf("Error 14: Page Fault\n");
    // printf("Page Fault Line Number: %d\n", get_line_number());

    printf("PCB %d: \n", 0);
    printf("Parent: %d, ESP: %x, EBP: %x, Terminal: %d\n", get_pcb_pid(0).parent_pcb_pid, get_pcb_pid(0).esp, get_pcb_pid(0).ebp, get_pcb_pid(0).terminal_idx);
    
    printf("PCB %d: \n", 1);
    printf("Parent: %d, ESP: %x, EBP: %x, Terminal: %d\n", get_pcb_pid(1).parent_pcb_pid, get_pcb_pid(1).esp, get_pcb_pid(1).ebp, get_pcb_pid(1).terminal_idx);
    
    printf("PCB %d: \n", 2);
    printf("Parent: %d, ESP: %x, EBP: %x, Terminal: %d\n", get_pcb_pid(2).parent_pcb_pid, get_pcb_pid(2).esp, get_pcb_pid(2).ebp, get_pcb_pid(2).terminal_idx);
    
    printf("PCB %d: \n", 3);
    printf("Parent: %d, ESP: %x, EBP: %x, Terminal: %d\n", get_pcb_pid(3).parent_pcb_pid, get_pcb_pid(3).esp, get_pcb_pid(3).ebp, get_pcb_pid(3).terminal_idx);
    
    printf("PCB %d: \n", 4);
    printf("Parent: %d, ESP: %x, EBP: %x, Terminal: %d\n", get_pcb_pid(4).parent_pcb_pid, get_pcb_pid(4).esp, get_pcb_pid(4).ebp, get_pcb_pid(4).terminal_idx);
    
    printf("PCB %d: \n", 5);
    printf("Parent: %d, ESP: %x, EBP: %x, Terminal: %d\n", get_pcb_pid(5).parent_pcb_pid, get_pcb_pid(5).esp, get_pcb_pid(5).ebp, get_pcb_pid(5).terminal_idx);
    
    printf("Terminal Array: %d %d %d \n", get_terminal_array_entry(0), get_terminal_array_entry(1), get_terminal_array_entry(2));
    while(1);
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_16(void); // x87 Floating-Point Exception
void ex_c_handler_16(){
    clear();
    printf("Error 16: x87 Floating-Point Exception\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_17(void); // Alignment Check
void ex_c_handler_17(){
    clear();
    printf("Error 17: Alignment Check\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_18(void); // Machine Check
void ex_c_handler_18(){
    clear();
    printf("Error 18: Machine Check\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_19(void); // SIMD Floating-Point Exception
void ex_c_handler_19(){
    clear();
    printf("Error 19: SIMD Floating-Point Exception\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_20(void); // Virtualization Exception
void ex_c_handler_20(){
    clear();
    printf("Error 20: Virtualization Exception\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_21(void); // Control Protection Exception
void ex_c_handler_21(){
    clear();
    printf("Error 21: Control Protection Exception\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_28(void); // Hypervisor Injection Exception
void ex_c_handler_28(){
    clear();
    printf("Error 28: Hypervisor Injection Exception\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_29(void); // VMM Communication Exception
void ex_c_handler_29(){
    clear();
    printf("Error 29: VMM Communication Exception\n");
    // goto execute_done;
    halt(1);
}

// extern void ex_c_handler_30(void); // Security Exception
void ex_c_handler_30(){
    clear();
    printf("Error 30: Security Exception\n");
    // goto execute_done;
    halt(1);
}

void ex_c_handler_32(){
    pit_handler();
}

/* Keyboard Handler */
void ex_c_handler_33(){
    keyboard_handler();
}

/* RTC Handler */
void ex_c_handler_40(){
    rtc_handler();
}

// extern void ex_c_handler_128(void); // System Call
// void ex_c_handler_128(){
//     clear();

//     //
//     printf("System Call");
//     while(1);
// }

