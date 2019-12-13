#include <iostream>
#include "../sonar/sonar.h"

#define I2C_SONAR 6

using namespace std;


int main(int argc, char **argv)
{
	Sonar sonar(I2C_SONAR);
	int command;
	unsigned long dist, prev = 0;
	
	sonar.Disable();
	cout << "Type character and press enter to begin...";
	cin >> command;
	sonar.Enable();
	while(true)
	{
		dist = sonar.Distance();
		
		if(prev != dist)
		{
			if(dist == SONAR_UNDEFINED_DISTANCE)
				cout << "Unknown distance" << endl;
			else
				cout << dist << endl;		
			
			prev = dist;
		}
	}
	cout << "Disabling and exiting";
	sonar.Disable();
	
	return 0;
}

