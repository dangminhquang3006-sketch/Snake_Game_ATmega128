#include <assert.h>
#include <stdio.h>
#include "../game/snake_game.c"
#include "../game/dynamic_obstacle.c"

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
    food.x=8; food.y=4;
    assert(!dynamic_obstacle_candidate_valid(4,4));
    snake_set_level(GAME_LEVEL_NORMAL);
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
    puts("PASS: BFS rejects bridge closure and occupied/unsafe cells");
    return 0;
}
