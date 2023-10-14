#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestRepurchase(PacketHeader* header)
{
	p3E8* p = reinterpret_cast<p3E8*>(header);

	SendRepurchase(clientId);
	return true;
}