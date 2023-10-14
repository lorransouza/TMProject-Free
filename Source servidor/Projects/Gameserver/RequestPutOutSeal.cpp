#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestPutOutSeal(PacketHeader* header)
{
	MSG_PUTOUTSEAL* p = reinterpret_cast<MSG_PUTOUTSEAL*>(header);

	if (!Mob[clientId].Mobs.Player.Status.curHP || Status != USER_PLAY)
	{
		SendHpMode(clientId);

		AddCrackError(clientId, 1, CRACK_USER_STATUS);
		return true;
	}

	if (Users[clientId].Trade.ClientId != MOB_EMPTY)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	if (p->SrcType != (int)SlotType::Inv)
	{
		Log(clientId, LOG_HACK, "Tentativa de usar Selo da Alma fora do inventário (%d).", p->SrcType);
		Log(SERVER_SIDE, LOG_HACK, "%s Tentativa de usar Selo da Alma fora do inventário (%d).", User.Username, p->SrcType);

		return true;
	}

	RemoveTrade(clientId);

	st_Item* srcItem = GetItemPointer(p->Header.ClientId, p->SrcType, p->SrcSlot);
	if (srcItem == nullptr)
	{
		Log(clientId, LOG_HACK, "Item inválido. SrcType %d, SrcSlot %d", p->SrcType, p->SrcSlot);

		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		return true;
	}

	if (srcItem->Index != 3443)
	{
		Log(clientId, LOG_HACK, "Tentativa de usar Selo da Alma com outro item no slot enviado.", p->SrcType, p->SrcSlot);
		Log(SERVER_SIDE, LOG_HACK, "%s Tentativa de usar Selo da Alma com outro item no slot enviado.", User.Username, p->SrcType, p->SrcSlot);

		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		return true;
	}

	if (Mob[clientId].Mobs.Player.bStatus.Level < 279)
	{
		SendClientMessage(clientId, "Level mínimo para tirar o selo do personagem é 280");

		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		return true;
	}

	if (srcItem->Effect[0].Index == 0)
	{
		Log(clientId, LOG_HACK, "Tentativa de usar Selo da Alma sem efeito.", p->SrcType, p->SrcSlot);
		Log(SERVER_SIDE, LOG_HACK, "%s Tentativa de usar Selo da Alma sem efeito", User.Username, p->SrcType, p->SrcSlot);

		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		return true;
	}

	int newSlot = -1;
	for (int i = 0; i < 4; i++)
	{
		if (!CharList.Name[i][0])
		{
			newSlot = i;

			break;
		}
	}

	if (newSlot == -1)
	{
		SendClientMessage(clientId, "Não possui espaço na seleção de personagem");

		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		return true;
	}

	CharLogOut(clientId);

	p->MobName[15] = '\0';
	p->Header.PacketId = OutSealPacket;
	p->Header.ClientId = clientId;

	AddMessageDB(reinterpret_cast<BYTE*>(p), sizeof MSG_PUTOUTSEAL);

	SendSignalParm(clientId, clientId, 0x3B4, newSlot);

	Log(SERVER_SIDE, LOG_INGAME, "%s usou Selo da Alma", User.Username);
	Log(clientId, LOG_INGAME, "Usou o Selo da Alma no slot %d", p->SrcSlot);
	return true;
}