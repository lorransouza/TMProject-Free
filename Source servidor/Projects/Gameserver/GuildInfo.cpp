#include "cServer.h"
#include "Basedef.h"

bool GetGuildInfo(PacketHeader *Header)
{
	MSG_ADDGUILD *p = (MSG_ADDGUILD*)(Header);

	if (p->Type == 0)
	{ // Adicionar fame
		g_pGuild[p->guildIndex].Fame = p->Value;
	}
	else if (p->Type == 1)
	{ // Mudar cidadania
		g_pGuild[p->guildIndex].Citizen = p->Value;
	}
	else if (p->Type == 2)
	{ // Mudar wins
		g_pGuild[p->guildIndex].Wins = p->Value;
	}

	return true;
}

bool GetNewGuild(PacketHeader *Header)
{
	MSG_CREATEGUILD *p = (MSG_CREATEGUILD*)(Header);

	INT32 guildId = p->guildId;
	if (guildId <= 0 || guildId >= MAX_GUILD)
		return false;

	memset(&g_pGuild[p->guildId], 0, sizeof stGuild);
	g_pGuild[p->guildId].Name = std::string(p->GuildName);

	g_pGuild[p->guildId].Fame = 0;
	g_pGuild[p->guildId].Citizen = p->citizen;
	g_pGuild[p->guildId].Kingdom = p->kingDom;

	return true;
}

bool AddSubLider(PacketHeader *Header)
{
	MSG_ADDSUB *p = (MSG_ADDSUB*)(Header);

	if (p->SubIndex < 0 || p->SubIndex >= 3)
	{
		Log(SERVER_SIDE, LOG_ERROR, "SubIndex = %d #01", p->SubIndex);

		return true;
	}

	if (p->Status == 1)
		g_pGuild[p->GuildIndex].SubGuild[p->SubIndex][0] = 0;
	else
		g_pGuild[p->GuildIndex].SubGuild[p->SubIndex] = p->Name;

	return true;
}