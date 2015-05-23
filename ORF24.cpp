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
 * nRF24L01 Initialization
 * 
 * @return  status
 */
bool ORF24::begin(void)
{
	if (debug)
	{
		std::cout << "Setting up SPI Communication Controller...\n";
	}

	/* Settipng up CE pin */
	pinMode(ce, OUTPUT);
	digitalWrite(ce, LOW);

	/* Initializing SPI communication */
	wiringPiSPISetup(spiChannel, spiSpeed);

	/* Pulldown MOSI and SCK pin */
	pullUpDnControl(MOSI_PIN, PUD_DOWN);
	pullUpDnControl(SLCK_PIN, PUD_DOWN);

	if (debug)
	{
		std::cout << "SPI communication initialized.\n";
	}

	delay(100);

	if (debug)
	{
		std::cout << "Setting up nRF24L01...\n";
	}

	/* Setting up nRF24L01 configuration */
	setRetries(0b0100, 0b1111);
	setPowerLevel(RF_PA_MIN);
	setDataRate(RF_DR_1MBPS);
	setCRCLength(CRC_1_BYTE);
	writeRegister(DYNPD, 0);
	writeRegister(STATUS, (1 < RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
	setChannel(0);

	flushRX();
	flushTX();

	if (debug)
	{
		std::cout << "nRF24L01 initialized.\n\n";
	}

	return true;
}

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
 * Read multibyte from nRF24L01 register
 * 
 * @param  	reg 	register address
 * @param 	buf 	read buffer
 * @param 	len 	data length to read
 * @return     		read register value
 */
unsigned char ORF24::readRegister(unsigned char reg, unsigned char *buf, int len)
{
	unsigned char *p = buffer;

	*p++ = (R_REGISTER | (RW_MASK & reg));

	for (int i = 0; i < len; i++)
	{
		*p++ = NOP;
	}

	wiringPiSPIDataRW(spiChannel, buffer, len + 1);

	p = buffer + 1;
	for (int i = 0; i < len; i++)
	{
		*buf++ = *p++;
	}

	return *buffer;
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

	*p++ = (W_REGISTER | (RW_MASK & reg));		/* Set SPI command to write register */
	*p = value;									/* Set data to write */

	wiringPiSPIDataRW(spiChannel, buffer, 2);		/* Start write register */

	return *buffer;								/* Status is the first byte of receive buffer */
}

/**
 * Write multibyte to nRF24L01 register
 * 
 * @param  reg   	register address
 * @param  buf 		write buffer
 * @param  len 		data length to write
 * @return       	nRF24L01 status
 */
unsigned char ORF24::writeRegister(unsigned char reg, const unsigned char *buf, int len)
{
	unsigned char *p = buffer;

	*p++ = (W_REGISTER | (RW_MASK & reg));

	for (int i = 0; i < len; i++)
	{
		*p++ = *buf++;
	}

	wiringPiSPIDataRW(spiChannel, buffer, len + 1);

	return *buffer;
}
/**
 * Write payload to send
 * 
 * @param  data 	data to send
 * @param  len  	data length in byte
 * @return      	nRF24L01 status
 */
unsigned char ORF24::writePayload(unsigned char *data, int len)
{
	unsigned char *p = buffer;

	*p++ = W_TX_PAYLOAD;
	
	for (int i = 0; i < len; i++)
	{
		*p++ = *data++;
	}

	wiringPiSPIDataRW(spiChannel, buffer, len + 1);

	return *buffer;
}

/**
 * Set delay and number of retry for retransmission
 *
 * @param delay 	retransmission delay
 * @param count 	retransmission count
 */
void ORF24::setRetries(int delay, int count)
{
	if (debug)
	{
		std::cout << "Setting up retransmission configuration...\n";
	}

	writeRegister(SETUP_RETR, (delay & 0xF) << ARD | (count & 0xF) << ARC);
}

/**
 * Set RF channel
 * 
 * @param channel 	channel number
 */
void ORF24::setChannel(int channel)
{
	if (debug)
	{
		std::cout << "Setting up RF channel...\n";
	}

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
	if (debug)
	{
		std::cout << "Setting up payload size to " << size << "...\n";
	}

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
	if (debug)
	{
		std::cout << "Setting up RF power level...\n";
	}

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
	if (debug)
	{
		std::cout << "Setting up air data rate...\n";
	}

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
	if (debug)
	{
		std::cout << "Setting up CRC...\n";
	}

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

/**
 * Set auto acknowledgment
 * 
 * @param enable 	enable or disable auto acknowledgment
 */
void ORF24::setAutoACK(bool enable)
{
	if (enable)
	{
		if (debug)
		{
			std::cout << "Enabling Auto Acknowledgment...\n";
		}

		writeRegister(EN_AA, 0b111111);
	}
	else
	{
		if (debug)
		{
			std::cout << "Disabling Auto Acknowledgment...\n";
		}

		writeRegister(EN_AA, 0);
	}
}

/**
 * Set auto acknowledgment
 * 
 * @param pipe   	pipe number
 * @param enable 	enable or disable auto acknowledgment
 */
void ORF24::setAutoACK(int pipe, bool enable)
{
	if (pipe < 6)
	{
		unsigned char aa = readRegister(EN_AA);

		if (enable)
		{
			if (debug)
			{
				std::cout << "Enabling Auto Acknowledgment on pipe " << pipe << "...\n";
			}

			aa |= (1 << pipe);
		}
		else
		{
			if (debug)
			{
				std::cout << "Disabling Auto Acknowledgment on pipe " << pipe << "...\n";
			}

			aa &= ~(1 << pipe);
		}

		writeRegister(EN_AA, aa);
	}
}

/**
 * Flush RX FIFO
 * 
 * @return  status
 */
unsigned char ORF24::flushRX(void)
{
	if (debug)
	{
		std::cout << "Flushing RX FIFO...\n";
	}

	unsigned char *p = buffer;

	*p = FLUSH_RX;

	wiringPiSPIDataRW(spiChannel, buffer, 1);

	return *buffer;
}

/**
 * Flush TX FIFO
 * 
 * @return  status
 */
unsigned char ORF24::flushTX(void)
{
	if (debug)
	{
		std::cout << "Flushing TX FIFO...\n";
	}

	unsigned char *p = buffer;

	*p = FLUSH_TX;

	wiringPiSPIDataRW(spiChannel, buffer, 1);

	return *buffer;
}

/**
 * Write payload to open writing pipe
 * 
 * @param  data 	data to write
 * @param  len  	data length
 * @return      	status
 */
bool ORF24::write(unsigned char *data, int len)
{
	bool result = false;

	if (debug)
	{
		std::cout << "\nSending payload...\n";
		printAddressRegister("RX Address", RX_ADDR_P0, true);
		printRegister("RF Channel", RF_CH);
	}

	startWrite(data, len);

	unsigned char observeTX, status;
	int sentAt = millis();
	const unsigned long timeout = 500;

	do
	{
		status = readRegister(OBSERVE_TX, &observeTX, 1);
	} while (! (status & (1 << TX_DS | 1 << MAX_RT)) && (millis() - sentAt < timeout));

	bool txOK, txFail, rxReady;

	status = writeRegister(STATUS, 1 << RX_DR | 1 << TX_DS | 1 << MAX_RT);
	txOK = status & (1 << TX_DS);
	txFail = status & (1 << MAX_RT);
	rxReady = status & (1 << RX_DR);

	result = txOK;

	if (debug)
	{
		if (result)
		{
			std::cout << "\nSending payload success.\n";
		}
		else
		{
			std::cout <<  "\nSending payload failed.\n";
		}

		printRegister("OBSERVE_TX", OBSERVE_TX);
		printRegister("STATUS", STATUS);
		std::cout << std::endl;
	}

	powerDown();

	flushTX();

	return result;
}

/**
 * Start writing payload
 * 
 * @param data 	data to write
 * @param len  	data length
 */
void ORF24::startWrite(unsigned char *data, int len)
{
	unsigned char config = readRegister(CONFIG);
	config |= (1 << PWR_UP);
	config &= ~(1 << PRIM_RX);
	writeRegister(CONFIG, config);

	if (debug)
		printRegister("CONFIG", CONFIG);

	delayMicroseconds(150);

	writePayload(data, len);

	digitalWrite(ce, HIGH);
	delayMicroseconds(15);
	digitalWrite(ce, LOW);
}

/**
 * Set nRF24L01 to standby mode
 */
void ORF24::powerUp(void)
{
	unsigned char config = readRegister(CONFIG);

	config |= (1 << PWR_UP);

	if (debug)
	{
		std::cout << "Setting nRF24L01 to Standby-I mode...\n";
	}

	writeRegister(CONFIG, config);
}

/**
 * Set nRF24L01 to power down mode
 */
void ORF24::powerDown(void)
{
	unsigned char config = readRegister(CONFIG);

	config &= ~(1 << PWR_UP);

	if (debug)
	{
		std::cout << "Setting nRF24L01 to Power Down mode...\n";
	}

	writeRegister(CONFIG, config);
}

/**
 * Open writing pipe
 * 
 * @param address 	pipe address
 */
void ORF24::openWritingPipe(const char *address)
{
	unsigned char *tmp = (unsigned char *) address;

	unsigned char setupAW = readRegister(SETUP_AW);

	int addressSize;
	switch (setupAW)
	{
		case 0b01:
			addressSize = 3;
			break;

		case 0b10:
			addressSize = 4;
			break;

		case 0b11:
		default:
			addressSize = 5;
	}

	if (debug)
	{
		std::cout << "Opening writing pipe with address \"" << address << "\"...\n";
	}

	unsigned char addr[addressSize];

	for (int i = 0; i < addressSize; i++)
	{
		addr[i] = tmp[addressSize - 1 - i];
	}

	writeRegister(RX_ADDR_P0, addr, 5);
	writeRegister(TX_ADDR, addr, 5);

	const int maxPayloadSize = 32;
	writeRegister(RX_PW_P0, maxPayloadSize > payloadSize ? payloadSize : maxPayloadSize);
}

/**
 * Enable debugging information
 */
void ORF24::enableDebug(void)
{
	debug = true;

	if (debug)
	{
		std::cout << "Debug is enabled.\n";
	}
}

/**
 * Print register value
 *
 * @param  name 	register name
 * @param  reg 		register address	
 */
void ORF24::printRegister(std::string name, unsigned char reg)
{
	std::cout << name << "\t";

	if (name.length() < 8)
		std::cout << "\t";

	unsigned char value = readRegister(reg);
	printf("0x%02X\n", value);
}

/**
 * Print address register
 *
 * @param  name 	register name
 * @param  reg 		register address	
 */
void ORF24::printAddressRegister(std::string name, unsigned char reg)
{
	printAddressRegister(name, reg, false);
}

/**
 * Print address register as string
 *
 * @param  name 	register name
 * @param  reg 		register address
 * @param  str 		print as string
 */
void ORF24::printAddressRegister(std::string name, unsigned char reg, bool str)
{
	std::cout << name << ":\t";

	if (name.length() < 8)
		std::cout << "\t";

	unsigned char buffer[5];
	readRegister(reg, buffer, sizeof(buffer));

	if (!str)
	{
		printf("0x");
	}

	unsigned char *p = buffer + sizeof(buffer);

	while (--p >= buffer)
	{
		if (str)
			printf("%c", *p);
		else
			printf("%02X", *p);
	}

	printf("\r\n");
}

/**
 * Print all register value
 */
void ORF24::printAllRegister(void)
{
	std::cout << "\nREGISTER VALUES\n";

	printRegister("CONFIG", CONFIG);
	printRegister("EN_AA", EN_AA);
	printRegister("EN_RXADDR", EN_RXADDR);
	printRegister("SETUP_AW", SETUP_AW);
	printRegister("SETUP_RETR", SETUP_RETR);
	printRegister("RF_CH", RF_CH);
	printRegister("RF_SETUP", RF_SETUP);
	printRegister("STATUS", STATUS);
	printRegister("OBSERVE_TX", OBSERVE_TX);
	printRegister("CD", CD);
	printAddressRegister("RX_ADDR_P0", RX_ADDR_P0);
	printAddressRegister("RX_ADDR_P1", RX_ADDR_P1);
	printRegister("RX_ADDR_P2", RX_ADDR_P2);
	printRegister("RX_ADDR_P3", RX_ADDR_P3);
	printRegister("RX_ADDR_P4", RX_ADDR_P4);
	printRegister("RX_ADDR_P5", RX_ADDR_P5);
	printAddressRegister("TX_ADDR", TX_ADDR);
	printRegister("RX_PW_P0", RX_PW_P0);
	printRegister("RX_PW_P1", RX_PW_P1);
	printRegister("RX_PW_P2", RX_PW_P2);
	printRegister("RX_PW_P3", RX_PW_P3);
	printRegister("RX_PW_P4", RX_PW_P4);
	printRegister("RX_PW_P5", RX_PW_P5);
	printRegister("FIFO_STATUS", FIFO_STATUS);
	printRegister("DYNPD", DYNPD);
	printRegister("FEATURE", FEATURE);

	std::cout << std::endl;
}