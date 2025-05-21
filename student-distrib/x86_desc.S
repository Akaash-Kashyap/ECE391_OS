# x86_desc.S - Set up x86 segment descriptors, descriptor tables
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"

.text

.globl ldt_size, tss_size
.globl gdt_desc, ldt_desc, tss_desc
.globl tss, tss_desc_ptr, ldt, ldt_desc_ptr
.globl gdt_ptr, gdt_desc_ptr
.globl idt_desc_ptr, idt
# .globl isr_generic_int  No longer needed 

.globl useless_handler

.globl ex_asm_handler_0, ex_asm_handler_1, ex_asm_handler_2, ex_asm_handler_3
.globl ex_asm_handler_4, ex_asm_handler_5, ex_asm_handler_6, ex_asm_handler_7
.globl ex_asm_handler_8, ex_asm_handler_9,ex_asm_handler_10, ex_asm_handler_11
.globl ex_asm_handler_12, ex_asm_handler_13,ex_asm_handler_14, ex_asm_handler_15
.globl ex_asm_handler_16, ex_asm_handler_17,ex_asm_handler_18, ex_asm_handler_19
.globl ex_asm_handler_20, ex_asm_handler_21 # ex_asm_handler_2, ex_asm_handler_3
.globl ex_asm_handler_28, ex_asm_handler_29, ex_asm_handler_30, ex_asm_handler_32

.globl ex_asm_handler_33, ex_asm_handler_40
.globl ex_asm_handler_128
.globl load_page_directory, enable_paging, flush_tlbs
.globl sys_call_context_switch_setup

.align 4


tss_size:
    .long tss_bottom - tss - 1

ldt_size:
    .long ldt_bottom - ldt - 1

    .word 0 # Padding
ldt_desc:
    .word KERNEL_LDT
    .long ldt

    .align 4
tss:
_tss:
    .rept 104
    .byte 0
    .endr
tss_bottom:


    .align 4 
    .word 0 # padding
gdt_desc:   # pointer to GDTR
    .word gdt_bottom - gdt - 1 # size of GDT
    .long gdt       # address of GDT

    .align  16

gdt:
_gdt:

    # First GDT entry cannot be used
    .quad 0

    # NULL entry
    .quad 0

    # Segmentation will not be used
    # CS and DS both are 0-4GB r/w segments
    #
    # The layout is (from Intel IA-32 reference manual):
    #  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
    # |----------------------------------------------------------------------|
    # |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
    # | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
    # |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
    # |----------------------------------------------------------------------|
    #
    # |----------------------------------------------------------------------|
    # |                                    |                                 |
    # | Base 15:0                          | Segment Limit 15:0              |
    # |                                    |                                 |
    # |----------------------------------------------------------------------|

gdt_ptr:
    # Set up an entry for kernel CS
    .quad 0x00CF9A000000FFFF

    # Set up an entry for kernel DS
    .quad 0x00CF92000000FFFF

    # Set up an entry for user CS
    .quad 0x00CFFA000000FFFF

    # Set up an entry for user DS
    .quad 0x00CFF2000000FFFF

    # Set up an entry for TSS
tss_desc_ptr:
    .quad 0

    # Set up one LDT
ldt_desc_ptr:
    .quad 0

gdt_bottom:

    .align 16
ldt:
    .rept 4
    .quad 0
    .endr
ldt_bottom:

.align 4
    .word 0 # Padding 
idt_desc_ptr:
    .word idt_bottom - idt - 1
    .long idt
    

    .align  16
idt:
_idt:
    .rept NUM_VEC
    .quad 0
    .endr

idt_bottom:

# void load_page_directory(void* page_directory_pointer)
# DESCRIPTION: Sets CR3 register to the memory location of the start
#              of the page directory (pointer argument)
# INPUTS: void* page_directory_pointer (pointer to the start of the page directory)
# OUTPUTS: None
# SIDE EFFECTS: Changes CR3 and EAX values
.align 4 # aligns to 32 bits
load_page_directory:
    push %ebp
    mov %esp, %ebp
    # M[EBP + 8] gets the value of the first argument
    mov 8(%ebp), %eax
    mov %eax, %cr3
    leave
    ret

.align 4
flush_tlbs:
    push %ebp
    mov %esp, %ebp
    mov %cr3, %eax
    mov %eax, %cr3
    leave
    ret

# void enable_paging()
# DESCRIPTION: Enables 4 MB and 4 kB paging by setting fields in CR0 and CR4 registers
# INPUTS: None
# OUTPUTS: None
# SIDE EFFECTS: Changes CR3, CR4, and EAX values
.align 4 # aligns to 32 bits
enable_paging:
    push %ebp
    mov %esp, %ebp

    # sets page size extention flag in CR4 high
    mov %cr4, %eax
    # 0x00000010 gets bit 4, which sets the page size extention (allowing for 4 MB pages) high
    or 0x00000010, %eax 
    mov %eax, %cr4

    # sets protection enable flag in CR0 high
    mov %cr0, %eax
    # 0x00000001 gets bit 0, which sets the protection enable flag high
    # protection enable must be set before setting paging enable (PG) flag
    or $0x00000001, %eax
    mov %eax, %cr0

    # sets the paging enable flag in CR0 high
    mov %cr0, %eax
    # 0x80000000 gets bit 31, which sets the paging enable flag high 
    or $0x80000000, %eax
    mov %eax, %cr0
    
    leave
    ret

useless_handler: # generic handler all unnassigned handlers will set to 

    iret

# Divide by zero assembly linkage
ex_asm_handler_0: 

    cli
    pushal 
    cld
    call ex_c_handler_0 
    popal
    iret

# Debug assembly linkage
ex_asm_handler_1: 

    cli
    pushal
    cld
    call ex_c_handler_1 
    popal
    iret

# NMI assembly linkage
ex_asm_handler_2: 

    cli
    pushal
    cld
    call ex_c_handler_2 
    popal
    iret

# Breakpoint assembly linkage
ex_asm_handler_3: 

    cli
    pushal
    cld
    call ex_c_handler_3 
    popal
    iret

# Overflow assembly linkage
ex_asm_handler_4: 

    cli
    pushal
    cld
    call ex_c_handler_4 
    popal
    iret

# BOUND Range Exceeded assembly linkage
ex_asm_handler_5: 

    cli
    pushal
    cld
    call ex_c_handler_5 
    popal
    iret

# Invalid Opcode assembly linkage
ex_asm_handler_6: 

    cli
    pushal
    cld
    call ex_c_handler_6 
    popal
    iret

# Device Not Available assembly linkage
ex_asm_handler_7: 

    cli
    pushal
    cld
    call ex_c_handler_7 
    popal
    iret

# Double Fault assembly linkage
ex_asm_handler_8: 

    cli
    pushal
    cld
    call ex_c_handler_8 
    popal
    iret

# Coprocessor Segment Overrun assembly linkage
ex_asm_handler_9: 

    cli
    pushal
    cld
    call ex_c_handler_9 
    popal
    iret

# Invalid TSS assembly linkage
ex_asm_handler_10: 

    cli
    pushal
    cld
    call ex_c_handler_10 
    popal
    iret

# Segment Not Present assembly linkage
ex_asm_handler_11: 

    cli
    pushal
    cld
    call ex_c_handler_11 
    popal
    iret

# Stack-Segment Fault assembly linkage
ex_asm_handler_12: 

    cli
    pushal
    cld
    call ex_c_handler_12 
    popal
    iret

# General Protection assembly linkage
ex_asm_handler_13: 

    cli
    pushal
    cld
    call ex_c_handler_13 
    popal
    iret

# Page Fault assembly linkage
ex_asm_handler_14: 

    cli
    pushal
    cld
    call ex_c_handler_14 
    popal
    iret

# x87 FPU Floating-Point Error assembly linkage
ex_asm_handler_16: 

    cli
    pushal
    cld
    call ex_c_handler_16 
    popal
    iret

# Alignment Check assembly linkage
ex_asm_handler_17: 

    cli
    pushal
    cld
    call ex_c_handler_17 
    popal
    iret

# Machine Check assembly linkage
ex_asm_handler_18: 

    cli
    pushal
    cld
    call ex_c_handler_18 
    popal
    iret

# SIMD Floating-Point Exception assembly linkage
ex_asm_handler_19: 

    cli
    pushal
    cld
    call ex_c_handler_19 
    popal
    iret

# Virtualization Exception assembly linkage
ex_asm_handler_20: 

    cli
    pushal
    cld
    call ex_c_handler_20 
    popal
    iret

# Control Protection Exception assembly linkage
ex_asm_handler_21: 

    cli
    pushal
    cld
    call ex_c_handler_21 
    popal
    iret

# Hypervisor Injection Exception assembly linkage
ex_asm_handler_28: 

    cli
    pushal
    cld
    call ex_c_handler_28 
    popal
    iret

# VMM Communication Exception assembly linkage
ex_asm_handler_29: 

    cli
    pushal
    cld
    call ex_c_handler_29 
    popal
    iret

# Security Exception assembly linkage
ex_asm_handler_30: 

    cli
    pushal
    cld
    call ex_c_handler_30 
    popal
    iret

# PIT Interrupt assembly linkage
ex_asm_handler_32: 

    # no cli 
    pushal
    cld
    call ex_c_handler_32
    popal
    iret

# Keyboard Interrupt assembly linkage
ex_asm_handler_33: 

    # no cli 
    pushal
    cld
    call ex_c_handler_33 
    popal
    iret

# RTC Interrupt assembly linkage
ex_asm_handler_40: 

    # no cli
    pushal
    cld
    call ex_c_handler_40 
    popal
    iret

# System Call assembly linkage
ex_asm_handler_128:   

    # cli
    # Check if saving registers in right order
    # pushal
    cld
    # check if eax is within bounds (0-10)

    cmpl $1, %eax
    jb error_syscall_number
    cmpl $10, %eax
    ja error_syscall_number

    pushl %ebp
    pushl %edi
    pushl %esi
 
    pushl %EDX
    pushl %ECX
    pushl %EBX
    # pushl $1
    # need a level of indirection to setup arguements for the c functions and call them directly there
    
    # _use jumptable for indirection

    # call *sys_call_table(0, %eax, 4)

    call *sys_call_table(, %eax, 4)

    popl %ebx
    popl %ECX
    popl %EDX
    popl %esi
    popl %edi
    popl %ebp
    # popal
    iret

error_syscall_number:
    movl $-1, %eax
    iret

sys_call_table: 
    .long 0x0, sys_call_halt, sys_call_execute, sys_call_read, sys_call_write, sys_call_open, sys_call_close, sys_call_get_args, sys_call_vidmap, sys_call_sethandler, sys_call_sigreturn




# setup_halt:
#     # organize the stack for the c function call
#     call sys_call_halt
#     # check for error code stored in eax
#     # proper post funct ion call handling
#     ret
# 
# setup_execute:
# setup_read:
# setup_write:
# setup_open:
# setup_close:
# setup_getargs:
# setup_vidmap:
# setup_set_handler:
# setup_sigreturn:

#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET

DO_CALL(halt,1)
DO_CALL(execute,2)
DO_CALL(read,3)
DO_CALL(write,4)
DO_CALL(open,5)
DO_CALL(close,6)
DO_CALL(getargs, 7)
DO_CALL(vidmap,8)
DO_CALL(set_handler, 9)
DO_CALL(sigreturn,10)


sys_call_context_switch_setup:
    push %ebp
    mov %esp, %ebp

    pushl 20(%ebp)  # gets ds
    pushl 16(%ebp)  # gets esp
    pushfl
    # popl %eax
    # orl $0x00000200, %eax
    # orl $0x00004000, %eax
    # pushl %eax
    pushl 12(%ebp) # gets  cs
    pushl 8(%ebp)  # gets eip
    
    sti
    # pushl 8(%ebp)  # ds
    # pushl 12(%ebp) # esp
    # pushfl
    # popl %eax
    # orl $0x00000200, %eax
    # orl $0x00004000, %eax
    # orl $0x00003000, %eax
    # pushl %eax
    # pushl 16(%ebp)  # cs
    # pushl 20(%ebp)  # eip
    
    # movw $0x02b, %ax
    # movw %ax, %ds
    iret

    # popl %edx
    # popl %edx
    # popl %edx
    # popl %edx
    # popl %edx

    # leave
    # ret
