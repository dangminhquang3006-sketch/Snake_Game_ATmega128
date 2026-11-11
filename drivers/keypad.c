#include <mega128.h>
#include "../config/board_config.h"
#include "keypad.h"

#define KEY_RELEASED 0
#define KEY_MULTIPLE 10
#define KEY_DEBOUNCE_MS 20UL

static unsigned char candidate=KEY_RELEASED;
static unsigned char stable=KEY_RELEASED;
static unsigned long changed_at=0;
static unsigned long last_scan=0;
static unsigned char scanned=0;
static unsigned char locked=0;
static KeyEvent pending=KEY_NONE;
static unsigned char pending_position=KEY_RELEASED;

static void columns_high(void)
{
    KEY_COL1_PORT=1;
    KEY_COL2_PORT=1;
    KEY_COL3_PORT=1;
}

static void column_on(unsigned char col)
{
    columns_high();
    if (col==0) KEY_COL1_PORT=0;
    else if (col==1) KEY_COL2_PORT=0;
    else KEY_COL3_PORT=0;
}

static unsigned char scan_raw(void)
{
    unsigned char row,col,found=KEY_RELEASED;
    unsigned char pressed[3];
    for (col=0; col<3; ++col) {
        column_on(col);
        /* GPIO reads allow the selected column to settle without delay. */
        pressed[0]=(unsigned char)!KEY_ROW1_PIN;
        pressed[1]=(unsigned char)!KEY_ROW2_PIN;
        pressed[2]=(unsigned char)!KEY_ROW3_PIN;
        for (row=0; row<3; ++row) {
            if (pressed[row]) {
                if (found!=KEY_RELEASED) {
                    columns_high();
                    return KEY_MULTIPLE;
                }
                found=(unsigned char)(row*3+col+1);
            }
        }
    }
    columns_high();
    return found;
}

static KeyEvent event_from_position(unsigned char key)
{
    /* Row-major key positions per docs/so_do_chan_ban_phim.jpeg. */
    switch (key) {
    case 2: return KEY_UP;
    case 8: return KEY_DOWN;
    case 4: return KEY_LEFT;
    case 6: return KEY_RIGHT;
    case 5: return KEY_PAUSE;
    default: return KEY_NONE;
    }
}

void keypad_init(void)
{
    KEY_ROW1_DDR=0; KEY_ROW1_PORT=1;
    KEY_ROW2_DDR=0; KEY_ROW2_PORT=1;
    KEY_ROW3_DDR=0; KEY_ROW3_PORT=1;
    columns_high();
    KEY_COL1_DDR=1;
    KEY_COL2_DDR=1;
    KEY_COL3_DDR=1;
    candidate=KEY_RELEASED; stable=KEY_RELEASED;
    changed_at=0; last_scan=0; scanned=0; locked=0;
    pending=KEY_NONE; pending_position=KEY_RELEASED;
}

void keypad_update(unsigned long now_ms)
{
    unsigned char raw;
    if (scanned && now_ms==last_scan) return;
    scanned=1;
    last_scan=now_ms;
    raw=scan_raw();
    if (raw!=candidate) {
        candidate=raw;
        changed_at=now_ms;
        return;
    }
    if (raw==stable || (unsigned long)(now_ms-changed_at)<KEY_DEBOUNCE_MS) return;
    stable=raw;
    if (stable==KEY_RELEASED) locked=0;
    else if (!locked) {
        locked=1;
        if (stable!=KEY_MULTIPLE) {
            pending=event_from_position(stable);
            pending_position=stable;
        }
    }
}

KeyEvent keypad_get_event(void)
{
    KeyEvent result=pending;
    pending=KEY_NONE;
    pending_position=KEY_RELEASED;
    return result;
}

unsigned char keypad_get_press(unsigned char *row, unsigned char *col)
{
    unsigned char position=pending_position;
    if (position==KEY_RELEASED) return KEY_RELEASED;
    *row=(unsigned char)((position-1)/3+1);
    *col=(unsigned char)((position-1)%3+1);
    pending_position=KEY_RELEASED;
    pending=KEY_NONE;
    return position;
}
