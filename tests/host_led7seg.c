#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "mock/mega128.h"
#include "../config/board_config.h"
#include "../drivers/led7seg.h"

extern unsigned char led7_mock_eeprom;
extern unsigned int led7_mock_eeprom_writes;
extern unsigned char led7_mock_eeprom_fail;

/* Expected electrical levels are reconstructed from GPIO pins, not internals. */
static unsigned char output_levels(void)
{
    unsigned char value=0;
    if (PORTF&LED7_SEG_A_MASK) value|=0x01;
    if (PORTF&LED7_SEG_B_MASK) value|=0x02;
    if (PORTA&LED7_SEG_C_MASK) value|=0x04;
    if (PORTD&LED7_SEG_D_MASK) value|=0x08;
    if (PORTD&LED7_SEG_E_MASK) value|=0x10;
    if (PORTF&LED7_SEG_F_MASK) value|=0x20;
    if (PORTD&LED7_SEG_G_MASK) value|=0x40;
    if (PORTF&LED7_SEG_DP_MASK) value|=0x80;
    return value;
}

static void reset_gpio(unsigned char stored)
{
    PORTA=0x55; PORTD=0xa1; PORTF=0x96;
    PORTB=0x91; DDRB=0xff; DDRA=0; DDRD=0; DDRF=0;
    PINB=0xff;
    led7_mock_eeprom=stored;
    led7_mock_eeprom_writes=0;
    led7_mock_eeprom_fail=0;
    led7seg_init();
    assert(DDRB==(unsigned char)(0xffu & (unsigned char)~LED7_MODE_BUTTONS_MASK));
    assert(PORTB==(unsigned char)(0x91u|LED7_MODE_BUTTONS_MASK));
    assert((PORTA&0x7fu)==0x55u);
    assert((PORTD&(unsigned char)~LED7_SEG_D_PORT_MASK)==0xa1u);
    assert(led7_mock_eeprom_writes==0u);
}

static void check_digit(Led7Mode mode,unsigned char digit,unsigned char pattern)
{
    static const unsigned char masks[4]={1u,8u,16u,64u};
    unsigned char active=(unsigned char)(PORTF&LED7_DIGITS_MASK);
    unsigned char expected=(mode==LED7_MODE_CA) ? masks[digit] :
        (unsigned char)(LED7_DIGITS_MASK & (unsigned char)~masks[digit]);
    unsigned char levels=(mode==LED7_MODE_CA) ? (unsigned char)~pattern : pattern;
    assert(active==expected);
    assert(output_levels()==levels);
    assert((PORTA&0x7fu)==0x55u);
    assert((PORTD&(unsigned char)~LED7_SEG_D_PORT_MASK)==0xa1u);
}

static void check_patterns(void)
{
    static const unsigned char patterns[10]={
        0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f
    };
    unsigned char type,n,i;
    unsigned long now;
    for (type=0;type<2u;++type) {
        for (n=0;n<10u;++n) {
            reset_gpio(type==0u ? 0xa5u : 0x5au);
            assert(led7seg_get_mode()==(Led7Mode)type);
            led7seg_set_number((unsigned int)(1111u*n));
            for (i=0;i<8u;++i) {
                now=(unsigned long)i*2u;
                led7seg_scan(now);
                check_digit((Led7Mode)type,(unsigned char)(i&3u),patterns[n]);
            }
            assert(led7_mock_eeprom_writes==0u);
        }
    }
    reset_gpio(0x5a);
    led7seg_set_number(1234);
    for (i=0;i<4u;++i) {
        led7seg_scan((unsigned long)i*2u);
        check_digit(LED7_MODE_CC,i,patterns[i+1u]);
    }
    led7seg_blank();
    assert((PORTF&LED7_DIGITS_MASK)==LED7_DIGITS_MASK);
    led7seg_resume();
    check_digit(LED7_MODE_CC,3,patterns[4]);
    led7seg_clear();
    assert((PORTF&LED7_DIGITS_MASK)==LED7_DIGITS_MASK);
    assert(output_levels()==0u);
    led7seg_scan(100);
    assert((PORTF&LED7_DIGITS_MASK)==LED7_DIGITS_MASK);
    led7seg_set_number(99999u);
    led7seg_scan(102);
    check_digit(LED7_MODE_CC,0,patterns[9]);
    /* Switch polarity while a nonzero four-digit value is being displayed. */
    reset_gpio(0xa5);
    led7seg_set_number(1234);
    led7seg_scan(0);
    led7seg_scan(2);
    assert(led7seg_set_mode(LED7_MODE_CC)==1u);
    for (i=0;i<4u;++i) {
        led7seg_scan(3UL+(unsigned long)i*2u);
        check_digit(LED7_MODE_CC,i,patterns[i+1u]);
    }
    assert(led7seg_set_mode(LED7_MODE_CA)==1u);
    for (i=0;i<4u;++i) {
        led7seg_scan(11UL+(unsigned long)i*2u);
        check_digit(LED7_MODE_CA,i,patterns[i+1u]);
    }
    assert(led7_mock_eeprom_writes==2u);
}

static void check_persistence(void)
{
    static const unsigned char invalid[6]={0xff,0,1,0xfe,0xa6,0x5b};
    unsigned char i;
    for (i=0;i<6u;++i) {
        reset_gpio(invalid[i]);
        assert(led7seg_get_mode()==LED7_MODE_CA);
        assert(led7seg_set_mode(LED7_MODE_CA)==1u);
        assert(led7_mock_eeprom==0xa5u && led7_mock_eeprom_writes==1u);
        assert(led7seg_set_mode(LED7_MODE_CA)==1u);
        assert(led7_mock_eeprom_writes==1u);
    }
    reset_gpio(0xa5);
    assert(led7seg_set_mode(LED7_MODE_CC)==1u);
    assert(led7_mock_eeprom==0x5au && led7_mock_eeprom_writes==1u);
    assert(led7seg_set_mode(LED7_MODE_CC)==1u && led7_mock_eeprom_writes==1u);
    /* A power cycle loses GPIO/RAM state but retains the EEPROM byte. */
    PORTF=0; PORTD=0xa1; PORTA=0x55; PINB=0xff;
    led7seg_init();
    assert(led7seg_get_mode()==LED7_MODE_CC);
    assert(led7_mock_eeprom_writes==1u);
    assert(led7seg_set_mode((Led7Mode)2)==0u);
    assert(led7_mock_eeprom==0x5au && led7_mock_eeprom_writes==1u);
    led7_mock_eeprom_fail=1;
    assert(led7seg_set_mode(LED7_MODE_CA)==0u);
    assert(led7seg_get_mode()==LED7_MODE_CC);
    led7_mock_eeprom_fail=0;
    assert(led7seg_set_mode(LED7_MODE_CA)==1u);
    assert(led7_mock_eeprom==0xa5u && led7seg_get_mode()==LED7_MODE_CA);
}

static void check_buttons(void)
{
    reset_gpio(0xa5);
    PINB=(unsigned char)(0xffu & (unsigned char)~LED7_BT2_MASK);
    led7seg_scan(100);
    led7seg_scan(119);
    assert(led7seg_get_mode()==LED7_MODE_CA && led7_mock_eeprom_writes==0u);
    PINB=0xff; led7seg_scan(120); /* Contact bounce starts the timer again. */
    PINB=(unsigned char)(0xffu & (unsigned char)~LED7_BT2_MASK);
    led7seg_scan(121); led7seg_scan(140);
    assert(led7seg_get_mode()==LED7_MODE_CA);
    led7seg_scan(141);
    assert(led7seg_get_mode()==LED7_MODE_CC && led7_mock_eeprom_writes==1u);
    check_digit(LED7_MODE_CC,0,0x3f);
    led7seg_scan(150);
    assert(led7_mock_eeprom_writes==1u);
    /* Moving directly to the other held button does not create a new press. */
    PINB=(unsigned char)(0xffu & (unsigned char)~LED7_BT1_MASK);
    led7seg_scan(160); led7seg_scan(180);
    assert(led7seg_get_mode()==LED7_MODE_CC && led7_mock_eeprom_writes==1u);
    PINB=0xff; led7seg_scan(190); led7seg_scan(210);
    PINB=(unsigned char)(0xffu & (unsigned char)~LED7_BT1_MASK);
    led7seg_scan(220); led7seg_scan(240);
    assert(led7seg_get_mode()==LED7_MODE_CA && led7_mock_eeprom_writes==2u);
    PINB=0xff; led7seg_scan(250); led7seg_scan(270);
    PINB=(unsigned char)(0xffu & (unsigned char)~LED7_MODE_BUTTONS_MASK);
    led7seg_scan(280); led7seg_scan(300);
    assert(led7seg_get_mode()==LED7_MODE_CA && led7_mock_eeprom_writes==2u);
    PINB=0xff; led7seg_scan(310); led7seg_scan(330);
    led7seg_clear();
    PINB=(unsigned char)(0xffu & (unsigned char)~LED7_BT2_MASK);
    led7seg_scan(340); led7seg_scan(360);
    assert(led7seg_get_mode()==LED7_MODE_CC && led7_mock_eeprom_writes==3u);
    assert((PORTF&LED7_DIGITS_MASK)==LED7_DIGITS_MASK);
    assert(output_levels()==0u); /* Changing mode must not re-enable cleared LEDs. */
    reset_gpio(0xa5);
    PINB=(unsigned char)(0xffu & (unsigned char)~LED7_BT2_MASK);
    led7seg_scan(ULONG_MAX-10UL);
    led7seg_scan(9UL);
    assert(led7seg_get_mode()==LED7_MODE_CC);
}

int main(void)
{
    check_patterns();
    check_persistence();
    check_buttons();
    puts("PASS: CA/CC electrical patterns, digit polarity, shared GPIO, BT debounce and EEPROM restore");
    return 0;
}
