#pragma once
#include <string>
#include <vector>
#include "controller.h"

#define DRIVE_OPCODE_STATUS 0x00
#define DRIVE_OPCODE_WHEEL_POSTITION 0x01
#define DRIVE_OPCODE_DRIVE 0x02
#define DRIVE_OPCODE_RECALIBRATE 0x03

#define DRIVE_STATUS_READY 0x01
#define DRIVE_STATUS_MOVING_NONBLOCKED 0x02
#define DRIVE_STATUS_MOVING 0xF2
#define DRIVE_STATUS_RECALIBRATING 0x03
#define DRIVE_STATUS_FAULT 0xFF

class DriveMotor: Controller
{
	public:
		DriveMotor(int I2C_Address);
		~DriveMotor();
		bool Recalibrate();
		bool Drive(bool forward, int speed, long millis);
		bool Turn(int angle);
		
		
		int Status(unsigned char* data, int len);
};
