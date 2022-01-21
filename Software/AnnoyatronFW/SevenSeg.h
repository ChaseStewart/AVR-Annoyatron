/*
 * SevenSeg.h
 *
 * Created: 1/14/2022 8:50:13 AM
 *  Author: vtrre
 */ 

#include <stdbool.h>
#include "stdint.h"

#ifndef SEVENSEG_H_
#define SEVENSEG_H_


#define LEN_NUM_TABLE 18
static const uint8_t numbertable[LEN_NUM_TABLE] = {
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
#define SEVENSEG_ALL  LEN_NUM_TABLE - 2
#define SEVENSEG_NONE LEN_NUM_TABLE - 1

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
#define _HT16K33_BLINK_OFF    0x00
#define _HT16K33_BLINK_2HZ    0x02
#define _HT16K33_BLINK_1HZ    0x04
#define _HT16K33_BLINK_HALFHZ 0x06

/* Public DISP commands */
#define HT16K33_CMD_DISP_OFF (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYOFF | _HT16K33_BLINK_OFF)
#define HT16K33_CMD_DISP_ON_NOBLINK (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYON | _HT16K33_BLINK_OFF)
#define HT16K33_CMD_DISP_ON_BLINK (_HT16K33_DISP_SET_ADDR | _HT16K33_DISP_SET_DISPLAYON | _HT16K33_BLINK_2HZ)


#define _HT16K33_DIM_ADDR 0xE0

/*Public DIM commands */
#define HT16K33_CMD_DIM_LEVEL(dimNibble) (_HT16K33_DIM_ADDR + (dimNibble & 0x0F))


extern volatile uint8_t display_buffer[5];


void initSevenSeg(void);
void setSevenSegValue(uint8_t index, uint8_t value);
void writeSevenSeg(void);
void sevenSegBlink(bool blinkEnable);



#endif /* SEVENSEG_H_ */