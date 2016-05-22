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

// en.wikipedia.org/wiki/Seven-segment_display#Displaying_letters
const uint8_t digit[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66,
                         0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile uint16_t centiSeconds = 0;
volatile uint16_t minutes = 0;

void writeDigit(uint8_t, uint8_t);
void writeSegments(uint8_t);
void selectDigit(uint8_t);
void toggleLatchPB2();

int main() {
  // Seven segment
  // PB0 - Data, PB1 - Clock, PB2 - Latch
  DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

  // Digit select
  // PD2 - Data, PD3 - Clock, PD4 - Latch
  DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4);

  // Start CTC timer1 with interrupt on OCF1A match
  OCR1A = 10000;                        // 0.01 seconds
  TCCR1B |= (1 << WGM12) | (1 << CS10); // CTC
  TIMSK1 |= (1 << OCIE1A);              // Interrupt match OCF1A
  sei();                                // Enable global interrupt

  uint16_t tmp;
  while (1) {
    // Write seconds and centiSeconds
    tmp = centiSeconds;
    for (uint8_t n = 0; n < 4; n++) {
      writeDigit(tmp % 10, n);
      tmp /= 10;
    }
    // Write minutes
    tmp = minutes % 60;
    writeDigit(tmp % 10, 4);
    tmp /= 10;
    writeDigit(tmp % 10, 5);
    // Write hours
    tmp = minutes / 60;
    writeDigit(tmp % 10, 6);
    tmp /= 10;
    writeDigit(tmp % 10, 7);
  }
}

ISR(TIMER1_COMPA_vect) {
  centiSeconds++;
  if (centiSeconds == 6000) {
    centiSeconds = 0;
    minutes++;
    if (minutes == 1440)
      minutes = 0;
  }
}

void writeDigit(uint8_t value, uint8_t location) {
  writeSegments(digit[value]); // Select segments
  selectDigit(-1);             // Deselect digit
  toggleLatchPB2();            // Display segments
  selectDigit(location);       // Select digit n
  _delay_loop_1(100);
}

void writeSegments(uint8_t value) {
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
  // Toggle latch
  PORTD |= (1 << PD4);
  _delay_loop_1(1);
  PORTD &= ~(1 << PD4);
  _delay_loop_1(1);
}
