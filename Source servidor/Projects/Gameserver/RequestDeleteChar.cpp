#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"

bool CUser::RequestDeleteChar(PacketHeader* header)
{
	p211 *p = (p211*)(header);
	p->Name[15] = '\0';
	p->Pwd[11] = '\0';

	st_Mob* mob = &Mob[clientId].Mobs.Player;
	if (Status != USER_SELCHAR)
	{
		SendClientMessage(clientId, "Deleting Character, wait a moment");

		Log(clientId, LOG_HACK, "Tentativa de deletar personagem enquanto não na CharList. Status atual: %d", Status);
		return true;
	}

	for (int i = 1; i < 15; i++)
	{
		if (mob->Equip[i].Index <= 0 || mob->Equip[i].Index >= MAX_ITEMLIST)
			continue;

		SendClientMessage(clientId, "Desequipe todos os itens do personagem para excluir");
		return true;
	}

	if (Mob[clientId].Mobs.Player.Gold != 0)
	{
		SendClientMessage(clientId, "Não é possível excluir enquanto possuir gold no inventário");
		return true;
	}

	Log(clientId, LOG_INGAME, "Solicitado deletar personagem %s", p->Name);

	Status = USER_DELWAIT;

	header->PacketId = 0x809;
	return AddMessageDB((BYTE*)header, sizeof p211);
}