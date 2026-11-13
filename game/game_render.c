#include "../config/board_config.h"
#include "../drivers/display.h"
#include "snake_game.h"
#include "game_render.h"
#include "game_map.h"
#include "bonus_food.h"
#include "dynamic_obstacle.h"

static flash char paused_text[]="PAUSED";
static flash char over_text[]="GAME OVER";
static flash char win_text[]="YOU WIN";
static flash char time_up_text[]="TIME UP";
static flash char restart_text[]="9:RESTART";
static flash char menu_text[]="1:MENU";
static char score_text[]="SCORE: 0000";

static void render_final_score(void)
{
    unsigned int value=snake_get_score();
    score_text[7]=(char)('0'+value/1000);
    score_text[8]=(char)('0'+(value/100)%10);
    score_text[9]=(char)('0'+(value/10)%10);
    score_text[10]=(char)('0'+value%10);
    display_text_ram(0,18,score_text);
    display_text(0,31,restart_text);
    display_text(0,40,menu_text);
}

void game_render(void)
{
    unsigned char i,x,y,j;
    SnakePoint point;
    SnakeState state=snake_state();
    display_clear();
    if (state==SNAKE_GAME_OVER) {
        display_text(0,4,over_text);
        render_final_score();
        return;
    }
    if (state==SNAKE_WIN) {
        display_text(0,4,win_text);
        render_final_score();
        return;
    }
    if (state==SNAKE_TIME_UP) {
        display_text(0,4,time_up_text);
        render_final_score();
        return;
    }
    for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x)
        if (game_map_is_blocked(x,y))
            display_rect((unsigned char)(x*CELL_SIZE),
                         (unsigned char)(y*CELL_SIZE),CELL_SIZE,CELL_SIZE);
    for (i=0;i<dynamic_obstacle_get_count();++i) {
        point=dynamic_obstacle_point(i);
        for (j=0;j<5u;++j) {
            display_fill_rect((unsigned char)(point.x*CELL_SIZE+j),
                              (unsigned char)(point.y*CELL_SIZE+j),1,1);
            display_fill_rect((unsigned char)(point.x*CELL_SIZE+4u-j),
                              (unsigned char)(point.y*CELL_SIZE+j),1,1);
        }
    }
    point=snake_food();
    display_fill_rect((unsigned char)(point.x*CELL_SIZE+1),
                      (unsigned char)(point.y*CELL_SIZE+1),3,3);
    if (bonus_food_active()) {
        point=bonus_food_position();
        display_rect((unsigned char)(point.x*CELL_SIZE),
                     (unsigned char)(point.y*CELL_SIZE),5,5);
    }
    for (i=0; i<snake_length(); ++i) {
        point=snake_segment(i);
        display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                          (unsigned char)(point.y*CELL_SIZE),
                          CELL_SIZE-1,CELL_SIZE-1);
    }
    if (state==SNAKE_PAUSED) display_text(0,0,paused_text);
}
