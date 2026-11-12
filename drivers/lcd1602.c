#include <mega128.h>
#include <alcd.h>
#include "lcd1602.h"

static char desired[32];
static char shown[32];
static unsigned char next_position=0;

static flash char level_prefix[]="LEVEL: ";
static flash char easy_name[]="EASY";
static flash char normal_name[]="NORMAL";
static flash char hard_name[]="HARD";
static flash char select_line[]="SELECT LEVEL";
static flash char best_prefix[]="BEST:";
static flash char playing_line2[]="STATUS: PLAYING";
static flash char title_line[]="SNAKE GAME";
static flash char paused_line2[]="GAME PAUSED";
static flash char over_line1[]="GAME OVER";
static flash char win_line1[]="YOU WIN!";
static flash char score_prefix[]="SCORE: ";
static flash char ready_line2[]="STATUS: READY";

static void spaces(void)
{
    unsigned char i;
    for (i=0; i<32; ++i) desired[i]=' ';
}

static void copy_flash(unsigned char offset, flash char *text)
{
    unsigned char i=0;
    while (*text && i<16) {
        desired[offset+i]=*text;
        ++i;
        ++text;
    }
}

static unsigned char copy_level(unsigned char offset, GameLevel level)
{
    if (level==GAME_LEVEL_NORMAL) {
        copy_flash(offset,normal_name);
        return 6;
    }
    if (level==GAME_LEVEL_HARD) {
        copy_flash(offset,hard_name);
        return 4;
    }
    copy_flash(offset,easy_name);
    return 4;
}

static void put_four_digits(unsigned char offset, unsigned int value)
{
    if (value>9999u) value=9999u;
    desired[offset]=(char)('0'+value/1000u);
    desired[offset+1]=(char)('0'+(value/100u)%10u);
    desired[offset+2]=(char)('0'+(value/10u)%10u);
    desired[offset+3]=(char)('0'+value%10u);
}

static void show_level_line(GameLevel level)
{
    spaces();
    copy_flash(0,level_prefix);
    copy_level(7,level);
}

void lcd1602_init(void)
{
    unsigned char i;
    lcd_init(16);
    for (i=0; i<32; ++i) {
        desired[i]=' ';
        shown[i]=0;
    }
    next_position=0;
}

void lcd1602_show_menu(GameLevel level, unsigned int best)
{
    unsigned char length,offset;
    spaces();
    copy_flash(0,select_line);
    length=copy_level(16,level);
    offset=(unsigned char)(16+length+1);
    copy_flash(offset,best_prefix);
    put_four_digits((unsigned char)(offset+5),best);
}

void lcd1602_show_playing(GameLevel level)
{
    show_level_line(level);
    copy_flash(16,playing_line2);
}

void lcd1602_show_paused(GameLevel level)
{
    show_level_line(level);
    copy_flash(16,paused_line2);
}

void lcd1602_show_result(unsigned char won, unsigned int score)
{
    if (score>9999u) score=9999u;
    spaces();
    if (won) copy_flash(0,win_line1);
    else copy_flash(0,over_line1);
    copy_flash(16,score_prefix);
    put_four_digits(23,score);
}

void lcd1602_show_test_ready(void)
{
    spaces();
    copy_flash(0,title_line);
    copy_flash(16,ready_line2);
}

unsigned char lcd1602_is_dirty(void)
{
    unsigned char i;
    for (i=0; i<32; ++i)
        if (shown[i]!=desired[i]) return 1;
    return 0;
}

void lcd1602_service(void)
{
    unsigned char n,index;
    for (n=0; n<32; ++n) {
        index=(unsigned char)((next_position+n)&31u);
        if (shown[index]!=desired[index]) {
            lcd_gotoxy((unsigned char)(index&15u),(unsigned char)(index>>4));
            lcd_putchar(desired[index]);
            shown[index]=desired[index];
            next_position=(unsigned char)((index+1u)&31u);
            return;
        }
    }
}
