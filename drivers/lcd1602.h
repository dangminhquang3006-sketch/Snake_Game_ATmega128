#ifndef LCD1602_H
#define LCD1602_H

#include "../game/game_menu.h"

void lcd1602_init(void);
void lcd1602_show_menu(GameLevel level, unsigned int best);
void lcd1602_show_map_menu(MapId map, unsigned int best);
void lcd1602_show_mode_menu(GameMode mode, unsigned int best);
void lcd1602_show_playing(GameLevel level, MapId map);
void lcd1602_show_paused(GameLevel level, MapId map);
void lcd1602_show_result(unsigned char won, unsigned int score);
void lcd1602_show_time(unsigned char seconds, unsigned char paused);
void lcd1602_show_time_up(unsigned int score);
void lcd1602_show_survival(GameLevel level,unsigned char obstacles,
                           unsigned char paused);
void lcd1602_show_test_ready(void);
unsigned char lcd1602_is_dirty(void);
void lcd1602_service(void);

#endif
