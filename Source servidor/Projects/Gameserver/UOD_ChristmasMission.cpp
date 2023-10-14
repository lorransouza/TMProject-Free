#include "Basedef.h"	
#include "SendFunc.h"
#include "UOD_ChristmasMission.h"

constexpr auto TimeToAccept = 3min;
constexpr auto TimeToComplete = 15min;

TOD_ChristmasMission::TOD_ChristmasMission(std::chrono::milliseconds interval)
	: TOD_Event(interval)
{
}

void TOD_ChristmasMission::Work()
{
	if (sServer.Christmas.Missions.size() == 0)
		return;

	auto now = std::chrono::steady_clock::now();
	std::vector<CUser*> users;
	for (auto& user : Users)
	{
		if (user.Status != USER_PLAY)
			continue;

		auto& mission = user.Christmas.Mission;

		if (user.Christmas.Mission.Status == TOD_ChristmasMission_Status::WaitingNextRound)
		{
			if (now - mission.LastUpdate <= mission.NextMission)
				continue;

			users.push_back(&user);
		}
		else if (user.Christmas.Mission.Status == TOD_ChristmasMission_Status::Accepted)
		{
			if (now - mission.LastUpdate <= TimeToComplete)
				continue;

			Log(user.clientId, LOG_INGAME, "A missão não pôde ser completada em tempo hábil.");
			SendClientMessage(user.clientId, "A missão não pôde ser completada em tempo hábil.");
			SetNextRound(user);
		}
		else if (user.Christmas.Mission.Status == TOD_ChristmasMission_Status::WaitingAnswer)
		{
			if (now - mission.LastUpdate <= TimeToAccept)
				continue;

			Log(user.clientId, LOG_INGAME, "Não aceitou a missão a tempo.");
			SetNextRound(user);
		}
	}

	if (users.size() == 0)
		return;

	for (auto& user : users)
	{
		auto mission = select_randomly(std::begin(sServer.Christmas.Missions), std::end(sServer.Christmas.Missions));
		auto& userMission = user->Christmas.Mission;

		userMission.MissionId = mission->Id;
		
		std::fill(std::begin(userMission.Count), std::end(userMission.Count), 0);

		user->Christmas.Mission.Status = TOD_ChristmasMission_Status::WaitingAnswer;
		user->Christmas.Mission.LastUpdate = std::chrono::steady_clock::now();

		SendChristmasMission(user->clientId, 0);
	}
}

void TOD_ChristmasMission::SetNextRound(CUser& user)
{
	SendChristmasMission(user.clientId, 2);

	user.Christmas.Mission.Status = TOD_ChristmasMission_Status::WaitingNextRound;
	user.Christmas.Mission.MissionId = -1;

	int nextRoundTime = (Rand() % 30) + 10;
	user.Christmas.Mission.NextMission = std::chrono::milliseconds(nextRoundTime * 1000 * 60);

	Log(user.clientId, LOG_INGAME, "A próxima missão acontecerá em aproximadamente %d minutos", nextRoundTime);

	// Limpamos a contagem
	std::fill(std::begin(user.Christmas.Mission.Count), std::end(user.Christmas.Mission.Count), 0);

	user.Christmas.Mission.LastUpdate = std::chrono::steady_clock::now();
}

bool TOD_ChristmasMission::HandleReply(CUser& user, pMsgSignal* packet)
{
	if (user.Christmas.Mission.Status != TOD_ChristmasMission_Status::WaitingAnswer)
	{
		SendClientMessage(user.clientId, "Você já aceitou a missão ou não tem uma missão para ser aceita");

		Log(user.clientId, LOG_INGAME, "Aceitou uma missão sem estar esperando uma. Recusando a missão");
		return true;
	}

	if (packet->Value == 0)
	{
		SendClientMessage(user.clientId, "Você recusou a missão. Que pena.");

		Log(user.clientId, LOG_INGAME, "Rejeitou a missão do Papai Noel.");
		SetNextRound(user);
		return true;
	}

	user.Christmas.Mission.Status = TOD_ChristmasMission_Status::Accepted;
	user.Christmas.Mission.LastUpdate = std::chrono::steady_clock::now();
	SendClientMessage(user.clientId, "Missão aceita. Você tem 15 minutos para entregar a missão");

	Log(user.clientId, LOG_INGAME, "Missão foi aceita");

	SendChristmasMission(user.clientId, 1);
	return true;
}

void TOD_ChristmasMission::RefreshChristmasMission(int clientId, int mobId)
{
	if (mobId < MAX_PLAYER || clientId >= MAX_PLAYER)
		return;

	CUser& user = Users[clientId];
	auto& userMissionInfo = user.Christmas.Mission;

	if (userMissionInfo.MissionId == -1 || userMissionInfo.Status != TOD_ChristmasMission_Status::Accepted)
		return;

	bool anyChange = false;
	std::string mobName{ Mob[mobId].Mobs.Player.Name };

	auto missionIt = std::find_if(std::begin(sServer.Christmas.Missions), std::end(sServer.Christmas.Missions), [&](const TOD_ChristmasMissionInfo& info) {
		return userMissionInfo.MissionId == info.Id;
	});

	if (missionIt == std::end(sServer.Christmas.Missions))
	{
		SendClientMessage(clientId, "Esta missão está inválida");

		SetNextRound(Users[clientId]);
		return;
	}

	auto& mission = *missionIt;
	for (INT32 i = 0; i < 3; i++)
	{
		if (!mission.MobName[i][0])
			continue;

		if (mobName != mission.MobName[i] || userMissionInfo.Count[i] >= mission.Count[i])
			continue;

		userMissionInfo.Count[i] ++;
		anyChange = true;
	}

	if (anyChange)
		SendChristmasMission(clientId, 1);

	bool completed = true;
	for (int i = 0; i < 3; i++)
	{
		if (!mission.MobName[i][0])
			continue;

		if (userMissionInfo.Count[i] < mission.Count[i])
			completed = false;
	}

	if (!completed)
		return;

	for (const auto& item : mission.Reward)
	{
		if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
			continue;

		int slotId = GetFirstSlot(clientId, 0);
		if (slotId == -1)
		{
			SendClientMessage(clientId, "Falta espaço no inventário");

			Log(clientId, LOG_INGAME, "Faltou espaço no inventário para receber a premiação. Item %s %s", ItemList[item.Index].Name, item.toString().c_str());
			break;
		}

		Mob[clientId].Mobs.Player.Inventory[slotId] = item;
		SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);

		Log(clientId, LOG_INGAME, "Recebeu o item %s %s", ItemList[item.Index].Name, item.toString().c_str());
		SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item.Index].Name);
	}

	SendClientMessage(clientId, "Missão foi concluída com sucesso.");
	SetNextRound(Users[clientId]);
}
