#include <iostream>
#include "../drive/drive.h"

#define BUFFER_SIZE 5
#define I2C_MOTOR 5

using namespace std;
int main(int argc, char **argv)
{
	DriveMotor motor(I2C_MOTOR);
	int command;
	
	while(1)
	{
		cout << "Enter command: ";
		cin >> command;
		if(command == '.')
			break;
			
	}
	
	return 0;
}

