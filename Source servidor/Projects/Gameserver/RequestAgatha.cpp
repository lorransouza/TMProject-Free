#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAgatha(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	st_Mob *player = &Mob[clientId].Mobs.Player;

	for(int i = 0; i < 6;i++)
	{
		if(p->slot[i] == -1)
			continue;

		if(p->slot[i] < 0 || p->slot[i] > 60)
		{
			SendClientMessage(clientId, "Inválido");

			return true;
		}

		for(int y = 0; y < 6; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, " Banido por enviar item com mesmo slotId - NPC Agatha - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Agatha - %d", player->Name, p->items[i].Index);
				
				SendCarry(clientId);
				return true;
			}
		}

		if(!memcmp(&p->items[i], &player->Inventory[p->slot[i]], sizeof st_Item))
			continue;

		SendClientMessage(clientId, "Inválido");
		return true;
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	for(int i = 0 ; i < 6; i ++)
	{
		Log(clientId, LOG_COMP, "Agatha - %d - %s [%d] [%d %d %d %d %d %d] - %d", i, ItemList[p->items[i].Index].Name, p->items[i].Index,
			p->items[i].Effect[0].Index, p->items[i].Effect[0].Value, p->items[i].Effect[1].Index, p->items[i].Effect[1].Value, p->items[i].Effect[2].Index,
			p->items[i].Effect[2].Value, p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	st_Item *item = p->items;

	if(GetEffectValueByIndex(item[0].Index, EF_MOBTYPE) != 1)
	{
		SendClientMessage(clientId, "Utilize apenas itens archs na composição");
		return true;
	}

	int itemClass = GetEffectValueByIndex(item[1].Index, EF_UNKNOW1);
	if(itemClass < 4)
	{
		SendClientMessage(clientId, "Utilize apenas itens [D] ou superior");
		return true;
	}

	const sItemData& itemArch = ItemList[item[0].Index];
	const sItemData& itemMortal = ItemList[item[1].Index];

	if(GetItemSanc(&item[0]) >= 10)
	{
		SendClientMessage(clientId, "Não é possível transferir para itens acima de +9");

		return true;
	}

	if(itemArch.Pos != itemMortal.Pos)
	{
		SendClientMessage(clientId, "Os itens devem ter o mesmo tipo");
		return true;
	}

	if(itemArch.Pos > 32)
	{
		SendClientMessage(clientId, "Posição do item inválido");

		return true;
	}
	
	int reqClassArch = GetEffectValueByIndex(item[0].Index, EF_CLASS);
	int reqClassMortal = GetEffectValueByIndex(item[1].Index, EF_CLASS);

	if(reqClassArch != reqClassMortal)
	{
		SendClientMessage(clientId, "Necessário ser da mesma classe");

		return true;
	}
	for(int i = 2;i < 6;i++)
	{
		if(item[i].Index != 3140)
		{
			SendClientMessage(clientId, "Utilize 4 Pedras da Luz");

			return true;
		}
	}
	
	int _rand = Rand() % 100;
	int totalChance = 12;
	int grade = ItemList[item[1].Index].Grade;

	constexpr std::array rateByGrade = { 20, 25, 35, 45 };

	if (grade >= 1 && grade <= 4 && itemClass == 4)
		totalChance = rateByGrade[grade - 1];
	else if (itemClass == 5)
		totalChance = 26;

	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	if(_rand <= totalChance)
	{
		Log(clientId, LOG_COMP, "Agatha - Sucesso na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			ItemList[item[1].Index].Name, item[1].Index, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, ItemList[item[0].Index].Name,
			item[0].Index, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);

		Log(SERVER_SIDE, LOG_COMP, "[%s] - Agatha - Sucesso na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			User.Username, ItemList[item[1].Index].Name, item[1].Index, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, ItemList[item[0].Index].Name,
			item[0].Index, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);

		LogPlayer(clientId, "Sucesso na transferência de adicional de %s para %s", ItemList[item[1].Index].Name, ItemList[item[0].Index].Name);

		int sanc = GetItemSanc(&item[1]);
		for(int i = 0 ;i < 3;i++)
		{
			if(item[1].Effect[i].Index == EF_SANC || (item[1].Effect[i].Index >= 116 && item[1].Effect[i].Index <= 125))
			{
				item[1].Effect[i].Value = 7;

				break;
			}
		}

		int index = item[1].Index;
		item[1].Index = item[0].Index;

		SendNotice(".%s transferiu com sucesso adicional para %s", player->Name, ItemList[item[0].Index].Name);
		SendClientMessage(clientId, "Jogador %s obteve sucesso na composição de %s (%d/%d)", player->Name, ItemList[item[1].Index].Name, _rand, totalChance);

		for(int i = 2;i < 6;i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);
			
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		memset(&player->Inventory[p->slot[1]], 0, sizeof st_Item);
		memcpy(&player->Inventory[p->slot[0]], &item[1], sizeof st_Item);
		
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
		SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);
	}
	else
	{
		memset(&player->Inventory[p->slot[0]], 0, sizeof st_Item);

		SendNotice(".%s falhou na transferência de adicional para %s", player->Name, ItemList[item[0].Index].Name);
		SendClientMessage(clientId, "Falha na composição do item. (%d/%d)", _rand, totalChance);

		for(int i = 2;i<6;i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);
			
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}
		
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		LogPlayer(clientId, "Falha na transferência de adicional de %s para %s", ItemList[item[1].Index].Name, ItemList[item[0].Index].Name);
		Log(clientId, LOG_COMP, "Agatha - Falha na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			ItemList[item[1].Index].Name, item[1].Index, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, ItemList[item[0].Index].Name, 
			item[0].Index, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);

		Log(SERVER_SIDE, LOG_COMP, "[%s] - Agatha - Falha na transferência de add: %s [%d] [%d %d %d %d %d %d] para %s [%d] [%d %d %d %d %d %d] - %d/%d",
			User.Username, ItemList[item[1].Index].Name, item[1].Index, item[1].EF1, item[1].EFV1, item[1].EF2, item[1].EFV2, item[1].EF3, item[1].EFV3, ItemList[item[0].Index].Name,
			item[0].Index, item[0].EF1, item[0].EFV1, item[0].EF2, item[0].EFV2, item[0].EF3, item[0].EFV3, _rand, totalChance);
	}

	SaveUser(clientId, 0);
	return true;
}