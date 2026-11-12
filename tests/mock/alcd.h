#ifndef HOST_MOCK_ALCD_H
#define HOST_MOCK_ALCD_H
void lcd_init(unsigned char columns);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_putchar(char value);
#endif
