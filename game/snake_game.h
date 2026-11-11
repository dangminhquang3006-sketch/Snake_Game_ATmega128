#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "../drivers/keypad.h"
#include "../config/board_config.h"

#define SNAKE_INITIAL_LENGTH 3
#define SNAKE_MAX_LENGTH (GRID_COLS*GRID_ROWS)

typedef struct {
    unsigned char x;
    unsigned char y;
} SnakePoint;

typedef enum {
    SNAKE_RUNNING=0,
    SNAKE_PAUSED,
    SNAKE_GAME_OVER,
    SNAKE_WIN
} SnakeState;

void snake_init(void);
void snake_handle_key(KeyEvent key);
unsigned char snake_step(void);
SnakeState snake_state(void);
SnakePoint snake_segment(unsigned char index);
unsigned char snake_length(void);
SnakePoint snake_food(void);
unsigned int snake_get_score(void);
void snake_mix_entropy(unsigned long now_ms);

#endif
