#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestMotion(PacketHeader *Header)
{
	p36A *p = (p36A*)Header;

	if (Mob[clientId].Mobs.Player.Status.curHP == 0 || Users[clientId].Status != 22)
	{
		SendHpMode(clientId);

		AddCrackError(clientId, 4, 6);
		return true;
	}
	
	p->Header.ClientId = clientId;

	if(p->Motion == 15)
		Mob[clientId].Motion = 1;
	else if(p->Motion == 13)
		Mob[clientId].Motion = 2;
	else
		Mob[clientId].Motion = 0;

	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)p, 0);
	return true;
}


