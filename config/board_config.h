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

#endif
