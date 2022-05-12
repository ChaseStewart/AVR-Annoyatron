/*!
 * @file random.c
 *
 * Copyright (c) 2017, ?ukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Lightweight library of 16 bit random number generator based on LFSR.
 */

#include <avr/eeprom.h>
#include "main.h"
#include "random.h"

static uint16_t random_number = 0;  ///< Returned random number

/*!
 * @brief return next LFSR value
 *
 * @param n
 *  the current LFSR-returned value
 * 
 * @return next uint16_t LFSR value
 */
static uint16_t lfsr16_next(uint16_t n)
{
	return (n >> 0x01U) ^ (-(n & 0x01U) & 0xB400U);
}

/*!
 * @brief Initialize the random generator.
 * 
 * @param seed
 *  A uint16_t random seed to start the LFSR values
 */
void random_init(uint16_t seed)
{
#ifdef USE_RANDOM_SEED
	random_number = lfsr16_next(eeprom_read_word((uint16_t *)RANDOM_SEED_ADDRESS) ^ seed);
        eeprom_write_word((uint16_t *)0, random_number);
#else
	random_number = seed;
#endif	/* !USE_RANDOM_SEED */
}

/*!
 * @brief Return a random number.
 *
 * @return random uint16_t
 */
uint16_t random(void)
{
	return (random_number = lfsr16_next(random_number));
}

/*!
 * @brief Use the ATTiny1606 ADC to get a random seed to use in random_init.
 *
 * @return random seed as uint16_t
 */
uint16_t adcGetSeed(void)
{
   // Start ADC capture, and then wait for result to return
   ADC0.COMMAND = ADC_STARTEI_bm;
   while (!ADCResRdy);
   
   return ADC0.RES;   
}
