/*
 * main.h
 *
 * Created: 12/12/2021 6:23:38 PM
 * Author: Chase E. Stewart for Hidden Layer Design
 */ 

#include "stdbool.h"
#include "stdint.h"

#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 3333333

#define NUM_CUT_WIRES 4
#define CUT_WIRES_bm (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)

typedef enum board_state_enum
{
   board_state_waiting,
   board_state_countdown,
   board_state_cut
} board_state_t;

extern volatile bool ADCResRdy;

/**
 *	Blink the user LED `count` or infinity times at a rate of `blinkPeriodMsec`
 */
void ledUsrBlink(uint8_t count, const int blinkPeriodMsec);


#endif /* MAIN_H_ */
