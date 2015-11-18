/*
 * 7-Segment Character Maps
 *          _____
 *       f / a  / b
 *        /____/
 *     e / g  / c
 *      /____/ o dp
 *       d
 */

#include <avr/io.h>
#include <util/delay.h>

uint8_t digit[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

void writeDigit(uint8_t);

int main() {
  // PB0 - Data (DS)
  // PB1 - Shift clock (SH_CP)
  // PB2 - Store Clock (ST_CP)
  DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
  while (1) {
    for (uint8_t i = 0; i < 10; i++) {
      writeDigit(digit[i]);
      PORTB |= (1 << PB2);
      _delay_loop_1(1);
      PORTB &= ~(1 << PB2);
      _delay_ms(500);
    }
  }
}

void writeDigit(uint8_t value) {
  for (uint8_t i = 0; i < 8; i++) {
    if (1 << i & value)
      PORTB |= (1 << PB0);
    else
      PORTB &= ~(1 << PB0);
    PORTB |= (1 << PB1);
    PORTB &= ~(1 << PB1);
  }
}
