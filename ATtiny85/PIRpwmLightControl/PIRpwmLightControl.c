#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t brightness = 0;
uint8_t maxBright = 255;

ISR(INT0_vect) {
  switch (maxBright) {
  case 255:
    maxBright = 0;
    break;
  case 0:
    maxBright = 64;
    break;
  case 64:
    maxBright = 128;
    break;
  default:
    maxBright = 255;
  }
  OCR0A = brightness = maxBright;
}

ISR(PCINT0_vect) {
  for (int i = brightness; i < maxBright; ++i) {
    brightness = i;
    OCR0A = brightness;
    _delay_ms(10);
  }
}

int main() {
  // initialise timer0 in PWM mode using PB0
  TCCR0A |= (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
  TCCR0B |= (1 << CS00);
  DDRB |= (1 << PB0);
  OCR0A = 0;

  // initialise interrupt on PB4, PIR sensor
  GIMSK |= (1 << PCIE);
  PCMSK |= (1 << PCINT4);

  // initialise interrupt on INT0 (PB2), change max brightness
  GIMSK |= (1 << INT0);
  MCUCR |= (1 << ISC01); // Falling edge trigger

  // set (global) interrupt enable bit
  sei();

  while (1)
    ; // run forever

  return 0; // should never get here
}
