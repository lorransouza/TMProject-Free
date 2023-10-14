#include "TOD_Bin2CsvConverter.h"
#include <fstream>

void TOD_Bin2CsvConverter::Read()
{
	std::fstream stream{ "Itemlist.bin", std::ios::in | std::ios::binary };
	if (!stream.is_open())
		throw std::exception("Can't open ItemList.bin");

	for (auto& item : _itemList)
	{
		stream.read(reinterpret_cast<char*>(&item), sizeof TOD_ItemList);

		for (size_t i = 0; i < sizeof TOD_ItemList; ++i)
			*reinterpret_cast<uint8_t*>((uint32_t)&item + i) ^= 0x5A;
	}

	stream.close();
}

void TOD_Bin2CsvConverter::Convert()
{
	std::fstream stream{ "ItemList.csv", std::ios::out | std::ios::trunc };
	if (!stream.is_open())
		throw std::exception("Can't open ItemList.csv to write");

	for (auto itemIt = std::begin(_itemList); itemIt != std::end(_itemList); ++itemIt)
	{
		const auto& item = *itemIt;
		if (item.Name[0] == '\0')
			continue;

		int itemId = std::distance(std::begin(_itemList), itemIt);

		stream << itemId << "," << item.Name << ",";
		stream << item.Mesh << "." << item.SubMesh << ",";
		stream << item.Level << "." << item.STR << ".";
		stream << item.INT << "." << item.DEX << ".";
		stream << item.CON << "," << item.Unique << ",";
		stream << item.Price << "," << item.Pos << ",";
		stream << item.Extreme << "," << item.Grade;

		for (size_t i = 0; i < 12; i++)
		{
			int index = item.Effect[i].Index;
			if (index <= 0)
				continue;

			std::string effectName;
			for (const auto& [effName, effIndex] : _effects)
			{
				if (effIndex == index)
				{
					effectName = effName;

					break;
				}
			}

			if (effectName.empty())
				continue;

			stream << "," << effectName << "," << item.Effect[i].Value;
		}

		stream << std::endl;
	}

	stream.close();
}