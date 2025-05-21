/**********************************************
 * Scheduling
 * Scheduling.c
 *********************************************/

#include "scheduling.h"
#define MAX_PIT_SPEED 1193182

/* Variables */
int curr_scheduled = 0; // Terminal index of current program running (1 of 3)
uint32_t esp_val = 0;   // ESP Register
uint32_t ebp_val = 0;   // EBP Register

/* Debug variables */
int pid_arr_idx = 0;
int num_pit_interrupts = 0;
int page_fault_line_number = 0;

/* Global active program */
int global_pid = 0;
uint8_t* cmd_name;

/*
 * setup_pit
 *   DESCRIPTION: Initialize the PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables IRQ0 and PIT
 *   NOTE: PIT = Programmable Interrupt Timer
 */  
int setup_pit() {

    /* Calculate round robbin switching freq */
    int freq = MAX_PIT_SPEED / 50;       // 100 Hz

    /* Configure PIT */
    outb(0x36, 0x43);                   // 0x06 = mode 3 (Square Wave) | 0x30 = two byte config
    outb(freq & 0xFF, 0x40);            // 1st Byte: Low byte of the frequency
    outb((freq & 0xFF00) >> 8, 0x40);   // 2nd Byte: High byte of the frequency

    /* Turn on PIC port */
    enable_irq(0);
    return 0;
}

/*
 * pit_handler
 *   DESCRIPTION: Switch to next scheduled task every time
 *                the PIT generates an interrrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Updates global PID and changes stacks
 */  
void pit_handler() {

    send_eoi(0);
    memcpy((char *) 0xB8000, (char*) 0xB8000 + get_term_num() * 0x1000, 4096);
    // return;
    num_pit_interrupts += 1;
    if (num_pit_interrupts == 1) {
        execute((const uint8_t*)"shell");
    }

    /* Find next program to schedule and check if valid */
    if (schedule() < 0) {

        /* No program to schedule, return */
        send_eoi(0);
        return;
    }
    
    /* Task switch with next program found from schedule() */
    // cli();
    scheduling_context_switch();

}

/*
 * schedule()
 *   DESCRIPTION: Searches active program list to find next
 *                program via round robin search
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 = Success, -1 = Failure
 *   SIDE EFFECTS: none
 * 
 */
int schedule() {
    
    /* Next active program to schedule */
    int loopstart = (curr_scheduled + 1) % 3;
    int i;

    /* Scan terminal array for next program to schedule */
    for (i = loopstart; i < (loopstart + 3); i++) {

        /* Confirm non-NULL entry */
        if (get_terminal_array_entry((i % 3)) >= 0) {

            /* Set next scheduled program */
            curr_scheduled = (i % 3);
            return 0;
        }
    }

    /* No programs running, No scheduling */
    return -1;
}

/*
 * scheduling_context_switch()
 *   DESCRIPTION: Updated global PID and task switches to
 *                run next program
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE:
 *   SIDE EFFECTS: Edits ss0, esp0, ebp, and esp
 * 
 */
void scheduling_context_switch() {
    
    /********** Get current program PID **********/
    global_pid = get_global_pid();

    int past_pid = global_pid;

    /********** Save EBP and ESP  **********/
    //? Vid map maybe?

   

    /*************** Update to next Global PID  ***************/
    set_global_pid(get_terminal_array_entry(curr_scheduled));
    global_pid = get_global_pid();
    //TODO: Video Map?

    /* Ensure not a terminal */
    // if(get_pcb_pid(global_pid).parent_pcb_pid != -1){

        /*************** Setup New Paging  ***************/
        execute_page_setup((uint32_t) (4 * global_pid + 8)); // sets up the page + VA and PA mapping
        // flush_tlbs();

        // cmd_name = (uint8_t*) get_pcb_pid(global_pid).cmd;
        // user_level_program_loader(cmd_name);

        /*************** Set TSS ***************/
            set_tss_ss0(0x00000018);                            //0x800000: 8MB, virtual adress of bottom of kernel page
            set_tss_esp0(0x800000 - (global_pid * 0x2000)-4); //0x2000: 8 kb, the amount of space per task kernel stack*/

        // printf("pid: %d\n", global_pid);

            
            
            /********** Reset interrrupt **********/
            send_eoi(0);
            


             /********** Save EBP and ESP  **********/
                asm volatile(
                    "movl %%esp, %0     \n;"
                    :"=r" (esp_val)
                    :
                    :"memory"
                );
                asm volatile(
                    "movl %%ebp, %0     \n;"
                    :"=r" (ebp_val)
                    :
                    :"memory"
                );

                /*************** Save old ESP & EBP to past PID ***************/
                set_pcb_esp(past_pid, esp_val);
                set_pcb_ebp(past_pid, ebp_val);


                /********** Get next EBP and ESP from next PID **********/
            esp_val = get_esp(global_pid);
            ebp_val = get_ebp(global_pid);

            // sti();


            // printf("esp_val: %x\n", esp_val);
            // printf("ebp_val: %x\n", ebp_val);
            /********** Set next EBP and ESP **********/
            asm volatile(
                "movl %0, %%esp     \n;"
                "movl %1, %%ebp     \n;"
                :
                :"g" (esp_val), "g" (ebp_val)
                :"memory"
            );


            
            
    // }
    // else{
    //     send_eoi(0);
    // }   

    /********** Switch **********/
    // printf("Finished!\n");

    return;
}


//! Old code

    //      * Scheduling order
    //  * 
    //  *
    //  *
    //  *
    //  *
    //  * set tss.ss0 to kernel ds
    //  * set tss.esp0 to the tasks kernel stack (wherever that may be)
    //  *  -> maybe put it as a variable into the pcb?
    //  * sti()
    //  * save the ebp and esp values using small asm snippet
    //  *  -> put it into the pcb
    //  * sti() again for some reason ?// might not need it
    //  * pass in the magic numbers using asm snippet look at system_calls.c:395
    //  * have a asm label to jump back to, make sure it is not "returnhere"
    //  * return 0
    //  * 
    //  * HONESTLY we should make a context switch function and use that function 
    //  * everywhere, with inputs and stuff
    //  * something like this
    //  * 
    //  * context_switch(vars that tell us where to switch); 
    //  * 
    //  * 

    // int get_line_number() {
    //     return page_fault_line_number;
    // }


    //   asm volatile(
    //         "movl %%ecx, %%ebp         \n;"
    //         :
    //         :"c" (ebp_val)
    //         :"memory"
    //     );

    //     asm volatile(
    //         "movl %%ebx, %%esp         \n;"
    //         "popl %%ecx                \n;"
    //         :
    //         :"b" (esp_val)
    //         :"memory"
    //     );

    //     page_fault_line_number = 319;


    ////////
        /* Next item scheduled */
    // int loopstart = (curr_scheduled + 1) % 3;
    
    // int set_curr_scheduled_to = -1;
    // int i;
    
    // for (i = loopstart; i < 3; i++) {
    //     if (get_terminal_array_entry(i) >= 0) {
    //         set_curr_scheduled_to = i;
    //         break;
    //     }
    // }
    // if (set_curr_scheduled_to == -1) {
    //     for (i = 0; i < loopstart; i++) {
    //         if (get_terminal_array_entry(i) >= 0) {
    //             set_curr_scheduled_to = i;
    //             break;
    //         }
    //     }
    // }

    // if (set_curr_scheduled_to < 0) {
    //     return -1;
    // }

    // curr_scheduled = set_curr_scheduled_to;

    // return 0;

    ///////////////////

    /* Saves the current ESP and EBP values */
    // asm volatile(
    //     "mov %%ebp, %%eax            ;\n"
    //     "mov %%esp, %%ebx            ;\n"
    //     :"=a" (ebp_val), "=b" (esp_val)
    //     :
    // );
    // asm volatile(
    //     "pushl %%ecx               \n;"
    //     "movl %%ebp, %%ecx         \n;"
    //     :"=c" (ebp_val)
    //     :
    //     :"memory"
    // );

    // page_fault_line_number = 250;

    // asm volatile (
    //     "movl %%esp, %%ebx         \n;"
    //     :"=b" (esp_val)
    //     :
    //     :"memory"
    // );

    ///////////////////////
        /* Clear interrupt */
    
    // return;
    // return;

    // num_pit_interrupts += 1;

    // if (num_pit_interrupts == 1) {
    //     execute((const uint8_t*)"shell");

    // }

    // if (num_pit_interrupts == 20) {
    //     // send_eoi(1);
    //     // cli();
    //     // memcpy(0xb8000, 0xba000, 4096);
    //     // sti();
    //     // set_term_num(2);
    //     // switch_terminals(2);

    //     // send_eoi(1);
    //     // while(1);
    //     // send_eoi(1);

    //     // execute((const uint8_t*)"shell");
    // }

    // else if (num_pit_interrupts == 40) {
    //     // send_eoi(1);
    //     // cli();
    //     // memcpy(0xb8000, 0xbb000, 4096);
    //     // sti();
    //     // set_term_num(3);
    //     // switch_terminals(3);
    //     // send_eoi(1);
    //     // execute((const uint8_t*)"shell");
    // }

    // else if (num_pit_interrupts == 60) {
    //     // cli();
    //     // memcpy(0xb8000, 0xb9000, 4096);
    //     // sti();
    //     // set_term_num(1);
    //     // switch_terminals(1);
    // }

    ////////////////////// 

    // /* Gets the PID of the next process to run and sets the PID value */
    // // int curr_pid_no = get_terminal_array_entry(curr_scheduled);
    
    // page_fault_line_number = 270;

    // /* Gets the updated PID value and gets the PCB of the updated PID*/
    // new_scheduled_pcb = get_pcb_pid(global_pid);

    // page_fault_line_number = 276;

    // /* Gets the physical addreses of the PCB with the PID. Sets up paging and loads the program into 128-132 MB of virtual memory */
    // phys_address_mb = (global_pid * 4) + 8;
    // execute_page_setup(phys_address_mb); // sets up the page + VA and PA mapping
    // user_level_program_loader((uint8_t*) new_scheduled_pcb.cmd); // copies the file to the given VA

