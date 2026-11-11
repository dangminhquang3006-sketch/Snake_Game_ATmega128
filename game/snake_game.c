#include "board_config.h"
#include "snake_game.h"

typedef enum { DIR_UP=0, DIR_RIGHT, DIR_DOWN, DIR_LEFT } SnakeDirection;

static SnakePoint segments[SNAKE_LENGTH];
static SnakeDirection direction;
static SnakeDirection next_direction;
static unsigned char turn_queued;
static SnakeState state;

void snake_init(void)
{
    segments[0].x=7; segments[0].y=4;
    segments[1].x=6; segments[1].y=4;
    segments[2].x=5; segments[2].y=4;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    turn_queued=0; state=SNAKE_RUNNING;
}

void snake_handle_key(KeyEvent key)
{
    SnakeDirection requested;
    if (key==KEY_PAUSE) {
        if (state==SNAKE_RUNNING) state=SNAKE_PAUSED;
        else if (state==SNAKE_PAUSED) state=SNAKE_RUNNING;
        return;
    }
    if (state!=SNAKE_RUNNING || turn_queued) return;
    if (key==KEY_UP) requested=DIR_UP;
    else if (key==KEY_DOWN) requested=DIR_DOWN;
    else if (key==KEY_LEFT) requested=DIR_LEFT;
    else if (key==KEY_RIGHT) requested=DIR_RIGHT;
    else return;
    if (requested==direction || ((requested+2)&3)==direction) return;
    next_direction=requested;
    turn_queued=1;
}

unsigned char snake_step(void)
{
    SnakePoint head;
    unsigned char i;
    if (state!=SNAKE_RUNNING) return 0;
    if (turn_queued) direction=next_direction;
    turn_queued=0;
    head=segments[0];
    if (direction==DIR_UP) {
        if (head.y==0) { state=SNAKE_STOPPED; return 1; }
        --head.y;
    } else if (direction==DIR_DOWN) {
        if (head.y==GRID_ROWS-1) { state=SNAKE_STOPPED; return 1; }
        ++head.y;
    } else if (direction==DIR_LEFT) {
        if (head.x==0) { state=SNAKE_STOPPED; return 1; }
        --head.x;
    } else {
        if (head.x==GRID_COLS-1) { state=SNAKE_STOPPED; return 1; }
        ++head.x;
    }
    for (i=SNAKE_LENGTH-1; i>0; --i) segments[i]=segments[i-1];
    segments[0]=head;
    return 1;
}

SnakeState snake_state(void) { return state; }

SnakePoint snake_segment(unsigned char index)
{
    SnakePoint empty={0,0};
    if (index>=SNAKE_LENGTH) return empty;
    return segments[index];
}
