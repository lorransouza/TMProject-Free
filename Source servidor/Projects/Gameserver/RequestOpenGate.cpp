#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestOpenGate(PacketHeader *Header)
{ // 00430C81
	p374 *p = (p374*)Header; // LOCAL1154

	if(!Mob[clientId].Mobs.Player.Status.curHP || Users[clientId].Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

	INT32 gateId = p->gateId - 10000; // LOCAL1155
	if(gateId < 0 || gateId >= 4096)
		return false;

	if (gateId == 24)
	{
		SendClientMessage(clientId, "Monte o seu grupo e utilize o Zakum para ter acesso a quest.");

		return true;
	}

	INT32 packetStatus = p->status ; // LOCAL1156
	INT32 initItemStatus = pInitItem[gateId].Status; // LOCAL1157

	if (initItemStatus == 3 || packetStatus == 3)
	{
		INT32 itemAbility = GetItemAbility(&pInitItem[gateId].Item, EF_KEYID); // LOCAL1159
		INT32 keySlot = 0; // LOCAL1160

		if (itemAbility != 0)
		{
			for(INT32 i = 0; i < 64; i++) // LOCAL1161
			{
				keySlot = GetItemAbility(&Mob[clientId].Mobs.Player.Inventory[i], EF_KEYID);
				if (keySlot == itemAbility)
				{
					memset(&Mob[clientId].Mobs.Player.Inventory[i], 0, sizeof st_Item);

					SendItem(clientId, SlotType::Inv, i, &Mob[clientId].Mobs.Player.Inventory[i]);
					break;
				}
			}

			// 00430E1A
			if (keySlot != itemAbility)
			{
				if(pInitItem[gateId].Item.Index != 773)
					SendClientMessage(clientId, g_pLanguageString[_NN_No_Key]);

				return true;
			}
		}
	}

	INT32 height = 0; // LOCAL1162

	INT32 updateItem = UpdateItem(gateId, packetStatus, &height); // LOCAL1163
	if (updateItem == 0)
		return true;

	p->unknow = height;

	GridMulticast_2(pInitItem[gateId].PosX, pInitItem[gateId].PosY, (BYTE*)p, 0);

	return true;
}