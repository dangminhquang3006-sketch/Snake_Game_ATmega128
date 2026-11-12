#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mock/mega128.h"
#include "../game/game_menu.h"
#include "../storage/highscore_eeprom.h"
#include "../drivers/display.h"

extern unsigned char hs_slot_a[13],hs_slot_b[13];

void display_clear(void) {}
void display_text(unsigned char x,unsigned char y,char *value)
{
    (void)x; (void)y; (void)value;
}

static unsigned char record_crc(unsigned char *data)
{
    unsigned char result=0,i,index;
    for (i=0; i<11; ++i) {
        result ^= data[i];
        for (index=0; index<8; ++index)
            result=(result&0x80u) ?
                   (unsigned char)((result<<1)^0x07u) :
                   (unsigned char)(result<<1);
    }
    return result;
}

int main(void)
{
    unsigned char snapshot[13];
    memset(hs_slot_a,0xff,13);
    memset(hs_slot_b,0xff,13);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY)==0);
    assert(hs_slot_a[12]==0xff && hs_slot_b[12]==0xff);

    game_menu_init(GAME_LEVEL_EASY);
    assert(game_get_move_period(GAME_LEVEL_EASY)==500);
    assert(game_get_move_period(GAME_LEVEL_NORMAL)==300);
    assert(game_get_move_period(GAME_LEVEL_HARD)==150);
    game_menu_handle_key(KEY_UP);
    assert(game_menu_selected()==GAME_LEVEL_HARD);
    game_menu_handle_key(KEY_DOWN);
    assert(game_menu_selected()==GAME_LEVEL_EASY);
    game_menu_handle_key(KEY_DOWN);
    assert(game_menu_selected()==GAME_LEVEL_NORMAL);
    assert(game_menu_handle_key(KEY_PAUSE)==1);
    game_menu_init((GameLevel)-1);
    assert(game_menu_selected()==GAME_LEVEL_EASY);
    game_menu_init((GameLevel)256);
    assert(game_menu_selected()==GAME_LEVEL_EASY);
    game_menu_render();

    assert(highscore_record_result(GAME_LEVEL_EASY,50)==HIGHSCORE_SAVED);
    memcpy(snapshot,hs_slot_a,13);
    assert(highscore_record_result(GAME_LEVEL_EASY,30)==HIGHSCORE_UNCHANGED);
    assert(memcmp(snapshot,hs_slot_a,13)==0);
    assert(highscore_record_result(GAME_LEVEL_NORMAL,80)==HIGHSCORE_SAVED);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY)==50);
    assert(highscore_get(GAME_LEVEL_NORMAL)==80);
    assert(highscore_get(GAME_LEVEL_HARD)==0);
    assert(highscore_record_result((GameLevel)-1,50)==HIGHSCORE_SAVE_FAILED);
    assert(highscore_record_result((GameLevel)256,50)==HIGHSCORE_SAVE_FAILED);
    assert(highscore_get((GameLevel)-1)==0);
    assert(highscore_get((GameLevel)256)==0);

    assert(highscore_record_result(GAME_LEVEL_EASY,100)==HIGHSCORE_SAVED);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY)==100);
    hs_slot_a[11]^=1u; /* Corrupt newest slot; older slot survives. */
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY)==50);
    assert(highscore_get(GAME_LEVEL_NORMAL)==80);

    assert(highscore_record_result(GAME_LEVEL_HARD,20)==HIGHSCORE_SAVED);
    hs_slot_b[12]=0; /* Simulate an interrupted write to the old slot. */
    highscore_init();
    assert(highscore_get(GAME_LEVEL_HARD)==20);
    assert(highscore_record_result(GAME_LEVEL_EASY,1110)==HIGHSCORE_SAVE_FAILED);

    hs_slot_a[3]=0xff; hs_slot_a[4]=0xff;
    hs_slot_a[11]=record_crc(hs_slot_a);
    memcpy(hs_slot_b,hs_slot_a,13);
    hs_slot_b[3]=0; hs_slot_b[4]=0;
    hs_slot_b[5]=60;
    hs_slot_b[11]=record_crc(hs_slot_b);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY)==60);
    assert(highscore_record_result(GAME_LEVEL_EASY,70)==HIGHSCORE_SAVED);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY)==70);
    puts("PASS: menu periods, EEPROM records, corruption, rollover, bounds");
    return 0;
}
