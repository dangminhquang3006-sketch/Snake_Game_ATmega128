#ifndef BONUS_FOOD_H
#define BONUS_FOOD_H

#include "snake_game.h"

void bonus_food_reset(void);
unsigned char bonus_food_active(void);
SnakePoint bonus_food_position(void);
void bonus_food_spawn(SnakePoint point);
void bonus_food_collect(void);
unsigned char bonus_food_advance(unsigned long elapsed_ms);
unsigned long bonus_food_remaining_ms(void);

#endif
