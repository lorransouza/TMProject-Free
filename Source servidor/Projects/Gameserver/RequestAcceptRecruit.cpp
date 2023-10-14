#include "cServer.h"
#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"

bool CUser::RequestAcceptRecruit(PacketHeader* header)
{
	auto p = reinterpret_cast<MSG_RECRUITREQUEST*>(header);
	p->GuildName[15] = '\0';
	p->Nickname[15] = '\0';

	if (p->ClientId <= 0 || p->ClientId >= MAX_PLAYER)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Enviou o pacote de aceitar recrutamento com o clientId inválido: %d", p->ClientId);

		return false;
	}

	CUser* targetUser = &Users[p->ClientId];
	auto targetUserIt = std::find(std::begin(targetUser->invitedUsers), std::end(targetUser->invitedUsers), p->Nickname);
	if (targetUserIt == std::end(targetUser->invitedUsers))
	{
		Log(SERVER_SIDE, LOG_INGAME, "Enviou o pacote de aceitar recrutamento sem ser convidado");

		return false;
	}

	auto targetMob = &Mob[p->ClientId];
	if (targetMob->Mobs.Player.GuildIndex != p->GuildId)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Enviou o pacote de recrutamento com o guildId inválido. Enviado: %d. Esperado: %d", p->GuildId, targetMob->Mobs.Player.GuildIndex);

		return false;
	}

	if (strncmp(targetMob->Mobs.Player.Name, p->Nickname, 16) != 0)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Enviou o pacote de recrutamento com o nickname inválido. Enviado: %s. Esperado: %s", p->Nickname, targetMob->Mobs.Player.Name);

		return true;
	}

	if (Mob[clientId].Mobs.Player.GuildIndex != 0)
	{
		SendClientMessage(clientId, "Você já possui uma guild");

		return true;
	}

	auto memberType = Mob[p->ClientId].Mobs.Player.GuildMemberType;
	if (memberType != 9 && (memberType < 3 || memberType > 9))
	{
		SendClientMessage(clientId, "O outro jogador não é líder/sub da guild");

		return true;
	}

	if (Mob[p->ClientId].Mobs.Player.Gold < 4000000)
	{
		SendClientMessage(clientId, "O outro jogador não possui o valor para recrutamento");

		return true;
	}

	int guildIndex = p->GuildId;
	Mob[clientId].Mobs.Player.GuildIndex = guildIndex;
	Mob[clientId].Mobs.Player.GuildMemberType = 1;

	Mob[p->ClientId].Mobs.Player.Gold -= 4000000;
	SendSignalParm(p->ClientId, p->ClientId, 0x3AF, Mob[p->ClientId].Mobs.Player.Gold);

	SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Recruit_GuildEnter]);
	SendClientMessage(p->ClientId, g_pLanguageString[_NN_Guild_Recruit], Mob[clientId].Mobs.Player.Name);

	p364 packetMob;
	GetCreateMob(clientId, (BYTE*)&packetMob);

	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packetMob, 0);

	Log(p->ClientId, LOG_INGAME, "Recrutou %s", Mob[clientId].Mobs.Player.Name);
	Log(clientId, LOG_INGAME, "Foi recrutado por %s. Guildname: %s. Index: %d", Mob[p->ClientId].Mobs.Player.Name, g_pGuild[guildIndex].Name.c_str(), guildIndex);

	targetUser->invitedUsers.erase(targetUserIt);
	return true;
}