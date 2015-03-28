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
Speech::Speech(const char* db)
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

	int tenBitAddress = 0;
	int opResult = 0;

	// Create a file descriptor for the I2C bus
	_i2cHandle = open("/dev/i2c-1", O_RDWR); 

	// I2C device is not 10-bit
	opResult = ioctl(_i2cHandle, I2C_TENBIT, tenBitAddress);
#ifdef DEBUG
	cout << "Set I2C to non 10-bit mode result: " << opResult << endl;
#endif		
	
	// set address of speech board to I2C
	opResult = ioctl(_i2cHandle, I2C_SLAVE, I2C_SPEECH);
#ifdef DEBUG
	cout << "Set I2C speech board address result: " << opResult << endl;
#endif		
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
	unsigned char byte;
	
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

bool Speech::sendOpCode(int opcode) 
{
	int opResult = 0;
	opResult = write(_i2cHandle, &opcode, 1);
	if(opResult != 1)
	{ 
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif			
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
		return SPEECH_STATUS_FAULT;
	}
	usleep(1000); //sleep for 1 millisecond
	read(_i2cHandle, data, len);
	if(*data != SPEECH_OPCODE_STATUS)
	{
#ifdef DEBUG
		cout << "Unexpected Response OPCODE\n";
#endif	
		Abort();
		return SPEECH_STATUS_FAULT;
	}
	return *(data+1);
}

void Speech::Abort()
{
#ifdef DEBUG
	cout << "ABORT CALLED\n";
#endif
	
	if(!sendOpCode(SPEECH_OPCODE_ABORT))
	{
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif			
	}
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
		opResult = write(_i2cHandle, &c, 1);
		if(opResult != 1)
		{ 
#ifdef DEBUG
			cout << "No ACK bit!\n";
#endif			
		}
		usleep(1000); //sleep for 1 millisecond
	}
	opResult = read(_i2cHandle, rxBuffer, 32);
#ifdef DEBUG
	//cout << "Sent: " << bytes.size() << ", received: " << rxBuffer[0] << endl;
#endif	
		return true;
}

bool Speech::ClassicSpeech(int memAddr)
{
	int opResult = 0;
	char rxBuffer[32];	//	receive buffer
	//unsigned char* ptr = (unsigned char*)&memAddr;
	//unsigned char ptr[] = {0xFB, 0xC1};
	unsigned char ptr[] = {0xFA, 0x4B};
	sendOpCode(SPEECH_OPCODE_SPEECH);
	
#ifdef DEBUG
	cout << "sending: ";
	cout << std::hex << *ptr;
	cout << " ";
	cout << std::hex << *(ptr+1) << endl;
#endif
	
	opResult = write(_i2cHandle, &ptr[0], 1);
	if(opResult != 1)
	{ 
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif			
	}
	usleep(1000); //sleep for 1 millisecond
	opResult = write(_i2cHandle, &ptr[1], 1);
	if(opResult != 1)
	{ 
#ifdef DEBUG
		cout << "No ACK bit!\n";
#endif			
	}
	usleep(1000); //sleep for 1 millisecond
	opResult = read(_i2cHandle, rxBuffer, 32);
	
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
			cout << "\tWord not found in dicitonary!" << endl;
#endif		
			for(int hold = 0; hold < 10; hold++)
				v_word.push_back(0x00);//make prolonged EH sound
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
}
