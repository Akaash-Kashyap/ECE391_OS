#include "system_calls.h"
// #include "i8259.h"
// #define VIDEO_MEM   0xB8000
// #define VIDMAP_VA   0xF0000000

static int num_times_run = 0;
static char argument_passed[128];
static int global_pid = -1;
static int prev_global_pid = -1;
 /* int32_t sys_call_read()
 * DESCRIPTION: reads from a particular device based on file descriptor.
 * INPUTS: file descriptor, buffer that needs to be filled up, and how many bytes that need to be read.
 * OUTPUTS: nothing
 * SIDE EFFECTS: reads from a device. The read position is updated for the fd array entry. 
 * RETURN: the number of bytes read
*/
int32_t sys_call_read (int32_t fd, void* buf, int32_t nbytes){

    struct pcb pcb_val;

    sti();
    /* Input validation */
    if(nbytes < 0)  return -1; 
    if(fd < 0)      return -1;
    if(buf == 0)    return -1;
    if(pcb_valid(global_pid, fd) == -1) return -1;

    // printf("\nchecked args");
   
    /* Read from the file*/
    pcb_val = get_pcb_pid(global_pid);
    uint32_t num_bytes_read;
    if (fd == 0) {
        num_bytes_read = terminal_read(fd, buf, nbytes);
    }
    else if (pcb_val.fd_array[fd].ops.read == &rtc_read) {
        num_bytes_read = rtc_read(fd, buf, nbytes);
    }
    else if (pcb_val.fd_array[fd].ops.read == &dir_read) {
        num_bytes_read = dir_read(fd, buf, nbytes);
        if (num_bytes_read > 0) {
            set_pcb_file_position(global_pid, fd, pcb_val.fd_array[fd].file_position + 32);
        }
    }
    else {
        num_bytes_read = file_read(fd, buf, nbytes);
        set_pcb_file_position(global_pid, fd, pcb_val.fd_array[fd].file_position + num_bytes_read);
    }

    /* Done file read */
    return num_bytes_read;
}

/* int32_t sys_call_write()
 * DESCRIPTION: writes to a device based on a user passed buffer. Decides whether to 
 *              write to rtc, directory, or file based on fd.
 * INPUTS: file descriptor, buffer that needs to be written, and how many bytes that need to be written.
 * OUTPUTS: a file is written to
 * SIDE EFFECTS: writes to a device
 * RETURN: the number of bytes written
*/
int32_t sys_call_write (int32_t fd, const void* buf, int32_t nbytes){

    sti();

    //input validation
    if(nbytes < 0) return -1; 
    if(fd < 0) return -1;
    if(buf==0) return -1;
    if(pcb_valid(global_pid, fd) == -1) return -1;

    uint32_t num_bytes_written;
    if (fd == 1) {
        num_bytes_written = terminal_write(fd, buf, nbytes);
    }
    else if (get_pcb_pid(global_pid).fd_array[fd].ops.write == &rtc_write) {
        num_bytes_written = rtc_write(fd, buf, nbytes);
    }
    else if (get_pcb_pid(global_pid).fd_array[fd].ops.write == &dir_write) {
        num_bytes_written = dir_write(fd, buf, nbytes);
    }
    else {
        num_bytes_written = file_write(fd, buf, nbytes);
    }

    /* Write to file */
    if(num_bytes_written != nbytes) return -1;

    /* Done writing */
    return 0;
}

/* int32_t sys_call_open()
 * DESCRIPTION: opens a file. Sets the ops table field of the fd array entry to correspond to 
                the respective device's operations. Sets the status of the file to be in use.
                Sets inode based on the dentry of the file. Sets file position to 0. 
 * INPUTS: filename string
 * OUTPUTS: return value from particular device's open function
 * SIDE EFFECTS: sets all entries of a particular fd array entry
 * RETURN: return value from particular file's open function
*/
int32_t sys_call_open (const uint8_t* filename){

    sti();

    /* Local varaibles */
    struct dentry_t dentry;
    uint32_t cur_file_open = -1;

    /* Input validation */
    if(read_dentry_by_name(filename, &dentry) == -1) return -1;

    /* Get empty file descriptor entry */
    int i;
    /*magic numbers:
        2: first index of non stdin and stdout file 
        8: we want to loop through index 7*/
    for(i = 2; i < 8; i++){
        if(!get_pcb_pid(global_pid).fd_array[i].flags){
            cur_file_open = i;
            break;
        }
    }

    /* 6 files are open already, cannot open more */
    if (cur_file_open == -1) {
        return -1;
    }

    /* Set file operations */
    switch (dentry.file_type){
        case (0):
            //! Fix RTC
            // Sets read, write, open, close, and inode num
            set_pcb_open(global_pid, cur_file_open,(int32_t *) &rtc_open);
            set_pcb_read(global_pid, cur_file_open,(int32_t *) &rtc_read);
            set_pcb_write(global_pid, cur_file_open,(int32_t *) &rtc_write);
            set_pcb_close(global_pid, cur_file_open,(int32_t *) &rtc_close);
            set_pcb_inode(global_pid, cur_file_open, dentry.inode_num);
            break;

        case (1):
            // Sets read, write, open, close, and inode num
            set_pcb_open(global_pid, cur_file_open,(int32_t *) &dir_open);
            set_pcb_read(global_pid, cur_file_open,(int32_t *) &dir_read);
            set_pcb_write(global_pid, cur_file_open,(int32_t *) &dir_write);
            set_pcb_close(global_pid, cur_file_open,(int32_t *) &dir_close);
            set_pcb_inode(global_pid, cur_file_open, 0);
            break;

        case (2):
            // Sets read, write, open, close, and inode num
            set_pcb_open(global_pid, cur_file_open, (int32_t *) &file_open);
            set_pcb_read(global_pid, cur_file_open, (int32_t *) &file_read);
            set_pcb_write(global_pid, cur_file_open, (int32_t *) &file_write);
            set_pcb_close(global_pid, cur_file_open, (int32_t *) &file_close);
            set_pcb_inode(global_pid, cur_file_open, dentry.inode_num);
            break;
    }

    /* Set generic PCB */
    set_pcb_file_position(global_pid, cur_file_open, 0);
    set_pcb_flags(global_pid, cur_file_open, 1);

    /* Open file, opens any file */
    get_pcb_pid(global_pid).fd_array[cur_file_open].ops.open(filename);
    return cur_file_open;
}

/* int32_t sys_call_close()
 * DESCRIPTION: closes a file by setting its in use status to 0. Resets other status variables inside fd array element.
                Resets all the function pointer in the operations table of the fd array entry.
 * INPUTS: file descriptor (fd)
 * OUTPUTS: the return value from the particular file's close function
 * SIDE EFFECTS: fields of fd array entry completely reset 
 * RETURN: return value from particular file's close function
*/
int32_t sys_call_close (int32_t fd){

    // sti();
    //printf("\nsys_call_close");

    /* Input validation */
    /* magic number 8: fd cannot be more than 7*/
    if(fd <= 1 || fd >= 8) return -1; // there are 8 elements in the fd array
    if(pcb_valid(global_pid, fd) == -1) return -1;

    /* Close files */
    int32_t retval = get_pcb_pid(global_pid).fd_array[fd].ops.close(fd);

    /* Reset file flags to close file */
    get_pcb_pid(global_pid).fd_array[fd].inode = 0;
    get_pcb_pid(global_pid).fd_array[fd].flags = 0;
    get_pcb_pid(global_pid).fd_array[fd].file_position = 0;

    /* Reset file operations, except close */
    get_pcb_pid(global_pid).fd_array[fd].ops.open = NULL;
    get_pcb_pid(global_pid).fd_array[fd].ops.close = NULL;
    get_pcb_pid(global_pid).fd_array[fd].ops.read = NULL;
    get_pcb_pid(global_pid).fd_array[fd].ops.write = NULL;
    
    /* Done closing */
    return retval;
}

/* int32_t sys_call_execute()
 * DESCRIPTION: takes in command, parses it, and then loads executable into memory to begin execution of program.
 *              Reads from files specified in command. Context of task is pushed onto the stack.
 * INPUTS: command, which is a string
 * OUTPUTS: prints to the screen,
 * SIDE EFFECTS: program executable memory address if overwritten. pid incremented.
 * RETURN: -1 if failure to read command
*/
int32_t sys_call_execute (const uint8_t* command){

    //printf("\nsys_call_execute");
    // cli();
    sti();
    num_times_run++;
    // if(num_times_run == 1){

    // }
    /* Check if NULL command */
     if(command == 0){
        return -1;
    }

    // /* Extract command (space) from text line */
    uint32_t i = 0;
    uint32_t space_index = strlen((int8_t*)command);
    for(i = 0; i < strlen((int8_t*)command); i++){
        /*magic number 32 = ascii value of space key*/
        if(command[i] == 32){ // 32 represents space
            space_index = i;
            break;
        }
    }

    //create buffers to hold the command and the command argument/
    int cmd_arg_len = 1;

    //there is a space followed by an argument/
    if(space_index < strlen((int8_t*)command)){
        cmd_arg_len = strlen((int8_t*)command) - space_index;
    }
    char cmd[128];
    for (i = 0; i < 128; i++) {
        cmd[i] = 0;
    }
    if (space_index == strlen((int8_t*)command)) {
        for(i = 0; i < space_index + 1; i++){
            cmd[i] = command[i];
        }
    }
    else {
        for(i = 0; i < space_index; i++){
            cmd[i] = command[i];
        }
    }

    char cmd_arg[128];
    for(i = 0; i < 128; i++) {
            argument_passed[i] = 0;
    }

    /* Parse user input into command and command args */

    if (!(space_index == strlen((int8_t*)command))) {
        for (i = 0; i < 128; i++) {
            cmd_arg[i] = 0;
        }
        for(i = space_index + 1; i < strlen((int8_t*)command); i++){
            cmd_arg[i - space_index - 1] = command[i];
        }
        for(i = 0; i < strlen((int8_t*)cmd_arg); i++) {
            argument_passed[i] = cmd_arg[i];
        }
    }

    /* Read first four bytes of executable */
    /*magic number 4 = number of bytes we want to read*/
    struct dentry_t dentry;
    if(read_dentry_by_name((uint8_t*)cmd, &dentry) == -1) return -1;
    char first_four[4]; // gets the first 4 bytes to see if the file is an executable
    if(read_data(dentry.inode_num, 0, first_four, 4) != 4) return -1;

    /* Magic numbers that tell you if file is executable */
    /* 0x7F, 0x45, 0x4C, 0x46 */
    if((int) first_four[0] != 0x7F){
        return -1;
    } 
    if((int)first_four[1] != 0x45){
        return -1;
    } 
    if((int)first_four[2] != 0x4C){
        return -1;
    }
    if((int)first_four[3] != 0x46){
        return -1;
    }       

    /* Read program starting address from file */
    /*magic numbers:
     * 4: we want 4 bytes
     * 24: want to read starting from byte 24 */
    char execaddr[4];
    if(read_data(dentry.inode_num, 24, execaddr, 4) != 4) {
        printf("could not get bytes 24-27\n");
        return -1;
    }

    /* Increment PID, if not base shell (PID = 0) */

    int lowest_null_index;

    for (i = 0; i < 6; i++) {
        if (get_pcb_pid(i).fd_array[0].flags == 0) {
            lowest_null_index = i;
            break;
        }
    }

    prev_global_pid = global_pid;
    global_pid = lowest_null_index;

    /*initialize pcb*/
    int curr_terminal_pcb;
    curr_terminal_pcb = get_terminal_array_entry(get_term_num() - 1);
    init_pcb(get_term_num() - 1, curr_terminal_pcb, global_pid);     // INIT basic PCB
    set_pcb_cmd(global_pid, (uint8_t*) cmd);
    // set_pcb_ebp(global_pid, 0x800000 - (global_pid * 0x2000));

    set_terminal_array_entry(get_term_num() - 1, global_pid);

    /* Unpack starting address from file */
    /* Known starting addresses: 0x080482E8 Shell, 0x08048248 LS */
    // 0xFF because this masks the byte being read
    uint32_t image_start = (execaddr[0] & 0x0FF) + ((execaddr[1] & 0x0FF)<<8) + ((execaddr[2] & 0x0FF)<<16) + ((execaddr[3] & 0x0FF)<<24);
    // if (num_times_run == 2) {
    //     image_start = 0x8048248;
    // }
    set_pcb_image_start(global_pid, image_start);

    /*4 = 4 mb per page, 8 = starting 8 mb physical address*/
    int phys_address_mb = (global_pid * 4) + 8;
    // if (num_times_run == 2) {
    //     phys_address_mb = 8;
    // }
    execute_page_setup(phys_address_mb); // sets up the page + VA and PA mapping
    user_level_program_loader((uint8_t*) cmd); // copies the file to the given VA
    


    /* Task Switch */
    tss.ss0 = KERNEL_DS;
    /*magic numbers:
     * 0x800000: 8MB, virtual adress of bottom of kernel page
     * 0x2000: 8 kb, the amount of space per task kernel stack*/
    tss.esp0 = 0x800000 - (global_pid * 0x2000) - 4;//- 4;

    uint32_t sav_ebp;
    uint32_t sav_esp;

    asm volatile(
        "mov %%ebp, %0            ;\n"
        "mov %%esp, %1            ;\n"
        :"=g" (sav_ebp), "=g" (sav_esp)
        :
        :"memory", "cc"
    );

    set_ebpesp_halt(global_pid, sav_ebp, sav_esp);
    set_ebpesp(global_pid, sav_ebp, sav_esp); 


    // sti();
    // cli();
    asm volatile(
            "pushl %0                     ;\n"
            "pushl %3                     ;\n"
            "pushfl                       ;\n"
            "popl %%eax                       ;\n"
            "orl $0x200, %%eax                       ;\n"
            "pushl %%eax                       ;\n"
            "pushl %1                     ;\n"
            "pushl %2                     ;\n"
            "iret                         ;\n"
            : 
            :"g" (0x0000002B),"g" (0x00000023),"g" (image_start), "g" (0x8400000 - 4)
            :"memory"                   
    );

    // 0x0000002B because this is the value of USER_DS
    // 0x00000023 because this is the value of USER_CS
    // 0x8400000 is the starting address of the 128-132MB page stack. 
    // 4 because there are 4 bytes for each address in memory, so this goes to the previous address before the start of the stack
    //sti();
    //printf("no page fault");
asm("returnhere:");
    /* Turn off FD's */
    /*magic numbers:
     *2 = index of first file after stdin and stdout
     *8 = loop through index 7 */

    // for(i = 2; i < 8; i++){
    //     get_pcb_pid(global_pid).fd_array[i].flags = 0;
    // }


    return 0;
    
}

/* int32_t sys_call_halt()
 * DESCRIPTION: takes in command, parses it, and then loads executable into memory to begin execution of program.
 *              Reads from files specified in command. Context of task is pushed onto the stack.
 * INPUTS: command, which is a string
 * OUTPUTS: prints to the screen,
 * SIDE EFFECTS: program executable memory address if overwritten. pid incremented.
 * RETURN: -1 if failure to read command
*/
int32_t sys_call_halt(uint8_t status) {

    cli();

    int parent_pcb_val, term_number, child_pcb_val;

    if(get_pcb_pid(global_pid).parent_pcb_pid < 0){
        clear_pcb(global_pid);
        const uint8_t shell[] = "shell";
        sys_call_execute(shell);
    }

    child_pcb_val = global_pid;
    parent_pcb_val = get_pcb_pid(global_pid).parent_pcb_pid;
    term_number = get_pcb_pid(global_pid).terminal_idx;

    clear_pcb(global_pid);
    set_terminal_array_entry(term_number, parent_pcb_val);
  
    /* Close FD entires */
    int i;
    /*Magic number 8 = total number of files in fd array*/
    for(i = 0; i < 8; i++) {
        set_pcb_flags(global_pid, i, 0);
    }

    global_pid = parent_pcb_val;
  
    /* Decrement PID, unless base */
    
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - (global_pid * 0x2000);
    // 0x800000 is the value of 8 MB in memory, and 0x2000 is 8 kB

    /* Restore page and delete PCB */
    /*Magic numbers:
     * 4 = 4 mb
     * 8 = 8 mb*/
    int phys_address_mb = (global_pid * 4) + 8;
    execute_page_setup(phys_address_mb); // sets up the page + VA and PA mapping

    //!TEST
    //setup_4kb_page(0xb8000, 0x0F0000000, 0);
    if (get_term_num() == 1) {
        setup_4kb_page(0xb9000, 0x0F0000000, 0);
    }
    else if (get_term_num() == 2) {
        setup_4kb_page(0xbA000, 0x0F0000000, 0);
    }
    else if (get_term_num() == 3) {
        setup_4kb_page(0xbB000, 0x0F0000000, 0);
    }
        // Jump to execute table

    // goto execute_done;
    uint32_t sav_ebp, sav_esp;
    sav_ebp = get_ebp_halt(child_pcb_val);
    sav_esp = get_esp_halt(child_pcb_val);

    // sav_ebp = 0x7fffcc;
    // sav_esp = 0x7ffcb4;

    sti();

    asm volatile(
        "movl %1, %%esp      ;\n"
        "movl %0, %%ebp      ;\n"
        :
        :"r"(sav_ebp), "r"(sav_esp)
        :"memory" 
    );

    asm("jmp returnhere");

    return 0;
}

/* int32_t sys_call_get_args (uint8_t* buf, int32_t nbytes)
 * DESCRIPTION: copies current argument into a buffer
 * INPUTS: uint8_t* buf, an empty buffer
 *          uint32_t nbytes, number of bytes to be copied
 * OUTPUTS: none
 * SIDE EFFECTS: argument_passed is read, buffer is written to 
 * RETURN: -1 if null buffer passed, 0 if successful
*/
int32_t sys_call_get_args (uint8_t* buf, int32_t nbytes){
    int i;

    // return error if buf is NULL
    if(buf == 0) return -1; 
    if ((strlen(argument_passed) == 0) || strlen(argument_passed) > 32) {
        return -1;
    } 

    // fills the buffer with the argument
    for (i = 0; i < strlen(argument_passed); i++) {
        buf[i] = argument_passed[i];
    }

    // NULL terminating the buffer
    buf[strlen(argument_passed)] = 0;
    return 0;
}
/* int32_t sys_call_vidmap (uint8_t** screen_start)
 * DESCRIPTION: points a pointer to seperate virtual address of video memory
 * INPUTS: uint8_t** screen_start, pointer that is currently not pointing to anything
 * OUTPUTS: none
 * SIDE EFFECTS: user pointer points to a virtual address, page is created
 * RETURN: -1 if null buffer passed or pointer is in kernel memory
*/
int32_t sys_call_vidmap (uint8_t** screen_start){
    // // *screen_start = 0xB8000;
    // //!TEST
    // if(screen_start == 0 || screen_start <= (uint8_t**) 0x400000) return -1; 

    // /*choose the 4kb block to start at 0xF0000000 virtual memory*/
    // /*0xb8000 is the physical memory address of video memory*/
    // uint32_t va = 0xF0000000;
    // uint32_t video_mem_pa = 0xb8000;

    // /*setup the 4kb page, set it to present*/
    // setup_4kb_page(video_mem_pa, va, 1);

    // /*set the pointer to the virtual address*/
    // *screen_start = (uint8_t*) va;
    // return 0;

    // *screen_start = 0xB8000;
    //!TEST
    if(screen_start == 0 || screen_start <= (uint8_t) 0x400000) return -1; 

    //choose the 4kb block to start at 0xF0000000 virtual memory/
    //0xb8000 is the physical memory address of video memory/

    //EFFFF000 for t2
    //EFFFE000 for t3/
    uint32_t va_t1= 0xF0000000;
    uint32_t va_t2 = 0xEFFFE000;
    uint32_t va_t3 = 0xEFFFC000;



    //uint32_t video_mem_pa= 0xb8000;


    
    //setup the 4kb page, set it to present*/
    //set the pointer to the virtual address/

    switch(get_term_num()){
        case 1:
            setup_4kb_page((uint32_t) (0xB8000 + get_term_num() * 0x1000), va_t1, 1);
            *screen_start = (uint8_t*) va_t1;
            return 0;
        case 2:
           setup_4kb_page((uint32_t) (0xB8000 + get_term_num() * 0x1000), va_t2, 1);
           *screen_start = (uint8_t*) va_t2;
           return 0;
        case 3:
            setup_4kb_page((uint32_t) (0xB8000 + get_term_num() * 0x1000), va_t3, 1);
            *screen_start = (uint8_t*) va_t3;
            return 0;
        default:
            return 0; 
    }

    //set the pointer to the virtual address/

   
    return 0;
}
int32_t sys_call_sethandler (int32_t signum, void* handler_address){
    return -1;
}
int32_t sys_call_sigreturn (void){
    return -1;
}

int get_global_pid() {
    return global_pid;
}

void set_global_pid(int val) {
    global_pid = val;
}

void set_tss_ss0(int ss0) {
    tss.ss0 = ss0;
}
void set_tss_esp0(int esp0) {
    tss.esp0 = esp0;
}
