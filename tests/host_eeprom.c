#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mock/mega128.h"
#include "../game/game_menu.h"
#include "../storage/highscore_eeprom.h"
#include "../drivers/display.h"

extern unsigned char hs_slot_a[13],hs_slot_b[13];
extern unsigned char hs_v2_a[25],hs_v2_b[25];
void display_clear(void) {}
void display_text(unsigned char x,unsigned char y,char *value)
{ (void)x; (void)y; (void)value; }

static unsigned char crc(unsigned char *data,unsigned char count)
{
    unsigned char result=0,i,j;
    for (i=0;i<count;++i) {
        result^=data[i];
        for (j=0;j<8;++j)
            result=(unsigned char)((result<<1)^((result&0x80u)?7u:0u));
    }
    return result;
}

static void make_v1(unsigned char *slot,unsigned int seq,
                    unsigned int easy,unsigned int normal,unsigned int hard)
{
    unsigned int scores[3];
    unsigned char i;
    scores[0]=easy; scores[1]=normal; scores[2]=hard;
    memset(slot,0,13);
    slot[0]=0x53; slot[1]=0x47; slot[2]=1;
    slot[3]=(unsigned char)seq; slot[4]=(unsigned char)(seq>>8);
    for (i=0;i<3;++i) {
        slot[5+2*i]=(unsigned char)scores[i];
        slot[6+2*i]=(unsigned char)(scores[i]>>8);
    }
    slot[11]=crc(slot,11); slot[12]=0xa5;
}

int main(void)
{
    unsigned char snapshot[13],v2_snapshot[25],i;
    memset(hs_slot_a,0xff,13); memset(hs_slot_b,0xff,13);
    memset(hs_v2_a,0xff,25); memset(hs_v2_b,0xff,25);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC)==0);
    assert(hs_v2_a[24]==0xff && hs_v2_b[24]==0xff);
    game_menu_init(GAME_LEVEL_EASY);
    assert(game_get_move_period(GAME_LEVEL_EASY)==500);
    assert(game_get_move_period(GAME_LEVEL_NORMAL)==300);
    assert(game_get_move_period(GAME_LEVEL_HARD)==150);
    game_menu_handle_key(KEY_UP);
    assert(game_menu_selected()==GAME_LEVEL_HARD);
    game_map_menu_init(MAP_CLASSIC);
    game_map_menu_handle_key(KEY_UP);
    assert(game_map_menu_selected()==MAP_LABYRINTH);
    game_map_menu_handle_key(KEY_DOWN);
    assert(game_map_menu_selected()==MAP_CLASSIC);
    assert(game_map_menu_handle_key(KEY_PAUSE));
    game_map_menu_render(GAME_LEVEL_EASY);

    make_v1(hs_slot_a,10,120,80,40);
    memcpy(snapshot,hs_slot_a,13);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC)==120);
    assert(highscore_get(GAME_LEVEL_NORMAL,MAP_CLASSIC)==80);
    assert(highscore_get(GAME_LEVEL_HARD,MAP_CLASSIC)==40);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_PILLARS)==0);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_LABYRINTH)==0);
    assert(highscore_record_result(GAME_LEVEL_EASY,MAP_PILLARS,50)==HIGHSCORE_SAVED);
    assert(memcmp(snapshot,hs_slot_a,13)==0);
    assert(highscore_record_result(GAME_LEVEL_EASY,MAP_PILLARS,30)==HIGHSCORE_UNCHANGED);
    memcpy(v2_snapshot,hs_v2_a,25);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC)==120);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_PILLARS)==50);
    assert(highscore_record_result(GAME_LEVEL_HARD,MAP_LABYRINTH,970)==HIGHSCORE_SAVED);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_HARD,MAP_LABYRINTH)==970);
    assert(highscore_record_result(GAME_LEVEL_HARD,MAP_LABYRINTH,980)==HIGHSCORE_SAVE_FAILED);
    assert(highscore_record_result((GameLevel)-1,MAP_CLASSIC,10)==HIGHSCORE_SAVE_FAILED);
    assert(highscore_record_result(GAME_LEVEL_EASY,(MapId)-1,10)==HIGHSCORE_SAVE_FAILED);
    for (i=0;i<9;++i) {
        GameLevel level=(GameLevel)(i/3u);
        MapId map=(MapId)(i%3u);
        assert(highscore_get(level,map)<=game_map_free_cells_for(map)*10u);
    }
    hs_v2_b[23]^=1u;
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_PILLARS)==50);
    assert(highscore_get(GAME_LEVEL_HARD,MAP_LABYRINTH)==0);
    assert(memcmp(v2_snapshot,hs_v2_a,25)==0);
    hs_v2_a[24]=0;
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC)==120);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_PILLARS)==0);
    for (i=0;i<9;++i) {
        GameLevel level=(GameLevel)(i/3u);
        MapId map=(MapId)(i%3u);
        unsigned int value=(unsigned int)(200u+10u*i);
        assert(highscore_record_result(level,map,value)==HIGHSCORE_SAVED);
    }
    highscore_init();
    for (i=0;i<9;++i)
        assert(highscore_get((GameLevel)(i/3u),(MapId)(i%3u))==
               (unsigned int)(200u+10u*i));
    puts("PASS: map menu, v1 migration, v2 slots, CRC, bounds");
    return 0;
}
