#ifndef GAME_MODE_H
#define GAME_MODE_H

#define DYNAMIC_OBSTACLE_INTERVAL_MS 10000UL
#define DYNAMIC_OBSTACLE_MAX_COUNT 8u
#define DYNAMIC_OBSTACLE_LENGTH 3u
#define DYNAMIC_OBSTACLE_SAFE_DIST 2u

typedef enum {
    MODE_STANDARD=0,
    MODE_SURVIVAL,
    MODE_COUNT
} GameMode;

unsigned char game_mode_select(GameMode mode);
GameMode game_mode_get_current(void);

#endif
