#ifndef KEYPAD_H
#define KEYPAD_H

typedef enum {
    KEY_NONE=0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_PAUSE
} KeyEvent;

void keypad_init(void);
void keypad_update(unsigned long now_ms);
KeyEvent keypad_get_event(void);

#endif
