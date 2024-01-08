/*!
 * @file main.h
 *
 * Created: 12/12/2021 6:23:38 PM
 *
 * Author: Chase E. Stewart for Hidden Layer Design
 *
 * Contains configuration variables and definitions of shared items
 */ 

#include "stdbool.h"
#include "stdint.h"

#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 3333333  ///< Frequency of CPU in HZ

#define NUM_CUT_WIRES 4  ///< Number of slots for cut wires on the device
#define CUT_WIRES_bm (PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm) ///< bit masks for GPIO pins of cut wires

#define BLINK_COUNT_SHORT  5 ///< Duration between toggles for short LED blinks
#define BLINK_COUNT_LONG  11 ///< Duration between toggles for long LED blinks

#define PIR_HIGH_COUNT_TO_COUNTDOWN 350 ///< Compare val for pirHighCount until state -> board_state_countdown
#define PIR_LOW_COUNT_TO_SLEEP 350 ///< Compare val for pirLowCount until state -> board_state_sleep

#define PC0_INTERRUPT  PORTC.INTFLAGS & PIN0_bm  ///< True if the PortC pin0 GPIO interrupt fired
#define PC0_CLEAR_INTERRUPT_FLAG  PORTC.INTFLAGS |= PIN0_bm  ///< Clear the PortC pin0 interrupt flag bit

/** All of the possible states in the state machine */
typedef enum board_state_enum
{
   board_state_wire_setup,
   board_state_sleep,
   board_state_waiting,
   board_state_countdown,
   board_state_success,
   board_state_failure,
   board_state_done
} board_state_t;

/** State machine to hold the blinking pattern for different cut_wires options */
typedef enum blink_state_enum
{
	blink_state_4_high = 8,
	blink_state_4_low  = 7,
	blink_state_3_high = 6,
	blink_state_3_low  = 5,
	blink_state_2_high = 4,
	blink_state_2_low  = 3,
	blink_state_1_high = 2,
	blink_state_1_low  = 1,
	blink_state_0      = 0,
} blink_state_t;

extern volatile bool ADCResRdy;

void ledUsrBlink(uint8_t count, const int blinkPeriodMsec);


#endif /* MAIN_H_ */
