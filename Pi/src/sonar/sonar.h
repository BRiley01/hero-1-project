#pragma once
#include <string>
#include <vector>
#include "controller.h"

#define SONAR_UNDEFINED_DISTANCE 0xFFFFFFFF

class Sonar: Controller
{
	public:
		Sonar(int I2C_Address);
		~Sonar();
		bool Enable();
		bool Disable();
		unsigned long Distance();		
};
