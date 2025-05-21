/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/*
 * i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC and
 *                disable all PIC interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void i8259_init(void) {
 
    /* Block external interrupts */
    unsigned long flag;
    cli_and_save(flag);

    /* Mask all PIC interrupts */
    outb(0xFF, MASTER_8259_PORT_DATA);
    outb(0xFF, SLAVE_8259_PORT_DATA);

    /* Start up commands for Primary PIC */
    outb(ICW1, MASTER_8259_PORT_CMD);
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW4, MASTER_8259_PORT_DATA);

    /* Start up commands for Secondary PIC */
    outb(ICW1, SLAVE_8259_PORT_CMD);
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);
    outb(ICW4, SLAVE_8259_PORT_DATA);
    
    /* Mask all PIC interrupts */
    outb(0xFF, MASTER_8259_PORT_DATA);
    outb(0xFF, SLAVE_8259_PORT_DATA);

    /* Enable interrupts */
    restore_flags(flag);
    printf("Initialized PIC\n");
}

/*
 * enable_irq
 *   DESCRIPTION: Enable (unmaks) a specific IRQ
 *   INPUTS: irq_num - Pin number to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables a PIC IRQ
 */  
void enable_irq(uint32_t irq_num) {

    /* Function variables */
    uint16_t port;  // PIC address
    uint8_t value;  // Mask value
 
    /* Ensure irq_num is inbounds */
    if((irq_num < MIN_IRQ) | (irq_num > MAX_IRQ)){
        return;
    }

    /* Check if MASTER PIC */
    if(irq_num < SLAVE_PIC_OFFSET) {
        port = MASTER_8259_PORT_DATA;
    }
    /* Slave PIC */
    else {
        port = SLAVE_8259_PORT_DATA;
        irq_num -= SLAVE_PIC_OFFSET;
    }
    
    /* Load mask into PIC */
    value = inb(port)  & ~(1 << irq_num);
    outb(value, port);  
}

/*
 * disable_irq
 *   DESCRIPTION: Disable (maks) a specific IRQ
 *   INPUTS: irq_num - Pin number to disable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Disables a PIC IRQ
 */  
void disable_irq(uint32_t irq_num) {

    /* Function variables */
    uint16_t port;  // PIC address
    uint8_t value;  // Mask value
 
    /* Ensure irq_num is inbounds */
    if((irq_num < MIN_IRQ) | (irq_num > MAX_IRQ)){
        return;
    }

    /* Check if MASTER PIC */
    if(irq_num < SLAVE_PIC_OFFSET) {
        port = MASTER_8259_PORT_DATA;
    }
    /* Slave PIC */
    else {
        port = SLAVE_8259_PORT_DATA;
        irq_num -= SLAVE_PIC_OFFSET;
    }

    /* Load mask into PIC */
    value = inb(port) | (1 << irq_num);
    outb(value, port);    
}

/*
 * send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal
 *                to reset PIC interrupt pin
 *   INPUTS: irq_num - Pin number to reset
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Resets a PIC IRQ state
 */  
void send_eoi(uint32_t irq_num) {

    /* Ensure irq_num is inbounds */
    if((irq_num < MIN_IRQ) | (irq_num > MAX_IRQ)){
        return;
    }

    /* Check if slave PIC */
    if(irq_num >= SLAVE_PIC_OFFSET){

        /* Offset IRQ to [0:7] */
        irq_num -=SLAVE_PIC_OFFSET;

        /* Reset SLAVE and MASTER PICs */
        outb((EOI | irq_num), SLAVE_8259_PORT_CMD);
        outb((EOI | SLAVE_IRQ), MASTER_8259_PORT_CMD);
    }
    else{

        /* Reset MASTER PIC */
        outb((EOI | irq_num), MASTER_8259_PORT_CMD);
    }
}
