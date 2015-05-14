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

#ifndef _ORF_24_H_
#define _ORF_24_H_

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "nRF24L01.h"

class ORF24
{
private:
	int ce;							/* CE pin number */
	int csn;						/* CSN pin number */
	int spiChannel;					/* Odroid SPI channel */
	int spiSpeed;					/* SPI clock frequency in Hz */		
	int payloadSize;				/* nRF24L01 payload size */
	unsigned char buffer[32];		/* RX and TX buffer */

protected:
	/**
	 * Read from nRF24L01 register
	 * 
	 * @param  	reg 	register address
	 * @return     		read register value
	 */
	unsigned char readRegister(unsigned char reg);

	/**
	 * Write to nRF24L01 register
	 * 
	 * @param  reg   	register address
	 * @param  value 	value to write
	 * @return       	nRF24L01 status
	 */
	unsigned char writeRegister(unsigned char reg, unsigned char value);

	/**
	 * Write payload to send
	 * 
	 * @param  data 	data to send
	 * @param  len  	data length in byte
	 * @return      	nRF24L01 status
	 */
	unsigned char writePayload(unsigned char *data, int len);

	/**
	 * Read received payload
	 * 
	 * @param  data 	data buffer to read into
	 * @param  len  	data length
	 * @return     		nRF24L01 status
	 */
	unsigned char readPayload(unsigned char *data, int len);

public:
	/**
	 * ORF24 Constructor
	 */
	ORF24(int _ce);

	/**
	 * ORF24 Constructor with SPI options
	 */
	ORF24(int _ce, int _spiChannel, int spiSpeed);

	/**
	 * nRF24L01 Initialization
	 * 
	 * @return  status
	 */
	bool begin(void);
};

#endif