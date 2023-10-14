#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestTiny(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;
	
	st_Mob *player = &Mob[clientId].Mobs.Player;
	for(int i = 0;i<3;i++)
	{
		if(p->slot[i] == -1)
		{
			SendClientMessage(clientId, "Coloque todos os itens");
			return true;
		}

		if((p->slot[i] < 0 || p->slot[i] >= 60))
		{
			Log(clientId, LOG_HACK, "Banido por enviar índice inválido - NPC Tiny - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar índice inválido - NPC Tiny - %d", player->Name, p->slot[i]);
			
			SendCarry(clientId);
			return true;
		}
		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Tiny - %d", p->items[i].Index);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Tiny - %d", player->Name, p->items[i].Index);
			
			SendCarry(clientId);
			return true;
		}
		for(int y = 0; y < 3; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Tiny - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Tiny - %d", player->Name, p->items[i].Index);
				
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
			Log(clientId, LOG_COMP, "Tiny - %d - Sem item\n", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Tiny - %d - %s %s - %hhd", i, ItemList[p->items[i].Index].Name, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (Trade.ClientId)
	{
		RemoveTrade(clientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);

		return true;
	}

	st_Item *items = p->items;
	
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	int chanceTotal = 25;
	if (p->slot[2] != -1)
	{
		sItemData *itemD = (sItemData*)&ItemList[items[2].Index];

		int itemType = GetEffectValueByIndex(items[2].Index, EF_UNKNOW1);
		if (itemType < 4)
		{
			SendClientMessage(clientId, "Utilize um item [D] ou superior na composição!");
			return true;
		}

		chanceTotal += ((itemType - 3) * 4);
	}

	for(int i = 0; i < 2; i++)
	{		
		sItemData *item = (sItemData*)&ItemList[items[i].Index];
		if(item->Grade < 5 || item->Grade > 8)
		{
			SendClientMessage(clientId, "Utilize itens ancts na composição");

			return true;
		}

		int ref = GetItemSanc(&player->Inventory[p->slot[i]]);
		if(ref < 9)
		{
			SendClientMessage(clientId, "Utilize itens +9 na composição");

			return true;
		}
	}

	if(player->Gold < 100000000)
	{
		SendClientMessage(clientId, "São necessários 100 milhões de gold para a composição");

		return true;
	}

	int _rand = Rand() % 100;	
	player->Gold -= 100000000;

	SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

	if (p->slot[2] != -1)
	{
		memset(&player->Inventory[p->slot[2]], 0, sizeof st_Item);
		SendItem(clientId, SlotType::Inv, p->slot[2], &player->Inventory[p->slot[2]]);
	}

	if(_rand <= chanceTotal)
	{
		//Sucesso na composição
		st_Item *itemArch = (st_Item*)&player->Inventory[p->slot[0]];
		st_Item *itemMortal = (st_Item*)&player->Inventory[p->slot[1]];

		Log(clientId, LOG_COMP, "Tiny - Sucesso na transferência de add para %s [%d] [%d %d %d %d %d %d]", ItemList[itemArch->Index].Name, itemMortal->Index, itemMortal->EF1, itemMortal->EFV1,
			itemMortal->EF2, itemMortal->EFV2, itemMortal->EF3, itemMortal->EFV3);
		Log(SERVER_SIDE, LOG_COMP, "[%s] - Tiny - Sucesso na transferência de add para %s [%d] [%d %d %d %d %d %d]", 
			User.Username, ItemList[itemArch->Index].Name, itemMortal->Index, itemMortal->EF1, itemMortal->EFV1, itemMortal->EF2, itemMortal->EFV2, itemMortal->EF3, itemMortal->EFV3);
		LogPlayer(clientId, "Sucesso na transferência de adicional de %s para %s", ItemList[itemMortal->Index].Name, ItemList[itemArch->Index].Name);

		SendNotice(".%s transferiu com sucesso o adicional para %s", player->Name, ItemList[itemArch->Index].Name);
		SendClientMessage(clientId, "Sucesso na transferência de adicional de %s para %s (%d/%d)", ItemList[itemMortal->Index].Name, ItemList[itemArch->Index].Name, _rand, chanceTotal);
		itemMortal->Index = itemArch->Index;

		SetItemSanc(itemMortal, 7, 0);

		memset(itemArch, 0, sizeof st_Item);

		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
		SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);
	}
	else
	{
		SendNotice(".%s falhou na transferência o adicional para %s", player->Name, ItemList[p->items[0].Index].Name);
		SendClientMessage(clientId, "Falha na transferência de adicional (%d/%d)", _rand, chanceTotal);

		LogPlayer(clientId, "Falha na transferência de adicional de %s para %s", ItemList[p->items[0].Index].Name, ItemList[p->items[1].Index].Name);
		Log(clientId, LOG_COMP, "Falha na transferência de adicional de %s para %s", ItemList[p->items[0].Index].Name, ItemList[p->items[1].Index].Name);
		Log(SERVER_SIDE, LOG_COMP, "[%s] - Falha na transferência de adicional de %s para %s", User.Username, ItemList[p->items[0].Index].Name, ItemList[p->items[1].Index].Name);
	}

	SaveUser(clientId, 0);
	return true;
}