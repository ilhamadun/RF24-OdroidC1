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

/**
 * Set delay and number of retry for retransmission
 *
 * @param delay 	retransmission delay
 * @param count 	retransmission count
 */
void ORF24::setRetries(int delay, int count)
{
	writeRegister(SETUP_RETR, (delay & 0xF) << ARD | (count & 0xF) << ARC);
}

/**
 * Set RF channel
 * 
 * @param channel 	channel number
 */
void ORF24::setChannel(int channel)
{
	const int max = 127;

	writeRegister(RF_CH, max > channel ? channel : max);
}

/**
 * Set payload size
 * 
 * @param size 	payload size
 */
void ORF24::setPayloadSize(int size)
{
	const int max = 32;
	payloadSize = max > size ? size : max;
}

/**
 * Set power level
 * 
 * @param level 	power level
 */
void ORF24::setPowerLevel(RFPower level)
{
	unsigned char setup = readRegister(RF_SETUP);

	setup &= ~(1 << RF_PWR_LOW | 1 << RF_PWR_HIGH);

	switch (level)
	{
		case RF_PA_MAX:
			setup |= (1 << RF_PWR_LOW | 1 << RF_PWR_HIGH);
			break;

		case RF_PA_HIGH:
			setup |= (1 << RF_PWR_HIGH);
			break;

		case RF_PA_LOW:
			setup |= (1 << RF_PWR_LOW);
			break;
	}

	writeRegister(RF_SETUP, setup);
}

/**
 * Set air data rate
 * 
 * @param rate 		data rate
 */
void ORF24::setDataRate(DataRate rate)
{
	unsigned char setup = readRegister(RF_SETUP);

	setup &= ~(1 << RF_DR);

	if (rate == RF_DR_2MBPS)
	{
		setup |= (1 << RF_DR);
	}

	writeRegister(RF_SETUP, setup);	
}

/**
 * Set CRC length
 * 
 * @param length 	CRC length
 */
void ORF24::setCRCLength(CRCLength length)
{
	unsigned char config = readRegister(CONFIG);

	config &= ~(1 << CRCO | 1 << EN_CRC);

	switch (length)
	{
		case CRC_1_BYTE:
			config |= (1 << EN_CRC);
			break;

		case CRC_2_BYTE:
			config |= (1 << EN_CRC) | (1 << CRCO);
			break;
	}

	writeRegister(CONFIG, config);
}