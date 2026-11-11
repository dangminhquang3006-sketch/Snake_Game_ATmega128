#include <mega128.h>
#include "board_config.h"
#include "display.h"
#include "timer.h"
#include "keypad.h"
#include "snake_game.h"
#include "game_render.h"

void main(void)
{
    unsigned long now_ms, last_move_ms;
    KeyEvent key;
    SnakeState before;
    unsigned char redraw;

    if (!display_init()) {
        while (1) { }
    }
    keypad_init();
    snake_init();
    timer_init();
    #asm("sei")
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
}
