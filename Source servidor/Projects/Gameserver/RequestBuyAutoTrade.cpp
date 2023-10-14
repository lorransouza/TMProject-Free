#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestBuyAutoTrade(PacketHeader *Header)
{
	p398 *p = (p398*)(Header);

	st_Mob *player = (st_Mob*)(&Mob[clientId].Mobs.Player);

	if(!player->Status.curHP || Status != USER_PLAY)
		return false;

	if(Users[clientId].Trade.ClientId != 0)
	{
		RemoveTrade(clientId);
		
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	INT32 mobId = p->mobid;
	INT32 itemPrice = p->price;
	INT32 itemTaxe = p->unk;
	INT32 slotId = p->slot;

	if(mobId <= 0 || mobId >= MAX_PLAYER)
		return false;
	
	st_Mob *mob = (st_Mob*)(&Mob[mobId].Mobs.Player);

	if(Users[mobId].Status != USER_PLAY)
		return false;

	if (Mob[clientId].Target.X < Mob[mobId].Target.X - VIEWGRIDX || Mob[clientId].Target.X > Mob[mobId].Target.X + VIEWGRIDX || Mob[clientId].Target.Y < Mob[mobId].Target.Y - VIEWGRIDY || Mob[clientId].Target.Y > Mob[mobId].Target.Y + VIEWGRIDY)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_SoFar_BuyShop]);

		return true;
	}

	// q?
	mobId = mobId;

	if(slotId < 0 || slotId >= 12)
		return false;

	INT32 slotBank = Users[mobId].AutoTrade.Slots[slotId];
	if(slotBank < 0 || slotBank >= 128)
		return false;

	if(itemTaxe != Users[mobId].AutoTrade.Unknown_1784)
		return false;

	if(itemPrice != Users[mobId].AutoTrade.Price[slotId])
		return false;

	if(Users[mobId].AutoTrade.Item[slotId].Index == 0)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_AlwaysBuy_BuyShop]);

		return true;
	}

	INT32 itemCheck = memcmp(&p->Item, &Users[mobId].AutoTrade.Item[slotId], 8);
	if(itemCheck != 0)
	{
		RemoveTrade(clientId);
		return true;
	}

	itemCheck = memcmp(&Users[mobId].User.Storage.Item[slotBank], &Users[mobId].AutoTrade.Item[slotId], 8);
	if(itemCheck != 0)
	{
		RemoveTrade(clientId);

		return true;
	}

	if(player->Gold < itemPrice)
	{ // Gold insuficiente
		SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_BuyShop]);

		return true;
	}

	if(itemPrice <= 1000) 
	{
		SendClientMessage(clientId, "Este item está em modo demonstração");

		return true;
	}

	st_Item *item = (st_Item*)&p->Item;
	
	INT32 nullSlot = GetFirstSlot(clientId, 0);
	if(nullSlot < 0 || nullSlot >= MAX_INVEN)
	{//sem espaço no inventário
		SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

		return true;
	}
	
	INT32 impost = 0;
	INT32 totalPrice = itemPrice; // local1190
	
	INT32 mobTargetX = Mob[mobId].Target.X; // local1178
	INT32 mobTargetY = Mob[mobId].Target.Y; // local1179
	INT32 villageId = GetVillage(mobTargetX, mobTargetY); // local1180

	if (villageId < 0 || villageId >= 5)
	{ // Permitido somente dentro da cidade
		SendClientMessage(clientId, g_pLanguageString[_NN_OnlyVillage]);
		return true;
	}

	INT32 tax = g_pCityZone[villageId].perc_impost; 
	if(itemPrice >= 100000)
	{
		impost = itemPrice / 100 * tax;

		totalPrice = itemPrice - impost;
	}

	UINT64 total = Users[mobId].User.Storage.Coin + totalPrice;
	if(total > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		return true;
	}

	g_pCityZone[villageId].impost += impost;

	memcpy(&player->Inventory[nullSlot], item, 8);

	SendItem(clientId, SlotType::Inv, nullSlot, &player->Inventory[nullSlot]);

	Users[mobId].AutoTrade.Slots[slotId] = -1; // -1;
	memset(&Users[mobId].AutoTrade.Item[slotId], 0, 8);
	Users[mobId].AutoTrade.Price[slotId] = 0;
	memset(&Users[mobId].User.Storage.Item[slotBank], 0, 8);


	SendItem(mobId, SlotType::Storage, slotBank, &Users[mobId].User.Storage.Item[slotBank]);

	player->Gold = player->Gold - itemPrice;

	if(Users[mobId].User.Storage.Coin < 2000000000)
		Users[mobId].User.Storage.Coin += totalPrice;
	
	SendSignalParm(clientId, clientId, 0x3AF, player->Gold);
	SendCargoCoin(mobId);

	p39B sm;
	sm.Header.PacketId = 0x39B;
	sm.Header.ClientId = 0x7530;

	sm.MobID = mobId;
	sm.SlotID = slotId;

	sm.Header.Size = 20;

	GridMulticast_2(mobTargetX, mobTargetY, (BYTE*)&sm, 0);

	SendClientMessage(p->mobid, "!O personagem [%s] comprou o item %s por %d", player->Name, ItemList[p->Item.Index].Name, p->price);
	
	Log(clientId, LOG_INGAME, "Comprou o item %s [%d] [%d %d %d %d %d %d] de %s (%s) por %d. Gold sobrando: %d - Impostos pago: %d", ItemList[item->Index].Name, item->Index, item->Effect[0].Index,
		item->Effect[0].Value, item->Effect[1].Index, item->Effect[1].Value, item->Effect[2].Index, item->Effect[2].Value, mob->Name, Users[p->mobid].User.Username, p->price,
		player->Gold, impost);

	Log(p->mobid, LOG_INGAME, "Vendeu o item %s [%d] [%d %d %d %d %d %d] para %s (%s) por %d. Gold banco: %d - Impostos: %d", ItemList[item->Index].Name, item->Index, item->Effect[0].Index,
		item->Effect[0].Value, item->Effect[1].Index, item->Effect[1].Value, item->Effect[2].Index, item->Effect[2].Value, player->Name, Users[clientId].User.Username,  p->price, Users[p->mobid].User.Storage.Coin,
		impost);

	LogPlayer(clientId, "Comprou o item %s da loja de %s por %d", ItemList[item->Index].Name, Mob[p->mobid].Mobs.Player.Name, p->price);
	LogPlayer(p->mobid, "Vendeu o item %s de sua loja para %s por %d. %d foram pagos de impostos", ItemList[item->Index].Name, Mob[clientId].Mobs.Player.Name, p->price, impost);

	if(impost > 50000000)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Pagos um total de %d de impostos... Valor maior que 50milhões.", impost);
		Log(SERVER_SIDE, LOG_INGAME, "[%s] Vendeu o item %s [%d] [%d %d %d %d %d %d] para %s (%s) por %d. Gold banco: %d - Impostos: %d", User.Username, ItemList[item->Index].Name, item->Index, item->Effect[0].Index,
			item->Effect[0].Value, item->Effect[1].Index, item->Effect[1].Value, item->Effect[2].Index, item->Effect[2].Value, player->Name, Users[clientId].User.Username,  p->price, Users[p->mobid].User.Storage.Coin,
			impost);
	}

	if (Users[p->mobid].PremierStore.Status && itemPrice >= 10000000)
	{
		Users[p->mobid].PremierStore.Time += (5 * 60);

		Log(clientId, LOG_INGAME, "Recebeu 5 minutos extras por vender item com preço acima de 10 milhões na loja!");
	}

	SaveUser(clientId, 0);
	SaveUser(p->mobid, 0);
	return true;
}