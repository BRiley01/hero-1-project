#include <iostream>
#include "../speech/speech.h"

#define BUFFER_SIZE 5
#define I2C_SPEECH 0x04

using namespace std;
int main(int argc, char **argv)
{
	Speech* sp;
	unsigned char buffer[BUFFER_SIZE];
	int status, lastStatus;
	try
	{
		sp = new Speech("dictionary.db", I2C_SPEECH);		
	}
	catch(int ex)
	{
		cout << "Speech library threw exception: " << ex << endl;
		return -1;
	}
	
	string phrase;
	while(phrase != "q")
	{
		cout << "Enter text: ";
		getline(cin, phrase);
		if(phrase != "q")
			sp->Say(phrase.c_str());
		status = sp->Status(buffer, BUFFER_SIZE);
		lastStatus = SPEECH_STATUS_READY;
		/*while(status != SPEECH_STATUS_READY)
		{
			if(status != lastStatus)
			{
				lastStatus = status;
				switch(status)
				{
					case SPEECH_STATUS_READY: 
						cout << "ready\n";
						break;
					case SPEECH_STATUS_LOADING:
						cout << "loading\n";
						break;
					case SPEECH_STATUS_SPEECH_READY:
						cout << "speech ready\n";
						break;
					case SPEECH_STATUS_SPEAKING:
						cout << "speaking: " << hex << (int)buffer[2] << endl;
						break;
					case SPEECH_STATUS_WAIT_AR:
						cout << "waiting on AR\n";
						break;
					case SPEECH_STATUS_ABORTING:
						cout << "aborting\n";
						break;
					case SPEECH_STATUS_FAULT:
						cout << "fault\n";
						break;
					default:
						cout << status << "\n";
				}
			}
			//usleep(10000); //sleep for 1 millisecond
			status = sp->Status(buffer, BUFFER_SIZE);
		}*/
	
	}
	delete sp;
	
	return 0;
}

