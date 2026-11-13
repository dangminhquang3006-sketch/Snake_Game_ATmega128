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
    lcd1602_show_menu(GAME_LEVEL_EASY,120); finish_update();
    assert(memcmp(screen[0],"SELECT LEVEL    ",16)==0);
    assert(memcmp(screen[1],"EASY BEST:0120  ",16)==0);
    lcd1602_show_menu(GAME_LEVEL_NORMAL,80); finish_update();
    assert(memcmp(screen[1],"NORMAL BEST:0080",16)==0);
    lcd1602_show_map_menu(MAP_PILLARS,120); finish_update();
    assert(memcmp(screen[0],"MAP: PILLARS    ",16)==0);
    assert(memcmp(screen[1],"BEST: 0120      ",16)==0);
    lcd1602_show_mode_menu(MODE_TIME_ATTACK,90); finish_update();
    assert(memcmp(screen[0],"MODE:TIME ATTACK",16)==0);
    assert(memcmp(screen[1],"BEST: 0090      ",16)==0);
    lcd1602_show_playing(GAME_LEVEL_HARD,MAP_LABYRINTH); finish_update();
    assert(memcmp(screen[0],"HARD LABYRINTH  ",16)==0);
    assert(memcmp(screen[1],"STATUS: PLAYING ",16)==0);
    lcd1602_show_paused(GAME_LEVEL_NORMAL,MAP_PILLARS); finish_update();
    assert(memcmp(screen[0],"NORMAL PILLARS  ",16)==0);
    assert(memcmp(screen[1],"GAME PAUSED     ",16)==0);
    lcd1602_show_time(60,0); finish_update();
    assert(memcmp(screen[0],"TIME: 01:00     ",16)==0);
    lcd1602_show_time(35,1); finish_update();
    assert(memcmp(screen[0],"TIME: 00:35     ",16)==0);
    assert(memcmp(screen[1],"GAME PAUSED     ",16)==0);
    lcd1602_show_time_up(120); finish_update();
    assert(memcmp(screen[0],"TIME UP!        ",16)==0);
    assert(memcmp(screen[1],"SCORE: 0120     ",16)==0);
    lcd1602_show_mode_menu(MODE_SURVIVAL,70); finish_update();
    assert(memcmp(screen[0],"MODE:SURVIVAL   ",16)==0);
    lcd1602_show_survival(GAME_LEVEL_HARD,3,0); finish_update();
    assert(memcmp(screen[0],"SURVIVAL HARD   ",16)==0);
    assert(memcmp(screen[1],"OBST:03 PLAYING ",16)==0);
    lcd1602_show_survival(GAME_LEVEL_HARD,3,1); finish_update();
    assert(memcmp(screen[0],"SURVIVAL PAUSED ",16)==0);
    assert(memcmp(screen[1],"OBSTACLES: 03   ",16)==0);
    lcd1602_show_result(0,50); finish_update();
    assert(memcmp(screen[0],"GAME OVER       ",16)==0);
    assert(memcmp(screen[1],"SCORE: 0050     ",16)==0);
    lcd1602_show_result(1,1090); finish_update();
    assert(memcmp(screen[0],"YOU WIN!        ",16)==0);
    assert(memcmp(screen[1],"SCORE: 1090     ",16)==0);
    puts("PASS: LCD four modes, survival count, countdown, pause and results");
    return 0;
}
