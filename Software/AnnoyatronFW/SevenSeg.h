/*!
 * @file SevenSeg.h
 *
 * Created: 1/14/2022 8:50:13 AM
 * Author: Chase E. Stewart for Hidden Layer Design
 */ 

#include <stdbool.h>
#include "stdint.h"

#ifndef SEVENSEG_H_
#define SEVENSEG_H_

#define SEVENSEG_TABLE_LEN 19  ///< length of the LED table and enum below

/** Table of HT16K33 LED mappings to render each character */
static const uint8_t numbertable[SEVENSEG_TABLE_LEN] = {
   0x3F, /* 0 */
   0x06, /* 1 */
   0x5B, /* 2 */
   0x4F, /* 3 */
   0x66, /* 4 */
   0x6D, /* 5 */
   0x7D, /* 6 */
   0x07, /* 7 */
   0x7F, /* 8 */
   0x6F, /* 9 */
   0x77, /* a */
   0x7C, /* b */
   0x39, /* C */
   0x5E, /* d */
   0x79, /* E */
   0x71, /* F */
   0x7F, /* all on */
   0x00, /* all off */
   0x40  /* dash */
};

/** Enumeration of the possible seven-seg chars, with 0x0 through 0xf mapped to their decimal values */
typedef enum sevenseg_digit_enum
{
	SEVENSEG_0    = 0,
	SEVENSEG_1    = 1,
	SEVENSEG_2    = 2,
	SEVENSEG_3    = 3,
	SEVENSEG_4    = 4,
	SEVENSEG_5    = 5,
	SEVENSEG_6    = 6,
	SEVENSEG_7    = 7,
	SEVENSEG_8    = 8,
	SEVENSEG_9    = 9,
	SEVENSEG_A    = 10,
	SEVENSEG_B    = 11,
	SEVENSEG_C    = 12,
	SEVENSEG_D    = 13,
	SEVENSEG_E    = 14,
	SEVENSEG_F    = 15,
	SEVENSEG_ALL  = 16,
	SEVENSEG_NONE = 17,
	SEVENSEG_DASH = 18 
	} sevenseg_digit_t;


#define SEVENSEG_ADDR (0x70 << 1)  ///< The I2C address of the HT16K33 device

#define _HT16K33_SYS_SETUP_ADDR 0x20  ///< System setup address to be logical OR'ed with SETUP commands
#define _HT16K33_SYS_SETUP_OSC_ON  0x01  ///< Setup command to turn HT16K33 internal oscillator ON
#define _HT16K33_SYS_SETUP_OSC_OFF 0x00  ///< Setup command to turn HT16K33 internal oscillator OFF

/* Public OSC commands*/
#define HT16K33_CMD_OSC_ENABLE (_HT16K33_SYS_SETUP_ADDR | _HT16K33_SYS_SETUP_OSC_ON)  ///< Convenience definition to turn oscillator ON
#define HT16K33_CMD_OSC_DISABLE (_HT16K33_SYS_SETUP_ADDR | _HT16K33_SYS_SETUP_OSC_OFF)  ///< Convenience definition of command to turn oscillator OFF

#define _HT16K33_DISP_SET_ADDR 0x80  ///< Display setup address to be logical OR'ed with DISP commands
#define _HT16K33_DISP_SET_DISPLAYON 0x01  ///< Display command to turn HT16K33 display ON
#define _HT16K33_DISP_SET_DISPLAYOFF 0x00 ///< Display command to turn HT16K33 display OFF

/** Enumeration of the possible HT16K33 blink values */
typedef enum sevenseg_blink_enum
{
	HT16K33_BLINK_OFF = 0x00,
	HT16K33_BLINK_2HZ = 0x02,
	HT16K33_BLINK_1HZ = 0x04,
	HT16K33_BLINK_HALFHZ = 0x06
} sevenseg_blink_t;

#define _HT16K33_BLINK_MASK  0x06 ///< Bitmask for all possible blink options


/* Public DISP commands */
#define HT16K33_CMD_DISP_OFF (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYOFF | HT16K33_BLINK_OFF)  ///< Convenience definition to turn display off
#define HT16K33_CMD_DISP_ON_NOBLINK (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYON | HT16K33_BLINK_OFF)  ///< Convenience definition to turn display on without blinking
#define HT16K33_CMD_DISP_ON_BLINK (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYON | HT16K33_BLINK_2HZ)  ///< Convenience definition to turn display on blinking at 2HZ

#define _HT16K33_DIM_ADDR 0xE0 ///< Dimming setup address to be logical OR'ed with DIM commands

/** Set dimming level using provided nybble (0x0F is brightest, 0x00 is dimmest) */
#define HT16K33_CMD_DIM_LEVEL(dimNibble) (_HT16K33_DIM_ADDR + (dimNibble & 0x0F)) 

extern volatile uint8_t display_buffer[5]; 
void initSevenSeg(void);
void setSevenSegValue(uint8_t index, sevenseg_digit_t value);
void writeSevenSeg(void);
void sevenSegBlink(sevenseg_blink_t blinkSpeed);
void writeAllDigits(sevenseg_digit_t value);


#endif /* SEVENSEG_H_ */
