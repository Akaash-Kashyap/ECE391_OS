/**********************************************
 * REAL TIME CLOCK
 * rtc.c
 *********************************************/

#include "rtc.h"
#include "lib.h"
#include "i8259.h"

/* RTC interrupt flag used to broadcast RTC interrupts */
//! May need to be volitile
static uint8_t rtc_tick[4];
static uint32_t rtc_count[4];

/*
 * rtc_init
 *   DESCRIPTION: Initialize the RTC to default 1024 Hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables IRQ8 and RTC
 */  
void rtc_init(void) {

    /* Block external interrupts */
    unsigned long flag;
    cli_and_save(flag);

    /* Disable NMI */
    outb((NMI_OFF | REG_B), RTC_PORT_CMD);

    /* Read previous B reg value */
    char prev = inb(0x71);

    /* Select reg B again, inb resets to reg D */
    outb((NMI_OFF | REG_B), RTC_PORT_CMD);

    /* Enable RTC IRQ pin driver */
    outb((prev | BIT_6TH_MASK), RTC_PORT_DATA);
   
    /* Enable interrupts */
    restore_flags(flag);

    /* Enable system IRQs */
    enable_irq(0x02);   // Master PIC passthrough
    enable_irq(0x08);   // Slave PIC with RTC

    /* RTC initialized */
    printf("Initialized RTC\n");
}

/*
 * rtc_handler
 *   DESCRIPTION: Action to perform when RTC interrupt
 *                is generated. Resets RTC and system
 *                for next interrupt.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Read and reset RTC register C
 *                 Sets rtc_tick flag high
 */  
void rtc_handler(void){

    /* Block external interrupts */
    unsigned long flag;
    cli_and_save(flag);

    /* Reset RTC tick flag */
    rtc_tick[get_term_num()] = 0x01;

    /* Select reg C and read contents to reset */
    outb(REG_C, RTC_PORT_CMD);
    inb(RTC_PORT_DATA);

    /* Enable interrupts */
    restore_flags(flag);

    /* Clear system interrupt */
    send_eoi(0x08);

}

/*
 * rtc_divide_freq
 *   DESCRIPTION: Change the clock division of the RTC
 *                without changing it's oscillation freq
 *   INPUTS: rate - Value to divide clock by
 *                  Range must be [3:15]
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes the interrupt rate of the RTC
 */  
void rtc_divide_freq(uint8_t rate){

    /* Check if rate inbounds [3~15] */
    if((rate < 3) | (rate > 15)){
        printf("Invalid rtc_divide_freq() rate: %x", rate);
        return;
    }

    /* Get lower 4 bits of rate */
    rate &= 0x0F;

    /* Block external interrupts */
    unsigned long flag;
    cli_and_save(flag);

    /* Select reg A, Disable NMI */
    outb((REG_A | NMI_OFF), RTC_PORT_CMD);

    /* Read previous A data */
    char prev = inb(RTC_PORT_DATA);

    /* Select reg A, inb resets to reg D */
    outb((REG_A | NMI_OFF), RTC_PORT_CMD);

    /* Write new rate to reg A */
    outb(((prev & 0xF0) | rate), RTC_PORT_DATA);

    /* Enable interrupts */
    restore_flags(flag);
}

/*
 * rtc_wait
 *   DESCRIPTION: Wait for an RTC interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Temporarily sets rtc_tick low
 */  
void rtc_wait(void){

    /* Reset RTC tick flag set high by RTC handler */
    rtc_tick[get_term_num()] = 0x00;

    /* Wait for an RTC tick */
    while(rtc_tick[get_term_num()] == 0x00);
}

/*
 * rtc_change
 *   DESCRIPTION: Change freq of virtualized RTC
 *   INPUTS: filename - Unused string to open file
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - Success
 *   SIDE EFFECTS: none
 *   NOTE: RTC will always be on
 */
int32_t rtc_change(int32_t freq){

    /* Local Variables */
    int freq_mult = 0;      // Number of interrrupts needed for desired frequency

    /* Check if freq within bounds */
    if((freq > RTC_MAX) || (freq < RTC_MIN)){
        return -1;
    }

    /* Check if power of 2 frequency */
    /* 
     * EX: 4 = 0x100
     * 0x100 & (0x100 - 1)
     * 0x100 & (0x011) = 0 -> Power of 2
     */
    if((freq & (freq - 1)) != 0){
        return -1;
    }

    /* Calculate frequency iterations */
    freq_mult = RTC_MAX / freq;

    /* Block external interrupts */
    unsigned long flag;
    cli_and_save(flag);

    /* Update RTC Counter */
    rtc_count[get_term_num()] = freq_mult;

    /* Enable interrupts */
    restore_flags(flag);

    return 0;
}

/*
 * rtc_read
 *   DESCRIPTION: Wait for an RTC interrupt
 *                at a specific frequency
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - Success, -1 - Fail
 *   SIDE EFFECTS: Temporarily sets rtc_tick low
 */  
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){

    /* Local vars */
    int count;
    
    /* Loop until desired frequency of RTC interrupts */
    for(count = 0; count < rtc_count[get_term_num()]; count++){
        rtc_wait();
    }

    /* Return 0, RTC waiting over */
    return 0;
}

/*
 * rtc_write
 *   DESCRIPTION: Change freq of virtualized RTC
 *   INPUTS: fd - file descriptor index, buf - buffer, nbytes - size to write
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - Success
 *   SIDE EFFECTS: none
 *   NOTE: RTC will always be on
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    //! TO DO
    //!set virtual frequency
    //!buffer sends in frequency
    int32_t * buffer = (int32_t *)buf;
    if(buffer == 0) return -1;
    int32_t freq = *buffer;
    if(rtc_change(freq) == -1) return -1;
    return 0;
}

/*
 * rtc_open
 *   DESCRIPTION: Open virtualized RTC with freq of 2 Hz
 *   INPUTS: filename - Unused string to open file
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - Success, -1 - Fail
 *   SIDE EFFECTS: none
 *   NOTE: RTC will always be on
 */
int32_t rtc_open(const uint8_t* filename){

    /* Reset RTC frequency to 2 Hz */
    int i = rtc_change(2);
    
    /* Check if bad write */
    if(i != 0){
        return -1;
    }

    /* Succesfully opened */
    return 0;
}

/*
 * rtc_close
 *   DESCRIPTION: Close virtualized RTC
 *   INPUTS: file directory - Unused path to close file
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - Success
 *   SIDE EFFECTS: none
 *   NOTE: RTC will always be on -> close nothing
 */
int32_t rtc_close(int32_t fd){

    /* Check for valid file directory */
    if (fd <= 1 || fd >= 8) {
        return -1;
    }

    /* Fake closed RTC */
    return 0;
}
