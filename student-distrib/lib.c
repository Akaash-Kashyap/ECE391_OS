/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "multiple_terminals.h"
//originally chose 0xC0000, 0xC2000, 0xC4000
// #define VIDEO_T1     0xC0000
// #define VIDEO_T2     0xC2000
// #define VIDEO_T3     0xC4000
#define VIDEO        0xB8000
#define VIDEO_T1     0xB9000
#define VIDEO_T2     0xBA000
#define VIDEO_T3     0xBB000
#define NUM_COLS    80
#define NUM_ROWS    25
//#define ATTRIB      0xE

static char ATTRIB = 0xE;
static char ATTRIB_1 = 0XE;
static char ATTRIB_2 = 0xA;
static char ATTRIB_3 = 0xD;
static int screen_x;
static int screen_y;

static int screen_x_t1 = 0;
static int screen_y_t1 = 0;

static int screen_x_t2 = 0;
static int screen_y_t2 = 0;

static int screen_x_t3 = 0;
static int screen_y_t3 = 0;


static char* video_mem = (char *)VIDEO;
//static int terminal_number = 1; // current terminal being displayed


//char video_mem_t1[4096];
static char* video_mem_t1 = (char *)VIDEO_T1;
static char* video_mem_t2 = (char *)VIDEO_T2;
static char* video_mem_t3 = (char *)VIDEO_T3;

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */

//clear cursor too, set screen x and y to zero zero
void clear(void) {
    int32_t i;
    char* clear_mem_loc;
    int tid = get_term_num();
    clear_mem_loc = video_mem_t1;
    switch(tid){
        case 1:
            clear_mem_loc = video_mem_t1;
            screen_x_t1 = 0;
            screen_y_t1 = 0;
            ATTRIB = ATTRIB_1;
            break;
        case 2:
            clear_mem_loc = video_mem_t2;
            screen_x_t2 = 0;
            screen_y_t2 = 0;
            ATTRIB = ATTRIB_2;
            break;
        case 3:
            clear_mem_loc = video_mem_t3;
            screen_x_t3 = 0;
            screen_y_t3 = 0;
            ATTRIB = ATTRIB_3;
            break;
        default:
            break;
    }
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(clear_mem_loc + (i << 1)) = ' ';
        *(uint8_t *)(clear_mem_loc + (i << 1) + 1) = ATTRIB;
    }
    // cli();
    memcpy(video_mem, clear_mem_loc,  NUM_ROWS * NUM_COLS * 2);
    // sti();

    //memcpy(video_mem, video_mem_t1, NUM_ROWS*NUM_COLS*2);
    //!set to specific terminal?
    screen_x = 0;
    screen_y = 0;
    
    update_cursor();
    
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}


/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console
 * Side Effects: changes screen x and y positions */
void put_display_text(uint8_t c) {
    char* cur_term_loc;
    int tid = get_pcb_pid(get_global_pid()).terminal_idx + 1; // get_term_num();
    cur_term_loc = video_mem_t1;
    switch(tid){
        case 1:
            cur_term_loc = video_mem_t1;
            screen_x = screen_x_t1;
            screen_y = screen_y_t1;
            ATTRIB = ATTRIB_1;
            break;
        case 2:
            cur_term_loc = video_mem_t2;
            screen_x = screen_x_t2;
            screen_y = screen_y_t2;
            ATTRIB = ATTRIB_2;
            break;
        case 3:
            cur_term_loc = video_mem_t3;
            screen_x = screen_x_t3;
            screen_y = screen_y_t3;
            ATTRIB = ATTRIB_3;
            break;
        default:
            break;
    }
    if(c == '\n' || c == '\r') {
        /*no need for scrolling if you are not at the last line*/
        if(screen_y < NUM_ROWS-1){
            /*update cursor position to start of next line*/
            screen_y++;
            screen_x = 0;
        }
        /*if you are at the last line (NUM_ROWS-1), then you must scroll*/
        else{
            shift_up(cur_term_loc);
        }
    } 
    /*any other character*/
    else {
        /*put the character into video memory*/
        *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;

        /*if you have reached the end of the line*/
        if (screen_x == NUM_COLS) {
            /*if you are not on the last row*/
            if(screen_y < NUM_ROWS-1){
                /*update the cursor to be at the start of next line*/
                screen_x = 0; 
                screen_y++;
                
            }
            /*you are at the last row. displaying this character must cause scrolling. */
            else{
                shift_up(cur_term_loc);
            }
        }
        else{
            screen_x %= NUM_COLS;
            screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
        }
    }
    switch(tid){
        case 1:
            screen_x_t1 = screen_x;
            screen_y_t1 = screen_y;
            break;
        case 2:
            screen_x_t2 = screen_x;
            screen_y_t2 = screen_y;
            break;
        case 3:
            screen_x_t3 = screen_x;
            screen_y_t3 = screen_y;
            break;
        default:
            break;
    }
    // cli();
    memcpy(video_mem, cur_term_loc,  NUM_ROWS * NUM_COLS * 2);
    // sti();
    update_cursor();
    // sti();
}








/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console
 * Side Effects: changes screen x and y positions */
void putc_term(uint8_t c, int loc) {
    char* cur_term_loc;
    int tid;
    tid = loc + 1;
    cur_term_loc = video_mem_t1;
    /* Get current terminal position */
    switch(tid){
        case 1:
            cur_term_loc = video_mem_t1;
            screen_x = screen_x_t1;
            screen_y = screen_y_t1;
            ATTRIB = ATTRIB_1;
            break;
        case 2:
            cur_term_loc = video_mem_t2;
            screen_x = screen_x_t2;
            screen_y = screen_y_t2;
            ATTRIB = ATTRIB_2;
            break;
        case 3:
            cur_term_loc = video_mem_t3;
            screen_x = screen_x_t3;
            screen_y = screen_y_t3;
            ATTRIB = ATTRIB_3;
            break;
        default:
            break;
    }
    /*enter is pressed*/
    if(c == '\n' || c == '\r') {
        /*no need for scrolling if you are not at the last line*/
        if(screen_y < NUM_ROWS-1){
            /*update cursor position to start of next line*/
            screen_y++;
            screen_x = 0;
        }
        /*if you are at the last line (NUM_ROWS-1), then you must scroll*/
        else{
            shift_up(cur_term_loc);
        }
    } 
    /*any other character*/
    else {
        /*put the character into video memory*/
        *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;

        /*if you have reached the end of the line*/
        if (screen_x == NUM_COLS) {
            /*if you are not on the last row*/
            if(screen_y < NUM_ROWS-1){
                /*update the cursor to be at the start of next line*/
                screen_x = 0; 
                screen_y++;
                
            }
            /*you are at the last row. displaying this character must cause scrolling. */
            else{
                shift_up(cur_term_loc);
            }
        }
        else{
            screen_x %= NUM_COLS;
            screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
        }
    }
    /* Save new terminal position */
    switch(tid){
        case 1:
            screen_x_t1 = screen_x;
            screen_y_t1 = screen_y;
            break;
        case 2:
            screen_x_t2 = screen_x;
            screen_y_t2 = screen_y;
            break;
        case 3:
            screen_x_t3 = screen_x;
            screen_y_t3 = screen_y;
            break;
        default:
            break;
    }
    // cli();
    // if (loc == get_term_num()) {
        // memcpy(video_mem, cur_term_loc,  NUM_ROWS * NUM_COLS * 2);
    // }
    // sti();
    update_cursor();
    
}





















/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console
 * Side Effects: changes screen x and y positions */
void putc(uint8_t c) {
    char* cur_term_loc;
    int tid;
    tid = get_term_num();
    cur_term_loc = video_mem_t1;
    /* Get current terminal position */
    switch(tid){
        case 1:
            cur_term_loc = video_mem_t1;
            screen_x = screen_x_t1;
            screen_y = screen_y_t1;
            ATTRIB = ATTRIB_1;
            break;
        case 2:
            cur_term_loc = video_mem_t2;
            screen_x = screen_x_t2;
            screen_y = screen_y_t2;
            ATTRIB = ATTRIB_2;
            break;
        case 3:
            cur_term_loc = video_mem_t3;
            screen_x = screen_x_t3;
            screen_y = screen_y_t3;
            ATTRIB = ATTRIB_3;
            break;
        default:
            break;
    }
    /*backspace is pressed*/
    if(c == 8){
        /*only do a backspace if you are not at the top left*/
        if(NUM_COLS * screen_y + screen_x>0){
            /*replace character to the left of cursor with null */
            *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x - 1) << 1)) = 0;
            *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x - 1) << 1) + 1) = ATTRIB;

            /*update the cursor position*/
            screen_x--;
        }
    }
    /*enter is pressed*/
    else if(c == '\n' || c == '\r') {
        /*no need for scrolling if you are not at the last line*/
        if(screen_y < NUM_ROWS-1){
            /*update cursor position to start of next line*/
            screen_y++;
            screen_x = 0;
        }
        /*if you are at the last line (NUM_ROWS-1), then you must scroll*/
        else{
            shift_up(cur_term_loc);
        }
    } 
    /*any other character*/
    else {
        /*put the character into video memory*/
        *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;

        /*if you have reached the end of the line*/
        if (screen_x == NUM_COLS) {
            /*if you are not on the last row*/
            if(screen_y < NUM_ROWS-1){
                /*update the cursor to be at the start of next line*/
                screen_x = 0; 
                screen_y++;
                
            }
            /*you are at the last row. displaying this character must cause scrolling. */
            else{
                shift_up(cur_term_loc);
            }
        }
        else{
            screen_x %= NUM_COLS;
            screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
        }
    }
    /* Save new terminal position */
    switch(tid){
        case 1:
            screen_x_t1 = screen_x;
            screen_y_t1 = screen_y;
            break;
        case 2:
            screen_x_t2 = screen_x;
            screen_y_t2 = screen_y;
            break;
        case 3:
            screen_x_t3 = screen_x;
            screen_y_t3 = screen_y;
            break;
        default:
            break;
    }
    // cli();
    memcpy(video_mem, cur_term_loc,  NUM_ROWS * NUM_COLS * 2);
    update_cursor();
    // sti();
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/* void update_cursor(void)
 * Inputs: none
 * Return Value: none
 * Function: updates cursor position based on x and y coordinates of screen
 * Pulled from OSDev.org
 */
void update_cursor()
{
    /*calculate current position based on global variables*/
	uint16_t pos;// = screen_y * NUM_COLS + screen_x;
    int tid = get_term_num();
    switch(tid){
        case 1:
            pos = screen_y_t1 * NUM_COLS + screen_x_t1;
            break;
        case 2:
            pos = screen_y_t2 * NUM_COLS + screen_x_t2;
            break;
        case 3:
            pos = screen_y_t3 * NUM_COLS + screen_x_t3;
            break;
        default:
            break;
    }
 
    /*write to ports 0x3D4 and 0x3D5*/
	outb(0x0F, 0x3D4);
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}

/* void shift_up(void)
 * Inputs: none
 * Return Value: none
 * Function: scrolls the screen up. Called when you want 
 *           to type beyond the last line or you hit enter
 *           when youre at the last line
 * Side Effects: updates video memory and changes global variables screen x and y
 */
void shift_up(char* cur_term_loc){
    /*scrolling up means shifting rows of text up one by one*/

    /*looping variables*/
    int i, j;

    /*loop through each row and column to shift each row upwards.
        Do not loop through first row (i = 0) because you just want to overwrrite it */
    for(i = 1; i<NUM_ROWS; i++){
        for(j = 0; j < NUM_COLS; j++){
            /*replace row above with row below*/
            *(uint8_t *)(cur_term_loc + ((NUM_COLS * (i-1) + j) << 1)) = *(uint8_t *)(cur_term_loc + ((NUM_COLS * i + j) << 1)) ;
            *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
            /*clear the last row*/
            if(i == NUM_ROWS-1){
                *(uint8_t *)(cur_term_loc + ((NUM_COLS * (i) + j) << 1)) = 0;
                *(uint8_t *)(cur_term_loc + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
            }
        }
    }
    /*update x and y positions*/
    screen_x = 0; 
    screen_y = NUM_ROWS-1;
}
// int switch_terminals (int term_num) {
    
//     if (term_num < 1 || term_num > 3) {
//         return -1;
//     }
    
//     //call paging function that points video memory to repective terminal address
//     if (term_num == 1) {
//         // call function that points video memory to terminal 1
//         terminal_number = 1;
//         memcpy(video_mem, video_mem_t1,  NUM_ROWS * NUM_COLS * 2);
//         update_cursor();

//         // term_1_display();
//     }
//     else if (term_num == 2) {
//         // call function that points video memory to terminal 2
//         terminal_number = 2;
//         memcpy(video_mem, video_mem_t2,  NUM_ROWS * NUM_COLS * 2);
//         update_cursor();

//         // term_2_display();
//     }
//     else {
//         // call function that points video memory to terminal 3
//         terminal_number = 3;
//         memcpy(video_mem, video_mem_t3,  NUM_ROWS * NUM_COLS * 2);
//         update_cursor();

//         // term_3_display();
//     }
    
//     return 0;
// }
