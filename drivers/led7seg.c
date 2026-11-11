#include <mega128.h>
#include "../config/board_config.h"
#include "led7seg.h"

/* Pattern bit order: A B C D E F G. DP is always off. */
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
    /* Preserve every non-digit bit of PORTF. */
    if (LED7_DIGIT_ON_LEVEL)
        PORTF &= (unsigned char)~LED7_DIGITS_MASK;
    else
        PORTF |= LED7_DIGITS_MASK;
}

static void digit_on(unsigned char index)
{
    unsigned char mask;
    if (index==0) mask=LED7_DIGIT1_MASK;
    else if (index==1) mask=LED7_DIGIT2_MASK;
    else if (index==2) mask=LED7_DIGIT3_MASK;
    else mask=LED7_DIGIT4_MASK;

    if (LED7_DIGIT_ON_LEVEL) PORTF |= mask;
    else PORTF &= (unsigned char)~mask;
}

static void segment_out(unsigned char pattern)
{
    unsigned char f_on=0;
    unsigned char d_on=0;
    unsigned char a_on=0;

    if (pattern&0x01) f_on |= LED7_SEG_A_MASK;
    if (pattern&0x02) f_on |= LED7_SEG_B_MASK;
    if (pattern&0x04) a_on |= LED7_SEG_C_MASK;
    if (pattern&0x08) d_on |= LED7_SEG_D_MASK;
    if (pattern&0x10) d_on |= LED7_SEG_E_MASK;
    if (pattern&0x20) f_on |= LED7_SEG_F_MASK;
    if (pattern&0x40) d_on |= LED7_SEG_G_MASK;

    /* ALCD owns PA0..PA6; only PA7 is changed here. */
    DDRA |= LED7_SEG_C_MASK;
    if (LED7_SEGMENT_ON_LEVEL) {
        PORTF=(PORTF & (unsigned char)~LED7_SEG_F_PORT_MASK) | f_on;
        PORTD=(PORTD & (unsigned char)~LED7_SEG_D_PORT_MASK) | d_on;
        PORTA=(PORTA & (unsigned char)~LED7_SEG_C_MASK) | a_on;
    } else {
        PORTF=(PORTF & (unsigned char)~LED7_SEG_F_PORT_MASK) |
              ((unsigned char)~f_on & LED7_SEG_F_PORT_MASK);
        PORTD=(PORTD & (unsigned char)~LED7_SEG_D_PORT_MASK) |
              ((unsigned char)~d_on & LED7_SEG_D_PORT_MASK);
        PORTA=(PORTA & (unsigned char)~LED7_SEG_C_MASK) |
              ((unsigned char)~a_on & LED7_SEG_C_MASK);
    }
}

void led7seg_init(void)
{
    digit_off_all();
    DDRF |= LED7_DIGITS_MASK | LED7_SEG_F_PORT_MASK;
    DDRD |= LED7_SEG_D_PORT_MASK;
    DDRA |= LED7_SEG_C_MASK;
    segment_out(0);
    current_digit=0;
    scan_started=0;
    enabled=1;
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
    else {
        scan_started=1;
        current_digit=0;
    }
    segment_out(digit_pattern[digits[current_digit]]);
    digit_on(current_digit);
    last_scan_ms=now_ms;
}
