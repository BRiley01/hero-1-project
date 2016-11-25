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
		switch(command)
		{
			case 0:
				return 0;
			case 1:
				motor.Recalibrate();
				break;
			case 2:
				if(!motor.Turn(90))
					cout << "FAILED\n";
				break;
			case 3:
				if(!motor.Turn(-90))
					cout << "FAILED\n";
				break;
			case 4:
				cin >> command;
				if(!motor.Turn(command))
					cout << "FAILED\n";
				break;
		}			
	}
	
	return 0;
}

