#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestExitParty(PacketHeader *Header)
{
	p37E *p = (p37E*)Header;

	// Se targetId != 0 quer dizer que o líder tentou usar o sistema de kikar usuário
	int targetId = p->mobId;
	
	if(targetId >= MAX_PLAYER || targetId < 0)
		return false;

	if(p->Header.ClientId != clientId)
		return false;

	if (Arena.GroupIndex != MAXUINT32)
	{
		SendClientMessage(clientId, "Não é possível sair do grupo durante a Arena");

		return true;
	}

	if (targetId != 0)
	{
		CMob *player = &Mob[clientId];

		if (!player->Leader)
			RemoveParty(targetId);
		else
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);
	}
	else
		RemoveParty(clientId);

	return true;
}