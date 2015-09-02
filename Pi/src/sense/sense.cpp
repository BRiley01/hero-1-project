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
Sense::Sense(int I2C_Address) : Controller(I2C_Address)
{
#ifdef DEBUG
	cout << "Sense constructor called" << endl;
#endif	
	wiringPiSetup();
	pinMode(7, GPIO_CLOCK);
	gpioClockSet(7, 1000000);
}

Sense::~Sense()
{
#ifdef DEBUG
	cout << "Sense deconstructor called" << endl;
#endif	
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
