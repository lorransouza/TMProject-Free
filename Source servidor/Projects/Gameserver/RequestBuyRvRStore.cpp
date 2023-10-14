#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestBuyRvRShop(PacketHeader *Header)
{
	_MSG_BUYSTORE *p = (_MSG_BUYSTORE*)(Header);

	auto& items = sServer.RvR.Items;
	int index = -1;
	for (size_t t = 0; t < items.size(); t++)
	{
		if (!memcmp(&items[t].Item, &p->Item, sizeof st_Item))
		{
			index = t;
			break;
		}
	}

	bool can = false;
	for (int i = 1000; i < MAX_SPAWN_MOB; i++)
	{
		if (Mob[i].GenerateID == RVRSTORE_BLUE || Mob[i].GenerateID == RVRSTORE_RED)
		{
			can = Mob[i].GenerateID - RVRSTORE_BLUE == Mob[clientId].Mobs.Player.CapeInfo - CAPE_BLUE;

			break;
		}
	}

	if (!can)
		return true;

	auto& item = items[index];
	if (item.Available != -1 && item.Available <= 0)
	{
		SendClientMessage(clientId, "Item esgotado no momento. Aguarde a reposição.");

		return true;
	}

	if (Mob[clientId].Mobs.RvRPoints < item.Price)
	{
		SendClientMessage(clientId, "Você não possui pontos suficientes.");

		return true;
	}

	int slotId = GetFirstSlot(clientId, 0);
	if (slotId == -1)
	{
		SendClientMessage(clientId, "Se mespaço no inventário");

		return true;
	}

	memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);
	memcpy(&Mob[clientId].Mobs.Player.Inventory[slotId], &item.Item, sizeof st_Item);

	SendItem(clientId, SlotType::Inv, slotId, const_cast<st_Item*>(&item.Item));

	Mob[clientId].Mobs.RvRPoints -= item.Price;

	SendClientMessage(clientId, "Restam %d pontos.", Mob[clientId].Mobs.RvRPoints);

	Log(clientId, LOG_INGAME, "Compra item RVRSTORE [%s] %d [%d %d %d %d %d %d] preço %d. Restante: %d", ItemList[item.Item.Index].Name,
		item.Item.Index, item.Item.EF1, item.Item.EFV1, item.Item.EF2, item.Item.EFV2, item.Item.EF3, item.Item.EFV3, item.Price, item.Available - 1);

	Log(SERVER_SIDE, LOG_INGAME, "%s - Compra item RVRSTORE [%s] %d [%d %d %d %d %d %d] preço %d. Restante: %d", User.Username, ItemList[item.Item.Index].Name,
		item.Item.Index, item.Item.EF1, item.Item.EFV1, item.Item.EF2, item.Item.EFV2, item.Item.EF3, item.Item.EFV3, item.Price, item.Available - 1);

	if (item.Available != -1)
		item.Available--;

	return true;
}