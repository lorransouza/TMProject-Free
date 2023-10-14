#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include <ctime>
#include <algorithm>
#include <sstream>

bool CUser::RequestAutoPartyEnableAll(PacketHeader* header)
{
	AutoParty.EnableAll = !AutoParty.EnableAll;

	SendAutoPartyInfo(clientId);
	return true;
}