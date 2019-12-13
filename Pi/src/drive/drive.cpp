#include <cstddef>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <stdexcept>
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

bool DriveMotor::Drive(bool forward, int speed, unsigned long distance)
{
    char* block = (char*)&distance;
	char buff[7];
	buff[0] = OPCODE_DRIVE;
	buff[1] = forward;
	buff[2] = speed;
    buff[3] = block[3];
    buff[4] = block[2];
    buff[5] = block[1];
    buff[6] = block[0];
	return write(buff, 7) == 7;
}

bool DriveMotor::Turn(int angle)
{
	char buff[3];
	buff[0] = OPCODE_WHEEL_POSTITION;
	buff[1] = angle + 90;
	return write(buff, 2) == 2;
}

t_DRIVE_STATUS DriveMotor::Status()
{
	t_DRIVE_STATUS result;
	memset(&result, 0, sizeof(result));
	if(!sendOpCode(OPCODE_STATUS))
		throw runtime_error("Status request failed: error sending opcode");
	
	if(read(&result, sizeof(result)) != sizeof(result))
	{
		Abort();
	
		throw runtime_error("Status request failed: unexpected response!");
	}
	//result.Wheel.Angle -= 90;
	//result.Wheel.DestAngle -= 90;
	return result;
}

