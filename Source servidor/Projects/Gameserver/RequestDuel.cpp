#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestDuel(PacketHeader *Header)
{// 00422926
	p39F *p = (p39F*)Header;

	INT32 mobId = p->mobId,
		  typeId = p->Type;

	if(typeId < 0 || typeId > 4)
		return false;

	if (mobId <= 0 || mobId >= MAX_PLAYER)
		return false;

	if (Users[mobId].Status != USER_PLAY)
	{
		Log(clientId, LOG_INGAME, "Enviado Duelo para jogador que não está online");
		return true;
	}

	if (Users[mobId].AllStatus.Whisper)
	{
		SendClientMessage(clientId, "O outro jogador está com whisper ativo");

		return true;
	}

	if(typeId == 3)
	{
//		if(Mob[LOCAL_22].Mobs.MedalId != 509 || Mob[clientId].Mobs.MedalId != 509)
		{
			SendClientMessage(clientId, "Disponível apenas para líderes de guild");

			return true;
		}
	}

	if(typeId == 4)
	{
		if (Users[mobId].Status != USER_PLAY)
			return true;

		if (clientId != Users[mobId].Challenger.Index)
			return true;

		if(sServer.Challanger.RankingProcess != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Battle_In_Progress]);
			SendClientMessage(mobId, g_pLanguageString[_NN_Battle_In_Progress]);

			return true;
		}

		INT32 challId = Users[mobId].Challenger.Type;
		if (challId < 0 || challId > 3)
			return true;

		SendEtc(clientId);
		SendEtc(mobId);

		// 00422A11
		DoRanking(challId, clientId, mobId);
		return true;
	}

	auto now = std::chrono::steady_clock::now();
	if (now - LastDuel <= 2s)
	{
		SendClientMessage(clientId, "Aguarde para enviar novamente");

		LastDuel = now;
		return true;
	}

	//SendClientMessage(clientId, g_pLanguageString[_DN_D_Costs]);
	//SendClientMessage(LOCAL_22, g_pLanguageString[_DN_D_Costs]);

	if(typeId == 1 || typeId == 2)
	{
		if (Mob[clientId].Mobs.Player.Status.curHP <= 0 || Mob[mobId].Mobs.Player.Status.curHP <= 0)
			return true;
	}

	Users[clientId].Challenger.Index = mobId;
	Users[clientId].Challenger.Type = typeId;

	p->mobId = clientId;
	p->Header.ClientId = mobId;

	Users[mobId].AddMessage((BYTE*)p, sizeof p39F);

	LastDuel = std::chrono::steady_clock::now();

	Log(clientId, LOG_INGAME, "Enviado duelo para o jogador %s", Mob[mobId].Mobs.Player.Name);
	Log(mobId, LOG_INGAME, "Recebido duelo do jogador %s", Mob[clientId].Mobs.Player.Name);
	return true;
}