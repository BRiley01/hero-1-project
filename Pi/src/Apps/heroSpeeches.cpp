#include <iostream>
#include "../speech/speech.h"

#define BUFFER_SIZE 6

using namespace std;
int main(int argc, char **argv)
{
	Speech* sp;
	unsigned char buffer[BUFFER_SIZE];
	int status, lastStatus;
	try
	{
		sp = new Speech("dictionary.db");		
	}
	catch(int ex)
	{
		cout << "Speech library threw exception: " << ex << endl;
		return -1;
	}
	
	unsigned char speechBytes[2];
	int input;
	while(input != 0)
	{
		cout << "Enter text: ";
		cin >> hex >> input;
		if(input != 0)
		{
			speechBytes[1] = input & 0xFF;
			speechBytes[0] = (input >> 8) & 0xFF;
			cout << "sending" << endl;
			//sp->ClassicSpeech(0xFBC1);
			sp->ClassicSpeech(speechBytes);
			cout << "sent" << endl;
		}
		status = sp->Status(buffer, BUFFER_SIZE);
		lastStatus = SPEECH_STATUS_READY;
		while(status != SPEECH_STATUS_READY)
		//while(1)
		{
			if(status != lastStatus || true)
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
						cout << "speech ready: " << hex << (int)buffer[2] << ":" 
							 << (int)buffer[3] << ":" 
							 << (int)buffer[4] << ":" 
							 << (int)buffer[5] << endl;
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
			//usleep(100000); //sleep for 1 millisecond
			//usleep(1000);
			status = sp->Status(buffer, BUFFER_SIZE);
		}
	}
	delete sp;
	
	return 0;
}

