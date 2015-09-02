#pragma once
#include <string>
#include <vector>

#define STATUS_FAULT 0xFF
#define OPCODE_ABORT 0xFE

#define I2C_RETRY 5


class Controller
{
	protected:
		int _i2cHandle;
		bool sendOpCode(int opcode);
		ssize_t read(void* buf, size_t count);
		ssize_t write(void* buf, size_t count);		
	public:
		Controller(int I2C_Address);
		virtual ~Controller();
		void Abort();
};
