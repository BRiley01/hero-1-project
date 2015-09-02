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
#include "controller.h"

using namespace std;
Controller::Controller(int I2C_Address)
{
	int opResult;
#ifdef DEBUG
	cout << "Controller constructor called for address: " << I2C_Address << endl;
	opResult = 0;
#endif	
	int tenBitAddress = 0;
	

	// Create a file descriptor for the I2C bus
	_i2cHandle = open("/dev/i2c-1", O_RDWR); 

	// I2C device is not 10-bit
	opResult = ioctl(_i2cHandle, I2C_TENBIT, tenBitAddress);
#ifdef DEBUG
	cout << "Set I2C to non 10-bit mode result: " << opResult << endl;
#endif		
	
	// set address of speech board to I2C
	opResult = ioctl(_i2cHandle, I2C_SLAVE, I2C_Address);
#ifdef DEBUG
	cout << "Set board address result: " << opResult << endl;
#endif		
}

Controller::~Controller()
{
#ifdef DEBUG
	cout << "Freeing I2C handle" << endl;
#endif	
	close(_i2cHandle);
}

bool Controller::sendOpCode(int opcode) 
{
	int opResult = 0;
	opResult = write(&opcode, 1);
	if(opResult != 1)
	{ 
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif	
		return false;
	}	
	return true;
}


void Controller::Abort()
{
#ifdef DEBUG
	cout << "ABORT CALLED\n";
#endif
	
	if(!sendOpCode(OPCODE_ABORT))
	{
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif			
	}
}

ssize_t Controller::read(void* buf, size_t count)
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
		
ssize_t Controller::write(void* buf, size_t count)
{
	int tries = 0;
	int resp = -1;
#ifdef DEBUG
	cout << "writing: " << buf << " to: " << _i2cHandle << endl;
#endif	
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


