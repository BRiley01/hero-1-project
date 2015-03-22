#include <iostream>
#include "../speech/speech.h"

using namespace std;
int main(int argc, char **argv)
{
	Speech* sp;
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
	}
	delete sp;
	
	return 0;
}

