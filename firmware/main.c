/*************************************************************
 *       Comparator Test - Demonstrate Use of Comparator to Read Data from Headphone Port with ATmega168
 *
 *       Port C, bit 2 - input from comparator channel 2
 *       Port C, bit 1 - input from comparator channel 1
 *       Port C, bit 0 - output to scope
 *
 *       When the switch is pressed, the LED comes on
 *
 * Revision History
 * Date     Author      Description
 *          A. Weber    Initial Release
 * 09/05/05 A. Weber    Modified for JL8 processor
 * 01/13/06 A. Weber    Modified for CodeWarrior 5.0
 * 08/25/06 A. Weber    Modified for JL16 processor
 * 05/08/07 A. Weber    Some editing changes for clarification
 * 04/22/08 A. Weber    Added "one" variable to make warning go away
 * 04/02/11 A. Weber    Adapted for ATmega168
 * 03/28/13 C. Eubank   Changed for use as comparator test
 *************************************************************/

#include <avr/io.h>

//Console Log Support
#include "suart_459.h"
#include "suart_459.c"

//Support for Phone-Based Programming
#include "phone_support.h"
#include "phone_support.c"

//Support for Sending/Recieving with XBEE
#include "xbee_support.h"
#include "xbee_support.c"

//Support for Polling RTC
#include "rtc_support.h"
#include "rtc_support.c"

int main(void)
{
  //Initialize the Software UART
  suart_init();
  
  //Initialize Phone Support Init
  phone_support_init();

  while(1){
    
    phone_support();
    /* DDRA = 0;                    // Set PORTA as Input
  PORTA = 0;

  // Initial ATMega168 UART Peripheral
  suart_init();
  // Initial ATMega168 TWI/I2C Peripheral
  TWSR = 0x00;   // Select Prescaler of 1
  // SCL frequency = 11059200 / (16 + 2 * 48 * 1) = 98.743 khz
  TWBR = 0x30;   // 48 Decimal

  TCCR2A=0b10000011;            // Fast PWM MODE, Clear on OCRA
  TCCR2B=0b00000100;            // Used fclk/64 prescaller
 // Initial ATMega168 PWM using Timer/Counter2 Peripheral

 // Initial ATMega168 Timer/Counter0 Peripheral
  TCCR0A=0x00;                  // Normal Timer0 Operation
  TCCR0B=(1<<CS02)|(1<<CS00);   // Use maximum prescaller: Clk/1024
  TCNT0=0x94;                   // START counter from 0x94, overflow at 10 mSec
  TIMSK0=(1<<TOIE0);            // Enable Counter Overflow Interrupt
  sei();                        // Enable Interrupt
   */
    
  }
  
  return 0;   /* never reached */
}
