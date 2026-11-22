#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mock/alcd.h"
#include "../drivers/lcd1602.h"

static char screen[2][17];
static unsigned char cursor_x,cursor_y;

void lcd_init(unsigned char columns)
{
    assert(columns==16);
    memset(screen,' ',sizeof screen);
    screen[0][16]=0;
    screen[1][16]=0;
}
void lcd_gotoxy(unsigned char x,unsigned char y)
{
    assert(x<16 && y<2);
    cursor_x=x; cursor_y=y;
}
void lcd_putchar(char value) { screen[cursor_y][cursor_x]=value; }

static void finish_update(void)
{
    unsigned char count=0;
    while (lcd1602_is_dirty()) {
        lcd1602_service();
        assert(++count<=32);
    }
}

int main(void)
{
    lcd1602_init();
    lcd1602_show_menu(GAME_LEVEL_NORMAL); finish_update();
    assert(memcmp(screen[0],"SELECT LEVEL    ",16)==0);
    assert(memcmp(screen[1],"NORMAL          ",16)==0);
    lcd1602_show_menu(GAME_LEVEL_HARD); finish_update();
    assert(memcmp(screen[1],"HARD            ",16)==0);
    lcd1602_show_map_menu(MAP_PILLARS); finish_update();
    assert(memcmp(screen[0],"SELECT MAP      ",16)==0);
    assert(memcmp(screen[1],"PILLARS         ",16)==0);
    lcd1602_show_playing(GAME_LEVEL_HARD,MAP_LABYRINTH); finish_update();
    assert(memcmp(screen[0],"HARD LABYRINTH  ",16)==0);
    assert(memcmp(screen[1],"STATUS: PLAYING ",16)==0);
    lcd1602_show_obstacles(GAME_LEVEL_NORMAL,MAP_CLASSIC,1); finish_update();
    assert(memcmp(screen[0],"NORMAL CLASSIC  ",16)==0);
    assert(memcmp(screen[1],"OBST:01 PLAYING ",16)==0);
    lcd1602_show_obstacles(GAME_LEVEL_HARD,MAP_PILLARS,3); finish_update();
    assert(memcmp(screen[0],"HARD PILLARS    ",16)==0);
    assert(memcmp(screen[1],"OBST:03 PLAYING ",16)==0);
    lcd1602_show_obstacles(GAME_LEVEL_NORMAL,MAP_PILLARS,0); finish_update();
    assert(memcmp(screen[0],"NORMAL PILLARS  ",16)==0);
    assert(memcmp(screen[1],"OBST:00 PLAYING ",16)==0);
    lcd1602_show_result(0,50); finish_update();
    assert(memcmp(screen[0],"GAME OVER       ",16)==0);
    assert(memcmp(screen[1],"SCORE: 0050     ",16)==0);
    lcd1602_show_result(1,1090); finish_update();
    assert(memcmp(screen[0],"YOU WIN!        ",16)==0);
    assert(memcmp(screen[1],"SCORE: 1090     ",16)==0);
    puts("PASS: LCD level/map without high scores, obstacle count and results");
    return 0;
}
