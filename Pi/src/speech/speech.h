#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>
#include "controller.h"

#define SPEECH_OPCODE_STATUS 0x10
#define SPEECH_OPCODE_SAY 0x11
#define SPEECH_OPCODE_SPEECH 0x12

#define SPEECH_STATUS_READY 0x01
#define SPEECH_STATUS_LOADING 0x02
#define SPEECH_STATUS_SPEECH_READY 0x03
#define SPEECH_STATUS_SPEAKING 0x04
#define SPEECH_STATUS_WAIT_AR 0x05
#define SPEECH_STATUS_ABORTING 0x06
#define SPEECH_STATUS_FAULT 0xFF

class Speech: Controller
{
	private:
		sqlite3* _db;
		void toUpper(std::string& s);
	protected:
		std::vector<unsigned char> wordToPhonemeBytes(const char* word);
		bool send(std::vector<unsigned char> bytes);
	public:
		Speech(const char* db, int I2C_Address);
		~Speech();
		bool Say(const char* phrase);
		bool ClassicSpeech(unsigned char* memAddr);
		int Status(unsigned char* data = NULL, int len = 0);
		void Abort();
};
