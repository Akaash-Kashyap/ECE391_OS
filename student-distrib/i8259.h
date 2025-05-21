/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT_CMD    0x20
#define MASTER_8259_PORT_DATA   0x21
#define SLAVE_8259_PORT_CMD     0xA0
#define SLAVE_8259_PORT_DATA    0xA1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11    // Start Init, Edge-Trigger Inputs, Cascade Mode, 4-ICWs
#define ICW2_MASTER         0x20    // Location of IRQ registers
#define ICW2_SLAVE          0x28    // Location of IRQ registers
#define ICW3_MASTER         0x04    // Location of seondary PIC on IRQs
#define ICW3_SLAVE          0x02    // PIC is secondary PIC
#define ICW4                0x01    // Extra environment info (Use 8086 mode) 

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x20

/* Offsets */
#define SLAVE_PIC_OFFSET    0x08    // Slave IRQs [8:15]
#define SLAVE_IRQ           0x02    // Slave IRQ on master PIC
#define MIN_IRQ             0x00    // Min IRQ number 0
#define MAX_IRQ             0x0F    // Max IRQ number 15

/* Initialize both PICs */
void i8259_init(void);

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */

/*****************************************************
 *                8259A PIC Standard
 * ----
 * Each PIC has two ports (Command & Data)
 * ----
 * Master_PIC:  0x20 - Command
 *              0x21 - Data
 * Slave_PIC:   0x40 - Command
 *              0x41 - Data
 * 
*****************************************************/
