#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include <algorithm>

using namespace std::chrono_literals;

bool CUser::RequestDeleteItem(PacketHeader *Header)
{
	p2E4 *p = (p2E4*)Header;

	// Checagens de segurança padrão
	if(p->ItemID <= 0 || p->ItemID > MAX_ITEMLIST)
	{
		AddCrackError(clientId, 4, CRACK_USER_PKTHACK);

		return false;
	}

	if(p->SlotID < 0 || p->SlotID >= 60)
	{ 
		AddCrackError(clientId, 4, CRACK_USER_PKTHACK);

		return false;
	} 

	if(Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

	st_Item *item = &Mob[clientId].Mobs.Player.Inventory[p->SlotID];
	if(item->Index != p->ItemID)
	{
		SendItem(clientId, SlotType::Inv, p->SlotID, item);

		return true;
	}

	if(Trade.ClientId != 0)
	{
		RemoveTrade(Trade.ClientId);
		RemoveTrade(clientId);

		return true;
	}

	bool canDel = true;
	switch(item->Index)
	{
	case 509:
		canDel = false;
		break;
	}

	if(!canDel)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Delete_This_Item]);

		SendItem(clientId, SlotType::Inv, p->SlotID, item);
		return true;
	}

	auto [isFast, time] = CheckIfIsTooFast(item, p->SlotID);
	if (isFast)
	{
		Log(clientId, LOG_HACK, "O usuário deletou o item %s em %lldms.", item->toString().c_str(), time.count());
		Log(SERVER_SIDE, LOG_HACK, "O usuário %s deletou o item %s em %lldms.", User.Username, item->toString().c_str(), time.count());
	}

	auto now = std::chrono::steady_clock::now();
	auto timeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastDeletedItem);
	if (timeInMs <= 400ms)
	{
		Log(clientId, LOG_HACK, "O usuário deletou itens muito rapidamente. Tempo para deletar: %lld", timeInMs.count());
		Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuário deletou itens muito rapidamente. Tempo para deletar: %lld", User.Username, timeInMs.count());
	}

	Log(clientId, LOG_INGAME, "Item %s deletado [%d] [%d %d %d %d %d %d]. Slot %d", ItemList[item->Index].Name, item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, p->SlotID);
	LogPlayer(clientId, "Item %s deletado na lixeira do inventário", ItemList[item->Index].Name);

	memset(item, 0, sizeof st_Item);
	SendItem(clientId, SlotType::Inv, p->SlotID, item);

	Times.LastDeletedItem = now;
	return true;
}