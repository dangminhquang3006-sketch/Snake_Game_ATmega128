/* Gameplay implementation: map, snake, obstacles, menus and rendering.
   Compile this file once; public interfaces remain in the matching headers. */
#include "../config/board_config.h"
#include "../drivers/display.h"
#include "snake_game.h"
#include "game_map.h"
#include "game_mode.h"
#include "dynamic_obstacle.h"
#include "game_menu.h"
#include "game_render.h"

/* ---------- game_map ---------- */

#define SNAKE_MAP_MIN_CELLS 4u

/* CVAVR ATmega128 unsigned int is 16 bits. Bit x marks a wall at column x. */
static flash unsigned int walls[MAP_COUNT][GRID_ROWS]={
    {0,0,0,0,0,0,0,0},
    {0,0,0x0330u,0,0,0x0330u,0,0},
    {0,0x0408u,0x04c8u,0,0,0x0cccu,0,0}
};
static MapId current_map=MAP_CLASSIC;

static unsigned char blocked_at(MapId map, unsigned char x, unsigned char y)
{
    return (unsigned char)((walls[(unsigned char)map][y] >> x)&1u);
}

unsigned char game_map_validate(MapId map)
{
    unsigned int visited[GRID_ROWS]={0,0,0,0,0,0,0,0};
    unsigned char x,y,count=0,reached=0,changed;
    unsigned int valid_mask=(unsigned int)((1u<<GRID_COLS)-1u);
    if ((unsigned int)map>=MAP_COUNT) return 0;
    for (y=0; y<GRID_ROWS; ++y) {
        if (walls[(unsigned char)map][y] & (unsigned int)~valid_mask) return 0;
        for (x=0; x<GRID_COLS; ++x)
            if (!blocked_at(map,x,y)) ++count;
    }
    if (count<SNAKE_MAP_MIN_CELLS || blocked_at(map,5,4) ||
        blocked_at(map,6,4) || blocked_at(map,7,4)) return 0;
    /* Fixed layouts also have an exact expected capacity. */
    if ((map==MAP_CLASSIC && count!=112u) ||
        (map==MAP_PILLARS && count!=104u) ||
        (map==MAP_LABYRINTH && count!=100u)) return 0;
    visited[4]=(unsigned int)(1u<<7);
    do {
        changed=0;
        for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x) {
            unsigned int mask=(unsigned int)(1u<<x);
            if (blocked_at(map,x,y) || (visited[y]&mask)) continue;
            if ((x>0 && (visited[y]&(mask>>1))) ||
                (x+1<GRID_COLS && (visited[y]&(mask<<1))) ||
                (y>0 && (visited[y-1]&mask)) ||
                (y+1<GRID_ROWS && (visited[y+1]&mask))) {
                visited[y]|=mask;
                changed=1;
            }
        }
    } while (changed);
    for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x)
        if (visited[y] & (1u<<x)) ++reached;
    return (unsigned char)(reached==count);
}

unsigned char game_map_select(MapId map)
{
    if (!game_map_validate(map)) return 0;
    current_map=map;
    return 1;
}

MapId game_map_get_current(void) { return current_map; }

unsigned char game_map_is_blocked(signed int x, signed int y)
{
    if (x<0 || x>=GRID_COLS || y<0 || y>=GRID_ROWS) return 1;
    return blocked_at(current_map,(unsigned char)x,(unsigned char)y);
}

unsigned char game_map_get_free_cells(void)
{
    return game_map_free_cells_for(current_map);
}

unsigned char game_map_free_cells_for(MapId map)
{
    unsigned char x,y,count=0;
    if ((unsigned int)map>=MAP_COUNT) return 0;
    for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x)
        if (!blocked_at(map,x,y)) ++count;
    return count;
}

/* ---------- snake_game ---------- */

typedef enum { DIR_UP=0, DIR_RIGHT, DIR_DOWN, DIR_LEFT } SnakeDirection;

static SnakePoint segments[SNAKE_MAX_LENGTH];
static unsigned char current_length;
static SnakePoint food;
static FoodType food_type;
static unsigned int score;
static SnakeDirection direction;
static SnakeDirection next_direction;
static unsigned char turn_queued;
static SnakeState state;
static GameLevel current_level=GAME_LEVEL_NORMAL;
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
    if (level!=GAME_LEVEL_NORMAL && level!=GAME_LEVEL_HARD) level=GAME_LEVEL_NORMAL;
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

static void reverse_segments(void)
{
    unsigned char left=0u,right=(unsigned char)(current_length-1u);
    SnakePoint saved;
    while (left<right) {
        saved=segments[left];
        segments[left]=segments[right];
        segments[right]=saved;
        ++left;
        --right;
    }
}

/* After reversal, the head must move away from its adjacent neck. */
static unsigned char direction_from_neck(SnakeDirection *heading)
{
    SnakePoint head,neck;
    if (current_length<2u) return 0;
    head=segments[0];
    neck=segments[1];
    if (head.y==neck.y) {
        if ((head.x==(unsigned char)(neck.x+1u)) ||
            (head.x==0u && neck.x==GRID_COLS-1u && snake_can_wrap())) {
            *heading=DIR_RIGHT;
            return 1;
        }
        if ((neck.x==(unsigned char)(head.x+1u)) ||
            (head.x==GRID_COLS-1u && neck.x==0u && snake_can_wrap())) {
            *heading=DIR_LEFT;
            return 1;
        }
    } else if (head.x==neck.x) {
        if ((head.y==(unsigned char)(neck.y+1u)) ||
            (head.y==0u && neck.y==GRID_ROWS-1u && snake_can_wrap())) {
            *heading=DIR_DOWN;
            return 1;
        }
        if ((neck.y==(unsigned char)(head.y+1u)) ||
            (head.y==GRID_ROWS-1u && neck.y==0u && snake_can_wrap())) {
            *heading=DIR_UP;
            return 1;
        }
    }
    return 0;
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
                !occupied(x,y)) ++empty_count;

    if (empty_count==0) {
        state=SNAKE_WIN;
        return;
    }
    target=(unsigned char)(random_next() % empty_count);
    for (y=0; y<GRID_ROWS; ++y) {
        for (x=0; x<GRID_COLS; ++x) {
            if (!game_map_is_blocked(x,y) && !dynamic_obstacle_is_blocked(x,y) &&
                !occupied(x,y)) {
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

void snake_init(void)
{
    dynamic_obstacle_reset();
    segments[0].x=7; segments[0].y=4;
    segments[1].x=6; segments[1].y=4;
    segments[2].x=5; segments[2].y=4;
    current_length=SNAKE_INITIAL_LENGTH;
    score=0;
    food_type=FOOD_CIRCLE;
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
    if (current_length==dynamic_obstacle_playable_cells()) state=SNAKE_WIN;
    else if (growing) {
        if (current_level==GAME_LEVEL_HARD) {
            reverse_segments();
            if (!direction_from_neck(&direction)) {
                state=SNAKE_GAME_OVER;
                return 1;
            }
            next_direction=direction;
            turn_queued=0u;
        }
        place_food();
    }
    return 1;
}

SnakeState snake_state(void) { return state; }
unsigned char snake_length(void) { return current_length; }
unsigned int snake_get_score(void) { return score; }
SnakePoint snake_food(void) { return food; }
FoodType snake_get_food_type(void) { return food_type; }

SnakePoint snake_segment(unsigned char index)
{
    SnakePoint empty={0,0};
    if (index>=current_length) return empty;
    return segments[index];
}

/* ---------- dynamic_obstacle ---------- */

/* 8 row masks use 16 bytes; queue and visited are static to bound stack. */
static unsigned int blocked_rows[GRID_ROWS];
static unsigned char queue[GRID_COLS*GRID_ROWS];
static unsigned int visited[GRID_ROWS];
static unsigned char obstacle_count;
/* One left endpoint per horizontal bar; collision uses all occupied cells. */
static SnakePoint obstacle_starts[DYNAMIC_OBSTACLE_MAX_COUNT];
static unsigned long elapsed_ms;

void dynamic_obstacle_reset(void)
{
    unsigned char y;
    for (y=0;y<GRID_ROWS;++y) blocked_rows[y]=0;
    obstacle_count=0;
    elapsed_ms=0;
}

unsigned char dynamic_obstacle_is_blocked(signed int x,signed int y)
{
    if (x<0 || x>=GRID_COLS || y<0 || y>=GRID_ROWS) return 1;
    return (unsigned char)((blocked_rows[(unsigned char)y]>>x)&1u);
}

unsigned char dynamic_obstacle_get_count(void) { return obstacle_count; }

unsigned char dynamic_obstacle_playable_cells(void)
{
    unsigned char x,y,blocked=0;
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (dynamic_obstacle_is_blocked(x,y)) ++blocked;
    return (unsigned char)(game_map_get_free_cells()-blocked);
}

SnakePoint dynamic_obstacle_point(unsigned char index)
{
    SnakePoint point={0,0};
    if (index>=obstacle_count) return point;
    return obstacle_starts[index];
}

static unsigned char is_wall(unsigned char x,unsigned char y,
                             unsigned char candidate_x,unsigned char candidate_y)
{
    return (unsigned char)(game_map_is_blocked(x,y) ||
        dynamic_obstacle_is_blocked(x,y) ||
        (y==candidate_y && x>=candidate_x &&
         x<candidate_x+DYNAMIC_OBSTACLE_LENGTH));
}

static void visit(unsigned char x,unsigned char y,unsigned char *end)
{
    unsigned int mask=(unsigned int)(1u<<x);
    if (visited[y]&mask) return;
    visited[y]|=mask;
    queue[*end]=(unsigned char)(y*GRID_COLS+x);
    ++*end;
}

/* Static walls only: snake body moves and must not be treated as permanent. */
static unsigned char remains_connected(unsigned char candidate_x,
                                        unsigned char candidate_y)
{
    unsigned char start=0,end=0,cell,x,y,seen=0,free_count=0,wrap;
    SnakePoint head;
    SnakePoint food;

    head = snake_segment(0);
    food = snake_food();
    wrap=snake_can_wrap();
    for (y=0;y<GRID_ROWS;++y) {
        visited[y]=0;
        for (x=0;x<GRID_COLS;++x)
            if (!is_wall(x,y,candidate_x,candidate_y)) ++free_count;
    }
    visit(head.x,head.y,&end);
    while (start<end) {
        cell=queue[start++];
        x=(unsigned char)(cell%GRID_COLS);
        y=(unsigned char)(cell/GRID_COLS);
        ++seen;
        if (x>0 && !is_wall((unsigned char)(x-1u),y,candidate_x,candidate_y))
            visit((unsigned char)(x-1u),y,&end);
        if (x+1u<GRID_COLS && !is_wall((unsigned char)(x+1u),y,candidate_x,candidate_y))
            visit((unsigned char)(x+1u),y,&end);
        if (y>0 && !is_wall(x,(unsigned char)(y-1u),candidate_x,candidate_y))
            visit(x,(unsigned char)(y-1u),&end);
        if (y+1u<GRID_ROWS && !is_wall(x,(unsigned char)(y+1u),candidate_x,candidate_y))
            visit(x,(unsigned char)(y+1u),&end);
        if (wrap) {
            if (x==0u && !is_wall((unsigned char)(GRID_COLS-1u),y,candidate_x,candidate_y))
                visit((unsigned char)(GRID_COLS-1u),y,&end);
            if (x==GRID_COLS-1u && !is_wall(0u,y,candidate_x,candidate_y))
                visit(0u,y,&end);
            if (y==0u && !is_wall(x,(unsigned char)(GRID_ROWS-1u),candidate_x,candidate_y))
                visit(x,(unsigned char)(GRID_ROWS-1u),&end);
            if (y==GRID_ROWS-1u && !is_wall(x,0u,candidate_x,candidate_y))
                visit(x,0u,&end);
        }
    }
    return (unsigned char)(seen==free_count &&
        (visited[food.y]&(1u<<food.x))!=0u);
}

static unsigned char cell_safe(unsigned char x,unsigned char y)
{
    unsigned char i,distance,dx,dy;
    SnakePoint head,point,food;
    signed int next_x,next_y;
    if (x>=GRID_COLS || y>=GRID_ROWS ||
        game_map_is_blocked(x,y) || dynamic_obstacle_is_blocked(x,y)) return 0;
    head=snake_segment(0);
    dx=(unsigned char)((x>head.x)?(x-head.x):(head.x-x));
    dy=(unsigned char)((y>head.y)?(y-head.y):(head.y-y));
    if (snake_can_wrap()) {
        if (dx>GRID_COLS-dx) dx=(unsigned char)(GRID_COLS-dx);
        if (dy>GRID_ROWS-dy) dy=(unsigned char)(GRID_ROWS-dy);
    }
    distance=(unsigned char)(dx+dy);
    if (distance<=DYNAMIC_OBSTACLE_SAFE_DIST) return 0;
    snake_next_head(&next_x,&next_y);
    if (next_x==(signed int)x && next_y==(signed int)y) return 0;
    food=snake_food();
    if (x==food.x && y==food.y) return 0;
    for (i=0;i<snake_length();++i) {
        point=snake_segment(i);
        if (point.x==x && point.y==y) return 0;
    }
    return 1;
}

/* Validate the entire bar together, including connectivity after all 3 cells
   are blocked. Bars stay inside the display even on wrapping levels. */
unsigned char dynamic_obstacle_candidate_valid(unsigned char x,unsigned char y)
{
    unsigned char j;
    if (x>GRID_COLS-DYNAMIC_OBSTACLE_LENGTH || y>=GRID_ROWS) return 0;
    for (j=0;j<DYNAMIC_OBSTACLE_LENGTH;++j)
        if (!cell_safe((unsigned char)(x+j),y)) return 0;
    return remains_connected(x,y);
}

unsigned char dynamic_obstacle_update(unsigned long elapsed_running_ms)
{
    unsigned char offset,i,x,y,j;
    if (snake_state()!=SNAKE_RUNNING ||
        obstacle_count>=DYNAMIC_OBSTACLE_MAX_COUNT) return 0;
    if (elapsed_running_ms>=DYNAMIC_OBSTACLE_INTERVAL_MS)
        elapsed_ms=DYNAMIC_OBSTACLE_INTERVAL_MS;
    else {
        elapsed_ms+=elapsed_running_ms;
        if (elapsed_ms<DYNAMIC_OBSTACLE_INTERVAL_MS) return 0;
    }
    /* Drop excess elapsed time: never catch up with multiple spawns. */
    elapsed_ms=0;
    offset=snake_random_index((unsigned char)(GRID_COLS*GRID_ROWS));
    for (i=0;i<(unsigned char)(GRID_COLS*GRID_ROWS);++i) {
        unsigned char cell=(unsigned char)((offset+i)%(GRID_COLS*GRID_ROWS));
        x=(unsigned char)(cell%GRID_COLS);
        y=(unsigned char)(cell/GRID_COLS);
        if (dynamic_obstacle_candidate_valid(x,y)) {
            for (j=0;j<DYNAMIC_OBSTACLE_LENGTH;++j)
                blocked_rows[y]|=(unsigned int)(1u<<(x+j));
            obstacle_starts[obstacle_count].x=x;
            obstacle_starts[obstacle_count].y=y;
            ++obstacle_count;
            return 1;
        }
    }
    return 0;
}

/* ---------- game_menu ---------- */

static GameLevel selected_level=GAME_LEVEL_NORMAL;
static MapId selected_map=MAP_CLASSIC;
static flash char title_text[]="SNAKE GAME";
static flash char normal_selected[]="> NORMAL";
static flash char normal_plain[]   ="  NORMAL";
static flash char hard_selected[]  ="> HARD";
static flash char hard_plain[]     ="  HARD";
static flash char help_text[]="2/8:MOVE 5:OK";
static flash char map_title[]="SELECT MAP";
static flash char level_normal[]="LEVEL: NORMAL";
static flash char level_hard[]="LEVEL: HARD";
static flash char classic_selected[]="> CLASSIC";
static flash char classic_plain[]="  CLASSIC";
static flash char pillars_selected[]="> PILLARS";
static flash char pillars_plain[]="  PILLARS";
static flash char labyrinth_selected[]="> LABYRINTH";
static flash char labyrinth_plain[]="  LABYRINTH";

void game_menu_init(GameLevel selected)
{
    if (selected!=GAME_LEVEL_NORMAL && selected!=GAME_LEVEL_HARD) selected=GAME_LEVEL_NORMAL;
    selected_level=selected;
}

unsigned char game_menu_handle_key(KeyEvent key)
{
    if (key==KEY_UP) {
        if (selected_level==GAME_LEVEL_NORMAL) selected_level=GAME_LEVEL_HARD;
        else selected_level=(GameLevel)(selected_level-1);
    } else if (key==KEY_DOWN) {
        if (selected_level==GAME_LEVEL_HARD) selected_level=GAME_LEVEL_NORMAL;
        else selected_level=(GameLevel)(selected_level+1);
    } else if (key==KEY_CONFIRM) return 1;
    return 0;
}

GameLevel game_menu_selected(void)
{
    return selected_level;
}

unsigned int game_get_move_period(GameLevel level)
{
    if (level==GAME_LEVEL_HARD) return GAME_HARD_MOVE_MS;
    return GAME_NORMAL_MOVE_MS;
}

void game_menu_render(void)
{
    display_clear();
    display_text(0,0,title_text);
    if (selected_level==GAME_LEVEL_NORMAL) display_text(0,14,normal_selected);
    else display_text(0,14,normal_plain);
    if (selected_level==GAME_LEVEL_HARD) display_text(0,26,hard_selected);
    else display_text(0,26,hard_plain);
    display_text(0,40,help_text);
}

void game_map_menu_init(MapId selected)
{
    if ((unsigned int)selected>=MAP_COUNT) selected=MAP_CLASSIC;
    selected_map=selected;
}

unsigned char game_map_menu_handle_key(KeyEvent key)
{
    if (key==KEY_UP) {
        if (selected_map==MAP_CLASSIC) selected_map=MAP_LABYRINTH;
        else selected_map=(MapId)(selected_map-1);
    } else if (key==KEY_DOWN) {
        if (selected_map==MAP_LABYRINTH) selected_map=MAP_CLASSIC;
        else selected_map=(MapId)(selected_map+1);
    } else if (key==KEY_CONFIRM) return 1;
    return 0;
}

MapId game_map_menu_selected(void) { return selected_map; }

void game_map_menu_render(GameLevel level)
{
    display_clear();
    display_text(0,0,map_title);
    if (level==GAME_LEVEL_HARD) display_text(0,8,level_hard);
    else display_text(0,8,level_normal);
    if (selected_map==MAP_CLASSIC) display_text(0,17,classic_selected);
    else display_text(0,17,classic_plain);
    if (selected_map==MAP_PILLARS) display_text(0,26,pillars_selected);
    else display_text(0,26,pillars_plain);
    if (selected_map==MAP_LABYRINTH) display_text(0,35,labyrinth_selected);
    else display_text(0,35,labyrinth_plain);
}

/* ---------- game_render ---------- */

static flash char over_text[]="GAME OVER";
static flash char win_text[]="YOU WIN";
static flash char restart_text[]="9:RESTART";
static flash char menu_text[]="1:MENU";
static char score_text[]="SCORE: 0000";

static void render_fruit(SnakePoint point,FoodType type)
{
    unsigned char x=(unsigned char)(point.x*CELL_SIZE);
    unsigned char y=(unsigned char)(point.y*CELL_SIZE);
    if (type==FOOD_SQUARE) {
        display_fill_rect((unsigned char)(x+1u),(unsigned char)(y+1u),3,3);
    } else if (type==FOOD_STAR) {
        display_fill_rect((unsigned char)(x+2u),y,1,1);
        display_fill_rect(x,(unsigned char)(y+2u),5,1);
        display_fill_rect((unsigned char)(x+1u),(unsigned char)(y+3u),1,1);
        display_fill_rect((unsigned char)(x+3u),(unsigned char)(y+3u),1,1);
        display_fill_rect(x,(unsigned char)(y+4u),1,1);
        display_fill_rect((unsigned char)(x+4u),(unsigned char)(y+4u),1,1);
    } else {
        display_fill_rect((unsigned char)(x+1u),y,3,1);
        display_fill_rect(x,(unsigned char)(y+1u),1,3);
        display_fill_rect((unsigned char)(x+4u),(unsigned char)(y+1u),1,3);
        display_fill_rect((unsigned char)(x+1u),(unsigned char)(y+4u),3,1);
    }
}

static void render_final_score(void)
{
    unsigned int value=snake_get_score();
    if (value>9999u) value=9999u;
    score_text[7]=(char)('0'+value/1000);
    score_text[8]=(char)('0'+(value/100)%10);
    score_text[9]=(char)('0'+(value/10)%10);
    score_text[10]=(char)('0'+value%10);
    display_text_ram(0,18,score_text);
    display_text(0,31,restart_text);
    display_text(0,40,menu_text);
}

void game_render(void)
{
    unsigned char i,x,y;
    SnakePoint point;
    SnakeState state=snake_state();
    display_clear();
    if (state==SNAKE_GAME_OVER) {
        display_text(0,4,over_text);
        render_final_score();
        return;
    }
    if (state==SNAKE_WIN) {
        display_text(0,4,win_text);
        render_final_score();
        return;
    }
    for (y=0; y<GRID_ROWS; ++y) for (x=0; x<GRID_COLS; ++x)
        if (game_map_is_blocked(x,y))
            display_rect((unsigned char)(x*CELL_SIZE),
                         (unsigned char)(y*CELL_SIZE),CELL_SIZE,CELL_SIZE);
    for (i=0;i<dynamic_obstacle_get_count();++i) {
        point=dynamic_obstacle_point(i);
        display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                          (unsigned char)(point.y*CELL_SIZE),
                          DYNAMIC_OBSTACLE_LENGTH*CELL_SIZE,CELL_SIZE);
    }
    point=snake_food();
    render_fruit(point,snake_get_food_type());
    for (i=0; i<snake_length(); ++i) {
        point=snake_segment(i);
        display_fill_rect((unsigned char)(point.x*CELL_SIZE),
                          (unsigned char)(point.y*CELL_SIZE),
                          CELL_SIZE-1,CELL_SIZE-1);
    }
}
