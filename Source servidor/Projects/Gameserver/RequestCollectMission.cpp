#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "UOD_EventManager.h"
#include "UOD_Mission.h"
#include <ctime>
#include <algorithm>
#include <sstream>

bool CUser::RequestCollectMission(PacketHeader* header)
{
	auto& daily = Mob[clientId].Mobs.DailyQuest;
	if (daily.QuestId == -1)
	{
		SendClientMessage(clientId, "Você não tem uma em andamento para coletar");

		Log(clientId, LOG_INGAME, "Tentou coletar a premiação de uma missão sem ter uma no momento ativa");
		return true;
	}

	if (!daily.IsAccepted)
	{
		SendClientMessage(clientId, "Você não tem uma em andamento para coletar");

		Log(clientId, LOG_INGAME, "Tentou coletar a premiação de uma missão sem ter aceitado");
		return true;
	}

	const auto& quest = std::find_if(std::begin(sServer.Missions), std::end(sServer.Missions), [&](const TOD_MissionInfo& info) {
		return info.QuestId == daily.QuestId;
	});

	if (quest == std::end(sServer.Missions))
	{
		Log(clientId, LOG_INGAME, "Não foi encontrado a missão de id %d", daily.QuestId);

		SendClientMessage(clientId, "Falha ao recolher a recompensa");
		return true;
	}

	bool isMobGoalCompleted = true;
	for (size_t i = 0; i < 5; i++)
	{
		auto killed = daily.MobCount[i];
		if (quest->Mob.Amount[i] != 0 && killed < quest->Mob.Amount[i])
			isMobGoalCompleted = false;
	}

	bool isDropGoalCompleted = true;
	for (size_t i = 0; i < 5; i++)
	{
		auto dropped = daily.ItemCount[i];
		if (quest->Drop.Amount[i] != 0 && dropped < quest->Drop.Amount[i])
			isDropGoalCompleted = false;
	}

	if (!isMobGoalCompleted || !isDropGoalCompleted)
	{
		SendClientMessage(clientId, "Você ainda não completou sua missão");

		return true;
	}

	CMob& mob = Mob[clientId];

	// Tudo pronto para coletar... vamos lá ;) 
	unsigned long long rewardGold = static_cast<unsigned long long>(quest->Gold);
	if (sServer.GoldDailyBonus != 0)
		rewardGold = rewardGold * sServer.GoldDailyBonus / 100;

	auto gold = static_cast<unsigned long long>(mob.Mobs.Player.Gold) + rewardGold;
	if (gold > 2000000000LL)
		mob.Mobs.Player.Gold = 2000000000;
	else
		mob.Mobs.Player.Gold = static_cast<int>(gold);

	Log(clientId, LOG_INGAME, "Recebido %d de gold da Missão. Total de gold: %d", rewardGold, gold);
	long long exp = quest->Exp;
	if (sServer.ExpDailyBonus != 0)
		exp = exp * sServer.ExpDailyBonus / 100;

	int evolution = Mob[clientId].Mobs.Player.Equip[0].Effect[1].Value;
	int level = Mob[clientId].Mobs.Player.Status.Level;
	if ((evolution <= ARCH && level == MAX_LEVEL) || (evolution >= CELESTIAL && level >= MAX_LEVEL_CELESTIAL))
		exp = 0;

	if (mob.Mobs.Player.Equip[0].Effect[1].Value != MORTAL)
		exp = GetExpApply_2(exp, clientId, clientId, false);

	Log(clientId, LOG_INGAME, "Recebido %lld de experiência da Missão", exp);
	mob.Mobs.Player.Exp += exp;

	auto giveReward = [](int clientId, const std::array<TOD_DailyQuestInfo_RewardItem, 6> reward) {
		for (const auto& reward : reward)
		{
			if (reward.Item.Index <= 0 || reward.Item.Index >= MAX_ITEMLIST)
				continue;

			for (size_t i = 0; i < reward.Amount; i++)
			{
				int slotId = GetFirstSlot(clientId, 0);
				if (!PutItem(clientId, const_cast<st_Item*>(&reward.Item)))
				{
					Log(clientId, LOG_INGAME, "Não foi possível receber o item da premiação da Missão. Loop: %u. Total: %u", i, reward.Amount);

					SendClientMessage(clientId, "Falta espaço no inventário");
					break;
				}

				Log(clientId, LOG_INGAME, "Recebido o item %s %s da premiação da Missão", ItemList[reward.Item.Index].Name, reward.Item.toString().c_str());
				SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[reward.Item.Index].Name);
			}
		}
	};

	giveReward(clientId, quest->FreeReward);

	auto passTimeRemaining = TimeRemaining(daily.BattlePass);
	if (passTimeRemaining > 0.0)
	{
		Log(clientId, LOG_INGAME, "Recebimento da premiação referente ao Passe de Batalha");
		giveReward(clientId, quest->BattlePassReward);
	}

	SendClientMessage(clientId, "Recompensa coletada com sucesso");
	Log(clientId, LOG_INGAME, "Coletado recompensa da missão");

	while (mob.CheckGetLevel())
		;

	SendEtc(clientId);

	auto mission = static_cast<TOD_Mission*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::Mission));
	if (mission != nullptr)
		mission->Reset(*this);

	SendMissionInfo(clientId);
	return true;
}