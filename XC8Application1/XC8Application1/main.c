/*
* main.c
*
* Created: 12/11/2021 2:35:04 PM
* Author: Chase E. Stewart for Hidden Layer Design
*/
#include "main.h"

#include <xc.h>
#include <avr/io.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include "I2C.h"


volatile uint16_t display_buffer[8];


int main(void)
{
   initPeripherals();

   ledUsrBlink(5, 500);
   _delay_ms(1000);

   initSevenSeg();
   
   setSevenSegValue(0, 0);
   setSevenSegValue(1, 1);
   setSevenSegValue(3, 2);
   setSevenSegValue(4, 3);
   writeSevenSeg();

   while(1)
   {
      /* set USR_LED IFF PIR sensor is triggered */
      PORTC.OUT = (PORTC.IN & PIN0_bm) ? PIN2_bm : 0;
   }
}

/**
 * Setup all peripherals 	
 */
void initPeripherals(void)
{
   initLED();
   initPIR();
   I2C_init();
}

void initSevenSeg(void)
{
   uint8_t writeCmdByte = 0;
   uint8_t readBuf[10] = {0};
   uint8_t status;

   /* Clear keylines by reading them */
   //I2C_read_bytes(SEVENSEG_ADDR, readBuf, HT16K33_KEYDATA0_ADDR, HT16K33_KEYDATA_LEN);

   /* Enable HT16K33 oscillator */
   writeCmdByte = 0x21; //HT16K33_CMD_OSC_ENABLE;
   I2C_start(SEVENSEG_ADDR);
   status =    I2C_wait_ACK();
   if (1 == status)
   {
      ledUsrBlink(0, 200);
   }
   I2C_write(&writeCmdByte);
   I2C_stop();
   
   /* Enable HT16K33 display */ 
   writeCmdByte = 0x81; //HT16K33_CMD_DISP_ON_NOBLINK;
   I2C_start(SEVENSEG_ADDR);
   I2C_write(&writeCmdByte);
   I2C_stop();

   /* Set dimming level to 16/16 */
   writeCmdByte = 0xEF; //HT16K33_CMD_DIM_LEVEL(0x0F);
   I2C_start(SEVENSEG_ADDR);
   I2C_write(&writeCmdByte);
   I2C_stop();

   for (int i=0; i<8; i++)
   {
      display_buffer[i] = 0;
   }
   writeSevenSeg();
}

void writeSevenSeg(void)
{
   uint8_t convert_array[16];
   
   for (int i = 0; i < 8; i++) 
   {
      convert_array[i << 1] = display_buffer[i] & 0xFF;
      convert_array[(i << 1) + 1] = display_buffer[i] >> 8;
   }

   I2C_write_bytes(SEVENSEG_ADDR, convert_array, 0x00, 16);
}

void setSevenSegValue(uint8_t index, uint8_t value)
{
 	/*this function sets a single position in the number table*/
 	if (index > 4 || value > 17) 
   {
    	return;
 	}
 	display_buffer[index] = numbertable[value];  
}

void initPIR(void)
{
   PORTC.DIRCLR = PIN0_bm;
   PORTC.PIN0CTRL &= ~PORT_PULLUPEN_bm;
}

/** 
 * Set up the user LED
 */
void initLED(void)
{
   PORTC.DIR |= PIN2_bm;
}

/**
 *	Blink usr LED 'count'-many times at a period of 2 * 'blinkPeriodMsec'
 */
void ledUsrBlink(uint8_t count, const int blinkPeriodMsec)
{
   uint8_t iter=0;

   while (!count || iter < count)
   {
       PORTC.OUT |= PIN2_bm;
       _delay_ms(blinkPeriodMsec);
       PORTC.OUT &= ~PIN2_bm;
       _delay_ms(blinkPeriodMsec);
      iter++;
   }

   // clear LED after blinking is done
   PORTC.OUT &= ~PIN2_bm;
}  
