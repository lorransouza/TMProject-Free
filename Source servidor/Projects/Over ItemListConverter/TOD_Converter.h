#pragma once

#include <map>
#include <array>
#include <string>
#include <exception>
#include "TOD_ItemList.h"

class TOD_Converter
{
private:
	void ReadItemEffect();

protected:
	TOD_Converter()
	{
		ReadItemEffect();
	}

	std::map<std::string, int> _effects;
	std::array<TOD_ItemList, MaxItemlist> _itemList;

	virtual void Read() = 0;

public:
	virtual void Convert() = 0;
};

