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
    
  }
  
  return 0;   /* never reached */
}