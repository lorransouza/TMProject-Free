#include "UOD_Arena.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include <ctime>

//Quadrado da área: 195,625,143,546
constexpr int ArenaDurationMinutes = 15;
constexpr std::chrono::milliseconds ArenaDuration = std::chrono::milliseconds(ArenaDurationMinutes * 60 * 1000);

constexpr std::array<st_Position, 4> StartPosition =
{
	{
		{ 190, 621 },
		{ 190, 555 },
		{ 147, 555 },
		{ 147, 621 }
	}
};

bool TOD_Arena::CanRegister(CUser& user)
{
	std::time_t rawnow = std::time(nullptr);
	struct tm now; localtime_s(&now, &rawnow);

	if (_registeredUsers.size() >= 52)
		return false;

	return IsOnHour() && now.tm_min < 5;
}

bool TOD_Arena::Register(CUser& user, st_Item* item)
{
	for (const auto& userRegistered : _registeredUsers)
	{
		if (userRegistered == &user)
		{
			Log(user.clientId, LOG_INGAME, "Usuário já registrado na arena");
			return false;
		}

		if (memcmp(&userRegistered->MacAddress, &user.MacAddress, 8) == 0)
		{
			Log(user.clientId, LOG_INGAME, "Usuário já registrado na arena com a conta %s", userRegistered->User.Username);
			return false;
		}
	}

	_registeredUsers.push_back(&user);

	Log(user.clientId, LOG_INGAME, "Registrado na Arena Real");
	Log(SERVER_SIDE, LOG_INGAME, "[%s] - %s registrado na Arena Real. Total: %02d/%02d", user.User.Username, Mob[user.clientId].Mobs.Player.Name, _registeredUsers.size(), 52);
	SendChatMessage(0xFFFF7F27, "%s registrou-se na arena. Total: %02d/%02d", Mob[user.clientId].Mobs.Player.Name, _registeredUsers.size(), 52);
	return true;
}

void TOD_Arena::Work()
{
	// Se não estiver nem na hora, não queremos que o sistema faça qualquer coisa
	if (!IsOnHour())
		return;

	std::time_t rawNow = std::time(nullptr);
	struct tm now;
	localtime_s(&now, &rawNow);

	if (_status == TOD_Arena_Status::Disabled && now.tm_min < 5)
	{
		SendChatMessage(0xFFFF7F27, "O registro para a Arena foi aberto! Participe falando com Odin, próximo ao PvP Armia");
		_status = TOD_Arena_Status::WaitingToStart;
	}
	// Se não tiver iniciado ainda mas está no minuto de inicialização 
	else if (_status == TOD_Arena_Status::WaitingToStart && now.tm_min == 5)
	{
		if (StartArena())
			_status = TOD_Arena_Status::Enabled;
		else
			_status = TOD_Arena_Status::WaitingToDisable;

		return;
	}

	if (_status == TOD_Arena_Status::WaitingToStart && now.tm_min < 5)
	{
		auto nowChrono = std::chrono::steady_clock::now();
		if (nowChrono - _lastMessage >= 1min)
		{
			SendChatMessage(0xFFFF7F27, "Participe da Arena Real falando com o Odin, próximo ao PvP Armia!");
			_lastMessage = nowChrono;
		}
	}

	if (_status == TOD_Arena_Status::Disabled || _status == TOD_Arena_Status::WaitingToStart)
		return;

	if (_status == TOD_Arena_Status::WaitingToDisable)
	{
		_status = TOD_Arena_Status::Disabled;

		return;
	}

	auto nowChrono = std::chrono::steady_clock::now();
	if (nowChrono - _startTime >= ArenaDuration)
	{
		DecideWinner();
		Reset();

		return;
	}

	bool needUpdate = false;
	for (auto& group : _groups)
	{
		auto it = group.begin();
		while (it != group.end())
		{
			if (!IsUserValid(*it))
			{
				(*it).User->Arena.GroupIndex = -1;

				DoRecall((*it).User->clientId);
				SendSignalParm((*it).User->clientId, (*it).User->clientId, ArenaScoreboardSetStatusPacket, 0);

				it = group.erase(it);

				needUpdate = true;
			}
			else
				++it;
		}
	}

	if (needUpdate)
		RefreshScoreboard();
}

bool TOD_Arena::StartArena()
{
	// Removemos todos os usuários que não são válidos para estarem na arena
	// Assim evita distribuição incorreta do OverPower
	_registeredUsers.erase(std::remove_if(std::begin(_registeredUsers), std::end(_registeredUsers), [&](const CUser* user) {
		return user->Status != USER_PLAY || Mob[user->clientId].Mobs.Player.Equip[0].EFV2 < CELESTIAL;
	}), std::end(_registeredUsers));

	Log(SERVER_SIDE, LOG_INGAME, "Total de participantes após remoção de offlines e não celestiais/sub: %u", _registeredUsers.size());

	// Organiza a lista por OverPower
	std::sort(std::begin(_registeredUsers), std::end(_registeredUsers), [](const CUser* lhs, const CUser* rhs) {
		return GetOverPower(Mob[lhs->clientId]) > GetOverPower(Mob[rhs->clientId]);
	});

	if (_registeredUsers.size() < MinimumParticipants)
	{
		Reset();

		Log(SERVER_SIDE, LOG_INGAME, "Arena não iniciada por falta de participantes");
		return false;
	}

	std::stringstream str;
	{
		int groupIndex = 0;
		for (auto& user : _registeredUsers)
		{
			auto& group = _groups[groupIndex];
			group.push_back(InternalArena{ user, true });

			RemoveParty(user->clientId);
			Log(user->clientId, LOG_INGAME, "Usuário foi colocado no grupo %s", GroupsName[groupIndex]);

			str << user->User.Username << " - " << Mob[user->clientId].Mobs.Player.Name << " entrou no grupo " << GroupsName[groupIndex] << " com " << GetOverPower(Mob[user->clientId]) << " OverPower\n";

			groupIndex++;
			groupIndex %= 4;
		}
	}

	int groupIndex = 0;
	for (auto& group : _groups)
	{
		if (group.size() == 0)
			continue;

		auto groupLeader = group[0].User;
		Mob[groupLeader->clientId].Leader = 0;

		SendAddParty(groupLeader->clientId, groupLeader->clientId, 1);

		int partyIndex = 0;
		for (auto& memberInfo : group)
		{
			auto user = memberInfo.User;
			if (user != groupLeader)
				Mob[groupLeader->clientId].PartyList[partyIndex++] = user->clientId;

			SendAddParty(groupLeader->clientId, user->clientId, 0);
			SendAddParty(user->clientId, groupLeader->clientId, 1);

			if (user != groupLeader)
				Mob[user->clientId].Leader = groupLeader->clientId;

			for (auto& memberUser : group)
			{
				if (memberUser.User == groupLeader)
					continue;

				SendAddParty(user->clientId, memberUser.User->clientId, 0);
			}

			Mob[user->clientId].GuildDisable = 1;
			Teleportar(user->clientId, StartPosition[groupIndex].X, StartPosition[groupIndex].Y);
			SendSignalParm(user->clientId, user->clientId, 0x3A1, ArenaDurationMinutes * 60);

			user->Arena.GroupIndex = groupIndex;
			user->Arena.Kills = 0;

			p364 packet{};
			GetCreateMob(user->clientId, reinterpret_cast<BYTE*>(&packet));

			GridMulticast_2(Mob[user->clientId].Target.X, Mob[user->clientId].Target.Y, reinterpret_cast<BYTE*>(&packet), 0);
		}

		groupIndex++;
	}

	_pointsToWin = sServer.Arena.MaxKills;

	if (_registeredUsers.size() > 12)
	{
		int extraPointsToWin = (_registeredUsers.size() - 12) / 4;
		_pointsToWin += (extraPointsToWin * 10);
	}

	Log(SERVER_SIDE, LOG_INGAME, "Configurado para um total de %d kills para vencer", _pointsToWin);
	RefreshScoreboard();

	Log(SERVER_SIDE, LOG_INGAME, str.str().c_str());

	_startTime = std::chrono::steady_clock::now();
	return true;
}

bool TOD_Arena::IsOnHour() const
{
	std::time_t rawnow = std::time(nullptr);
	struct tm now; localtime_s(&now, &rawnow);

	return std::find(std::begin(sServer.Arena.HoursAllowed), std::end(sServer.Arena.HoursAllowed), now.tm_hour) != std::end(sServer.Arena.HoursAllowed);
}

void TOD_Arena::Reset()
{
	_registeredUsers.clear();
	_status = TOD_Arena_Status::Disabled;

	for (const auto& group : _groups)
	{
		for (const auto& memberInfo : group)
		{
			auto user = memberInfo.User;
			SendSignalParm(user->clientId, user->clientId, ArenaScoreboardSetStatusPacket, 0);
			user->Arena.GroupIndex = MAXUINT32;
			user->Arena.Kills = 0;

			MulticastGetCreateMob(user->clientId);
		}
	}

	for (int i = 1; i < MAX_PLAYER; ++i)
	{
		if (Users[i].Status != USER_PLAY || Users[i].AccessLevel == 0)
			continue;

		if ((Mob[i].Target.X / 128) == 1 && (Mob[i].Target.Y / 128) == 4)
			SendSignalParm(i, i, ArenaScoreboardSetStatusPacket, 0);
	}

	for (auto& group : _groups)
		group.clear();

	ClearArea(143, 546, 195, 625);
}

void TOD_Arena::DecideWinner()
{
	auto points = GetPoints();

	auto maxIt = std::max_element(std::begin(points), std::end(points));
	bool equal = false;

	for (auto it = std::begin(points); it != std::end(points); ++it)
	{
		if (it == maxIt)
			continue;

		// Dereferência porque queremos comparar o valor, não os iteradores
		if (*it == *maxIt)
			equal = true;
	}

	if (equal)
	{
		SendNotice("Nenhum grupo foi vencedor pois o número de vidas é igual");

		Log(SERVER_SIDE, LOG_INGAME, "Nenhum grupo vencedor pois o número de vidas é igual");
		return;
	}

	int groupIndex = std::distance(std::begin(points), maxIt);
	GiveReward(groupIndex);

	SendNotice("O grupo %s foi vencedor por possuir %d pontos.", GroupsName[groupIndex], points[groupIndex]);
	Log(SERVER_SIDE, LOG_INGAME, "O grupo %s foi vencedor por possuir %d pontos", GroupsName[groupIndex], points[groupIndex]);
}

void TOD_Arena::GiveReward(CUser* user, const ArenaReward& reward)
{
	if (user == nullptr)
		return;

	CMob& mob = Mob[user->clientId];
	size_t ev = mob.Mobs.Player.Equip[0].Effect[1].Value - 1;
	if (ev < reward.Experience.size())
	{
		auto expInfo = reward.Experience[ev];

		for (const auto& expEv : expInfo)
		{
			if (expEv.MaximumLevel != 0 && mob.Mobs.Player.bStatus.Level > expEv.MaximumLevel)
				continue;

			mob.Mobs.Player.Exp += expEv.Experience;
			Log(user->clientId, LOG_INGAME, "[ARENA] Recebeu %llu de experiência. Total de experiência: %lld", expEv.Experience, mob.Mobs.Player.Exp);
			break;
		}
	}
	auto gold = reward.Gold;
	if (gold != 0)
	{
		mob.Mobs.Player.Gold += gold;
		Log(user->clientId, LOG_INGAME, "[ARENA] Recebeu %d de gold. Gold total: %d", gold, mob.Mobs.Player.Gold);
	}

	for (const auto item : reward.Items)
	{
		int slotId = GetFirstSlot(user->clientId, 0);
		if (slotId == -1)
		{
			Log(user->clientId, LOG_INGAME, "[ARENA] Não recebeu o item %s %s por falta de espaço no inventário", ItemList[item.Index].Name, item.toString().c_str());

			continue;
		}

		mob.Mobs.Player.Inventory[slotId] = item;
		SendItem(user->clientId, SlotType::Inv, slotId, const_cast<st_Item*>(&item));

		SendClientMessage(user->clientId, "!Chegou um item: [%s]", ItemList[item.Index].Name);
		Log(user->clientId, LOG_INGAME, "[ARENA] Recebeu o item %s %s", ItemList[item.Index].Name, item.toString().c_str());
	}

	SendEtc(user->clientId);
}

void TOD_Arena::GiveReward(int groupIndex)
{
	for (auto& memberInfo : _groups[groupIndex])
	{
		GiveReward(memberInfo.User, sServer.Arena.WinnerRewards);

		Log(memberInfo.User->clientId, LOG_INGAME, "[ARENA] Vencedor da Arena");
	}

	InternalArena topKill{};
	bool isSamePoints = false;
	for (const auto& group : _groups)
	{
		for (auto& memberInfo : group)
		{
			InternalArena topKillWinner = memberInfo;
			if (topKill.User == nullptr || topKillWinner.User->Arena.Kills > topKill.User->Arena.Kills)
			{
				topKill = topKillWinner;
				isSamePoints = false;
			}
			else if (topKillWinner.User->Arena.Kills == topKill.User->Arena.Kills)
				isSamePoints = true;
		}
	}

	if (isSamePoints)
		Log(SERVER_SIDE, LOG_INGAME, "[ARENA] Havia dois membros com o mesma pontuação, não enviado premiação de top kill");
	else if (topKill.User != nullptr)
	{
		SendChatMessage(0xFFFF7F27, "O usuário %s foi o Top Kill com %d kills", Mob[topKill.User->clientId].Mobs.Player.Name, topKill.User->Arena.Kills);
		GiveReward(topKill.User, sServer.Arena.TopKillRewards);
	}

	int index = 0;
	for (const auto& group : _groups)
	{
		if (index != groupIndex)
		{
			for (auto& memberInfo : group)
			{
				if (memberInfo.User->Arena.Kills <= 0)
				{
					Log(memberInfo.User->clientId, LOG_INGAME, "[ARENA] Não recebeu premiação de participante por não ter nenhuma kill");

					SendClientMessage(memberInfo.User->clientId, "Não recebeu premiação de participante por não ter nenhuma kill");
					continue;
				}

				GiveReward(memberInfo.User, sServer.Arena.ParticipantRewards);
				Log(memberInfo.User->clientId, LOG_INGAME, "[ARENA] Participante da Arena");
			}
		}

		index++;
	}
}

bool TOD_Arena::MobKilled(int killed, int killer)
{
	if (_status != TOD_Arena_Status::Enabled)
		return false;

	int groupIndex = Users[killed].Arena.GroupIndex;
	if (groupIndex < 0 || groupIndex >= 4)
		return false;

	auto userGroup = std::find_if(std::begin(_groups[groupIndex]), std::end(_groups[groupIndex]), [&](const InternalArena& arenaInfo) {
		return arenaInfo.User->clientId == killed;
	});

	// Procura o usuário no grupo
	if (userGroup == std::end(_groups[groupIndex]))
		return false;

	for (int i = 0; i < 32; i++)
	{
		if (Mob[killed].Mobs.Affects[i].Index == 32)
		{
			memset(&Mob[killed].Mobs.Affects[i], 0, sizeof st_Affect);

			break;
		}
	}


	Users[killer].Arena.Kills++;

	Users[killed].Potion.CountHp = Mob[killed].Mobs.Player.Status.maxHP;
	Mob[killed].Mobs.Player.Status.curHP = Mob[killed].Mobs.Player.Status.maxHP;
	Users[killed].Potion.CountMp = Mob[killed].Mobs.Player.Status.maxMP;
	Mob[killed].Mobs.Player.Status.curMP = Mob[killed].Mobs.Player.Status.maxMP;

	SetReqHp(killed);
	SendScore(killed);

	Teleportar(killed, StartPosition[Users[killed].Arena.GroupIndex].X, StartPosition[Users[killed].Arena.GroupIndex].Y);

	p364 packet{};
	GetCreateMob(killed, (BYTE*)&packet);

	GridMulticast_2(Mob[killed].Target.X, Mob[killed].Target.Y, (BYTE*)&packet, 0);
	SendAffect(killed);

	char message[128] = { 0 };
	sprintf_s(message, ".%s da equipe %s foi morto por %s.", Mob[killed].Mobs.Player.Name, GroupsName[groupIndex], Mob[killer].Mobs.Player.Name);
	SendNoticeArea(message, 143, 546, 195, 625);

	RefreshScoreboard();
	CheckForWinner();

	return true;
}

void TOD_Arena::CheckForWinner()
{
	auto points = GetPoints();

	bool hasWinner = false;
	for (const auto& point : points)
	{
		if (point >= _pointsToWin)
			hasWinner = true;
	}

	if (!hasWinner)
		return;

	auto winnerGroupIt = std::find_if(std::begin(points), std::end(points), [=](int point) {
		return point >= _pointsToWin;
	});

	if (winnerGroupIt == std::end(points))
	{
		Log(SERVER_SIDE, LOG_INGAME, "Falha ao tentar encontrar vencedor");

		return;
	}

	int groupIndex = std::distance(std::begin(points), winnerGroupIt);
	GiveReward(groupIndex);

	SendChatMessage(0xFFFF7F27, "O grupo %s foi vencedor por possuir %d pontos.", GroupsName[groupIndex], points[groupIndex]);
	Log(SERVER_SIDE, LOG_INGAME, "O grupo %s foi vencedor por possuir %d pontos", GroupsName[groupIndex], points[groupIndex]);
	Reset();
}

void TOD_Arena::Unregister(CUser& user)
{
	auto registeredIt = std::find(std::begin(_registeredUsers), std::end(_registeredUsers), &user);
	if (registeredIt != std::end(_registeredUsers))
		_registeredUsers.erase(registeredIt);

	size_t groupIndex = user.Arena.GroupIndex;
	if (groupIndex >= _groups.size())
		return;

	auto userGroup = std::find_if(std::begin(_groups[groupIndex]), std::end(_groups[groupIndex]), [&](const InternalArena& userGroup) {
		return user.clientId == userGroup.User->clientId;
	});

	if (userGroup == std::end(_groups[groupIndex]))
		return;

	_groups[groupIndex].erase(userGroup);
	Log(user.clientId, LOG_INGAME, "Usuário foi desregistrado da Arena");
}

std::array<int, MaxGroups> TOD_Arena::GetPoints() const
{
	std::array<int, MaxGroups> points;

	int groupIndex = 0;
	for (const auto& group : _groups)
	{
		int totalPoints = 0;
		for (const auto& memberInfo : group)
			totalPoints += memberInfo.User->Arena.Kills;

		points[groupIndex++] = totalPoints;
	}

	return points;
}

void TOD_Arena::RefreshScoreboard() const
{
	auto points = GetPoints();

	for (const auto& group : _groups)
	{
		for (const auto& memberInfo : group)
		{
			SendArenaScoreboard(memberInfo.User->clientId, points);
			SendSignalParm(memberInfo.User->clientId, memberInfo.User->clientId, ArenaScoreboardSetStatusPacket, 1);
		}
	}

	for (int i = 1; i < MAX_PLAYER; ++i)
	{
		if (Users[i].Status != USER_PLAY || Users[i].AccessLevel == 0)
			continue;

		if ((Mob[i].Target.X / 128) == 1 && (Mob[i].Target.Y / 128) == 4)
		{
			SendArenaScoreboard(i, points);
			SendSignalParm(i, i, ArenaScoreboardSetStatusPacket, 1);
		}
	}
}

bool TOD_Arena::IsUserValid(const InternalArena& memberInfo) const
{
	if (memberInfo.User->Status != USER_PLAY)
		return false;

	auto clientId = memberInfo.User->clientId;
	if (memberInfo.IsParticipant)
		return true;

	if ((Mob[clientId].Target.X / 128) != 1 || (Mob[clientId].Target.Y / 128) != 4)
		return false;

	return true;
}