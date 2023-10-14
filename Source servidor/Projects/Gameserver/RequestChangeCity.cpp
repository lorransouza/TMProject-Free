#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestChangeCity(PacketHeader *Header)
{
	pMsgSignal *p = (pMsgSignal*)(Header);

	int cityIndex = p->Value;
	if(cityIndex < 0 || cityIndex >= 5)
		return false;

	Mob[clientId].Mobs.Player.Info.CityID = cityIndex;
	return true;
}