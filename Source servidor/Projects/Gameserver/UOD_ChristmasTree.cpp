#include "Basedef.h"
#include "SendFunc.h"
#include "UOD_ChristmasTree.h"
#include <sstream>

constexpr std::array<st_Position, 16> treePositions =
{
	{
		{2073, 2091},
		{2096, 2075},
		{2129, 2075},
		{2152, 2068},
		{2141, 2121},
		{2085, 2126},
		{2077, 2111},
		{2474, 1729},
		{2479, 1741},
		{2496, 1726},
		{2483, 1681},
		{2481, 1645},
		{2500, 1652},
		{2462, 1652},
		{2535, 1737},
		{2595, 1725}
	}
};

TOD_ChristmasTree::TOD_ChristmasTree(std::chrono::milliseconds interval)
	: TOD_Event(interval)
{
}

void TOD_ChristmasTree::SetStatus(bool status)
{
	_status = status;
}

void TOD_ChristmasTree::Work()
{
	if (!_status)
		return;

	st_Item GiftItem{ 4718 };
	auto position = select_randomly(std::begin(treePositions), std::end(treePositions));
	int totalGifts = (Rand() % 4) + 6;
	int rotation = 1;

	std::stringstream str;
	for (int i = 0; i < totalGifts; i++)
	{
		int initItemIndex = CreateItem(position->X, position->Y, &GiftItem, rotation, 0);
		if (initItemIndex == 0)
		{
			Log(SERVER_SIDE, LOG_INGAME, "Não foi possível criar o baú de index %d pois não há espaço", i);

			break;
		}

		sServer.Christmas.itemsOnGround.push_back(initItemIndex);
		
		str << "Gerado Presente de Natal com index " << initItemIndex << " em " 
			<< pInitItem[initItemIndex].PosX << "x "
			<< pInitItem[initItemIndex].PosY << "\n";
	}

	SendChatMessage(0xFFFF7F27, "!Espantalho deixou recompensas!! Procure-Os pelaS cidadeS");

	auto nextInterval = ((Rand() % 10) + 30) * 1000 * 60;
	SetInterval(std::chrono::milliseconds(nextInterval));
	
	str << "Próximo intervalo para os presentes: " << nextInterval
		<< "(aproximadamente "
		<< (nextInterval / 1000 / 60) << " minutos)";

	Log(SERVER_SIDE, LOG_INGAME, str.str().c_str());
}