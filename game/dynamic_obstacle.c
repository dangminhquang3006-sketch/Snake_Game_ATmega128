#include "../config/board_config.h"
#include "game_map.h"
#include "game_mode.h"
#include "bonus_food.h"
#include "dynamic_obstacle.h"

/* 8 row masks use 16 bytes; queue and visited are static to bound stack. */
static unsigned int blocked_rows[GRID_ROWS];
static unsigned char queue[GRID_COLS*GRID_ROWS];
static unsigned int visited[GRID_ROWS];
static unsigned char obstacle_count;
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
    return (unsigned char)(game_map_get_free_cells()-obstacle_count);
}

SnakePoint dynamic_obstacle_point(unsigned char index)
{
    unsigned char x,y,count=0;
    SnakePoint point={0,0};
    if (index>=obstacle_count) return point;
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (dynamic_obstacle_is_blocked(x,y)) {
            if (count==index) { point.x=x; point.y=y; return point; }
            ++count;
        }
    return point;
}

static unsigned char is_wall(unsigned char x,unsigned char y,
                             unsigned char candidate_x,unsigned char candidate_y)
{
    return (unsigned char)(game_map_is_blocked(x,y) ||
        dynamic_obstacle_is_blocked(x,y) ||
        (x==candidate_x && y==candidate_y));
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
    unsigned char start=0,end=0,cell,x,y,seen=0,free_count=0;
    SnakePoint head=snake_segment(0);
    SnakePoint food=snake_food();
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
    }
    return (unsigned char)(seen==free_count &&
        (visited[food.y]&(1u<<food.x))!=0u);
}

unsigned char dynamic_obstacle_candidate_valid(unsigned char x,unsigned char y)
{
    unsigned char i,distance;
    SnakePoint head,point,food;
    signed int next_x,next_y;
    if (game_mode_get_current()!=MODE_SURVIVAL ||
        x>=GRID_COLS || y>=GRID_ROWS ||
        game_map_is_blocked(x,y) || dynamic_obstacle_is_blocked(x,y)) return 0;
    head=snake_segment(0);
    distance=(unsigned char)(((x>head.x)?(x-head.x):(head.x-x))+
                             ((y>head.y)?(y-head.y):(head.y-y)));
    if (distance<=DYNAMIC_OBSTACLE_SAFE_DIST) return 0;
    snake_next_head(&next_x,&next_y);
    if (next_x==(signed int)x && next_y==(signed int)y) return 0;
    food=snake_food();
    if (x==food.x && y==food.y) return 0;
    if (bonus_food_active()) {
        point=bonus_food_position();
        if (x==point.x && y==point.y) return 0;
    }
    for (i=0;i<snake_length();++i) {
        point=snake_segment(i);
        if (point.x==x && point.y==y) return 0;
    }
    return remains_connected(x,y);
}

unsigned char dynamic_obstacle_update(unsigned long elapsed_running_ms)
{
    unsigned char offset,i,x,y;
    if (game_mode_get_current()!=MODE_SURVIVAL ||
        snake_state()!=SNAKE_RUNNING ||
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
            blocked_rows[y]|=(unsigned int)(1u<<x);
            ++obstacle_count;
            return 1;
        }
    }
    return 0;
}
