/*!
 * @file main.c
 *
 * @mainpage State machine and initialization code
 *
 * @section intro_sec Introduction
 *  This file implements the main logic of the project, other files 
 *  provide convenience functions and separate out complexity from this file 
 *  
 * @section dependencies Dependencies
 *  This code runs on an ATTiny1604- models with less memory 
 *  will not be able to hold the audio arrays
 * 
 * @section author Author
 *  Chase E. Stewart for Hidden Layer Design
 */
#include "audioArrays.h"
#include "main.h"
#include "I2C.h"
#include "SevenSeg.h"
#include "random.h"
 
#define __DELAY_BACKWARD_COMPATIBLE__  //< Required to be defined for delay_msec to work
#include <util/delay.h>
#include <stdbool.h>
#include <xc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

/* static functions */
static void initPeripherals(void);
static void initClocks(void);
static void initCutWires(void);
static void initPIR(void);
static void initLED(void);
static void initAudio(void);
static void initCountdownTimer(void);
static void initADC(void);
static bool PIRisTriggered(void);
static bool wireIsCut(void);
static bool properWireIsCut(uint8_t inWire);
static void setAudioIsEnabled(bool isAudioEnabled);

/* volatile variables */
volatile uint32_t audioIdx;  ///< Index into playing audio array from audioArrays.h
volatile bool ADCResRdy;  ///< True if ADC has results for random sample, else False
volatile bool counterRollover;  ///< True if countdown time has run out, else False
volatile uint32_t pirCount;  ///< How many times has the PIR sensor consecutively been tripped
volatile uint32_t tcbCount;  ///< Countdown timer count that is used to set 7 seg display
volatile uint8_t prevTcbCount;  ///< TODO this is possibly deprecated
volatile board_state_t boardState; ///< Current state enumeration of state machine

/* non-volatile variables */
uint8_t safeWire;  ///< integer index [0-3] of wire selected to be proper wire
uint8_t cut_wire_pos_array[NUM_CUT_WIRES] = {PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};  ///< GPIO bitmask for the wires-to-be-cut

/*!
 * @brief Perform initializations and then implement state machine
 * 
 * @return 0 for success, else failure
 */
int main(void)
{
   audioIdx = 0;
   pirCount = 0;
   ADCResRdy = false;
   counterRollover = false;
   boardState = board_state_waiting;
   tcbCount = 10*100;
   initPeripherals();
   sei();
   
   random_init(adcGetSeed());
   safeWire = (uint8_t) (random() % NUM_CUT_WIRES);
   writeAllDigits(safeWire);

   ledUsrBlink(3, 500);

   writeAllDigits(SEVENSEG_NONE);   
   TCB0.CTRLA = TCB_ENABLE_bm;

   while(1)
   {

      switch(boardState)
      {
		 case board_state_wire_setup:
			// waiting for the wires to be plugged in
			if (!wireIsCut())
			{
			   boardState = board_state_waiting;
               sevenSegBlink(HT16K33_BLINK_OFF);
               writeAllDigits(SEVENSEG_NONE);
			}
         case board_state_waiting:
		    if (wireIsCut())
			{
               sevenSegBlink(HT16K33_BLINK_1HZ);
               writeAllDigits(SEVENSEG_DASH);
			   boardState = board_state_wire_setup;
               PORTC.OUTCLR = PIN2_bm;			   
			   break;
			}
		 
            // Waiting for PIR to trigger and send this to countdown
            if (PIRisTriggered())
            {
               PORTC.OUTSET = PIN2_bm; 
            }
            else
            {
               PORTC.OUTCLR = PIN2_bm;               
            }
            break;
         
         case board_state_countdown:
            // Actively update count until wire is cut or counter rolls over
            setSevenSegValue(0, (tcbCount / 1000) % 10);
            setSevenSegValue(1, (tcbCount / 100)  % 10);
            setSevenSegValue(2, 0x02);
            setSevenSegValue(3, (tcbCount / 10) % 10);
            setSevenSegValue(4, tcbCount % 10);
            writeSevenSeg();
         
            if (counterRollover)
            {
			   audioIdx = 0;
               boardState = board_state_failure;
               counterRollover = false;
               sevenSegBlink(HT16K33_BLINK_2HZ);
               writeAllDigits(0);
               TCB0.CTRLA = 0;
            }
            else if (wireIsCut())
            {             
               if (properWireIsCut(safeWire))
               {
				  audioIdx = 0;
                  boardState = board_state_success;
                  sevenSegBlink(HT16K33_BLINK_2HZ);
                  writeSevenSeg();
               }
               else
               {
				  audioIdx = 0;
                  boardState = board_state_failure;
                  writeAllDigits(0);
                  sevenSegBlink(HT16K33_BLINK_2HZ);
               }
            }
            break;
         
         case board_state_failure:
		    if (audioIdx >= sizeof(goodbye))
			{
				setAudioIsEnabled(false);
				boardState = board_state_done;
				PORTC.OUTCLR = PIN2_bm;
			}
            break;
			
		case board_state_success:
		    if (audioIdx >= sizeof(shutdown))
		    {
			    setAudioIsEnabled(false);
			    boardState = board_state_done;
			    PORTC.OUTCLR = PIN2_bm;
		    }
			break;
			
         case board_state_done:
		    // get stuck forever
			sleep_cpu();
			break;
		 
         default:
            // should never get here!
            ledUsrBlink(0, 1000);
      }
      prevTcbCount = tcbCount;
   }
   return 0;
}

/*!
 * @brief Setup all peripherals.
 *
 * @param None
 *
 * @return None
 */
static void initPeripherals(void)
{
   initClocks();
   initCountdownTimer();
   initLED();
   initPIR();
   I2C_init();
   initSevenSeg();
   initADC();
   initAudio();
   initCutWires();
}

/*!
 * @brief Setup clock registers.
 *
 * @param None
 *
 * @return None
 */
static void initClocks(void)
{
   CLKCTRL.MCLKCTRLA |= CLKCTRL_CLKOUT_bm | CLKCTRL_CLKSEL_OSC20M_gc;
   CLKCTRL.MCLKCTRLB |= CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm;
}

/*!
 * @brief Startup the timer to measure centi-seconds.
 *  Still TODO get CCMP just right
 *
 * @param None
 *
 * @return None
 */
static void initCountdownTimer(void)
{
   TCB0.CTRLB = TCB_CNTMODE_INT_gc;
   TCB0.INTFLAGS = 1;
   TCB0.EVCTRL = TCB_CAPTEI_bm;
   TCB0.INTCTRL = TCB_CAPTEI_bm;
   TCB0.CNT = 0;
   TCB0.CCMP = 0xFFFF;
   TCB0.CTRLA = 0;
}

/*!
 * @brief Setup the TCA0 timer and PWM output to the audio IC.
 *  Also setup the ~SHDN pin 
 *
 * @param None
 *
 * @return None
 */
static void initAudio(void)
{
   /*
    *	set Audio ~SHDN pin to output low for now
    */
   PORTB.DIRSET = PIN3_bm;
   setAudioIsEnabled(false);

   /*
    *	Setup PORTMUX to provide alternate WO1 output
    */
   PORTMUX.CTRLC = PORTMUX_TCA01_bm;

   /*
    *	setup underflow interrupt
    */
   TCA0.SPLIT.INTCTRL = TCA_SPLIT_LUNF_bm;

   /*
    *	setup PWM
    */
   PORTB.DIRSET = PIN4_bm; // Set PWM pin to output
   TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm; // Enable split mode based on v1.9.9 pinout
   TCA0.SPLIT.CTRLB |= (TCA_SPLIT_LCMP1EN_bm); // LCMP1 corresponds to our WO1
   TCA0.SPLIT.LPER  = 0xFF; // use whole 8 bit array
   TCA0.SPLIT.LCMP1 = 0; // this will be controlled by the audioArray
   TCA0.SPLIT.CTRLA = (TCA_SPLIT_CLKSEL_DIV2_gc | TCA_SPLIT_ENABLE_bm);
}

/*!
 * @brief Set or clear audio output SHDN pin based on provided bool arg.
 * 
 * @param isAudioEnabled
 *   Set SHDN pin high if True, else set SHDN pin low
 * 
 * @return None
 */
static void setAudioIsEnabled(bool isAudioEnabled)
{
   if (isAudioEnabled) 
   {
      PORTB.OUTSET = PIN3_bm;
   }   
   else 
   {
      PORTB.OUTCLR = PIN3_bm;   
   }      
}

/*!
 * @brief Set up the PIR sensor for sensor readings.
 *
 * @param None
 *
 * @return None
 */
static void initPIR(void)
{
   PORTC.DIRCLR = PIN0_bm;
   PORTC.PIN0CTRL &= ~PORT_PULLUPEN_bm;
}

/*!
 * @brief Set up the GPIO for the four cut-wires.
 *
 * @param None
 *
 * @return None
 */
static void initCutWires(void)
{
   PORTA.DIRCLR = CUT_WIRES_bm;
   PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
   PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
   PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
   PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

/*!
 * @brief Set up the user LED.
 *
 * @param None
 *
 * @return None
 */
static void initLED(void)
{
   PORTC.DIRSET = PIN2_bm;
}

/*!
 * @brief Setup ADC to get random sample.
 *
 * @param None
 *
 * @return None
 */
static void initADC(void)
{
   ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;
   ADC0.CTRLA |= ADC_ENABLE_bm;
   ADC0.INTCTRL = ADC_RESRDY_bm;
}

/*!
 * @brief Return whether PIR sensor digital output is high at this moment.
 * 
 * @param None
 * 
 * @return True if PIR is currently triggered, else False
 */
static bool PIRisTriggered(void)
{
   return 0 != (PORTC.IN & PIN0_bm);
}

/*!
 * @brief Blink usr LED 'count'-many times at a period of 2 * 'blinkPeriodMsec'.
 *  NOTE: This is a blocking call!
 * 
 * @param count
 *  The number of times for the LED to blink- set to zero for infinite repetitions
 *  
 * @param blinkPeriodMsec
 *  The period between LED toggles in Msec
 */
void ledUsrBlink(uint8_t count, const int blinkPeriodMsec)
{
   uint8_t iter=0;

   while (!count || iter < count)
   {
      PORTC.OUTSET = PIN2_bm;
      _delay_ms(blinkPeriodMsec);
      PORTC.OUTCLR = PIN2_bm;
      _delay_ms(blinkPeriodMsec);
      iter++;
   }
   // clear LED after blinking is done
   PORTC.OUTCLR = PIN2_bm;
}

/*!
 * @brief Timer Interrupt for countdown timer TCB0.
 *
 * @param TCB0_INT_vect 
 *  Unused parameter required by interface
 *
 * @return None
 */
ISR(TCB0_INT_vect)
{
   TCB0.INTFLAGS = 1;
   
   if (boardState == board_state_countdown)
   {   
      tcbCount--;
      if (tcbCount == 0)
      {
         counterRollover = true;
         tcbCount = 10 * 100;
      }
   }      
   else if (boardState == board_state_waiting)
   {
      pirCount = (PIRisTriggered()) ? pirCount + 1 : 0;
      if (pirCount >= 350)
      {
         PORTC.OUTCLR = PIN2_bm;
         boardState = board_state_countdown;
         setAudioIsEnabled(true);   
      }
   }
}

/*!
 * @brief TCA interrupt for lower split underflow
 *
 * @param TCA0_LUNF_vect 
 *  Unused parameter required by interface
 * 
 * @return None
 */
ISR(TCA0_LUNF_vect)
{
   TCA0.SPLIT.INTFLAGS = TCA_SPLIT_LUNF_bm;
   switch (boardState)
   {
      case board_state_countdown:
         TCA0.SPLIT.LCMP1 = (siren[audioIdx]);
         audioIdx = (audioIdx < sizeof(siren)) ? audioIdx + 1 : 0;
		 break; 
	  case board_state_success:
         TCA0.SPLIT.LCMP1 = (shutdown[audioIdx]);
         audioIdx = (audioIdx < sizeof(shutdown)) ? audioIdx + 1 : audioIdx;
	     break;
		 
	  case board_state_failure:
         TCA0.SPLIT.LCMP1 = (goodbye[audioIdx]);
         audioIdx = (audioIdx < sizeof(goodbye)) ? audioIdx + 1 : audioIdx;
	     break;

      default:
         return;
   }
}

/*!
 * @brief ADC interrupt when ADC results are ready to be taken.
 * 
 * @param ADC0_RESRDY_vect
 *  Unused parameter required by interface
 *
 * @return None
 */
ISR(ADC0_RESRDY_vect)
{
   ADC0.INTFLAGS = ADC_RESRDY_bm;
   ADC0.CTRLA &= !ADC_ENABLE_bm;
   ADC0.INTCTRL &= !ADC_RESRDY_bm;
   ADCResRdy = true;
}

/*!
 * @brief Return whether any wire was cut.
 *
 * @param None
 * 
 * @return True if any wire has been cut, else False
 */
static bool wireIsCut(void)
{
   return PORTA.IN & CUT_WIRES_bm; // anything > 0 means a wire is cut
}

/*!
 * @brief Return whether only the proper wire was cut out of all wires.
 *
 * @param inWire
 *  The current state of cut and un-cut wires
 *
 * @return True if only the designated success wire is cut, else false
 */
static bool properWireIsCut(uint8_t inWire)
{
   return  ((PORTA.IN & CUT_WIRES_bm) == cut_wire_pos_array[inWire]);
}

