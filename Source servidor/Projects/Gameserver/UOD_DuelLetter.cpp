#include "Basedef.h"
#include "SendFunc.h"
#include "UOD_DuelLetter.h"
#include <map>
#include <vector>
#include <utility>

constexpr int DefaultTime = 220;

std::map<TOD_DuelLetter_Room, std::pair<int, int>> roomPositions =
{
	{ TOD_DuelLetter_Room::FirstRoom, { 786, 3680 } },
	{ TOD_DuelLetter_Room::SecondRoom, { 844, 3688 } },
	{ TOD_DuelLetter_Room::ThirdRoom, { 844, 3632 } },
	{ TOD_DuelLetter_Room::FourthRoom, { 786, 3639 } },
	{ TOD_DuelLetter_Room::BossRoom, { 0, 0 } },
};

std::map<TOD_DuelLetter_Room, std::vector<std::pair<int, int>>> runePositions =
{
	{ 
		TOD_DuelLetter_Room::FirstRoom, {
		{ 787, 3676 }
	}},
	{
		TOD_DuelLetter_Room::SecondRoom, {
		{ 845, 3676 },
		{ 844, 3676 }
	}},
	{ TOD_DuelLetter_Room::ThirdRoom, {
		{ 845, 3620 },
		{ 844, 3621 },
		{ 845, 3621 }
	}},
	{ TOD_DuelLetter_Room::FourthRoom, {
		{ 787, 3634},
		{ 786, 3634 },
		{ 787, 3635 },
		{ 786, 3635 },
	}},
	{ TOD_DuelLetter_Room::BossRoom, {
		{ 787, 3634 },
		{ 786, 3639 },
		{ 887, 3635 }
	}}
};

void TOD_DuelLetter::Work()
{
	if (!_status)
		return;

	_timer--;
	if (_timer <= 0)
	{
		Finish(true);

		return;
	}

	CheckUsers();

	if (_users.size() == 0)
	{
		Finish(false);

		return;
	}

	int totalMob = GetMobCountOnRoom();
	int index = GetInitialIndex();
	if (totalMob != 0)
	{
		for (const auto& user : _users)
			SendSignalParm(user->clientId, 0x7530, 0x3B0, totalMob);

		return;
	}

	if (_room == TOD_DuelLetter_Room::BossRoom)
		return;

	// Muda a sala do menino
	auto temporaryRoom = GetNextRoom();
	if (temporaryRoom == TOD_DuelLetter_Room::FourthRoom)
	{
		if (!_haveCourageSymbol)
			return;
	}

	_room = temporaryRoom;
	if (_room != TOD_DuelLetter_Room::BossRoom)
		_timer = DefaultTime;

	GenerateRoom();

	if (_room != TOD_DuelLetter_Room::BossRoom)
	{
		auto position = roomPositions[_room];
		totalMob = GetMobCountOnRoom();

		for (const auto& user : _users)
		{
			SendSignalParm(user->clientId, 0x7530, 0x3B0, totalMob);
			SendSignalParm(user->clientId, 0x7530, 0x3A1, _timer);

			Teleportar(user->clientId, position.first, position.second);
			LogGroup("Teleportado para a sala %dy %dx", position.first, position.second);
		}
	}
}

void TOD_DuelLetter::LogGroup(const char* message, ...)
{
	/* Arglist */
	char buffer[150];
	va_list arglist;
	va_start(arglist, message);
	vsprintf_s(buffer, message, arglist);
	va_end(arglist);
	/* Fim arlist */

	for (const auto& user : _users)
		Log(user->clientId, LOG_INGAME, buffer);
}

void TOD_DuelLetter::CheckUsers()
{
	_users.erase(std::remove_if(_users.begin(), _users.end(), [](const CUser* user) 
	{
		if (user->Status != USER_PLAY)
			return true;

		CMob& mob = Mob[user->clientId];
		return mob.Target.X < 782 || mob.Target.X > 886 || mob.Target.Y < 3598 || mob.Target.Y > 3695;
	}), _users.end());

	for (int i = 1; i < MAX_PLAYER; i++)
	{
		if (Mob[i].Mode == 0 || Users[i].IsAdmin)
			continue;

		if (Mob[i].Target.X >= 782 && Mob[i].Target.X <= 886 && Mob[i].Target.Y >= 3598 && Mob[i].Target.Y <= 3695)
		{
			if (std::find_if(_users.begin(), _users.end(), [&](CUser* user) {
				return user->clientId == i;
			}) == _users.end())
			{
				Log(i, LOG_INGAME, "Dentro da área sem estar presente na lista de membros da Carta de Duelo");
				DoRecall(i);
			}
		}
	}
}

bool TOD_DuelLetter::CanRegister(CUser& user)
{
	return !this->_status;
}

bool TOD_DuelLetter::Register(CUser& user, st_Item* item)
{
	if (item == nullptr)
		throw std::invalid_argument("Need to have an associate item");

	if (!CanRegister(user))
	{
		Log(user.clientId, LOG_INGAME, "Register da Carta de Duelo chamada sem ser possível de registrar");

		return false;
	}

	// Por precausão, limpamos a área por completo
	Finish(false);

	// Adicionamos o próprio líder aqui
	_users.push_back(&user);

	for (int i = 0; i < 12; i++)
	{
		auto mobId = Mob[user.clientId].PartyList[i];
		if (mobId <= 0 || mobId >= MAX_PLAYER)
			continue;

		_users.push_back(&Users[mobId]);

		Log(user.clientId, LOG_INGAME, "Carta de Duelo N - Grupo %s (%d)", Mob[mobId].Mobs.Player.Name, i);
	}

	_room = TOD_DuelLetter_Room::FirstRoom;
	_timer = DefaultTime;
	_type = GetType(item);

	if (Mob[user.clientId].Mobs.Player.Equip[10].Index == 1732)
	{
		_haveCourageSymbol = true;
		Log(user.clientId, LOG_INGAME, "Consumido Símbolo da Coragem");

		memset(&Mob[user.clientId].Mobs.Player.Equip[10], 0, sizeof st_Item);
		SendItem(user.clientId, SlotType::Equip, 10, &Mob[user.clientId].Mobs.Player.Equip[10]);
	}
	else
		Log(user.clientId, LOG_INGAME, "Não possuía o Símbolo da Coragem");

	GenerateRoom();

	int totalMob = GetMobCountOnRoom();

	const auto& position = roomPositions[_room];
	for (const auto& users : _users)
	{
		Teleportar(users->clientId, position.first, position.second);

		SendSignalParm(users->clientId, 0x7530, 0x3A1, DefaultTime);
		SendSignalParm(users->clientId, 0x7530, 0x3B0, totalMob);

		Log(users->clientId, LOG_INGAME, "Teleportado para quest %s no grupo de %s", ItemList[item->Index].Name, Mob[user.clientId].Mobs.Player.Name);
	}

	_status = true;
	return true;
}

TOD_DuelLetter_Type TOD_DuelLetter::GetType(st_Item* item) const
{
	TOD_DuelLetter_Type type = TOD_DuelLetter_Type::Arcane;
	if (item->Index == 3172)
		type = TOD_DuelLetter_Type::Normal;
	else if (item->Index == 3171)
		type = TOD_DuelLetter_Type::Mystical;

	return type;
}

int TOD_DuelLetter::GetInitialIndex() const
{
	if (_type == TOD_DuelLetter_Type::Mystical)
		return DuelLetter_Mystical_Index;
	else if (_type == TOD_DuelLetter_Type::Arcane)
		return DuelLetter_Arcane_Index;

	return DuelLetter_Normal_Index;
}

void TOD_DuelLetter::GenerateRune()
{
	// Na última sala não gera runa alguma
	if (_room == TOD_DuelLetter_Room::BossRoom)
		return;

	st_Item rune{};
	if (!(Rand() % 15))
		rune.Index = 1736;
	// Repetir a 1° runa gerada
	if (!(Rand() % 6) && _runesId.size() != 0)
		rune.Index = _runesId.begin()->second;
	else
	{
		int missingRune = 1733;

		if (_runesId.size() != 0)
		{
			for (int i = 1733; i < 1736; i++)
			{
				if (std::find_if(_runesId.begin(), _runesId.end(), [&](std::pair<TOD_DuelLetter_Room, int> rune) { return rune.second == i; }) != std::end(_runesId))
					continue;

				rune.Index = i;
			}
		}
		
		if(rune.Index == 0)
			rune.Index = 1733 + Rand() % 3;
	}

	_runesId[_room] = rune.Index;
	LogGroup("Runa %hu gerada na sala %d", rune.Index, static_cast<int>(_room));

	std::vector<TOD_DuelLetter_Room> rooms;
	if (_room == TOD_DuelLetter_Room::FirstRoom)
		rooms.push_back(TOD_DuelLetter_Room::FirstRoom);
	
	if (_room == TOD_DuelLetter_Room::SecondRoom)
	{
		rooms.push_back(TOD_DuelLetter_Room::FirstRoom);
		rooms.push_back(TOD_DuelLetter_Room::SecondRoom);
	}

	if (_room == TOD_DuelLetter_Room::ThirdRoom)
	{
		rooms.push_back(TOD_DuelLetter_Room::FirstRoom);
		rooms.push_back(TOD_DuelLetter_Room::SecondRoom);
		rooms.push_back(TOD_DuelLetter_Room::ThirdRoom);
	}

	if (_room == TOD_DuelLetter_Room::FourthRoom)
	{
		rooms.push_back(TOD_DuelLetter_Room::FirstRoom);
		rooms.push_back(TOD_DuelLetter_Room::SecondRoom);
		rooms.push_back(TOD_DuelLetter_Room::ThirdRoom);
		rooms.push_back(TOD_DuelLetter_Room::FourthRoom);
	}

	// Itera sobre todas as poisções que a runa deve ser criada
	for (size_t index = 0; index < runePositions[_room].size(); index++)
	{
		const auto& runePos = runePositions[_room][index];

		st_Item item{};
		item.Index = _runesId[rooms[index]];

		int initIndex = CreateItem(runePos.first, runePos.second, &item, 0, 0, 0);
		if (initIndex == 0)
		{
			Log(SERVER_SIDE, LOG_INGAME, "Falha ao gerar o item no chão");

			continue;
		}

		_initRunes[_room].push_back(initIndex);
	}
}

bool TOD_DuelLetter::CanGenerateBoss() const
{
	std::array haveRunes{ false, false, false };

	for (const auto& [room, rune] : _runesId)
	{
		if (rune >= 1733 && rune <= 1735)
			haveRunes[rune - 1733] = true;
	}

	return std::find(haveRunes.begin(), haveRunes.end(), false) == haveRunes.end();
}

void TOD_DuelLetter::GenerateRoom()
{
	int index = GetInitialIndex();

	int initialSum;
	int totalLoop;
	std::tie(initialSum, totalLoop) = GetGenerIndexes();

	if (_room == TOD_DuelLetter_Room::BossRoom)
	{
		if (CanGenerateBoss())
		{
			if (_type == TOD_DuelLetter_Type::Normal)
				GenerateMob(index + 11, 0, 0);
			else
				GenerateMob(index + 12, 0, 0);
		}

		return;
	}

	for (int i = 0; i < totalLoop; i++)
	{
		GenerateMob(index + i + initialSum, 0, 0);
		Log(SERVER_SIDE, LOG_INGAME, "Gerado o gener %d", index + i + initialSum);
	}

	GenerateRune();
}

std::tuple<int, int> TOD_DuelLetter::GetGenerIndexes() const
{
	if (_type == TOD_DuelLetter_Type::Normal)
	{
		int initialSum = 0;
		int totalLoop = 3;

		if (_room == TOD_DuelLetter_Room::SecondRoom)
		{
			initialSum = 3;
			totalLoop = 2;
		}

		if (_room == TOD_DuelLetter_Room::ThirdRoom)
			initialSum = 5;

		if (_room == TOD_DuelLetter_Room::FourthRoom)
		{
			initialSum = 8;
			totalLoop = 2;
		}

		if (_room == TOD_DuelLetter_Room::BossRoom)
		{
			totalLoop = 1;
			initialSum = 11;
		}

		return std::make_tuple(initialSum, totalLoop);
	}

	if (_type == TOD_DuelLetter_Type::Mystical)
	{
		int initialSum = 0;
		int totalLoop = 3;

		if (_room == TOD_DuelLetter_Room::SecondRoom)
			initialSum = 3;

		if (_room == TOD_DuelLetter_Room::ThirdRoom)
			initialSum = 6;

		if (_room == TOD_DuelLetter_Room::FourthRoom)
		{
			initialSum = 9;
			totalLoop = 2;
		}

		if (_room == TOD_DuelLetter_Room::BossRoom)
		{
			totalLoop = 1;
			initialSum = 12;
		}

		return std::make_tuple(initialSum, totalLoop);
	}

	if (_type == TOD_DuelLetter_Type::Arcane)
	{
		int initialSum = 0;
		int totalLoop = 3;

		if (_room == TOD_DuelLetter_Room::SecondRoom)
			initialSum = 3;

		if (_room == TOD_DuelLetter_Room::ThirdRoom)
			initialSum = 6;

		if (_room == TOD_DuelLetter_Room::FourthRoom)
		{
			initialSum = 9;
			totalLoop = 2;
		}

		if (_room == TOD_DuelLetter_Room::BossRoom)
		{
			totalLoop = 1;
			initialSum = 12;
		}

		return std::make_tuple(initialSum, totalLoop);
	}

	return std::make_tuple(0, 0);
}

int TOD_DuelLetter::GetMobCountOnRoom() const
{
	int initialIndex = GetInitialIndex();

	int initialSum;
	int totalLoop;
	std::tie(initialSum, totalLoop) = GetGenerIndexes();

	int totalMob = 0;
	for (int i = 0; i < totalLoop; i++)
		totalMob += mGener.pList[initialIndex + initialSum + i].MobCount;

	return totalMob;
}

TOD_DuelLetter_Room TOD_DuelLetter::GetNextRoom() const
{
	if (_room == TOD_DuelLetter_Room::FirstRoom)
		return TOD_DuelLetter_Room::SecondRoom;

	if (_room == TOD_DuelLetter_Room::SecondRoom)
		return TOD_DuelLetter_Room::ThirdRoom;

	if (_room == TOD_DuelLetter_Room::ThirdRoom)
		return TOD_DuelLetter_Room::FourthRoom;

	if (_room == TOD_DuelLetter_Room::FourthRoom)
		return TOD_DuelLetter_Room::BossRoom;

	throw std::runtime_error("Invalid room");
}

void TOD_DuelLetter::Finish(bool outOfTime)
{
	for (int i = 1000; i < 30000; i++)
	{
		if (Mob[i].GenerateID >= DuelLetter_Normal_Index && Mob[i].GenerateID <= DuelLetter_Arcane_Index + 12)
			DeleteMob(i, 1);
	}

	for (const auto& user : _users)
	{
		DoRecall(user->clientId);

		if (outOfTime)
		{
			SendClientMessage(user->clientId, "Tempo esgotado");
			Log(user->clientId, LOG_INGAME, "Jogado para fora da Carta de Duelo pois o tempo acabou");
		}
	}

	ClearArea(786, 3656, 826, 3695);
	ClearArea(782, 3599, 830, 3643);
	ClearArea(841, 3655, 886, 3695);
	ClearArea(841, 3598, 886, 3639);

	_room = TOD_DuelLetter_Room::InvalidRoom;
	_timer = 0;
	_status = false;

	_users.clear();

	// Tiramos os itens do chão
	for (auto& [room, runesList] : _initRunes)
	{
		for (const auto rune : runesList)
		{
			pInitItem[rune].Open = 0;
			g_pItemGrid[pInitItem[rune].PosY][pInitItem[rune].PosX] = 0;

			pInitItem[rune].Item = st_Item{};
		}
	}

	_runesId.clear();
	_initRunes.clear();
}