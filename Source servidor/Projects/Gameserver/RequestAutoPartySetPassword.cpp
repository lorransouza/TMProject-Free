#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include <ctime>
#include <algorithm>
#include <sstream>

bool CUser::RequestAutoPartySetPassword(PacketHeader* header)
{
	MSG_AUTOPARTY_SETPASSWORD *packet = reinterpret_cast<MSG_AUTOPARTY_SETPASSWORD*>(header);
	packet->Password[15] = '\0';

	AutoParty.Password = packet->Password;

	SendClientMessage(clientId, "Senha para autogrupo alterada");
	return true;
}