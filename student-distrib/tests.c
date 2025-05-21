#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "file_system.h"
#ifndef RUN_TESTS
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
static int idt_test(){
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/** 
 * * Divide by zero test
 * Input: NONE
 * Output: FAIL if divide by zero exception does not occur
 * Side Effects: Divide by zero exception
 * Coverage: IDT, Divide by zero exception
 * Files: kernel.c/ex_c_handlers.c/h/x86_desc.h/S
 */

static int zero_divide(){
	int a = 69;
	int b = 0;
	int c = 420;

	/* Trigger divide by zero exception */
	c = a / b;
	/* Should never reach here, fail if do */
	return FAIL;
}

/* IDT Test macro - Individual Exceptions
 * 
 * A macro? that allows us to test each exception via its
 * number/entry in the idt, can fill up with code that is
 * supposed to cause an exception
 * Inputs: interrupt number
 * Side Effects: Calls the interrupt associated with the 
 * 				 interrupt number and runs the handler
 * 				 If successful, handler should execute
 * Coverage: All exceptions, keyboard interrupt, rtc interrupt
 * Files: x86_desc.h/S, ex_c_handlers.h/c
 * 
 */

static int idt_test_individual_exceptions(int int_num){
	TEST_HEADER;

	int** b;
	switch (int_num)
	{
	case 0:
		zero_divide();
		break;
	
	case 1:
		asm("int $1");
		break;
	case 2:
		asm("int $2");
		break;
	case 3:
		asm("int $3");
		break;
	case 4:
		asm("int $4");
		break;
	case 5: 
		asm("int $5");
		break;
	case 6:
		asm("int $6");
		break;
	case 7:
		asm("int $7");
		break;
	case 8:
		asm("int $8");
		break;
	case 9:
		asm("int $9");
		break;
	case 10:
		asm("int $10");
		break;
	case 11:
		asm("int $11");
		break;
	case 12:
		asm("int $12");
		break;
	case 13:
		asm("int $13");
		break;
	case 14:
		b  = (int **) (0x00000000);
		printf("%d", *b);
		break;
	case 15:
		printf("Exception 15 Reserved\n");
		return PASS;
		break;
	case 16:
		asm("int $16");
		break;
	case 17:
		asm("int $17");
		break;
	case 18:
		asm("int $18");
		break;
	case 19:
		asm("int $19");
		break;
	case 20:
		asm("int $20");
		break;
	case 21:
		asm("int $21");
		break;

	case 28:
		asm("int $28");
		break;
	case 29:
		asm("int $29");
		break;
	case 30:
		asm("int $30");
		break;
	
	case 33:
		printf("Keyboard interrupt\nPress keys to type\nIf nothing types then fail.");
		return PASS;
		break;
	case 40:
		asm("int $40");
		break;
	case 128:
		asm("int $128");
		break;
	default:
		if((int_num < 28) & (int_num > 21))
			printf("22 - 27 is Reserved, current IDT entry is:%d",int_num);
		else
			printf("Not needed for cp1\n");
		return PASS;
		break;
	
	}
	
	
	return FAIL;
}

/** 
 * * RTC test
 * Input: NONE
 * Output: PASS if RTC works
 * Side Effects: RTC interrupt
 * Coverage: RTC
 * Files: rtc.c/h
 */
static int rtc_test(){
	TEST_HEADER;
	/* Counter */
	int count = 0;

	/* Print warning */
	printf("Entering RTC test, if nothing happens RTC failed!");
	/* Set RCT clock division 3 to 15 only! */
    rtc_divide_freq(0x0F);
	/* Loop for RTC */
	while(count < 10){
		rtc_wait();
		if(count == 0){
			clear();
		}
	    test_interrupts();
		count++;
	}
	clear();
	return PASS;
}

/** 
 * * Page fault test
 * Input: address to access
 * Output: PASS by defualt, nothing if page fault occurs
 * Side Effects: If the address is valid, the some address will be printed
 * 				if the address is not valid a page fault will occur
 * 
 * Coverage: Paging
 * Files: paging.c/h
 * 
 * ! Valid page: [0x00400000,0x00800000) // 4MB - 8MB // Kernel
 * ! Valid page: [0x000b8000,0x000b9000) // 184KB - 188KB // Video memory
 * 
 * pages out of the region will cause a page fault
 */
static int page_fault(uint32_t address){
	TEST_HEADER;
	int** b  = (int **) (address);
	printf("%d", *b);
	return PASS;
}

/* Checkpoint 2 tests */
/** 
 * * terminal_open_test
 * Input: NONE
 * Output: PASS if terminal works
 * Side Effects: keyboard interrupt
 * Coverage: terminal_open
 * Files: terminal.c/h
 */
static int terminal_open_test(){
	TEST_HEADER;

	uint8_t file_name = 1;
	if(terminal_open(&file_name) == 0){
		return PASS;
	}
	return FAIL;
}

/** 
 * * terminal_close_test
 * Input: NONE
 * Output: PASS if terminal works
 * Side Effects: keyboard interrupt
 * Coverage: terminal_close
 * Files: terminal.c/h
 */
static int terminal_close_test(){
	TEST_HEADER;

	if(terminal_close(1) == 0){
		return PASS;
	}
	return FAIL;
}


	/* Testing file system */
static int file_system_read_file(const uint8_t* filename, int buffer_size){
	int i;
	clear();
	TEST_HEADER;
	printf("Opening file....\n");
    int32_t returnval;
    returnval = file_open(filename);

    char buf[buffer_size];
    returnval = file_read(0, buf, buffer_size);
    printf("--------start--------\n");
    // printf(buf);
	for(i = 0 ; i < buffer_size; i++){
		// if((i % 80) == 0)
		// 	putc('\n');
        if(buf[i] != '\0'){
            putc(buf[i]);
        }
	}
    file_close(0);


    printf("\n--------end--------\n");
	return 1;
}

static int file_system_multiple_reads(const uint8_t* filename, int buffer_size, int buffer_size2){
	clear();
	int i;
	TEST_HEADER;
	printf("Opening file....\n");
    int32_t returnval;
    returnval = file_open(filename);
    char buf[buffer_size];
	char buf2[buffer_size2];
    returnval = file_read(0, buf, buffer_size);
	returnval = file_read(0, buf2, buffer_size2);
    printf("--------start--------\n");
    for(i = 0 ; i < buffer_size; i++){

        if(buf[i] != '\0'){
            putc(buf[i]);
        }
	}
	
    printf("\n--------end--------\n");
	printf("--------start--------\n");
	for(i = 0 ; i < buffer_size; i++){

        if(buf2[i] != '\0'){
            putc(buf2[i]);
        }
	}
    printf("\n--------end--------\n");
	return 1;
}
static int file_system_read_directory(){
	int i;
	int j;
	clear();
	TEST_HEADER;
    printf("Reading directory....\n");
    int32_t returnval;
    dir_open((uint8_t *)".");
    char buf[100];
	for(i = 0; i < 17; i++){
		returnval = dir_read(0,buf, 100);
		printf(buf);
		printf("\n");
		// clear buf
		for(j = 0; j < 100; j++){
			buf[j] = '\0';
		}
	}

	return 1;
}


/** 
 * * rtc_open_test
 * Input: NONE
 * Output: PASS if RTC works
 * Side Effects: RTC interrupt
 * Coverage: rtc_open
 * Files: rtc.c/h
 */
static int rtc_open_test(){
	TEST_HEADER;

	uint8_t file_name = 1;
	if(rtc_open(&file_name) == 0){
		return PASS;
	}
	return FAIL;
}

/** 
 * * rtc_close_test
 * Input: NONE
 * Output: PASS if RTC works
 * Side Effects: RTC interrupt
 * Coverage: rtc_close
 * Files: rtc.c/h
 */
static int rtc_close_test(){
	TEST_HEADER;

	if(rtc_close(1) == 0){
		return PASS;
	}
	return FAIL;
}




/** 
 * * rtc_freq_loop
 * Input: NONE
 * Output: PASS if RTC works
 * Side Effects: RTC interrupt
 * Coverage: rtc_close
 * Files: rtc.c/h
 */
static int rtc_freq_loop(){
	TEST_HEADER;

	/* Local Variables */
	uint32_t freq;			// Current frequency
	int count = 0;			// Number of loops at current frequency
	int count_offset = 4;	// Counter offset to see faster RTC for longer

	/* Loop through frequencies at powers of two */
	for(freq = RTC_MIN; freq <= RTC_MAX; freq = freq << 1){

		/* Reset for next frequency */
		clear();
		count = 0;

		/* Write next frequency */
		rtc_change(freq);

		/* Test RTC Read at given freq */
		while(count < count_offset){
			test_interrupts();
			rtc_read(0, NULL, 0);
			count++;
		}

		/* Increase offset for faster next frequency */
		count_offset = count_offset * 2;
	}

	/* Clear screen and return */
	clear();
	return PASS;
}



/*
 * * terminal test
 * Input: NONE
 * Output: Prints to terminal
 * Side Effects: stores user input in buffer
 * Coverage: terminal
 */
static int terminal_test(){

	TEST_HEADER;

	/*buffer to hold terminal characters*/
	uint8_t buf[256];

	/*variables to show how many bytes were written, read, and how much you are requesting to read/write*/
	int32_t bytes_written;
	int32_t bytes_read;
	int32_t nbytes; 
	while(1){

		/*try to read 0 bytes from terminal, and then write back to terminal*/
		nbytes = 0;
		printf("requesting to read %d bytes from the terminal\n", nbytes);
		bytes_read = terminal_read(0, buf, nbytes);
		printf("read %d bytes from the terminal\n\n", bytes_read);
		printf("writing what was just read: \n");
		bytes_written = terminal_write(0, buf, bytes_read);
		printf("\n");
		printf("wrote %d bytes from the terminal\n\n", bytes_written);

		/*try to read 10 bytes from terminal, and then write back to terminal*/
		nbytes = 10;
		printf("requesting to read %d bytes from the terminal\n", nbytes);
		bytes_read = terminal_read(0, buf, nbytes);
		printf("read %d bytes from the terminal\n\n", bytes_read);
		printf("writing what was just read: \n");
		bytes_written = terminal_write(0, buf, bytes_read);
		printf("\n");
		printf("wrote %d bytes from the terminal\n\n", bytes_written);

		/*try to read 94 bytes from terminal, and then write back to terminal*/
		nbytes = 94;
		printf("requesting to read %d bytes from the terminal\n", nbytes);
		bytes_read = terminal_read(0, buf, nbytes);
		printf("read %d bytes from the terminal\n\n", bytes_read);
		printf("writing what was just read: \n");
		bytes_written = terminal_write(0, buf, bytes_read);
		printf("\n");
		printf("wrote %d bytes from the terminal\n\n", bytes_written);

		/*try to read 127 bytes from terminal, and then write back to terminal*/
		nbytes = 127;
		printf("requesting to read %d bytes from the terminal\n", nbytes);
		bytes_read = terminal_read(0, buf, nbytes);
		printf("read %d bytes from the terminal\n\n", bytes_read);
		printf("writing what was just read: \n");
		bytes_written = terminal_write(0, buf, bytes_read);
		printf("\n");
		printf("wrote %d bytes from the terminal\n\n", bytes_written);
		
		/*try to read 128 bytes from terminal, and then write back to terminal*/
		nbytes = 128;
		printf("requesting to read %d bytes from the terminal\n", nbytes);
		bytes_read = terminal_read(0, buf, nbytes);
		printf("read %d bytes from the terminal\n\n", bytes_read);
		printf("writing what was just read: \n");
		bytes_written = terminal_write(0, buf, bytes_read);
		printf("\n");
		printf("wrote %d bytes from the terminal\n\n", bytes_written);

		/*try to read 129 bytes from terminal, and then write back to terminal*/
		nbytes = 129;
		printf("requesting to read %d bytes from the terminal\n", nbytes);
		bytes_read = terminal_read(0, buf, nbytes);
		printf("read %d bytes from the terminal\n\n", bytes_read);
		printf("writing what was just read: \n");
		bytes_written = terminal_write(0, buf, bytes_read);
		printf("\n");
		printf("wrote %d bytes from the terminal\n\n", bytes_written);

		/*try to read 140 bytes from terminal, and then write back to terminal*/
		nbytes = 140;
		printf("requesting to read %d bytes from the terminal\n", nbytes);
		bytes_read = terminal_read(0, buf, nbytes);
		printf("read %d bytes from the terminal\n\n", bytes_read);
		printf("writing what was just read: \n");
		bytes_written = terminal_write(0, buf, bytes_read);
		printf("\n");
		printf("wrote %d bytes from the terminal\n\n", bytes_written);

	}
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){

	// clear screen before launching tests
	clear();
	printf("Launching tests\n");
	/* Begin test cases*/
	/* Checkpoint 1 tests start */
	/* NOTE: keyboard can be tested as long as an exception has not been called */

	TEST_OUTPUT("rtc_test", rtc_test()); // rtc test
	TEST_OUTPUT("given idt_test", idt_test()); // 0 - 10  on table
	TEST_OUTPUT("checking exception entries", idt_test_individual_exceptions(14));
	TEST_OUTPUT("page_fault_test inside kernel", page_fault(0x00600000)); // inside kernel
	TEST_OUTPUT("page_fault_test inside video memory", page_fault(0x000b8090)); // inside video memory
	TEST_OUTPUT("page_fault_test before video memory", page_fault(0x00000000)); // before video memory
	TEST_OUTPUT(("page_fault_test after video memory"), page_fault(0x000b9000)); // after video memory
	TEST_OUTPUT("page_fault_test before kernel", page_fault(0x00300000)); // before kernel
	TEST_OUTPUT("page_fault_test after kernel", page_fault(0x00800000)); // after kernel
	/* Checkpoint 1 tests end */

	/* Checkpoint 2 tests start */

	//! Test reading a single file once
	TEST_OUTPUT("file_system_read_file", file_system_read_file((uint8_t *)"frame0.txt", 1000)); // read frame0.txt
	TEST_OUTPUT("file_system_read_file", file_system_read_file((uint8_t *)"verylargetextwithverylongname.txt", 10000));
	TEST_OUTPUT("file_system_read_file", file_system_read_file((uint8_t *)"grep", 100));
	TEST_OUTPUT("file_system_read_file", file_system_read_file((uint8_t *)"grep", 10000));
	//! Test reading a single file multiple times
	TEST_OUTPUT("file_system_multiple_reads", file_system_multiple_reads((uint8_t*) "frame0.txt", 30, 30)); // read frame0.txt twice 30 characters at a time
	TEST_OUTPUT("file_system_multiple_reads", file_system_multiple_reads((uint8_t *)"verylargetextwithverylongname.txt", 30, 30));

	//! Test reading directories
	TEST_OUTPUT("file_system_read_directory", file_system_read_directory());

	TEST_OUTPUT("rtc_read/write_test", rtc_freq_loop());
	TEST_OUTPUT("rt_open_test", rtc_open_test());
	TEST_OUTPUT("rtc_close_test", rtc_close_test());
	TEST_OUTPUT("terminal test", terminal_test());
	TEST_OUTPUT("terminal_open_test", terminal_open_test());
	TEST_OUTPUT("terminal_close_test", terminal_close_test());
	/* Checkpoint 2 tests end */

	//!Checkpoint 2 tests
	// TEST_OUTPUT("terminal test", terminal_test());
	// TEST_OUTPUT("terminal_open_test", terminal_open_test());
	// TEST_OUTPUT("terminal_close_test", terminal_close_test());

}
#endif
