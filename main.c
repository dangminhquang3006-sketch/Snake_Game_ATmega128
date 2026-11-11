#include <mega128.h>
#include "display.h"

static flash char banner[]="SNAKE GAME";

void main(void)
{
    if (!display_init()) {
        while (1) { }
    }
    display_clear();
    display_text(9,4,banner);
    display_fill_rect(23,23,7,7);
    display_fill_rect(31,23,7,7);
    display_fill_rect(39,23,7,7);
    display_fill_rect(58,24,4,4);
    while (1) { }
}
