#ifndef DISPLAY_H
#define DISPLAY_H

/* GLCD adapter for the phase 1 Nokia screen test. */
unsigned char display_init(void);
void display_clear(void);
void display_text(unsigned char x, unsigned char y, flash char *text);
void display_text_ram(unsigned char x, unsigned char y, char *text);
void display_rect(unsigned char x, unsigned char y,
                  unsigned char width, unsigned char height);
void display_fill_rect(unsigned char x, unsigned char y,
                       unsigned char width, unsigned char height);

#endif
