#ifndef GAME_MODE_H
#define GAME_MODE_H

#include "../config/board_config.h"
#define DYNAMIC_OBSTACLE_MAX_COUNT 8u
#define DYNAMIC_OBSTACLE_LENGTH 3u
#define DYNAMIC_OBSTACLE_SAFE_DIST 2u

/* Legacy score groups only; gameplay always uses the obstacle rule. */
typedef enum {
    MODE_STANDARD=0,
    MODE_SURVIVAL,
    MODE_COUNT
} GameMode;


#endif
