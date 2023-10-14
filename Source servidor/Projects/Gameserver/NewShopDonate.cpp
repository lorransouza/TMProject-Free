#include "SendFunc.h"
#include "Basedef.h"

void BuyItem(int clientId, int type, int page, int slot)
{
	if (clientId <= 0 || clientId >= 1000)
		return;

	if (type < 0 || type > 6)
		return;

	if (page < 0 || page > 3)
		return;

	if (slot < 0|| slot > 11)
		return;


	if (Users[clientId].Status != 22)
		return;

	for (auto& i : ControlDonateLoja)
	{
		if (i.type != type)
			continue;

		if (i.page != page)
			continue;

		if (i.slot != slot)
			continue;

		if (Users[clientId].User.Cash < (uint32_t)i.price)
		{
			SendClientMessage(clientId, "Você não possui o valor para realizar essa compra.");
			return;
		}

		if (i.stuck < 1)
		{
			SendClientMessage(clientId, "Acabou o estoque do Produto.");
			return;
		}

		/*	if (Func::ReturnSlotEmpty(clientId, SlotType::Storage) == -1)
			{
				Native::SendClientMessage(clientId, "Você não possui espaço no seu Guarda Carga.");
				return;
			}*/


		i.stuck--;
		int DonateValor = (Users[clientId].User.Cash - i.price);
		Users[clientId].User.Cash = DonateValor;

		st_Item* item = &i.item;

		PutItem(clientId, &i.item);

		SendClientMessage(clientId, "O item %s chegou no seu Guarda Carga .", ItemList[item->Index].Name);
		SendClientPacket(clientId);
    	SendEtc(clientId);
		SendLojaDonate(clientId, type, page, slot);
		return;
	}
}

void SendLojaDonate(int clientId, int Type, int Page, int Slot)
{
	struct {
		PacketHeader Header;

		int type;

		int page;

		st_Item item;

		int price;

		int stuck;

		int slot;

	} Packet;

	memset(&Packet, 0x0, sizeof(Packet));

	auto item = ControlDonateLoja;

	Packet.Header.Size = sizeof(Packet);
	Packet.Header.PacketId = 0xAA1;


	for (auto& i : ControlDonateLoja)
	{
		if (i.type != Type)
			continue;

		if (i.page != Page)
			continue;

		if (i.slot != Slot)
			continue;

		Packet.type = i.type;
		Packet.page = i.page;
		Packet.price = i.price;
		Packet.stuck = i.stuck;
		Packet.slot = i.slot;
		memcpy_s(&Packet.item, sizeof(st_Item), &i.item, sizeof(st_Item));
		break;
	}

	for (int i = 0; i < 1000; i++)
	{
		Packet.Header.ClientId = i;
		Users[i].AddMessage((BYTE*)&Packet, sizeof(Packet));
		Users[i].SendMessageA();
	}
}

void SendLojaDonate(int clientId)
{
	struct {
		PacketHeader Header;

		int quantidade;

		struct
		{
			int type;

			int page;

			st_Item item;

			int price;

			int stuck;

			int slot;
		}Produts[231];

	} Packet;

	memset(&Packet, 0x0, sizeof(Packet));

	auto item = ControlDonateLoja;

	Packet.Header.Size = sizeof(Packet);
	Packet.Header.PacketId = 0xAA2;
	Packet.Header.ClientId = 0x7530;
	Packet.quantidade = ControlDonateLoja.size();

	int Total = 0;
	for (auto& i : ControlDonateLoja)
	{
		if (Total > 230)
			break;


		Packet.Produts[Total].type = i.type;
		Packet.Produts[Total].page = i.page;
		Packet.Produts[Total].price = i.price;
		Packet.Produts[Total].stuck = i.stuck;
		Packet.Produts[Total].slot = i.slot;

		memcpy_s(&Packet.Produts[Total].item, sizeof(st_Item), &i.item, sizeof(st_Item));

		Total++;
	}

	Users[clientId].AddMessage((BYTE*)&Packet, sizeof(Packet));
	Users[clientId].SendMessageA();
}

void SendClientPacket(int clientId)
{
	struct stSendInfoClient
	{
		PacketHeader Header;
		int ExpBonus;
		int DropBonus;
		int AbsDamage;
		int PerfuDamage;
		int Cash;
	};

	if (clientId <= 0 || clientId >= 1000)
		return;

	stSendInfoClient packet;
	memset(&packet, 0, sizeof stSendInfoClient);

	packet.Header.PacketId = 0x2132;
	packet.Header.ClientId = clientId;
	packet.Header.Size = sizeof stSendInfoClient;

	packet.Cash = Users[clientId].User.Cash;

	Users[clientId].AddMessage((BYTE*)&packet, sizeof stSendInfoClient);
}
