#ifndef HIGHSCORE_EEPROM_H
#define HIGHSCORE_EEPROM_H

#include "../game/game_menu.h"

typedef enum {
    HIGHSCORE_UNCHANGED=0,
    HIGHSCORE_SAVED,
    HIGHSCORE_SAVE_FAILED
} HighscoreSaveResult;

void highscore_init(void);
unsigned int highscore_get(GameLevel level);
HighscoreSaveResult highscore_record_result(GameLevel level,
                                            unsigned int score);

#endif
