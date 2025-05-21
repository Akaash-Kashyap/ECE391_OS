/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */



/*
 * BINARY BARBARIANS
 */


#include "multiboot.h"
#include "x86_desc.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "paging.h"
#include "keyboard.h"
#include "rtc.h"
#include "lib.h"
#include "file_system.h"
#include "system_calls.h"
#include "pcb.h"
#include "scheduling.h"


// #define RUN_TESTS

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */

void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;
    struct boot_block_t *start_filesys;
   
    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        start_filesys = (struct boot_block_t*)(mod->mod_start);
        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start); // Start of boot block
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    /** 
     * Starting to fill out the IDT table here
     */
    int i;

    // Filling entire IDT with a generic/useless handler 
    for(i = 0; i < NUM_VEC; i++){
    {
        idt_desc_t the_idt_desc;
        the_idt_desc.present        = 0x1; // the interrupt exists
        the_idt_desc.dpl            = 0x0; // run in highest privilage level (level 0)
        the_idt_desc.reserved0      = 0x0; // must stay 0
        the_idt_desc.size           = 0x1; // type of interrupt here we use 0xF for a task
        the_idt_desc.reserved1      = 0x1; // It is over the next 4 bits with size being 
        the_idt_desc.reserved2      = 0x1; // being the high bit
        the_idt_desc.reserved3      = 0x0; // If 0, this is classified as an interrupt, if 1 is classified as a exception
        // reserved 4 is actually reserved - no need to modify
        the_idt_desc.seg_selector   = KERNEL_CS; 
        
        //place struct into the idt table 
        idt[i] = the_idt_desc;
        // set the given IDT entry (first arg) to run the function (second arg)
        SET_IDT_ENTRY(idt[i], &useless_handler);
    }
    }

    // looping through the first 32 IDT entries and set them to be classified as an exception 
    for(i = 0; i < 32; i++){
    {
        idt_desc_t the_idt_desc;
        the_idt_desc.present        = 0x1; // the interrupt exists
        the_idt_desc.dpl            = 0x0; // run in highest privilage level (level 0)
        the_idt_desc.reserved0      = 0x0; // must stay 0
        the_idt_desc.size           = 0x1; // type of interrupt here we use 0xF for a task
        the_idt_desc.reserved1      = 0x1; // It is over the next 4 bits with size being 
        the_idt_desc.reserved2      = 0x1; // being the high bit
        the_idt_desc.reserved3      = 0x1; // If 0, this is classified as an interrupt, if 1 is classified as a exception
        // reserved 4 is actually reserved - no need to modify
        the_idt_desc.seg_selector   = KERNEL_CS; 
        // place struct into the idt table
        idt[i] = the_idt_desc; 
    }
    }

    /** 
     * Filling out the IDT table with the exception handlers
     * 
     * NOTE: The IDT table is 256 entries long, but we only need to fill out the first 32
     *      because the rest of the entries are not used currently. Some of the first 32 
     *      are also not required due to them being reserved.
     */
    
    SET_IDT_ENTRY(idt[0], &ex_asm_handler_0);   // Divide by Zero error
    SET_IDT_ENTRY(idt[1], &ex_asm_handler_1);   // Debug
    SET_IDT_ENTRY(idt[2], &ex_asm_handler_2);   // NMI Interrupt
    SET_IDT_ENTRY(idt[3], &ex_asm_handler_3);   // Breakpoint
    SET_IDT_ENTRY(idt[4], &ex_asm_handler_4);   // Overflow
    SET_IDT_ENTRY(idt[5], &ex_asm_handler_5);   // Bound Range Exceeded
    SET_IDT_ENTRY(idt[6], &ex_asm_handler_6);   // Invalid Opcode
    SET_IDT_ENTRY(idt[7], &ex_asm_handler_7);   // Device Not Available
    SET_IDT_ENTRY(idt[8], &ex_asm_handler_8);   // Double Fault
    SET_IDT_ENTRY(idt[9], &ex_asm_handler_9);   //? Coprocessor Segment Overrun - crossed out on osdev ?
    SET_IDT_ENTRY(idt[10], &ex_asm_handler_10); // Invalid TSS
    SET_IDT_ENTRY(idt[11], &ex_asm_handler_11); // Segment Not Present
    SET_IDT_ENTRY(idt[12], &ex_asm_handler_12); // Stack-Segment Fault
    SET_IDT_ENTRY(idt[13], &ex_asm_handler_13); // General Protection Fault
    SET_IDT_ENTRY(idt[14], &ex_asm_handler_14); // Page Fault
    SET_IDT_ENTRY(idt[16], &ex_asm_handler_16); // x87 FPU Floating-Point Exception
    SET_IDT_ENTRY(idt[17], &ex_asm_handler_17); // Alignment Check
    SET_IDT_ENTRY(idt[18], &ex_asm_handler_18); // Machine Check
    SET_IDT_ENTRY(idt[19], &ex_asm_handler_19); // SIMD Floating-Point Exception
    SET_IDT_ENTRY(idt[20], &ex_asm_handler_20); // Virtualization Exception
    SET_IDT_ENTRY(idt[21], &ex_asm_handler_21); // Control Protection Exception
    SET_IDT_ENTRY(idt[28], &ex_asm_handler_28); // Hypervisor Injection Exception
    SET_IDT_ENTRY(idt[29], &ex_asm_handler_29); // VMM Communication Exception
    SET_IDT_ENTRY(idt[30], &ex_asm_handler_30); // Security Exception

    // Creating IDT entry for PIT
    {
        idt_desc_t the_idt_desc;
        the_idt_desc.present        = 0x1; // the interrupt exists
        the_idt_desc.dpl            = 0x0; // run in highest privilage level (level 0)
        the_idt_desc.reserved0      = 0x0; // must stay 0
        the_idt_desc.size           = 0x1; // type of interrupt here we use 0xF for a task
        the_idt_desc.reserved1      = 0x1; // It is over the next 4 bits with size being 
        the_idt_desc.reserved2      = 0x1; // being the high bit
        the_idt_desc.reserved3      = 0x0; // If 0, this is classified as an interrupt, if 1 is classified as a exception
        // reserved 4 is actually reserved - no need to modify
        the_idt_desc.seg_selector   = KERNEL_CS; 
        // place struct into the idt table
        idt[32] = the_idt_desc; 
        // set the given IDT entry (first arg) to run the function (second arg)
        SET_IDT_ENTRY(idt[32], &ex_asm_handler_32);
    }

    // Creating IDT entry for keyboard
    {
        idt_desc_t the_idt_desc;
        the_idt_desc.present        = 0x1; // the interrupt exists
        the_idt_desc.dpl            = 0x0; // run in highest privilage level (level 0)
        the_idt_desc.reserved0      = 0x0; // must stay 0
        the_idt_desc.size           = 0x1; // type of interrupt here we use 0xF for a task
        the_idt_desc.reserved1      = 0x1; // It is over the next 4 bits with size being 
        the_idt_desc.reserved2      = 0x1; // being the high bit
        the_idt_desc.reserved3      = 0x0; // If 0, this is classified as an interrupt, if 1 is classified as a exception
        // reserved 4 is actually reserved - no need to modify
        the_idt_desc.seg_selector   = KERNEL_CS; 
        // place struct into the idt table
        idt[33] = the_idt_desc; 
        // set the given IDT entry (first arg) to run the function (second arg)
        SET_IDT_ENTRY(idt[33], &ex_asm_handler_33);
    }

    // Creating IDT entry for RTC
    {
        idt_desc_t the_idt_desc;
        the_idt_desc.present        = 0x1; // the interrupt exists
        the_idt_desc.dpl            = 0x0; // run in highest privilage level (level 0)
        the_idt_desc.reserved0      = 0x0; // must stay 0
        the_idt_desc.size           = 0x1; // type of interrupt here we use 0xF for a task
        the_idt_desc.reserved1      = 0x1; // It is over the next 4 bits with size being 
        the_idt_desc.reserved2      = 0x1; // being the high bit
        the_idt_desc.reserved3      = 0x0; // If 0, this is classified as an interrupt, if 1 is classified as a exception
        // reserved 4 is actually reserved - no need to modify
        the_idt_desc.seg_selector   = KERNEL_CS; 
        // place struct into the idt table
        idt[40] = the_idt_desc;
        // set the given IDT entry (first arg) to run the function (second arg)
        SET_IDT_ENTRY(idt[40], &ex_asm_handler_40);
    }

    // Creating IDT entry for System Calls 0x80
    {
        idt_desc_t the_idt_desc;
        the_idt_desc.present        = 0x1; // the interrupt exists
        the_idt_desc.dpl            = 0x3; // run in userland privilage level (level 3)
        the_idt_desc.reserved0      = 0x0; // must stay 0
        the_idt_desc.size           = 0x1; // type of interrupt here we use 0xF for a task
        the_idt_desc.reserved1      = 0x1; // It is over the next 4 bits with size being 
        the_idt_desc.reserved2      = 0x1; // being the high bit
        the_idt_desc.reserved3      = 0x0; // If 0, this is classified as an interrupt, if 1 is classified as a exception
        // reserved 4 is actually reserved - no need to modify
        the_idt_desc.seg_selector   = KERNEL_CS; 
        // place struct into the idt table
        idt[128] = the_idt_desc;
        // set the given IDT entry (first arg) to run the function (second arg)
        SET_IDT_ENTRY(idt[128], &ex_asm_handler_128);
    }

    /** Tell computer where the IDT is */
    lidt(idt_desc_ptr);

    /** Setup Paging functionality */
    setup_paging();

    /* Enable Devices */
    i8259_init();       // enable the PIC
    keyboard_init();    // enable the keyboard
    rtc_init();         // enable the RTC
    setup_pit();

    clear();
    /* Memory, filesystem, any other initialization stuff... */
    file_system_init(start_filesys);

    

    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    // printf("Enabling Interrupts\n"); //! causes a page fault
    sti();
    // clear(); // clearing page for clarity

    
    execute((const uint8_t*)"shell");

    #ifdef RUN_TESTS
        /* Run tests */
        launch_tests();
    #endif
        /* Execute the first program ("shell") ... */

        /* Spin (nicely, so we don't chew up cycles) */
        asm volatile (".1: hlt; jmp .1;");
}
