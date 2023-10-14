#include "UOD_Mission.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

constexpr auto TimeToComplete = 6.0 * 60.0 * 60.0;
constexpr auto TimeToNextMission = 6.0 * 60.0 * 60.0;

void TOD_Mission::Work() 
{
	const auto& dailyQuests = sServer.Missions;
	if (dailyQuests.size() == 0)
		return;

	if (!_status)
		return;

	std::time_t rawnow = std::time(nullptr);
	struct tm now; localtime_s(&now, &rawnow);

	for (auto& user : Users)
	{
		if (user.Status != USER_PLAY)
			continue;

		auto& mob = Mob[user.clientId];
		if (mob.Mobs.Player.Status.Level < 69 && mob.Mobs.Player.Equip[0].Effect[1].Value == MORTAL)
			continue;

		auto now_time_t = std::mktime(&now);
		auto diffTime = std::difftime(now_time_t, mob.Mobs.DailyQuest.LastUpdate.GetTMStruct());

		if (mob.Mobs.DailyQuest.LastUpdate.Ano == 0)
			diffTime = TimeToNextMission + 1;

		if (!mob.Mobs.DailyQuest.IsAccepted)
		{
			if (diffTime > TimeToNextMission)
				SetNextDailyQuest(user);
		}
		else
		{
			if (diffTime > TimeToComplete)
				UncompletedMission(user);
		}
	}
}

void TOD_Mission::SetNextDailyQuest(CUser& user)
{
	std::vector<const TOD_MissionInfo*> availableQuests;

	CMob& mob = Mob[user.clientId];

	int evolution = mob.Mobs.Player.Equip[0].Effect[1].Value;
	for (const auto& quest : sServer.Missions)
	{
		// Procura se alguma das evoluções para aquela quest é da evolução do personagem em questão
		if (std::find(std::begin(quest.ClassMaster), std::end(quest.ClassMaster), evolution) == std::end(quest.ClassMaster))
			continue;

		int level = mob.Mobs.Player.Status.Level;
		if (evolution == MORTAL && (level < quest.MinLevel || level > quest.MaxLevel))
			continue;

		availableQuests.push_back(&quest);
	}

	if (availableQuests.size() == 0)
	{
		Log(user.clientId, LOG_INGAME, "Não há quests disponíveis para este personagem");

		SetLastUpdateMission(user);
		return;
	}

	auto& dailyQuest = mob.Mobs.DailyQuest;
	auto quest = *select_randomly(std::begin(availableQuests), std::end(availableQuests));
	dailyQuest.QuestId = quest->QuestId;
	dailyQuest.IsAccepted = false;

	std::fill(std::begin(dailyQuest.ItemCount), std::end(dailyQuest.ItemCount), 0);
	std::fill(std::begin(dailyQuest.MobCount), std::end(dailyQuest.MobCount), 0);

	SetLastUpdateMission(user);
	SendMissionInfo(user.clientId);

	SendClientMessage(user.clientId, "!Chegou uma nova missão!");

	Log(user.clientId, LOG_INGAME, "Recebeu a missão %s (%d)", quest->QuestName.c_str(), quest->QuestId);
}

void TOD_Mission::UncompletedMission(CUser& user)
{
	SendClientMessage(user.clientId, "!Você não completou sua missão a tempo. Que pena...");
	Log(user.clientId, LOG_INGAME, "O usuário não completou a missão em tempo hábio. Missão cancelada");
	
	Reset(user);
}

void TOD_Mission::Reset(CUser& user) const
{
	SetLastUpdateMission(user);

	auto& daily = Mob[user.clientId].Mobs.DailyQuest;
	daily.IsAccepted = false;
	daily.QuestId = -1;

	std::fill(std::begin(daily.ItemCount), std::end(daily.ItemCount), 0);
	std::fill(std::begin(daily.MobCount), std::end(daily.MobCount), 0);

	Log(user.clientId, LOG_INGAME, "Resetado missão do usuário");
}

void TOD_Mission::SetLastUpdateMission(CUser& user) const
{
	CMob& mob = Mob[user.clientId];

	std::time_t rawnow = std::time(nullptr);
	struct tm now; localtime_s(&now, &rawnow);

	auto& lastUpdate = mob.Mobs.DailyQuest.LastUpdate;
	lastUpdate.Ano = 1900 + now.tm_year;
	lastUpdate.Mes = now.tm_mon + 1;
	lastUpdate.Dia = now.tm_mday;
	lastUpdate.Hora = now.tm_hour;
	lastUpdate.Minuto = now.tm_min;
	lastUpdate.Segundo = now.tm_sec;

	Log(user.clientId, LOG_INGAME, "Foi atualizado o LastUpdate da missão diária para %02d/%02d/%02d %02d:%02d:%02d",
		lastUpdate.Dia, lastUpdate.Mes, lastUpdate.Ano, lastUpdate.Hora, lastUpdate.Minuto, lastUpdate.Segundo);
}

void TOD_Mission::MobKilled(int killer, int killed) const
{
	if (killed < MAX_PLAYER)
		return;

	// Checa se é evocação
	if (killer >= MAX_PLAYER && Mob[killer].Mobs.Player.CapeInfo != 4)
		return;

	if (killer >= MAX_PLAYER && Mob[killer].Mobs.Player.CapeInfo == 4)
		killer = Mob[killer].Summoner;

	if (killer >= MAX_PLAYER)
		return;

	std::string mobName{ Mob[killed].Mobs.Player.Name };

	INT32 leaderId = Mob[killer].Leader;
	if (leaderId == 0)
		leaderId = killer;

	for (size_t i = 0; i < 5; i++)
	{
		for (int iParty = 0; iParty < 13; iParty++)
		{
			int memberId = leaderId;
			if (iParty != 12)
				memberId = Mob[leaderId].PartyList[iParty];

			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			auto& daily = Mob[memberId].Mobs.DailyQuest;;
			if (daily.QuestId == -1 || !daily.IsAccepted)
				continue;

			if (memberId != killer)
			{
				INT32 distance = GetDistance(Mob[memberId].Target.X, Mob[memberId].Target.Y, Mob[killer].Target.X, Mob[killer].Target.Y);

				if (distance > HALFGRIDX)
					continue;
			}

			const auto& questIt = std::find_if(std::begin(sServer.Missions), std::end(sServer.Missions), [&](const TOD_MissionInfo& info) {
				return info.QuestId == daily.QuestId;
			});

			if (questIt == std::end(sServer.Missions))
			{
				Reset(Users[memberId]);
				continue;
			}

			const auto& quest = *questIt;
			if (quest.Mob.mobName[i].empty())
				continue;

			if (quest.Mob.mobName[i] != mobName || daily.MobCount[i] >= quest.Mob.Amount[i])
				continue;

			daily.MobCount[i] ++;

			Log(memberId, LOG_INGAME, "%s matou o mob %s da quest %s (%d). Total: %03d/%03d", Mob[killer].Mobs.Player.Name, mobName.c_str(), questIt->QuestName.c_str(), questIt->QuestId, daily.MobCount[i], quest.Mob.Amount[i]);
			SendMissionInfo(memberId);
		}
	}
}