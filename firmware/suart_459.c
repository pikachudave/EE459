//
//  suart_459.c
//  TestComparatorIO
//
//  Created by Chris Eubank on 4/4/13.
//
//

#include "suart_459.h"
#include <util/delay.h>

static double suart_bit_length = .1042;

void set_tx_pin(int bit_value){
  if(bit_value){
    PORTB |= 1 << PB5;
  }else{
    PORTB &= ~(1 << PB5);
  }
}

void suart_init(){
  //Uses PORTB bit 5 as our TX pin
  DDRB |= 1 << DDB5;  // Set PORTB bit 5 for output
  
  
  PORTB |= 1 << PB5;  //Initialize PORTB bit 5 to be high
  _delay_ms(1000);
  
  //Clear Screen
  suart_puts("\E[H\E[J");
  suart_puts("SUART Initializedr\n");

  
}

void suart_putchar(char c){
  int bit_counter = 0;
  
  //Set Start Bit
  set_tx_pin(0);
  _delay_ms(suart_bit_length);
  
  //Send Char
  while (bit_counter < 8) {
    set_tx_pin(c&0x01);
    _delay_ms(suart_bit_length);
    c = c >> 1;
    bit_counter++;
  }

  
  //Set Stop Bit
  set_tx_pin(1);
  _delay_ms(suart_bit_length);
  

}

void suart_puts( const char *s )
{
	while ( *s ) {
		suart_putchar( *s++ );
	}
}