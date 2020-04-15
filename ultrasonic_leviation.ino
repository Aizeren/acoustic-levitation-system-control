#include <avr/power.h>

// Settings for serial port
// Make sure BAUD is the same as in Control App
#define BAUD  115200UL
// #define BRC   ((F_CPU / (16*BAUD)) - 1)
#define RESOLUTION 24
#define STEP_SIZE 1

#define OUTPUT_WAVE(pointer, i) PORTC = pointer[i]

inline unsigned char USART_Receive();
inline void USART_Transmit(unsigned char);

static byte phases[RESOLUTION][RESOLUTION] = 
{{0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa},
{0x9,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x6,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa},
{0x9,0x9,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x6,0x6,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa},
{0x9,0x9,0x9,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x6,0x6,0x6,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa},
{0x9,0x9,0x9,0x9,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x6,0x6,0x6,0x6,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa},
{0x9,0x9,0x9,0x9,0x9,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x6,0x6,0x6,0x6,0x6,0xa,0xa,0xa,0xa,0xa,0xa,0xa},
{0x9,0x9,0x9,0x9,0x9,0x9,0x5,0x5,0x5,0x5,0x5,0x5,0x6,0x6,0x6,0x6,0x6,0x6,0xa,0xa,0xa,0xa,0xa,0xa},
{0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x5,0x5,0x5,0x5,0x5,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0xa,0xa,0xa,0xa,0xa},
{0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x5,0x5,0x5,0x5,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0xa,0xa,0xa,0xa},
{0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x5,0x5,0x5,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0xa,0xa,0xa},
{0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x5,0x5,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0xa,0xa},
{0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x5,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0xa},
{0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6},
{0x5,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0xa,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6},
{0x5,0x5,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0xa,0xa,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6},
{0x5,0x5,0x5,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0xa,0xa,0xa,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6},
{0x5,0x5,0x5,0x5,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0xa,0xa,0xa,0xa,0x6,0x6,0x6,0x6,0x6,0x6,0x6,0x6},
{0x5,0x5,0x5,0x5,0x5,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0xa,0xa,0xa,0xa,0xa,0x6,0x6,0x6,0x6,0x6,0x6,0x6},
{0x5,0x5,0x5,0x5,0x5,0x5,0x9,0x9,0x9,0x9,0x9,0x9,0xa,0xa,0xa,0xa,0xa,0xa,0x6,0x6,0x6,0x6,0x6,0x6},
{0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x9,0x9,0x9,0x9,0x9,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0x6,0x6,0x6,0x6,0x6},
{0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x9,0x9,0x9,0x9,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0x6,0x6,0x6,0x6},
{0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x9,0x9,0x9,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0x6,0x6,0x6},
{0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x9,0x9,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0x6,0x6},
{0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x9,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0x6}};

void setup()
{
  // state byte:
  // bit 0 - Turn On
  // bit 1 - Turn Off
  // bit 2 - Lift Up
  // bit 3 - Initial Position
  // bit 4 - Lift Down
  byte state = 0b00000000;
  byte phaseNum = 2;
  byte tmp = 0;
  byte* emittingPointer = &phases[0][0];
  short phasePartNum = 0;
  // counter for sending/receiving data
  int i = 0;
  // USART BAUD/8-N-1
  Serial.begin(BAUD);
  // UBRR0 = BRC;
  // Enable sending/receiving
  UCSR0B = (1<<TXEN0)|(1<<RXEN0);

  // Pins A0 - A3 are outputs
  // T- T+ B+ B-
  DDRC = 0b00001111;
  PORTC = 0b00000000;

  // Make sure pin 10 connected to pin 11
  pinMode(10, OUTPUT); //pin 10 (B2) will generate a signal to sync 
  pinMode(11, INPUT_PULLUP); //pin 11 (B3) is the sync in
   
  // generate a sync signal of ~ (40khz * resolution)
  noInterrupts();

  // Fast PWM mode, COM1B1 - clear OC1B on compare, CS10 - no prescaler
  TCCR1A = bit (WGM10) | bit (WGM11) | bit (COM1B1);
  TCCR1B = bit (WGM12) | bit (WGM13) | bit (CS10);
  OCR1A =  floor(F_CPU / (24*40000))-1;
  OCR1B = floor(F_CPU / (24*40000*2));
  
  interrupts();

  // Disable some options to free uptime
  ADCSRA = 0;  // ADC
  power_adc_disable ();
  power_spi_disable();
  power_twi_disable();
  power_timer0_disable();
  

  /*------------------------------------------------------------------------*/
  LOOP:
    
    while(PINB & 0b00001000); //wait for pin 11 (B3) to go low
    OUTPUT_WAVE(emittingPointer, phasePartNum);
    
    if (phasePartNum < RESOLUTION - 1)
      phasePartNum += STEP_SIZE;
    else {
      phasePartNum = 0;
      if(i != 10000){
        i += 1;
      } else {
        i = 0;
        USART_Transmit(phaseNum);
        if (phaseNum < RESOLUTION - 1){
          phaseNum += 1;
        } else phaseNum = 0;
      }
      state = USART_Receive(state);
    }
    
  goto LOOP;
  /*------------------------------------------------------------------------*/
}

void loop(){}

inline byte USART_Receive( byte oldData )
{
  // wait for data in register
  if( (UCSR0A & (1<<RXC0)) )
    // get and return received data
    return UDR0;
  else
    return oldData;
}

inline void USART_Transmit( byte data )
{
  // wait until the register is empty
  while ( !( UCSR0A & (1<<UDRE0)) );
  // send data
  UDR0 = data;
}
