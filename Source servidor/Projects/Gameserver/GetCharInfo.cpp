#include "cServer.h"
#include "Basedef.h"
 
bool CUser::GetCharInfo(PacketHeader *Header)
{
	p802 *p = (p802*)(Header);

	stCharInfo *charInfo = &Mob[clientId].Mobs;
	Mob[clientId].Mobs = p->Mob;

	User.Block.Blocked = p->Blocked;
	strncpy_s(User.Block.Pass, p->Pass, 16);

	Log(clientId, LOG_INGAME, "Recebido informação do personagem completa.");
	return true;
}