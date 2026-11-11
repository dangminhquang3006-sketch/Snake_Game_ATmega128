#include "board_config.h"
#include "nokia5110.h"

static unsigned char framebuffer[NOKIA_FB_SIZE];

/* 5x7 glyphs for the phase 1 banner and a fallback space. */
static const unsigned char font_letters[26][5] = {
    {0x7e,0x11,0x11,0x11,0x7e}, {0x7f,0x49,0x49,0x49,0x36},
    {0x3e,0x41,0x41,0x41,0x22}, {0x7f,0x41,0x41,0x22,0x1c},
    {0x7f,0x49,0x49,0x49,0x41}, {0x7f,0x09,0x09,0x09,0x01},
    {0x3e,0x41,0x49,0x49,0x7a}, {0x7f,0x08,0x08,0x08,0x7f},
    {0x00,0x41,0x7f,0x41,0x00}, {0x20,0x40,0x41,0x3f,0x01},
    {0x7f,0x08,0x14,0x22,0x41}, {0x7f,0x40,0x40,0x40,0x40},
    {0x7f,0x02,0x0c,0x02,0x7f}, {0x7f,0x04,0x08,0x10,0x7f},
    {0x3e,0x41,0x41,0x41,0x3e}, {0x7f,0x09,0x09,0x09,0x06},
    {0x3e,0x41,0x51,0x21,0x5e}, {0x7f,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7f,0x01,0x01},
    {0x3f,0x40,0x40,0x40,0x3f}, {0x1f,0x20,0x40,0x20,0x1f},
    {0x7f,0x20,0x18,0x20,0x7f}, {0x63,0x14,0x08,0x14,0x63},
    {0x03,0x04,0x78,0x04,0x03}, {0x61,0x51,0x49,0x45,0x43}
};

static void spi_byte(unsigned char value)
{
    unsigned char bit;
    for (bit=0; bit<8; ++bit) {
        NOKIA_CLK=0;
        NOKIA_DIN=(value & 0x80) ? 1 : 0;
        /* GPIO operations provide a conservative clock at 16 MHz. */
        NOKIA_CLK=1;
        value <<= 1;
    }
    NOKIA_CLK=0;
}

static void send_byte(unsigned char value, unsigned char is_data)
{
    NOKIA_DC=is_data ? 1 : 0;
    NOKIA_CS=0;
    spi_byte(value);
    NOKIA_CS=1;
}

static void command(unsigned char value) { send_byte(value,0); }
static void data(unsigned char value) { send_byte(value,1); }

void nokia_init(void)
{
    volatile unsigned int delay_count;
    NOKIA_DC_DDR=1; NOKIA_DIN_DDR=1; NOKIA_CLK_DDR=1;
    NOKIA_RST_DDR=1; NOKIA_CS_DDR=1; NOKIA_BL_DDR=1;
    NOKIA_CS=1; NOKIA_CLK=0; NOKIA_DIN=0; NOKIA_DC=0;
    NOKIA_BL=1;
    NOKIA_RST=0;
    /* Reset pulse only during initialization; not used for game timing. */
    for (delay_count=0; delay_count<1000; ++delay_count) { }
    NOKIA_RST=1;
    for (delay_count=0; delay_count<1000; ++delay_count) { }
    command(0x21);                       /* extended instructions */
    command(0x80 | NOKIA_CONTRAST);      /* Vop */
    command(0x04);                       /* temperature coefficient */
    command(0x14);                       /* bias 1:48 */
    command(0x20);                       /* basic instructions */
    command(0x0c);                       /* normal display */
    nokia_clear();
    nokia_update();
}

void nokia_clear(void)
{
    unsigned int i;
    for (i=0; i<NOKIA_FB_SIZE; ++i) framebuffer[i]=0;
}

void nokia_set_pixel(unsigned char x, unsigned char y, unsigned char on)
{
    unsigned int index;
    unsigned char mask;
    if (x>=NOKIA_WIDTH || y>=NOKIA_HEIGHT) return;
    index=(unsigned int)(y/8)*NOKIA_WIDTH+x;
    mask=(unsigned char)(1 << (y & 7));
    if (on) framebuffer[index] |= mask;
    else framebuffer[index] &= (unsigned char)~mask;
}

void nokia_rect(unsigned char x, unsigned char y, unsigned char width,
                unsigned char height, unsigned char filled)
{
    unsigned int dx,dy;
    for (dy=0; dy<height; ++dy) {
        for (dx=0; dx<width; ++dx) {
            if (filled || dx==0 || dy==0 || dx==width-1 || dy==height-1)
                if ((unsigned int)x+dx<NOKIA_WIDTH && (unsigned int)y+dy<NOKIA_HEIGHT)
                    nokia_set_pixel((unsigned char)(x+dx),(unsigned char)(y+dy),1);
        }
    }
}

void nokia_text(unsigned char x, unsigned char y, const char *str)
{
    unsigned char col,row,c,bits;
    while (*str && x<=NOKIA_WIDTH-6) {
        c=(unsigned char)*str++;
        for (col=0; col<5; ++col) {
            bits=(c>='A' && c<='Z') ? font_letters[c-'A'][col] : 0;
            for (row=0; row<7; ++row)
                nokia_set_pixel((unsigned char)(x+col),(unsigned char)(y+row),
                                (unsigned char)((bits>>row)&1));
        }
        for (row=0; row<7; ++row) nokia_set_pixel((unsigned char)(x+5),(unsigned char)(y+row),0);
        x+=6;
    }
}

void nokia_update(void)
{
    unsigned int i;
    command(0x40); /* Y=0 */
    command(0x80); /* X=0 */
    for (i=0; i<NOKIA_FB_SIZE; ++i) data(framebuffer[i]);
}
