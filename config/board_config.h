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

#define GRID_COLS 14
#define GRID_ROWS 8
#define CELL_SIZE 6
#define SNAKE_MOVE_MS 500UL

#endif
