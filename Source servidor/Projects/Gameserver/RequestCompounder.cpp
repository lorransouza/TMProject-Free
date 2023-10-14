#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

constexpr std::array blockedItems = { 1104,1116,1128,1140,1152,1416,1419,1422,1425,1428,1254,1266,1278,1290,1302,1553,1569,1572,1575,1578,938,939,940,941,942,943 }; // Armas Sephira

int GetMatchCombine(st_Item *item)
{
	short itemid = item[0].Index;
	short stone = item[1].Index;

	if (itemid <= 0 || itemid >= MAX_ITEMLIST)
		return 0;

	if (stone <= 0 || stone >= MAX_ITEMLIST)
		return 0;

	short unique = ItemList[itemid].Unique;
	if (unique < 41 || unique > 49)
		return 0; // Apenas armas podem ser compostas 

	short itemanctid = ItemList[itemid].Extreme;
	if (itemanctid == 0)
		return 0; // Armas de eventos não podem ser Anct

	int type = GetEffectValueByIndex(item[0].Index, EF_UNKNOW1);
	int refine = 1;
	for (int i = 2; i < 8; i++)
	{
		short index = item[i].Index;
		if (index <= 0 || index >= MAX_ITEMLIST)
			continue;

		short pos = GetItemAbility(&item[i], EF_POS);
		if (pos == 0)
			return 0;

		if (std::find(std::begin(blockedItems), std::end(blockedItems), index) != std::end(blockedItems))
			return 0;

		short sanc = GetItemSanc(&item[i]);
		if (sanc == 7)
			refine += 2;
		else if (sanc == 8)
			refine += 4;
		else if (sanc == 9)
			refine += 10;
		else
			refine += sanc + 2;

		int ef_type = GetEffectValueByIndex(index, EF_UNKNOW1);
		if (ef_type == type)
			continue;

		if (ef_type <= (type - 2))
			return 0;

		if (ef_type < type)
			refine -= 2;
		else if (ef_type > type)
			refine += 2;
	}

	return refine > 100 ? 100 : refine;
}

bool CUser::RequestCompounder(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	st_Mob *player = (st_Mob*)(&Mob[clientId].Mobs.Player);

	for (int i = 0; i < 8; i++)
	{
		if (p->items[i].Index <= 0 || p->items[i].Index >= MAX_ITEMLIST)
			continue;

		short slot = p->slot[i];
		if (slot < 0 || slot >= 60)
		{
			SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

			return false;
		}

		if (memcmp(&p->items[i], &player->Inventory[p->slot[i]], sizeof st_Item))
		{
			SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

			return false;
		}

		for (int y = 0; y < 8; y++)
		{
			if (y == i)
				continue;

			if (p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Compounder - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Compounder - %d", player->Name, p->items[i].Index);

				SendCarry(clientId);
				return false;
			}
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Compounder - %d - Sem item\n", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Compounder - %d - %s %s - %hhd", i, ItemList[p->items[i].Index].Name, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);

		return true;
	}

	int chance = GetMatchCombine(p->items);
	if (chance <= 0)
	{
		SendClientMessage(p->Header.ClientId, "Composição incorreta");

		return false;
	}

	short slot = p->slot[0];
	if (slot < 0 || slot >= 64)
	{
		Log(clientId, LOG_INGAME, "Envio de slot %d em Anct", slot);
		return false;
	}

	int stone = (p->items[1].Index - 2441);
	if (stone < 0 || stone > 3)
	{
		SendClientMessage(clientId, "Jóia incorreta na composição do item.");

		SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
		return true;
	}

	// Checagem de classe do item
	int type = GetEffectValueByIndex(p->items[0].Index, EF_UNKNOW1);
	for (int i = 2; i < 6; i++)
	{
		int _tmp_type = GetEffectValueByIndex(p->items[0].Index, EF_UNKNOW1);
		if (type >= _tmp_type)
			continue;

		if (type - 1 < _tmp_type)
		{
			SendClientMessage(clientId, "Itens tem de ser de até uma classe a menos");

			SendSignalParm(p->Header.ClientId, SERVER_SIDE, 0x3A7, 2);
			return true;
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (p->items[i].Index <= 0 || p->items[i].Index >= MAX_ITEMLIST)
			continue;

		short slot = p->slot[i];
		if (slot < 0 || slot >= 60)
			continue;

		memset(&player->Inventory[slot], 0, sizeof(st_Item));
		SendItem(clientId, SlotType::Inv, slot, &player->Inventory[slot]);
	}

	chance += GetCompounderDeal(clientId);

	int _rand = (Rand() % 115);
	if (_rand > 100)
		_rand -= 15;

	for (int i = 0; i < 8; i++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Compor Anct - %d - Sem item", i);
			continue;
		}

		Log(clientId, LOG_COMP, "Compor Anct - %d - %s [%d] [%d %d %d %d %d %d] %d - Slot: %d", i, ItemList[p->items[i].Index].Name, p->items[i].Index, p->items[i].EF1, p->items[i].EFV1,
			p->items[i].EF2, p->items[i].EFV2, p->items[i].EF3, p->items[i].EFV3, _rand, p->slot[i]);
	}

	if (_rand > chance)
	{
		SendServerNotice("%s falhou na composição do item %s para Ancient", player->Name, ItemList[p->items[0].Index].Name);
		SendClientMessage(clientId, "Falha na composição do item. (%d/%d)", _rand, chance);

		SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

		Log(clientId, LOG_COMP, "Falha na composição do item %s - %d %d %d %d %d %d %d - %d %d", ItemList[player->Inventory[slot].Index].Name,
			p->items[0].Index, p->items[0].EF1, p->items[0].EFV1, p->items[0].EF2, p->items[0].EFV2, p->items[0].EF3, p->items[0].EFV3,
			_rand, chance);

		LogPlayer(clientId, "Falha na composição do item %s para anct", ItemList[p->items[0].Index].Name);
		return true;
	}

	SendEmotion(clientId, 100, Rand() % 8);
	SendEmotion(clientId, 100, Rand() % 8);
	SendEmotion(clientId, 100, Rand() % 8);

	int itemid = p->items[0].Index;
	memcpy(&player->Inventory[slot], &p->items[0], sizeof st_Item);

	player->Inventory[slot].Index = (ItemList[itemid].Extreme + stone);

	SetItemSanc(&player->Inventory[slot], 7, 0);

	SendItem(clientId, SlotType::Inv, slot, &player->Inventory[slot]);

	SendServerNotice("Jogador %s obteve sucesso na composição de %s", player->Name, ItemList[player->Inventory[slot].Index].Name);
	SendClientMessage(clientId, "Jogador %s obteve sucesso na composição de %s (%d/%d)", player->Name, ItemList[player->Inventory[slot].Index].Name, _rand, chance);

	Log(p->Header.ClientId, LOG_COMP, "Sucesso na composição do item %s - %d %d %d %d %d %d %d - %d %d", ItemList[player->Inventory[slot].Index].Name,
		p->items[0].Index, p->items[0].EF1, p->items[0].EFV1, p->items[0].EF2, p->items[0].EFV2, p->items[0].EF3, p->items[0].EFV3,
		_rand, chance);

	LogPlayer(clientId, "Sucesso na composição do item %s para anct", ItemList[p->items[0].Index].Name);

	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
	return true;
}