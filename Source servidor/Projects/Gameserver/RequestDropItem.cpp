#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestDropItem(PacketHeader *Header)
{
	// fUNÇão original fica em : 42d484 : DESCOMPILAR
	p272 *p = (p272*)Header;

	return true; //se remover esse return = da pra jogar item no chão

	if(Mob[clientId].Mobs.Player.Status.curHP <= 0 || Status != USER_PLAY)
	{
		AddCrackError(clientId, 1, 14);
		SendHpMode(clientId);

		return true;
	}

	if(Trade.ClientId != 0)
	{
		RemoveTrade(Trade.ClientId);

		return true;
	}

	if(IsAutoTrading && p->invType == (int)SlotType::Storage)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_CantWhenAutoTrade]);

		return true;
	}
	
	if(AccessLevel == 0 || !IsAdmin)
		return true;

	INT32 refreshEquip = 0; // local897
	if(p->posX < 0 || p->posX >= 4096 || p->posY < 0 || p->posY >= 4096)
	{
		// error
		return true;
	}
	
	st_Item *slot; // local898
	INT32 dropPosX = p->posX, //local899
		  dropPosY = p->posY, // local900
		  itemGrid = GetEmptyItemGrid(&dropPosX, &dropPosY); // local901

	if(itemGrid == 0)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Drop_Here]);

		return true;
	}

	// 0042D65F
	if(p->invType == (int)SlotType::Equip)
	{
		if(p->InvSlot < 0 || p->InvSlot >= 16)
			return false;

		slot = &Mob[clientId].Mobs.Player.Equip[p->InvSlot];
		if(p->InvSlot < 8)
			refreshEquip = 1;
	}
	else if(p->invType == (int)SlotType::Inv)
	{
		if(p->InvSlot < 0 || p->InvSlot >= 60)
			return false;
		
		slot = &Mob[clientId].Mobs.Player.Inventory[p->InvSlot];
	}
	else if(p->invType == (int)SlotType::Storage)
	{
		if(p->InvSlot < 0 || p->InvSlot >= 120)
			return false;
		
		slot = &User.Storage.Item[p->InvSlot];
	}
	else
		return false;

	if(slot->Index <= 0 || slot->Index >= MAX_ITEMLIST)
		return true;
	 
	if(slot->Index == 0x1FC || slot->Index == 0x1FD || slot->Index == 0x20A || (slot->Index >= 0x20E && slot->Index < 0x219) || slot->Index == 0x1BE || slot->Index == 0x2EB)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Medal_Cant_Be_Droped]);

		return true;
	}

	INT32 tryCreateItem = CreateItem(p->posX, p->posY, slot, p->Unknown1, 1); //local902
	if (tryCreateItem >= 4096 || tryCreateItem <= 0)
	{
		SendClientMessage(clientId, "Can't create object(item)");

		return true;
	}

	Log(clientId, LOG_INGAME, "Derrubou o item %s [%d] [%d %d %d %d %d %d] em %dx %dy - InvType: %d - InvSlot: %d", 
		ItemList[slot->Index].Name, slot->Index, slot->Effect[0].Index, slot->Effect[0].Value,
		slot->Effect[1].Index, slot->Effect[1].Value, slot->Effect[2].Index, slot->Effect[2].Value,
		p->posX, p->posY, p->invType, p->InvSlot);

	memset(slot, 0, sizeof st_Item);

	p175 p2;
	memset(&p2, 0, sizeof p175);
	
	p2.Header.PacketId = 0x175;
	p2.Header.Size = sizeof p175;
	p2.Header.ClientId = 0xCCCC;

	p2.invType = p->invType;
	p2.invSlot = p->InvSlot;

	p2.posX = p->posX;
	p2.posY = p->posY;

	AddMessage((BYTE*)&p2, sizeof p175);

	if(Mob[clientId].Mobs.Player.Equip[6].Index == 0 && Mob[clientId].Mobs.Player.Equip[7].Index != 0)
	{
		p376 packet;
		packet.Header.PacketId = 0x376;
		packet.Header.Size = sizeof p376;
		packet.Header.ClientId = 0x7530;

		INT32 itemAbility = GetItemAbility(&Mob[clientId].Mobs.Player.Equip[7], 17); //local1663

		if (itemAbility != 128)
		{
			packet.DstType = 0;
			packet.DstSlot = 6;
			packet.SrcType = 0;
			packet.SrcSlot = 7;

			AddMessage((BYTE*)p, sizeof p376);

			memcpy(&Mob[clientId].Mobs.Player.Equip[6], &Mob[clientId].Mobs.Player.Equip[7], 8);
			memset(&Mob[clientId].Mobs.Player.Equip[7], 0, 8);
		}
	}

	if(p->invType == 0)
	{
		if(refreshEquip == 1)
			SendEquip(clientId);

		Mob[clientId].GetCurrentScore(clientId);
		
		SendScore(clientId);
	}

	return true;
}