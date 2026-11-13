#include "game_mode.h"
#include "bonus_food.h"

static unsigned char active;
static SnakePoint position;
static unsigned long remaining_ms;

void bonus_food_reset(void) { active=0; remaining_ms=0; }
unsigned char bonus_food_active(void) { return active; }
SnakePoint bonus_food_position(void) { return position; }
void bonus_food_spawn(SnakePoint point)
{
    position=point;
    remaining_ms=BONUS_LIFETIME_MS;
    active=1;
}
void bonus_food_collect(void) { active=0; remaining_ms=0; }
unsigned char bonus_food_advance(unsigned long elapsed_ms)
{
    if (!active) return 0;
    if (elapsed_ms>=remaining_ms) { bonus_food_collect(); return 1; }
    remaining_ms-=elapsed_ms;
    return 0;
}
unsigned long bonus_food_remaining_ms(void) { return remaining_ms; }
