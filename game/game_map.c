#include "../config/board_config.h"
#include "game_map.h"
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
