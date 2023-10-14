#pragma once

#include <string>
#include <stdio.h>
#include <ctime>

#include <cstdarg>

class doLog
{
private:
	FILE* _file{ nullptr };

	std::string _folder;
	std::string _username;

	int _day{ 0 };

	void Open();
	void Close();
	void BASE_GetFirstKey(const char* source, char* dest);
public:
	doLog(std::string folder, std::string username);
	doLog(std::string folder);
	~doLog();

	void Log(const char* message, va_list arglist);
};