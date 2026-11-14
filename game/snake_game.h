#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "../drivers/keypad.h"
#include "../config/board_config.h"
#include "game_menu.h"

#define SNAKE_INITIAL_LENGTH 3
#define SNAKE_MAX_LENGTH (GRID_COLS*GRID_ROWS)

typedef struct {
    unsigned char x;
    unsigned char y;
} SnakePoint;

typedef enum {
    FOOD_CIRCLE=0,
    FOOD_SQUARE,
    FOOD_STAR
} FoodType;

typedef enum {
    SNAKE_RUNNING=0,
    SNAKE_PAUSED,
    SNAKE_GAME_OVER,
    SNAKE_WIN,
    SNAKE_TIME_UP
} SnakeState;

void snake_init(void);
void snake_set_level(GameLevel level);
unsigned char snake_can_wrap(void);
void snake_handle_key(KeyEvent key);
unsigned char snake_step(void);
SnakeState snake_state(void);
SnakePoint snake_segment(unsigned char index);
unsigned char snake_length(void);
SnakePoint snake_food(void);
FoodType snake_get_food_type(void);
FoodType snake_food_type_from_roll(unsigned char roll);
unsigned int snake_get_score(void);
void snake_mix_entropy(unsigned long now_ms);
void snake_time_up(void);
void snake_next_head(signed int *x,signed int *y);
unsigned char snake_random_index(unsigned char count);

#endif
