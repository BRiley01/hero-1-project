#include <cstddef>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <wiringPi.h>
#include "sense.h"

using namespace std;
Sense::Sense()
{
#ifdef DEBUG
	cout << "Sense constructor called" << endl;
#endif	
	wiringPiSetup();
	pinMode(7, GPIO_CLOCK);
	gpioClockSet(7, 1000000) ;
	

	int tenBitAddress = 0;
	int opResult = 0;

	// Create a file descriptor for the I2C bus
	_i2cHandle = open("/dev/i2c-1", O_RDWR); 

	// I2C device is not 10-bit
	opResult = ioctl(_i2cHandle, I2C_TENBIT, tenBitAddress);
#ifdef DEBUG
	cout << "Set I2C to non 10-bit mode result: " << opResult << endl;
#endif		
	
	// set address of speech board to I2C
	opResult = ioctl(_i2cHandle, I2C_SLAVE, I2C_SENSE);
#ifdef DEBUG
	cout << "Set I2C Sense board address result: " << opResult << endl;
#endif		
}

Sense::~Sense()
{
#ifdef DEBUG
	cout << "Sense deconstructor called" << endl;
#endif	
	
#ifdef DEBUG
	cout << "Freeing Sense I2C handle" << endl;
#endif	
	close(_i2cHandle);
}

bool Sense::sendOpCode(int opcode) 
{
	int opResult = 0;
	opResult = write(&opcode, 1);
	if(opResult != 1)
	{ 
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif			
	}	
}

SENSE_MODE Sense::Status()
{
	const int len = 2;
	unsigned char data[len];	//	receive buffer
	memset(data, 0, sizeof(unsigned char) * len);
	if(!sendOpCode(SENSE_OPCODE_STATUS))
	{
#ifdef DEBUG
		cout << "Status request failed\n";
#endif	
		return FAULT;
	}
	//usleep(1000); //sleep for 1 millisecond
	read(data, len);
	if(*data != SENSE_OPCODE_STATUS)
	{
#ifdef DEBUG
		cout << "Unexpected Response OPCODE: " << hex << (int)*data << dec << " len: " << len << "\n";
#endif	
		return FAULT;
	}
	/*else
		cout << hex << (int)*(data+1);*/
		
	return (SENSE_MODE)(*(data+1));
}

ssize_t Sense::read(void* buf, size_t count)
{
	int tries = 0;
	int resp = -1;
	while(tries < I2C_RETRY && resp != count)
	{
		resp = ::read(_i2cHandle, buf, count);
		tries++;
	}
#ifdef DEBUG
	if(tries == I2C_RETRY)
		cout << "Max Read Attempt Exceeded!\n";
#endif	
	return resp;
}
		
ssize_t Sense::write(void* buf, size_t count)
{
	int tries = 0;
	int resp = -1;
	while(tries < I2C_RETRY && resp != count)
	{
		resp = ::write(_i2cHandle, buf, count);
		tries++;
	}
#ifdef DEBUG
	if(tries == I2C_RETRY)
		cout << "Max Write Attempt Exceeded!\n";
#endif	
	return resp;
}

bool Sense::SetMode(SENSE_MODE mode)
{
	return sendOpCode(mode);
}

unsigned char Sense::Sample()
{
	static size_t count = 1;
	unsigned char buf[count];
	read(buf, count);
	return buf[0];
}
