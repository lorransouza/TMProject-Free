#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

// TODO: Verificar se ele está perto do guarda cargas para passar o gold.
bool CUser::RequestTransferGoldToInv(PacketHeader *Header)
{
	pMsgSignal *p = (pMsgSignal*)(Header);

	if(Mob[clientId].Mobs.Player.Status.curHP <= 0 || Status != USER_PLAY)
	{
		SendHpMode(clientId);
		return true;
	}

		
	if(Trade.ClientId != 0)
		RemoveTrade(clientId);

	UINT32 gold = p->Value;
	if(User.Storage.Coin < gold || gold < 0 || gold > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Withdraw_That_Much]);

		return true;
	}

	INT32 totalGold = Mob[clientId].Mobs.Player.Gold + gold;
	if(totalGold > 2000000000 || totalGold < 0)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		return true;
	}

	Mob[clientId].Mobs.Player.Gold = totalGold;
	User.Storage.Coin -= gold;

	p->Header.ClientId = 0x7530;

	AddMessage((BYTE*)p, sizeof pMsgSignal);
	SendCargoCoin(clientId);
	
	Log(clientId, LOG_INGAME, "Transferiu %d de gold para o inventário. %d gold no banco. %d no inventário", p->Value, User.Storage.Coin, totalGold);
	LogPlayer(clientId, "Transferiu %d de gold para o inventário. %d gold no banco. %d no inventário", p->Value, User.Storage.Coin, totalGold);

	SaveUser(clientId, 0);
	return true;
}


bool CUser::RequestTransferGoldToBank(PacketHeader *Header)
{
	pMsgSignal *p = (pMsgSignal*)(Header);

	if(Mob[clientId].Mobs.Player.Status.curHP <=0 || Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

	if(Trade.ClientId != 0)
		RemoveTrade(clientId);

	INT32 gold = p->Value;

	if(Mob[clientId].Mobs.Player.Gold < gold || gold < 0 || gold > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Deposit_That_Much]);

		return true;
	}

	INT32 totalGold = User.Storage.Coin + gold;
	if(totalGold < 0 || totalGold > 2000000000)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);

		return true;
	}

	Mob[clientId].Mobs.Player.Gold -= gold;

	User.Storage.Coin = totalGold;

	AddMessage((BYTE*)p, sizeof pMsgSignal);
		
	p->Header.ClientId = 0x7530;
	SendCargoCoin(clientId);
	
	Log(clientId, LOG_INGAME, "Transferiu %d de gold para o banco. %d gold no banco. %d no inventário", p->Value, totalGold, Mob[clientId].Mobs.Player.Gold);
	LogPlayer(clientId, "Transferiu %d de gold para o banco. %d gold no banco. %d no inventário", p->Value, totalGold, Mob[clientId].Mobs.Player.Gold);

	SaveUser(clientId, 0);
	return true;
}