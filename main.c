#include <mega128.h>
#include "../config/board_config.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "drivers/keypad.h"
#include "game/snake_game.h"
#include "game/game_render.h"

#if KEYPAD_TEST_MODE
static flash char keypad_title[]="KEYPAD TEST";
static char key_line[]="KEY: 0";
static char row_line[]="ROW: 0";
static char col_line[]="COL: 0";
static char count_line[]="COUNT: 000";
static unsigned int press_count=0;

static void show_key(unsigned char key, unsigned char row, unsigned char col)
{
    key_line[5]=(char)('0'+key);
    row_line[5]=(char)('0'+row);
    col_line[5]=(char)('0'+col);
    if (press_count<999) ++press_count;
    count_line[7]=(char)('0'+press_count/100);
    count_line[8]=(char)('0'+(press_count/10)%10);
    count_line[9]=(char)('0'+press_count%10);
    display_clear();
    display_text(0,0,keypad_title);
    display_text_ram(0,10,key_line);
    display_text_ram(0,20,row_line);
    display_text_ram(0,30,col_line);
    display_text_ram(0,40,count_line);
}
#endif

void main(void)
{
    unsigned long now_ms;
#if KEYPAD_TEST_MODE
    unsigned char position,row,col;
#else
    unsigned long last_move_ms;
    KeyEvent key;
    SnakeState before;
    unsigned char redraw;
#endif

    if (!display_init()) {
        while (1) { }
    }
    keypad_init();
#if !KEYPAD_TEST_MODE
    snake_init();
#endif
    timer_init();
    #asm("sei")
#if KEYPAD_TEST_MODE
    display_clear();
    display_text(0,0,keypad_title);
    while (1) {
        now_ms=timer_get_ms();
        keypad_update(now_ms);
        position=keypad_get_press(&row,&col);
        if (position!=0) show_key(position,row,col);
    }
#else
    last_move_ms=timer_get_ms();
    game_render();
    while (1) {
        now_ms=timer_get_ms();
        keypad_update(now_ms);
        redraw=0;
        key=keypad_get_event();
        if (key!=KEY_NONE) {
            before=snake_state();
            snake_handle_key(key);
            if (before!=snake_state()) {
                redraw=1;
                if (snake_state()==SNAKE_RUNNING) last_move_ms=now_ms;
            }
        }
        if (snake_state()==SNAKE_RUNNING &&
            (unsigned long)(now_ms-last_move_ms)>=SNAKE_MOVE_MS) {
            last_move_ms=now_ms; /* Drop overdue ticks: at most one step. */
            redraw=snake_step();
        }
        if (redraw) game_render();
    }
#endif
}
