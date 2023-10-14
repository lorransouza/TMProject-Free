#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"

bool CUser::RequestGriffinMaster(PacketHeader *Header)
{
	static int lastClientId = -1;
	const char* griffinMasterName = "Mestre Grifo";
	pAD9 *p = (pAD9*)Header;

	if(p->Type != 1 && p->Type != 2)
	{
		AddCrackError(clientId, 10, CRACK_USER_PKTHACK);

		return true;
	}

	bool needRefresh = false;
	if (lastClientId == -1 || strcmp(Mob[lastClientId].Mobs.Player.Name, "Mestre Grifo") != 0)
		needRefresh = true;

	if (needRefresh)
	{
		for (int i = 1000; i < MAX_SPAWN_MOB; i++)
		{
			if (Mob[i].Mode == 0)
				continue;

			if (strcmp(Mob[i].Mobs.Player.Name, griffinMasterName) == 0)
			{
				lastClientId = i;
				break;
			}
		}
	}

	if (lastClientId <= 0 || lastClientId >= MAX_SPAWN_MOB)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Tentativa de enviar pacote de Mestre Grifo sem o mesmo estar vivo");

		return false;
	}

	if (GetDistance(Mob[lastClientId].Target.X, Mob[lastClientId].Target.Y, Mob[clientId].Target.X, Mob[clientId].Target.Y) > 15)
	{
		Log(clientId, LOG_ERROR, "Tentativa de usar o Mestre Grifo de longe, posição do indivíduo: %dx %dy", Mob[clientId].Target.X, Mob[clientId].Target.Y);
		Log(SERVER_SIDE, LOG_ERROR, "Tentativa de usar o Mestre Grifo de longe, posição do meliante: %s %s %dx %dy", Mob[clientId].Mobs.Player.Name, User.Username, Mob[clientId].Target.X, Mob[clientId].Target.Y);

		return false;
	}

	if(p->Type == 2)
	{
		if(p->Warp == 0)
		{
			SendClientMessage(clientId, "!Chegou no destino [Campo de Treino].");

			Teleportar(clientId,2112,2051);
		}
		else if(p->Warp == 1)
		{
			SendClientMessage(clientId, "!Chegou no destino [Defensor da alma].");

			Teleportar(clientId, 2372, 2099);
		}
		else if(p->Warp == 2)
		{
			SendClientMessage(clientId, "!Chegou no destino [Jardim de Deus].");

			Teleportar(clientId, 2220, 1714);
		}
		else if(p->Warp == 3)
		{
			SendClientMessage(clientId, "!Chegou no destino [Dungeon].");

			Teleportar(clientId, 2365, 2249);
		}
		else if(p->Warp == 4)
		{
			SendClientMessage(clientId, "!Chegou no destino [Submundo].");

			Teleportar(clientId, 1826, 1771);
		}
		else
			AddCrackError(clientId, 10, CRACK_USER_PKTHACK);
	}

	return true;
}