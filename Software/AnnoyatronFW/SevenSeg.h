/*
 * SevenSeg.h
 *
 * Created: 1/14/2022 8:50:13 AM
 * Author: Chase E. Stewart for Hidden Layer Design
 */ 

#include <stdbool.h>
#include "stdint.h"

#ifndef SEVENSEG_H_
#define SEVENSEG_H_

#define SEVENSEG_TABLE_LEN 18
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
   0xFF, /* all on */
   0x00 /* all off */
};

#define SEVENSEG_ALL  SEVENSEG_TABLE_LEN - 2
#define SEVENSEG_NONE SEVENSEG_TABLE_LEN - 1

#define SEVENSEG_ADDR (0x70 << 1)

#define _HT16K33_SYS_SETUP_ADDR 0x20
#define _HT16K33_SYS_SETUP_OSC_ON  0x01
#define _HT16K33_SYS_SETUP_OSC_OFF 0x00

/* Public OSC commands*/
#define HT16K33_CMD_OSC_ENABLE (_HT16K33_SYS_SETUP_ADDR | _HT16K33_SYS_SETUP_OSC_ON)
#define HT16K33_CMD_OSC_DISABLE (_HT16K33_SYS_SETUP_ADDR | _HT16K33_SYS_SETUP_OSC_OFF)

#define _HT16K33_DISP_SET_ADDR 0x80
#define _HT16K33_DISP_SET_DISPLAYON 0x01
#define _HT16K33_DISP_SET_DISPLAYOFF 0x00
#define _HT16K33_BLINK_MASK  0x06
#define HT16K33_BLINK_OFF    0x00
#define HT16K33_BLINK_2HZ    0x02
#define HT16K33_BLINK_1HZ    0x04
#define HT16K33_BLINK_HALFHZ 0x06

/* Public DISP commands */
#define HT16K33_CMD_DISP_OFF (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYOFF | HT16K33_BLINK_OFF)
#define HT16K33_CMD_DISP_ON_NOBLINK (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYON | HT16K33_BLINK_OFF)
#define HT16K33_CMD_DISP_ON_BLINK (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYON | HT16K33_BLINK_2HZ)

#define _HT16K33_DIM_ADDR 0xE0

/*Public DIM commands */
#define HT16K33_CMD_DIM_LEVEL(dimNibble) (_HT16K33_DIM_ADDR + (dimNibble & 0x0F))

extern volatile uint8_t display_buffer[5];

/**
 *	Setup the SevenSeg display to receive commands and display properly
 */
void initSevenSeg(void);

/**
 *	Set the buffer for sevenSeg digit `index` to display `value` from the numberTable
 * NOTE: This does not write directly, writeSevenSeg() must be called to print to display
 */
void setSevenSegValue(uint8_t index, uint8_t value);

/** 
 * Print all values in the buffer out onto the sevenSeg display
 */
void writeSevenSeg(void);

/**
 *	Enable or disable the blink function built into the displays
 */
void sevenSegBlink(uint8_t blinkSpeed);

/**
 *	Set all digits at once to value and then write to display
 */
void writeAllDigits(uint8_t value);



#endif /* SEVENSEG_H_ */
