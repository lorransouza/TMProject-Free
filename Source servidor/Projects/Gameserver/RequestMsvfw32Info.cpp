#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestMsvfw32Info(PacketHeader *header)
{
	_MSG_MSVFW32_INTEGRITY *packet = reinterpret_cast<_MSG_MSVFW32_INTEGRITY*>(header);
	packet->Path[MAX_PATH - 1] = 0x0;

	Log(clientId, LOG_INGAME, "MSvfw32 adulterada, caminho atual %s", packet->Path);
	return true;
}