#include <mega128.h>
#include "config/board_config.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "drivers/keypad.h"
#include "drivers/led7seg.h"
#include "drivers/lcd1602.h"
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

#if LCD1602_TEST_MODE
static flash char lcd_test_nokia[]="LCD TEST";
#endif

#if !KEYPAD_TEST_MODE && !LED7SEG_TEST_MODE && !LCD1602_TEST_MODE && !LED7_LCD_TEST_MODE
static void lcd_for_state(SnakeState state)
{
    if (state==SNAKE_RUNNING) lcd1602_show_playing();
    else if (state==SNAKE_PAUSED) lcd1602_show_paused();
    else lcd1602_show_result((unsigned char)(state==SNAKE_WIN),
                             snake_get_score());
}
#endif

void main(void)
{
    unsigned long now_ms;
#if KEYPAD_TEST_MODE
    unsigned char position,row,col;
#elif LED7SEG_TEST_MODE || LCD1602_TEST_MODE || LED7_LCD_TEST_MODE
    unsigned long last_test_change;
    unsigned char test_phase=0;
#else
    unsigned long last_move_ms;
    unsigned int shown_score;
    KeyEvent key;
    SnakeState before;
    unsigned char redraw;
#endif

#if !LED7SEG_TEST_MODE && !LED7_LCD_TEST_MODE
    if (!display_init()) while (1) { }
#endif

#if KEYPAD_TEST_MODE
    keypad_init();
#elif LED7SEG_TEST_MODE
    led7seg_init();
    led7seg_set_number(0);
#elif LCD1602_TEST_MODE
    lcd1602_init();
    lcd1602_show_test_ready();
    display_clear();
    display_text(0,0,lcd_test_nokia);
#elif LED7_LCD_TEST_MODE
    lcd1602_init();
    lcd1602_show_test_ready();
    led7seg_init();
    led7seg_set_number(1234);
#else
    lcd1602_init();
    led7seg_init();
    keypad_init();
    snake_init();
    shown_score=snake_get_score();
    led7seg_set_number(shown_score);
    lcd_for_state(snake_state());
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
#elif LED7SEG_TEST_MODE
    last_test_change=timer_get_ms();
    while (1) {
        now_ms=timer_get_ms();
        led7seg_scan(now_ms);
        if ((unsigned long)(now_ms-last_test_change)>=2000UL) {
            last_test_change=now_ms;
            test_phase=(unsigned char)((test_phase+1u)&3u);
            if (test_phase==0) led7seg_set_number(0);
            else if (test_phase==1) led7seg_set_number(1234);
            else if (test_phase==2) led7seg_set_number(5678);
            else led7seg_set_number(9999);
        }
    }
#elif LCD1602_TEST_MODE
    last_test_change=timer_get_ms();
    while (1) {
        now_ms=timer_get_ms();
        if (lcd1602_is_dirty()) lcd1602_service();
        if ((unsigned long)(now_ms-last_test_change)>=2000UL) {
            last_test_change=now_ms;
            test_phase=(unsigned char)((test_phase+1u)&3u);
            if (test_phase==0) lcd1602_show_test_ready();
            else if (test_phase==1) lcd1602_show_playing();
            else if (test_phase==2) lcd1602_show_paused();
            else lcd1602_show_result(0,50);
        }
    }
#elif LED7_LCD_TEST_MODE
    last_test_change=timer_get_ms();
    while (1) {
        now_ms=timer_get_ms();
        led7seg_scan(now_ms);
        if (lcd1602_is_dirty()) {
            led7seg_blank();
            lcd1602_service();
            led7seg_resume();
        }
        if ((unsigned long)(now_ms-last_test_change)>=2000UL) {
            last_test_change=now_ms;
            test_phase=(unsigned char)((test_phase+1u)%3u);
            if (test_phase==0) led7seg_set_number(1234);
            else if (test_phase==1) led7seg_set_number(5678);
            else led7seg_set_number(9090);
        }
    }
#else
    last_move_ms=timer_get_ms();
    led7seg_blank();
    game_render();
    led7seg_resume();
    while (1) {
        now_ms=timer_get_ms();
        led7seg_scan(now_ms);
        keypad_update(now_ms);
        redraw=0;
        key=keypad_get_event();
        if (key!=KEY_NONE) {
            before=snake_state();
            snake_mix_entropy(now_ms);
            snake_handle_key(key);
            if (key==KEY_RESTART || before!=snake_state()) {
                redraw=1;
                if (key==KEY_RESTART || snake_state()==SNAKE_RUNNING)
                    last_move_ms=now_ms;
            }
        }
        if (snake_state()==SNAKE_RUNNING &&
            (unsigned long)(now_ms-last_move_ms)>=SNAKE_MOVE_MS) {
            last_move_ms=now_ms;
            redraw=snake_step();
        }
        if (shown_score!=snake_get_score()) {
            shown_score=snake_get_score();
            led7seg_set_number(shown_score);
        }
        if (redraw) {
            lcd_for_state(snake_state());
            led7seg_blank();
            game_render();
            led7seg_resume();
        }
        if (lcd1602_is_dirty()) {
            led7seg_blank();
            lcd1602_service();
            led7seg_resume();
        }
    }
#endif
}
