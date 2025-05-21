#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include "types.h"
#include "paging.h"
#include "pcb.h"
#include "system_calls.h"
#include "multiple_terminals.h"



extern struct boot_block_t* starting_mem_ptr; // start of filesys memory

typedef struct __attribute__((packed)) dentry_t {
    uint8_t file_name[32];
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[24];
} dentry_t;

typedef struct __attribute__((packed)) boot_block_t {
    uint32_t num_dir_entries : 32;
    uint32_t num_inodes : 32;
    uint32_t num_data_blocks : 32;
    uint8_t reserved[52];
    struct dentry_t dir_entries[63];
} boot_block_t;

typedef struct __attribute__((packed)) inode_t {
    uint32_t length_in_bytes : 32;
    uint32_t data_blocks[1023];
} inode_t;


// struct file_sys_elems page_directory[1024];
// helper function
struct boot_block_t* get_starting_mem_ptr();

void file_system_init(struct boot_block_t* start_mem_pos);
// gets the name of the file, find dentry in boot_block with file name, set name of dentry
int32_t read_dentry_by_name (const uint8_t* fname, struct dentry_t* dentry);


int32_t read_dentry_by_index (uint32_t index, struct dentry_t* dentry);

// how many bytes it read
// writes data into buf
int32_t read_data (uint32_t inode, uint32_t offset, char* buf, uint32_t length);


int32_t file_open(const uint8_t* filename);

int32_t file_close(int32_t fd);

int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t dir_open(const uint8_t* filename);

int32_t dir_close(int32_t fd);

int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

#endif
