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

#define CUT_WIRES_bm (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)

typedef enum board_state_enum
{
   board_state_waiting,
   board_state_countdown,
   board_state_cut
} board_state_t;

void ledUsrBlink(uint8_t count, const int blinkPeriodMsec);


#endif /* MAIN_H_ */