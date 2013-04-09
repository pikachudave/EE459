//
//  rtc_support.c
//  SamsungSparkOS
//
//  Created by Chris Eubank on 4/4/13.
//
//

#include "rtc_support.h"
#include "suart_459.h"

/*****************************************************************************
 RTC example
 *****************************************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define IDENTIFIER_DS1307    0xD0        // I2C DS1307 Device Identifier
#define ADDRESS_DS1307 		 0x00        // I2C DS1307 Device Address
#define BAUD_RATE 19200					 //19200
#define MAXIMUM_ATTEMPTS 50
#define START_I2C 0
#define I2C_DATA 1
#define I2C_DATA_ACK 2
#define STOP_I2C 3
#define ACK 1
#define NACK 0
#define HOUR_24 0
#define HOUR_12 1

// Pin 27 SDA
// Pin 28 SCL
// DS1307 Register Address
// Second: ADDRESS_DS1307[0]
// Minute: ADDRESS_DS1307[1]
// Hour  : ADDRESS_DS1307[2]
// Day   : ADDRESS_DS1307[3]
// Date  : ADDRESS_DS1307[4]
// MONTH : ADDRESS_DS1307[5]
// Year  : ADDRESS_DS1307[6]
//char *DAY[]={"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
//char *MONTH[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

char ADDRESS_DS1307[7];
char SETDIGIT[3]={'0','0','\0'};
//char *DAY[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
//char *MONTH[]={"January","Febuary","March","April","May","June","July","August","September","October","November","December"};
char HOUR_MODE_24, HOUR_MODE_12;

void uart_init(void)
//void uart_init(void)
{
  UBRR0H = (((F_CPU/BAUD_RATE)/16)-1)>>8;	// set baud rate
  UBRR0L = (((F_CPU/BAUD_RATE)/16)-1);
  UCSR0B = (1<<RXEN0)|(1<<TXEN0); 			    // enable Rx & Tx
  UCSR0C=  (1<<UCSZ01)|(1<<UCSZ00);  	        // config USART; 8N1
}
//int uart_puts(char ch,FILE *stream)
int uart_putch(char ch,FILE *stream)
{
  if (ch == '\n')
    uart_putch('\r', stream);
  while (!(UCSR0A & (1<<UDRE0)));
  UDR0=ch;
  return 0;
}
int uart_getchar(FILE *stream)
{
  unsigned char ch;
  while (!(UCSR0A & (1<<RXC0)));
  ch=UDR0;
  uart_putch(ch,stream);    	// Output to terminal
  
  return ch;
}


void ansi_cl(void)
{
  // ANSI clear screen: cl=\E[H\E[J
  putchar(27);
  putchar('[');
  putchar('H');
  putchar(27);
  putchar('[');
  putchar('J');
}
void ansi_me(void)
{
  // ANSI turn off all attribute: me=\E[0m
  putchar(27);
  putchar('[');
  putchar('0');
  putchar('m');
}
void ansi_cm(unsigned char row,unsigned char col)
{
  // ANSI cursor movement: cl=\E%row;%colH
  putchar(27);
  putchar('[');
  printf("%d",row);
  putchar(';');
  printf("%d",col);
  putchar('H');
}

/* START I2C Routine */
unsigned char TRANSMIT_I2C(unsigned char type) {
  switch(type) {
    case START_I2C:    // START I2C
      TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
      break;
    case I2C_DATA:     // No Acknowledge w/ Data
      TWCR = (1 << TWINT) | (1 << TWEN);
      break;
    case I2C_DATA_ACK: // Acknowledge w/ Data
      TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
      break;
    case STOP_I2C:     // Stop I2C
      TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
      return 0;
  }
  while (!(TWCR & (1 << TWINT)));	 // Wait for TWINT flag set on Register TWCR
  return (TWSR & 0xF8);  			 // TWI Status Register w/ masked bits
}
char START_I2C(unsigned int dev_id, unsigned int dev_addr, unsigned char rw_type)
{
  unsigned char n = 0;
  unsigned char STATUS_TWI;
  char r_val = -1;
RETRY_I2C:
  if (n++ >= MAXIMUM_ATTEMPTS) return r_val;
  STATUS_TWI=TRANSMIT_I2C(START_I2C);
  
  // Check the TWI Status
  if (STATUS_TWI == TW_MT_ARB_LOST) goto RETRY_I2C;
  if ((STATUS_TWI != TW_START) && (STATUS_TWI != TW_REP_START)) goto QUIT_I2C;
  // Send slave address (SLA_W)
  TWDR = (dev_id & 0xF0) | (dev_addr & 0x07) | rw_type;
  // TRANSMIT I2C Data
  STATUS_TWI=TRANSMIT_I2C(I2C_DATA);
  // Check the TWSR status
  if ((STATUS_TWI == TW_MT_SLA_NACK) || (STATUS_TWI == TW_MT_ARB_LOST)) goto RETRY_I2C;
  if (STATUS_TWI != TW_MT_SLA_ACK) goto QUIT_I2C;
  r_val=0;
QUIT_I2C:
  return r_val;
}
void STOP_I2C(void)
{
  unsigned char STATUS_TWI;
  STATUS_TWI=TRANSMIT_I2C(STOP_I2C);		// TRANSMIT I2C Data
}
char WRITE_I2C(char data)
{
  unsigned char STATUS_TWI;
  char r_val = -1;
  // Send the Data to I2C Bus
  TWDR = data;
  // TRANSMIT I2C Data
  STATUS_TWI=TRANSMIT_I2C(I2C_DATA);
  // Check the TWSR status
  if (STATUS_TWI != TW_MT_DATA_ACK) goto QUIT_I2C;
  r_val=0;
QUIT_I2C:
  return r_val;
}
char READ_I2C(char *data,char ack_type)
{
  unsigned char STATUS_TWI;
  char r_val = -1;
  
  if (ack_type) {
    STATUS_TWI=TRANSMIT_I2C(I2C_DATA_ACK);   // Read I2C Data and Send Acknowledge
    if (STATUS_TWI != TW_MR_DATA_ACK) goto QUIT_I2C;
  } else {
    STATUS_TWI=TRANSMIT_I2C(I2C_DATA);		 // Read I2C Data and Send No Acknowledge
    if (STATUS_TWI != TW_MR_DATA_NACK) goto QUIT_I2C;
  }
  
  *data=TWDR;			 // Aquire Data
  r_val=0;
QUIT_I2C:
  return r_val;
}
Convert Decimal to Binary Coded Decimal (BCD)
char dec2bcd(char num)
{
  return ((num/10 * 16) + (num % 10));
}
// Convert Binary Coded Decimal (BCD) to Decimal
char bcd2dec(char num)
{
  return ((num/16 * 10) + (num % 16));
}
void Read_DS1307(void)
{
  char data;
  // Initialize pointer register to address 0x00 & start I2C Write
  START_I2C(IDENTIFIER_DS1307,ADDRESS_DS1307,TW_WRITE);
  WRITE_I2C(0x00);		 // Start from Address 0x00
  STOP_I2C();
  START_I2C(IDENTIFIER_DS1307,ADDRESS_DS1307,TW_READ);
  
  // Read appropriate register and send master acknowledge
  READ_I2C(&data,ACK);		// Read Seconds Register
  ADDRESS_DS1307[0]=bcd2dec(data & 0x7F);
  READ_I2C(&data,ACK);		 // Read Minutes Register
  ADDRESS_DS1307[1]=bcd2dec(data);
  READ_I2C(&data,ACK);	  // Read the Hour Register
  if ((data & 0x40) == 0x40) {
    HOUR_MODE_24 = HOUR_12;
    HOUR_MODE_12=(data & 0x20) >> 5;   // HOUR_MODE_12: 0-AM, 1-PM
    ADDRESS_DS1307[2]=bcd2dec(data & 0x1F);
  } else {
    HOUR_MODE_24 = HOUR_24;
    HOUR_MODE_12=0;
    ADDRESS_DS1307[2]=bcd2dec(data & 0x3F);
  }
  READ_I2C(&data,ACK);		// Day register w/ ack
  ADDRESS_DS1307[3]=bcd2dec(data);
  READ_I2C(&data,ACK); 	// Date register w/ ack
  ADDRESS_DS1307[4]=bcd2dec(data);
  READ_I2C(&data,ACK);		// Month register w/ ack
  ADDRESS_DS1307[5]=bcd2dec(data);
  READ_I2C(&data,NACK);	// Year register w/o ack
  ADDRESS_DS1307[6]=bcd2dec(data);
  STOP_I2C();
}
void SET_DATE_DS1307(void)
{
  unsigned char i, hour_format;
  // Make sure we enable the Oscillator control bit CH=0 on Register 0x00
  ADDRESS_DS1307[0]=ADDRESS_DS1307[0] & 0x7F;
  
  START_I2C(IDENTIFIER_DS1307,ADDRESS_DS1307,TW_WRITE);
  WRITE_I2C(0x00);
  for (i=0; i<7; i++) {
    if (i == 2) {
      hour_format=dec2bcd(ADDRESS_DS1307[i]);
      if (HOUR_MODE_24) {
        hour_format |= (1 << 6);
        if (HOUR_MODE_12)
          hour_format |= (1 << 5);
        else
          hour_format &= ~(1 << 5);
      } else {
        hour_format &= ~(1 << 6);
      }
      WRITE_I2C(hour_format);
    } else {
      WRITE_I2C(dec2bcd(ADDRESS_DS1307[i]));
    }
  }
  STOP_I2C();
}

/* void ds1307_set_date_time(BYTE day, BYTE mth, BYTE year, BYTE dow, BYTE hr, BYTE min, BYTE sec)
 {
 sec &= 0x7F;
 hr &= 0x3F;
 
 i2c_start();
 i2c_write(0xD0);            // I2C write address
 i2c_write(0x00);            // Start at REG 0 - Seconds
 i2c_write(bin2bcd(sec));      // REG 0
 i2c_write(bin2bcd(min));      // REG 1
 i2c_write(bin2bcd(hr));      // REG 2
 i2c_write(bin2bcd(dow));      // REG 3
 i2c_write(bin2bcd(day));      // REG 4
 i2c_write(bin2bcd(mth));      // REG 5
 i2c_write(bin2bcd(year));      // REG 6
 i2c_write(0x80);            // REG 7 - Disable squarewave output pin
 i2c_stop();
 }  */




char *num2str(char number)
{
  unsigned char digit;
  
  digit = '0';                       // START with ASCII '0'
  while(number >= 10)                // Keep Looping for larger than 10
  {
    digit++;                         // Increase ASCII character
    number -= 10;                    // Subtract number with 10
  }
  
  SETDIGIT[0]='0';                     // Default first Digit to '0'
  if (digit != '0')
    SETDIGIT[0]=digit;                 // Put the Second digit
  SETDIGIT[1]='0' + number;
  return SETDIGIT;
}
char getnumber(unsigned char min, unsigned char max)
{
  int inumber;
  scanf("%d",&inumber);
  if (inumber < min || inumber > max) {
    printf("\n\nInvalid [%d to %d]!",min,max);
    _delay_ms(500);
    return -1;
  }
  return inumber;
}
ISR(TIMER0_OVF_vect)
{
  static unsigned char tenms=1;
  //int iTemp;
  tenms++;                  // Read DS1307 every 100 x 10ms = 1 sec
  if (tenms >= 100) {
    cli();                                // Disable Interupt
    Read_DS1307();
    
    
    
    
    
    /* Display the Clock
     LCD_putcmd(LCD_HOME,LCD_2CYCLE);      // LCD Home
     LCD_puts(DAY[ADDRESS_DS1307[3] - 1]); LCD_puts(", ");
     LCD_puts(num2str(ADDRESS_DS1307[4])); LCD_puts(" ");
     LCD_puts(MONTH[ADDRESS_DS1307[5] - 1]); LCD_puts(" ");
     LCD_puts("20"); LCD_puts(num2str(ADDRESS_DS1307[6]));
     LCD_putcmd(LCD_NEXT_LINE,LCD_2CYCLE); // Goto Second Line
     */
    
    
    
    
    
    
    
    /* Display the Clock change from lcd to uart
     LCD_putcmd(LCD_HOME,LCD_2CYCLE);      // LCD Home
     (DAY[ADDRESS_DS1307[3] - 1]); LCD_puts(", ");
     LCD_puts(num2str(ADDRESS_DS1307[4])); LCD_puts(" ");
     LCD_puts(MONTH[ADDRESS_DS1307[5] - 1]); LCD_puts(" ");
     LCD_puts("20"); LCD_puts(num2str(ADDRESS_DS1307[6]));
     LCD_putcmd(LCD_NEXT_LINE,LCD_2CYCLE); // Goto Second Line
     
     
     
     
     
     
     
     if (HOUR_MODE_24) {
     LCD_puts(num2str(ADDRESS_DS1307[2])); LCD_puts(":");
     LCD_puts(num2str(ADDRESS_DS1307[1])); LCD_puts(":");
     LCD_puts(num2str(ADDRESS_DS1307[0]));
     
     if (HOUR_MODE_12)
     LCD_puts(" PM");
     else
     LCD_puts(" AM");
     } else {
     LCD_puts(num2str(ADDRESS_DS1307[2])); LCD_puts(":");
     LCD_puts(num2str(ADDRESS_DS1307[1])); LCD_puts(":");
     LCD_puts(num2str(ADDRESS_DS1307[0])); LCD_puts("   ");
     }
     */
    // Set ADMUX Channel for LM35DZ Input
    /////////////////////////////////////////////////////ADMUX=0x01;
    // START conversion by setting ADSC on ADCSRA Register
    ///////////////////////////////////////////////////////////ADCSRA |= (1<<ADSC);
    // wait until convertion complete ADSC=0 -> Complete
    ////////////////////////////////////////////////while (ADCSRA & (1<<ADSC));
    // Get the ADC Result
    //iTemp = ADCW;
    // ADC = (Vin x 1024) / Vref, Vref = 1 Volt, LM35DZ Out = 10mv/C
    //iTemp = (int)(iTemp) / 10.24;
    
    /*
     // Dislay Temperature
     LCD_puts(" ");
     LCD_puts(num2str((char)iTemp));   // Display Temperature
     LCD_putch(0xDF);                  // Degree Character
     LCD_putch('C');                   // Centigrade
     */
    tenms=1;
    sei();                            // Enable Interrupt
    
  }
  TCNT0=0x94;
}
// Assign I/O stream to UART
FILE uart_str = FDEV_SETUP_STREAM(uart_putch, uart_getchar, _FDEV_SETUP_RW);
int main(void)
{
  char MODE,SELECTION;
  int icount;
  // Initial PORT Used
  // DDRB = 0xFE;                 // Set PB0=Input, Others Output
  //PORTB = 0;
  DDRA = 0;                    // Set PORTA as Input
  PORTA = 0;
  DDRD = 0xFE;                // Set PD0=Input, Others Output   // original Set PORTD as Output
  PORTD = 0;
  
  // Define Output/Input Stream
  stdout = stdin = &uart_str;
  
  
  /* Initial LCD using 4 bits data interface
   initlcd();
   LCD_putcmd(0x0C,LCD_2CYCLE);      // Display On, Cursor Off
   LCD_putcmd(LCD_CLEAR,LCD_2CYCLE); // Clear LCD*/
  
  
  
  // Initial ATMega168 UART Peripheral
  suart_init();
  // Initial ATMega168 TWI/I2C Peripheral
  TWSR = 0x00;   // Select Prescaler of 1
                 // SCL frequency = 11059200 / (16 + 2 * 48 * 1) = 98.743 khz
  TWBR = 0x30;   // 48 Decimal
                 // Initial ATMega168 ADC Peripheral
                 //ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
                 // Free running ADC MODE
                 // ADCSRB = 0x00;
                 // Disable digital input on ADC0 and ADC1
                 // DIDR0 = 0x03;
                 // Initial ATMega168 PWM using Timer/Counter2 Peripheral
  TCCR2A=0b10000011;            // Fast PWM MODE, Clear on OCRA
  TCCR2B=0b00000100;            // Used fclk/64 prescaller
                                // OCR2A=0xFF;                   // Initial the OC2A (PB3) Out to 0xFF
                                // Initial ATMega168 Timer/Counter0 Peripheral
  TCCR0A=0x00;                  // Normal Timer0 Operation
  TCCR0B=(1<<CS02)|(1<<CS00);   // Use maximum prescaller: Clk/1024
  TCNT0=0x94;                   // START counter from 0x94, overflow at 10 mSec
  TIMSK0=(1<<TOIE0);            // Enable Counter Overflow Interrupt
  sei();                        // Enable Interrupt
                                // Initial MODE
  MODE=0;
  while(1)					//for(;;)
  {
    // Check if Button is pressed than enter to the Setup MODE
    if (bit_is_clear(PIND, PD0)) {          // if button is pressed
      _delay_us(100);                       // Wait for debouching
      if (bit_is_clear(PIND, PD0)) {        // if button is pressed
        MODE = 1;
        
        cli();  // Disable Interrupt
        /* LCD_putcmd(LCD_CLEAR,LCD_2CYCLE);   // Clear LCD
         LCD_puts("Setup MODE");
         // Dimmming the LCD
         for (icount=255;icount > 0;icount--) {
         OCR2A=icount;
         _delay_ms(3);
         }	*/
      }
    }
    if (MODE) {
      ////ds1307_set_date_time(day, mth, year, dow, hr, min, sec);
      ansi_me();
      ansi_cl();                            // Clear Screen
      ansi_cm(1,1);
      ansi_cm(3,1);
      printf("1) Time: %02d:%02d:%02d\n",ADDRESS_DS1307[2],ADDRESS_DS1307[1],ADDRESS_DS1307[0]);
      printf("2) MODE 24/12: %d, AM/PM: %d\n",HOUR_MODE_24,HOUR_MODE_12);
      printf("3) Date: %02d-%02d-20%02d, Week Day: %d\n",ADDRESS_DS1307[4],ADDRESS_DS1307[5],ADDRESS_DS1307[6],ADDRESS_DS1307[3]);
      printf("4) Save and Exit\n");
      printf("5) Exit\n");
      printf("\n\nEnter Choice: ");
      if ((SELECTION=getnumber(1,5)) < 0) continue;
      switch (SELECTION) {
        case 1:  // DS1307 Time Setup
          printf("\n\nHour [0-24]: ");
          if ((ADDRESS_DS1307[2]=getnumber(0,24)) < 0) continue;
          printf("\nMinute [0-59]: ");
          if ((ADDRESS_DS1307[1]=getnumber(0,59)) < 0) continue;
          printf("\nSecond [0-59]: ");
          if ((ADDRESS_DS1307[0]=getnumber(0,59)) < 0) continue;
          break;
        case 2:  // DS1307 Hour MODE Setup
          printf("\n\nMODE 0> 24, 1> 12: ");
          if ((HOUR_MODE_24=getnumber(0,1)) < 0) continue;
          printf("\nAM/PM 0> AM, 1> PM: ");
          if ((HOUR_MODE_12=getnumber(0,1)) < 0) continue;
          break;
        case 3:  // DS1307 Date Setup
          printf("\n\nDay [1-7]: ");
          if ((ADDRESS_DS1307[3]=getnumber(1,7)) < 0) continue;
          printf("\nDate [1-31]: ");
          if ((ADDRESS_DS1307[4]=getnumber(1,31)) < 0) continue;			  ;
          printf("\nMonth [1-12]: ");
          if ((ADDRESS_DS1307[5]=getnumber(1,12)) < 0) continue;
          printf("\nYear [0-99]: ");
          if ((ADDRESS_DS1307[6]=getnumber(0,99)) < 0) continue;
          break;
        case 4:  
          SET_DATE_DS1307();	// Save to DS1307 Register & Exit Setup
        case 5:  // Exit Setup
          MODE = 0;
          //ansi_cl();
          /* Illuminating the LCD
           for (icount=0;icount < 255;icount++) {
           OCR2A=icount;
           _delay_ms(3);
           }
           */
          TCNT0=0x94;
          sei();   // Enable Interrupt
          break;
      }
    }
  }
  return 0;
}