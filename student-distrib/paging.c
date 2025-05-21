#include "paging.h"
#define page_directory_size 1024
#define page_table_size 1024

/*
    the contents of page_directory and the page table need to be specific values
    depending on the flags https://wiki.osdev.org/Setting_Up_Paging read the
    comments on the code, go to intel manual and see what flags there are and
    read the mp3 doc on what flags we need.

    intel manual important pages
    General paging info  84, 85

    TLBs 103,406
    Table 3-3 86
    4kbyte pages 87 
    4mbyte pages 88
    Control Registers info 52



*/

/* void setup_paging()
 * DESCRIPTION: Sets up the page directory. Fills first 4 MB of virtual memory with 4 kB pages
 *              and fills the rest of the virtual memory with 4 MB pages. Sets up the page table
 *              and fills it with address of 4 kB page. Sets 4 MB - 8 MB (kernel memory) as present  
 *              and global. Sets the CR3 register to the starting address of the page directory. 
 *              Enables paging.
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS: Pages all of virtual memory. CR0, CR3, and CR4 registers are changed in load_page_directory
 *               and enable_paging functions. Paging is enabled.
*/
void setup_paging()
{
    /*
     * Sets up the page directory and fills it with page tables that
     * are also filled up 
    */

    // Template pt_entry
    // Initializes present, user/supervisor, write through, cache disabled, accessesed, dirty, 
    // page table attribute index, global page, available, and page table base address to 0
    // Initializes read/write to 1
    // Used as default values for the page table
    struct pt_entry template_pt_entry = {0,1,0,0,0,0,0,0,0,0,0}; 

    // loop counters
    int i;
    int j;

    // loops through all pages that are not present in physical memory from virtual memory
    // sets values to "not present". Read/write is set to 1, page_size is set to 1 to specify 4 MB pages
    // page_base address is set to loop index because next 22 bits in the page starting address are 0, so 
    // page base address bit 22 is incremented by 1 for each page. 
    // all other values are set to 0
    for(i = 2; i < page_directory_size; i++)
    {
        page_directory[i].pd_entry_union.MB.present = 0;
        page_directory[i].pd_entry_union.MB.read_write = 1;
        page_directory[i].pd_entry_union.MB.user_supervisor = 0;
        page_directory[i].pd_entry_union.MB.write_through = 0;
        page_directory[i].pd_entry_union.MB.cache_disabled = 0;
        page_directory[i].pd_entry_union.MB.accessed = 0;
        page_directory[i].pd_entry_union.MB.dirty = 0;
        page_directory[i].pd_entry_union.MB.page_size = 1;
        page_directory[i].pd_entry_union.MB.global_page = 0;
        page_directory[i].pd_entry_union.MB.available = 0;
        page_directory[i].pd_entry_union.MB.pat = 0;
        page_directory[i].pd_entry_union.MB.reserved = 0;
        page_directory[i].pd_entry_union.MB.page_base_address = i;
    }
    
    // fills in first 4 MB in virtual memory
    // sets values to "present". Read/write is set to 1, page_size is set to 0 to specify 4 kB pages
    // page_base address is set to 0 because this is the first page
    // global_page is set to 1 because it is present
    // all other values are set to 0

    page_directory[0].pd_entry_union.kB.present = 1;
    page_directory[0].pd_entry_union.kB.read_write = 1;
    page_directory[0].pd_entry_union.kB.user_supervisor = 0;
    page_directory[0].pd_entry_union.kB.write_through = 0;
    page_directory[0].pd_entry_union.kB.cache_disabled = 0;
    page_directory[0].pd_entry_union.kB.accessed = 0;
    page_directory[0].pd_entry_union.kB.reserved = 0;
    page_directory[0].pd_entry_union.kB.page_size = 0;
    page_directory[0].pd_entry_union.kB.global_page = 1;
    page_directory[0].pd_entry_union.kB.available = 0;

    // This loop sets all page table entries to the default mapping
    // Base address in the page table should increment by 1 for each page, because bit 12
    // increments by 1 for each 4096 element page.
    for(j = 0; j < page_table_size; j++){
        page_table[j] = template_pt_entry;
        page_table[j].page_base_address = j;
        // 4 kB page is present in physical memory ONLY IF it is the video memory
        // 184 because B8000 >> 12 is 184. 
        //for other pages used 192, 194, 196
        // if (j == 184 || j == 192 || j == 194 || j == 196) {
        //     page_table[j].present = 1;
        // }
        if (j == 184 || j == 185 || j == 186 || j == 187) {
            page_table[j].present = 1;
        }
    }
    // sets the address of the page table to the page table address shifted to the right by 12
    // 12 because the 12 LSBs are all 0, so when looping though page table, address will be shifted
    // left 12 bits again
    page_directory[0].pd_entry_union.kB.pt_base_address = ((uint32_t)page_table) >> 12;

    // fills in 4 MB - 8 MB in virtual memory
    // sets values to "present". Read/write is set to 1, page_size is set to 1 to specify 4 MB pages
    // page_base address is set to 1 because this is the second page
    // global_page is set to 1 because it is present
    // all other values are set to 0
    page_directory[1].pd_entry_union.MB.present = 1;
    page_directory[1].pd_entry_union.MB.read_write = 1;
    page_directory[1].pd_entry_union.MB.user_supervisor = 0;
    page_directory[1].pd_entry_union.MB.write_through = 0;
    page_directory[1].pd_entry_union.MB.cache_disabled = 0;
    page_directory[1].pd_entry_union.MB.accessed = 0;
    page_directory[1].pd_entry_union.MB.dirty = 0;
    page_directory[1].pd_entry_union.MB.page_size = 1;
    page_directory[1].pd_entry_union.MB.global_page = 1;
    page_directory[1].pd_entry_union.MB.available = 0;
    page_directory[1].pd_entry_union.MB.pat = 0;
    page_directory[1].pd_entry_union.MB.reserved = 0;
    page_directory[1].pd_entry_union.MB.page_base_address = 1;
    

    // calls function to set CR3 register (holds starting address of page directory) to the starting 
    // address of the page directory just created
    load_page_directory(page_directory);
    
    // calls function to enable paging
    enable_paging();

}


/* void execute_setup_paging()
 * DESCRIPTION: Sets up the page directory. Fills first 4 MB of virtual memory with 4 kB pages
 *              and fills the rest of the virtual memory with 4 MB pages. Sets up the page table
 *              and fills it with address of 4 kB page. Sets 4 MB - 8 MB (kernel memory) as present  
 *              and global. Sets the CR3 register to the starting address of the page directory. 
 *              Enables paging.
 *              Also maps a 4mb page at 128 mb to point to the input phys_address_mb
 * INPUTS: int phys_address_mb, the physical address to map 128 mb virtual address to 
 * OUTPUTS: None
 * SIDE EFFECTS: Pages all of virtual memory. CR0, CR3, and CR4 registers are changed in load_page_directory
 *               and enable_paging functions. Paging is enabled.
 *               Should also flush the TLB  since CR3 is changed.
 * RETURN: 1 if success, -1 if failure from incorrect input.
*/
uint32_t execute_page_setup(int phys_address_mb){
    // phys_address of program (where it is loaded)

    //input validation
    if((phys_address_mb % 4) != 0){
        return -1;
    }
    if(phys_address_mb == 0){
        return -1;
    }
    if(phys_address_mb == 4){
        return -1;
    }

    /*
     * Sets up the page directory and fills it with page tables that
     * are also filled up 
    */

    // start setting up the execute location in VA
    page_directory[32].pd_entry_union.MB.present = 1;
    page_directory[32].pd_entry_union.MB.read_write = 1;
    page_directory[32].pd_entry_union.MB.user_supervisor = 1; // should be set high for privilege level
    page_directory[32].pd_entry_union.MB.write_through = 0;
    page_directory[32].pd_entry_union.MB.cache_disabled = 0;
    page_directory[32].pd_entry_union.MB.accessed = 0;
    page_directory[32].pd_entry_union.MB.dirty = 0;
    page_directory[32].pd_entry_union.MB.page_size = 1;
    page_directory[32].pd_entry_union.MB.global_page = 0;
    page_directory[32].pd_entry_union.MB.available = 0;
    page_directory[32].pd_entry_union.MB.pat = 0;
    page_directory[32].pd_entry_union.MB.reserved = 0;
    page_directory[32].pd_entry_union.MB.page_base_address = phys_address_mb/4; // change this to be phys memory

    // calls function to set CR3 register (holds starting address of page directory) to the starting 
    // address of the page directory just created
    load_page_directory(page_directory);
    
    /*asm volatile(
        "movl %0, %%eax     \n\t"           \
        "movl %%eax, %%cr3  \n\t"           \
        :                                   \
        : "r" (&page_directory)             \
        : "%eax"                            \
    );*/

    // calls function to enable paging
    // enable_paging();
    return 1;   // success
}

/* uint32_t setup_4kb_page(uint32_t phys_address, uint32_t va, uint32_t present_status)
 * DESCRIPTION: sets up a 4kb page in virtual memory to point to physical video memory.
 * INPUTS: uint32_t phys_address, the physical address of video memory, 
 *         uint32_t va, virtual adress that is being paged
 *         uint32_t present_status, 0 or 1 if we are turning off or on this page
 * OUTPUTS: None
 * SIDE EFFECTS: pages a virtual address passed in by caller
 * RETURN: 1 if success, -1 if failure from incorrect input.
*/
uint32_t setup_4kb_page(uint32_t phys_address, uint32_t va, uint32_t present_status){
    // if((phys_address % 4) != 0){
    //     return -1;
    // }
    if(phys_address == 0){
        return -1;
    }
    //set up page directory entry

    /*22 = isolate the 10 MSB of virtual address to get index into pd*/
    uint32_t page_directory_index = (va)>>22;
    page_directory[page_directory_index].pd_entry_union.kB.present = present_status;
    page_directory[page_directory_index].pd_entry_union.kB.read_write = 1;
    page_directory[page_directory_index].pd_entry_union.kB.user_supervisor = 1; //!DPL????
    page_directory[page_directory_index].pd_entry_union.kB.write_through = 0;
    page_directory[page_directory_index].pd_entry_union.kB.cache_disabled = 0;
    page_directory[page_directory_index].pd_entry_union.kB.accessed = 0;
    page_directory[page_directory_index].pd_entry_union.kB.reserved = 0;
    page_directory[page_directory_index].pd_entry_union.kB.page_size = 0; // Set to 0 for 4 KB page
    page_directory[page_directory_index].pd_entry_union.kB.global_page = 1;
    page_directory[page_directory_index].pd_entry_union.kB.available = 0;

    /*shift out bottom 12 bits to get 20 msb of physical address of page table.*/
    page_directory[page_directory_index].pd_entry_union.kB.pt_base_address = ((uint32_t)vid_mem_page_table) >> 12;

    // Set up a page table entry
    /*12, 0x3FF: isolate middle 10 bits to get index into page table*/
    uint32_t page_table_index = (va >> 12) & 0x3FF;
    vid_mem_page_table[page_table_index].present = present_status;
    vid_mem_page_table[page_table_index].read_write = 1;
    vid_mem_page_table[page_table_index].user_supervisor = 1; //!DPL???
    vid_mem_page_table[page_table_index].write_through = 0;
    vid_mem_page_table[page_table_index].cache_disabled = 0;
    vid_mem_page_table[page_table_index].accessed = 0;
    vid_mem_page_table[page_table_index].dirty = 0;
    vid_mem_page_table[page_table_index].pt_attribute_index = 0;
    vid_mem_page_table[page_table_index].global_page = 1;
    vid_mem_page_table[page_table_index].available = 0;
    /*shift out bottom 12 bits to get 20 msb of physical address*/
    vid_mem_page_table[page_table_index].page_base_address = phys_address >> 12;


    return 0;
}

