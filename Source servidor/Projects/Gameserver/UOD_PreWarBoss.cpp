#include "UOD_PreWarBoss.h"
#include "Basedef.h"
#include "SendFunc.h"

using namespace std::chrono_literals;

constexpr int WeekDay = DOMINGO;
constexpr int StartHour = 19;
constexpr int StartMinute = 00;
constexpr auto BossDuration = 25min;
constexpr auto BonusDuration = 3h;

constexpr int AnnubisGenerateId = 386;

void TOD_PreWarBoss::Work()
{
	// Retorna o tempo para o pesadelo
	time_t nowraw;
	struct tm now;

	nowraw = time(NULL);
	localtime_s(&now, &nowraw);

	if (now.tm_wday != WeekDay)
		return;

	if (!_status)
	{
		if (now.tm_hour == StartHour && now.tm_min == StartMinute)
			Start();

		if (std::chrono::steady_clock::now() - _startTime > BonusDuration)
		{
			sServer.AnnubisBonus = 0;

			for (auto& user : Users)
			{
				if (user.Status == USER_PLAY)
					Mob[user.clientId].GetCurrentScore(user.clientId);
			}
		}
	}
	else 
	{
		if (std::chrono::steady_clock::now() - _startTime > BossDuration)
			Finalize();
	}
}

void TOD_PreWarBoss::Start()
{
	_status = true;
	_startTime = std::chrono::steady_clock::now();
	
	GenerateMob(AnnubisGenerateId, 0, 0);

	for (const auto& user : Users)
	{
		if (user.Status != USER_PLAY)
			continue;

		const CMob* mob = &Mob[user.clientId];
		if (mob->Target.X / 128 == 8 && mob->Target.Y / 128 == 13)
			DoRecall(mob->clientId);
	}

	SendNotice("O boss Anubis nasceu! Derrote-o e garanta bônus");
}

void TOD_PreWarBoss::Finalize()
{
	_status = false;

	for (int i = 1000; i < MAX_SPAWN_MOB; ++i)
	{
		if (Mob[i].GenerateID == AnnubisGenerateId)
			DeleteMob(i, 1);
	}

	Log(SERVER_SIDE, LOG_INGAME, "PreWarBoss finalized");
}

void TOD_PreWarBoss::MobKilled(int killer, int killed)
{
	if (Mob[killed].GenerateID != AnnubisGenerateId)
		return;

	const CMob& mob = Mob[killer];

	int guildId = mob.Mobs.Player.GuildIndex;
	if (guildId == 0)
	{
		sServer.AnnubisBonus = -1;

		Log(killer, LOG_INGAME, "Derrotou o anúbis e concedeu o bônus para %s (%d)", g_pGuild[guildId].Name.c_str(), guildId);
		Log(SERVER_SIDE, LOG_INGAME, "O boss anúbis foi derrotado pelo usuário %s sem guild", Mob[killer].Mobs.Player.Name);

		SendNotice("O Boss Anubis foi derrotado pelo jogador %s", Mob[killer].Mobs.Player.Name);
	}
	else
	{
		bool haveBit = false;
		for (int i = 0; i < 5; i++)
		{
			if (g_pCityZone[i].owner_index == guildId || g_pCityZone[i].chall_index == guildId || g_pCityZone[i].chall_index_2 == guildId)
				haveBit = true;
		}

		if (haveBit)
		{
			sServer.AnnubisBonus = guildId;

			for (auto& user : Users)
			{
				if (user.Status == USER_PLAY || Mob[user.clientId].Mobs.Player.GuildIndex == guildId)
					Mob[user.clientId].GetCurrentScore(user.clientId);
			}

			Log(SERVER_SIDE, LOG_INGAME, "A guild em questão possuía aposta ou era dono da cidade. Entregue o bônus de status");
		}

		g_pGuild[guildId].Fame += 100;

		Log(killer, LOG_INGAME, "Derrotou o anúbis e concedeu o bônus para %s (%d)", g_pGuild[guildId].Name.c_str(), guildId);
		Log(SERVER_SIDE, LOG_INGAME, "O boss anúbis foi derrotado pelo usuário %s - Concedido bônus para %s (%d). Fame nova: %d", Mob[killer].Mobs.Player.Name, g_pGuild[guildId].Name.c_str(), guildId, g_pGuild[guildId].Fame);

		SendNotice("O Boss Anubis foi derrotado pelo jogador %s da guild %s", Mob[killer].Mobs.Player.Name, g_pGuild[guildId].Name.c_str());
	}

	Finalize();
}