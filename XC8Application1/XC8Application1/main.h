/*
 * main.h
 *
 * Created: 12/12/2021 6:23:38 PM
 *  Author: vtrre
 */ 

#include "stdint.h"

#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 3333333

static const uint8_t numbertable[] = {
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

#define TRUE 1
#define FALSE 0

#define SEVENSEG_ADDR (0x70 << 1)

#define HT16K33_KEYDATA0_ADDR 0x40
#define HT16K33_KEYDATA_LEN 5

#define HT16K33_SYS_SETUP_ADDR 0x20
#define HT16K33_SYS_SETUP_OSC_ON  0x01
#define HT16K33_SYS_SETUP_OSC_OFF 0x00
#define HT16K33_CMD_OSC_ENABLE (HT16K33_SYS_SETUP_ADDR | HT16K33_SYS_SETUP_OSC_ON)
#define HT16K33_CMD_OSC_DISABLE (HT16K33_SYS_SETUP_ADDR | HT16K33_SYS_SETUP_OSC_OFF)

#define HT16K33_DISP_SET_ADDR 0x80
#define HT16K33_DISP_SET_DISPLAYON 0x01
#define HT16K33_DISP_SET_DISPLAYOFF 0x00
#define HT16K33_BLINK_OFF    0x00
#define HT16K33_BLINK_2HZ    0x02
#define HT16K33_BLINK_1HZ    0x04
#define HT16K33_BLINK_HALFHZ 0x06

#define HT16K33_DIM_ADDR 0xE0

#define HT16K33_CMD_DISP_OFF (HT16K33_DISP_SET_ADDR | HT16K33_DISP_SET_DISPLAYOFF | HT16K33_BLINK_OFF)
#define HT16K33_CMD_DISP_ON_NOBLINK (HT16K33_DISP_SET_ADDR | HT16K33_DISP_SET_DISPLAYON | HT16K33_BLINK_OFF)
#define HT16K33_CMD_DISP_ON_BLINK (HT16K33_DISP_SET_ADDR | HT16K33_DISP_SET_DISPLAYON | HT16K33_BLINK_1HZ)
#define HT16K33_CMD_DIM_LEVEL(dimNibble) (0xE0 + (dimNibble & 0x0F))

#define CUT_WIRES_bm (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)

void initPeripherals(void);
void initCutWires(void);
void initPIR(void);
void initLED(void);
void initSevenSeg(void);

void ledUsrBlink(uint8_t count, const int blinkPeriodMsec);

void setSevenSegValue(uint8_t index, uint8_t value);
void writeSevenSeg(void);



#endif /* MAIN_H_ */