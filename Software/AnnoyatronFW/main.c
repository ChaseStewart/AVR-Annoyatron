/*
* main.c
*
* Created: 12/11/2021 2:35:04 PM
* Author: Chase E. Stewart for Hidden Layer Design
*/

#include "main.h"
#include "I2C.h"
#include "SevenSeg.h"

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <stdbool.h>
#include <xc.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/* static functions */
static void initPeripherals(void);
static void initClocks(void);
static void initCutWires(void);
static void initPIR(void);
static void initLED(void);
static void initAudio(void);
static void initCountdownTimer(void);
static bool PIRisTriggered(void);

volatile uint32_t tcbCount;
volatile uint8_t prevTcbCount;

int main(void)
{
   tcbCount = 20*100;
   initPeripherals();
   sei();
   ledUsrBlink(3, 500);
   
   while(1)
   {
      if (PIRisTriggered())
      {
         PORTC.OUT = PIN2_bm;
         setSevenSegValue(0, (tcbCount / 1000) % 10);
         setSevenSegValue(1, (tcbCount / 100)  % 10);
         setSevenSegValue(2, 0x02);
         setSevenSegValue(3, (tcbCount / 10) % 10);
         setSevenSegValue(4, tcbCount % 10);
      }
      else
      {
         PORTC.OUT = 0;
         setSevenSegValue(0, SEVENSEG_NONE);
         setSevenSegValue(1, SEVENSEG_NONE);
         setSevenSegValue(2, SEVENSEG_NONE);
         setSevenSegValue(3, SEVENSEG_NONE);
         setSevenSegValue(4, SEVENSEG_NONE);
      }
      
      if (tcbCount != prevTcbCount)
      {
         writeSevenSeg();
      }
      prevTcbCount = tcbCount;
   }
}

/**
* Setup all peripherals
*/
static void initPeripherals(void)
{
   initClocks();
   initCountdownTimer();
   initLED();
   initPIR();
   initCutWires();
   I2C_init(); 
   initSevenSeg();
   initAudio();
}

/**
 *	Setup clock registers
 */
static void initClocks(void)
{
   CLKCTRL.MCLKCTRLA |= CLKCTRL_CLKOUT_bm | CLKCTRL_CLKSEL_OSC20M_gc;
   CLKCTRL.MCLKCTRLB |= CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm;      
}

/**
 *	Startup the timer to measure centi-seconds
 */
static void initCountdownTimer(void)
{
   TCB0.CTRLB = TCB_CNTMODE_INT_gc;
   TCB0.INTFLAGS = 1;
   TCB0.EVCTRL = TCB_CAPTEI_bm;
   TCB0.INTCTRL = TCB_CAPTEI_bm;
   TCB0.CNT = 0;
   TCB0.CCMP = 0xFFFF;
   TCB0.CTRLA = TCB_ENABLE_bm;
}


/**
 *	Setup the TCA0 timer and PWM output to the audio IC
 * Also set the ~SHDN pin high
 */
static void initAudio(void)
{
   /*
   *	set Audio ~SHDN pin high
   */
   PORTC.DIRSET = PIN3_bm;
   PORTC.OUT |= PIN3_bm;

   /*
   *	setup PWM
   */
   PORTA.DIRSET = PIN3_bm;
   TCA0.SINGLE.CTRLD &= ~TCA_SPLIT_ENABLE_bm;
   TCA0.SINGLE.PER = 5000;
   TCA0.SINGLE.CMP0 = 2500;
   TCA0.SINGLE.CNT = 0;
   TCA0.SINGLE.CTRLB |= (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_CMP0EN_bm); // we want WO3 so set 0b11
   TCA0.SINGLE.CTRLB |= (TCA_SINGLE_WGMODE0_bm | TCA_SINGLE_WGMODE1_bm); // WGMode 3 == single PWM
   TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV2_gc | TCA_SINGLE_ENABLE_bm); // now enable the module
}

/**
 *	Set up the PIR sensor for sensor readings
 */
static void initPIR(void)
{
   PORTC.DIRCLR = PIN0_bm;
   PORTC.PIN0CTRL &= ~PORT_PULLUPEN_bm;
}

/**
 *	Set up the GPIO for the four cut-wires
 */
static void initCutWires(void)
{
   PORTA.DIRCLR = CUT_WIRES_bm;
   PORTA.PIN4CTRL |= PORT_PULLUPEN_bm;
   PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
   PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;
   PORTA.PIN7CTRL |= PORT_PULLUPEN_bm;
}

/**
* Set up the user LED
*/
static void initLED(void)
{
   PORTC.DIR |= PIN2_bm;
}

/**
 *	Return whether PIR sensor digital output is high at this moment
 */
static bool PIRisTriggered(void)
{
   return 0 != (PORTC.IN & PIN0_bm);
}

/**
*	Blink usr LED 'count'-many times at a period of 2 * 'blinkPeriodMsec'
*/
void ledUsrBlink(uint8_t count, const int blinkPeriodMsec)
{
   uint8_t iter=0;

   while (!count || iter < count)
   {
      PORTC.OUT |= PIN2_bm;
      _delay_ms(blinkPeriodMsec);
      PORTC.OUT &= ~PIN2_bm;
      _delay_ms(blinkPeriodMsec);
      iter++;
   }

   // clear LED after blinking is done
   PORTC.OUT &= ~PIN2_bm;
}

ISR(TCB0_INT_vect)
{
   TCB0.INTFLAGS = 1;
   tcbCount = (tcbCount) ? tcbCount - 1  : 20 * 100 ;
}