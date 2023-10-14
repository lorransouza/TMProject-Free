#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestBlockItems(PacketHeader *Header)
{
	_MSG_BLOCKPASS *p = (_MSG_BLOCKPASS*)(Header);

	if(User.Block.Pass[0])
		return 1;

	strncpy_s(User.Block.Pass, p->Password, 16);

	User.Block.Blocked = 0;
	SendClientMessage(clientId, "Sua senha foi definida");

	Log(clientId, LOG_INGAME, "Definido senha de bloqueio de itens: %s", p->Password);
	return 1;
}