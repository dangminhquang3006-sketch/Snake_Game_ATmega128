#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "../drivers/keypad.h"

typedef enum {
    GAME_LEVEL_EASY=0,
    GAME_LEVEL_NORMAL,
    GAME_LEVEL_HARD
} GameLevel;

void game_menu_init(GameLevel selected);
unsigned char game_menu_handle_key(KeyEvent key);
GameLevel game_menu_selected(void);
unsigned int game_get_move_period(GameLevel level);
void game_menu_render(void);

#endif
