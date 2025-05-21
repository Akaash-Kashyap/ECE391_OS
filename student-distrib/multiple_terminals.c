#include "multiple_terminals.h"

#define VIDEO       0xB8000
#define VIDEO_T1     0xB9000
#define VIDEO_T2     0xBA000
#define VIDEO_T3     0xBB000
#define NUM_COLS    80
#define NUM_ROWS    25

static int terminal_number = 1; // current terminal being displayed
static char* video_mem = (char *)VIDEO;

//char video_mem_t1[4096];
static char* video_mem_t1 = (char *)VIDEO_T1;
static char* video_mem_t2 = (char *)VIDEO_T2;
static char* video_mem_t3 = (char *)VIDEO_T3;

static int terminal_array[3] = {-1, -1, -1};

// if any initialization needs to be done, it can be done here
void terminal_init() {
    return;
}

// switches terminals to terminal term_num specified in the argument
int switch_terminals (int term_num) {
    
    if (term_num < 1 || term_num > 3) {
        return -1;
    }
    
    // call paging function that points video memory to repective terminal address
    if (term_num == 1) {
        // call function that points video memory to terminal 1
        terminal_number = 1;
        // cli();
        memcpy(video_mem, video_mem_t1,  NUM_ROWS * NUM_COLS * 2);
        // sti();
        //!need to restore cursor position and screen x y
        // term_1_display();
    }
    else if (term_num == 2) {
        // call function that points video memory to terminal 2
        terminal_number = 2;
        // cli();
        memcpy(video_mem, video_mem_t2,  NUM_ROWS * NUM_COLS * 2);
        // sti();
        //!need to restore cursor position and screen x y
        // term_2_display();
    }
    else {
        // call function that points video memory to terminal 3
        terminal_number = 3;
        // cli();
        memcpy(video_mem, video_mem_t3,  NUM_ROWS * NUM_COLS * 2);
        // sti();
        //!need to restore cursor position and screen x y
        // term_3_display();
    }
    // update_cursor();
    return 0;
}

// writes first nbytes of data into first nbytes of terminal 1 buffer
int write_to_term_1 (int8_t* data, uint32_t nbytes) {
    if (nbytes > 4096) {
        return -1;
    }
    // write data to nbytes in terminal 1 address
    int i;
    for (i = 0; i < nbytes; i++) {
        terminal_1[i] = data[i];
    }
    return 0;
}

// writes first nbytes of data into first nbytes of terminal 2 buffer
int write_to_term_2 (int8_t* data, uint32_t nbytes) {
    if (nbytes > 4096) {
        return -1;
    }
    // write data from offset to offset + nbytes in terminal 2 data
    int i;
    for (i = 0; i < nbytes; i++) {
        terminal_2[i] = data[i];
    }
    return 0;
}

// writes first nbytes of data into first nbytes of terminal 3 data
int write_to_term_3 (int8_t* data, uint32_t nbytes) {
    if (nbytes > 4096) {
        return -1;
    }
    // write data from offset to offset + nbytes in terminal 2 data
    int i;
    for (i = 0; i < nbytes; i++) {
        terminal_3[i] = data[i];
    }
    return 0;
}

// copies terminal 1 buffer into video memory
int term_1_display () {
    //memcpy(VIDMEMADDR, TERM1ADDR, 4096); // 4096 because 4096 bytes in video memory page
    return 0;
}

// copies terminal 2 buffer into video memory
int term_2_display () {
    //memcpy(VIDMEMADDR, TERM2ADDR, 4096); // 4096 because 4096 bytes in video memory page
    return 0;
}

// copies terminal 3 buffer into video memory
int term_3_display () {
    //memcpy(VIDMEMADDR, TERM3ADDR, 4096); // 4096 because 4096 bytes in video memory page
    return 0;
}

// gets the terminal number that is currently being displayed, used for external files
int get_term_num() {
    return terminal_number;
}

void set_term_num(int term_num) {
    terminal_number = term_num;
}


//each terminal has theiir own pcb
/*keep track of parent pcb for each terminal*/

void set_terminal_array_entry(int idx, int input_pid) {
    terminal_array[idx] = input_pid;
}

int get_terminal_array_entry(int idx) {
    return terminal_array[idx];
}
