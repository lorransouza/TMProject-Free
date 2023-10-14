#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestChat(PacketHeader *Header)
{
	p333 *p = (p333*)(Header);

	//TODO: Check if eChat is 96 or 128
	p->eChat[127] = '\0';

	p->Header.ClientId = clientId;

	if(!strcmp(p->eChat, "guild"))
	{
		SendGuildList(clientId);

		return true;
	}
	else if(!strcmp(p->eChat, "guildoff"))
	{
		Mob[clientId].GuildDisable = 1;
		
		SendScore(clientId);
		return true;
	}
	else if(!strcmp(p->eChat, "guildon"))
	{
		if (Mob[clientId].Target.X >= 143 && Mob[clientId].Target.Y >= 546 && Mob[clientId].Target.X <= 195 && Mob[clientId].Target.Y <= 625)
			return true;

		Mob[clientId].GuildDisable = 0;
		
		SendScore(clientId);
		return true;
	}
	else if(!strncmp(p->eChat, "guildtax", 8))
	{
		INT32 taxes = 0;
		INT32 ret = sscanf_s(p->eChat, "guildtax %d", &taxes);
		 
		if(ret != 1 || taxes < 0 || taxes > 15)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Tax_0_to_30]);

			return true;
		}

		INT32 cityId = GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y);
		if(cityId < 0 || cityId >= 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_OnlyVillage]);

			return true;
		}

		INT32 guildIndex = Mob[clientId].Mobs.Player.GuildIndex;
		if((guildIndex != ChargedGuildList[sServer.Channel - 1][cityId] || Mob[clientId].Mobs.Player.GuildMemberType != 9) && !Users[clientId].IsAdmin)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_With_Guild_Master]);

			return true;
		}

		if(sServer.TaxesDay[cityId] != 0 && !Users[clientId].IsAdmin)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Once_Per_Day]);

			return true;
		}

		// Seta como se já tivesse trocado as taxas hoje
		if(!Users[clientId].IsAdmin)
			sServer.TaxesDay[cityId] = 1;

		// Seta a nova porcentagem no servidor
		g_pCityZone[cityId].perc_impost = taxes;

		SendClientMessage(clientId, g_pLanguageString[_DD_Set_Guild_Tax], taxes);
		return true;
	}
	else if(!strcmp(p->eChat, "whisper"))
	{
		AllStatus.Whisper = !AllStatus.Whisper;
		SendClientMessage(clientId, "Whisper : %s", (AllStatus.Whisper) ? "Off" : "On");

		return true;	
	}
	else if(!strcmp(p->eChat, "partychat"))
	{
		AllStatus.Citizen = !AllStatus.Citizen;
		
		SendClientMessage(clientId, "Citizen Chatting : %s", (AllStatus.Citizen) ? "Off" : "On");
		return true;
	}
	else if(!strcmp(p->eChat, "guildchat"))
	{
		AllStatus.Guild = !AllStatus.Guild;
		
		SendClientMessage(clientId, "Guild Chatting : %s", (AllStatus.Guild) ? "Off" : "On");
		return true;
	}
	else if(!strcmp(p->eChat, "kingdomchat"))
	{
		AllStatus.Kingdom = !AllStatus.Kingdom;
		
		SendClientMessage(clientId, "Kingdom Chatting : %s", (AllStatus.Kingdom) ? "Off" : "On");
		return true;
	}

	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)p, clientId);
	return true;
}