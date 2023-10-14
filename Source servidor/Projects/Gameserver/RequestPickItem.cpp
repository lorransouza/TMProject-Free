#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include "UOD_EventManager.h"
#include "UOD_HappyHarvest.h"

bool CUser::RequestPickItem(PacketHeader *Header)
{
	p270 *p = (p270*)Header;

	constexpr std::array blockedToPick = { 1727, 4700, 4701, 794, 795, 796, 797, 798,  4703, 4705, 4704};
	if(Mob[clientId].Mobs.Player.Status.curHP <= 0 || Users[clientId].Status != USER_PLAY)
	{
		AddCrackError(clientId, 1, CRACK_USER_STATUS);

		return true;
	}

	if(p->invType != (int)SlotType::Inv)
	{
		AddCrackError(clientId, 1, CRACK_USER_STATUS);

		return true;
	}
	
	INT32 initItemId = p->initID - 10000; // LOCAL843

	if (initItemId < 0 || initItemId > 4096)
		return true;

	if (initItemId < 0 || initItemId >= 4096 || pInitItem[initItemId].Open == 0)
	{
		if (pInitItem[initItemId].Open != 0)
		{
			Log(SERVER_SIDE, LOG_ERROR, "PickItem - Mudança de status inesperado - OPEN.");
			// 0042CA41
		}

		p16F packet;
		memset(&packet, 0, sizeof p16F);

		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;

		packet.initID = p->initID;

		AddMessage((BYTE*)&packet, sizeof p16F);
		return true;
	}

	// 0042CAF0
	if (Mob[clientId].Target.X < pInitItem[initItemId].PosX - 3 || Mob[clientId].Target.Y < pInitItem[initItemId].PosY - 3 || Mob[clientId].Target.X > pInitItem[initItemId].PosX + 3 || Mob[clientId].Target.Y > pInitItem[initItemId].PosY + 3)
	{
		Log(clientId, LOG_ERROR, "Tentando pegar o item do chão muito longe.");
		return true;
	}

	// 0042CC2C
	st_Item *item = &pInitItem[initItemId].Item; // LOCAL848

	// 0042CBFF
	if (std::find(blockedToPick.begin(), blockedToPick.end(), item->Index) != std::end(blockedToPick) && !IsAdmin)
		return true;

	INT32 itemIndex = item->Index;
	if(itemIndex <= 0 || itemIndex >= MAX_ITEMLIST)
		return true;

	if(item->Index == 470)
	{
		if(Mob[clientId].Mobs.Info.Pilula)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

			return true;
		}

		//0042CCEA
		SendClientMessage(clientId, g_pLanguageString[_NN_Get_Skill_Point]);

		Mob[clientId].Mobs.Info.Pilula = 1;

		INT32 initItemPosX = pInitItem[initItemId].PosX; // LOCAL852
		INT32 initItemPosY = pInitItem[initItemId].PosY; // LOCAL853

		memset(item, 0, sizeof st_Item);

		g_pItemGrid[initItemPosY][initItemPosX] = 0;

		pInitItem[initItemId].Open = 0;

		p16F packet;
		memset(&packet, 0, sizeof p16F);

		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;

		packet.initID = initItemId + 10000;

		AddMessage((BYTE*)&packet, sizeof p16F);

		GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&packet, 0);
		
		SendEmotion(clientId, 14, 3);
		return true;
	}

	if(itemIndex >= 490 && itemIndex <= 500)
	{
		INT32 initItemPosX = pInitItem[initItemId].PosX; // LOCAL858
		INT32 initItemPosY = pInitItem[initItemId].PosY; // LOCAL859

		g_pItemGrid[initItemPosY][initItemPosX] = 0;
		pInitItem[initItemId].Open = 0;

		p16F packet;
		memset(&packet, 0, sizeof p16F);

		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;

		packet.initID = initItemId + 10000;

		AddMessage((BYTE*)&packet, sizeof p16F);

		GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&packet, 0);
	}

	if (itemIndex == 4706)
	{
		auto happyHarvest = static_cast<TOD_HappyHarvest*>(TOD_EventManager::GetInstance().GetEventItem(TOD_EventType::HappyHarvest));
		if (happyHarvest != nullptr)
		{
			if (happyHarvest->PickItem(*this, initItemId))
			{
				INT32 initItemPosX = pInitItem[initItemId].PosX; // LOCAL858
				INT32 initItemPosY = pInitItem[initItemId].PosY; // LOCAL859

				g_pItemGrid[initItemPosY][initItemPosX] = 0;
				pInitItem[initItemId].Open = 0;

				p16F packet;
				memset(&packet, 0, sizeof p16F);

				packet.Header.PacketId = 0x16F;
				packet.Header.Size = sizeof p16F;

				packet.initID = initItemId + 10000;

				AddMessage((BYTE*)&packet, sizeof p16F);

				GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&packet, 0);
			}

			return true;
		}
	}

	auto gift = std::find(std::begin(sServer.Christmas.itemsOnGround), std::end(sServer.Christmas.itemsOnGround), initItemId);
	if (gift != std::end(sServer.Christmas.itemsOnGround) && pInitItem[initItemId].Item.Index == 4718)
	{
		auto now = std::chrono::steady_clock::now();

		if (now - Christmas.Tree.LastBox <= 5min)
		{
			SendClientMessage(clientId, "Somente um presente a cada 5 minutos");

			return true;
		}

		// Está pegando um presente do chão
		int rand = Rand() % 5;
		if (rand == 1)
		{
			int amount = Rand() % 3 + 1;
			for (int i = 0; i < amount; ++i)
			{
				int slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					SendClientMessage(clientId, "Sem espaço no inventário");

					return true;
				}

				st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
				memset(item, 0, sizeof st_Item);

				item->Index = 4707;

				SendItem(clientId, SlotType::Inv, slotId, item);
				SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item->Index].Name);

				Log(clientId, LOG_INGAME, "Coletou a %s do Presente de Natal. Quantidade: %d", ItemList[item->Index].Name, amount);
			}
		}
		else
		{
			int amount = Rand() % 6 + 1;
			for (int i = 0; i < amount; i++)
			{
				// Bola de Neve I
				int slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					SendClientMessage(clientId, "Sem espaço no inventário");

					continue;
				}

				st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
				memset(item, 0, sizeof st_Item);

				item->Index = 4706;

				SendItem(clientId, SlotType::Inv, slotId, item);
				SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item->Index].Name);

				Log(clientId, LOG_INGAME, "Coletou a %s. Total recebido: %d", ItemList[item->Index].Name, amount);
			}
		}

		INT32 initItemPosX = pInitItem[initItemId].PosX; // LOCAL858
		INT32 initItemPosY = pInitItem[initItemId].PosY; // LOCAL859

		g_pItemGrid[initItemPosY][initItemPosX] = 0;
		pInitItem[initItemId].Open = 0;

		p16F packet;
		memset(&packet, 0, sizeof p16F);

		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;
		packet.initID = initItemId + 10000;

		AddMessage((BYTE*)&packet, sizeof p16F);

		GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&packet, 0);

		Christmas.Tree.LastBox = now;

		for (auto& user : Users)
		{
			if (user.clientId == clientId)
				continue;

			if (memcmp(MacAddress, user.MacAddress, 8) != 0)
				continue;

			user.Christmas.Tree.LastBox = now;
			Log(user.clientId, LOG_INGAME, "A conta %s pegou o presente. Esta conta está relacionada e não será possível coletar",
				User.Username);
			Log(clientId, LOG_INGAME, "Conta %s associada. Não será possível coletar novamente", user.User.Username);
		}

		return true;
	}
	
	INT32 initItemPosX = pInitItem[initItemId].PosX;
	INT32 initItemPosY = pInitItem[initItemId].PosY;
	
	p16F sm; // LOCAL872
	sm.Header.PacketId = 0x16F;
	sm.Header.Size = sizeof p16F;
	sm.Header.ClientId = 0x7530;
	sm.initID = p->initID;

	if(initItemPosX < 0 || initItemPosX >= 4096 || initItemPosY <= 0 || initItemPosY >= 4096)
	{
		AddMessage((BYTE*)&sm, sizeof p16F);
		
		pInitItem[initItemId].Open = 0;
		return true;
	}

	if (g_pItemGrid[initItemPosY][initItemPosX] != initItemId)
	{
		AddMessage((BYTE*)&sm, sizeof p16F);

		if(g_pItemGrid[initItemPosY][initItemPosX] == 0)
			g_pItemGrid[initItemPosY][initItemPosX] = initItemId;

		return true;
	}

	// 0042D06F
	if(p->posX != initItemPosX || p->posY != initItemPosY)
	{
		AddMessage((BYTE*)&sm, sizeof p16F);

		return true;
	}

	INT32 itemAbility = GetItemAbility(item, EF_VOLATILE); // local880

	int slotId = GetFirstSlot(clientId, 0);
	if (slotId == -1)
	{
		SendClientMessage(clientId, "Sem espaço no inventário");

		return true;
	}

	if (itemAbility == 2)
	{
		// 0042D0E8
		// Não feito, não cai mais gold no chão
	}
	else
	{
		st_Item *invSlotItem = &Mob[clientId].Mobs.Player.Inventory[slotId]; // LOCAL864

		memcpy(invSlotItem, item, sizeof st_Item);
		memset(item, 0, sizeof st_Item);

		SendItem(clientId, SlotType::Inv, slotId, invSlotItem);
		Log(clientId, LOG_INGAME, "Pegou o item %s %s do chão", ItemList[invSlotItem->Index].Name, invSlotItem->toString().c_str());
	}

	GridMulticast_2(initItemPosX, initItemPosY, (BYTE*)&sm, 0);

	g_pItemGrid[initItemPosY][initItemPosX] = 0;
	pInitItem[initItemId].Open = 0;

	return true;
}