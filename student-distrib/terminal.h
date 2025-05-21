#ifndef TERMINAL_H
#define TERMINAL_H
#include "types.h"
#include "lib.h"
#include "multiple_terminals.h"
#include "pcb.h"
#include "system_calls.h"
#define BUF_SIZE  128
#define ENTER_10    10
#define ENTER_13    13
#define TAB         9
#define BACKSPACE   8

//struct that tracks details buffer
typedef struct terminal_info{
    volatile int enter_pressed; /*flag that gets set when enter is pressed, letting terminal_Read know to start reading*/
    volatile int count;         /*tracks how many chars added to the buffer*/
    //! Need buffer for all 3 termminals
    //!need to keep track of terminal id
}terminal_info;






/*read up to 128 characters from the terminal and transfer into user passed in buffer*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/*write contents of user passed in buffer to terminal*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/*open the terminal given a file name*/
int32_t terminal_open(const uint8_t* filename);

/*close terminal given file descriptor*/
int32_t terminal_close(int32_t fd);

/*copy char from keyboard interrupt into 128 length buffer*/
void add_char(uint8_t c);
#endif
