//
//  phone_support.c
//  SamsungSparkOS
//
//  Created by Chris Eubank on 4/4/13.
//
//

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
}
void phone_support(){

  while (1) {
    
    if (PINB & 0x04){
      PORTB |= 1 << PB0;
    }
    else{
      PORTB &= ~(1 << PB0);
    }
  }
}
