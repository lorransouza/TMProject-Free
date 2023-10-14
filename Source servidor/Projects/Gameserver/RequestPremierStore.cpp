#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestPremierStore(PacketHeader *Header)
{
	auto p = reinterpret_cast<pMsgSignal*>(Header);

	_MSG_NEWSTORE msg;
	memset(&msg, 0, sizeof _MSG_NEWSTORE);

	msg.Header.PacketId = 0x16B;
	msg.Header.Size     = sizeof _MSG_NEWSTORE;
	
	if (p->Value == 1)
	{
		memcpy(msg.Item, sServer.PremierStore.item, sizeof st_Item * MAX_PREMIERSTORE);
		memcpy(msg.Price, sServer.PremierStore.Price, sizeof(INT32) * MAX_PREMIERSTORE);

		msg.Type = TOD_Store_Type::PremierStore;
	}
	else if (p->Value == 2)
	{
		memcpy(msg.Item, sServer.ArenaStore.item, sizeof st_Item * MAX_PREMIERSTORE);
		memcpy(msg.Price, sServer.ArenaStore.Price, sizeof(INT32) * MAX_PREMIERSTORE);

		msg.Type = TOD_Store_Type::ArenaStore;
	}

	AddMessage((BYTE*)&msg, sizeof _MSG_NEWSTORE);
	return true;
}