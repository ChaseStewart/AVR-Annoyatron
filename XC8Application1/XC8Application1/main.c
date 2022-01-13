/*
* main.c
*
* Created: 12/11/2021 2:35:04 PM
* Author: Chase E. Stewart for Hidden Layer Design
*/
#include "main.h"
#include "I2C.h"

#include <xc.h>
#include <avr/io.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>


volatile uint8_t display_buffer[5] = {0};

int main(void)
{
   uint8_t count = 0;
   initPeripherals();
   ledUsrBlink(3, 500);

   initSevenSeg();
   
   setSevenSegValue(0, 0);
   setSevenSegValue(1, 0);
   setSevenSegValue(3, 0);
   setSevenSegValue(4, 0);
   writeSevenSeg();


   while(1)
   {
	  if ((PORTA.IN & CUT_WIRES_bm) == CUT_WIRES_bm)
	  {
		  PORTC.OUT = 0;
	  }
	  else
	  {
		PORTC.OUT = PIN2_bm;
	  }
     setSevenSegValue(0, count);
     setSevenSegValue(1, count);
     setSevenSegValue(2, 0x02);
     setSevenSegValue(3, count);
     setSevenSegValue(4, count);
     writeSevenSeg();
     _delay_ms(200);
     count = (count + 1) % 10;

   }
}

/**
 * Setup all peripherals 	
 */
void initPeripherals(void)
{
   CLKCTRL.MCLKCTRLA |= CLKCTRL_CLKOUT_bm | CLKCTRL_CLKSEL_OSC20M_gc;
   CLKCTRL.MCLKCTRLB |= CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm;
   initLED();
   initPIR();
   initCutWires();
   I2C_init();
   initAudio();
}

void initSevenSeg(void)
{
   uint8_t writeCmdByte = 0;
   uint8_t status;

   /* Enable HT16K33 oscillator */
   writeCmdByte = HT16K33_CMD_OSC_ENABLE;
   I2C_start(SEVENSEG_ADDR);
   status = I2C_wait_ACK();
   if (0 != status)
   {
      ledUsrBlink(0, 200);
   }
   
   I2C_write(&writeCmdByte);
   status = I2C_wait_ACK();
   if (0 != status)
   {
      ledUsrBlink(0, 100);
   }
   I2C_stop();
   
   /* Enable HT16K33 display */ 
   writeCmdByte = HT16K33_CMD_DISP_ON_NOBLINK;
   I2C_start(SEVENSEG_ADDR);
   status = I2C_wait_ACK();
   if (0 != status)
   {
      ledUsrBlink(0, 300);
   }

   I2C_write(&writeCmdByte);
   status = I2C_wait_ACK();
   if (0 != status)
   {
      ledUsrBlink(0, 400);
   }

   I2C_stop();

   /* Set dimming level to 16/16 */
   writeCmdByte = HT16K33_CMD_DIM_LEVEL(0x08);
   I2C_start(SEVENSEG_ADDR);
   status = I2C_wait_ACK();
   if (0 != status)
   {
      ledUsrBlink(0, 500);
   }

   I2C_write(&writeCmdByte);
   status = I2C_wait_ACK();
   if (0 != status)
   {
      ledUsrBlink(0, 1000);
   }
   I2C_stop();

   for (int i = 0; i < 8; i++)
   {
      display_buffer[i] = 0xff;
   }
   writeSevenSeg();
}

void writeSevenSeg(void)
{   
   for (int i = 0; i < 5; i++)
   {
     I2C_write_bytes(SEVENSEG_ADDR, &display_buffer[i], i * 2, 1);
   }
}

void setSevenSegValue(uint8_t index, uint8_t value)
{
   if ( index == 2)
   {
       	display_buffer[index] =  (0x02 == value ) ? 0x02: 0;      
   }      
   
 	/*this function sets a single position in the number table*/
 	if ( index > 8 || value > LEN_NUM_TABLE) 
   {
    	return;
 	}
 	display_buffer[index] = numbertable[value];  
}

void initAudio(void)
{
   /*
    *	set Audio ~SHDN pin high
    */
   PORTC.DIRSET = PIN3_bm;
   PORTC.OUT |= PIN3_bm;

   /*
    *	setup PWM
    */
   PORTA.DIRSET = PIN3_bm;
   TCA0.SINGLE.CTRLD &= ~TCA_SPLIT_ENABLE_bm;
   TCA0.SINGLE.PER = 5000;
   TCA0.SINGLE.CMP0 = 2500;
   TCA0.SINGLE.CNT = 0;
   TCA0.SINGLE.CTRLB |= (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_CMP0EN_bm); // we want WO3 so set 0b11
   TCA0.SINGLE.CTRLB |= (TCA_SINGLE_WGMODE0_bm | TCA_SINGLE_WGMODE1_bm); // WGMode 3 == single PWM
   TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV2_gc | TCA_SINGLE_ENABLE_bm); // now enable the module
}

void initPIR(void)
{
   PORTC.DIRCLR = PIN0_bm;
   PORTC.PIN0CTRL &= ~PORT_PULLUPEN_bm;
}

void initCutWires(void)
{
   PORTA.DIRCLR = CUT_WIRES_bm;
   PORTA.PIN4CTRL |= PORT_PULLUPEN_bm;	
   PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
   PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;
   PORTA.PIN7CTRL |= PORT_PULLUPEN_bm;
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
