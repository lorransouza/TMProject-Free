#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestOpenShop(PacketHeader *Header)
{
	p27B *p = (p27B*)Header;
	if(p->npcID < 1000 || p->npcID >= MAX_SPAWN_MOB)
		return false;

	st_Mob *spw = &Mob[p->npcID].Mobs.Player;

	int merch = spw->bStatus.Merchant.Merchant;
	if(merch != 1 && merch != 3 && merch != 19)
		return false;

	if (Mob[p->npcID].GenerateID == 384 || Mob[p->npcID].GenerateID == 385)
	{
		p17E packet{};
		packet.Header.PacketId = 0x17E;
		for (size_t i = 0; i < sServer.RvR.Items.size(); ++i)
		{
			memcpy(&packet.Item[i], &sServer.RvR.Items[i].Item, sizeof st_Item);
			packet.Price[i] = sServer.RvR.Items[i].Price;
		}

		AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
		return true;
	}

	p17C packet;
	packet.Header.ClientId = 0x7530;
	packet.Header.PacketId = 0x17C;
	packet.Header.Size = sizeof p17C;

	INT32 cityZone = GetVillage(Mob[p->npcID].Target.X, Mob[p->npcID].Target.Y);
	if(cityZone == 5)
		cityZone = 4;

	packet.Taxes = g_pCityZone[cityZone].perc_impost;
	packet.Unknow = merch;

	for(int i = 0;i<27;i++)
		memcpy(&packet.Item[i], &spw->Inventory[(i % 9) + ((i / 9) * 27)], sizeof st_Item);

	if(Mob[p->npcID].Mobs.Player.bStatus.INT == 4001)
		packet.Taxes = 0;
	else
		packet.Taxes = g_pCityZone[cityZone].perc_impost;

	AddMessage((BYTE*)&packet, sizeof p17C);
	return true;
}