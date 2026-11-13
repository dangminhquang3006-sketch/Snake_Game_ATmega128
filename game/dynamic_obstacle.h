#ifndef DYNAMIC_OBSTACLE_H
#define DYNAMIC_OBSTACLE_H

#include "snake_game.h"

void dynamic_obstacle_reset(void);
unsigned char dynamic_obstacle_update(unsigned long elapsed_running_ms);
unsigned char dynamic_obstacle_is_blocked(signed int x,signed int y);
unsigned char dynamic_obstacle_get_count(void);
unsigned char dynamic_obstacle_playable_cells(void);
unsigned char dynamic_obstacle_candidate_valid(unsigned char x,unsigned char y);
SnakePoint dynamic_obstacle_point(unsigned char index);

#endif
