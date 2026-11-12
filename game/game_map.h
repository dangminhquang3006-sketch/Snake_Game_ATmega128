#ifndef GAME_MAP_H
#define GAME_MAP_H

typedef enum {
    MAP_CLASSIC=0,
    MAP_PILLARS,
    MAP_LABYRINTH,
    MAP_COUNT
} MapId;

unsigned char game_map_select(MapId map);
MapId game_map_get_current(void);
unsigned char game_map_is_blocked(signed int x, signed int y);
unsigned char game_map_get_free_cells(void);
unsigned char game_map_free_cells_for(MapId map);
unsigned char game_map_validate(MapId map);

#endif
