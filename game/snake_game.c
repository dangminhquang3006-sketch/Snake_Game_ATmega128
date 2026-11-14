#include "../config/board_config.h"
#include "snake_game.h"
#include "game_map.h"
#include "game_mode.h"
#include "bonus_food.h"
#include "dynamic_obstacle.h"

typedef enum { DIR_UP=0, DIR_RIGHT, DIR_DOWN, DIR_LEFT } SnakeDirection;

static SnakePoint segments[SNAKE_MAX_LENGTH];
static unsigned char current_length;
static SnakePoint food;
static FoodType food_type;
static unsigned int score;
static unsigned char normal_foods_eaten;
static SnakeDirection direction;
static SnakeDirection next_direction;
static unsigned char turn_queued;
static SnakeState state;
static GameLevel current_level=GAME_LEVEL_EASY;
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

FoodType snake_food_type_from_roll(unsigned char roll)
{
    if (roll<60u) return FOOD_CIRCLE;
    if (roll<95u) return FOOD_SQUARE;
    return FOOD_STAR;
}

static unsigned char food_score(FoodType type)
{
    if (type==FOOD_STAR) return 50u;
    if (type==FOOD_SQUARE) return 20u;
    return 10u;
}

unsigned char snake_random_index(unsigned char count)
{
    if (count==0u) return 0;
    return (unsigned char)(random_next()%count);
}

void snake_set_level(GameLevel level)
{
    if ((unsigned int)level>=3u) level=GAME_LEVEL_EASY;
    current_level=level;
}

unsigned char snake_can_wrap(void)
{
    return (unsigned char)(current_level==GAME_LEVEL_NORMAL ||
                           current_level==GAME_LEVEL_HARD);
}

static void wrap_position(signed int *x,signed int *y)
{
    if (!snake_can_wrap()) return;
    if (*x<0) *x=GRID_COLS-1;
    else if (*x>=GRID_COLS) *x=0;
    if (*y<0) *y=GRID_ROWS-1;
    else if (*y>=GRID_ROWS) *y=0;
}

void snake_next_head(signed int *x,signed int *y)
{
    SnakeDirection heading=turn_queued ? next_direction : direction;
    *x=(signed int)segments[0].x;
    *y=(signed int)segments[0].y;
    if (heading==DIR_UP) --*y;
    else if (heading==DIR_DOWN) ++*y;
    else if (heading==DIR_LEFT) --*x;
    else ++*x;
    wrap_position(x,y);
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
            if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
                !occupied(x,y) &&
                (!bonus_food_active() || bonus_food_position().x!=x ||
                 bonus_food_position().y!=y)) ++empty_count;

    if (empty_count==0 && bonus_food_active()) {
        bonus_food_collect();
        for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
            if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
                !occupied(x,y)) ++empty_count;
    }
    if (empty_count==0) {
        state=SNAKE_WIN;
        return;
    }
    target=(unsigned char)(random_next() % empty_count);
    for (y=0; y<GRID_ROWS; ++y) {
        for (x=0; x<GRID_COLS; ++x) {
            if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
                !occupied(x,y) &&
                (!bonus_food_active() || bonus_food_position().x!=x ||
                 bonus_food_position().y!=y)) {
                if (target==0) {
                    food.x=x;
                    food.y=y;
                    /* One PRNG draw per new fruit type. */
                    food_type=snake_food_type_from_roll(
                        (unsigned char)(random_next()%100u));
                    return;
                }
                --target;
            }
        }
    }
}

static void try_spawn_bonus(void)
{
    unsigned char x,y,count=0,target;
    SnakePoint point;
    if ((game_mode_get_current()!=MODE_BONUS &&
         game_mode_get_current()!=MODE_TIME_ATTACK) ||
        bonus_food_active()) return;
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
            !occupied(x,y) &&
            !(food.x==x && food.y==y)) ++count;
    if (count==0) return;
    target=(unsigned char)(random_next()%count);
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
            !occupied(x,y) &&
            !(food.x==x && food.y==y)) {
            if (target==0) {
                point.x=x; point.y=y;
                bonus_food_spawn(point);
                return;
            }
            --target;
        }
}

void snake_init(void)
{
    dynamic_obstacle_reset();
    segments[0].x=7; segments[0].y=4;
    segments[1].x=6; segments[1].y=4;
    segments[2].x=5; segments[2].y=4;
    current_length=SNAKE_INITIAL_LENGTH;
    score=0;
    food_type=FOOD_CIRCLE;
    normal_foods_eaten=0;
    bonus_food_reset();
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
    unsigned char i,growing,eating_bonus,collision_limit;
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

    wrap_position(&new_x,&new_y);

    if (new_x<0 || new_x>=GRID_COLS || new_y<0 || new_y>=GRID_ROWS) {
        state=SNAKE_GAME_OVER;
        return 1;
    }
    if (game_map_is_blocked(new_x,new_y) ||
        dynamic_obstacle_is_blocked(new_x,new_y)) {
        state=SNAKE_GAME_OVER;
        return 1;
    }
    new_head.x=(unsigned char)new_x;
    new_head.y=(unsigned char)new_y;
    growing=(unsigned char)(new_head.x==food.x && new_head.y==food.y);
    eating_bonus=(unsigned char)(bonus_food_active() &&
        new_head.x==bonus_food_position().x &&
        new_head.y==bonus_food_position().y);

    /* The tail vacates only on a non-growing move. */
    collision_limit=growing ? current_length : (unsigned char)(current_length-1);
    for (i=1; i<collision_limit; ++i) {
        if (new_head.x==segments[i].x && new_head.y==segments[i].y) {
            state=SNAKE_GAME_OVER;
            return 1;
        }
    }

    if (growing) {
        /* Food occupies a free cell, so length is below map capacity. */
        for (i=current_length; i>0; --i) segments[i]=segments[i-1];
        ++current_length;
        if (score<=65535u-food_score(food_type))
            score=(unsigned int)(score+food_score(food_type));
        else score=65535u;
    } else {
        for (i=(unsigned char)(current_length-1); i>0; --i)
            segments[i]=segments[i-1];
    }
    segments[0]=new_head;
    if (eating_bonus) {
        if (score<=65535u-BONUS_SCORE)
            score=(unsigned int)(score+BONUS_SCORE);
        else score=65535u;
        bonus_food_collect();
    }

    if (current_length==dynamic_obstacle_playable_cells()) state=SNAKE_WIN;
    else if (growing) {
        ++normal_foods_eaten;
        place_food();
        if (state==SNAKE_RUNNING &&
            normal_foods_eaten%BONUS_TRIGGER_FOODS==0u) try_spawn_bonus();
    }
    return 1;
}

SnakeState snake_state(void) { return state; }
unsigned char snake_length(void) { return current_length; }
unsigned int snake_get_score(void) { return score; }
void snake_time_up(void)
{
    if (state==SNAKE_RUNNING && game_mode_get_current()==MODE_TIME_ATTACK)
        state=SNAKE_TIME_UP;
}
SnakePoint snake_food(void) { return food; }
FoodType snake_get_food_type(void) { return food_type; }

SnakePoint snake_segment(unsigned char index)
{
    SnakePoint empty={0,0};
    if (index>=current_length) return empty;
    return segments[index];
}
