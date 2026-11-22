#ifndef LED7SEG_H
#define LED7SEG_H

typedef enum {
    LED7_MODE_CA=0,
    LED7_MODE_CC
} Led7Mode;

/* Init restores the selected polarity; an unrecognized EEPROM byte uses CA. */
void led7seg_init(void);
Led7Mode led7seg_get_mode(void);
/* Returns 1 when the selection is already saved or was written and verified. */
unsigned char led7seg_set_mode(Led7Mode mode);
void led7seg_set_number(unsigned int number);
/* Services BT1/BT2 and scans one digit when due. */
void led7seg_scan(unsigned long now_ms);
void led7seg_blank(void);
void led7seg_resume(void);
void led7seg_clear(void);

#endif
