#include <iostream>
#include "../sense/sense.h"

#define BUFFER_SIZE 5
#define I2C_SENSE 0x03

using namespace std;
int main(int argc, char **argv)
{
	Sense* sense;
	bool first = true;
	unsigned char sample, lastSample;
	unsigned char buffer[BUFFER_SIZE];
	int status, lastStatus;
	try
	{
		sense = new Sense(I2C_SENSE);		
	}
	catch(int ex)
	{
		cout << "Sense library threw exception: " << ex << endl;
		return -1;
	}
	
	int mode;
	cout << "Mode: (1: Light, 2: Sound): ";
	cin >> mode;
	sense->SetMode((SENSE_MODE) mode);
	while(1)
	{
		sample = sense->Sample(); 
		if(sample != lastSample || first)
		{
			first = false;
			lastSample = sample;
			cout << "Sample: " << (int)sample << endl;
		}
	}
	delete sense;
	
	return 0;
}

