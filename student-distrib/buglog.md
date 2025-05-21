# Checkpoint 1 Bugs

## Bug 1: Cannot load starting address of GDT into GDTR
Type of Issue: Algorithmic Error <br>
Difficulty of Finding the Bug: Easy (Stuck in boot loop)
Actual Issue: The struct was not being initialized properly. The padding variable was already defined in the GDT variable, and we were redefining it. This caused the boot loop to happen. <br>
Solution: We removed the redefinition of the padding variable in the struct, and this solved the boot loop issue <br>
Solution Difficulty: Medium 

## Bug 2: Cannot print interrupt number, random value is being displayed on the screen
Type of Issue: Algorithmic Error <br>
Difficulty of Finding the Bug: Easy (Wrong interrupt number is displayed on the screen) <br>
Actual Issue: We were trying to handle all interrupts in the same function. Even with pushing the interrupt number to the stack before calling the handler, we were still unable to display the interrupt number. <br>
Solution: We changed the one function approach to include multiple handlers, one for each interrupt. For each interrupt, we loaded different handler was loaded into the IDT, specific to the interrupt that occurred. We were able to print different exception codes as a result. <br>
Solution Difficulty: Easy <br>

## Bug 3: Keyboard interrupt not being recognized 
Type of Issue: Algorithmic Error <br>
Difficulty of Finding the Bug: Very hard <br>
Issue: sti() not being uncommented at the end of kernel entry. Tried this earlier, but was giving a general protection fault. Sti not being uncommented meant keyboard interrupts could not happen. <br>
Solution: had to fix masks of irq number masks and initialization inside PIC and get rid of cli/sti inside pic (originally copied from lecture notes). <br>
Solution difficulty: hard <br>

## Bug 4: Paging is Stuck in Boot Loop
Type of Issue: Algorithmic Error <br>
Difficulty of Finding the Bug: Easy (Screen keeps bootlooping) <br>
Issue: There were many issues. (1) We were trying to translate between the 4 kB and 4 MB page bit formats, which was wrong and causing many bugs. (2) When trying to set the CR3 register, in a function where the argument was the address to set the CR3 regiser to, we tried to access the first argument using M[ESP + 8] instead of M[EBP + 8] <br>
Solution: For issue 1, we created a union between the structs the store the 4 kB and 4 MB page bit formats for the page directory. This allowed us to have 1 array that stored both 4 kB format and 4 MB format structs. For 2, we changed the way to access the first argument from relying on the stack pointer, which changes with the function, to relying on the base pointer, which stays the same throughout the operation of the subroutine <br>
Solution difficulty: hard <br>

## Bug 5: RTC Failed to initialize
Type of Issue: Port connection was incorrect <br>
Difficulty of Finding the Bug: Medium (Had to proofread code) <br>
Issue: The DATA and CMD ports were incorrectly placed, also the outb parameters were swapped. <br>
Solution: Use the correct CMD or DATA port when calling outb and swapped the outb parameter order <br>


# Checkpoint 2 Bugs

## Bug 1: File System Read Prints Random Characters
Type of Issue: Algorithmic Error <br>
Difficulty of Finding the Bug: Easy (Data printed to screen was not the data stored in the file) <br>
Issue: There were a couple issues. (1) When calculating the address to read the data from to load into the buffer, I was adding block pointer types with offsets within the block, which was giving a memory address that was wrong. (2) When printing the data, I was using a printf statement, which was formatting the output in a way that made no sense. <br>
Solution: For issue 1, I separated the address calculation into two steps: 1 step to calculate the starting address of the block, and the second to calculate the offset within the block. Now, I wasn't using different pointer types. For issue 2, I used a PUTC statement instead of a PRINTF, which formatted the contents of the file appropriately. <br>
Solution Difficulty: Hard <br>

## Bug 2: Read Directory Reads the Entire Directory Instead of Reading One File at a Time
Type of Issue: Algorithm Error <br>
Difficulty of Finding the Bug: Easy (entire directory is displaying on the screen) <br>
Issue: I never made sure that the number of bytes to read was appropriate for one file. As a result, I kept going to the next file until the number of bytes was met. <br>
Solution: I added a condition to make sure that the read directory would only read the contents of one directory entry at a time, rather than the entire directory at once. <br>
Solution Difficulty: Medium <br>

## Bug 3: Backspace causing loss of data within terminal_read
Type of Issue: Algorithm Error <br>
Difficulty of Finding the Bug: Medium-had to create a fairly obscure test case to reveal the bug <br>
Issue: Filling up the buffer in the terminal driver by going over 128 chars, then backspacing a bit to go below 128, then typing more characters caused some characters to not be read and displayed via terminal_write. <br>
Solution: Conditional added at the end of list of conditionals in add_char (function that <br> populates 128 length buffer which terminal_read eventually reads) that increased count of <br> pressed characters even if it overshot the buffer, and didn't attempt to add anything to the buffer. <br> This means after the buffer filed up, a running count was kept of how many total chars, and <br> when backspace was pressed that count was decremented, and then the logic checks if the count <br>is less than the buffer size to decide whether or not to replace that position in the buffer with a backspace or not.
Solution Difficulty: Medium <br>

# Checkpoint 3 Bugs

## Bug 1: Corrupted Terminal Driver (from CP2)
Type of Issue: Indexing Error <br>
Difficulty of Finding the Bug: hard- It was difficult to find source of bug since it only showed up once the terminal driver and the filesystem code were integrated into master. <br>
Issue: The terminal driver had very weird functionality once we integrated the filesystem into it. It would start off on caps lock, and sometimes be able to be toggled off. Enter would also not work and pressing more than 3 characters at once would cause it to clear the page. Since the issue was all over the place it was hard to locate if the issue was with the terminal driver or some other part of the code. <br>
Solution: We had to go to the `file_system_init` function and change the bounds for the for loop such that it goes from 0 to 62 inclusive, or the first 63 entries of the `dir_entries` of the boot block. Since it was going over the first 64 entries it must have been overwriting something related to the terminal driver that happened to be located next to the boot block in memory. <br>
Solution Difficulty: Easy <br>

## Bug 2: Setting Up Paging does not work
Type of Issue: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - Took a bit of GDB'ing and commenting out some code to find the source of the problem. <br>
Issue: The `enable_paging` function was being called a second time when setting up the pages to load the page at 128 MB virtual memory. <br>
Solution: Load the page directory again, with the updated page directory entires, but do not enable paging again, because it was cloberring the CR0 register and causing a page fault. <br>
Solution Difficulty: Medium <br>

## Bug 3: Program is not getting loaded with Memcpy function
Type of Issue: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - contents of the file does not properly print after copied. <br>
Issue: The method that we were using the calculate the length of the file was off. The length of the file that our code told us was super small compared to the actual size of the file. <br>
Solution: We set the length of the number of bytes to read to a super high number, higher than the number of bytes that any file in the filesystem would have. This allowed the entire file to get read. <br>
Solution Difficulty: Medium <br>

## Bug 4: When performing the context switch, the program is Page Faulting. 
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Extermely Hard - It was almost impossible to GDB through and find the root cause of the issue. We tried looking at everything, from the Intel IA-32 Manual to the Discussion Slides provided. <br>
Issue: When calculating the starting address of the executable file, we were not bit-masking, which was causing a large issue. As a result, the value that we were passing in as the starting address of the file was off by a little bit, causing a page fault. <br>
Solution: We had to calculate the starting address of the file using a bitmask, as the 24th and 25th bytes of the file had leading F hex values. <br> 
Solution Difficulty: Extremely Hard <br>

## Bug 5: After IRET is called and the Shell prints the "391OS>" prompt, the keyboard input does not display on the screen
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - It was hard to think of reasons why this would happen
Issue: When calling `IRET`, we did not have an `sti` function, to enable interrupts after the `IRET`. Because the keyboard input is an interrupt, the keyboard interrupt did not display on the screen. <br>
Solution: We added an `sti` call before and after the `IRET` call. This allowed for the keyboard to be functional even after the context switch was performed. <br>
Solution Difficulty: Easy <br>

# Checkpoint 4 Bugs
## Bug 1: "ls" command only displays one file at a time
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Hard - because GDB would not go back to the 391 Ls command C file given, it was hard to find the root cause of the bug. <br>
Issue: In the terminal_write function, the number of bytes written returned by the function does not account for NULL characters that the terminal_write function skips over. However, in the dir_read function, the return value would include the NULL characters in the bufer outputted. <br> 
Solution: In the dir_read function, I only returned the number of non-NULL character written to the buffer. <br>
Solution Difficulty: Easy <br>

## Bug 2: "fish" command fills the screen with the same character
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - Although GDB helped immensely with finding the bug, it was super easy to gloss over the issue and assume it worked <br>
Issue: When updating the file position afte rreadin ghte file, rather than adding the current position of the file to the number of bytes read, I was only putting in the number of bytes read. <br>
Solution: When updating hte position to start from inside of the file, I added the urrent file position to the number of bytes read. <br>
Solution Difficulty: Easy <br>

## Bug 3: "sigerr" command page faults on test 8
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - GDB really helped find the issue <br>
Issue: In the assembly linkage for system calls, we used the RET command, which caused memory to be accessed that wasn't present in following instructions <br>
Solution: Changed the "RET" command to "IRET" <br>
Solution Difficulty: Easy <br>

## Bug 4: "grep" does not always work
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - GDB and print statements really helped find the bug <br>
Issue: At the end of the get_args array, it was not NULL-terminated, which was ocassionally corrupting the argument <br>
Solution: Added a NULL at the end of the argument <br>
Solution Difficulty: Easy <br>

# Checkpoint 5 Bugs
## Bug 1: When switching to a new terminal (either terminal 2 or 3), it does not accept any more keyboard inputs. 
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - GDB was not easy to use to solve this issue, and we had to rely on going line by line to solve it. <br>
Issue: When executing a new shell among switching to a new terminal, we were not sending an EOI signal to signal the end of an interrupt. Since the shell is like an infinite loop, the interrupt line was never raised again, so there were no keyboard interrupts taken. <br> 
Solution: We put a send_eoi signal prior to executing the shell. <br>
Solution Difficulty: Easy <br>

## Bug 2: When running a program like pingpong or counter and switching the terminal, the program would write over the video memory even in a new terminal it was not called in
Type of Error: Terminal Write Error <br>
Difficulty of Finding the Bug: Medium - I initially thought it was an issue with my buffers and that programs like pingpong were writing directly to video memory. <br>
Issue: The PUTC function was writing to the buffer corresponding to the terminal being displayed regardless of the whether the task in that terminal was scheduled or not. <br>
Solution: I wrote another PUT function to write the data from the program into the buffer for that terminal. <br>
Solution Difficulty: Medium <br>

## Bug 3: Fish user program is flickering when scheduling is applied
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - Needed to create new buffers similar to multiple terminals process <br>
Solution: I created three separate virtual address buffers corresponding to three pages in physical addresses. Uses memcpy of active buffer that new PUT function uses. <br>
Solution Difficulty: Medium <br>

## Bug 4: Same EBP and ESP being used in execute and halt is also being used in scheduling EBP and ESP
Type of Error: Algorithmic Error <br>
Difficulty of Finding the Bug: Medium - We were page faulting and had no clue why. We used GDB to step through the execute and halt functions and realized that we were using the same stack pointers in two places that should not use the same ones. <br>
Solution: In the PCB struct, we created separate fields for the EBP and ESP used in halt and execute versus the EBP and ESP used in scheduling. <br>
Solution Difficulty: Easy <br>
