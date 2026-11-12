#include <assert.h>
#include <stdio.h>
/* Include implementation to exercise the hard-to-reach tail and win states. */
#include "../game/snake_game.c"

static void check_food(void)
{
    unsigned char i;
    SnakePoint point=snake_food();
    assert(point.x<GRID_COLS && point.y<GRID_ROWS);
    for (i=0; i<snake_length(); ++i) {
        SnakePoint body=snake_segment(i);
        assert(body.x!=point.x || body.y!=point.y);
    }
}

int main(void)
{
    SnakePoint head;
    unsigned char x,y,i;

    snake_init();
    assert(snake_length()==3 && snake_get_score()==0);
    head=snake_segment(0); assert(head.x==7 && head.y==4);
    head=snake_segment(1); assert(head.x==6 && head.y==4);
    head=snake_segment(2); assert(head.x==5 && head.y==4);
    check_food();

    snake_handle_key(KEY_LEFT); /* Opposite of RIGHT is ignored. */
    snake_handle_key(KEY_UP);
    snake_handle_key(KEY_DOWN); /* A second turn cannot be queued. */
    snake_step();
    head=snake_segment(0); assert(head.x==7 && head.y==3);
    snake_handle_key(KEY_PAUSE);
    assert(snake_state()==SNAKE_PAUSED && snake_step()==0);
    head=snake_segment(0); assert(head.x==7 && head.y==3);
    snake_handle_key(KEY_PAUSE);
    assert(snake_state()==SNAKE_RUNNING);

    snake_handle_key(KEY_RESTART);
    food.x=8; food.y=4;
    snake_step();
    assert(snake_length()==4 && snake_get_score()==10);
    head=snake_segment(0); assert(head.x==8 && head.y==4);
    check_food();

    snake_handle_key(KEY_RESTART);
    segments[0].x=13; segments[0].y=4;
    food.x=0; food.y=0;
    snake_step();
    assert(snake_state()==SNAKE_GAME_OVER);
    assert(segments[0].x==13 && segments[0].y==4);
    assert(snake_step()==0);

    snake_handle_key(KEY_RESTART);
    current_length=4;
    segments[0].x=5; segments[0].y=5;
    segments[1].x=5; segments[1].y=4;
    segments[2].x=4; segments[2].y=4;
    segments[3].x=4; segments[3].y=5;
    direction=DIR_UP; next_direction=DIR_UP;
    food.x=0; food.y=0;
    snake_step();
    assert(snake_state()==SNAKE_GAME_OVER);

    snake_handle_key(KEY_RESTART);
    current_length=4;
    segments[0].x=4; segments[0].y=4;
    segments[1].x=4; segments[1].y=5;
    segments[2].x=3; segments[2].y=5;
    segments[3].x=3; segments[3].y=4;
    direction=DIR_LEFT; next_direction=DIR_LEFT;
    food.x=0; food.y=0;
    snake_step();
    head=snake_segment(0);
    assert(snake_state()==SNAKE_RUNNING && head.x==3 && head.y==4);
    assert(snake_length()==4);

    snake_handle_key(KEY_RESTART);
    current_length=111;
    segments[0].x=0; segments[0].y=0;
    i=1;
    for (y=0; y<GRID_ROWS; ++y)
        for (x=0; x<GRID_COLS; ++x)
            if (!(y==0 && (x==0 || x==1))) {
                segments[i].x=x; segments[i].y=y;
                ++i;
            }
    assert(i==111);
    food.x=1; food.y=0;
    score=1080;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    snake_step();
    assert(snake_state()==SNAKE_WIN);
    assert(snake_length()==112 && snake_get_score()==1090);
    assert(snake_step()==0);
    puts("PASS: snake init, turns, food, pause, collisions, tail, win");
    return 0;
}
