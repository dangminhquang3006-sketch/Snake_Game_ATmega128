#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mock/mega128.h"
#include "../game/game_menu.h"
#include "../storage/highscore_eeprom.h"
#include "../drivers/display.h"

extern unsigned char hs_slot_a[13],hs_slot_b[13];
extern unsigned char hs_v2_a[25],hs_v2_b[25];
extern unsigned char hs_v3_a[61],hs_v3_b[61];
extern unsigned char hs_v4_a[79],hs_v4_b[79];
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

static void make_record(unsigned char *slot,unsigned char version,
                        unsigned char count,unsigned int first)
{
    unsigned char i,size=(unsigned char)(7u+2u*count);
    unsigned int value;
    memset(slot,0,size);
    slot[0]=0x53; slot[1]=0x47; slot[2]=version;
    slot[3]=1; slot[4]=0;
    for (i=0;i<count;++i) {
        value=(unsigned int)(first+10u*i);
        slot[5u+2u*i]=(unsigned char)value;
        slot[6u+2u*i]=(unsigned char)(value>>8);
    }
    slot[size-2u]=crc(slot,(unsigned char)(size-2u));
    slot[size-1u]=0xa5;
}

static void clear_eeprom(void)
{
    memset(hs_slot_a,0xff,13); memset(hs_slot_b,0xff,13);
    memset(hs_v2_a,0xff,25); memset(hs_v2_b,0xff,25);
    memset(hs_v3_a,0xff,61); memset(hs_v3_b,0xff,61);
    memset(hs_v4_a,0xff,79); memset(hs_v4_b,0xff,79);
}

int main(void)
{
    unsigned char i,v3_copy[61];
    clear_eeprom();
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==0);
    game_mode_menu_init(MODE_STANDARD);
    game_mode_menu_handle_key(KEY_UP);
    assert(game_mode_menu_selected()==MODE_SURVIVAL);
    game_mode_menu_handle_key(KEY_DOWN);
    assert(game_mode_menu_selected()==MODE_STANDARD);
    game_mode_menu_render();

    make_record(hs_slot_a,1,3,100);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==100);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL)==0);
    assert(hs_v4_a[78]==0xff);
    make_record(hs_v2_a,2,9,200);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_HARD,MAP_LABYRINTH,MODE_STANDARD)==280);
    make_record(hs_v3_a,3,27,300);
    memcpy(v3_copy,hs_v3_a,61);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==300);
    assert(highscore_get(GAME_LEVEL_HARD,MAP_LABYRINTH,MODE_TIME_ATTACK)==560);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL)==0);
    assert(highscore_record_result(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL,50)==HIGHSCORE_SAVED);
    assert(memcmp(v3_copy,hs_v3_a,61)==0);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL)==50);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==300);
    assert(highscore_record_result(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL,40)==HIGHSCORE_UNCHANGED);
    assert(highscore_record_result(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL,60)==HIGHSCORE_SAVED);
    hs_v4_b[77]^=1u;
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL)==50);
    hs_v4_a[78]=0;
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL)==0);
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==300);
    for (i=0;i<36u;++i) {
        GameMode mode=(GameMode)(i/9u);
        GameLevel level=(GameLevel)((i%9u)/3u);
        MapId map=(MapId)(i%3u);
        unsigned int value=(unsigned int)(600u+10u*i);
        assert(highscore_record_result(level,map,mode,value)==HIGHSCORE_SAVED);
    }
    highscore_init();
    for (i=0;i<36u;++i)
        assert(highscore_get((GameLevel)((i%9u)/3u),(MapId)(i%3u),
                             (GameMode)(i/9u))==(unsigned int)(600u+10u*i));
    assert(highscore_record_result(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL,1110)==HIGHSCORE_SAVED);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL)==1110u);
    assert(highscore_record_result(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_SURVIVAL,5460)==HIGHSCORE_SAVE_FAILED);
    clear_eeprom();
    make_record(hs_v4_a,4,36,100);
    make_record(hs_v4_b,4,36,200);
    hs_v4_a[3]=0xff; hs_v4_a[4]=0xff;
    hs_v4_a[77]=crc(hs_v4_a,77);
    hs_v4_b[3]=0; hs_v4_b[4]=0;
    hs_v4_b[77]=crc(hs_v4_b,77);
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==200u);
    hs_v4_b[78]=0;
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==100u);
    hs_v4_b[78]=0xa5;
    hs_v4_a[77]^=1u;
    highscore_init();
    assert(highscore_get(GAME_LEVEL_EASY,MAP_CLASSIC,MODE_STANDARD)==200u);
    puts("PASS: mode menu, EEPROM v1/v2/v3 to v4, CRC, 36 scores");
    return 0;
}
