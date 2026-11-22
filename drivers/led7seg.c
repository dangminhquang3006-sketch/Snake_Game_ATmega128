#include <mega128.h>
#include "../config/board_config.h"
#include "led7seg.h"

/* Logical pattern bits: A B C D E F G DP. DP is always off. */
static flash unsigned char digit_pattern[10]={
    0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f
};
static unsigned char digits[4]={0,0,0,0};
static unsigned char current_digit=0;
static unsigned char scan_started=0;
static unsigned char enabled=1;
static unsigned long last_scan_ms=0;
static Led7Mode display_mode=LED7_MODE_CA;
static unsigned char mode_saved=0;

/* One encoded byte, outside the former high-score ranges (through 0x039E).
   Do not initialize it on startup: a reset must preserve the saved choice. */
#define LED7_EEPROM_CA 0xa5u
#define LED7_EEPROM_CC 0x5au
#ifdef HOST_TEST
unsigned char led7seg_test_eeprom_read(void);
void led7seg_test_eeprom_write(unsigned char value);
#else
eeprom unsigned char led7_saved_mode @0x03f0;
#endif

static unsigned char mode_candidate=0;
static unsigned char mode_stable=0;
static unsigned char mode_locked=0;
static unsigned char mode_scanned=0;
static unsigned long mode_changed_at=0;
static unsigned long mode_last_scan=0;

static unsigned char mode_eeprom_read(void)
{
#ifdef HOST_TEST
    return led7seg_test_eeprom_read();
#else
    return led7_saved_mode;
#endif
}

static void mode_eeprom_write(unsigned char value)
{
#ifdef HOST_TEST
    led7seg_test_eeprom_write(value);
#else
    led7_saved_mode=value;
#endif
}

static void digit_off_all(void)
{
    /* Preserve segments and every non-digit bit of PORTF. */
    if (display_mode==LED7_MODE_CA)
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

    if (display_mode==LED7_MODE_CA) PORTF |= mask;
    else PORTF &= (unsigned char)~mask;
}

static void segment_out(unsigned char pattern)
{
    unsigned char f_bits=0;
    unsigned char d_bits=0;
    unsigned char a_bits=0;
    unsigned char levels;

    /* CA segments are active low; CC segments are active high.
       Invert the 8-bit logical pattern, not the unrelated GPIO bits. */
    levels=(display_mode==LED7_MODE_CA) ? (unsigned char)~pattern : pattern;
    if (levels&0x01u) f_bits |= LED7_SEG_A_MASK;
    if (levels&0x02u) f_bits |= LED7_SEG_B_MASK;
    if (levels&0x04u) a_bits |= LED7_SEG_C_MASK;
    if (levels&0x08u) d_bits |= LED7_SEG_D_MASK;
    if (levels&0x10u) d_bits |= LED7_SEG_E_MASK;
    if (levels&0x20u) f_bits |= LED7_SEG_F_MASK;
    if (levels&0x40u) d_bits |= LED7_SEG_G_MASK;
    if (levels&0x80u) f_bits |= LED7_SEG_DP_MASK;

    /* ALCD owns PA0..PA6; only PA7 is changed here. */
    DDRA |= LED7_SEG_C_MASK;
    PORTF=(PORTF & (unsigned char)~LED7_SEG_F_PORT_MASK) | f_bits;
    PORTD=(PORTD & (unsigned char)~LED7_SEG_D_PORT_MASK) | d_bits;
    PORTA=(PORTA & (unsigned char)~LED7_SEG_C_MASK) | a_bits;
}

Led7Mode led7seg_get_mode(void)
{
    return display_mode;
}

unsigned char led7seg_set_mode(Led7Mode mode)
{
    unsigned char encoded;
    if (mode!=LED7_MODE_CA && mode!=LED7_MODE_CC) return 0;
    if (mode==display_mode && mode_saved) return 1;

    encoded=(mode==LED7_MODE_CA) ? LED7_EEPROM_CA : LED7_EEPROM_CC;
    /* EEPROM writes are synchronous: keep all digits off during the write. */
    digit_off_all();
    mode_eeprom_write(encoded);
    scan_started=0;
    current_digit=0;
    if (mode_eeprom_read()!=encoded) {
        mode_saved=0;
        return 0;
    }
    display_mode=mode;
    mode_saved=1;
    digit_off_all();
    segment_out(0);
    return 1;
}

static void mode_buttons_update(unsigned long now_ms)
{
    unsigned char raw=0;
    if (mode_scanned && now_ms==mode_last_scan) return;
    mode_scanned=1;
    mode_last_scan=now_ms;
    if (!(PINB&LED7_BT1_MASK)) raw|=1u;
    if (!(PINB&LED7_BT2_MASK)) raw|=2u;
    if (raw!=mode_candidate) {
        mode_candidate=raw;
        mode_changed_at=now_ms;
        return;
    }
    if (raw==mode_stable ||
        (unsigned long)(now_ms-mode_changed_at)<LED7_MODE_DEBOUNCE_MS) return;
    mode_stable=raw;
    if (mode_stable==0u) mode_locked=0;
    else if (!mode_locked) {
        mode_locked=1;
        if (mode_stable==1u) led7seg_set_mode(LED7_MODE_CA);
        else if (mode_stable==2u) led7seg_set_mode(LED7_MODE_CC);
        /* Both buttons held: ignore until both are released stably. */
    }
}

void led7seg_init(void)
{
    unsigned char encoded=mode_eeprom_read();
    display_mode=(encoded==LED7_EEPROM_CC) ? LED7_MODE_CC : LED7_MODE_CA;
    mode_saved=(unsigned char)(encoded==LED7_EEPROM_CA || encoded==LED7_EEPROM_CC);
    DDRB &= (unsigned char)~LED7_MODE_BUTTONS_MASK;
    PORTB |= LED7_MODE_BUTTONS_MASK;
    mode_candidate=0; mode_stable=0; mode_locked=0; mode_scanned=0;
    mode_changed_at=0; mode_last_scan=0;
    digit_off_all();
    DDRF |= LED7_DIGITS_MASK | LED7_SEG_F_PORT_MASK;
    DDRD |= LED7_SEG_D_PORT_MASK;
    DDRA |= LED7_SEG_C_MASK;
    segment_out(0);
    current_digit=0;
    scan_started=0;
    last_scan_ms=0;
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
    mode_buttons_update(now_ms);
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
