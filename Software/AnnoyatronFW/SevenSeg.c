/*!
 * @file SevenSeg.c
 *
 * @mainpage Helper functions for HT16K33 I2C interface
 *
 * @section intro_sec Introduction
 *  This project uses the <a href="https://www.adafruit.com/product/1002">
 * Adafruit 7-Segment display with I2C Backpack</a> as the visual interface
 *  for the countdown timer. Thus, this file and the associated header file
 *  each govern the interactions with the display
 *
 * @section author Author
 *
 * Chase E. Stewart for Hidden Layer Design
 * 
 */ 

#include "main.h"
#include "SevenSeg.h"
#include "I2C.h"

volatile uint8_t display_buffer[5] = {0};   ///< buffer for the four characters and the optional colon

static void SetSevenSegConfig(int configValue);


/*!
 * @brief Use I2C protocol to send init commands to HT16K33 chip in SevenSegment display
 *
 * @return None
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
   writeAllDigits(SEVENSEG_NONE);
   writeSevenSeg();
}

/*!
 * @brief Update one of the indices in the display buffer with a new value
 * to be written to the SevenSeg display next time writeSevenSeg() is called
 *
 * @param index
 *  Index of the display char to update; 0-1 are the chars before the colon,
 *  2 is the colon itself,
 *  3-4 are the chars after the colon
 *
 * @param value
 *  The value to set the provided index to- values are 0-9 or SEVENSEG_ALL or SEVENSEG_NONE
 *
 * @return None
 */
void setSevenSegValue(uint8_t index, sevenseg_digit_t value)
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

/*!
 * @brief Write all the values currently in the display buffer to the SevenSeg display.
 *
 * @return None
 */
void writeSevenSeg(void)
{
   for (int i = 0; i < 5; i++)
   {
      I2C_write_bytes(SEVENSEG_ADDR, &display_buffer[i], i * 2, 1);
   }
}


/*!
 * @brief Set the display to blink at one of the rates provided 
 *
 * @param blinkSpeed
 *  An enum value to be written to the HT16K33 Blink setting 
 *
 * @return None
 */
void sevenSegBlink(uint8_t blinkSpeed)
{
   uint8_t writeCmdByte = 0;
   uint8_t status = 0;

   /* Set blink on or off based on provided boolean */
   writeCmdByte = ( _HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYON | (blinkSpeed & _HT16K33_BLINK_MASK));
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


/*!
 * @brief Set the display to blink at one of the rates provided 
 *
 * @param value
 *  An enum value to be written to all digit places
 *
 * @return None
 */
void writeAllDigits(uint8_t value)
{
   if (value >= SEVENSEG_TABLE_LEN) return;
   
   setSevenSegValue(0, value);
   setSevenSegValue(1, value);
   setSevenSegValue(2, value);
   setSevenSegValue(3, value);
   setSevenSegValue(4, value);
   writeSevenSeg();
   
}

/*!
 * @brief Set a single config value in the HT16K33 
 *
 * @param configValue
 *  A single configuration value to set on HT16K33- see documentation for examples
 *
 * @return None
 */
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
