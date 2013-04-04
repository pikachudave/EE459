//
//  phone_support.c
//  SamsungSparkOS
//
//  Created by Chris Eubank on 4/4/13.
//
//

#include "phone_support.h"
#include "suart_459.h"


int phone_get_sample(int channel_number){
  if (channel_number == 1) {
    return (PINB & 0x04);
  }else{
    return (PINB & 0x04);
  }
}

void phone_support_init(){
  DDRB |= 1 << DDB0;       // Set PORTC bit 0 for output
  
  PORTB |= 1 << PB1;       // Enable pull-up for switch on PORTC bit 1
  PORTB |= 1 << PB2;       // Enable pull-up for switch on PORTC bit 2
  
  suart_puts("Phone Support Intialized\r\n");

}

void phone_support(){

}
