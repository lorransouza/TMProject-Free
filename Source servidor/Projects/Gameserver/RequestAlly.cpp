#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestAlly(PacketHeader *Header)
{
	pE12 *p = (pE12*)(Header);

	INT32 LOCAL_4 = p->GuildIndex1;
	INT32 LOCAL_5 = p->GuildIndex2;
	INT32 LOCAL_6 = MAX_GUILD;

	if(LOCAL_4 <= 0 || LOCAL_5 < 0 || LOCAL_4 >= LOCAL_6 || LOCAL_5 >= LOCAL_6)
		return true;
	
	if(Mob[clientId].Mobs.Player.GuildMemberType != 9)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Declare_Ally]);

		return true;
	}

/*	if(Mob[clientId].Mobs.Player.GuildIndex != LOCAL_4 || Mob[clientId].Mobs.MedalId != 509)
	{
		SendClientMessage(p->Header.ClientId, g_pLanguageString[_NN_Only_With_Guild_Master]);

		return true;
	}
	*/
	INT32 LOCAL_7 = sServer.Channel;
	INT32 LOCAL_8 = LOCAL_4 >> 12;
	INT32 LOCAL_9 = LOCAL_4 & 0x0FFF;
	INT32 LOCAL_10 = 1;

	if(LOCAL_7 < 0 || LOCAL_7 >= 10 || LOCAL_8 < 0 || LOCAL_8 >= 16 || LOCAL_9 < 0 || LOCAL_9 >= 4096)
		LOCAL_10 = 0;

	if(LOCAL_10 == 0)
	{
		SendClientMessage(p->Header.ClientId, "Guild inválida.");

		return true;
	}
	
	INT32 LOCAL_11 = g_pGuildAlly[LOCAL_4];
	if ((LOCAL_11 > 0 && LOCAL_11 < LOCAL_6 && g_pGuildAlly[LOCAL_11] == LOCAL_4) || g_pGuild[LOCAL_4].Kingdom != g_pGuild[LOCAL_5].Kingdom)
	{
		SendClientMessage(p->Header.ClientId, g_pLanguageString[_NN_Cant_Declare_Ally]);

		return true;
	}
	else
	{
		p->Header.ClientId = clientId;
		
		AddMessageDB((BYTE*)p, sizeof pE12);
	}

	return true;
}