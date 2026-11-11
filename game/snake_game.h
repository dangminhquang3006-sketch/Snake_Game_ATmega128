#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "../drivers/keypad.h"

#define SNAKE_LENGTH 3

typedef struct {
    unsigned char x;
    unsigned char y;
} SnakePoint;

typedef enum {
    SNAKE_RUNNING=0,
    SNAKE_PAUSED,
    SNAKE_STOPPED
} SnakeState;

void snake_init(void);
void snake_handle_key(KeyEvent key);
unsigned char snake_step(void);
SnakeState snake_state(void);
SnakePoint snake_segment(unsigned char index);

#endif
