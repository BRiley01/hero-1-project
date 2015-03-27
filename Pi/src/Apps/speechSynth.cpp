#include <iostream>
#include "../speech/speech.h"

using namespace std;
int main(int argc, char **argv)
{
	Speech* sp;
	int status;
	try
	{
		sp = new Speech("dictionary.db");		
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
		status = sp->Status();
		while(status != SPEECH_STATUS_READY)
		{
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
					cout << "speaking\n";
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
			}
			//usleep(10000); //sleep for 1 millisecond
			status = sp->Status();
		}
	}
	delete sp;
	
	return 0;
}

