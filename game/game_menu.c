#include "../config/board_config.h"
#include "../drivers/display.h"
#include "game_menu.h"

static GameLevel selected_level=GAME_LEVEL_EASY;
static MapId selected_map=MAP_CLASSIC;
static flash char title_text[]="SNAKE GAME";
static flash char easy_selected[] ="> EASY";
static flash char easy_plain[]    ="  EASY";
static flash char normal_selected[]="> NORMAL";
static flash char normal_plain[]   ="  NORMAL";
static flash char hard_selected[]  ="> HARD";
static flash char hard_plain[]     ="  HARD";
static flash char help_text[]="2/8:MOVE 5:OK";
static flash char map_title[]="SELECT MAP";
static flash char level_easy[]="LEVEL: EASY";
static flash char level_normal[]="LEVEL: NORMAL";
static flash char level_hard[]="LEVEL: HARD";
static flash char classic_selected[]="> CLASSIC";
static flash char classic_plain[]="  CLASSIC";
static flash char pillars_selected[]="> PILLARS";
static flash char pillars_plain[]="  PILLARS";
static flash char labyrinth_selected[]="> LABYRINTH";
static flash char labyrinth_plain[]="  LABYRINTH";

void game_menu_init(GameLevel selected)
{
    if ((unsigned int)selected>=3u) selected=GAME_LEVEL_EASY;
    selected_level=selected;
}

unsigned char game_menu_handle_key(KeyEvent key)
{
    if (key==KEY_UP) {
        if (selected_level==GAME_LEVEL_EASY) selected_level=GAME_LEVEL_HARD;
        else selected_level=(GameLevel)(selected_level-1);
    } else if (key==KEY_DOWN) {
        if (selected_level==GAME_LEVEL_HARD) selected_level=GAME_LEVEL_EASY;
        else selected_level=(GameLevel)(selected_level+1);
    } else if (key==KEY_PAUSE) return 1;
    return 0;
}

GameLevel game_menu_selected(void)
{
    return selected_level;
}

unsigned int game_get_move_period(GameLevel level)
{
    if (level==GAME_LEVEL_NORMAL) return GAME_NORMAL_MOVE_MS;
    if (level==GAME_LEVEL_HARD) return GAME_HARD_MOVE_MS;
    return GAME_EASY_MOVE_MS;
}

void game_menu_render(void)
{
    display_clear();
    display_text(0,0,title_text);
    if (selected_level==GAME_LEVEL_EASY) display_text(0,10,easy_selected);
    else display_text(0,10,easy_plain);
    if (selected_level==GAME_LEVEL_NORMAL) display_text(0,18,normal_selected);
    else display_text(0,18,normal_plain);
    if (selected_level==GAME_LEVEL_HARD) display_text(0,26,hard_selected);
    else display_text(0,26,hard_plain);
    display_text(0,40,help_text);
}

void game_map_menu_init(MapId selected)
{
    if ((unsigned int)selected>=MAP_COUNT) selected=MAP_CLASSIC;
    selected_map=selected;
}

unsigned char game_map_menu_handle_key(KeyEvent key)
{
    if (key==KEY_UP) {
        if (selected_map==MAP_CLASSIC) selected_map=MAP_LABYRINTH;
        else selected_map=(MapId)(selected_map-1);
    } else if (key==KEY_DOWN) {
        if (selected_map==MAP_LABYRINTH) selected_map=MAP_CLASSIC;
        else selected_map=(MapId)(selected_map+1);
    } else if (key==KEY_PAUSE) return 1;
    return 0;
}

MapId game_map_menu_selected(void) { return selected_map; }

void game_map_menu_render(GameLevel level)
{
    display_clear();
    display_text(0,0,map_title);
    if (level==GAME_LEVEL_NORMAL) display_text(0,8,level_normal);
    else if (level==GAME_LEVEL_HARD) display_text(0,8,level_hard);
    else display_text(0,8,level_easy);
    if (selected_map==MAP_CLASSIC) display_text(0,17,classic_selected);
    else display_text(0,17,classic_plain);
    if (selected_map==MAP_PILLARS) display_text(0,26,pillars_selected);
    else display_text(0,26,pillars_plain);
    if (selected_map==MAP_LABYRINTH) display_text(0,35,labyrinth_selected);
    else display_text(0,35,labyrinth_plain);
}
