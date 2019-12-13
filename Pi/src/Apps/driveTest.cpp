#include <iostream>
#include "../drive/drive.h"

#define BUFFER_SIZE 5
#define I2C_MOTOR 5

using namespace std;

bool fullTest(DriveMotor &motor);
void showStatus(DriveMotor &motor);
void speedTest(DriveMotor &motor);
void distanceTest(DriveMotor &motor, unsigned long distance);

int main(int argc, char **argv)
{
	DriveMotor motor(I2C_MOTOR);
	int command;
    unsigned long distance;
	
	cout << sizeof(t_DRIVE_STATUS) << endl << sizeof(t_DRIVE_INFO) << endl << sizeof(t_WHEEL_MOVEMENT) << endl;
	
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
			case 5:
				if(!motor.Drive(1, 1))
					cout << "FAILED\n";
				break;
			case 6:
				if(!motor.Drive(0, 64))
					cout << "FAILED\n";
				break;
			case 7:
				if(!motor.Drive(0, 0))
					cout << "FAILED\n";
				break;
			case 8:
				fullTest(motor);
				break;
			case 9:
				showStatus(motor);
				break;
			case 10:
				speedTest(motor);
                break;
            case 11:
                cin >> distance;
                distanceTest(motor, distance);
		}			
	}
	
	return 0;
}

void distanceTest(DriveMotor &motor, unsigned long distance)
{
   motor.Drive(1, 64, distance); 
}

void speedTest(DriveMotor &motor)
{
	int i;
	for(i =0; i< 64; i++)
	{
		cout << "Forward: " << i << endl;
		motor.Drive(1, i);
		while(motor.Status().Drive.Speed != i);
		usleep(1000000);
	}
		
	for(i =0; i< 64; i++)
	{
		cout << "Reverse: " << i << endl;
		motor.Drive(0, i);
		while(motor.Status().Drive.Speed != i);
		usleep(1000000);
	}
    cout << "Reverse: 1" << endl;
    motor.Drive(0, 1);
    while(motor.Status().Drive.Speed != 1);
    usleep(1000000);

    cout << "Test Complete" << endl;
    motor.Drive(0, 0);
}

void showStatus(DriveMotor &motor)
{
	cout << "Getting Status...";
	t_DRIVE_STATUS status = motor.Status();
	cout << "cal:" << (int)status.Recalibrating << " " 
		<< "Forward: " << (int)status.Drive.Forward << " "
		<< "drspeed: " << (int)status.Drive.Speed << " "
		<< "pos:" << (int)status.Wheel.Pos << " "
		<< "angle:" << (int)status.Wheel.Angle << " "
		<< "dpos: " << (int)status.Wheel.DestPos << " "
		<< "dangle:" << (int)status.Wheel.DestAngle << "\n";
}

bool fullTest(DriveMotor &motor)
{
	t_DRIVE_STATUS status;
	cout << "Forward drive\n";
	if(!motor.Drive(1, 64))
		cout << "1 failed";
	cout << "recalibrate\n";
	if(!motor.Recalibrate())
		cout << "2 failed";
	while(motor.Status().Recalibrating != 0) usleep(100);
	cout << "stop drive\n";
	if(!motor.Drive(0, 0))
		cout << "2a failed";
	cout << "turn to -90\n";
	if(!motor.Turn(-90))
		cout << "3 failed";
	cout << "slow reverse\n";
	if(!motor.Drive(0, 1))
		cout << "4 failed";
	
	
	cout << "Wait for -90\n";
	while(status.Wheel.Angle != status.Wheel.DestAngle)
	{ 
		status = motor.Status();
		usleep(100);
	}
		
	cout << "faster forward\n";
	// wait until turn is done	
	if(!motor.Drive(1, 30))
		cout << "5 failed";
	cout << "turn to 90\n";
	if(!motor.Turn(90))
		cout << "6 failed";
	
	cout << "wait for 90\n";
	status = motor.Status();
	while(status.Wheel.Angle != status.Wheel.DestAngle)
	{
		status = motor.Status();
		usleep(100);
	}
		
	cout << "stop drive\n";
	if(!motor.Drive(1, 0))
		cout << "7 failed";	
	cout << "Turn to 0\n";
	if(!motor.Turn(0))
		cout << "8 failed";
}
