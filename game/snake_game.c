#include "../config/board_config.h"
#include "snake_game.h"

typedef enum { DIR_UP=0, DIR_RIGHT, DIR_DOWN, DIR_LEFT } SnakeDirection;

static SnakePoint segments[SNAKE_MAX_LENGTH];
static unsigned char current_length;
static SnakePoint food;
static unsigned int score;
static SnakeDirection direction;
static SnakeDirection next_direction;
static unsigned char turn_queued;
static SnakeState state;
/* A nonzero 16-bit Galois LFSR state. It is pseudo-random, not true entropy. */
static unsigned int rng_state=0xACE1u;

static unsigned int random_next(void)
{
    unsigned char low_bit=(unsigned char)(rng_state & 1u);
    rng_state >>= 1;
    if (low_bit) rng_state ^= 0xB400u;
    if (rng_state==0) rng_state=0xACE1u;
    return rng_state;
}

void snake_mix_entropy(unsigned long now_ms)
{
    rng_state ^= (unsigned int)now_ms;
    rng_state ^= (unsigned int)(now_ms >> 16);
    if (rng_state==0) rng_state=0xACE1u;
}

static unsigned char occupied(unsigned char x, unsigned char y)
{
    unsigned char i;
    for (i=0; i<current_length; ++i)
        if (segments[i].x==x && segments[i].y==y) return 1;
    return 0;
}

static void place_food(void)
{
    unsigned char x,y,empty_count=0,target;
    for (y=0; y<GRID_ROWS; ++y)
        for (x=0; x<GRID_COLS; ++x)
            if (!occupied(x,y)) ++empty_count;

    if (empty_count==0) {
        state=SNAKE_WIN;
        return;
    }
    target=(unsigned char)(random_next() % empty_count);
    for (y=0; y<GRID_ROWS; ++y) {
        for (x=0; x<GRID_COLS; ++x) {
            if (!occupied(x,y)) {
                if (target==0) {
                    food.x=x;
                    food.y=y;
                    return;
                }
                --target;
            }
        }
    }
}

void snake_init(void)
{
    segments[0].x=7; segments[0].y=4;
    segments[1].x=6; segments[1].y=4;
    segments[2].x=5; segments[2].y=4;
    current_length=SNAKE_INITIAL_LENGTH;
    score=0;
    direction=DIR_RIGHT;
    next_direction=DIR_RIGHT;
    turn_queued=0;
    state=SNAKE_RUNNING;
    place_food();
}

void snake_handle_key(KeyEvent key)
{
    SnakeDirection requested;
    if (key==KEY_RESTART) {
        snake_init();
        return;
    }
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
    signed int new_x,new_y;
    unsigned char i,growing,collision_limit;
    SnakePoint new_head;
    if (state!=SNAKE_RUNNING) return 0;

    if (turn_queued) direction=next_direction;
    turn_queued=0;
    new_x=(signed int)segments[0].x;
    new_y=(signed int)segments[0].y;
    if (direction==DIR_UP) --new_y;
    else if (direction==DIR_DOWN) ++new_y;
    else if (direction==DIR_LEFT) --new_x;
    else ++new_x;

    if (new_x<0 || new_x>=GRID_COLS || new_y<0 || new_y>=GRID_ROWS) {
        state=SNAKE_GAME_OVER;
        return 1;
    }
    new_head.x=(unsigned char)new_x;
    new_head.y=(unsigned char)new_y;
    growing=(unsigned char)(new_head.x==food.x && new_head.y==food.y);

    /* The tail vacates only on a non-growing move. */
    collision_limit=growing ? current_length : (unsigned char)(current_length-1);
    for (i=1; i<collision_limit; ++i) {
        if (new_head.x==segments[i].x && new_head.y==segments[i].y) {
            state=SNAKE_GAME_OVER;
            return 1;
        }
    }

    if (growing) {
        /* Food occupies a free cell, so length is below the 112-cell limit. */
        for (i=current_length; i>0; --i) segments[i]=segments[i-1];
        ++current_length;
        score=(unsigned int)(score+10u);
    } else {
        for (i=(unsigned char)(current_length-1); i>0; --i)
            segments[i]=segments[i-1];
    }
    segments[0]=new_head;

    if (current_length==SNAKE_MAX_LENGTH) state=SNAKE_WIN;
    else if (growing) place_food();
    return 1;
}

SnakeState snake_state(void) { return state; }
unsigned char snake_length(void) { return current_length; }
unsigned int snake_get_score(void) { return score; }
SnakePoint snake_food(void) { return food; }

SnakePoint snake_segment(unsigned char index)
{
    SnakePoint empty={0,0};
    if (index>=current_length) return empty;
    return segments[index];
}
