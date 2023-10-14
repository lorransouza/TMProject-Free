#include "cServer.h"
#include "Basedef.h"
#include <sstream>
#include <iomanip>

bool CUser::SendCharList(PacketHeader *Header)
{
	p10A *p = (p10A*)Header;
	std::string toUsername{ p->UserName };
	std::string userName{ User.Username };

	if (toUsername != userName)
	{
		Log(clientId, LOG_INGAME, "Falha ao receber pacote de charlist. ID que recebeu: %s. ID correto: %s", p->UserName, User.Username);

		CloseUser(clientId);
		return true;
	}

	p->Header.PacketId = 0x10A;

	// Copia a estrutura da charlist para o User
	memcpy(&this->CharList, &p->CharList, sizeof p->CharList);

	for (int iChar = 0; iChar < 4; ++iChar)
	{
		if (!p->CharList.Name[iChar][0])
			continue;

		if (p->CharList.Equip[iChar][12].Index == 0)
			continue;

		for (int i = 1; i < 11; ++i)
		{
			for (int iEffect = 0; iEffect < 3; ++iEffect)
			{
				if (p->CharList.Equip[iChar][i].Effect[iEffect].Index >= 116 && p->CharList.Equip[iChar][i].Effect[iEffect].Index <= 125)
				{
					p->CharList.Equip[iChar][i].Effect[iEffect].Index = EF_SANC;

					break;
				}
			}
		}
	}

	// Seta as HashsKeys do usuário e envia no pacote
	for (int i = 0; i < 16; i++)
	{
		// Gera a nova rand
		BYTE _rand = KeyTable[Rand() % 512];

		// Seta a key no pacote e na estrutura do usuário
		p->Keys[i] = (BYTE)_rand;
		Keys[i] = (BYTE)_rand;
	}

	memcpy(&this->User.Storage.Item, p->Storage, sizeof(st_Item) * 128);
	this->User.Storage.Coin = p->GoldStorage;
	
	IsAdmin = false;

	// Seta como se o usuári oestivesse na charList
	Status = USER_SELCHAR;

	IsAutoTrading = false;

	// Seta o clientId do pacote como 0x7532
	p->Header.ClientId = 0x7532;

	AddMessage((BYTE*)p, sizeof p10A);

	Log(clientId, LOG_INGAME, "Usuário logou no jogo com clientId %d. - Servidor %d. Gold do banco: %d", clientId, sServer.Channel, User.Storage.Coin);
	LogPlayer(clientId, "Usuário logou no jogo");

	std::stringstream str;
	str << "[INFORMAÇÕES DO BANCO]\n";
	str << "Gold: " << User.Storage.Coin << "\n";

	for (int i = 0; i < 128; i++)
	{
		st_Item* item = &User.Storage.Item[i];
		if (item->Index <= 0 || item->Index > MAX_ITEMLIST)
			continue;

		str << "[" << std::setw(3) << i << "] - " << ItemList[item->Index].Name << item->toString().c_str() << "\n";
	}

	Log(clientId, LOG_INGAME, str.str().c_str());
	return true;
}