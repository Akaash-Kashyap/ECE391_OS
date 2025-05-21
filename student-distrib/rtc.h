/**********************************************
 * REAL TIME CLOCK
 * rtc.h
 *********************************************/

#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "multiple_terminals.h"

/* Ports that the RTC sits on */
#define RTC_PORT_CMD    0x70
#define RTC_PORT_DATA   0x71

/* Non-Maskable Interrupts */
#define NMI_OFF         0x80
#define NMI_ON          0x00

/* RTC Registers */
#define REG_A           0x0A 
#define REG_B           0x0B
#define REG_C           0x0C

/* General Masks */
#define BIT_6TH_MASK    0x40

/* RTC Frequencies (Hz) */
#define RTC_MAX         1024
#define RTC_MIN         2

/* Initialize the RTC */
void rtc_init(void);

/* Run when RTC interrupt is generated */
void rtc_handler(void);

/* Change the clock division of the RTC */
void rtc_divide_freq(uint8_t rate);

/* Wait for an RTC interrupt */
void rtc_wait(void);

/* Set RTC virtualized frequency */
int32_t rtc_change(int32_t freq);

/* Read the RTC at the current virtualized frequency */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Write to the RTC to change it's virtual frequency */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Open virtual RTC with 2 Hz frequency */
int32_t rtc_open(const uint8_t* filename);

/* Close virtual RTC */
int32_t rtc_close(int32_t fd);

#endif /* _RTC_H */

/*****************************************************
 *                  RTC Standard
 * ----
 * RTC_PORTS:   0x70 - Register Index, Disable NMI
 *              0x71 - Data
 * ----
 * Non-Maskable-Interrupts:
 *              The RTC may be left undefined if NMI
 *              aren't off, because it's never init
 *              by BIOS and always battery powered.
 * ----
 * Periodic Interrupt Enable:
 *              6th bit of reg B that allows int flag
 *              in reg C to drive IRQ pin. 1 = Enabled
 * ----
 * Reg C Interrupt Bitmask:
 *              When RTC IRQ reg C contains bitmask of
 *              type of interrupt (Periodic, Update end,
 *              alarm) that must be reset by reading
 *              before next same interrupt can occur.
 *              (7th bit Interrupt request flag)
 * ----
 * Divider Value:
 *              Lower 4 bits of register A are clock
 *              divider bits. Used to change frequency.
 *              Default: 0110b (6), Range[1, 15]
 *              0 Disabled interrupts
 * ----
 * 
*****************************************************/
