
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "types.h"
#include "paging.h"
#include "multiple_terminals.h"
#include "system_calls.h"

#define KEYBOARD_PORT   0x60
#define ACK             0xFA
#define RESEND          0xFE
#define CMD_SCAN        0xF4


/*send command to keyboard to enable scanning*/
void enable_scanning(void);
uint32_t check_keyboard_ack(void);

/* Keyboar ini */
void keyboard_init(void);

/* Keyboard handler */
void keyboard_handler(void);

int get_num_t2(void);

int get_num_t3(void);

#endif /* _KEYBOARD_H */
