#include <mega128.h>
#include "nokia5110.h"

void main(void)
{
    nokia_init();
    nokia_clear();
    nokia_text(9,4,"SNAKE GAME");
    nokia_rect(23,23,7,7,1);
    nokia_rect(31,23,7,7,1);
    nokia_rect(39,23,7,7,1);
    nokia_rect(58,24,4,4,1);
    nokia_update();
    while (1) { }
}
