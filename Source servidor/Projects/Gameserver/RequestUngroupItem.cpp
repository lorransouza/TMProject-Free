#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"

bool CUser::RequestUngroupItem(PacketHeader *Header)
{
	p2E5 *p = (p2E5*)Header;

	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return false;

	if (Trade.ClientId != 0)
	{
		RemoveTrade(p->Header.ClientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	if (p->SlotID < 0 || p->SlotID >= 60)
		return false;
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	INT32 amount = GetItemAmount(&Mob[clientId].Mobs.Player.Inventory[p->SlotID]);
	if(amount < p->Amount) // Packet alterado 
	{
		SendItem(clientId, SlotType::Inv, p->SlotID, &Mob[clientId].Mobs.Player.Inventory[p->SlotID]);

		return true;
	}

	if(p->Amount <= 0) 
		return false;

	if(Mob[clientId].Mobs.Player.Inventory[p->SlotID].Index != p->ItemID) 
	{
		SendItem(clientId, SlotType::Inv, p->SlotID, &Mob[clientId].Mobs.Player.Inventory[p->SlotID]);

		return true;
	}

	int Slot = GetFirstSlot(clientId, 0);
	if(Slot == -1) 
	{
		SendClientMessage(clientId, "Não há espaço para separar!");

		return true;
	}

	bool canUngroup = false;
	switch (p->ItemID)
	{
		case 412:
		case 413:
		case 3200:
		case 3201:
		case 3202:
		case 3203:
		case 3204:
		case 3205:
		case 3206:
		case 3207:
		case 3208:
		case 3209:
		case 3214:
		case 2390:
		case 2391:
		case 2392:
		case 2393:
		case 2394:
		case 2395:
		case 2396:
		case 2397:
		case 2398:
		case 2399:
		case 2400:
		case 2401:
		case 2402:
		case 2403:
		case 2404:
		case 2405:
		case 2406:
		case 2407:
		case 2408:
		case 2409:
		case 2410:
		case 2411:
		case 2412:
		case 2413:
		case 2414:
		case 2415:
		case 2416:
		case 2417:
		case 2418:
		case 2419:
		case 415:
		case 419:
		case 420:
		case 4550:
		case 4140:
		case 4158:
		case 4159:
		case 3314:
		case 4599:
		case 4601:
		case 4603:
		case 4016:
		case 4017:
		case 4018:
		case 4019:
		case 4020:
		case 777:
		case 3182:
		case 4850:
		case 4547:
		case 4528:
		case 3252:
		case 3253:
		case 1739:
		case 4641:
		case 4719:
			canUngroup = true;
			break;
	}

	if (!canUngroup)
	{
		SendClientMessage(p->Header.ClientId, "Não foi possível desagrupar");

		return true;
	}
	
	for(int i = 0;i < 3;i++)
	{
		if(Mob[clientId].Mobs.Player.Inventory[p->SlotID].Effect[i].Index == EF_NOTRADE)
		{
			SendClientMessage(clientId, "Item imóvel!");

			return true;
		}
	}
	
	for(int i = 0;i < 3;i++)
	{
		if(Mob[clientId].Mobs.Player.Inventory[p->SlotID].Effect[i].Index == EF_AMOUNT)
		{
			if(Mob[clientId].Mobs.Player.Inventory[p->SlotID].Effect[i].Value < p->Amount)
			{
				SendClientMessage(p->Header.ClientId, "Não pode separar mais do que se tem.");

				return true;
			}

			Mob[clientId].Mobs.Player.Inventory[p->SlotID].Effect[i].Value -= p->Amount;
			break;
		}
	}
	
	Log(clientId, LOG_INGAME, "Desagrupando o item %s. Tirando %d unidades. Novo item colocado no slot %d", Mob[clientId].Mobs.Player.Inventory[p->SlotID].toString().c_str(), p->Amount, Slot);

	memset(&Mob[clientId].Mobs.Player.Inventory[Slot], 0, sizeof st_Item);

	Mob[clientId].Mobs.Player.Inventory[Slot].Index = p->ItemID;
	Mob[clientId].Mobs.Player.Inventory[Slot].EF1 = EF_AMOUNT;
	Mob[clientId].Mobs.Player.Inventory[Slot].EFV1 = p->Amount;
	
	SendItem(clientId, SlotType::Inv, p->SlotID, &Mob[clientId].Mobs.Player.Inventory[p->SlotID]);
	SendItem(clientId, SlotType::Inv, Slot, &Mob[clientId].Mobs.Player.Inventory[Slot]);
	return true;
}