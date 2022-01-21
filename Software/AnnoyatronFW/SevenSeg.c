/*
 * SevenSeg.c
 *
 * Created: 1/14/2022 8:49:43 AM
 *  Author: vtrre
 */ 

#include "main.h"
#include "SevenSeg.h"
#include "I2C.h"

volatile uint8_t display_buffer[5] = {0};

/**
 *	 Use I2C protocol to send init commands to HT16K33 chip in SevenSegment display
 */
void initSevenSeg(void)
{
   uint8_t writeCmdByte = 0;
   uint8_t status = 0;

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

/**
 *	Update one of the indices in the display buffer with a new value
 * to be written to the SevenSeg display next time writeSevenSeg() is called
 */
void setSevenSegValue(uint8_t index, uint8_t value)
{
   if ( index > 5 || value > LEN_NUM_TABLE) return;

   /**
    * display_buffer[2] controls the colon. This take a value 
    * outside the number table, so it gets special logic 
    */
   if ( index == 2)
   {
      display_buffer[index] =  (value) ? 0x02: 0;
   }
   
   display_buffer[index] = numbertable[value];
}

/**
 *	Write all the values currently in the display buffer to the SevenSeg display
 */
void writeSevenSeg(void)
{
   for (int i = 0; i < 5; i++)
   {
      I2C_write_bytes(SEVENSEG_ADDR, &display_buffer[i], i * 2, 1);
   }
}

void sevenSegBlink(bool blinkEnable)
{
   uint8_t writeCmdByte = 0;
   uint8_t status = 0;

   /* Set blink on or off based on provided boolean */
   writeCmdByte = (blinkEnable) ? HT16K33_CMD_DISP_ON_BLINK : HT16K33_CMD_DISP_ON_NOBLINK;
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
}