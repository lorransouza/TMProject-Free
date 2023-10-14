#pragma once

#include "UOD_Singleton.h"
#include <fstream>

class FileWrapper
{
public:
	FileWrapper(std::string fileName)
	{
		fopen_s(&_file, fileName.c_str(), "wb+");
	}

	~FileWrapper()
	{
		if (_file != nullptr)
		{
			fclose(_file);

			_file = nullptr;
		}
	}

	operator FILE*() const
	{
		return _file;
	}
private:
	FILE* _file{ nullptr };
};

class TOD_DroplistGenerator : public TOD_Singleton<TOD_DroplistGenerator>
{
public:
	bool Generate();
private:
};