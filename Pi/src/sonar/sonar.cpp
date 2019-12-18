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
#include "sonar.h"

//#define OPCODE_DISTANCE_REQ 0x00
#define OPCODE_ENABLE       0x01
#define OPCODE_DISABLE      0x02

using namespace std;
Sonar::Sonar(int I2C_Address) : Controller(I2C_Address)
{
#ifdef DEBUG
	cout << "Sonar constructor called" << endl;
#endif	
}

Sonar::~Sonar()
{
#ifdef DEBUG
	cout << "Sonar deconstructor called" << endl;
#endif	
}

bool Sonar::Enable()
{
	if(!sendOpCode(OPCODE_ENABLE))
	{
#ifdef DEBUG
		cout << "Sonar enable request failed\n";
#endif	
		return false;
	}
	return true;
}

bool Sonar::Disable()
{
	if(!sendOpCode(OPCODE_DISABLE))
	{
#ifdef DEBUG
		cout << "Sonar enable request failed\n";
#endif	
		return false;
	}
	return true;
}

unsigned long Sonar::Distance()
{
	unsigned long micros = SonarDelay();
	if (micros <= 10)
		return (35 * micros) + 2300;
	else
		return (160 * micros) + 980;
}

unsigned long Sonar::SonarDelay()
{
	unsigned long result;
	if(read(&result, 4) != sizeof(result))
		throw runtime_error("Distance request failed: unexpected response!");
	return result;	
}


