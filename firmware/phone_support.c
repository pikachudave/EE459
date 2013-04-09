//
//  phone_support.c
//  SamsungSparkOS
//
//  Created by Chris Eubank on 4/4/13.
//
//

#include "phone_support.h"
#include "suart_459.h"
#include <stdio.h>

#define BUFFER_SIZE 10


void ShiftLeftByOne(int *arr, int len);
int SumArrayValues(int *arr, int len);
int phone_get_sample(int channel_number);

void phone_support_init(){
  DDRB |= 1 << DDB0;       // Set PORTC bit 0 for output
  PORTB &= ~(1 << PB0);    // Set PORTB but 0 to 0

  
  PORTB |= 1 << PB1;       // Enable pull-up for switch on PORTB bit 1
  PORTB |= 1 << PB2;       // Enable pull-up for switch on PORTB bit 2
  
  suart_puts("Phone Support Intialized\r\n");

}

void phone_support(){
  static char running_parse = 0;
  
  if(!running_parse && phone_get_sample(1)){
    //Take control of the system
    running_parse = 1;

    //Variable Declarations
    //Input Buffers
    int data_buffer[BUFFER_SIZE] = { 0 };
    int clock_buffer[BUFFER_SIZE] ={ 0 };
    //Key Values
    int clock_cycles = 0;
    int day_number = 0;
    int hour_number = 0;
    int minute_number = 0;
    //Trigger Boolean
    int clock_high = 0;
    
    while(running_parse){
      char flag_up = 0;
      //Sample from Clock & Data Lines
      int data_sample = phone_get_sample(2);
      int clock_sample = phone_get_sample(1);
      
      //Shift Out Old Samples
      ShiftLeftByOne(data_buffer, BUFFER_SIZE);
      ShiftLeftByOne(clock_buffer, BUFFER_SIZE);
      
      //Write New Samples Into Buffer
      data_buffer[BUFFER_SIZE-1] = data_sample;
      clock_buffer[BUFFER_SIZE-1] = clock_sample;
      
      //Check for high transition
      if( !clock_high && (SumArrayValues(clock_buffer, BUFFER_SIZE) > BUFFER_SIZE/5)){
        clock_high = 1;
        clock_cycles++;
        
        if (clock_cycles < 18) {
          PORTB |= 1 << PB0;
          day_number += (SumArrayValues(data_buffer, BUFFER_SIZE) > BUFFER_SIZE/5);
        } else if (clock_cycles < 48){
          PORTB &= ~(1 << PB0);
          hour_number += (SumArrayValues(data_buffer, BUFFER_SIZE) > BUFFER_SIZE/5);
        } else if (clock_cycles < 120){
          PORTB |= 1 << PB0;
          minute_number += (SumArrayValues(data_buffer, BUFFER_SIZE) > BUFFER_SIZE/5);
        } else if (clock_cycles >= 120){
          PORTB &= ~(1 << PB0);
          running_parse = 0;
        }
        
      }else if (clock_high && SumArrayValues(clock_buffer, BUFFER_SIZE) <= BUFFER_SIZE/5){
        clock_high = 0;
      }
      
    }
    
    int n;
    char clock_cycles_str [50];
    char day_number_str [50];
    char hour_number_str [50];
    char minute_number_str [50];
    n=sprintf (clock_cycles_str, "%d",clock_cycles);
    n=sprintf (day_number_str, "%d",day_number);
    n=sprintf (hour_number_str, "%d",hour_number);
    n=sprintf (minute_number_str, "%d",minute_number);

    suart_puts("Clock Cycles:"); suart_puts(clock_cycles_str); suart_puts("\r\n");
    suart_puts("Day Count:"); suart_puts(day_number_str); suart_puts("\r\n");
    suart_puts("Hour Count:"); suart_puts(hour_number_str); suart_puts("\r\n");
    suart_puts("Minute Count:"); suart_puts(minute_number_str); suart_puts("\r\n");

    suart_puts("Parse Complete\r\n");
  
  
  }
}


int phone_get_sample(int channel_number){
  if (channel_number == 1) {
    return (PINB & 0x02);
  }else{
    return (PINB & 0x04);
  }
}

//Shift array values left one space
void ShiftLeftByOne(int *arr, int len)
{
  //printf("Array Value: ");
  int i = 0;
  while (i < len - 1) {
    arr[i] = arr[i+1];
    i++;
  }
}


//Return sum of the array values
int SumArrayValues(int *arr, int len){
  int i = 0;
  int sum = 0;
  while (i < len - 1) {
    sum += arr[i];
    i++;
  }
  return sum;
}