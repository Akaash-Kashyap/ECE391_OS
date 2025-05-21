#ifndef PCB_H
#define PCB_H
#include "types.h"
#include "file_system.h"
#include "rtc.h"
#include "terminal.h"
#include "lib.h"
#include "system_calls.h"

/* Function pointers for file system */
typedef struct file_operations{
    int32_t (*open) (const uint8_t* filename);
    int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close) (int32_t fd);
}file_operations;

/* File descriptor array */
typedef struct fd_array_entry{
    struct file_operations ops;   // Function pointers
    uint32_t inode;                 // Index of inode
    uint32_t file_position;         // File read offset
    uint32_t flags;                 // Active/Inactive entry
}fd_array_entry; 

/* Process Control Block */
typedef struct pcb{
    struct fd_array_entry fd_array[8];  // Active task entries
    // struct pcb * parent_pcb;            // Previous PCB pointer
    int parent_pcb_pid;
    int terminal_idx;
    uint8_t active;                     // if the current process is actively running or not
    uint32_t esp;
    uint32_t esp_halt;
    uint32_t ebp;
    uint32_t ebp_halt;
    uint32_t eflags;
    uint32_t user_ds;
    uint32_t eip;
    uint32_t cs;
    uint32_t image_start;
    uint8_t* cmd; 
}pcb;

/* Current global process ID */
extern int pid;

extern struct pcb pcb_array[6];

/* Gets the value of PID for functions outside of pcb.c */
// int get_pid(void);

// void set_pid(int num);

/* Get PCB */
// struct pcb get_pcb(void);

struct pcb get_pcb_pid(int input_pid);

void set_ebpesp(int pid_in, uint32_t ebp, uint32_t esp);
void set_ebpesp_halt(int pid_in, uint32_t ebp, uint32_t esp);

void set_pcb_open(int pid_in, int file_num, int32_t * addr);
void set_pcb_read(int pid_in, int file_num, int32_t * addr);
void set_pcb_write(int pid_in, int file_num, int32_t * addr);
void set_pcb_close(int pid_in, int file_num, int32_t * addr);
void set_pcb_inode(int pid_in, int file_num, int32_t inode_val);
void set_pcb_file_position(int pid_in, int file_num, int32_t file_position_val);
void set_pcb_flags(int pid_in, int file_num, int32_t flags_val);

void set_pcb_esp(int in_pid, uint32_t esp_val);
void set_pcb_ebp(int in_pid, uint32_t ebp_val);
void set_pcb_eflags(int in_pid, uint32_t eflags_val);
void set_pcb_user_ds(int in_pid, uint32_t user_ds_val);
void set_pcb_eip(int in_pid, uint32_t eip_val);
void set_pcb_cs(int in_pid, uint32_t cs_val);
void set_pcb_image_start(int in_pid, uint32_t image_start_val);
void set_pcb_cmd(int in_pid, uint8_t* cmd);

uint32_t get_esp(int pid_in);

uint32_t get_ebp(int pid_in);

uint32_t get_ebp_halt(int pid_in);

uint32_t get_esp_halt(int pid_in);

/* Initialize a PCB entry for a program */
uint32_t init_pcb(int term_num, int parent_pcb_val, int global_pcb_val);

/*clear pcb*/
uint32_t clear_pcb(int pid_in);

/* Change PID value */
// void pid_change(int x);


/* Copy program image into 128MB memory */
uint32_t user_level_program_loader(const uint8_t * filename);

/* Check if pcb enabled */
uint32_t pcb_valid(int pid_in, int32_t fd);

/* Depreciated Function */
char* read_file_noFD(const uint8_t* filename, uint32_t offset, uint32_t bytes_to_read);

int get_current_term();

#endif

//! Testing PCB Array
/* Global PCB array declared in kernal space in physical memory */
// static struct pcb * pcb_array[6] = {(struct pcb * ) 0x7FE000,
//                                     (struct pcb * ) 0x7FC000,
//                                     (struct pcb * ) 0x7FA000,
//                                     (struct pcb * ) 0x7F8000,
//                                     (struct pcb * ) 0x7F6000,
//                                     (struct pcb * ) 0x7F4000
//                                    };
