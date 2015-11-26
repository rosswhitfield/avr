/*
 * 7-Segment Character Maps
 *          _____
 *       f / a  / b
 *        /____/
 *     e / g  / c
 *      /____/ o dp
 *       d
 */

#include <avr/interrupt.h>
#include <util/delay.h>

const uint8_t digit[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66,
                         0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile uint16_t centiSeconds = 0;
volatile uint16_t minutes = 0;

void writeDigit(uint8_t);
void selectDigit(uint8_t);
void toggleLatchPB2();

ISR(TIMER1_COMPA_vect) {
  centiSeconds++;
  if (centiSeconds == 6000) {
    centiSeconds = 0;
    minutes++;
    if (minutes == 1440)
      minutes = 0;
  }
}

int main() {
  // Seven segment
  // PB0 - Data // PB1 - Clock // PB2 - Latch
  DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

  // Digit select
  // PD2 - Data // PD3 - Clock // PD4 - Latch
  DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4);

  // Start timer
  OCR1A = 10000;
  TCCR1B |= (1 << WGM12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();

  while (1) {
    uint16_t number = centiSeconds;
    for (uint8_t n = 0; n < 4; n++) {
      writeDigit(digit[number % 10]);
      selectDigit(-1);
      toggleLatchPB2();
      selectDigit(n);
      _delay_loop_1(100);
      number /= 10;
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

void toggleLatchPB2() {
  PORTB |= (1 << PB2);
  _delay_loop_1(1);
  PORTB &= ~(1 << PB2);
  _delay_loop_1(1);
}

void selectDigit(uint8_t number) {
  for (uint8_t i = 0; i < 8; i++) {
    if (i == number)
      PORTD &= ~(1 << PB2);
    else
      PORTD |= (1 << PB2);
    PORTD |= (1 << PB3);
    _delay_loop_1(1);
    PORTD &= ~(1 << PB3);
    _delay_loop_1(1);
  }
  PORTD |= (1 << PD4);
  _delay_loop_1(1);
  PORTD &= ~(1 << PD4);
  _delay_loop_1(1);
}
