#include "game_mode.h"

static GameMode current_mode=MODE_STANDARD;

unsigned char game_mode_select(GameMode mode)
{
    if ((unsigned int)mode>=MODE_COUNT) return 0;
    current_mode=mode;
    return 1;
}

GameMode game_mode_get_current(void) { return current_mode; }
