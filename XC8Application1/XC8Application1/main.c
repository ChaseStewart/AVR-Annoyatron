/*
 * main.c
 *
 * Created: 12/11/2021 2:35:04 PM
 * Author: Chase E. Stewart for Hidden Layer Design
 */ 

#define F_CPU 3333333
#include <xc.h>
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	PORTC.DIR |= PIN2_bm;
	
	
    while(1)
    {
		PORTC.OUT |= PIN2_bm;
		_delay_ms(500);
		PORTC.OUT &= ~PIN2_bm;
		_delay_ms(500);
    }
}