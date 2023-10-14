#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestCloseAutoTrade(PacketHeader *Header)
{
	if(IsAutoTrading)
	{
		IsAutoTrading = false;

		p364 packet;
		GetCreateMob(clientId, (BYTE*)&packet);

		AddMessage((BYTE*)&packet, sizeof packet);
		SendGridMob(clientId);
		SendScore(clientId);
		SendEquip(clientId);

		Log(clientId, LOG_INGAME, "O AutoTrade foi fechado");

		EventAutoTrade.IsValid = false;
	}

	if (Trade.ClientId != 0)
		RemoveTrade(Trade.ClientId);

	RemoveTrade(clientId);
	return true;
}	