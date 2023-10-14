#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAcceptParty(PacketHeader *Header)
{
	p3AB *p = (p3AB*)Header;

	if(p->liderID <= 0 || p->liderID >= MAX_PLAYER)
		return false;

	CMob *liderSpw = &Mob[p->liderID];
	CMob *targetSpw = &Mob[clientId];

	if(liderSpw->Leader != 0)
	{
		SendClientMessage(clientId, "O usuário não é líder do grupo.");

		return true;
	}
	else if(targetSpw->Leader != 0)
	{
		SendClientMessage(clientId, "Saia do grupo para criar um.");

		return true;
	}

	INT8 i;
	for(i = 0; i < 12; i++)
	{
		if(liderSpw->PartyList[i] == 0)
			break;
	}

	if(i >= 12)
	{
		SendClientMessage(clientId, "Grupo está cheio.");

		return true;
	}
	
	if(Mob[clientId].Target.X >= 2608 && Mob[clientId].Target.X <= 2647  && Mob[clientId].Target.Y >= 1708 && Mob[clientId].Target.Y <= 1748)
		return true;

	int t;
	for(t = 0; t < 12; t++)
	{
		if(liderSpw->PartyList[t] != 0)
			break;
	}

	if(t == 12)
	{
		SendAddParty(p->liderID, p->liderID, 1);
		SendAddParty(clientId, clientId, 0);
	}

	targetSpw->Leader = p->liderID;
	liderSpw->PartyList[i] = clientId;

	SendAddParty(clientId, p->liderID, 1);
	SendAddParty(p->liderID, clientId, 0);

	for(INT8 x = 0; x < 12; x++)
	{
		int id = liderSpw->PartyList[x];
		if(id <= 0 || id >= MAX_SPAWN_MOB)
			continue;

		SendAddParty(id, clientId, 0);
		SendAddParty(clientId, id, 0);
	}

	return true;
}