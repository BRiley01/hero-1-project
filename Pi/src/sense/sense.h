#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>
#include "controller.h"

#define I2C_RETRY 5
#define SENSE_OPCODE_STATUS 0x00
#define SENSE_OPCODE_STREAM_LIGHT 0x01
#define SENSE_OPCODE_STREAM_SOUND 0x02
#define SENSE_OPCODE_STOP_STREAM 0xFE

enum SENSE_MODE { OFF = 0x00, LIGHT = 0x01, SOUND = 0x02, FAULT = 0xFF };

class Sense : Controller
{
	public:
		Sense(int I2C_Address);
		~Sense();
		bool SetMode(SENSE_MODE mode);
		unsigned char Sample();
		SENSE_MODE Status();
};
