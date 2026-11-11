#include <mega128.h>
#include <glcd.h>
#include <font5x7.h>
#include "board_config.h"
#include "display.h"

unsigned char display_init(void)
{
    GLCDINIT_t init_data;

    /* PD7 is shared by Nokia-BL and LCD-BL on the supplied schematic. */
    NOKIA_BL_DDR=1;
    NOKIA_BL=1;

    init_data.font=font5x7;
    init_data.readxmem=0;
    init_data.writexmem=0;
    init_data.temp_coef=0;
    init_data.bias=4;
    init_data.vlcd=NOKIA_CONTRAST;

    return (unsigned char)glcd_init(&init_data);
}

void display_clear(void)
{
    glcd_clear();
}

void display_text(unsigned char x, unsigned char y, flash char *text)
{
    glcd_outtextxyf(x,y,text);
}

void display_rect(unsigned char x, unsigned char y,
                  unsigned char width, unsigned char height)
{
    if (width==0 || height==0 || x>=NOKIA_WIDTH || y>=NOKIA_HEIGHT) return;
    if ((unsigned int)x+width>NOKIA_WIDTH) width=NOKIA_WIDTH-x;
    if ((unsigned int)y+height>NOKIA_HEIGHT) height=NOKIA_HEIGHT-y;
    glcd_rectangle(x,y,x+width-1,y+height-1);
}

void display_fill_rect(unsigned char x, unsigned char y,
                       unsigned char width, unsigned char height)
{
    if (width==0 || height==0 || x>=NOKIA_WIDTH || y>=NOKIA_HEIGHT) return;
    if ((unsigned int)x+width>NOKIA_WIDTH) width=NOKIA_WIDTH-x;
    if ((unsigned int)y+height>NOKIA_HEIGHT) height=NOKIA_HEIGHT-y;
    glcd_bar(x,y,x+width-1,y+height-1);
}
