#include <mega128.h>
#include <alcd.h>
#include "lcd1602.h"

static char desired[32];
static char shown[32];
static unsigned char next_position=0;

static flash char playing_line1[]="LEVEL: EASY";
static flash char playing_line2[]="STATUS: PLAYING";
static flash char paused_line1[]="SNAKE GAME";
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

void lcd1602_show_playing(void)
{
    spaces();
    copy_flash(0,playing_line1);
    copy_flash(16,playing_line2);
}

void lcd1602_show_paused(void)
{
    spaces();
    copy_flash(0,paused_line1);
    copy_flash(16,paused_line2);
}

void lcd1602_show_result(unsigned char won, unsigned int score)
{
    if (score>9999u) score=9999u;
    spaces();
    if (won) copy_flash(0,win_line1);
    else copy_flash(0,over_line1);
    copy_flash(16,score_prefix);
    desired[23]=(char)('0'+score/1000u);
    desired[24]=(char)('0'+(score/100u)%10u);
    desired[25]=(char)('0'+(score/10u)%10u);
    desired[26]=(char)('0'+score%10u);
}

void lcd1602_show_test_ready(void)
{
    spaces();
    copy_flash(0,paused_line1);
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
