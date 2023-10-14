#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <algorithm>

bool CUser::RequestSellShop(PacketHeader *Header)
{
	p37A *p = (p37A*)Header;

	int fairyId = Mob[clientId].Mobs.Player.Equip[13].Index;

	int mobId = p->npcId;
	if (mobId >= MAX_SPAWN_MOB || mobId < 0)
	{
		Log(clientId, LOG_ERROR, "Pacote RequestSellShop enviado com ID de mob %d", mobId);

		return false;
	}

	if((mobId < 1000 || mobId >= MAX_SPAWN_MOB) && fairyId != 3914 && fairyId != 3915)
	{ 
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	if (p->sellSlot < 0 || p->sellSlot >= 60)
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	CMob *mob = nullptr;
	if (mobId != 0)
	{
		mob = &Mob[mobId];
		auto distance = GetDistance(mob->Target.X, mob->Target.Y, Mob[clientId].Target.X, Mob[clientId].Target.Y);
		if (distance > VIEWGRIDX / 2 && fairyId != 3914 && fairyId != 3915)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			Log(clientId, LOG_INGAME, "Uso do NPC %s fora do range para tentar vender %dx %dy %dx %dy. Distância: %d", mob->Mobs.Player.Name, mob->Target.X, mob->Target.Y, Mob[clientId].Target.X, Mob[clientId].Target.Y, distance);
			return false;
		}
	}

	st_Item *item = &Mob[clientId].Mobs.Player.Inventory[p->sellSlot];
	if(item->Index <= 0 || item->Index > MAX_ITEMLIST)
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	sItemData *rItem = &ItemList[item->Index];
	INT32 cityZone = 5;
	if (mob != nullptr)
		cityZone = GetVillage(mob->Target.X, mob->Target.Y);

	st_Item temporaryItem{ *item };
	auto sellItemResult = SellItem(clientId, item);

	if(sellItemResult == TOD_SellItemResult::GoldLimit)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		SendItem(clientId, (SlotType)p->type, p->sellSlot, item);
		return true;
	}

	if (sellItemResult == TOD_SellItemResult::BlockedItem)
	{
		SendClientMessage(clientId, "Não é possível vender este item");

		SendItem(clientId, (SlotType)p->type, p->sellSlot, item);
		return true;
	}

	auto found = std::find_if(Repurchase.Items.begin(), Repurchase.Items.end(), [](const st_Item& item) {
		return item.Index <= 0 || item.Index >= MAX_ITEMLIST;
	});

	if(found == Repurchase.Items.end())
	{
		if (++Repurchase.LastIndex >= Repurchase.Items.size())
			Repurchase.LastIndex = 0;

		found = Repurchase.Items.begin() + Repurchase.LastIndex;
	}

	(*found) = temporaryItem;

	if(mob != nullptr)
		Log(clientId, LOG_INGAME, "Vendido item no NPC %s", mob->Mobs.Player.Name);

	SendItem(clientId, SlotType::Inv, p->sellSlot, item);
	return true;
}