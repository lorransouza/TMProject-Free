#ifndef UOD_LogH
#define UOD_LogH

#include <string>
#include <stdio.h>
#include <ctime>

#include <cstdarg>

class TOD_Log
{
private:
	FILE* _file{ nullptr };

	std::string _folder;
	std::string _username;

	int _day{ 0 };

	void Open();
	void Close();
	void BASE_GetFirstKey(const char * source, char * dest);
public:
	TOD_Log(std::string folder, std::string username);
	TOD_Log(std::string folder);
	~TOD_Log();

	void Log(const char* message, va_list arglist);
};


#endif