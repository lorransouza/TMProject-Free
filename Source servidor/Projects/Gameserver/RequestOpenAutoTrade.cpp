#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestOpenAutoTrade(PacketHeader *Header)
{
	p39A *p = (p39A*)(Header);

	INT32 idx = p->Index;
	if (Mob[clientId].Mobs.Player.Status.curHP == 0 || Users[clientId].Status != USER_PLAY)
	{
		SendHpMode(clientId);
		AddCrackError(clientId, 10, 87);
		return true;
	}

	if (idx <= 0 || idx >= MAX_PLAYER)
		return false;

	if (!Users[idx].IsAutoTrading)
		return false;

	SendClientMessage(idx, "%s abriu sua loja", Mob[clientId].Mobs.Player.Name);

	SendAutoTrade(clientId, idx);
	return true;
}