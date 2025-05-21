#include "types.h"
#include "lib.h"
#include "paging.h"

void terminal_init(void);

int switch_terminals(int term_num);

int write_to_term_1(int8_t* data, uint32_t nbytes);

int write_to_term_2(int8_t* data, uint32_t nbytes);

int write_to_term_3(int8_t* data, uint32_t nbytes);

int term_1_display(void);

int term_2_display(void);

int term_3_display(void);

int get_term_num(void);

void set_term_num(int term_num);

// buffer that holds the value of terminal 1 data
uint8_t terminal_1[4096] __attribute__((aligned(4096)));

// buffer that holds the value of terminal 2 data
uint8_t terminal_2[4096] __attribute__((aligned(4096)));

// buffer that holds the value of terminal 3 data
uint8_t terminal_3[4096] __attribute__((aligned(4096)));

void set_terminal_array_entry(int idx, int input_pid);

int get_terminal_array_entry(int idx);
