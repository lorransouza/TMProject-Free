#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "UOD_BossEvent.h"

constexpr std::array<st_Position, 4> npcPositions =
{
	{
		{ 2073, 2092 },
		{ 2082, 2127 },
		{ 2148, 2123 },
		{ 2152, 2068 }
	}
};

constexpr std::array<st_Position, 2> BossPosition =
{
	{
		{ 1090, 1444 },
		{ 1090, 1500 }
	}
};

constexpr std::array<st_Position, 4> playerPosition =
{
	{
		{ 1130, 1512 },
		{ 1130, 1499 },
		{ 1131, 1451 },
		{ 1131, 1437 }
	}
};

constexpr std::array eventStartMin =
{
	0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55
};

// 2033,2063,1935,2160

bool TOD_BossEvent::CanRegister(CUser& user)
{
	return _status;
}

bool TOD_BossEvent::Register(CUser& user, st_Item* item)
{
	for (const auto& userInArea : _users)
	{
		if (memcmp(user.MacAddress, userInArea->MacAddress, 8) == 0)
		{
			SendClientMessage(user.clientId, "Somente uma conta por computador");

			Log(user.clientId, LOG_INGAME, "Conta \"%s\" na área do evento!", userInArea->User.Username);
			return false;
		}
	}

	_users.push_back(&user);
	user.Christmas.HitsOnBoss = 0;

	const st_Position& position = *select_randomly(std::begin(playerPosition), std::end(playerPosition));
	Teleportar(user.clientId, position.X, position.Y);

	Log(user.clientId, LOG_INGAME, "Entrou na área do evento. Teleportado para %hu %hu", position.X, position.Y);
	return true;
}

void TOD_BossEvent::Reset()
{
	for (const auto& user : _users)
	{
		if (user->Christmas.HitsOnBoss <= 0)
		{
			Log(user->clientId, LOG_INGAME, "Não recebeu o item do evento pois não atacou o boss. Total de ataques: %d", user->Christmas.HitsOnBoss);

			continue;
		}

		int totalItems = Rand() % 5 + 1;
		if (_doubleMode)
			totalItems = Rand() % 9 + 1;
	
		if (_medalId != 0 && !_doubleMode)
		{
			int medal = GetFirstSlot(user->clientId, _medalId);
			if (medal != -1)
			{
				totalItems = Rand() % 9 + 2;

				Log(user->clientId, LOG_INGAME, "Consumiu uma unidade de Medalha de Caça. Quantidades que possuía: %d", GetItemAmount(&Mob[user->clientId].Mobs.Player.Inventory[medal]));
				AmountMinus(&Mob[user->clientId].Mobs.Player.Inventory[medal]);
				SendItem(user->clientId, SlotType::Inv, medal, &Mob[user->clientId].Mobs.Player.Inventory[medal]);
			}
		}

		Log(user->clientId, LOG_INGAME, "Gerado %d para n° de %s", totalItems, ItemList[_item.Index].Name);

		int totalReceived = 0;
		for (int i = 0; i < totalItems; i++)
		{
			int slotId = GetFirstSlot(user->clientId, 0);
			if (slotId == -1)
			{
				SendClientMessage(user->clientId, "Sem espaço no inventário");

				break;
			}

			Mob[user->clientId].Mobs.Player.Inventory[slotId] = this->_item;
			SendItem(user->clientId, SlotType::Inv, slotId, &this->_item);

			totalReceived++;
		}

		Log(user->clientId, LOG_INGAME, "Total de itens de evento recebido: %d", totalReceived);
	}

    if (_npcId > 0 && _npcId < MAX_SPAWN_MOB)
    {
        if (_npcName == Mob[_npcId].Mobs.Player.Name && Mob[_npcId].Mobs.Player.Status.curHP > 0)
            DeleteMob(_npcId, 1);
    }

	for (int i = 0; i < TotalBoss; ++i)
	{
		int bossId = _bossId[i];
		if (bossId > 0 && bossId < MAX_SPAWN_MOB)
		{
			if (_bossName == Mob[bossId].Mobs.Player.Name && Mob[bossId].Mobs.Player.Status.curHP > 0)
				DeleteMob(bossId, 1);
		}
	}

	_lastUsersCount = _users.size();

	// Limpamos a área por completo...
	ClearArea(1030, 1409, 1150, 1535, "Evento de Boss");

	_users.clear();

	_bossId.fill(-1);
	_teleportId = -1;
	_status = false;
	_teleportInitIndex = -1;
    _npcId = -1;
}

void TOD_BossEvent::Work()
{
	if (!_eventStatus)
		return;

	_users.erase(std::remove_if(std::begin(_users), std::end(_users), [&](const CUser* user) {
		if (user->Status != USER_PLAY)
			return true;

		CMob& mob = Mob[user->clientId];
		return !(mob.Target.X >= 1030 && mob.Target.X <= 1150 && mob.Target.Y >= 1409 && mob.Target.Y <= 1535);
	}), std::end(_users));

	time_t rawtime{ time(0) };
	struct tm timeinfo;
	localtime_s(&timeinfo, &rawtime);

	bool isTime = std::find(std::begin(eventStartMin), std::end(eventStartMin), timeinfo.tm_min) != std::end(eventStartMin);
	if (!isTime || _status)
		return;

	_status = true;
	_teleportId = Rand() % 4;

	const st_Position npcPosition = npcPositions[_teleportId];
	st_Item teleportItem{ 794 };

    int npcId = CreateMob(_npcName.c_str(), npcPosition.X, npcPosition.Y, "npc");
    if (npcId <= 0)
    {
        Log(SERVER_SIDE, LOG_INGAME, "Não foi posssível criar o NPC de Teleporte pois não existiu slot para NPCs");

        Reset();
        return;
    }

	for (int i = 0; i < TotalBoss; ++i)
	{
		const st_Position bossPosition = BossPosition[i];
		int bossId = CreateMob(_bossName.c_str(), bossPosition.X, bossPosition.Y, "npc");
		if (bossId <= 0)
		{
			Log(SERVER_SIDE, LOG_INGAME, "Não foi posssível criar o Boss pois não existiu slot para NPCs");

			Reset();
			return;
		}

		if (_lastUsersCount <= 5)
		{
			Mob[bossId].Mobs.Player.Status.curHP = 500;
			Mob[bossId].Mobs.Player.Status.maxHP = Mob[bossId].Mobs.Player.Status.curHP;
			Mob[bossId].Mobs.Player.bStatus.curHP = Mob[bossId].Mobs.Player.Status.curHP;
			Mob[bossId].Mobs.Player.bStatus.maxHP = Mob[bossId].Mobs.Player.Status.curHP;
		}
		else
		{
			Mob[bossId].Mobs.Player.Status.curHP = 80 * _lastUsersCount + 150;
			Mob[bossId].Mobs.Player.Status.maxHP = Mob[bossId].Mobs.Player.Status.curHP;
			Mob[bossId].Mobs.Player.bStatus.curHP = Mob[bossId].Mobs.Player.Status.curHP;
			Mob[bossId].Mobs.Player.bStatus.maxHP = Mob[bossId].Mobs.Player.Status.curHP;
		}

		Log(SERVER_SIDE, LOG_INGAME, "Boss posição: %hu %hu. HP: %d. LastUserCount: %d", bossPosition.X, bossPosition.Y, Mob[bossId].Mobs.Player.Status.maxHP, _lastUsersCount);
		_bossId[i] = bossId;
	}

    _npcId = npcId;
	SendNotice("Liberado o teleporte para a área do Evento");
}