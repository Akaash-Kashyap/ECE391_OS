#ifndef PAGING_H
#define PAGING_H
#include "types.h"
#include "x86_desc.h"

// This struct is used to fill the 32-bit entry in the page directory for any 4 kB page
// The parameters/format of this struct follows the bit format in page 90 of the Intel Manual
// which implements the flowchart given in page 87 of the same manual 
typedef struct __attribute__((packed)) pd_entry_4kB { // 32 bits
    uint8_t present : 1; // 1 bit, present or not
    uint8_t read_write : 1; // 1 bit, read only or read + write
    uint8_t user_supervisor : 1; // 1 bit, who can access data
    uint8_t write_through : 1; // 1 bit, 
    uint8_t cache_disabled : 1; // 1 bit, 
    uint8_t accessed : 1; // 1 bit, 
    uint8_t reserved : 1; // 1 bit, set to 0 always (Intel convention)
    uint8_t page_size : 1; // 1 bit, set to 0 to indicate 4 kb page
    uint8_t global_page : 1; // 1 bit, set to 1 if present, 0 otherwise
    uint8_t available : 3; // 3 bits, available for programmer or not
    uint32_t pt_base_address : 20; // 20 bits, 20 MSB of page table address
} pd_entry_4kB; 

// This struct is used to fill the 32-bit entry in the page directory for any 4 MB page
// The parameters/format of this struct follows the bit format in page 91 of the Intel Manual
// which implements the flowchart given in page 88 of the same manual
typedef struct __attribute__((packed)) pd_entry_4MB { // 32 bits
    uint8_t present : 1; // 1 bit, present or not
    uint8_t read_write : 1; // 1 bit, read only or read + write
    uint8_t user_supervisor : 1; // 1 bit who can access data
    uint8_t write_through : 1; // 1 bit, 
    uint8_t cache_disabled : 1; // 1 bit, 
    uint8_t accessed : 1; // 1 bit, 
    uint8_t dirty : 1; // 1 bit, set to 0 always (Intel convention)
    uint8_t page_size : 1; // 1 bit, set to 1 to indicate 4 MB page
    uint8_t global_page : 1; // 1 bit, set to 1 if present, 0 otherwise
    uint8_t available : 3; // 3 bits, available for programmer or not
    uint8_t pat : 1; // 1 bit, page attribute table index, set to 0 always
    uint16_t reserved : 9; // 9 bits, always set to 0 (Intel convention)
    uint32_t page_base_address : 10; // 10 bits, 10 MSB of 4 MB page address
} pd_entry_4MB; 


// creates a union between the kB and MB page structs so that one array can be used with 
// both kB and MB page data
typedef struct __attribute__((packed)) pd_entry {
    union {
        struct pd_entry_4kB kB;
        struct pd_entry_4MB MB;
    } pd_entry_union;
} pd_entry;

// This struct is used to fill the 32-bit entry in the page table for any 4 kB page
// The parameters/format of this struct follows the bit format in page 90 of the Intel Manual
// which implements the flowchart given in page 87 of the same manual 
typedef struct __attribute__((packed)) pt_entry {
    uint8_t present : 1; // 1 bit, present or not
    uint8_t read_write : 1; // 1 bit, read only or read + write
    uint8_t user_supervisor : 1; // 1 bit who can access data
    uint8_t write_through : 1; // 1 bit, 
    uint8_t cache_disabled : 1; // 1 bit, 
    uint8_t accessed : 1; // 1 bit, 
    uint8_t dirty : 1; // 1 bit, set to 0 always (Intel convention)
    uint8_t pt_attribute_index : 1; // 1 bit, 
    uint8_t global_page : 1; // 1 bit, set to 1 if present, 0 otherwise
    uint8_t available : 3; // available for programmer
    uint32_t page_base_address : 20; // 3 bits, 20 MSB of page address
} pt_entry;

// function which sets up paging, including 4 kB and 4 MB pages at correct locations
extern void setup_paging(); 
extern uint32_t execute_page_setup(int phys_address_mb);
extern uint32_t setup_4kb_page(uint32_t phys_address, uint32_t va, uint32_t present_status);

// array of page directory entries. length is 1024 because there are 10 bits for the page directory number
// and 2^10 = 1024. Aligned to 4096 so that 12 LSBs are all 0.  
struct pd_entry page_directory[1024] __attribute__((aligned(4096)));

// array of page table entries. length is 1024 because there are 10 bits for the page directory number
// and 2^10 = 1024. Aligned to 4096 so that 12 LSBs are all 0 when address is stored in page directory
struct pt_entry page_table[1024] __attribute__((aligned(4096)));

struct pt_entry vid_mem_page_table[1024] __attribute__((aligned(4096)));
#endif
