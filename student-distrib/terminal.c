#include "terminal.h"
static uint8_t key_buf[BUF_SIZE];                     //buffer to store keyboard input
static uint8_t key_buf_1[BUF_SIZE];   
static uint8_t key_buf_2[BUF_SIZE];   
static uint8_t key_buf_3[BUF_SIZE];   
//initialize struct variables to 0
static terminal_info info = {0, 0};
static int typed_command;

/* void add_char(void)
 * DESCRIPTION: copies char processed in keyboard interrupt into buffer of size 128
 * INPUTS: unsigned 8 bit integer (char), passed in by keyboard interrupt handler
 * OUTPUTS: None
 * SIDE EFFECTS: Fills in buffer with 1 character at a time, sets enter_pressed flag, and increments count.
 * 
*/

void add_char(uint8_t c){
    int i;
    int tid = get_term_num();
    uint8_t * cur_buf = key_buf_1;
    switch(tid){
        case 1:
            cur_buf = key_buf_1;
            break;
        case 2:
            cur_buf = key_buf_2; 
            break;
        case 3:
            cur_buf = key_buf_3;
            break;
        default:
            break;
    }
    switch(c){
        case ENTER_10:
            if(info.count == BUF_SIZE){
                cur_buf[BUF_SIZE-1] = c;
            } 
            else{
                cur_buf[info.count] = c;
                info.count++;
            }
            info.enter_pressed = 1;
            putc(c);
            break;
        case BACKSPACE:
            if(info.count > 0){
                cur_buf[info.count-1] = 0;
                info.count--;
                putc(c);
            }
            else{
                info.count = 0;
            }
            break;
        case TAB:
            for(i=0; i<4; i++){
            /*only add as many null chars to represent space as allowed by the buffer.
                buffer overflow is handled when enter is pressed (if condition above).
                So, here the last character of the buffer might be set to nul but when
                enter is eventually pressed, index 127 will be replaced with '\n'*/
                if(info.count < BUF_SIZE){
                    cur_buf[info.count] = 32;
                    putc(0);
                    info.count++;
                }
            }
            break;
        default:
            if(info.count < BUF_SIZE){
            /*add the character to the buffer */
                cur_buf[info.count] = c;
                info.count++;
                putc(c);
            }
            break;
    }
    // cli();
    memcpy(key_buf, cur_buf, 128);
    // stid();
}

/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * DESCRIPTION: copies char_buffer into user buffer. Ensures that more than 128 
                by are tried to be copied into user buffer. 
 * INPUTS: file descriptor, user buffer (generic pointer type), and number of bytes to be read
 * OUTPUTS: returns the number of bytes read from the key_buf
 * SIDE EFFECTS: resets count, enter flag, and buffer after every read.
 * 
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    /*-Returns number of bytes read, or -1 in failure
        - fill in buffer passed in as argument with one line terminated by enter (from key_buf),
          or how much fits in buffer from one such line. Includes line feed (enter, ascii 10).
         */
    int tid = get_term_num();
    uint8_t * cur_buf = key_buf_1;
    switch(tid){
        case 1:
            cur_buf = key_buf_1;
            break;
        case 2:
            cur_buf = key_buf_2; 
            break;
        case 3:
            cur_buf = key_buf_3;
            break;
        default:
            break;
    }
    /*null pointer has been passed, exit with failure*/
    if(buf == 0){
        return -1;
    }

    /*initialize bytes read to 0*/
    int32_t bytes_read = 0;

    //printf("\nEnter hasn't been pressed yet : fd = %d, nbytes = %d", fd, nbytes);
    //while(1);
    /*do not read from terminal until user presses enter*/
    while(info.enter_pressed != 1){}
    
    /*user buffer passed as a void pointer, cast as a char (uint8_t) pointer*/
    uint8_t* buffer = (uint8_t*) buf;
    //printf("\nRight before CLI");
    /*do not allow interrupts, namely keyboard interrupts. 
        This is because the following critical section needs to execute 
        without global variable key_buf, enter_pressed, and count being overwritten.
        Keyboard interrupts call add_char, which reads/writes to these global variables. */
    cli();
    /*loop variable i*/
    int i;
    /*if the number of bytes user wants to read is greater than what is contained in the buffer,
        only copy the entire char_buffer to the user's buffer. */
    if(nbytes > BUF_SIZE){
        for(i = 0; i < BUF_SIZE; i++){
            buffer[i] = cur_buf[i];
            bytes_read++;
            if(cur_buf[i] == '\n') break;
        }
    }
    /*otherwise, only copy over the number of bytes that the user requests*/
    else{
        for(i = 0; i < nbytes; i++){
            buffer[i] = cur_buf[i];
            bytes_read++;
            if(cur_buf[i] == '\n') break;
        }
    }

    /*reset flag, count, and buffer*/
    info.enter_pressed = 0; 
    info.count = 0;
    for(i = 0; i<BUF_SIZE; i++){
        cur_buf[i] = 0;
    }
    /*unmask interrupts*/
    sti();
    //printf("\nAfter STI");
    return bytes_read;
}

/* int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes)
 * DESCRIPTION: write contents of user buffer to the terminal.
 * INPUTS: file descriptor, user buffer (generic pointer type), and number of bytes to be read
 * OUTPUTS: returns the number of bytes written from the user buffer
 * SIDE EFFECTS: characters are printed to the screen
 * 
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    /*-writes data to terminal
        - display all data to screen immediately (printf?)
        - return number of bytes written, or -1 in failure*/
    cli();
    int terminal = get_pcb_pid(get_global_pid()).terminal_idx;
    // printf("Terminal Number: %d\n", terminal);
    /*null pointer is passed, return with failure*/
    if(buf ==0){
        return -1;
    }

    /*user buffer passed as a void pointer, cast as a char (uint8_t) pointer*/
    uint8_t * buffer = (uint8_t *)buf;

    /*initialize bytes_written to 0*/
    int32_t bytes_written = 0;

    /*counter to loop through chars in caller's buffer*/
    int i;
    
    /*if the length of the caller's buffer is less than the number of bytes that the caller wants
        to write, set the number of bytes to write equal to the size of the buffer.*/
    if(strlen((const char*) buffer) < nbytes){
        nbytes = strlen((const char*) buffer);
    }
    
    /*loop through the caller's biffer and write it to the terminal*/
    for(i=0; i < nbytes; i++){
        /*ignore null characters. */
        if(buffer[i]==0){
            continue;
        }
        /*print out all other characters*/
        else{
            putc_term(buffer[i], terminal);
            bytes_written++;
        }
    }
    sti();

    /*return bytes written*/
    return bytes_written; 
    
}

/* int32_t terminal_open(const uint8_t* filename)
 * DESCRIPTION: returns 0 (terminal does not need to be opened)
 * INPUTS: filename uint8 pointer
 * OUTPUTS: 0
 * SIDE EFFECTS: none
 * 
*/
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/* int32_t terminal_close(const uint8_t* filename)
 * DESCRIPTION: returns 0 (terminal does not need to be closed)
 * INPUTS: file descriptor
 * OUTPUTS: 0
 * SIDE EFFECTS: none
 * 
*/
int32_t terminal_close(int32_t fd){
    return 0;
}

int get_command_type() {
    return typed_command;
}

int set_command_type(int val) {
    typed_command = val;
    return 0;
}
