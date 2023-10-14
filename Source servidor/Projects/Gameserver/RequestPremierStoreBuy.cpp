#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestPremierStoreBuy(PacketHeader *Header)
{
	pMsgSignal2 *p = (pMsgSignal2*)(Header);

	if(p->Value < 0 || p->Value >= MAX_PREMIERSTORE)
		return false;

	auto type = (TOD_Store_Type)p->Value2;

	st_Item* item = nullptr;
	int price = 0;
	int itemToSearch = -1;
	if (type == TOD_Store_Type::PremierStore)
	{
		item = &sServer.PremierStore.item[p->Value];
		price = sServer.PremierStore.Price[p->Value];

		itemToSearch = 4547;
	}
	else if (type == TOD_Store_Type::ArenaStore)
	{
		item = &sServer.ArenaStore.item[p->Value];
		price = sServer.ArenaStore.Price[p->Value];

		itemToSearch = 4641;
	}
	else
	{
		Log(clientId, LOG_HACK, "Envio de ID de loja inválido. %d", p->Value2);
		return true;
	}

	INT32 amount  = GetInventoryAmount(clientId, itemToSearch);
	if(amount < price)
	{
		SendClientMessage(clientId, "Valor insuficientes!");

		return true;
	}

	if(RemoveAmount(clientId, itemToSearch, price) == price)
	{
		INT32 slotId = GetFirstSlot(clientId, 0);
		if(slotId == -1)
		{
			SendClientMessage(clientId, "Inventário cheio!");

			return true;
		}

		// zera o inv
		memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);
		memcpy(&Mob[clientId].Mobs.Player.Inventory[slotId], item, sizeof st_Item);

		SendItem(clientId, SlotType::Inv, slotId, item);
	}

	return true;
}