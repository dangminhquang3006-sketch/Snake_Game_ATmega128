#ifndef HOST_MOCK_MEGA128_H
#define HOST_MOCK_MEGA128_H
/* Host tests only: do not add this directory to the CodeVisionAVR project. */
#define flash
#define eeprom
extern unsigned char PORTA,PORTB,PORTD,PORTF,DDRA,DDRB,DDRD,DDRF,PINB;
extern unsigned char SREG,TCCR1A,TCCR1B,TIFR,TIMSK;
extern unsigned int TCNT1,OCR1A;
extern unsigned char host_bits[168];
#endif
