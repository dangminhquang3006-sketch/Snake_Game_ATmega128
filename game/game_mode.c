#include "game_mode.h"

static GameMode current_mode=MODE_STANDARD;
static unsigned long remaining_ms=TIME_ATTACK_DURATION_MS;

unsigned char game_mode_select(GameMode mode)
{
    if ((unsigned int)mode>=MODE_COUNT) return 0;
    current_mode=mode;
    return 1;
}

GameMode game_mode_get_current(void) { return current_mode; }

void game_mode_reset(void) { remaining_ms=TIME_ATTACK_DURATION_MS; }

unsigned char game_mode_advance(unsigned long elapsed_ms)
{
    if (current_mode!=MODE_TIME_ATTACK) return 0;
    if (elapsed_ms>=remaining_ms) {
        remaining_ms=0;
        return 1;
    }
    remaining_ms-=elapsed_ms;
    return 0;
}

unsigned long game_mode_remaining_ms(void) { return remaining_ms; }

unsigned char game_mode_seconds_display(void)
{
    return (unsigned char)((remaining_ms+999UL)/1000UL);
}
