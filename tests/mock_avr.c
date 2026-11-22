/* Host-only GPIO and EEPROM backend. Never add this file to CodeVision. */
#include "mock/mega128.h"

unsigned char PORTA,PORTB,PORTD,PORTF,DDRA,DDRB,DDRD,DDRF,PINB;
unsigned char SREG,TCCR1A,TCCR1B,TIFR,TIMSK;
unsigned int TCNT1,OCR1A;
unsigned char host_bits[168];
unsigned char led7_mock_eeprom=0xff;
unsigned int led7_mock_eeprom_writes=0;
unsigned char led7_mock_eeprom_fail=0;

unsigned char led7seg_test_eeprom_read(void)
{
    return led7_mock_eeprom;
}

void led7seg_test_eeprom_write(unsigned char value)
{
    ++led7_mock_eeprom_writes;
    if (!led7_mock_eeprom_fail) led7_mock_eeprom=value;
}
