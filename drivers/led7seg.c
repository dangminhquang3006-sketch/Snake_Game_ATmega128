#include <mega128.h>
#include "../config/board_config.h"
#include "led7seg.h"

/* Bit order: A B C D E F G. Decimal point is always off. */
static flash unsigned char digit_pattern[10]={
    0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f
};
static unsigned char digits[4]={0,0,0,0};
static unsigned char current_digit=0;
static unsigned char scan_started=0;
static unsigned char enabled=1;
static unsigned long last_scan_ms=0;

static void digit_off_all(void)
{
    LED7_DIGIT1_PORT=!LED7_DIGIT_ON_LEVEL;
    LED7_DIGIT2_PORT=!LED7_DIGIT_ON_LEVEL;
    LED7_DIGIT3_PORT=!LED7_DIGIT_ON_LEVEL;
    LED7_DIGIT4_PORT=!LED7_DIGIT_ON_LEVEL;
}

static void digit_on(unsigned char index)
{
    if (index==0) LED7_DIGIT1_PORT=LED7_DIGIT_ON_LEVEL;
    else if (index==1) LED7_DIGIT2_PORT=LED7_DIGIT_ON_LEVEL;
    else if (index==2) LED7_DIGIT3_PORT=LED7_DIGIT_ON_LEVEL;
    else LED7_DIGIT4_PORT=LED7_DIGIT_ON_LEVEL;
}

static void segment_out(unsigned char pattern)
{
    /* ALCD uses PA0..PA6; reassert PA7 after every ALCD write. */
    LED7_SEG_C_DDR=1;
    LED7_SEG_A_PORT=(pattern&0x01) ? LED7_SEGMENT_ON_LEVEL : !LED7_SEGMENT_ON_LEVEL;
    LED7_SEG_B_PORT=(pattern&0x02) ? LED7_SEGMENT_ON_LEVEL : !LED7_SEGMENT_ON_LEVEL;
    LED7_SEG_C_PORT=(pattern&0x04) ? LED7_SEGMENT_ON_LEVEL : !LED7_SEGMENT_ON_LEVEL;
    LED7_SEG_D_PORT=(pattern&0x08) ? LED7_SEGMENT_ON_LEVEL : !LED7_SEGMENT_ON_LEVEL;
    LED7_SEG_E_PORT=(pattern&0x10) ? LED7_SEGMENT_ON_LEVEL : !LED7_SEGMENT_ON_LEVEL;
    LED7_SEG_F_PORT=(pattern&0x20) ? LED7_SEGMENT_ON_LEVEL : !LED7_SEGMENT_ON_LEVEL;
    LED7_SEG_G_PORT=(pattern&0x40) ? LED7_SEGMENT_ON_LEVEL : !LED7_SEGMENT_ON_LEVEL;
    LED7_SEG_DP_PORT=!LED7_SEGMENT_ON_LEVEL;
}

void led7seg_init(void)
{
    digit_off_all();
    LED7_DIGIT1_DDR=1; LED7_DIGIT2_DDR=1;
    LED7_DIGIT3_DDR=1; LED7_DIGIT4_DDR=1;
    LED7_SEG_A_DDR=1; LED7_SEG_B_DDR=1;
    LED7_SEG_C_DDR=1; LED7_SEG_D_DDR=1;
    LED7_SEG_E_DDR=1; LED7_SEG_F_DDR=1;
    LED7_SEG_G_DDR=1; LED7_SEG_DP_DDR=1;
    segment_out(0);
    current_digit=0; scan_started=0; enabled=1;
    led7seg_set_number(0);
}

void led7seg_set_number(unsigned int number)
{
    if (number>9999u) number=9999u;
    digits[0]=(unsigned char)(number/1000u);
    digits[1]=(unsigned char)((number/100u)%10u);
    digits[2]=(unsigned char)((number/10u)%10u);
    digits[3]=(unsigned char)(number%10u);
    enabled=1;
}

void led7seg_blank(void)
{
    digit_off_all();
}

void led7seg_resume(void)
{
    digit_off_all();
    if (enabled && scan_started) {
        segment_out(digit_pattern[digits[current_digit]]);
        digit_on(current_digit);
    }
}

void led7seg_clear(void)
{
    enabled=0;
    digit_off_all();
    segment_out(0);
}

void led7seg_scan(unsigned long now_ms)
{
    if (!enabled) return;
    if (scan_started && (unsigned long)(now_ms-last_scan_ms)<LED7_SCAN_MS) return;
    digit_off_all();
    if (scan_started) current_digit=(unsigned char)((current_digit+1u)&3u);
    else { scan_started=1; current_digit=0; }
    segment_out(digit_pattern[digits[current_digit]]);
    digit_on(current_digit);
    last_scan_ms=now_ms;
}
