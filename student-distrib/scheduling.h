#include "lib.h"
#include "i8259.h"
#include "pcb.h"
#include "x86_desc.h"
#include "paging.h"
#include "system_calls.h"
#include "lib.h"


extern int pid_arr_idx;

int setup_pit();

void pit_handler();

int schedule();

void scheduling_context_switch();

int get_line_number();
