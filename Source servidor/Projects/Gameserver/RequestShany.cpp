#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestShany(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;
	
	bool sucess = false;

	st_Mob *player = &Mob[clientId].Mobs.Player;

	for(int i = 0;i < 3;i++)
	{
		if(p->slot[i] < 0 || p->slot[i] >= 60)
		{
			Log(clientId, LOG_HACK, "Banido por enviar índice inválido - NPC Shany - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar índice inválido - NPC Shany - %d", player->Name, p->slot[i]);
			
			SendCarry(clientId);
			return true;
		}

		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Ehre - %d", p->items[i].Index);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Shany - %d", player->Name, p->items[i].Index);
			
			SendCarry(clientId);
			return true;
		}

		for(int y = 0; y < 3; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Ehre - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Shany - %d", player->Name, p->items[i].Index);
				
				SendCarry(clientId);
				return true;
			}
		}
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	for(int i = 0 ; i < 3; i ++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Shany - %d - Sem item\n", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Shany - %d - %s %s - %hhd", i, ItemList[p->items[i].Index].Name, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	st_Item *items = (st_Item*)p->items;
	
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
	for(int i = 0; i < 3; i++)
	{
		int itemId = items[i].Index;
		if(itemId != 540 && itemId != 541)
		{
			SendClientMessage(clientId, "Insira a Pedra Espiritual [E] nos dois primeiros slots");

			return true;
		}

		if(i != 2)
		{
			int sanc = GetItemSanc(&items[i]);
			if(sanc != 9)
			{
				SendClientMessage(clientId, "Pedra Espiritual [E] deve estar +9");

				return true;
			}
		}
	}

	for(int i = 3; i < 7;i++)
	{
		if(items[i].Index != 413)
		{
			SendClientMessage(clientId, "Insira [04] Poeiras de Lactolerium");

			return true;
		}
	}

	if(player->Equip[0].EFV2 == 1 || (player->Equip[0].EFV2 == 2 && player->bStatus.Level < 255))
	{
		SendClientMessage(clientId, "Necessário ser no mínimo arch nivel 256");

		return true;
	}

	int _rand = Rand() % 100;

	for(int i = 0; i < 7; i ++)
	{
		Log(clientId, LOG_COMP, "NPC Shany - Espiritual: %s [%d] [%d %d %d %d %d %d] - Slot %d", ItemList[items[i].Index].Name, items[i].Index, items[i].EF1, items[i].EFV1, items[i].EF2,
			items[i].EFV2, items[i].EF3, items[i].EFV3, i);
	}

	if(_rand <= 60)
	{
		for(int i = 2; i < 7; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}
		
		SendClientMessage(clientId, "Houve uma falha na composição");

		Log(clientId, LOG_COMP, "Falha na composiçaõ Shany - %d", _rand);
		LogPlayer(clientId, "Falha na composição de Pedra Espiritual [F]");

		SaveUser(clientId, 0);
		return true;
	}

	SendClientMessage(clientId, "Sucesso na composição");

	if(player->Exp <= 2000000)
		player->Exp = 0;
	else
		player->Exp -= 2000000;

	if(player->Equip[0].EFV2 >= 3)
	{
		while(player->Exp < g_pNextLevel[2][player->bStatus.Level - 1])
			player->bStatus.Level --;
	}
	else
	{
		while(player->bStatus.Level > 0 && player->Exp < g_pNextLevel[0][player->bStatus.Level - 1])
			player->bStatus.Level --;
	}

	for(int i = 3; i < 7; i++)
	{
		memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);
	
		SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}
	
	memset(&player->Inventory[p->slot[0]], 0, sizeof st_Item);
	SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
	
	memset(&player->Inventory[p->slot[1]], 0, sizeof st_Item);
	SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);

	memset(&items[2], 0, sizeof st_Item);
	_rand = Rand() % 100;

	if(_rand <= 33)
		items[2].Index = 631;
	else if(_rand <= 66)
		items[2].Index = 632;
	else 
		items[2].Index = 633;

	memcpy(&player->Inventory[p->slot[2]], &items[2], sizeof st_Item);

	Log(clientId, LOG_COMP, "Sucesso na composição - SHANY : %d ", items[2].Index);
	LogPlayer(clientId, "Sucesso na composição de Pedra Espiritual [F]");

	SendItem(clientId, SlotType::Inv, p->slot[2], &player->Inventory[p->slot[2]]);

	Mob[clientId].GetCurrentScore(clientId);
	SendEtc(clientId);
	SendScore(clientId);

	SaveUser(clientId, 0);
	return true;
}