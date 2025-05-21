#include "pcb.h"

/* uint32_t init_pcb()
 * DESCRIPTION: initializes the process control block for a task. Called from execute. Sets
 *              stdin and stdout to always being in use. Creates empty fd array entries and op tables
 *              for remaining 6 files. 
 * INPUTS: Nothing
 * OUTPUTS: nothing
 * SIDE EFFECTS: pcb at an address is filled
 * RETURN: 0 if pcb is initialized properly
*/
// int pid = 0;
struct pcb pcb_array[6];
uint32_t init_pcb(int term_num, int parent_pcb_val, int global_pcb_val){
 
    /* Set parent pcb */
    // int a = pid;
    // pid = a;
    if(global_pcb_val == 0){
        pcb_array[global_pcb_val].parent_pcb_pid = -1;
    }
    else{
        pcb_array[global_pcb_val].parent_pcb_pid = parent_pcb_val;
    }
    // printf("\nfinished parent node\n\n\n");
    // printf("pid: %d\n", pid);
    /* ================================= */
    /* Initialize file descriptor arrary */
    /* ================================= */

    /* Set STDIN */
    
    // printf("\nright before terminal_read");
    pcb_array[global_pcb_val].fd_array[0].ops.read = &terminal_read;
    //printf("\nright after terminal_read");
    pcb_array[global_pcb_val].fd_array[0].ops.open = NULL;
    pcb_array[global_pcb_val].fd_array[0].ops.write = NULL;
    pcb_array[global_pcb_val].fd_array[0].ops.close = NULL;
    pcb_array[global_pcb_val].fd_array[0].inode = 0;
    pcb_array[global_pcb_val].fd_array[0].file_position = 0;
    pcb_array[global_pcb_val].fd_array[0].flags = 1;

    /* Set STDOUT */
    pcb_array[global_pcb_val].fd_array[1].ops.open = NULL;
    pcb_array[global_pcb_val].fd_array[1].ops.read = NULL;
    pcb_array[global_pcb_val].fd_array[1].ops.write = &terminal_write;
    pcb_array[global_pcb_val].fd_array[1].ops.close = NULL;
    pcb_array[global_pcb_val].fd_array[1].inode = 0;
    pcb_array[global_pcb_val].fd_array[1].file_position = 0;
    pcb_array[global_pcb_val].fd_array[1].flags = 1;

    // printf("\nfinished 0 and 1 node");
    // while(1);

    /* Reset remaining flags */
    int i;
    for(i = 2; i < 8; i++){ // 8 elements in the fd_array
        pcb_array[global_pcb_val].fd_array[i].flags = 0;
    }
    pcb_array[global_pcb_val].terminal_idx = term_num;

    /* PCB intialized */
    return 0;
}

uint32_t clear_pcb(int pid_in){
 
    /* Set parent pcb */
    // int a = pid;
    // pid = a;
    // if(pid == 0){
    //     pcb_array[pid].parent_pcb = NULL;
    // }
    // else{
    //     pcb_array[pid].parent_pcb = &pcb_array[pid - 1];
    // }
    // printf("\nfinished parent node\n\n\n");
    // printf("pid: %d\n", pid);
    /* ================================= */
    /* Initialize file descriptor arrary */
    /* ================================= */

    /* Set STDIN */
    
    // printf("\nright before terminal_read");
    pcb_array[pid_in].fd_array[0].ops.read = NULL;
    //printf("\nright after terminal_read");
    pcb_array[pid_in].fd_array[0].ops.open = NULL;
    pcb_array[pid_in].fd_array[0].ops.write = NULL;
    pcb_array[pid_in].fd_array[0].ops.close = NULL;
    pcb_array[pid_in].fd_array[0].inode = 0;
    pcb_array[pid_in].fd_array[0].file_position = 0;
    pcb_array[pid_in].fd_array[0].flags = 0;

    /* Set STDOUT */
    pcb_array[pid_in].fd_array[1].ops.open = NULL;
    pcb_array[pid_in].fd_array[1].ops.read = NULL;
    pcb_array[pid_in].fd_array[1].ops.write = NULL;
    pcb_array[pid_in].fd_array[1].ops.close = NULL;
    pcb_array[pid_in].fd_array[1].inode = 0;
    pcb_array[pid_in].fd_array[1].file_position = 0;
    pcb_array[pid_in].fd_array[1].flags = 0;

    // printf("\nfinished 0 and 1 node");
    // while(1);

    /* Reset remaining flags */
    int i;
    for(i = 2; i < 8; i++){ // 8 elements in the fd_array
        pcb_array[pid_in].fd_array[i].flags = 0;
    }

    /* PCB intialized */
    return 0;
}

/* uint32_t pcb_valid()
 * DESCRIPTION: checks the file is in use by checking its flags.
 *              Flags needs to be set to 1 for a file to be in use.
 * INPUTS: file descriptor
 * OUTPUTS: signals to the caller if a file is open or not
 * SIDE EFFECTS: none
 * RETURN: 0 if file is open, -1 if its not
*/
uint32_t pcb_valid(int pid_in, int32_t fd) {
    if (pcb_array[pid_in].fd_array[fd].flags) {
        return 0;
    }
    return -1;
}

/* uint32_t user_level_program_loader(const uint8_t * filename)
 * DESCRIPTION: loads an executable into memory
 * INPUTS: file name of executable
 * OUTPUTS: copies bytes of program into memory
 * SIDE EFFECTS: writes to memory of where program begins execution
 * RETURN: 0 
*/
uint32_t user_level_program_loader(const uint8_t * filename){

    // create a dentry
    struct dentry_t program_to_load;
    
    read_dentry_by_name(filename, &program_to_load);
    // while(1);
    uint32_t program_inode;
    uint32_t length;
    program_inode = program_to_load.inode_num;
    length = 200000;
    char buf[length]; // if this doesnt work just change length to be a very large number
    uint32_t bytes_read;
    //buf contains all program bytes
    bytes_read = read_data(program_inode, 0, buf, length);
    /*Copy program into 0x08000000*/
    //!correct offset? Is this 0x08000000 or 0x08048000?
    /*0x800000 + 0x48000;*/
    uint32_t * dest = (uint32_t *) 0x08048000; // start of the program image
    int i;
    memcpy(dest,buf,bytes_read);
    
    uint32_t* file_address;
    file_address = dest;
    uint8_t char_at_address;  
    for (i = 0; i < 28; i++) { // prints first 28 bytes
        char_at_address = *file_address;
        //printf("%x ", char_at_address);
        file_address++;
    }

    return 0;
}

/*
 * get_pid
 * Description: Gets the value of the PID static variable for files outside of pcb.c
 * Inputs: None
 * Outputs: int (returns the value of the PID static variable)
 * Side Effects: None
 */
// int get_pid(void) {
//     return pid;
// }

// void set_pid(int num){
//     pid = num;

// }

/*
 * pid_change
 * Description: increments pid 
 * Inputs: x, tells how much you want to increment pid by
 * Outputs: int (returns the value of the PID static variable)
 * Side Effects: writes to pid
 */
// void pid_change(int x){
//     pid += x;
// }

void set_pcb_open(int pid_in, int file_num, int32_t * addr) {
    pcb_array[pid_in].fd_array[file_num].ops.open = (int32_t (*)(const uint8_t*))addr;
}

void set_pcb_read(int pid_in, int file_num, int32_t * addr) {
    pcb_array[pid_in].fd_array[file_num].ops.read = (int32_t (*)(int32_t fd, void* buf, int32_t nbytes))addr;
}

void set_pcb_write(int pid_in, int file_num, int32_t * addr) {
    pcb_array[pid_in].fd_array[file_num].ops.write = (int32_t (*)(int32_t fd, const void* buf, int32_t nbytes))addr;
}

void set_pcb_close(int pid_in, int file_num, int32_t * addr) {
    pcb_array[pid_in].fd_array[file_num].ops.close =(int32_t (*)(int32_t fd)) addr;
}

void set_pcb_inode(int pid_in, int file_num, int32_t inode_val) {
    pcb_array[pid_in].fd_array[file_num].inode = inode_val;
}

void set_pcb_file_position(int pid_in, int file_num, int32_t file_position_val) {
    pcb_array[pid_in].fd_array[file_num].file_position = file_position_val;
}

void set_pcb_flags(int pid_in, int file_num, int32_t flags_val) {
    pcb_array[pid_in].fd_array[file_num].flags = flags_val;
}



void set_pcb_esp(int in_pid, uint32_t esp_val) {
    pcb_array[in_pid].esp = esp_val;
}
void set_pcb_ebp(int in_pid, uint32_t ebp_val) {
    pcb_array[in_pid].ebp = ebp_val;
}
void set_pcb_eflags(int in_pid, uint32_t eflags_val) {
    pcb_array[in_pid].eflags = eflags_val;
}
void set_pcb_user_ds(int in_pid, uint32_t user_ds_val) {
    pcb_array[in_pid].user_ds = user_ds_val;
}
void set_pcb_eip(int in_pid, uint32_t eip_val) {
    pcb_array[in_pid].eip = eip_val;
}
void set_pcb_cs(int in_pid, uint32_t cs_val) {
    pcb_array[in_pid].cs = cs_val;
}
void set_pcb_image_start(int in_pid, uint32_t image_start_val) {
    pcb_array[in_pid].image_start = image_start_val;
}
void set_pcb_cmd(int in_pid, uint8_t* cmd) {
    // int i;
    // for (i = 0; i < 128; i++) {
        pcb_array[in_pid].cmd = cmd;
    // }
}

/*
 * set_ebpesp
 * Description: set the esp and ebp fields of the pcb to values passed in
 * Inputs: ebp, esp passed in by caller
 * Outputs: none
 * Side Effects: writes to esp and ebp fields of current pcb
 */

void set_ebpesp(int pid_in, uint32_t ebp, uint32_t esp){
    pcb_array[pid_in].esp = esp;
    pcb_array[pid_in].ebp = ebp;
}

void set_ebpesp_halt(int pid_in, uint32_t ebp, uint32_t esp){
    pcb_array[pid_in].esp_halt = esp;
    pcb_array[pid_in].ebp_halt = ebp;
}

/*
 * get_ebp
 * Description: returns the base pointer of the current process
 * Inputs: nothing
 * Outputs: base pointer of current process
 * Side Effects: none
 */
uint32_t get_ebp(int pid_in){
    return pcb_array[pid_in].ebp;
}

/*
 * get_esp
 * Description: returns the stack pointer of the current process
 * Inputs: nothing
 * Outputs: stack pointer of current process
 * Side Effects: none
 */
uint32_t get_esp(int pid_in){
    return pcb_array[pid_in].esp;
}


uint32_t get_ebp_halt(int pid_in){
    return pcb_array[pid_in].ebp_halt;
}

uint32_t get_esp_halt(int pid_in){
    return pcb_array[pid_in].esp_halt;
}

/*
 * get_pcb
 * Description: returns the pcb for the current process
 * Inputs: nothing
 * Outputs: the current pcb
 * Side Effects: calls get_pid (pid is read)
 */
// struct pcb get_pcb(void){
//     return pcb_array[get_pid()];
// }

struct pcb get_pcb_pid(int input_pid) {
    return pcb_array[input_pid];
}

int get_current_term() {
    return pcb_array[get_global_pid()].terminal_idx;
}

// -------------------------------------------------------------------------------------------------------

// //!need to pass in the buffer from outside function context, buf gets 
// //! destroyed once function returns
// char* read_file_noFD(const uint8_t* filename, uint32_t offset, uint32_t bytes_to_read){
//     struct dentry_t program_to_load;

    
//     read_dentry_by_name(filename, &program_to_load);
//     uint32_t program_inode;
//     uint32_t length;
//     program_inode = program_to_load.inode_num;
//     struct inode_t* inode_address = (inode_t*)starting_mem_ptr + (program_inode + 1);
//     struct inode_t curr_inode = *inode_address;
//     length = curr_inode.length_in_bytes;
//     if(bytes_to_read != -1)
//         length = bytes_to_read;
//     char buf[length]; // if this doesnt work just change length to be a very large number
//     uint32_t bytes_read;

//     //buf contains all program bytes
//     bytes_read = read_data(program_inode, offset, buf, length);
//     return buf;
// }














