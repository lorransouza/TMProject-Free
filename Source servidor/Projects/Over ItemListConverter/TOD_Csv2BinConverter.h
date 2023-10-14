#pragma once
#include "TOD_Converter.h"

class TOD_Csv2BinConverter : public TOD_Converter
{
protected:
	virtual void Read();

public:
	TOD_Csv2BinConverter()
	{
		Read();
	}

	virtual void Convert();
};

