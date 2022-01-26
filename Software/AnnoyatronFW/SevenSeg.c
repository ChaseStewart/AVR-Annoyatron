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

static void SetSevenSegConfig(int configValue);


/**
 *	 Use I2C protocol to send init commands to HT16K33 chip in SevenSegment display
 */
void initSevenSeg(void)
{
   /* Enable HT16K33 oscillator */
   SetSevenSegConfig(HT16K33_CMD_OSC_ENABLE);

   /* Enable HT16K33 display */
   SetSevenSegConfig(HT16K33_CMD_DISP_ON_NOBLINK);

   /* Set dimming level to 16/16 */
   SetSevenSegConfig(HT16K33_CMD_DIM_LEVEL(0x08));

   /* display_buffer is initialized to zero */
   writeSevenSeg();
}

/**
 *	Update one of the indices in the display buffer with a new value
 * to be written to the SevenSeg display next time writeSevenSeg() is called
 */
void setSevenSegValue(uint8_t index, uint8_t value)
{
   if ( index > 5 || value > SEVENSEG_TABLE_LEN) return;

   /**
    * display_buffer[2] controls the colon. 
    * This will set colon unless value is one of {0, SEVENSEG_NONE} 
    */
   if ( index == 2)
   {
      display_buffer[index] =  (SEVENSEG_NONE) ? 0: (value) ? 0x02: 0;
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

void setAllDigits(uint8_t value)
{
   if (value >= SEVENSEG_TABLE_LEN) return;
   
   setSevenSegValue(0, value);
   setSevenSegValue(1, value);
   setSevenSegValue(2, value);
   setSevenSegValue(3, value);
   setSevenSegValue(4, value);
   writeSevenSeg();
   
}

static void SetSevenSegConfig(int configValue)
{
   uint8_t writeCmdByte = configValue;
   uint8_t status; 
   
   I2C_start(SEVENSEG_ADDR);
   status = I2C_wait_ACK();
   if (0 != status) ledUsrBlink(0, 200);
   
   I2C_write(&writeCmdByte);
   status = I2C_wait_ACK();
   if (0 != status) ledUsrBlink(0, 100);
   I2C_stop();
}
