/** 
 * file_system.c 
 * Authors: Mostly Milind + a little Akaash
 * Provides 8 functions to be used to read directories and contents of a
 * along with 3 helper functions
 * 
 */
#include "file_system.h"


/** Local variables to this file, need them saved so we used static keyword */
struct boot_block_t* starting_mem_ptr; // start of filesys memory
static uint32_t num_dentries;   // number of dentries
static uint32_t num_n;          // number of inodes
static uint32_t num_d;          // number of data blocks
static struct dentry_t first_dentry[63];    // first dentry

/** 
 * file_system_init
 * DESCRIPTION: Initializes the file system
 * INPUTS: start_mem_pos - pointer to the start of the file system
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECTS: initializes the file system
 */
void file_system_init(struct boot_block_t* start_mem_pos) {

    if (start_mem_pos == NULL)  // if the pointer is null, return
    return;

    /** Variable declarations */
    struct boot_block_t boot_block;     // create a boot block
    int i;  // loop variable

    starting_mem_ptr = start_mem_pos;   // assign the starting memory pointer to the start of the file system
    boot_block = *start_mem_pos;        // assign the boot block to the start of the file system

    num_n = boot_block.num_inodes;      // assign the number of inodes
    num_d = boot_block.num_data_blocks; // assign the number of data blocks
    num_dentries = boot_block.num_dir_entries;  // assign the number of dentries


    for (i = 0; i < 63; i++) { // loop through the dentries 
        first_dentry[i] = boot_block.dir_entries[i]; // assign the first dentry to the boot block dentries
    }

    return;
}

/** 
 * read_dentry_by_name
 * DESCRIPTION: Reads the dentry by name
 * INPUTS: fname - pointer to the name of the file
 *         dentry - pointer to the dentry
 * OUTPUTS: none
 * RETURN VALUE: returns dentry index if successful, -1 if not
 * SIDE EFFECTS: none
 */
int32_t read_dentry_by_name (const uint8_t* fname, struct dentry_t* dentry) {
    // ! Check for bad args
    // Checking for bad arguments //
    if(fname == NULL)   // if the pointer is null, return
        return -1; // not a valid filename
    if(fname == '\0')   // if the pointer is null, return
        return -1; // not valid  filename

    /** Variable declarations */
    int i;
    int j;


    for (i = 0; i < num_dentries; i++) {        // loop through the dentries
        for (j = 0; j < 32; j++) {              // loop through the file name
            if (!(fname[j] == first_dentry[i].file_name[j])) {  // if the file name is not the same
                j = 32; // break out of the loop
            }           
            else if ((j == 31) | ((fname[j] == '\0') & (first_dentry[i].file_name[j] == '\0'))) {      // if the file name is the same or we reached the end of the file name
                *dentry = first_dentry[i];      // assign the dentry to the first dentry
                return i; // found the right name
            }
        }
    }
    return -1; // did not find the right name
}

/** 
 * read_dentry_by_index
 * DESCRIPTION: Reads the dentry by index
 * INPUTS: index - index of the dentry
 *         dentry - pointer to the dentry
 * OUTPUTS: none
 * RETURN VALUE: returns 0 if successful, -1 if not
 * SIDE EFFECTS: none
 */
int32_t read_dentry_by_index (uint32_t index, struct dentry_t* dentry) {
    // Checking for bad arguments // 
    if (index < 0 || index > num_dentries) {
        return -1;  // return -1 if the index is out of bounds
    }

    *dentry = starting_mem_ptr -> dir_entries[index];       // assign the dentry to the starting memory pointer dir entries at the given index
    return 0;       // return 0 if successful
}

/** 
 * read_data
 * DESCRIPTION: Reads the data
 * INPUTS: inode - inode number
 *         offset - offset of the data
 *         buf - pointer to the buffer
 *         length - length of the data
 * OUTPUTS: none
 * RETURN VALUE: returns the number of bytes read
 * SIDE EFFECTS: none
 */
int32_t read_data (uint32_t inode, uint32_t offset, char* buf, uint32_t length) {

    /** Variable declarations  +  a few instantiations*/
    uint32_t inode_offset = (inode + 1);                                    // offset of the inode
    struct inode_t* inode_address = (inode_t*)starting_mem_ptr + inode_offset;        // address of the inode
    struct inode_t curr_inode;                                              // current inode
    int i;                                                                  // loop variable
    int32_t inode_data_block_num;                                           // inode data block number
    int32_t inode_data_block_offset;                                        // inode data block offset
    int32_t data_block_num;                                                 // data block number
    //? int32_t data_block_offset; is this needed?????
    int32_t bytes_up_to;                                                    // bytes up to the offset

    curr_inode = *inode_address;        // assign the current inode to the inode address

    if (offset + length > curr_inode.length_in_bytes) { // if we are trying to read past the end
        bytes_up_to = curr_inode.length_in_bytes;       // set the hard stop at the end of the file

        // need to make sure the rest of the buffer is empty since we will not be filling it up
        for (i = bytes_up_to - offset; i < length; i++)     // loop through the rest of the buffer
        {
            buf[i] = '\0';      // set the rest of the buffer to null
        }
        
    } else { // if we are not trying to read past the end
        bytes_up_to = offset + length;     // set the hard stop at the end of the read
    }


    for (i = offset; i < bytes_up_to; i++) {    // loop through the bytes up to the offset
        inode_data_block_num = i / 4096; // finds the data block number
        inode_data_block_offset = i % 4096;  // finds the offset in the data block
        data_block_num = curr_inode.data_blocks[inode_data_block_num];  // finds the data block number

        uint8_t* block_add_address = (uint8_t *)(starting_mem_ptr + (1 + num_n + data_block_num));  // finds the address of the data block
        block_add_address += inode_data_block_offset;   // adds the offset to the address of the data block
        buf[i - offset] = *block_add_address;           // assigns the data block to the buffer

    }
    return bytes_up_to - offset;    // return the number of bytes read

}


/** 
 * file_open
 * DESCRIPTION: Opens the file
 * INPUTS: filename - pointer to the name of the file
 * OUTPUTS: none
 * RETURN VALUE: returns 0 if successful, -1 if not
 * SIDE EFFECTS: sets the file index, inode, and bytes read
 */
int32_t file_open(const uint8_t* filename) {
    // TODO
    //! Initialize any temporary structures
    //! check for valid filename
    return 0;      // file is already put into fd array in sys_call_open
}

/** 
 * file_close
 * DESCRIPTION: Closes the file
 * INPUTS: fd - file descriptor
 * OUTPUTS: none
 * RETURN VALUE: returns 0 if successful, -1 if not
 * SIDE EFFECTS: undoes what was done in the open function
 */
int32_t file_close(int32_t fd) {
    // TODO
    // ! Undo what was done in the open function

    if (fd <= 1 || fd >= 8) {
        return -1;
    }

    return 0;

    // return 0;   // return 0
}

/** 
 * file_write
 * DESCRIPTION: Writes to the file
 * INPUTS: fd - file descriptor
 *         buf - pointer to the buffer
 *         nbytes - number of bytes
 * OUTPUTS: none
 * RETURN VALUE: returns -1
 * SIDE EFFECTS: none
 * NOTE: This function is not implemented - not needed for this demo
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1; // read-only file system, so return error
}

/** 
 * file_read
 * DESCRIPTION: Reads the file
 * INPUTS: fd - file descriptor
 *         buf - pointer to the buffer
 *         nbytes - number of bytes
 * OUTPUTS: none
 * RETURN VALUE: returns the number of bytes read
 * SIDE EFFECTS: none
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    // TODO
    // ! Read count bytes of data from file into buf

    if (buf == NULL || fd <= 1 || fd >= 8) { // checks for bad args
        return -1;
    }

    uint32_t bytes_read;
    struct pcb pcb_array_entry;
    uint8_t* buff;                  // buffer

    pcb_array_entry = get_pcb_pid(get_global_pid()); // gets the pcb, which is used to read data from the file
    buff = (uint8_t*) buf; 

    bytes_read = read_data(pcb_array_entry.fd_array[fd].inode, pcb_array_entry.fd_array[fd].file_position, (char *)buff, nbytes); // function writes bytes into the buffer

    return bytes_read; // return the number of bytes read
}

/** 
 * dir_open
 * DESCRIPTION: Opens the directory
 * INPUTS: filename - pointer to the name of the file
 * OUTPUTS: none
 * RETURN VALUE: returns 0
 * SIDE EFFECTS: none
 * NOTE: This function is not implemented - not needed for this demo
 */
int32_t dir_open(const uint8_t* filename) {
    // TODO
    // ! Opens a directory file 

    return 0; // success

}

/** 
 * dir_close
 * DESCRIPTION: Closes the directory
 * INPUTS: fd - file descriptor
 * OUTPUTS: none
 * RETURN VALUE: returns 0
 * SIDE EFFECTS: none
 * NOTE: This function is not implemented - not needed for this demo
 */
int32_t dir_close(int32_t fd) {
    if (fd <= 1 || fd >= 8) { // check for bad args
        return -1;
    }
    
    return 0; // success

}

/** 
 * dir_write
 * DESCRIPTION: Writes to the directory
 * INPUTS: fd - file descriptor
 *         buf - pointer to the buffer
 *         nbytes - number of bytes
 * OUTPUTS: none
 * RETURN VALUE: returns -1
 * SIDE EFFECTS: none
 * NOTE: This function is not implemented - not needed for this demo
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1; // read-only file system
}

/** 
 * dir_read
 * DESCRIPTION: Reads the directory
 * INPUTS: fd - file descriptor
 *         buf - pointer to the buffer
 *         nbytes - number of bytes
 * OUTPUTS: none
 * RETURN VALUE: returns the number of bytes read
 * SIDE EFFECTS: none
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    // TODO
    // ! Reads files filename by filename, including "."
    /* Variable Declarations/Instantiations */

    uint32_t bytes_read = 0;        // setup bytes read
    struct dentry_t dentry;         // dentry
    uint8_t* buff;                  // buffer
    buff = (uint8_t*) buf;          // assign the buffer to the buffer pointer
    int i;                          // loop variable
    struct pcb pcb_array_entry;
    int32_t start_entry;
    pcb_array_entry = get_pcb_pid(get_global_pid());
    int null_index = 32;

    // gets the file number inside of the file system
    start_entry = (pcb_array_entry.fd_array[fd].file_position) / 32;
    if (nbytes < 32){   // if nbytes is less than 32
        // write to buffer the first nbytes of the filename
        for(i = 0; i < nbytes; i++){
            buff[i] = dentry.file_name[i];
        }
        // fill up the rest of the buffer with null characters
        for(i = nbytes; i < 32; i++){
            buff[i] = '\0';
        }
        return 0;
    } 
    else {
        // gets the directory entry at the file index
        dentry = starting_mem_ptr -> dir_entries[start_entry];
        // writes the file name into the buffer
        for(i = 0; i < 32; i++){
            buff[i] = dentry.file_name[i];
        }
        // calculates the first NULL character (useful for terminal_write)
        for (i = 0; i < 32; i++) {
            if (dentry.file_name[i] == 0) {
                null_index = i;
                break;
            }
        }
        // returns the number of non-NULL characters
        return null_index;
    }
    // returns the number of bytes read if it somehow gets here
    return bytes_read;
}
