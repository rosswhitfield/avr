// program to change brightness of an LED
// demonstration of PWM

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t brightness = 0;

// initialize PWM
void pwm_init()
{
  // initialize timer0 in PWM mode
  TCCR0A |= (1<<WGM00)|(1<<COM0A1)|(1<<WGM01);
  TCCR0B |= (1<<CS00);

  // make sure to make OC0 pin (pin PB3 for atmega32) as output pin
  DDRB |= (1<<PB0);
  OCR0A = 0;
}

/* Run every time button state changes */
ISR(PCINT0_vect) {
  brightness += 32;
  OCR0A = brightness;
}

// initalise interrupt
void initPinChangeInterrupt4(void) {
  GIMSK |= (1 << PCIE);        /* set pin-change interrupt for D pins */
  PCMSK |= (1 << PCINT4);      /* set mask to look for PCINT18 / PD2 */
  sei();                          /* set (global) interrupt enable bit */
}

int main()
{
  // initialize timer0 in PWM mode
  pwm_init();

  // initalise interrupt
  DDRB |= (1<<PB4);  // set output
  PORTB |= (1<<PB4); // pullup
  initPinChangeInterrupt4();

  // run forever
  while(1) {}
  return 0;
}
