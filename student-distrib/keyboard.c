#include "keyboard.h"
#include "i8259.h"
#include "terminal.h"
#include "multiple_terminals.h"

#define CTRL_IDX                        0
#define ALT_IDX                         1
#define CAPS_IDX                        2
#define SHIFT_IDX                       3
#define SCAN_CTRL                       0x1D    //!also listed as 0xE0 in osdev
#define SCAN_ALT                        0x38    //!also listed as 0xE0 in osdev
#define SCAN_LEFT_SHIFT                 0x2A
#define SCAN_RIGHT_SHIFT                0x36
#define SCAN_CAPS                       0x3A
#define SCAN_F1                         0x3B	
#define SCAN_F2                         0x3C	
#define SCAN_F3                         0x3D

#define SCAN_CTRL_RELEASED              0x9D     //!right control released is also defined as 0xE0
#define SCAN_ALT_RELEASED               0xB8    //!Left alt released, right alt released not defined for code set 1
#define SCAN_LEFT_SHIFT_RELEASED        0xAA
#define SCAN_RIGHT_SHIFT_RELEASED       0xB6

#define SCAN_L_PRESSED                  0x26
#define SCAN_F1_PRESSED                 0x3B
#define SCAN_ESC_PRESSED                0x01
#define ASCII_LC_A                      97
#define ASCII_LC_Z                      122
#define LC_TO_UC                        32


/*table that uses scancode as index to convert scan code into alphanumeric characters to print to screen.
    Currently, only the first 50 scancodes are mapped, which means scancodes relating to releasing keys does not cause undefined behavior.*/
static unsigned char scan_table[0x3B] = {' ',' ' , '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                        '-', '=', 8,  9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
                        10,  0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 
                        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, 32, 0};
static unsigned int special_flags[] = {0, 0, 0, 0};
static int num_t1 = 1;
static int num_t2 = 0;
static int num_t3 = 0;

/* Unused */
void enable_scanning(void){
    outb(CMD_SCAN, KEYBOARD_PORT);
    return;
}

/* Unused */
uint32_t check_keyboard_ack(void){
    uint32_t packet = inb(KEYBOARD_PORT); // potentially use port 0x64 (command register for ps/2)
    return packet; 
}

/* void keyboard_init(void)
 * DESCRIPTION: Signals to the PIC to enable interrupts from irq #1 on master PIC.      
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS: Interrupt # 1 is unmasked on PIC. Will allow keyboard presses to be treated as interrupts. 
*/
void keyboard_init(void){
    enable_irq(1);
}

/* void keyboard_handler(void)
 * DESCRIPTION: keyboard interrupt handler that is called whenever a key is pressed on the keyboard.
 *              Reads from keyboard port (0x60), and converts packet (Scan code) into character to be echoed to screen. 
 *              Processes only alphanumeric characters and function keys (shift, alt, caps lock, backspace, tab).    
 *              
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS: Echoes a character to the screen, sets and resets flags in flag array.
 * 
*/
void keyboard_handler(void){

    /*Save flags and disable interrupts*/
    //unsigned int flag;
    //cli_and_save(flag);

    /*read from keyboard port*/
    uint32_t scan = inb(KEYBOARD_PORT);

    /*variable to store character to print*/
    unsigned int display_character;
   

    /*if any function keys are pressed, set the flag in the flag arrray.
        Special case is caps lock, which is just set to the opposite of its current value. */
    if(scan == SCAN_ALT || scan == SCAN_LEFT_SHIFT || scan == SCAN_CTRL || scan == SCAN_RIGHT_SHIFT || scan == SCAN_CAPS){
        switch(scan){
            case SCAN_ALT:
                special_flags[ALT_IDX] = 1;
                break;
            case SCAN_CTRL:
                special_flags[CTRL_IDX] = 1;
                break;
            case SCAN_LEFT_SHIFT:
                special_flags[SHIFT_IDX] = 1;
                break;
            case SCAN_RIGHT_SHIFT:
                special_flags[SHIFT_IDX] = 1;
                break;
            case SCAN_CAPS:
                special_flags[CAPS_IDX] = !special_flags[CAPS_IDX];
                break;
            default:
                break;
        }
    }
    /*if both ctrl+l or ctrl+L is pressed, clear the screen */
    else if(special_flags[CTRL_IDX] && scan == SCAN_L_PRESSED){
        clear();
        //! Do not clear buffer after ctrl L
    }
    else if(special_flags[ALT_IDX] && scan == SCAN_F1){
        //!SWITCH TO TERMINAL 1
        //term_1_display();
        num_t1++;
        // clear_pcb(NULL); // ! maybe needs to be removed? 
        switch_terminals(1);
        // setup_4kb_page(0xb9000, 0xb8000, 1);
    }
    else if(special_flags[ALT_IDX] && scan == SCAN_F2){
        //!SWITCH TO TERMINAL 2
        num_t2++;
        // if (num_t2 == 1) {
        //     execute("shell");
        // }
        // clear_pcb(NULL); // ! maybe needs to be removed? 
        switch_terminals(2);
        // setup_4kb_page(0xba000, 0xb8000, 1);
        if (num_t2 == 1) {
            send_eoi(1);
            // cli();
            execute((uint8_t*)"shell");
        }
    }
    else if(special_flags[ALT_IDX] && scan == SCAN_F3){
        //!SWITCH TO TERMINAL 3
        num_t3++;
        // clear_pcb(NULL); // ! maybe needs to be removed? 
        switch_terminals(3);
        // setup_4kb_page(0xbb000, 0xb8000, 1);
        if (num_t3 == 1) {
            send_eoi(1);
            // cli();
            execute((uint8_t*)"shell");
        }
    }
    /*run through cases of all other characters to print*/
    else{
        /*if alphanumeric character has been pressed*/
        if(scan < SCAN_F1_PRESSED && scan > SCAN_ESC_PRESSED){
            /*index into the scan_table to get the ascii char to print*/
            display_character = scan_table[scan];
            /*if an alphabet character has been pressed and it needs to be displayed as uppercase*/
            if(display_character >= ASCII_LC_A && display_character <=ASCII_LC_Z && (special_flags[CAPS_IDX] || special_flags[SHIFT_IDX])){
                //putc(display_character - LC_TO_UC);
                add_char(display_character - LC_TO_UC);
            }
            /*else if the shift or caps lock is not pressed, just display the lowercase character*/
            else if(display_character >= ASCII_LC_A && display_character <=ASCII_LC_Z){
                //putc(display_character);
                add_char(display_character);
            }
            /*otherwise, if shift is pressed and you are not displaying an alphabet character*/
            else if(special_flags[SHIFT_IDX]){
                /*display the shifted character*/
                switch (display_character){
                    case '`':
                        //putc('~');
                        add_char('~');
                        break;
                    case '1':
                        //putc('!');
                        add_char('!');
                        break;
                    case '2':
                        //putc('@');
                        add_char('@');
                        break;
                    case '3':
                        //putc('#');
                        add_char('#');
                        break;
                    case '4':
                        //putc('$');
                        add_char('$');
                        break;
                    case '5':
                        //putc(37);     //!!!!
                        add_char(37);
                        break;
                    case '6':
                        //putc('^');
                        add_char('^');
                        break;
                    case '7':
                        //putc('&');
                        add_char('&');      //!!!!
                        break;
                    case '8':
                        //putc('*');
                        add_char('*');
                        break;
                    case '9':
                        //putc('(');
                        add_char('(');
                        break;
                    case '0':
                        //putc(')');
                        add_char(')');
                        break;
                    case '-':
                        //putc('_');
                        add_char('_');
                        break;
                    case '=':
                        //putc('+');
                        add_char('+');
                        break;
                    case '[':
                        //putc('{');
                        add_char('{');
                        break;
                    case ']':
                        //putc('}');
                        add_char('}');
                        break;
                    case '\\':
                        //putc('|');
                        add_char('|');
                        break;
                    case ';':
                        //putc(':');
                        add_char(':');
                        break;
                    case '\'':
                        //putc('"');
                        add_char('"');
                        break;
                    case ',':
                        //putc('<');
                        add_char('<');
                        break;
                    case '.':
                        //putc('>');
                        add_char('>');
                        break;
                    case '/':
                        //putc('?');
                        add_char('?');
                        break;
                    default:
                        break;
                }
            }
            else{
                /*otherwise, display the special character or number without shifting*/
                //putc(display_character);
                add_char(display_character);
            }
        }
    }
    /*check if any function keys were released*/
    switch(scan){
        case SCAN_ALT_RELEASED:
            special_flags[ALT_IDX] = 0;
            break;
        case SCAN_CTRL_RELEASED:
            special_flags[CTRL_IDX] = 0;
            break;
        case SCAN_LEFT_SHIFT_RELEASED:
            special_flags[SHIFT_IDX] = 0;
            break;
         case SCAN_RIGHT_SHIFT_RELEASED:
            special_flags[SHIFT_IDX] = 0;
            break;
        default:
            break;
    }
    

    //call terminal handler here

    /*only print if scancode is within bounds (temporary)*/
    // uint32_t key = inb(0x60);   //read scan code from keyboard

    /*enable interrupts*/
    //restore_flags(flag);

    /*signal to the pic that the interrupt is over*/
    send_eoi(1);
    
}

int get_num_t2() {
    return num_t2;
}

int get_num_t3() {
    return num_t3;
}
