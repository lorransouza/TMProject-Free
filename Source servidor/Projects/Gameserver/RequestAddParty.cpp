#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAddParty(PacketHeader *Header)
{
	p37F *p = (p37F*)Header;
	
	// Checagens de segurança
	if(p->targetId <= 0 || p->targetId >= MAX_PLAYER)
		return false;

	if(p->leaderId <= 0 || p->leaderId >= MAX_PLAYER)
		return false;

	if(Users[p->leaderId].Status != USER_PLAY || Users[p->targetId].Status != USER_PLAY)
		return true;

	CMob *liderSpw = &Mob[p->leaderId];
	CMob *targetSpw = &Mob[p->targetId];

	if(liderSpw->Leader != 0)
	{
		SendClientMessage(clientId, "Saia do grupo para criar um.");

		return true;
	}
	else if(targetSpw->Leader != 0)
	{
		SendClientMessage(clientId, "O outro jogador já possui grupo.");

		return true;
	}

	if(targetSpw->Leader == 0)
	{
		bool has = false;
		for(int i = 0 ; i < 12; i++)
		{
			if(Mob[p->targetId].PartyList[i] != 0)
			{
				has = true;
				break;
			}
		}

		if(has)
		{
			SendClientMessage(clientId, "O outro jogador já é líder de um grupo.");

			return true;
		}
	}

	p->partyId = 1;
	p->unk = 204;
	p->Header.ClientId = 0x7530;

	Users[p->targetId].AddMessage((BYTE*)p, sizeof p37F);
	return true;
}