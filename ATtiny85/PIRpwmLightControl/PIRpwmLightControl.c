/*         +----------+
           |          |
           |1        8|---- +5V
           |          |         __|__
           |2        7|---------o   o--- GND
           | ATtiny85 |
   PIR ----|3        6|      GND        +5V 
           |          |    |/           _|_
   GND ----|4        5|----| MOSFET    _\_/_ LED
           |          |    |\            |
           +----------+      +-----+-----+      */

#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t brightness = 255;
uint8_t time = 0; // 1 equals ~4.2sec at 1MHz

ISR(INT0_vect) {
  switch (brightness) {
  case 255:
    brightness = 0;
    break;
  case 0:
    brightness = 64;
    break;
  case 64:
    brightness = 128;
    break;
  default:
    brightness = 255;
  }
  OCR0A = brightness;
  time = 0;
}

ISR(PCINT0_vect) {
  for (int i = OCR0A; i <= brightness; i++) {
    OCR0A = i;
    _delay_ms(10);
  }
  time = 0;
}

ISR(TIM1_OVF_vect) {
  time++;
  if (time >= 30)
    for (int i = OCR0A - 1; i >= 0; i--) {
      OCR0A = i;
      _delay_ms(10);
    }
}

int main() {
  // timer0 in fast PWM mode using PB0
  TCCR0A |= (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
  TCCR0B |= (1 << CS00);
  DDRB |= (1 << PB0);
  OCR0A = 0;

  // timer1 interrupt for turnoff timer
  TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10);
  TIMSK |= (1 << TOIE1);

  // interrupt on PCINT4 (PB4), PIR sensor
  GIMSK |= (1 << PCIE);
  PCMSK |= (1 << PCINT4);

  // interrupt on INT0 (PB2), change max brightness
  GIMSK |= (1 << INT0);
  MCUCR |= (1 << ISC01); // Falling edge trigger

  // set (global) interrupt enable bit
  sei();

  while (1)
    ; // do nothing forever

  return 0; // should never get here
}
