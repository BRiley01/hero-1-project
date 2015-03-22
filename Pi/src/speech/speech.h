#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>

#define I2C_SPEECH 0x04

class Speech
{
	private:
		sqlite3* _db;
		int _i2cHandle;
		void toUpper(std::string& s);
	protected:
		std::vector<unsigned char> wordToPhonemeBytes(const char* word);
		bool send(std::vector<unsigned char> bytes);
	public:
		Speech(const char* db);
		~Speech();
		bool Say(const char* phrase);
};
