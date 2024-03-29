/*!
 * @file I2C.c
 *
 * Author: Dieter Reinhardt
 *
 * Tested with Alternate Pinout
 *
 * This software is covered by a modified MIT License, see paragraphs 4 and 5
 *
 * Copyright (c) 2019 Dieter Reinhardt
 *
 * 1. Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 2. The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * 3. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * 4. This software is strictly NOT intended for safety-critical or life-critical applications.
 * If the user intends to use this software or parts thereof for such purpose additional 
 * certification is required.
 *
 * 5. Parts of this software are adapted from Microchip I2C polled master driver sample code.
 * Additional license restrictions from Microchip may apply.
 */ 
#include "main.h"


#include <avr/io.h>
#include <util/delay.h>

#include "I2C.h"

#define NOP() asm volatile(" nop \r\n")  ///< Define a no-op action from assembly

uint8_t timeout_cnt = 0; ///< count of consecutive timeouts

/*!
 * @ingroup HardwareInit
 *
 * @brief set up I2C for ATTiny1606.
 *
 * @return None
 */
void I2C_init(void)
{
   /* Set PB0 for SCL */
   PORTB.DIRCLR = PIN0_bm;	//PORTB_set_pin_dir(0, PORT_DIR_IN);
   PORTB.OUTCLR = PIN0_bm; //PORTB_set_pin_level(0, false);
   PORTB.PIN0CTRL &= ~PORT_PULLUPEN_bm;  //PORTB_set_pin_pull_mode(0, PORT_PULL_OFF);
   PORTB.PIN0CTRL &= ~PORT_INVEN_bm;  //PORTB_pin_set_inverted(0, false);
   PORTB.PIN0CTRL |= PORT_ISC_INTDISABLE_gc;  //PORTB_pin_set_isc(0, PORT_ISC_INTDISABLE_gc);

   /* Set PB1 for SDA */
   PORTB.DIRSET = PIN1_bm;  //PORTB_set_pin_dir(1, PORT_DIR_OUT);
   PORTB.OUTCLR = PIN1_bm;  //PORTB_set_pin_level(1, false);
   PORTB.PIN1CTRL |= ~PORT_PULLUPEN_bm;  //PORTB_set_pin_pull_mode(1, PORT_PULL_OFF);
   PORTB.PIN1CTRL &= ~PORT_INVEN_bm;  //PORTB_pin_set_inverted(1, false);
   PORTB.PIN1CTRL |= PORT_ISC_INTDISABLE_gc;  //PORTB_pin_set_isc(1, PORT_ISC_INTDISABLE_gc);

   /* Setup TWI0 register */
	TWI0.MBAUD = (uint8_t) 0x5F; //TWI0_BAUD(100000, 0);				// set MBAUD register, TWI0_BAUD macro calculates parameter for 100 kHz
	TWI0.MCTRLB = TWI_FLUSH_bm;									// clear the internal state of the master
	TWI0.MCTRLA =	  1 << TWI_ENABLE_bp							// Enable TWI Master: enabled
					| 0 << TWI_QCEN_bp								// Quick Command Enable: disabled
					| 0 << TWI_RIEN_bp								// Read Interrupt Enable: disabled
					| 0 << TWI_SMEN_bp								// Smart Mode Enable: disabled
					| TWI_TIMEOUT_DISABLED_gc						// Bus Timeout Disabled (inoperative, see errata)
					| 0 << TWI_WIEN_bp;								// Write Interrupt Enable: disabled

   PORTB.DIRSET = PIN0_bm;  //PORTB_set_pin_dir(0, PORT_DIR_OUT);	

	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;							// force bus idle
	TWI0.MSTATUS |= (TWI_RIF_bm | TWI_WIF_bm | TWI_BUSERR_bm);		// clear flags	
}

/*!
 * @brief get I2C back to a good state after a failure. 
 *
 * @return None	
 */
void I2C_recover(void)												// clock out I2C bus if in invalid state (e.g. after incomplete transaction)
{
	uint8_t	i;
	TWI0.MCTRLB |= TWI_FLUSH_bm;									// clear the internal state of the master
	TWI0.MCTRLA = 0;												// disable TWI Master

   PORTB.DIRCLR = PIN1_bm;  //PORTB_set_pin_dir(1, PORT_DIR_IN);
	for (i = 0; i < 9; i++)											// SCL, 9 x bit-banging
	{
      PORTB.DIRSET = PIN0_bm;  //PORTB_set_pin_dir(0, PORT_DIR_OUT);
		_delay_us(5);
      PORTB.DIRCLR = PIN0_bm;  //PORTB_set_pin_dir(0, PORT_DIR_IN);
		_delay_us(5);
	}
	
// re-enable master twice
// for unknown reasons the master might get stuck if re-enabled only once
// second re-enable will fail if SDA not enabled beforehand

	TWI0.MCTRLB = TWI_FLUSH_bm;										// clear the internal state of the master
	TWI0.MCTRLA =	  1 << TWI_ENABLE_bp							// Enable TWI Master: enabled
					| 0 << TWI_QCEN_bp								// Quick Command Enable: disabled
					| 0 << TWI_RIEN_bp								// Read Interrupt Enable: disabled
					| 0 << TWI_SMEN_bp								// Smart Mode Enable: disabled
					| TWI_TIMEOUT_DISABLED_gc						// Bus Timeout Disabled (inoperative, see errata)
					| 0 << TWI_WIEN_bp;								// Write Interrupt Enable: disabled			

   PORTB.DIRSET = PIN1_bm;  //PORTB_set_pin_dir(1, PORT_DIR_OUT);	
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;							// force bus idle
	TWI0.MSTATUS |= (TWI_RIF_bm | TWI_WIF_bm | TWI_BUSERR_bm);		// clear flags

	TWI0.MCTRLB = TWI_FLUSH_bm;										// clear the internal state of the master (glitch on SDA)
	TWI0.MCTRLA =	  1 << TWI_ENABLE_bp							// Enable TWI Master: enabled
					| 0 << TWI_QCEN_bp								// Quick Command Enable: disabled
					| 0 << TWI_RIEN_bp								// Read Interrupt Enable: disabled
					| 0 << TWI_SMEN_bp								// Smart Mode Enable: disabled
					| TWI_TIMEOUT_DISABLED_gc						// Bus Timeout Disabled (inoperative, see errata)
					| 0 << TWI_WIEN_bp;								// Write Interrupt Enable: disabled

   PORTB.DIRSET = PIN0_bm;  //PORTB_set_pin_dir(0, PORT_DIR_OUT);

	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;							// force bus idle
	TWI0.MSTATUS |= (TWI_RIF_bm | TWI_WIF_bm | TWI_BUSERR_bm);		// clear flags
}

/*!
 * @brief Execute the I2C start message
 * 
 * @param device_addr
 *  address of device to be contacted
 *
 * @return 0 in all cases
 */
uint8_t I2C_start(uint8_t device_addr)								// device_addr LSB set if READ
{
	TWI0.MSTATUS |= (TWI_RIF_bm | TWI_WIF_bm);						// clear Read and Write interrupt flags
	if (TWI0.MSTATUS & TWI_BUSERR_bm) return 4;						// Bus Error, abort
	TWI0.MADDR = device_addr;
	return 0;
}

/*!
 * @brief Block until either a timeout occurs or an ACK is received
 * 
 * @return 0 if ACK was received, else 0xFF for timeout
 */
uint8_t I2C_wait_ACK(void)											// wait for slave response after start of Master Write
{
	timeout_cnt = 0;												// reset timeout counter, will be incremented by ms tick interrupt
	while (!(TWI0.MSTATUS & TWI_RIF_bm) && !(TWI0.MSTATUS & TWI_WIF_bm))	// wait for RIF or WIF set
	{
		if (timeout_cnt > ADDR_TIMEOUT) return 0xff;				// return timeout error
	}
	TWI0.MSTATUS |= (TWI_RIF_bm | TWI_WIF_bm);						// clear Read and Write interrupt flags
	if (TWI0.MSTATUS & TWI_BUSERR_bm) return 4;						// Bus Error, abort
	if (TWI0.MSTATUS & TWI_ARBLOST_bm) return 2;					// Arbitration Lost, abort
	if (TWI0.MSTATUS & TWI_RXACK_bm) return 1;						// Slave replied with NACK, abort
	return 0;														// no error
}

// the Atmel device documentation mentions a special command for repeated start TWI_MCMD_REPSTART_gc,
// but this is not used in Atmel's demo code, so we don't use it either

/*!
 * @brief Execute I2C repeated-start message
 *
 * @param device_addr
 *  The address of the device to be contacted
 *
 * @return None
 */
void I2C_rep_start(uint8_t device_addr)								// send repeated start, device_addr LSB set if READ
{
	TWI0.MADDR = device_addr;	
}

/*! 
 * @brief read data into pointer over I2C
 * 
 * @param data
 *  A pointer to a buffer to store read data
 *
 * @param ack_flag
 *  0 means send an ACK after reading, 1 means send a NACK
 *
 * @return 0 for success, else 8 for bus contention or 0xFF for timeout
 */
uint8_t	I2C_read(uint8_t *data, uint8_t ack_flag)					// read data, ack_flag 0: send ACK, 1: send NACK, returns status
{
	timeout_cnt = 0;												// reset timeout counter, will be incremented by ms tick interrupt
	if ((TWI0.MSTATUS & TWI_BUSSTATE_gm) == TWI_BUSSTATE_OWNER_gc)	// if master controls bus
	{		
		while (!(TWI0.MSTATUS & TWI_RIF_bm))						// wait for RIF set (data byte received)
		{
			if (timeout_cnt > READ_TIMEOUT) return 0xff;			// return timeout error
		}
		TWI0.MSTATUS |= (TWI_RIF_bm | TWI_WIF_bm);					// clear Read and Write interrupt flags	
		if (TWI0.MSTATUS & TWI_BUSERR_bm) return 4;					// Bus Error, abort
		if (TWI0.MSTATUS & TWI_ARBLOST_bm) return 2;				// Arbitration Lost, abort
		if (TWI0.MSTATUS & TWI_RXACK_bm) return 1;					// Slave replied with NACK, abort				
		if (ack_flag == 0) TWI0.MCTRLB &= ~(1 << TWI_ACKACT_bp);	// setup ACK
		else		TWI0.MCTRLB |= TWI_ACKACT_NACK_gc;				// setup NACK (last byte read)
		*data = TWI0.MDATA;
		if (ack_flag == 0) TWI0.MCTRLB |= TWI_MCMD_RECVTRANS_gc;	// send ACK, more bytes to follow					
		return 0;
	}
	else return 8;													// master does not control bus
}

/*! 
 * @brief write data at pointer over I2C
 * 
 * @param data
 *  A pointer to null-terminated data to send
 *
 * @return 0 for success, else 8 for bus contention, 0xFF for timeout, 4 for bus error, or 1 for NACK
 */
uint8_t I2C_write(uint8_t *data)									// write data, return status
{
	timeout_cnt = 0;												// reset timeout counter, will be incremented by ms tick interrupt
	if ((TWI0.MSTATUS & TWI_BUSSTATE_gm) == TWI_BUSSTATE_OWNER_gc)	// if master controls bus
	{
		TWI0.MDATA = *data;		
		while (!(TWI0.MSTATUS & TWI_WIF_bm))						// wait until WIF set, status register contains ACK/NACK bit
		{
			if (timeout_cnt > WRITE_TIMEOUT) return 0xff;			// return timeout error
		}
		if (TWI0.MSTATUS & TWI_BUSERR_bm) return 4;					// Bus Error, abort
		if (TWI0.MSTATUS & TWI_RXACK_bm) return 1;					// Slave replied with NACK, abort
		return 0;													// no error	
	}
	else return 8;													// master does not control bus
}

/*!
 * @brief Execute the stop I2C message
 *
 * @return None
 */
void I2C_stop()
{
	TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
}

/*! 
 * @brief Convenience function to read several bytes from an address on a specified device
 * 
 * @param slave_addr
 *  Address of the device to be read from
 *
 * @param addr_ptr
 *  pointer to the buffer where read bytes should be received
 *
 * @param slave_reg
 *  register on device to be read from
 *
 * @param num_bytes
 *  the number of bytes to read
 *
 * @return 0 for success, else 0xFF for timeout or 1 for NACK
 */
uint8_t	I2C_read_bytes(uint8_t slave_addr, uint8_t *addr_ptr, uint8_t slave_reg, uint8_t num_bytes)
{
	uint8_t status;
	if (num_bytes > MAX_LEN) num_bytes = MAX_LEN;
	status = I2C_start(slave_addr & 0xfe);							// slave write address, LSB 0
	if (status != 0) goto error;
	status = I2C_wait_ACK();										// wait for slave ACK
	if (status == 1) {
		I2C_stop();													// NACK, abort
		return 1;
	}
	if (status != 0) goto error;
	status = I2C_write(&slave_reg);									// send slave start register
	if (status != 0) goto error;
	I2C_rep_start((slave_addr & 0xfe) + 1);							// slave read address, LSB 1
	while (num_bytes > 1) {
		status = I2C_read(addr_ptr, 0);								// first bytes, send ACK
		if (status != 0) goto error;
		addr_ptr++;
		num_bytes--;
	}
	status = I2C_read(addr_ptr, 1);									// single or last byte, send NACK
	if (status != 0) goto error;
	I2C_stop();
	return 0;
	
error:
	I2C_recover();													// clock out possibly stuck slave, reset master
	return 0xff;													// flag error
}

/*! 
 * @brief Convenience function to write several bytes to an address on a specified device
 * 
 * @param slave_addr
 *  Address of the device to be written to
 *
 * @param addr_ptr
 *  pointer to the buffer of bytes to be written
 *
 * @param slave_reg
 *  register on device to be written to
 *
 * @param num_bytes
 *  the number of bytes to write
 *
 * @return 0 for success, else 0xFF for timeout or 1 for NACK
 */
uint8_t	I2C_write_bytes(uint8_t slave_addr, uint8_t *addr_ptr, uint8_t slave_reg, uint8_t num_bytes)
{
	uint8_t status;
	if (num_bytes > MAX_LEN) num_bytes = MAX_LEN;
	status = I2C_start(slave_addr & 0xfe);							// slave write address, LSB 0
	if (status != 0) goto error;
	status = I2C_wait_ACK();										// wait for Slave ACK
	if (status == 1) {
		I2C_stop();													// NACK, abort	
		return 1;													
	}	 										
	if (status != 0) goto error;
	status = I2C_write(&slave_reg);
	if (status != 0) goto error;
	while (num_bytes > 0) {											// write bytes
		status = I2C_write(addr_ptr);
		if (status != 0) goto error;
		addr_ptr++;
		num_bytes--;		
	}
	I2C_stop();
	return 0;

error:
	I2C_recover();
   ledUsrBlink(0, 100);
	return 0xff;
}