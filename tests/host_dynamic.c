#include <assert.h>
#include <stdio.h>
#include "../game/game.c"

/* Exercise placement against every fixed layout and wrapping rule. */
static void check_bars_all_maps(void)
{
    unsigned char map,level,seed,attempt,i,j,x,y,count;
    SnakePoint start;
    for (map=0;map<MAP_COUNT;++map)
        for (level=0;level<3u;++level)
            for (seed=1;seed<=32u;++seed) {
                assert(game_map_select((MapId)map));
                snake_set_level((GameLevel)level);
                rng_state=seed;
                snake_init();
                assert(!dynamic_obstacle_candidate_valid(GRID_COLS-2u,0));
                assert(dynamic_obstacle_update(DYNAMIC_OBSTACLE_INTERVAL_MS));
                for (attempt=0;attempt<20u;++attempt)
                    dynamic_obstacle_update(DYNAMIC_OBSTACLE_INTERVAL_MS);
                count=0;
                for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
                    if (dynamic_obstacle_is_blocked(x,y)) {
                        ++count;
                        assert(!game_map_is_blocked(x,y));
                        assert(!occupied(x,y));
                        assert(!(food.x==x && food.y==y));
                    }
                assert(count==DYNAMIC_OBSTACLE_LENGTH*obstacle_count);
                assert(dynamic_obstacle_playable_cells()==
                       game_map_get_free_cells()-count);
                /* An off-board candidate adds nothing: check current graph. */
                assert(remains_connected(0u,GRID_ROWS));
                for (i=0;i<obstacle_count;++i) {
                    start=dynamic_obstacle_point(i);
                    assert(start.x<=GRID_COLS-DYNAMIC_OBSTACLE_LENGTH);
                    for (j=0;j<DYNAMIC_OBSTACLE_LENGTH;++j)
                        assert(dynamic_obstacle_is_blocked(start.x+j,start.y));
                }
            }
}

int main(void)
{
    unsigned char x,y,i;
    signed int next_x,next_y;
    assert(game_mode_select(MODE_SURVIVAL));
    assert(game_map_select(MAP_CLASSIC));
    snake_init();
    /* Seven wall cells leave one bridge at (4,4). Closing it disconnects
       the left and right halves even though it is outside head safety zone. */
    for (y=0;y<GRID_ROWS;++y)
        if (y!=4u) blocked_rows[y]=(unsigned int)(1u<<4);
    obstacle_count=7u;
    segments[0].x=10u; segments[0].y=4u;
    food.x=8; food.y=4;
    assert(!dynamic_obstacle_candidate_valid(4,4));
    snake_set_level(GAME_LEVEL_NORMAL);
    segments[1].x=8u; segments[1].y=3u;
    segments[2].x=9u; segments[2].y=3u;
    assert(dynamic_obstacle_candidate_valid(4,4));
    snake_set_level(GAME_LEVEL_EASY);
    assert(dynamic_obstacle_candidate_valid(0,0));
    assert(!dynamic_obstacle_candidate_valid(4,3));
    assert(!dynamic_obstacle_candidate_valid(8,4));
    assert(!dynamic_obstacle_candidate_valid(7,4));
    dynamic_obstacle_reset();
    snake_set_level(GAME_LEVEL_NORMAL);
    segments[0].x=13u; segments[0].y=4u;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    food.x=8u; food.y=4u;
    assert(!dynamic_obstacle_candidate_valid(0u,4u));
    snake_next_head(&next_x,&next_y);
    assert(next_x==0 && next_y==4);
    blocked_rows[4]=1u;
    obstacle_count=1u;
    assert(snake_step()==1 && snake_state()==SNAKE_GAME_OVER);
    assert(segments[0].x==13u && segments[0].y==4u);
    snake_set_level(GAME_LEVEL_EASY);
    snake_init();
    snake_set_level(GAME_LEVEL_HARD);
    blocked_rows[7]=(unsigned int)(1u<<13);
    obstacle_count=1u;
    elapsed_ms=12345UL;
    food.x=8u; food.y=4u; food_type=FOOD_CIRCLE;
    assert(snake_step()==1 && snake_state()==SNAKE_RUNNING);
    assert(segments[0].x==5u && direction==DIR_LEFT);
    assert(obstacle_count==1u && elapsed_ms==12345UL);
    assert(!dynamic_obstacle_candidate_valid(segments[0].x,segments[0].y));
    snake_set_level(GAME_LEVEL_EASY);
    snake_init();
    blocked_rows[7]=(unsigned int)(1u<<13);
    obstacle_count=1u;
    current_length=110u;
    segments[0].x=0; segments[0].y=0;
    i=1;
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (!(y==0u && (x==0u || x==1u)) &&
            !(y==7u && x==13u)) {
            segments[i].x=x; segments[i].y=y; ++i;
        }
    assert(i==110u);
    food.x=1; food.y=0;
    food_type=FOOD_CIRCLE;
    score=1070u;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    assert(snake_step()==1);
    assert(snake_state()==SNAKE_WIN);
    assert(snake_length()==111u && snake_get_score()==1080u);
    check_bars_all_maps();
    puts("PASS: 3-cell bars across 3 maps, 3 levels, 32 seeds; connectivity and collisions");
    return 0;
}
