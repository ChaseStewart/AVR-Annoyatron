/**
 * main.c
 *
 * Created: 12/11/2021 2:35:04 PM
 * Author: Chase E. Stewart for Hidden Layer Design
 */
#include "audioArrays.h"
#include "main.h"
#include "I2C.h"
#include "SevenSeg.h"
#include "random.h"

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
static void initADC(void);
static bool PIRisTriggered(void);
static bool wireIsCut(void);
static bool properWireIsCut(uint8_t inWire);
static void setAudioIsEnabled(bool isAudioEnabled);


volatile uint32_t audioIdx;
volatile bool ADCResRdy;
volatile bool counterRollover;
volatile uint32_t pirCount;
volatile uint32_t tcbCount;
volatile uint8_t prevTcbCount;
volatile board_state_t boardState;
uint8_t safeWire;

uint8_t cut_wire_pos_array[NUM_CUT_WIRES] = {PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm};


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
         case board_state_waiting:
            // Waiting for PIR to trigger and send this to countdown
            PORTC.OUT = (PIRisTriggered()) ? (PIN2_bm | PIN3_bm) : PIN3_bm;
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
               setAudioIsEnabled(false);
               boardState = board_state_cut;
               counterRollover = false;
               sevenSegBlink(HT16K33_BLINK_2HZ);
               writeAllDigits(0);
               TCB0.CTRLA = 0;
            }
            else if (wireIsCut())
            {
               setAudioIsEnabled(false);
               
               if (properWireIsCut(safeWire))
               {
                  boardState = board_state_cut;
                  sevenSegBlink(HT16K33_BLINK_2HZ);
                  writeSevenSeg();
               }
               else
               {
                  boardState = board_state_cut;
                  writeAllDigits(0);
                  sevenSegBlink(HT16K33_BLINK_1HZ);
               }
            }
            break;
         
         case board_state_cut:
            // get stuck forever
            break;
         
         default:
            // should never get here!
            ledUsrBlink(0, 1000);
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
   I2C_init();
   initSevenSeg();
   initADC();
   initAudio();
   initCutWires();
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
 * Still TODO get CCMP just right
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
   setAudioIsEnabled(false);

   /*
    *	setup underflow interrupt
    */
   TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm;

   /*
    *	setup PWM
    */
   PORTA.DIRSET = PIN3_bm; // Set PWM pin to output
   TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm; // Enable split mode based on v1.9.9 pinout
   TCA0.SPLIT.CTRLB |= (TCA_SPLIT_HCMP0EN_bm); // HCMP0 corresponds to our WO3
   TCA0.SPLIT.HPER  = 0xFF; // use whole 8 bit array
   TCA0.SPLIT.HCMP0 = 0; // this will be controlled by the audioArray
   TCA0.SPLIT.CTRLA = (TCA_SPLIT_CLKSEL_DIV16_gc | TCA_SPLIT_ENABLE_bm);
}

/**
 *	Set or clear audio output SHDN pin based on provided bool arg
 */
static void setAudioIsEnabled(bool isAudioEnabled)
{
   if (isAudioEnabled) 
   {
      PORTC.OUTSET = PIN3_bm;
   }   
   else 
   {
      PORTC.OUTCLR = PIN3_bm;   
   }      
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
   PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
   PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
   PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
   PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

/**
 * Set up the user LED
 */
static void initLED(void)
{
   PORTC.DIR |= PIN2_bm;
}

static void initADC(void)
{
   ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;
   ADC0.CTRLA |= ADC_ENABLE_bm;
   ADC0.INTCTRL = ADC_RESRDY_bm;
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
 *  NOTE: This is a blocking call!
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

/**
 *	Timer Interrupt for countdown timer TCB0
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

ISR(TCA0_HUNF_vect)
{
   TCA0.SPLIT.INTFLAGS = TCA_SPLIT_HUNF_bm;
   switch (boardState)
   {
      case board_state_countdown:
         // *** START TODO I know this is working, re-enable when I can make audio quieter!
         //TCA0.SPLIT.HCMP0 = (shortSiren[audioIdx]);
         // *** END TODO I know this is working, re-enable when I can make audio quieter!
         audioIdx = (audioIdx < sizeof(shortSiren)) ? audioIdx + 1 : 0;
      default:
         return;
   }
}

/**
 *	ADC interrupt when ADC results are ready to be taken
 */
ISR(ADC0_RESRDY_vect)
{
   ADC0.INTFLAGS = ADC_RESRDY_bm;
   ADC0.CTRLA &= !ADC_ENABLE_bm;
   ADC0.INTCTRL &= !ADC_RESRDY_bm;
   ADCResRdy = true;
}

static bool wireIsCut(void)
{
   return PORTA.IN & CUT_WIRES_bm; // anything > 0 means a wire is cut
}

static bool properWireIsCut(uint8_t inWire)
{
   return  ((PORTA.IN & CUT_WIRES_bm) == cut_wire_pos_array[inWire]);
}
