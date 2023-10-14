#include "UOD_Log.h"

TOD_Log::TOD_Log(std::string folder, std::string username)
	: _username(username)
	, _folder(folder)
{
	Open();
}

TOD_Log::TOD_Log(std::string folder)
	: _username("")
	, _folder(folder)
{
	Open();
}

TOD_Log::~TOD_Log()
{
	Close();
}

void TOD_Log::Close()
{
	if (_file != nullptr)
	{
		fclose(_file);

		_file = nullptr;
	}

	_day = 0;
}

void TOD_Log::Open()
{
	if (_file)
		Close();

	char szFileName[512] = { 0 };

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	if (!_username.empty())
	{
		char first[16] = { 0 };
		BASE_GetFirstKey(_username.c_str(), first);

		sprintf_s(szFileName, "%s\\%s\\%s - %02d-%02d-%02d.txt", _folder.c_str(), first, _username.c_str(), now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));
	}
	else
		sprintf_s(szFileName, "%s\\%02d-%02d-%02d.txt", _folder.c_str(), now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));

	_day = now.tm_mday;

	// Abre o arquivo de log 
	_file = _fsopen(szFileName, "at+", _SH_DENYNO);
}

void TOD_Log::Log(const char* message, va_list arglist)
{
	try 
	{
		time_t rawnow = time(NULL);
		struct tm now; localtime_s(&now, &rawnow);

		// Se o dia atual for diferente do dia registrado na abertura do log
		// É porque o usuário passou o dia
		if (now.tm_mday != _day || _file == nullptr)
			Open();

		if (_file == nullptr || message == nullptr || arglist == nullptr)
			return;

		// Insere a hora no arquivo
		fprintf(_file, "\n%02d:%02d:%02d : ",
			now.tm_hour, now.tm_min, now.tm_sec);

		// Insere o log em si
		std::vfprintf(_file, message, arglist);

		fflush(_file);
	}
	catch (std::exception& e)
	{

	}
}

void TOD_Log::BASE_GetFirstKey(const char * source, char * dest)
{
	if ((source[0] >= 'A' && source[0] <= 'Z') || (source[0] >= 'a' && source[0] <= 'z'))
	{
		dest[0] = source[0];
		dest[1] = 0;

		return;
	}

	strcpy_s(dest, 4, "etc");
}
