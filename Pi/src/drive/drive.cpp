#include <cstddef>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "drive.h"

#define OPCODE_STATUS 0x00
#define OPCODE_WHEEL_POSTITION 0x01
#define OPCODE_DRIVE 0x02
#define OPCODE_RECALIBRATE 0x03

using namespace std;
DriveMotor::DriveMotor(int I2C_Address) : Controller(I2C_Address)
{
#ifdef DEBUG
	cout << "Drive motor constructor called" << endl;
#endif	
}

DriveMotor::~DriveMotor()
{
#ifdef DEBUG
	cout << "DriveMotor deconstructor called" << endl;
#endif	
}

bool DriveMotor::Recalibrate()
{
	if(!sendOpCode(OPCODE_RECALIBRATE))
	{
#ifdef DEBUG
		cout << "Status request failed\n";
#endif	
		return false;
	}
	return true;
}

bool DriveMotor::Drive(bool forward, int speed, long millis)
{
}

bool DriveMotor::Turn(int angle)
{
	char buff[2];
	if(!sendOpCode(OPCODE_WHEEL_POSTITION))
	{
#ifdef DEBUG
		cout << "Status request failed\n";
#endif	
		return false;
	}
	buff[0] = (angle < 0)?0:1;
	buff[1] = abs(angle);
	return write(buff, 2) == 2;
}

int DriveMotor::Status(unsigned char* data, int len)
{
	unsigned char rxBuffer[5];	//	receive buffer
	if(!data || len == 0)
	{
		data = rxBuffer;
		len = 5;
	}
	memset(data, 0, sizeof(unsigned char) * len);
	if(!sendOpCode(OPCODE_STATUS))
	{
#ifdef DEBUG
		cout << "Status request failed\n";
#endif	
		return STATUS_FAULT;
	}
	read(data, len);
	if(*data != OPCODE_STATUS)
	{
#ifdef DEBUG
		cout << "Unexpected Response OPCODE: " << hex << (int)*data << dec << " len: " << len << "\n";
#endif	
		Abort();
		return STATUS_FAULT;
	}
	/*else
		cout << hex << (int)*(data+1);*/
		
	return *(data+1);
}

