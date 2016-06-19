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

#define FOSC 16000000 // Clock Speed
#define BAUDRATE 9600
#define MYUBRR FOSC / 16 / BAUDRATE - 1

// en.wikipedia.org/wiki/Seven-segment_display#Displaying_letters
const uint8_t digit[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66,
                         0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile uint16_t centiSeconds = 0;
volatile uint16_t minutes = 0;
volatile uint8_t digit_select = 0;

void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
void writeDigit(uint8_t, uint8_t);
void writeSegments(uint8_t);
void selectDigit(uint8_t);
void toggleLatchPB2();

int main() {
  // USART Initialization
  USART_Init(MYUBRR);

  // Seven segment
  // PB0 - Data, PB1 - Clock, PB2 - Latch
  DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

  // Digit select
  // PD2 - Data, PD3 - Clock, PD4 - Latch
  DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4);

  // Start CTC timer1 with interrupt on OCF1A match
  OCR1A = 19999;                        // 0.01 seconds
  TCCR1B |= (1 << WGM12) | (1 << CS10); // CTC
  TIMSK1 |= (1 << OCIE1A);              // Interrupt match OCF1A

  // Start timer0 with overflow interrupt
  TCCR0B |= (1 << CS01);  // clk/8 prescaler
  TIMSK0 |= (1 << TIFR0); // Enable Overflow Interrupt

  // Enable global interrupt
  sei();

  while (1) {
    USART_Transmit(minutes / 600 + 48);
    USART_Transmit(minutes / 60 % 10 + 48);
    USART_Transmit(':');
    USART_Transmit(minutes % 60 / 10 + 48);
    USART_Transmit(minutes % 10 + 48);
    USART_Transmit(':');
    USART_Transmit(centiSeconds / 1000 + 48);
    USART_Transmit(centiSeconds / 100 % 10 + 48);
    USART_Transmit('\n');
    USART_Transmit('\r');
    _delay_ms(1000);
  }
}

void USART_Init(unsigned int ubrr) {
  // Set baud rate
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  // Enable transmitter
  UCSR0B = (1 << TXEN0);
  // Set frame format: 8bit data
  UCSR0C = (1 << UMSEL00) | (1 << UCSZ00) | (1 << UCSZ01);
}

void USART_Transmit(unsigned char data) {
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  // Put data into buffer, sends the data
  UDR0 = data;
}

ISR(TIMER0_OVF_vect) {
  switch (digit_select) {
  case 0:
    writeDigit(centiSeconds % 10, 0);
    break;
  case 1:
    writeDigit(centiSeconds / 10 % 10, 1);
    break;
  case 2:
    writeDigit(centiSeconds / 100 % 10, 2);
    break;
  case 3:
    writeDigit(centiSeconds / 1000, 3);
    break;
  case 4:
    writeDigit(minutes % 10, 4);
    break;
  case 5:
    writeDigit(minutes % 60 / 10, 5);
    break;
  case 6:
    writeDigit(minutes / 60 % 10, 6);
    break;
  case 7:
    writeDigit(minutes / 600, 7);
    break;
  }
  digit_select = (digit_select + 1) & 7;
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
