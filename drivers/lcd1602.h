#ifndef LCD1602_H
#define LCD1602_H

void lcd1602_init(void);
void lcd1602_show_playing(void);
void lcd1602_show_paused(void);
void lcd1602_show_result(unsigned char won, unsigned int score);
void lcd1602_show_test_ready(void);
unsigned char lcd1602_is_dirty(void);
void lcd1602_service(void);

#endif
