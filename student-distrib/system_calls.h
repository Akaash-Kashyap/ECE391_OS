#ifndef SYSTEM_CALLS_H
#define SYSTEM_CALLS_H
#include "pcb.h"
#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "keyboard.h"
#include "i8259.h"
#include "multiple_terminals.h"


// Called by user
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);


// Called by kernel
extern int32_t sys_call_halt(uint8_t status);
extern int32_t sys_call_execute(const uint8_t* command);
extern int32_t sys_call_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_call_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_call_open(const uint8_t* filename);
extern int32_t sys_call_close(int32_t fd);
extern int32_t sys_call_get_args(uint8_t* buf, int32_t nbytes);
extern int32_t sys_call_vidmap(uint8_t** screen_start);
extern int32_t sys_call_sethandler(int32_t signum, void* handler_address);
extern int32_t sys_call_sigreturn(void);

void set_global_pid(int val);
int get_global_pid();
void set_tss_ss0(int ss0);
void set_tss_esp0(int esp0);
#endif 
