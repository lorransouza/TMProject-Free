#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"

bool CUser::RequestDonateShop(PacketHeader *Header)
{
	SendOverStoreInfo(clientId);

	return true;
}