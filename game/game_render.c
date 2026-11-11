#include "board_config.h"
#include "display.h"
#include "snake_game.h"
#include "game_render.h"

static flash char paused_text[]="PAUSE";
static flash char stopped_text[]="STOP";

void game_render(void)
{
    unsigned char i;
    SnakePoint point;
    display_clear();
    for (i=0; i<SNAKE_LENGTH; ++i) {
        point=snake_segment(i);
        display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                          (unsigned char)(point.y*CELL_SIZE),
                          CELL_SIZE,CELL_SIZE);
    }
    if (snake_state()==SNAKE_PAUSED) display_text(0,0,paused_text);
    else if (snake_state()==SNAKE_STOPPED) display_text(0,0,stopped_text);
}
