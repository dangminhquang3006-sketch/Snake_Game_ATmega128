#include <mega128.h>
#include "config/board_config.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "drivers/keypad.h"
#include "drivers/led7seg.h"
#include "drivers/lcd1602.h"
#include "game/snake_game.h"
#include "game/game_render.h"
#include "game/game_menu.h"
#include "game/game_map.h"
#include "storage/highscore_eeprom.h"

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
typedef enum { APP_STATE_LEVEL_SELECT=0, APP_STATE_MAP_SELECT, APP_STATE_GAME } AppState;
static flash char invalid_map_text[]="INVALID MAP";

static void lcd_for_state(SnakeState state, GameLevel level, MapId map)
{
    if (state==SNAKE_RUNNING) lcd1602_show_playing(level,map);
    else if (state==SNAKE_PAUSED) lcd1602_show_paused(level,map);
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
    unsigned int move_period_ms;
    GameLevel current_level;
    GameLevel previous_selection;
    MapId current_map;
    MapId previous_map;
    AppState app_state;
    KeyEvent key;
    SnakeState before;
    unsigned char redraw;
    unsigned char update_lcd;
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
    highscore_init();
    game_menu_init(GAME_LEVEL_EASY);
    current_level=GAME_LEVEL_EASY;
    current_map=MAP_CLASSIC;
    game_map_menu_init(current_map);
    move_period_ms=game_get_move_period(current_level);
    app_state=APP_STATE_LEVEL_SELECT;
    shown_score=0;
    led7seg_set_number(0);
    lcd1602_show_menu(current_level,highscore_get(current_level,MAP_CLASSIC));
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
            else if (test_phase==1) lcd1602_show_playing(GAME_LEVEL_EASY,MAP_CLASSIC);
            else if (test_phase==2) lcd1602_show_paused(GAME_LEVEL_EASY,MAP_CLASSIC);
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
    game_menu_render();
    led7seg_resume();
    while (1) {
        now_ms=timer_get_ms();
        led7seg_scan(now_ms);
        keypad_update(now_ms);
        redraw=0;
        update_lcd=0;
        key=keypad_get_event();
        if (app_state==APP_STATE_LEVEL_SELECT && key!=KEY_NONE) {
            previous_selection=game_menu_selected();
            snake_mix_entropy(now_ms);
            if (game_menu_handle_key(key)) {
                current_level=game_menu_selected();
                app_state=APP_STATE_MAP_SELECT;
                redraw=1;
                update_lcd=1;
            } else if (previous_selection!=game_menu_selected()) {
                redraw=1;
                update_lcd=1;
            }
        } else if (app_state==APP_STATE_MAP_SELECT && key!=KEY_NONE) {
            if (key==KEY_MENU) {
                app_state=APP_STATE_LEVEL_SELECT;
                redraw=1;
                update_lcd=1;
            } else {
                previous_map=game_map_menu_selected();
                snake_mix_entropy(now_ms);
                if (game_map_menu_handle_key(key)) {
                    if (game_map_select(game_map_menu_selected())) {
                        current_map=game_map_get_current();
                        move_period_ms=game_get_move_period(current_level);
                        snake_init();
                        app_state=APP_STATE_GAME;
                        last_move_ms=now_ms;
                        shown_score=0;
                        led7seg_set_number(0);
                        redraw=1;
                        update_lcd=1;
                    } else {
                        led7seg_blank();
                        display_clear();
                        display_text(0,16,invalid_map_text);
                        led7seg_resume();
                    }
                } else if (previous_map!=game_map_menu_selected()) {
                    redraw=1;
                    update_lcd=1;
                }
            }
        } else if (app_state==APP_STATE_GAME) {
            if (key==KEY_MENU &&
                (snake_state()==SNAKE_GAME_OVER || snake_state()==SNAKE_WIN)) {
                app_state=APP_STATE_LEVEL_SELECT;
                game_menu_init(current_level);
                shown_score=0;
                led7seg_set_number(0);
                redraw=1;
                update_lcd=1;
            } else if (key!=KEY_NONE) {
                before=snake_state();
                snake_mix_entropy(now_ms);
                snake_handle_key(key);
                if (key==KEY_RESTART || before!=snake_state()) {
                    redraw=1;
                    update_lcd=1;
                    if (key==KEY_RESTART || snake_state()==SNAKE_RUNNING)
                        last_move_ms=now_ms;
                }
            }
            if (app_state==APP_STATE_GAME && snake_state()==SNAKE_RUNNING &&
                (unsigned long)(now_ms-last_move_ms)>=move_period_ms) {
                last_move_ms=now_ms;
                redraw=snake_step();
                if (snake_state()==SNAKE_GAME_OVER || snake_state()==SNAKE_WIN) {
                    led7seg_blank();
                    highscore_record_result(current_level,current_map,snake_get_score());
                    led7seg_resume();
                    update_lcd=1;
                }
            }
            if (app_state==APP_STATE_GAME && shown_score!=snake_get_score()) {
                shown_score=snake_get_score();
                led7seg_set_number(shown_score);
            }
        }
        if (redraw) {
            led7seg_blank();
            if (app_state==APP_STATE_LEVEL_SELECT) game_menu_render();
            else if (app_state==APP_STATE_MAP_SELECT) game_map_menu_render(current_level);
            else game_render();
            led7seg_resume();
        }
        if (update_lcd) {
            if (app_state==APP_STATE_LEVEL_SELECT)
                lcd1602_show_menu(game_menu_selected(),
                                  highscore_get(game_menu_selected(),MAP_CLASSIC));
            else if (app_state==APP_STATE_MAP_SELECT)
                lcd1602_show_map_menu(game_map_menu_selected(),
                       highscore_get(current_level,game_map_menu_selected()));
            else lcd_for_state(snake_state(),current_level,current_map);
        }
        if (lcd1602_is_dirty()) {
            led7seg_blank();
            lcd1602_service();
            led7seg_resume();
        }
    }
#endif
}
