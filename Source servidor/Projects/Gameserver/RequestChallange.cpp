#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestChallange(PacketHeader *Header)
{
	pMsgSignal2 *p = (pMsgSignal2*)(Header);

	INT32 mobId = p->Value;
	INT32 gold = p->Value2;

	if (mobId <= 0 || mobId >= MAX_SPAWN_MOB)
		return false;	
	
	if(gold < 0)
		return false;

	INT32 merchantId = Mob[mobId].Mobs.Player.Info.Merchant - 6;
	if (merchantId < 0 || merchantId >= 5)
		return true;

	if (merchantId == 4)
		return true;

	Challange(clientId, mobId, gold);
	return true;
}
