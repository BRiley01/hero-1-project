#include <cstddef>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "speech.h"

using namespace std;
Speech::Speech(const char* db, int I2C_Address) : Controller(I2C_Address)
{
#ifdef DEBUG
	cout << "Speech constructor called" << endl;
#endif	
	_db = NULL;
	int result=sqlite3_open_v2(db,&_db, SQLITE_OPEN_READONLY, NULL);
#ifdef DEBUG
	cout << "db open result: " << result << endl;
#endif
	if (result != SQLITE_OK) 
	{
		sqlite3_close(_db) ;
		throw result;
	}
}

Speech::~Speech()
{
#ifdef DEBUG
	cout << "Speech deconstructor called" << endl;
#endif	
	if(_db != NULL)
	{
#ifdef DEBUG
	cout << "Freeing database" << endl;
#endif	
		sqlite3_close(_db) ;
	}
	
#ifdef DEBUG
	cout << "Freeing I2C handle" << endl;
#endif	
	close(_i2cHandle);
}

vector<unsigned char> Speech::wordToPhonemeBytes(const char* word)
{
	vector<unsigned char> bytes;
	if(!_db) return bytes;
	const char* pzTest;
	
	sqlite3_stmt *stmt;
	const char* sql = "select sp.code from CMU_Words w "
		"inner join CMU_WXP x on w.wordid= x.wordid "
		"inner join CMU_SC01 cs on x.phoneme = cs.CMU_PH " 
		"inner join SC01_Phonemes sp on sp.phoneme = cs.SC01_PH " 
		"where word = ? " 
		"order by x.sort, cs.sort";
		
	int rc = sqlite3_prepare(_db, sql, strlen(sql), &stmt, &pzTest);
	if( rc != SQLITE_OK ) return bytes;	
	
	sqlite3_bind_text(stmt, 1, word, strlen(word), 0);
	
	while(sqlite3_step(stmt) == SQLITE_ROW)
		bytes.push_back(sqlite3_column_int(stmt, 0));
	sqlite3_finalize(stmt);
	return bytes;
}

void Speech::toUpper(string& s) {
   for (string::iterator p = s.begin();
        p != s.end(); ++p) {
      *p = toupper(*p); // toupper is for char
   }
}

int Speech::Status(unsigned char* data, int len)
{
	unsigned char rxBuffer[5];	//	receive buffer
	if(!data || len == 0)
	{
		data = rxBuffer;
		len = 5;
	}
	memset(data, 0, sizeof(unsigned char) * len);
	if(!sendOpCode(SPEECH_OPCODE_STATUS))
	{
#ifdef DEBUG
		cout << "Status request failed\n";
#endif	
cout <<"AAAA";
		return SPEECH_STATUS_FAULT;
	}
	//usleep(1000); //sleep for 1 millisecond
	read(data, len);
	if(*data != SPEECH_OPCODE_STATUS)
	{
#ifdef DEBUG
		cout << "Unexpected Response OPCODE: " << hex << (int)*data << dec << " len: " << len << "\n";
#endif	
		Controller::Abort();
cout <<"BBBBB";
		return SPEECH_STATUS_FAULT;
	}
	/*else
		cout << hex << (int)*(data+1);*/
		
	return *(data+1);
}


bool Speech::send(vector<unsigned char> bytes)
{
	int opResult = 0;
	unsigned char c;
	char rxBuffer[32];	//	receive buffer
	
	sendOpCode(SPEECH_OPCODE_SAY);
	for (vector<unsigned char>::iterator byte = bytes.begin(); byte != bytes.end(); ++byte)
	{
		c = *byte;
		opResult = write(&c, 1);
		if(opResult != 1)
		{ 
#ifdef DEBUG
			cout << "No ACK bit!\n";
#endif			
		}
		usleep(1000); //sleep for 1 millisecond
	}
	opResult = read(rxBuffer, 32);
#ifdef DEBUG
	//cout << "Sent: " << bytes.size() << ", received: " << rxBuffer[0] << endl;
#endif	
		return true;
}

bool Speech::ClassicSpeech(unsigned char* memAddr)
{
	int opResult = 0;
	char rxBuffer[32];	//	receive buffer	
	sendOpCode(SPEECH_OPCODE_SPEECH);
	
#ifdef DEBUG
	cout << "sending: ";
	cout << std::hex << (int)memAddr[0] << (int)memAddr[1] << endl;
#endif
	
	opResult = write(memAddr, 2);
	if(opResult != 2)
	{ 
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif			
		return false;
	}
	usleep(1000); //sleep for 1 millisecond
	opResult = read(rxBuffer, 32);
	return true;
}

bool Speech::Say(const char* phrase)
{
	string words(phrase);
	istringstream buf(words);
    istream_iterator<string> beg(buf), end;
    vector<unsigned char> v_phrase, v_word;

    vector<string> tokens(beg, end); // done!

    for (vector<string>::iterator word = tokens.begin(); word != tokens.end(); ++word)
    {
		toUpper(*word);
#ifdef DEBUG
		cout << "Getting bytes for: \"" << *word << "\": " << endl;
#endif		

		v_word = wordToPhonemeBytes((*word).c_str());        
		if(v_word.empty())
		{
#ifdef DEBUG
			cout << "\tWord not found in dictionary!" << endl;
#endif		
			for(int hold = 0; hold < 10; hold++)
				v_word.push_back(0x00);//make prolonged EH sound
			return false;
		}
		else
		{
#ifdef DEBUG
			for (vector<unsigned char>::iterator byte = v_word.begin(); byte != v_word.end(); ++byte)
				cout << "\t" << hex << (int)*byte << endl;
#endif	
		}
		v_phrase.insert(v_phrase.end(), v_word.begin(), v_word.end());
		v_phrase.push_back(0x03); //pause between words
	}
	v_phrase.push_back(0x3F); // STOP
	v_phrase.push_back(0xFF); // Tell Aurdino to send
	
#ifdef DEBUG
	cout << "sending:";
	for (vector<unsigned char>::iterator byte = v_phrase.begin(); byte != v_phrase.end(); ++byte)
		cout << " " << hex << (int)*byte;
	cout << endl;
#endif	
	send(v_phrase);
	return true;
}
