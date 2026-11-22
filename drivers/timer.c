#include <mega128.h>
#include "../config/board_config.h"
#include "timer.h"

/* Keep the millisecond API and hardware prescaler in sync with config. */
#if TIMER1_TICKS_PER_SECOND != 1000UL || TIMER1_CLOCK_PRESCALER != 64UL
#error Timer1 requires a 1 ms tick and clock/64 prescaler
#endif
#define TIMER1_CLOCK_DIVISOR (TIMER1_CLOCK_PRESCALER*TIMER1_TICKS_PER_SECOND)
#define TIMER1_TOP ((_MCU_CLOCK_FREQUENCY_ / TIMER1_CLOCK_DIVISOR) - 1UL)
#if (_MCU_CLOCK_FREQUENCY_ % TIMER1_CLOCK_DIVISOR) != 0
#error CPU clock must produce an exact 1 ms Timer1 tick with prescaler 64
#endif
#if TIMER1_TOP > 65535UL
#error Timer1 compare value exceeds 16 bits
#endif

static volatile unsigned long system_ms=0;

interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
    ++system_ms;
}

void timer_init(void)
{
    TCCR1A=0;
    TCCR1B=0;
    TCNT1=0;
    OCR1A=(unsigned int)TIMER1_TOP;
    TIFR=(1<<OCF1A); /* Clear any pending compare flag. */
    TIMSK |= (1<<OCIE1A);
    TCCR1B=(1<<WGM12)|(1<<CS11)|(1<<CS10); /* CTC, clock/64. */
}

unsigned long timer_get_ms(void)
{
    unsigned char saved_sreg;
    unsigned long value;
    saved_sreg=SREG;
    #asm("cli")
    value=system_ms;
    SREG=saved_sreg;
    return value;
}
