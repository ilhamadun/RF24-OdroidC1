/**
 * Odroid nRF24L01 Library
 *
 * Copyright (c) 2015 Ilham Imaduddin <ilham.imaduddin@mail.ugm.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ORF24.h"

ORF24::ORF24(int _ce)
	: ce(_ce),
	  csn(10),
	  spiChannel(0),
	  spiSpeed(4000000),
	  payloadSize(32)
{ }

ORF24::ORF24(int _ce, int _spiChannel, int _spiSpeed)
	: ce(_ce),
	  csn(spiChannel ? 11 : 10),
	  spiChannel(_spiSpeed),
	  spiSpeed(_spiSpeed),
	  payloadSize(32)
{ }

/**
 * Read from nRF24L01 register
 * 
 * @param  	reg 	register address
 * @return     		read register value
 */
unsigned char ORF24::readRegister(unsigned char reg)
{
	unsigned char *p = buffer;

	*p++ = (R_REGISTER | (RW_MASK & reg)); 		/* Set SPI command to read register */
	*p = NOP;									/* Set dummy data */

	wiringPiSPIDataRW(spiChannel, buffer, 2); 		/* Start read register */

	return *p;									/* Read register value */
}

/**
 * Write to nRF24L01 register
 * 
 * @param  reg   	register address
 * @param  value 	value to write
 * @return       	nRF24L01 status
 */
unsigned char ORF24::writeRegister(unsigned char reg, unsigned char value)
{
	unsigned char *p = buffer;

	*p = (W_REGISTER | (RW_MASK & reg));		/* Set SPI command to write register */
	*p = value;									/* Set data to write */

	wiringPiSPIDataRW(spiChannel, buffer, 2);		/* Start write register */

	return *buffer;								/* Status is the first byte of receive buffer */
}