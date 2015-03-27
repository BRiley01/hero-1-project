#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>

#define I2C_SPEECH 0x04
#define SPEECH_OPCODE_STATUS 0x00
#define SPEECH_OPCODE_SAY 0x01
#define SPEECH_OPCODE_ABORT 0xFE

#define SPEECH_STATUS_READY 0x01
#define SPEECH_STATUS_LOADING 0x02
#define SPEECH_STATUS_SPEECH_READY 0x03
#define SPEECH_STATUS_SPEAKING 0x04
#define SPEECH_STATUS_WAIT_AR 0x05
#define SPEECH_STATUS_ABORTING 0x06
#define SPEECH_STATUS_FAULT 0xFF

class Speech
{
	private:
		sqlite3* _db;
		int _i2cHandle;
		void toUpper(std::string& s);
		bool sendOpCode(int opcode);
	protected:
		std::vector<unsigned char> wordToPhonemeBytes(const char* word);
		bool send(std::vector<unsigned char> bytes);
	public:
		Speech(const char* db);
		~Speech();
		bool Say(const char* phrase);
		int Status();
		void Abort();
};
