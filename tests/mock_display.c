/* Host-only display mock for linking the complete gameplay implementation. */
#include "mock/mega128.h"
#include "../drivers/display.h"

void display_clear(void) {}
void display_text(unsigned char x,unsigned char y,flash char *value)
{ (void)x; (void)y; (void)value; }
void display_text_ram(unsigned char x,unsigned char y,char *value)
{ (void)x; (void)y; (void)value; }
void display_rect(unsigned char x,unsigned char y,unsigned char width,
                  unsigned char height)
{ (void)x; (void)y; (void)width; (void)height; }
void display_fill_rect(unsigned char x,unsigned char y,unsigned char width,
                       unsigned char height)
{ (void)x; (void)y; (void)width; (void)height; }
