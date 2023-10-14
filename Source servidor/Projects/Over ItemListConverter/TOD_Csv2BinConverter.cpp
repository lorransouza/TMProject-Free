#include "TOD_Csv2BinConverter.h"
#include <fstream>
#include <iostream>

void TOD_Csv2BinConverter::Read()
{
	std::ifstream stream{ "ItemList.csv " };
	if (!stream.is_open())
		throw std::exception("Can't open ItemList.csv");

	std::string line;
	while (std::getline(stream, line))
	{
		TOD_ItemList item{};
		int itemId = 0;
		char meshBuf[16] = { 0 };
		char scoreBuf[32] = { 0 };
		char effBuf[12][32] = { { 0 } };

		std::string fixedString;
		for (auto& character : line)
		{
			if (character == ',')
				fixedString += ' ';
			else
				fixedString += character;
		}

		int ret = sscanf_s(fixedString.c_str(), "%d %63s %15s %31s %hd %d %hd %hd %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd %31s %hd",
			&itemId, item.Name, 63, meshBuf, 15, scoreBuf, 31, &item.Unique, &item.Price, &item.Pos, &item.Extreme, &item.Grade,
			effBuf[0], 31, &item.Effect[0].Value, effBuf[1], 31, &item.Effect[1].Value, effBuf[2], 31, &item.Effect[2].Value,
			effBuf[3], 31, &item.Effect[3].Value, effBuf[4], 31, &item.Effect[4].Value, effBuf[5], 31, &item.Effect[5].Value,
			effBuf[6], 31, &item.Effect[6].Value, effBuf[7], 31, &item.Effect[7].Value, effBuf[8], 31, &item.Effect[8].Value,
			effBuf[9], 31, &item.Effect[9].Value, effBuf[10], 31, &item.Effect[10].Value, effBuf[11], 31, &item.Effect[11].Value);

		if (ret < 9 || itemId <= 0 || itemId >= MaxItemlist)
			continue;

		sscanf_s(meshBuf, "%hd.%hd", &item.Mesh, &item.SubMesh);
		sscanf_s(scoreBuf, "%hd.%hd.%hd.%hd.%hd", &item.Level, &item.STR, &item.INT, &item.DEX, &item.CON);

		if (itemId == 4808)
			itemId = itemId;

		for (int i = 0; i < 12; i++)
		{
			if (effBuf[i][0] == '\0')
				continue;

			if (_effects.find(effBuf[i]) == std::end(_effects))
			{
				std::cout << "Nâo foi possível encontrar o effect \"" << effBuf[i] << "\"" << std::endl;
				
				continue;
			}

			item.Effect[i].Index = _effects[effBuf[i]];
		}

		_itemList[itemId] = item;
	}

	stream.close();
}

void TOD_Csv2BinConverter::Convert()
{
	std::fstream stream{ "Itemlist.bin", std::ios::out | std::ios::binary };
	if (!stream.is_open())
		throw std::exception("Can't open ItemList.bin");

	auto itemList = _itemList;

	for(auto itemIt = std::begin(itemList); itemIt != std::end(itemList); ++itemIt)
	{
		TOD_ItemList itemToWrite = *itemIt;

		for (size_t i = 0; i < sizeof TOD_ItemList; ++i)
			*reinterpret_cast<uint8_t*>((uint32_t)& itemToWrite + i) ^= 0x5A;

		int itemId = std::distance(std::begin(itemList), itemIt);
		int position = sizeof TOD_ItemList * itemId;

		stream.seekg(position, std::ios::beg);
		stream.write(reinterpret_cast<char*>(&itemToWrite), sizeof TOD_ItemList);
	}

	stream.close();
}