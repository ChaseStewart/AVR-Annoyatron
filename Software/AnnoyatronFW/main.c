/*!
 * @file main.c
 *
 * @mainpage Annoyatron v2.0 Firmware
 *
 * @section intro_sec Introduction
 *  This documentation describes firmware for an AVR ATTiny1606 on a custom-designed PCB to become a James Bond movie prop. 
 *  On boot, A PIR sensor is used to detect motion nearby. When motion is detected, a timer counts down and a sound begins to play.
 *  The user is presented with four wires to cut- cutting the right one before the timer hits zero will play the success noise, and cutting any other wire or letting
 *  the timer fully count down without cutting any wires at all will play the failure noise. Either way, the success or failure state each play a one-shot noise,
 *  and then put the MCU in sleep mode until it is reset. 
 *
 * @section history History
 *  This project recreates an undergraduate project at Rice University in 2013-2014. 
 *  The midterm group project for ELEC 327 required designing and building an embedded system based on the Texas Instruments MSP430
 *  and some amount of sensors and/or actuators. We wound up creating what we described as a "James Bond movie prop", and with only a few fly-wires
 *  managed to get it working well enough to pick up an A grade. Despite my enthusiasm and hard work, my partner was much more experienced in PCB design
 *  and very good at software to boot- so honestly I could not at the time have gotten the project to work by myself. I wanted to redesign the project around a
 *  more modern microcontroller, and to do a solo implementation of the PCB now that I am able, adding some improvements along the way.
 *  
 * @section dependencies Dependencies
 *  The project file for this code is for Microchip Studio (based on the Eclipse IDE), which you can get from <a href="microchip.com/en-us/tools-resources/develop/microchip-studio">Microchip's website</a>.
 *  This code runs on an ATTiny1606- ATTiny models with less memory than 16K will not be able to hold the audio arrays.
 *  You will need Python3 and `pymcuprog` to actually flash the MCU with the compiled code.
 *  PCB setup, assembly, and connectivity to your computer is left as an exercise to the reader ;)
 * 
 * @section author Author
 *  Chase E. Stewart for <a href="https://www.hiddenlayerdesign.com">Hidden Layer Design</a>
 *
 * @section source_code Source Code 
 * <a href="https://github.com/ChaseStewart/AVR-Annoyatron">AVR-Annoyatron on GitHub</a>
 */
#include "audio/audioArrays.h"
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
static void setLed(bool isLedSet);

/* volatile variables */
volatile uint32_t audioIdx;  ///< Index into playing audio array from audioArrays.h
volatile bool ADCResRdy;  ///< True if ADC has results for random sample, else False
volatile bool counterRollover;  ///< True if countdown time has run out, else False
volatile uint32_t pirHighCount;  ///< How many times has the PIR sensor consecutively been tripped
volatile uint32_t pirLowCount;  ///< How many times has the PIR sensor consecutively been tripped
volatile uint32_t tcbCount;  ///< Countdown timer count that is used to set 7 seg display
volatile board_state_t boardState; ///< Current state enumeration of state machine
volatile blink_state_t blinkState;  ///< State machine for blinking LED during countdown
volatile uint32_t blinkCount;  ///< Actual counter value for nonblocking LED blink

/* non-volatile variables */
uint8_t safeWire;  ///< integer index [0-3] of wire selected to be proper wire
uint8_t cut_wire_pos_array[NUM_CUT_WIRES] = {PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};  ///< GPIO bitmask for the wires-to-be-cut
blink_state_t blinkReloadState[4] = {blink_state_1_high, blink_state_2_high, blink_state_3_high, blink_state_4_high};  ///< Location to restart the blink pattern


/*!
 * @brief Perform initializations and then implement state machine
 * 
 * @return 0 for success, else failure
 */
int main(void)
{
   audioIdx = 0;
   pirHighCount = 0;
   pirLowCount = 0;
   ADCResRdy = false;
   counterRollover = false;
   tcbCount = 10*100;
   blinkCount = BLINK_COUNT_SHORT;
   initPeripherals();
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   sei();
   
   random_init(adcGetSeed());
   safeWire = (uint8_t) (random() % NUM_CUT_WIRES);
   blinkState = blinkReloadState[safeWire];
   writeAllDigits(safeWire + 1);

   ledUsrBlink(3, 500);

   writeAllDigits(SEVENSEG_NONE);   
   TCB0.CTRLA = TCB_ENABLE_bm;

   boardState = board_state_sleep;
   setLed(false);
   PORTC.PIN0CTRL |= PORT_ISC_BOTHEDGES_gc;
   
   while(1)
   {

      switch(boardState)
      {
		 case board_state_wire_setup:
			// waiting for the wires to be plugged in
			if (!wireIsCut())
			{
               sevenSegBlink(HT16K33_BLINK_OFF);
               writeAllDigits(SEVENSEG_NONE);
               boardState = board_state_sleep;
			   pirHighCount = 0;
			   pirLowCount = 0;
               setLed(false);
               PORTC.PIN0CTRL |= PORT_ISC_BOTHEDGES_gc;
			}
			break;
			
         case board_state_sleep:
			sleep_mode();
		    break;
		 
         case board_state_waiting:
		    if (wireIsCut())
			{
               sevenSegBlink(HT16K33_BLINK_1HZ);
               writeAllDigits(SEVENSEG_DASH);
               boardState = board_state_wire_setup;
               setLed(false);
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
                  sevenSegBlink(HT16K33_BLINK_HALFHZ);
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
		    if (audioIdx >= sizeof(youLose))
			{
				setAudioIsEnabled(false);
				boardState = board_state_done;
				setLed(false);
			}
            break;
			
		case board_state_success:
		    if (audioIdx >= sizeof(youWin))
		    {
			    setAudioIsEnabled(false);
			    boardState = board_state_done;
			    setLed(false);
		    }
			break;
			
         case board_state_done:
		    // get stuck forever
			sleep_mode();
			break;
		 
         default:
            // should never get here!
            ledUsrBlink(0, 1000);
      }
   }
   // should never get here!
   return 0;
}

/*!
 * @defgroup HardwareInit
 * Functions to initialize the hardware for this project
 */

/*!
 * @ingroup HardwareInit
 *
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
 * @ingroup HardwareInit
 *
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
 * @ingroup HardwareInit
 *
 * @brief Startup the timer to measure centi-seconds.
 * TODO get CCMP just right, validate with scope
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
 * @ingroup HardwareInit
 *
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
 * @ingroup HardwareInit
 *
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
 * @ingroup HardwareInit
 *
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
 * @ingroup HardwareInit
 *
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
 * @ingroup HardwareInit
 *
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
 * @defgroup ISRs
 * Interrupt service routines with custom handlers for this project
 */

/*!
 * @ingroup ISRs
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
	  blinkCount--;
	  
	  if (!blinkCount)
	  {
	    blinkState--;
		if (blinkState == blink_state_0)
		{
			setLed(true);
			blinkState = blinkReloadState[safeWire];
			blinkCount = BLINK_COUNT_SHORT;
		}
		else if (blinkState == blink_state_1_low)
		{
			setLed(false);
			blinkCount = BLINK_COUNT_LONG;
		}
		else
		{
			blinkCount = BLINK_COUNT_SHORT;
			setLed( (blinkState % 2) ? false : true);
		}
	  }
	  
      if (tcbCount == 0)
      {
         counterRollover = true;
         tcbCount = 10 * 100;
      }
   }      
   else if (boardState == board_state_waiting)
   {
	  /** 
	   * start counting towards the configured amount of
	   * PIR detects until proceeding to countdown mode
	   */
	  if (PIRisTriggered())
	  {
		  pirHighCount += 1;
		  pirLowCount = 0;
		  if (pirHighCount >= PIR_HIGH_COUNT_TO_COUNTDOWN)
		  {
			  boardState = board_state_countdown;
			  setAudioIsEnabled(true);
			  setLed(false);
		  }
	  }
	  /** 
	   * start counting towards the separately configured amount of
	   * PIR non-detects until proceeding to PWR_DOWN sleep mode
	   */
	  else
	  {
		  pirHighCount = 0;
		  pirLowCount += 1;		  
		  if (pirLowCount >= PIR_LOW_COUNT_TO_SLEEP)
		  {
			  boardState = board_state_sleep;
			  setAudioIsEnabled(false);
			  setLed(false);
              PORTC.PIN0CTRL |= PORT_ISC_BOTHEDGES_gc;
		  }
	  }
   }
}

/*!
 * @ingroup ISRs
 * @brief TCA interrupt for PWM timer, to load next audio sample into compare register.
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
         TCA0.SPLIT.LCMP1 = (youWin[audioIdx]);
         audioIdx = (audioIdx < sizeof(youWin)) ? audioIdx + 1 : audioIdx;
	     break;
		 
	  case board_state_failure:
         TCA0.SPLIT.LCMP1 = (youLose[audioIdx]);
         audioIdx = (audioIdx < sizeof(youLose)) ? audioIdx + 1 : audioIdx;
	     break;

      default:
         return;
   }
}

/*!
 * @ingroup ISRs
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
 * @ingroup ISRs
 * @brief GPIO interrupt driven by PIR sensor when in sleep mode
 * 
 * @param PORTC_PORT_vect
 *  Unused parameter required by interface
 *
 * @return None
 */
ISR(PORTC_PORT_vect)
{
   if (PC0_INTERRUPT)
   {
	   // clear int flag so we don't repeatedly trigger
	   PC0_CLEAR_INTERRUPT_FLAG; 
	   
	   // disable this interrupt until state machine re-enables later
	   PORTC.PIN0CTRL &= ~PORT_ISC_BOTHEDGES_gc;
	   
	  // wake up to board_state_waiting
      boardState = board_state_waiting;
	  setLed(true);

   }
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

/*!
 * @brief Set or clear the user LED according to the provided parameter
 *
 * @param isLedSet
 *  True means LED will be set, false means LED will be cleared
 *
 * @return None
 */
static void setLed(bool isLedSet)
{
	if (isLedSet)
	{
		PORTC.OUTSET = PIN2_bm;
	}
	else
	{
		PORTC.OUTCLR = PIN2_bm;
	}
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
      setLed(true);
      _delay_ms(blinkPeriodMsec);
      setLed(false);
      _delay_ms(blinkPeriodMsec);
      iter++;
   }
   // clear LED after blinking is done
   setLed(false);
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
