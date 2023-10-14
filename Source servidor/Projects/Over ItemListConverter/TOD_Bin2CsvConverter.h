#pragma once

#include "TOD_Converter.h"

class TOD_Bin2CsvConverter : public TOD_Converter
{
private:
	virtual void Read();

public:
	TOD_Bin2CsvConverter()
	{
		Read();
	}

	virtual void Convert();
};

