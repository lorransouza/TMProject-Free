#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestClientInfo(PacketHeader *Header)
{
	INT32 requested = aHack.Question,
		  status    = aHack.Response;

	if(status != 1)
	{
		CloseUser(clientId);

		return true;
	}

	switch(requested)
	{
		case 0: // request macaddress
		{
			p655 *p = (p655*)(Header);

			if(p->Requested != 0)
			{
				CloseUser(clientId);

				return true;
			}
/*
			if(memcmp(p->Mac, this->MacAddress, 6) != 0)
			{
				Log(clientId, LOG_INGAME, "ANTIHACK - Identificado aparente alteração de macaddress! Antigo: %02X:%02X:%02X:%02X:%02X:%02X. Novo: %02X:%02X:%02X:%02X:%02X:%02X",
					this->MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5], p->Mac[0], p->Mac[1], p->Mac[2], p->Mac[3], p->Mac[4], p->Mac[5]);

				Log(SERVER_SIDE, LOG_INGAME, "ANTOHACK - [%s] : Identificado aparente alteração de macaddress! Antigo: %02X:%02X:%02X:%02X:%02X:%02X. Novo: %02X:%02X:%02X:%02X:%02X:%02X",
					User.Username, this->MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5], p->Mac[0], p->Mac[1], p->Mac[2], p->Mac[3], p->Mac[4], p->Mac[5]);

				memcpy(this->MacAddress, p->Mac, 6);

				aHack.Error ++;
			}*/
		}
		break;
		case 1:
		{
			p656 *p = (p656*)(Header);

			if(p->Requested != 1)
			{
				CloseUser(clientId);

				return true;
			}

			if(strncmp(p->nick, Mob[clientId].Mobs.Player.Name, 16) != 0)
			{
				Log(clientId, LOG_INGAME, "ANTIHACK - Respondido pacote incorretamente. Nickname respondido: %s. Nickname correto: %s", 
					p->nick, Mob[clientId].Mobs.Player.Name);

				Log(clientId, LOG_INGAME, "ANTIHACK - [%S] : Respondido pacote incorretamente. Nickname respondido: %s. Nickname correto: %s", 
					User.Username, p->nick, Mob[clientId].Mobs.Player.Name);

				aHack.Error ++;
			}
		}
		break;
		case 2:
			{
				
			}
			break;
	}
	
	aHack.Response = 0;
	aHack.Question = -1;
	aHack.Last     = sServer.SecCounter;
	aHack.Next     = 30;
	return true;
}