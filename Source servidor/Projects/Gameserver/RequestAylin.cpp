#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAylin(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;
	
	st_Mob *player = &Mob[clientId].Mobs.Player;

	for(int i = 0;i<7;i++)
	{
		if(i >= 3 && i <= 6)
			if(p->slot[i] == -1)
				continue;

		if(p->slot[i] < 0 || p->slot[i] > 60)
		{			
			Log(clientId, LOG_HACK, "[HACK] Banido por enviar índice inválido - NPC Aylin - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "[HACK] %s - Banido por enviar índice inválido - NPC Aylin - %d", player->Name, p->slot[i]);
			
			SendCarry(clientId);

			return true;
		}

		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Aylin - %d", p->items[i].Index);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Aylin - %d", player->Name, p->items[i].Index);
			
			SendCarry(clientId);

			return true;
		}
		
		for(int y = 0; y < 7; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Aylin - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Aylin - %d", player->Name, p->items[i].Index);

				CloseUser(clientId);
				return true;
			}
		}
	}
	
	if(User.Block.Blocked)
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

	st_Item *items = p->items;

	if(ItemList[items[0].Index].Pos >= 256 || ItemList[items[1].Index].Pos == 0)
	{
		SendClientMessage(clientId, "Só é possível com armas ou sets");

		return true;
	}
	
	int sanc_1 = GetItemSanc(&items[0]);
	int sanc_2 = GetItemSanc(&items[1]);

	if(sanc_1 < 9 || sanc_2 < 9)
	{
		SendClientMessage(clientId, "Só é possível combinar com itens +9");

		return true;
	}

	for(int i = 0;i<2;i++)
	{
		if(ItemList[items[i].Index].Unique >= 41 && ItemList[items[i].Index].Unique <= 49)
		{
			if(ItemList[items[i].Index].Grade < 5 || ItemList[items[i].Index].Grade > 8)
			{
				SendClientMessage(clientId, "Utilize itens ancts na composição");
				return true;
			}
		}
	}

	if(items[0].Index != items[1].Index)
	{
		SendClientMessage(clientId, "Combinação incorreta.");

		return true;
	}

	if(player->Gold < 50000000)
	{
		SendClientMessage(clientId, "São necessários 50.000.000 de gold para continuar");

		return true;
	}

	int chance = 0;
	for(int i=3;i<7;i++)
	{
		if(items[i].Index >= 2441 && items[i].Index <= 2444)
			chance++;
		else 
		{
			SendClientMessage(clientId, "Incorreto");
			return true;
		}
	}

	if(chance == 4) 
	{
		if(items[3].Index != items[4].Index || items[3].Index != items[5].Index || items[3].Index != items[6].Index)
		{
			SendClientMessage(clientId, "As jóias devem ser iguais!");
			return true;
		}
	}
	else if(chance == 3) 
	{
		if(items[3].Index != items[4].Index || items[3].Index != items[5].Index)
		{
			SendClientMessage(clientId, "As jóias devem ser iguais!");
			return true;
		}
	} 
	else if(chance == 2)
	{
		if(items[3].Index != items[4].Index)
		{
			SendClientMessage(clientId, "As jóias devem ser iguais!");
			return true;
		}
	} 

	if(chance == 4)
		chance = 50;
	else if(chance == 3)
		chance = 32;
	else if(chance == 2)
		chance = 24;
	else if(chance == 1)
		chance = 12;
	else
		return true;

	int sanc = items[3].Index - 2211;
	if(sanc < 230 || sanc > 233)
	{
		SendClientMessage(clientId, "Invalidade . Contate a administração");

		return true;
	}
	
	chance += GetCompounderDeal(clientId);

	int _rand = (Rand() % 101);

	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	player->Gold -= 50000000;

	SendSignalParm(clientId, clientId, 0x3AF, player->Gold);
	
	for(int i = 0; i < 7;i++)
	{
		if(p->slot[i] == -1)
			continue;

		Log(clientId, LOG_COMP, "Aylin - Item %d %s %s", i, ItemList[items[i].Index].Name,  items[i].toString().c_str());
	}

	if(_rand > chance)
	{
		for(int i = 2;i < 7;i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);
			
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		SendClientMessage(clientId, "Houve uma falha na composição");
		LogPlayer(clientId, "Falha na composição de %s para +10", ItemList[p->items[0].Index].Name);

		SendNotice("Jogador %s falhou a composição do item %s +10", player->Name, ItemList[p->items[0].Index].Name);

		Log(clientId, LOG_COMP, "Aylin - Falha na composição - %d/%d", _rand, chance);
		Log(SERVER_SIDE, LOG_COMP, "Aylin - %s Falha na composição do item +10 %s", User.Username, ItemList[p->items[0].Index].Name);
		return true;
	}

	st_Item *Item1 = (st_Item*)&player->Inventory[p->slot[0]];
	st_Item *Item2 = (st_Item*)&player->Inventory[p->slot[1]];

	sItemData itemData = ItemList[Item1->Index];
	int rand2 = Rand() % 50;

	st_Item *newItem;
	if(rand2 <= 25)
	{
		memcpy(Item1, Item2, sizeof st_Item);
		memset(Item2, 0, sizeof st_Item);
	
		newItem = Item1;
	}
	else
	{
		memcpy(Item2, Item1, sizeof st_Item);
		memset(Item1, 0, sizeof st_Item);

		newItem = Item2;
	}

	if(ItemList[newItem->Index].Pos >= 64 && ItemList[newItem->Index].Pos != 128)
	{
		int newGrade = sanc - 230;
		int baseId = newItem->Index - (ItemList[newItem->Index].Grade - 5);

		newItem->Index = baseId + newGrade;
	}

	for(int i = 0;i<3;i++)
	{
		if(newItem->Effect[i].Index == 43 || (newItem->Effect[i].Index >= 116 && newItem->Effect[i].Index <= 125))
		{
			newItem->Effect[i].Value = sanc;

			break;
		}
	}

	for(int i = 2;i < 7;i++)
	{
		memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);
		
		SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}
	
	SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
	SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);

	char szTMP[1024];

	sprintf_s(szTMP, ".Jogador %s concluiu com sucesso a composição do item %s +10", player->Name, ItemList[newItem->Index].Name);
	SendNotice(szTMP);

	Log(clientId, LOG_COMP, "Aylin - Sucesso na composição do item %s +10 - (%d) %d %d %d %d %d %d - %d/%d", ItemList[newItem->Index].Name, newItem->Index, newItem->Effect[0].Index, 
		 newItem->Effect[0].Value,  newItem->Effect[1].Index,  newItem->Effect[1].Value, newItem->Effect[2].Index,  newItem->Effect[2].Value, _rand, chance);
	LogPlayer(clientId, "Sucesso na composição do item %s para +10",  ItemList[newItem->Index].Name);

	Log(SERVER_SIDE, LOG_COMP, "Aylin - %s - Sucesso na composição do item %s +10 - (%d) %d %d %d %d %d %d - %d/%d", User.Username, ItemList[newItem->Index].Name, newItem->Index, newItem->Effect[0].Index,
		newItem->Effect[0].Value, newItem->Effect[1].Index, newItem->Effect[1].Value, newItem->Effect[2].Index, newItem->Effect[2].Value, _rand, chance);

	SaveUser(clientId, 0);
	return true;
}