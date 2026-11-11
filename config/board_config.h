#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <mega128.h>

/* Verified from docs/so_do_chan.jpeg and the architecture document. */
#define NOKIA_WIDTH 84
#define NOKIA_HEIGHT 48

/* GLCD owns PG0..PG4. Configure these in CVAVR project, not GPIO macros:
   DC=PG0, DIN=PG1, CLK=PG2, RST=PG3, CE/CS=PG4. */
#define NOKIA_BL PORTD.7
#define NOKIA_BL_DDR DDRD.7

/* Initial PCD8544 VLCD setting; adjust after a hardware display test. */
#define NOKIA_CONTRAST 0x40

/* Keypad: docs/so_do_chan_ban_phim.jpeg, column drive / row sense. */
#define KEY_ROW1_DDR DDRC.6
#define KEY_ROW1_PORT PORTC.6
#define KEY_ROW1_PIN PINC.6
#define KEY_ROW2_DDR DDRE.1
#define KEY_ROW2_PORT PORTE.1
#define KEY_ROW2_PIN PINE.1
#define KEY_ROW3_DDR DDRD.0
#define KEY_ROW3_PORT PORTD.0
#define KEY_ROW3_PIN PIND.0
#define KEY_COL1_DDR DDRC.4
#define KEY_COL1_PORT PORTC.4
#define KEY_COL2_DDR DDRC.5
#define KEY_COL2_PORT PORTC.5
#define KEY_COL3_DDR DDRE.0
#define KEY_COL3_PORT PORTE.0

/* Set to 1 (or define in CVAVR project) for the independent keypad test. */
#ifndef KEYPAD_TEST_MODE
#define KEYPAD_TEST_MODE 0
#endif

#ifndef LED7SEG_TEST_MODE
#define LED7SEG_TEST_MODE 0
#endif
#ifndef LCD1602_TEST_MODE
#define LCD1602_TEST_MODE 0
#endif
#ifndef LED7_LCD_TEST_MODE
#define LED7_LCD_TEST_MODE 0
#endif
#if (KEYPAD_TEST_MODE+LED7SEG_TEST_MODE+LCD1602_TEST_MODE+LED7_LCD_TEST_MODE)>1
#error Enable only one peripheral test mode at a time
#endif

/* 7SEG-CA: docs/so_do_chan_mach.jpeg. Masks avoid CVAVR bit-selector macros. */
#define LED7_DIGIT1_MASK (1u<<0) /* PF0, pin 61 */
#define LED7_DIGIT2_MASK (1u<<3) /* PF3, pin 58 */
#define LED7_DIGIT3_MASK (1u<<4) /* PF4, pin 57 */
#define LED7_DIGIT4_MASK (1u<<6) /* PF6, pin 55 */
#define LED7_SEG_A_MASK (1u<<1)  /* PF1, pin 60 */
#define LED7_SEG_B_MASK (1u<<5)  /* PF5, pin 56 */
#define LED7_SEG_C_MASK (1u<<7)  /* PA7, pin 44 */
#define LED7_SEG_D_MASK (1u<<1)  /* PD1, pin 26 */
#define LED7_SEG_E_MASK (1u<<2)  /* PD2, pin 27 */
#define LED7_SEG_F_MASK (1u<<2)  /* PF2, pin 59 */
#define LED7_SEG_G_MASK (1u<<3)  /* PD3, pin 28 */
#define LED7_SEG_DP_MASK (1u<<7) /* PF7, pin 54 */
#define LED7_DIGITS_MASK (LED7_DIGIT1_MASK|LED7_DIGIT2_MASK|LED7_DIGIT3_MASK|LED7_DIGIT4_MASK)
#define LED7_SEG_F_PORT_MASK (LED7_SEG_A_MASK|LED7_SEG_B_MASK|LED7_SEG_F_MASK|LED7_SEG_DP_MASK)
#define LED7_SEG_D_PORT_MASK (LED7_SEG_D_MASK|LED7_SEG_E_MASK|LED7_SEG_G_MASK)

/* Direct common-anode default: digit high, segment low. Verify in Proteus. */
#define LED7_DIGIT_ON_LEVEL 1
#define LED7_SEGMENT_ON_LEVEL 0
#define LED7_SCAN_MS 2UL

#define GRID_COLS 14
#define GRID_ROWS 8
#define CELL_SIZE 6
#define SNAKE_MOVE_MS 500UL

#endif
