#ifndef LED7SEG_H
#define LED7SEG_H

void led7seg_init(void);
void led7seg_set_number(unsigned int number);
void led7seg_scan(unsigned long now_ms);
void led7seg_blank(void);
void led7seg_resume(void);
void led7seg_clear(void);

#endif
