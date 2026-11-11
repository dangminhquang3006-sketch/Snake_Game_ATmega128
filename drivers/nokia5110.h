#ifndef NOKIA5110_H
#define NOKIA5110_H

void nokia_init(void);
void nokia_clear(void);
void nokia_set_pixel(unsigned char x, unsigned char y, unsigned char on);
void nokia_rect(unsigned char x, unsigned char y, unsigned char width,
                unsigned char height, unsigned char filled);
void nokia_text(unsigned char x, unsigned char y, const char *text);
void nokia_update(void);

#endif
