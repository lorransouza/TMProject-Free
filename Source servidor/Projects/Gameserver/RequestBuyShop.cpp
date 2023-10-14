#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <tuple>

bool CUser::RequestBuyShop(PacketHeader *Header)
{
	p379 *p = (p379*)(Header);

	if (p->mobID >= MAX_SPAWN_MOB)
	{
		Log(clientId, LOG_INGAME, "Enviado pacote de compra de item com npcId %d", p->mobID);
		return false;
	}
	int mobId = p->mobID;
	if(p->sellSlot < 0 || p->sellSlot > 63 || p->invSlot < 0 || p->invSlot >= 60)
	{
		Log(clientId, LOG_ERROR, "Enviando slots com indices irregular.");
		return false;
	}

	// recompra de items
	if (mobId < 1000)
	{
		if (p->sellSlot == 27)
			p->sellSlot = 9;

		if (p->sellSlot >= static_cast<int>(Repurchase.Items.size()))
		{
			Log(clientId, LOG_ERROR, "Enviado pacote de recompra de item com index de %d", p->sellSlot);

			return false;
		}

		st_Item* item = &Repurchase.Items[p->sellSlot];

		// TODO : log error
		if (item->Index <= 0 || item->Index >= MAX_ITEMLIST)
			return false;

		long long price = static_cast<unsigned long long>(ItemList[item->Index].Price);
		bool sellAmount = false;
		switch (item->Index)
		{
		case 419:
		case 420:
		case 412:
		case 413:
			sellAmount = true;
			break;
		}

		if (sellAmount)
			price *= GetItemAmount(item);

		if (price > 2000000000LL)
		{
			SendClientMessage(clientId, "Não é possível recomprar este item.");

			Log(clientId, LOG_HACK, "Tentativa de recomprar item com valor maior que 2bi. Valor: %lld. %s %s", price, ItemList[item->Index].Name, item->toString().c_str());
			Log(clientId, LOG_HACK, "%s Tentativa de recomprar item com valor maior que 2bi. Valor: %lld. %s %s", User.Username, price, ItemList[item->Index].Name, item->toString().c_str());
			return true;
		}

		if (Mob[clientId].Mobs.Player.Gold < price)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

			return true;
		}

		int slotId = GetFirstSlot(clientId, 0);
		if (slotId == -1)
		{
			SendClientMessage(clientId, "Não há espaço no inventário");

			return true;
		}

		Mob[clientId].Mobs.Player.Gold -= static_cast<int>(price);
		SendSignalParm(clientId, clientId, 0x3AF, Mob[clientId].Mobs.Player.Gold);

		Mob[clientId].Mobs.Player.Inventory[slotId] = *item;
		SendItem(clientId, SlotType::Inv, slotId, item);

		Log(clientId, LOG_INGAME, "Recomprado o item %s %s pelo preço de %d", ItemList[item->Index].Name, item->toString().c_str(), price);
		*item = st_Item{};

		SendRepurchase(clientId);
		return true;
	}

	st_Mob *spw = &Mob[mobId].Mobs.Player;
	st_Mob *mySpawn = &Mob[clientId].Mobs.Player;

	auto distance = GetDistance(Mob[mobId].Target.X, Mob[mobId].Target.Y, Mob[clientId].Target.X, Mob[clientId].Target.Y);
	if (distance > VIEWGRIDX / 2)
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		Log(clientId, LOG_INGAME, "Uso do NPC %s fora do range para tentar comprar %ux %uy %ux %uy. Distância: %d", spw->Name, Mob[mobId].Target.X, Mob[mobId].Target.Y, Mob[clientId].Target.X, Mob[clientId].Target.Y, distance);
		return false;
	}

	if(spw->bStatus.Merchant.Merchant != 1)
	{
		Log(SERVER_SIDE, LOG_INGAME, "%s - Tentativa de comprar item de mob! Mob: %s. Posição: %dx %dy.", User.Username, spw->Name, Mob[mobId].Target.X, Mob[mobId].Target.Y);

		return true;
	}

	st_Item *item = &spw->Inventory[p->sellSlot];
	if(item->Index <= 0 || item->Index >= MAX_ITEMLIST)
		return false;

	if(mySpawn->Inventory[p->invSlot].Index != 0)
	{
		SendItem(clientId, SlotType::Inv, p->invSlot, &mySpawn->Inventory[p->invSlot]);

		return true;
	}

	if(p->invSlot >= 30 && p->invSlot < 45)
	{
		float time = TimeRemaining(Mob[clientId].Mobs.Player.Inventory[60].EFV1, Mob[clientId].Mobs.Player.Inventory[60].EFV2, Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900);
		if(time <= 0)
			return true;
	}

	else if(p->invSlot >= 45 && p->invSlot < 60)
	{
		float time = TimeRemaining(Mob[clientId].Mobs.Player.Inventory[61].EFV1, Mob[clientId].Mobs.Player.Inventory[61].EFV2, Mob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900);
		if(time <= 0.0f)
			return true;
	}

	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	sItemData *itemList = &ItemList[item->Index];

	INT32 cityZone = GetVillage(Mob[mobId].Target.X, Mob[mobId].Target.Y);
	if(cityZone == 5)
		cityZone = 4;
	
	INT64 itemPrice = itemList->Price;

	INT32 perc_impost = g_pCityZone[cityZone].perc_impost;
	INT64 impost = 0;
	if(perc_impost != 0)
	{
		impost = (itemPrice * static_cast<INT64>(perc_impost) / 100LL);
		itemPrice = (itemPrice + impost);
	}

	if (itemPrice > 2000000000LL)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

		return true;
	}
	
	if (mySpawn->Gold < itemPrice)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

		return true;
	}
	
	if(impost >= 2000000LL)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Imposto maior que 2milhões.");
		Log(SERVER_SIDE, LOG_INGAME, "%s - Comprou o item %s na loja de %s. Preço: %lld. Imposto: %lld. Gold: %d.", User.Username, itemList->Name, spw->Name, itemPrice, impost, Mob[clientId].Mobs.Player.Gold);
	}

	// Arrecada o imposto da cidade
	g_pCityZone[cityZone].impost += impost;

	Log(clientId, LOG_INGAME, "Comprou o item %s %s na loja de %s. Preço total: %lld. Imposto: %lld. Gold: %d anterior. Gold após a compra: %d", itemList->Name, item->toString().c_str(), spw->Name, itemPrice, impost, Mob[clientId].Mobs.Player.Gold, mySpawn->Gold - static_cast<int>(itemPrice));
	LogPlayer(clientId, "Comprou o item %s na loja de %s por %lld. Impostos: %lld", itemList->Name, spw->Name, itemPrice, impost);

	// Retira o gold COM o imposto
	mySpawn->Gold -= static_cast<int>(itemPrice);

	SendSignalParm(clientId, clientId, 0x3AF, mySpawn->Gold);
	memcpy(&mySpawn->Inventory[p->invSlot], item, sizeof st_Item);

	SendItem(clientId, SlotType::Inv, p->invSlot, item);
	return true;
}
