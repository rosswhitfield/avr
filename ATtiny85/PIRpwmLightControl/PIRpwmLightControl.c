#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t brightness = 255;
uint8_t time = 0;

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
}

ISR(PCINT0_vect) {
  for (int i = OCR0A; i < brightness; ++i) {
    OCR0A = i;
    _delay_ms(10);
  }
}

ISR(TIM1_OVF_vect) {
  for (int i = OCR0A; i > 0; --i) {
    OCR0A = i;
    _delay_ms(10);
  }
}

int main() {
  // initialise timer0 in PWM mode using PB0
  TCCR0A |= (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
  TCCR0B |= (1 << CS00);
  DDRB |= (1 << PB0);
  OCR0A = 0;

  // initialise timer1 for turnoff timer
  TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10);
  TIMSK |= (1 << TOIE1);

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
