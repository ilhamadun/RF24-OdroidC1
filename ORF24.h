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

#include <iostream>
#include <string>
#include <cstdio>
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
	bool debug = false;

protected:
	
	/**
	 * Read one byte from nRF24L01 register
	 * 
	 * @param  	reg 	register address
	 * @return     		read register value
	 */
	unsigned char readRegister(unsigned char reg);

	/**
	 * Read multibyte from nRF24L01 register
	 * 
	 * @param  	reg 	register address
	 * @param 	buf 	read buffer
	 * @param 	len 	data length to read
	 * @return     		read register value
	 */
	unsigned char readRegister(unsigned char reg, unsigned char *buf, int len);

	/**
	 * Write one byte to nRF24L01 register
	 * 
	 * @param  reg   	register address
	 * @param  value 	value to write
	 * @return       	nRF24L01 status
	 */
	unsigned char writeRegister(unsigned char reg, unsigned char value);

	/**
	 * Write multibyte to nRF24L01 register
	 * 
	 * @param  reg   	register address
	 * @param  buf 		write buffer
	 * @param  len 		data length to write
	 * @return       	nRF24L01 status
	 */
	unsigned char writeRegister(unsigned char reg, const unsigned char *buf, int len);

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

	/**
	 * Flush RX FIFO
	 * 
	 * @return  status
	 */
	unsigned char flushRX(void);

	/**
	 * Flush TX FIFO
	 * 
	 * @return  status
	 */
	unsigned char flushTX(void);

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

	/**
	 * Set delay and number of retry for retransmission
	 *
	 * @param delay 	retransmission delay
	 * @param count 	retransmission count
	 */
	void setRetries(int delay, int count);

	/**
	 * Set RF channel
	 * 
	 * @param channel 	channel number
	 */
	void setChannel(int channel);

	/**
	 * Set payload size
	 * 
	 * @param size 	payload size
	 */
	void setPayloadSize(int size);

	/**
	 * Set power level
	 * 
	 * @param level 	power level
	 */
	void setPowerLevel(RFPower level);

	/**
	 * Set air data rate
	 * 
	 * @param rate 		data rate
	 */
	void setDataRate(DataRate rate);

	/**
	 * Set CRC length
	 * 
	 * @param length 	CRC length
	 */
	void setCRCLength(CRCLength length);

	/**
	 * Set nRF24L01 to standby mode
	 */
	void powerUp(void);

	/**
	 * Set nRF24L01 to power down mode
	 */
	void powerDown(void);

	/**
	 * Enable debugging information
	 */
	void enableDebug(void);

	/**
	 * Print register value
	 *
	 * @param  name 	register name
	 * @param  reg 		register address	
	 */
	void printRegister(std::string name, unsigned char reg);

	/**
	 * Print address register
	 *
	 * @param  name 	register name
	 * @param  reg 		register address	
	 */
	void printAddressRegister(std::string name, unsigned char reg);

	/**
	 * Print all register value
	 */
	void printAllRegister(void);

};

#endif