/*****************************************************************************
 RTC example
 *****************************************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include <stdio.h>

# define FOSC 9830400 // Clock frequency = Oscillator freq .
# define BDIV ( FOSC / 100000 - 16) / 2 + 1
#define DS1307_ID    0xD0        // I2C DS1307 Device Identifier
#define DS1307_ADDR  0x00        // I2C DS1307 Device Address
#define I2C_START 0
#define I2C_DATA 1
#define I2C_DATA_ACK 2
#define I2C_STOP 3
#define BAUD_RATE 19200 				 //19200
#define MAXIMUM_ATTEMPTS 50
#define ACK 1
#define NACK 0
#define HOUR_24 0
#define HOUR_12 1
//#define 	TW_READ   1
//#define 	TW_WRITE   0



void rtc_support_init(void) {
	
	char ADDRESS_DS1307[7];
	char HOUR_MODE_24, HOUR_MODE_12;
	TWSR = 0x00;   // Select Prescaler of 1
	// SCL frequency = 11059200 / (16 + 2 * 48 * 1) = 98.743 khz
	TWBR = 0x30;   // 48 Decimal
	TCCR2A=0b10000011;            // Fast PWM MODE, Clear on OCRA
	TCCR2B=0b00000100;            // Used fclk/64 prescaller
	// OCR2A=0xFF;                   // Initial the OC2A (PB3) Out to 0xFF
	// Initial ATMega168 Timer/Counter0 Peripheral
	TCCR0A=0x00;                  // Normal Timer0 Operation
	TCCR0B=(1<<CS02)|(1<<CS00);   // Use maximum prescaller: Clk/1024
	TCNT0=0x94;                   // START counter from 0x94, overflow at 10 mSec
	TIMSK0=(1<<TOIE0);            // Enable Counter Overflow Interrupt
	sei();              
}




/* START I2C Routine */
unsigned char TRANSMIT_I2C(unsigned char type) {
	switch(type) {
		case I2C_START:    // START I2C
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
	STATUS_TWI=TRANSMIT_I2C(I2C_START);
	
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
	STATUS_TWI=TRANSMIT_I2C(I2C_STOP);		// TRANSMIT I2C Data
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
void READ_DS1307(void)
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


void SET_DATE_TIME_DS1307(char day, char mth, char year, char dow, char hr, char min, char sec) 
{ 
	sec &= 0x7F; 
	hr &= 0x3F; 
	
	START_I2C(); 
	WRITE_I2C(0xD0);             // I2C write address 
	WRITE_I2C(0x00);             // Start at REG 0 - Seconds 
	WRITE_I2C(dec2bcd(sec));     // REG 0 
	WRITE_I2C(dec2bcd(min));     // REG 1 
	WRITE_I2C(dec2bcd(hr));      // REG 2 
	WRITE_I2C(dec2bcd(dow));     // REG 3 
	WRITE_I2C(dec2bcd(day));     // REG 4 
	WRITE_I2C(dec2bcd(mth));     // REG 5 
	WRITE_I2C(dec2bcd(year));    // REG 6 
	WRITE_I2C(0x80);             // REG 7 - Disable squarewave output pin 
	STOP_I2C(); 
}


ISR(TIMER0_OVF_vect)
{
	static unsigned char tenms=1;
	tenms++;                  // Read DS1307 every 100 x 10ms = 1 sec
	if (tenms >= 100) {
		cli();                                // Disable Interupt
		READ_DS1307();   
		tenms=1;
		sei();                            // Enable Interrupt
		
	}
	TCNT0=0x94;
}


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
