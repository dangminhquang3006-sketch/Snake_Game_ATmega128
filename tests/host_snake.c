#include <assert.h>
#include <stdio.h>
/* Include implementation to exercise the hard-to-reach tail and win states. */
#include "../game/snake_game.c"

static void check_fruit_rules(void)
{
    unsigned int counts[3]={0,0,0};
    unsigned int i;
    unsigned char type;
    assert(snake_food_type_from_roll(0)==FOOD_CIRCLE);
    assert(snake_food_type_from_roll(59)==FOOD_CIRCLE);
    assert(snake_food_type_from_roll(60)==FOOD_SQUARE);
    assert(snake_food_type_from_roll(94)==FOOD_SQUARE);
    assert(snake_food_type_from_roll(95)==FOOD_STAR);
    assert(snake_food_type_from_roll(99)==FOOD_STAR);
    rng_state=0xACE1u;
    for (i=0;i<30000u;++i) {
        type=(unsigned char)snake_food_type_from_roll(
            (unsigned char)(random_next()%100u));
        ++counts[type];
    }
    assert(counts[FOOD_CIRCLE]>17000u && counts[FOOD_CIRCLE]<19000u);
    assert(counts[FOOD_SQUARE]>9500u && counts[FOOD_SQUARE]<11500u);
    assert(counts[FOOD_STAR]>1100u && counts[FOOD_STAR]<1900u);
    snake_init();
    food.x=8; food.y=4; food_type=FOOD_CIRCLE;
    assert(snake_step()==1 && score==10u && current_length==4u);
    food.x=9; food.y=4; food_type=FOOD_SQUARE;
    assert(snake_step()==1 && score==30u && current_length==5u);
    food.x=10; food.y=4; food_type=FOOD_STAR;
    assert(snake_step()==1 && score==80u && current_length==6u);
}

static void check_wrap_rules(void)
{
    unsigned char level,edge;
    signed int next_x,next_y;
    SnakePoint head;
    assert(game_map_select(MAP_CLASSIC));
    assert(game_mode_select(MODE_STANDARD));
    for (level=0u;level<3u;++level) {
        snake_set_level((GameLevel)level);
        assert(snake_can_wrap()==(unsigned char)(level!=0u));
        for (edge=0u;edge<4u;++edge) {
            snake_init();
            food.x=6u; food.y=5u;
            if (edge==0u) {
                segments[0].x=13u; segments[0].y=4u;
                direction=DIR_RIGHT;
            } else if (edge==1u) {
                segments[0].x=0u; segments[0].y=4u;
                direction=DIR_LEFT;
            } else if (edge==2u) {
                segments[0].x=6u; segments[0].y=0u;
                direction=DIR_UP;
            } else {
                segments[0].x=6u; segments[0].y=7u;
                direction=DIR_DOWN;
            }
            next_direction=direction;
            snake_next_head(&next_x,&next_y);
            assert(snake_step()==1);
            if (level==0u) {
                assert(snake_state()==SNAKE_GAME_OVER);
            } else {
                assert(snake_state()==SNAKE_RUNNING);
                head=snake_segment(0);
                assert(head.x==(unsigned char)next_x &&
                       head.y==(unsigned char)next_y);
                assert(head.x==((edge==0u)?0u:((edge==1u)?13u:6u)));
                assert(head.y==((edge==2u)?7u:((edge==3u)?0u:4u)));
                assert(direction==next_direction);
            }
        }
    }
    snake_set_level(GAME_LEVEL_NORMAL);
    snake_init();
    segments[0].x=13u; segments[0].y=4u;
    segments[1].x=12u; segments[1].y=4u;
    segments[2].x=0u; segments[2].y=4u;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    food.x=7u; food.y=7u;
    assert(snake_step()==1 && snake_state()==SNAKE_RUNNING);
    assert(segments[0].x==0u && segments[1].x==13u);
    snake_init();
    current_length=4u;
    segments[0].x=13u; segments[0].y=4u;
    segments[1].x=0u; segments[1].y=4u;
    segments[2].x=1u; segments[2].y=4u;
    segments[3].x=12u; segments[3].y=4u;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    food.x=7u; food.y=7u;
    assert(snake_step()==1 && snake_state()==SNAKE_GAME_OVER);
    assert(segments[0].x==13u);
    for (edge=0u;edge<3u;++edge) {
        snake_init();
        segments[0].x=13u; segments[0].y=4u;
        direction=DIR_RIGHT; next_direction=DIR_RIGHT;
        food.x=0u; food.y=4u; food_type=(FoodType)edge;
        assert(snake_step()==1 && current_length==4u);
        assert(score==((edge==0u)?10u:((edge==1u)?20u:50u)));
        assert(segments[0].x==0u && segments[0].y==4u);
    }
    assert(game_mode_select(MODE_BONUS));
    snake_init();
    segments[0].x=13u; segments[0].y=4u;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    food.x=7u; food.y=7u;
    head.x=0u; head.y=4u; bonus_food_spawn(head);
    assert(snake_step()==1 && score==30u && current_length==3u);
    assert(!bonus_food_active());
    snake_handle_key(KEY_RESTART);
    assert(snake_can_wrap() && snake_get_score()==0u && snake_length()==3u);
    assert(game_mode_select(MODE_STANDARD));
    snake_set_level(GAME_LEVEL_EASY);
}

static void check_reverse_rules(void)
{
    unsigned char kind,edge,i;
    SnakePoint head,neck;
    assert(game_map_select(MAP_CLASSIC));
    assert(game_mode_select(MODE_STANDARD));
    for (kind=0u;kind<3u;++kind) {
        snake_set_level(GAME_LEVEL_HARD);
        snake_init();
        food.x=8u; food.y=4u; food_type=(FoodType)kind;
        assert(snake_step()==1 && snake_state()==SNAKE_RUNNING);
        assert(current_length==4u);
        assert(score==((kind==0u)?10u:((kind==1u)?20u:50u)));
        assert(segments[0].x==5u && segments[1].x==6u);
        assert(segments[2].x==7u && segments[3].x==8u);
        assert(direction==DIR_LEFT && next_direction==DIR_LEFT && !turn_queued);
        snake_handle_key(KEY_RIGHT); /* Direct return into neck is rejected. */
        assert(!turn_queued);
        food.x=10u; food.y=7u;
        assert(snake_step()==1 && segments[0].x==4u);
    }
    snake_init();
    food.x=8u; food.y=4u; food_type=FOOD_CIRCLE;
    assert(snake_step()==1);
    food.x=4u; food.y=4u; food_type=FOOD_SQUARE;
    assert(snake_step()==1 && score==30u && current_length==5u);
    assert(segments[0].x==8u && direction==DIR_RIGHT);
    food.x=9u; food.y=4u; food_type=FOOD_STAR;
    assert(snake_step()==1 && score==80u && current_length==6u);
    assert(segments[0].x==4u && direction==DIR_LEFT);
    snake_handle_key(KEY_PAUSE);
    assert(snake_step()==0 && segments[0].x==4u);
    snake_handle_key(KEY_PAUSE);
    food.x=10u; food.y=7u;
    assert(snake_step()==1 && segments[0].x==3u && score==80u);

    /* A bent tail points down after reversal, not opposite the old RIGHT. */
    snake_init();
    current_length=4u;
    segments[0].x=7u; segments[0].y=4u;
    segments[1].x=6u; segments[1].y=4u;
    segments[2].x=5u; segments[2].y=4u;
    segments[3].x=5u; segments[3].y=5u;
    food.x=8u; food.y=4u; food_type=FOOD_CIRCLE;
    assert(snake_step()==1 && current_length==5u);
    assert(segments[0].x==5u && segments[0].y==5u);
    assert(direction==DIR_DOWN);
    food.x=10u; food.y=7u;
    assert(snake_step()==1 && segments[0].y==6u);

    /* Old tail and neck can be adjacent across each of the four edges. */
    for (edge=0u;edge<4u;++edge) {
        snake_init();
        if (edge==0u) {
            segments[0].x=12u; segments[0].y=4u;
            segments[1].x=13u; segments[1].y=4u;
            segments[2].x=0u; segments[2].y=4u;
            direction=DIR_UP; food.x=12u; food.y=3u;
        } else if (edge==1u) {
            segments[0].x=1u; segments[0].y=4u;
            segments[1].x=0u; segments[1].y=4u;
            segments[2].x=13u; segments[2].y=4u;
            direction=DIR_UP; food.x=1u; food.y=3u;
        } else if (edge==2u) {
            segments[0].x=6u; segments[0].y=6u;
            segments[1].x=6u; segments[1].y=7u;
            segments[2].x=6u; segments[2].y=0u;
            direction=DIR_LEFT; food.x=5u; food.y=6u;
        } else {
            segments[0].x=6u; segments[0].y=1u;
            segments[1].x=6u; segments[1].y=0u;
            segments[2].x=6u; segments[2].y=7u;
            direction=DIR_LEFT; food.x=5u; food.y=1u;
        }
        next_direction=direction; food_type=FOOD_CIRCLE;
        assert(snake_step()==1 && snake_state()==SNAKE_RUNNING);
        head=segments[0]; neck=segments[1];
        assert(head.x==((edge==0u)?0u:((edge==1u)?13u:6u)));
        assert(head.y==((edge==2u)?0u:((edge==3u)?7u:4u)));
        assert(neck.x==((edge==0u)?13u:((edge==1u)?0u:6u)));
        assert(neck.y==((edge==2u)?7u:((edge==3u)?0u:4u)));
        assert(direction==((edge==0u)?DIR_RIGHT:((edge==1u)?DIR_LEFT:
                           ((edge==2u)?DIR_DOWN:DIR_UP))));
    }

    /* Bonus has no growth or reversal, even in HARD. */
    assert(game_mode_select(MODE_BONUS));
    snake_init();
    food.x=10u; food.y=7u;
    head.x=8u; head.y=4u; bonus_food_spawn(head);
    assert(snake_step()==1 && score==30u && current_length==3u);
    assert(segments[0].x==8u && direction==DIR_RIGHT);
    assert(normal_foods_eaten==0u && !bonus_food_active());
    snake_handle_key(KEY_RESTART);
    assert(current_length==3u && score==0u && direction==DIR_RIGHT);
    assert(snake_can_wrap());
    for (i=0u;i<3u;++i) assert(segments[i].x==(unsigned char)(7u-i));
    current_length=SNAKE_MAX_LENGTH;
    for (i=0u;i<current_length;++i) {
        segments[i].x=i;
        segments[i].y=0u;
    }
    reverse_segments();
    for (i=0u;i<current_length;++i)
        assert(segments[i].x==(unsigned char)(current_length-1u-i));
    assert(game_mode_select(MODE_STANDARD));
    snake_set_level(GAME_LEVEL_EASY);
}

static void check_food(void)
{
    unsigned char i;
    SnakePoint point=snake_food();
    assert(point.x<GRID_COLS && point.y<GRID_ROWS);
    assert(!game_map_is_blocked(point.x,point.y));
    for (i=0; i<snake_length(); ++i) {
        SnakePoint body=snake_segment(i);
        assert(body.x!=point.x || body.y!=point.y);
    }
}

static void check_map_win(MapId map)
{
    unsigned char x,y,i=1;
    assert(game_map_select(map));
    snake_init();
    current_length=(unsigned char)(game_map_get_free_cells()-1u);
    segments[0].x=0; segments[0].y=0;
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (!game_map_is_blocked(x,y) && !(y==0 && (x==0 || x==1))) {
            segments[i].x=x; segments[i].y=y; ++i;
        }
    assert(i==current_length);
    food.x=1; food.y=0;
    food_type=FOOD_CIRCLE;
    score=(unsigned int)(current_length-3u)*10u;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    assert(snake_step()==1);
    assert(snake_state()==SNAKE_WIN);
    assert(snake_length()==game_map_get_free_cells());
    assert(segments[0].x==1u && segments[0].y==0u);
    assert(snake_get_score()==(unsigned int)(game_map_get_free_cells()-3u)*10u);
}

int main(void)
{
    SnakePoint head;
    unsigned char x,y,i;

    assert(game_map_validate(MAP_CLASSIC));
    assert(game_map_validate(MAP_PILLARS));
    assert(game_map_validate(MAP_LABYRINTH));
    assert(game_map_free_cells_for(MAP_CLASSIC)==112);
    assert(game_map_free_cells_for(MAP_PILLARS)==104);
    assert(game_map_free_cells_for(MAP_LABYRINTH)==100);
    assert(!game_map_validate((MapId)-1));
    assert(!game_map_select((MapId)3));
    assert(game_map_select(MAP_CLASSIC));
    check_fruit_rules();

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
    food_type=FOOD_CIRCLE;
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
    assert(game_map_select(MAP_PILLARS));
    snake_init();
    for (i=0;i<50;++i) { check_food(); place_food(); }
    segments[0].x=4; segments[0].y=1;
    direction=DIR_DOWN; next_direction=DIR_DOWN;
    head=segments[0];
    snake_step();
    assert(snake_state()==SNAKE_GAME_OVER);
    assert(segments[0].x==head.x && segments[0].y==head.y);
    assert(game_map_select(MAP_LABYRINTH));
    snake_init();
    for (i=0;i<50;++i) { check_food(); place_food(); }
    check_map_win(MAP_PILLARS);
    check_map_win(MAP_LABYRINTH);
    assert(game_map_select(MAP_CLASSIC));

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
    food_type=FOOD_CIRCLE;
    score=1080;
    direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    snake_step();
    assert(snake_state()==SNAKE_WIN);
    assert(snake_length()==112 && snake_get_score()==1090);
    assert(snake_step()==0);
    assert(game_mode_select(MODE_BONUS));
    snake_init();
    for (i=0;i<3u;++i) {
        segments[0].x=7; segments[0].y=4;
        segments[1].x=6; segments[1].y=4;
        segments[2].x=5; segments[2].y=4;
        current_length=3;
        direction=DIR_RIGHT; next_direction=DIR_RIGHT;
        food.x=8; food.y=4;
        food_type=FOOD_CIRCLE;
        assert(snake_step()==1);
    }
    assert(normal_foods_eaten==3u && bonus_food_active());
    head=bonus_food_position();
    assert(!game_map_is_blocked(head.x,head.y));
    assert(!(head.x==food.x && head.y==food.y));
    assert(bonus_food_remaining_ms()==5000UL);
    assert(!bonus_food_advance(2500UL));
    assert(bonus_food_remaining_ms()==2500UL);
    assert(bonus_food_advance(2500UL) && !bonus_food_active());
    segments[0].x=7; segments[0].y=4;
    segments[1].x=6; segments[1].y=4;
    segments[2].x=5; segments[2].y=4;
    current_length=3; direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    food.x=0; food.y=0;
    head.x=8; head.y=4; bonus_food_spawn(head);
    score=30;
    assert(snake_step()==1);
    assert(score==60u && current_length==3u && !bonus_food_active());
    /* A live bonus cannot consume the sole cell needed by normal food. */
    current_length=111u;
    segments[0].x=0; segments[0].y=0;
    i=1;
    for (y=0;y<GRID_ROWS;++y) for (x=0;x<GRID_COLS;++x)
        if (!(y==0 && (x==0 || x==1))) {
            segments[i].x=x; segments[i].y=y; ++i;
        }
    head.x=1; head.y=0; bonus_food_spawn(head);
    place_food();
    assert(!bonus_food_active());
    assert(food.x==1 && food.y==0);
    assert(game_mode_select(MODE_TIME_ATTACK));
    game_mode_reset();
    assert(game_mode_seconds_display()==60u);
    assert(!game_mode_advance(1UL) && game_mode_seconds_display()==60u);
    assert(!game_mode_advance(999UL) && game_mode_seconds_display()==59u);
    snake_init();
    assert(game_mode_advance(59000UL));
    snake_time_up();
    assert(snake_state()==SNAKE_TIME_UP && snake_step()==0);
    assert(game_mode_remaining_ms()==0UL);
    snake_handle_key(KEY_RESTART);
    game_mode_reset();
    assert(snake_state()==SNAKE_RUNNING && !bonus_food_active());
    assert(game_mode_seconds_display()==60u);
    assert(game_mode_select(MODE_STANDARD));
    snake_init();
    normal_foods_eaten=2u;
    food.x=8; food.y=4;
    food_type=FOOD_CIRCLE;
    assert(snake_step()==1 && !bonus_food_active());
    assert(!dynamic_obstacle_update(20000UL) && dynamic_obstacle_get_count()==0u);
    assert(game_mode_select(MODE_SURVIVAL));
    assert(game_map_select(MAP_PILLARS));
    snake_init();
    assert(dynamic_obstacle_update(20000UL));
    assert(dynamic_obstacle_playable_cells()==103u);
    assert(game_map_select(MAP_LABYRINTH));
    snake_init();
    assert(dynamic_obstacle_update(20000UL));
    assert(dynamic_obstacle_playable_cells()==99u);
    assert(game_map_select(MAP_CLASSIC));
    snake_init();
    assert(dynamic_obstacle_get_count()==0u);
    assert(!dynamic_obstacle_candidate_valid(7,4));
    assert(!dynamic_obstacle_candidate_valid(food.x,food.y));
    assert(!dynamic_obstacle_update(19999UL));
    assert(dynamic_obstacle_get_count()==0u);
    assert(dynamic_obstacle_update(1UL));
    assert(dynamic_obstacle_get_count()==1u);
    head=dynamic_obstacle_point(0);
    assert(dynamic_obstacle_is_blocked(head.x,head.y));
    assert(!(head.x==food.x && head.y==food.y));
    assert(dynamic_obstacle_playable_cells()==111u);
    for (i=0;i<40u;++i) { place_food(); check_food();
        assert(!dynamic_obstacle_is_blocked(food.x,food.y)); }
    snake_handle_key(KEY_PAUSE);
    assert(!dynamic_obstacle_update(60000UL));
    assert(dynamic_obstacle_get_count()==1u);
    snake_handle_key(KEY_PAUSE);
    for (i=0;i<20u;++i) dynamic_obstacle_update(20000UL);
    assert(dynamic_obstacle_get_count()==DYNAMIC_OBSTACLE_MAX_COUNT);
    assert(!dynamic_obstacle_update(20000UL));
    assert(dynamic_obstacle_playable_cells()==
           (unsigned char)(112u-dynamic_obstacle_get_count()));
    head=dynamic_obstacle_point(0);
    food.x=13; food.y=7;
    current_length=3;
    if (head.x>0u) {
        segments[0].x=(unsigned char)(head.x-1u); segments[0].y=head.y;
        direction=DIR_RIGHT; next_direction=DIR_RIGHT;
    } else {
        segments[0].x=1u; segments[0].y=head.y;
        direction=DIR_LEFT; next_direction=DIR_LEFT;
    }
    head=segments[0];
    assert(snake_step()==1 && snake_state()==SNAKE_GAME_OVER);
    assert(segments[0].x==head.x && segments[0].y==head.y);
    snake_handle_key(KEY_RESTART);
    assert(dynamic_obstacle_get_count()==0u && snake_get_score()==0u);
    normal_foods_eaten=2u;
    food.x=8; food.y=4;
    assert(snake_step()==1 && !bonus_food_active());
    check_wrap_rules();
    snake_set_level(GAME_LEVEL_HARD);
    check_map_win(MAP_CLASSIC);
    snake_set_level(GAME_LEVEL_EASY);
    check_reverse_rules();
    puts("PASS: snake/maps, fruit, wrap, reverse, bonus/time attack and survival");
    return 0;
}
