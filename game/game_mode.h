#ifndef GAME_MODE_H
#define GAME_MODE_H

#define TIME_ATTACK_DURATION_MS 60000UL
#define BONUS_LIFETIME_MS 5000UL
#define BONUS_TRIGGER_FOODS 3u
#define BONUS_SCORE 30u
#define DYNAMIC_OBSTACLE_INTERVAL_MS 20000UL
#define DYNAMIC_OBSTACLE_MAX_COUNT 8u
#define DYNAMIC_OBSTACLE_SAFE_DIST 2u

typedef enum {
    MODE_STANDARD=0,
    MODE_BONUS,
    MODE_TIME_ATTACK,
    MODE_SURVIVAL,
    MODE_COUNT
} GameMode;

unsigned char game_mode_select(GameMode mode);
GameMode game_mode_get_current(void);
void game_mode_reset(void);
unsigned char game_mode_advance(unsigned long elapsed_ms);
unsigned long game_mode_remaining_ms(void);
unsigned char game_mode_seconds_display(void);

#endif
