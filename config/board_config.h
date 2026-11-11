#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <mega128.h>

/* Verified from docs/so_do_chan.jpeg; Nokia RST/CE differ from the proposal. */
#define NOKIA_WIDTH 84
#define NOKIA_HEIGHT 48
#define NOKIA_FB_SIZE 504

/* CVAVR bit selectors: use directly, e.g. NOKIA_DC = 1. */
#define NOKIA_DC PORTC.0
#define NOKIA_DC_DDR DDRC.0
#define NOKIA_DIN PORTG.1
#define NOKIA_DIN_DDR DDRG.1
#define NOKIA_CLK PORTG.2
#define NOKIA_CLK_DDR DDRG.2
#define NOKIA_RST PORTG.3
#define NOKIA_RST_DDR DDRG.3
#define NOKIA_CS PORTG.4
#define NOKIA_CS_DDR DDRG.4
#define NOKIA_BL PORTD.7
#define NOKIA_BL_DDR DDRD.7

/* PCD8544 Vop, adjust after checking the actual LCD and supply. */
#define NOKIA_CONTRAST 0x40

#endif
