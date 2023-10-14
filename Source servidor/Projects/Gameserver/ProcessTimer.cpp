#include "cServer.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "Basedef.h"
#include "UOD_EventManager.h"

using namespace std::chrono_literals;

void ProcessSendTimer()
{
	for (int i = 1; i < MAX_PLAYER; ++i)
	{
		if (Users[i].Status == USER_EMPTY)
			continue;

		if (Users[i].Socket.nSendPosition == 0)
			continue;

		Users[i].SendMessageA();
	}
}

void ProcessSecTimer()
{
	// Cada segundo incrementa 1
	// !(counter % 10) quer dizer que passaram-se 10 segundos
	int counter = sServer.SecCounter;

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	counter++;

	CurrentTime = GetTickCount();

	auto nowChrono = std::chrono::steady_clock::now();
	for (auto&[type, event] : TOD_EventManager::GetInstance().GetEvents())
	{
		if (nowChrono - event->GetLastExecution() <= event->GetInterval())
			continue;

		event->Fire();
		event->SetLastExecution(nowChrono);
	}

	if (!(counter % 2))
		PremiumTime();

	if (!(counter % 2))
	{
		for (auto& user : Users)
		{
			if (user.Status < USER_SELCHAR)
				continue;

			user.Time++;
			user.LogEquipsAndInventory(false);
		}
	}

	if (!(counter % 2) && sServer.AutoTradeEvent.Status)
	{
		auto now = std::chrono::high_resolution_clock::now();
		for (int clientId = 1; clientId < MAX_PLAYER; clientId++)
		{
			CUser& user = Users[clientId];
			if (user.Status != USER_PLAY)
				continue;

			if (!user.EventAutoTrade.IsValid || now - user.EventAutoTrade.LastTime <= user.EventAutoTrade.TimeToWin)
				continue;

			st_Item *item = &sServer.AutoTradeEvent.item;
			if (PutItem(clientId, &sServer.AutoTradeEvent.item))
			{
				Log(clientId, LOG_INGAME, "[EVENTO] AutoLoja - Recebido [%s] [%d] [%d %d %d %d %d %d]", ItemList[item->Index].Name, item->Index,
					item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3);

				SendClientMessage(clientId, "!Chegou um item [%s]", ItemList[item->Index].Name);
			}
			else
			{
				Log(clientId, LOG_INGAME, "[EVENTO] AutoLoja - Não recebeu por falta de espaço", ItemList[item->Index].Name, item->Index,
					item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3);

				SendClientMessage(clientId, "!Chegou um item [%s]", ItemList[item->Index].Name);
			}

			user.EventAutoTrade.LastTime = now;
			user.GenerateNewAutoTradeTime();
		}
	}

	// Se estiver com a RvR ativa
	if (sServer.RvR.Status)
	{
		for (auto& user : Users)
		{
			if (user.Status != USER_PLAY)
				continue;

			auto& mob = Mob[user.clientId];
			if (mob.Mobs.Player.CapeInfo != CAPE_BLUE && mob.Mobs.Player.CapeInfo != CAPE_RED)
				continue;

			if (mob.Mobs.Player.Status.curHP >= mob.Mobs.Player.Status.maxHP)
				continue;

			bool isOnBase = false;
			if (mob.Target.X >= 1230 && mob.Target.X <= 1244 && mob.Target.Y >= 1949 && mob.Target.Y <= 1987)
				isOnBase = true;
			else if (mob.Target.X >= 1205 && mob.Target.X <= 1244 && mob.Target.Y >= 1949 && mob.Target.Y <= 1962)
				isOnBase = true;
			else if (mob.Target.X >= 1056 && mob.Target.X <= 1068 && mob.Target.Y >= 2108 && mob.Target.Y <= 2146)
				isOnBase = true;
			else if (mob.Target.X >= 1056 && mob.Target.X <= 1095 && mob.Target.Y >= 2132 && mob.Target.Y <= 2142)
				isOnBase = true;

			if (!isOnBase)
				continue;

			int heal = mob.Mobs.Player.Status.maxHP * 5 / 100;
			INT32 totalHp = mob.Mobs.Player.Status.curHP + heal;
			if (totalHp > mob.Mobs.Player.Status.maxHP)
				totalHp = mob.Mobs.Player.Status.maxHP;

			Mob[user.clientId].Mobs.Player.Status.curHP = totalHp;
			user.Potion.CountHp += heal;

			SetReqHp(user.clientId);
			SetReqMp(user.clientId);

			p18A packet{};
			packet.Header.PacketId = 0x18A;
			packet.Header.ClientId = user.clientId;
			packet.Header.Size = sizeof p18A;

			packet.CurHP = totalHp;
			packet.Incress = heal;

			GridMulticast_2(mob.Target.X, mob.Target.Y, reinterpret_cast<BYTE*>(&packet), 0);
		}
	}

	if (!(counter % 2))
	{
		for (auto& boss : sServer.Boss)
		{
			if (boss.GenerGenerated == 0 && boss.Geners.size() != 0)
			{
				if (nowChrono - boss.LastUpdate < std::chrono::milliseconds(boss.TimeToReborn * 1000))
					continue;

				auto generId = *select_randomly(std::begin(boss.Geners), std::end(boss.Geners));
				GenerateMob(generId, 0, 0);

				boss.LastUpdate = nowChrono;
				boss.GenerGenerated = generId;

				for (int i = 1000; i < MAX_SPAWN_MOB; ++i)
				{
					if (Mob[i].Mode == 0 || Mob[i].GenerateID != boss.GenerGenerated || Mob[i].Leader != 0)
						continue;

					Mob[i].BossInfoId = boss.Index;
					break;
				}

				SendNotice("Boss %s renasceu", mGener.pList[generId].Leader.Name);
				Log(SERVER_SIDE, LOG_INGAME, "Boss \"%s\" nasceu", mGener.pList[generId].Leader.Name);
			}
			else
			{
				if (nowChrono - boss.LastUpdate < std::chrono::milliseconds(boss.MaxTimeIngame * 1000))
					continue;

				int i;
				for (i = 1000; i < MAX_SPAWN_MOB; ++i)
				{
					if (Mob[i].Mode == 0 || Mob[i].GenerateID != boss.GenerGenerated || Mob[i].Leader != 0)
						continue;

					if (Mob[i].Mode == 5)
						break;
				}

				if (i != MAX_SPAWN_MOB)
					continue;

				for (i = 1000; i < MAX_SPAWN_MOB; ++i)
				{
					if (Mob[i].Mode == 0 || Mob[i].GenerateID != boss.GenerGenerated)
						continue;

					DeleteMob(i, 3);
				}

				boss.LastUpdate = nowChrono;
				boss.GenerGenerated = 0;
			}
		}
	}

	// a cada 1 segundo
	if (!(counter % 2) && sServer.BossEvent.Status)
	{
		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			INT32 has = -1;
			for (INT32 t = 0; t < 32; t++)
			{
				if (Mob[i].Mobs.Affects[t].Index == 56)
				{
					has = t;

					break;
				}
			}

			if (has < 0)
				continue;

			Mob[i].Mobs.Affects[has].Value++;

			if (Mob[i].Mobs.Affects[has].Value >= 600)
			{
				st_Item item;
				memcpy(&item, &sServer.BossEvent.item, sizeof st_Item);

				if ((Rand() % 100) <= 20)
					item.Index++;

				if (PutItem(i, &item))
				{
					SendClientMessage(i, "!Chegou um item: [%s]", ItemList[item.Index].Name);
					Log(i, LOG_INGAME, "Recebeu item [%s] após 10 minutos online", ItemList[item.Index].Name);
				}
				else
				{
					SendClientMessage(i, "!Não recebeu o item do evento por falta de espaço");
					Log(i, LOG_INGAME, "NÃO Recebeu item [%s] após 10 minutos online", ItemList[item.Index].Name);
				}

				Mob[i].Mobs.Affects[has].Value = 0;
			}
		}
	}

	if (!(counter % 2) && (now.tm_min == 10 || now.tm_min == 30 || now.tm_min == 50) && now.tm_sec == 0)
	{
		ClearArea(1282, 217, 1152, 130);

		for (int i = 0; i < 8; i++)
		{
			sServer.Nightmare[2].Alive[i] = false;
			GenerateMob(g_pPesaGenerate[2][i][0], 0, 0);
		}

		sServer.Nightmare[2].Status = 1;
		sServer.Nightmare[2].TimeLeft = 240;
	}

	if (!(counter % 2) && (now.tm_min == 5 || now.tm_min == 25 || now.tm_min == 45) && now.tm_sec == 0)
	{
		ClearArea(1130, 334, 1049, 272);

		for (int i = 0; i < 8; i++)
		{
			sServer.Nightmare[1].Alive[i] = false;
			GenerateMob(g_pPesaGenerate[1][i][0], 0, 0);
		}

		sServer.Nightmare[1].Status = 1;
		sServer.Nightmare[1].TimeLeft = 240;
	}

	if (!(counter % 2) && (!now.tm_min || now.tm_min == 20 || now.tm_min == 40) && !now.tm_sec)
	{
		ClearArea(1329, 364, 1294, 301);

		for (int i = 0; i < 5; i++)
		{
			sServer.Nightmare[0].Alive[i] = false;
			GenerateMob(g_pPesaGenerate[0][i][0], 0, 0);
		}

		sServer.Nightmare[0].Status = 0;
		sServer.Nightmare[0].TimeLeft = 240;
	}

	// Sistema de Poções
	{
		for (INT32 LOCAL_103 = 1; LOCAL_103 < MAX_PLAYER; LOCAL_103++)
		{
			if (Users[LOCAL_103].Status != USER_PLAY)
				continue;

			if (!Mob[LOCAL_103].Mobs.Player.Status.curHP)
				continue;

			Users[LOCAL_103].Potion.bQuaff = 0;

			INT32 LOCAL_104 = ApplyHp(LOCAL_103);
			INT32 LOCAL_105 = ApplyMp(LOCAL_103);

			if (LOCAL_104 != 0)
				SendScore(LOCAL_103);
			else if (LOCAL_105 != 0)
				SendSetHpMp(LOCAL_103);
		}
	}

	if (!(counter % 4))
	{
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			if (Mob[i].IsInsideValley())
			{
				if (Mob[i].Mobs.Player.Equip[13].Index != 3917 && Mob[i].Mobs.Player.Equip[13].Index != 3916)
				{
					DoRecall(i);

					Log(i, LOG_HACK, "Usuário removido por não estar com a Fada do Vale equipada na área do vale. %ux %uy", Mob[i].Target.X, Mob[i].Target.Y);
					Log(SERVER_SIDE, LOG_HACK, "Usuário %s removido da área do Vale por não estar com a fada equipada. %u %u", Users[i].User.Username, Mob[i].Target.X, Mob[i].Target.Y);
				}
			}
		}
	}

	// Reenvia a quest diaria pra geral ^^ xD xD xD
	if (now.tm_hour == 0 && now.tm_min == 0 && !sServer.StatusDaily)
	{
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			if (now.tm_yday != Users[i].User.Water.Day)
			{
				Log(i, LOG_INGAME, "Resetado a contagem de entradas água! Day: %d-%d. Total: %d", Users[i].User.Water.Day, now.tm_yday, Users[i].User.Water.Total);

				Users[i].User.Water.Day = now.tm_yday;
				Users[i].User.Water.Total = 0;

				LogPlayer(i, "Resetado a contagem de entradas água");
			}

			if (now.tm_yday != Mob[i].Mobs.Contribuition.DayOfYear)
			{
				Log(i, LOG_INGAME, "Resetado contagem de pontos de contribuição. Day: %d-%d. Total de pontos: %d. Limite: %d", now.tm_yday, Mob[i].Mobs.Contribuition.DayOfYear, Mob[i].Mobs.Contribuition.Points,
					Mob[i].Mobs.Contribuition.Limit);

				Mob[i].Mobs.Contribuition.DayOfYear = now.tm_yday;
				memset(&Mob[i].Mobs.Contribuition.Count, 0, sizeof Mob[i].Mobs.Contribuition.Count);
			}

			if (Mob[i].Mobs.Player.Inventory[60].Index == 3467 && !Mob[i].isBagActive(TOD_Bag::FirstBag))
			{
				Log(i, LOG_INGAME, "Bolsa do Andarilho (1) expirou. %s", Mob[i].Mobs.Player.Inventory[60].toString().c_str());

				memset(&Mob[i].Mobs.Player.Inventory[60], 0, sizeof st_Item);
				SendItem(i, SlotType::Inv, 60, &Mob[i].Mobs.Player.Inventory[60]);
			}

			if (Mob[i].Mobs.Player.Inventory[61].Index == 3467 && !Mob[i].isBagActive(TOD_Bag::SecondBag))
			{
				Log(i, LOG_INGAME, "Bolsa do Andarilho (2) expirou. %s", Mob[i].Mobs.Player.Inventory[61].toString().c_str());

				memset(&Mob[i].Mobs.Player.Inventory[61], 0, sizeof st_Item);
				SendItem(i, SlotType::Inv, 61, &Mob[i].Mobs.Player.Inventory[61]);
			}

			Log(i, LOG_INGAME, "Usuário online desde o dia anterior");
			Log(i, LOG_INGAME, "IP: %s Mac: (%02X:%02X:%02X:%02X:%02X:%02X)", Users[i].IP, Users[i].MacAddress[0], Users[i].MacAddress[1], Users[i].MacAddress[2],
				Users[i].MacAddress[3], Users[i].MacAddress[4], Users[i].MacAddress[5]);

			Log(i, LOG_INGAME, Users[i].LogSameAccounts().c_str());
		}

		sServer.StatusDaily = 1;

		for (INT32 i = 0; i < 5; i++)
			Log(SERVER_SIDE, LOG_INGAME, "Imposto cidade %s: %I64d", szCitys[i], g_pCityZone[i].impost);
	}

	if (now.tm_hour == 0 && now.tm_min == 1 && sServer.StatusDaily == 1)
		sServer.StatusDaily = 0;

	if (!(counter % 2))
	{
		for (INT32 LOCAL_102 = 0; LOCAL_102 < MAX_PLAYER; LOCAL_102++)
		{
			if (sServer.SaveCount >= MAX_PLAYER)
				sServer.SaveCount = 1;

			if (Users[sServer.SaveCount].Status == USER_PLAY || Mob[sServer.SaveCount].Mode != 0)
			{
				SaveUser(sServer.SaveCount, 0);

				sServer.SaveCount++;
				break;
			}

			sServer.SaveCount++;
		}
	}

	ProcessDecayItem();

	INT32 LOCAL_106 = 0;
	INT32 LOCAL_7 = counter % 4;
	if (LOCAL_7 == 0)
	{
		DoWater();
		DoNightmare();
		DoRune();

		ProcessRanking();
	}

	if (!(counter % 2))
	{
		//	AnswerClient_aHack(counter);
		GuildProcess();
	}

	/*if(now.tm_hour == 21 && now.tm_min == 0 && sServer.RvR.Status == 0)
	{
		GenerateMob(TORRE_RVR, 0, 0);
		GenerateMob(TORRE_RVR + 1, 0, 0);

		sServer.RvR.Status = 1;

		SendNotice("Guerra RvR foi iniciada!");

		Log(SERVER_SIDE, LOG_ADMIN, "RvR iniciada com sucesso");
	}*/

	if (now.tm_wday >= SEGUNDA && now.tm_wday <= SEXTA)
	{
		if (now.tm_hour == sServer.TowerWar.Hour && now.tm_min == 0 && sServer.TowerWar.Status == 0)
			InitializeTowerWar();
		else if (now.tm_hour == sServer.TowerWar.Hour && now.tm_min == 30 && sServer.TowerWar.Status == 1)
			FinalizeTowerWar();
	}

	// Inicia o RvR
	if (now.tm_wday >= SEGUNDA && now.tm_wday <= SABADO && sServer.NoviceChannel)
	{
		if (now.tm_hour == sServer.RvR.Hour - 1 && now.tm_min == 55 && !sServer.RvR.Annoucement)
		{
			SendNotice("A guerra de reinos iniciará em 5 minutos. Acesse o teleporte de Noatun.");

			sServer.RvR.Annoucement = 1;
		}
		else if (now.tm_hour == sServer.RvR.Hour && now.tm_min == 0 && !sServer.RvR.Status)
		{
			sServer.RvR.Status = 1;
			SendNotice("Guerra entre Reinos iniciada. Acesse o teleporte de Noatun!");

			GenerateMob(TORRE_RVR_BLUE, 0, 0);
			GenerateMob(TORRE_RVR_RED, 0, 0);

			Log(SERVER_SIDE, LOG_ADMIN, "Portal RvR liberado.");
		}
		else if (now.tm_hour == sServer.RvR.Hour && now.tm_min < 30 && !(now.tm_min % 2) && !sServer.RvR.Annoucement_Point && sServer.RvR.Status == 1)
		{
			pD1D packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xD1D;
			packet.Header.ClientId = 0x7530;
			packet.Header.Size = sizeof pD1D;

			sprintf_s(packet.eMsg, "Guerra Reino vs Reino");
			AddMessageDB((BYTE*)&packet, sizeof pD1D);
			sprintf_s(packet.eMsg, "Red: %d - Blue: %d", sServer.RvR.Points[1], sServer.RvR.Points[0]);
			AddMessageDB((BYTE*)&packet, sizeof pD1D);

			sServer.RvR.Annoucement_Point = 1;
		}
		else if (now.tm_hour == sServer.RvR.Hour && now.tm_min < 30 && (now.tm_min % 2) && sServer.RvR.Annoucement_Point && sServer.RvR.Status == 1)
			sServer.RvR.Annoucement_Point = 0;
		else if (now.tm_hour == sServer.RvR.Hour && now.tm_min == 30 && sServer.RvR.Status == 1)
		{
			for (INT32 i = 1000; i < MAX_SPAWN_MOB; i++)
			{
				if (Mob[i].GenerateID == TORRE_RVR_BLUE || Mob[i].GenerateID == TORRE_RVR_RED)
					MobKilled(i, 29999, 0, 0);
			}

			int winner = 0;
			if (sServer.RvR.Points[0] > sServer.RvR.Points[1])
			{
				SendNotice("O reino azul foi o vencedor do RvR com %d pontos vs %d!", sServer.RvR.Points[0], sServer.RvR.Points[1]);
				Log(SERVER_SIDE, LOG_ADMIN, "O Reino blue foi campeão com %d pontos. Red: %d", sServer.RvR.Points[0], sServer.RvR.Points[1]);

				winner = CAPE_BLUE;
				GenerateMob(RVRSTORE_BLUE, 0, 0);
			}
			else if (sServer.RvR.Points[1] > sServer.RvR.Points[0])
			{
				SendNotice("O reino vermelho foi o vencedor do RvR com %d pontos vs %d!", sServer.RvR.Points[1], sServer.RvR.Points[0]);
				Log(SERVER_SIDE, LOG_ADMIN, "O Reino red foi campeão com %d pontos. Blue: %d", sServer.RvR.Points[1], sServer.RvR.Points[0]);

				winner = CAPE_RED;
				GenerateMob(RVRSTORE_RED, 0, 0);
			}
			else
			{
				SendNotice("A guerra do RvR terminou em empate!");
				Log(SERVER_SIDE, LOG_ADMIN, "Nenhum reino foi campeão. Blue: %d. Red: %d", sServer.RvR.Points[0], sServer.RvR.Points[1]);

				winner = 0;
			}

			sServer.RvR.Points[0] = 0;
			sServer.RvR.Points[1] = 0;
			sServer.RvR.Bonus = winner;

			sServer.RvR.Status = 0;
			sServer.RvR.Annoucement = 0;

			for (int i = 1; i < MAX_PLAYER; ++i)
			{
				int posX = 0;
				int posY = 0;
				if (Mob[i].Target.X >= 1041 && Mob[i].Target.X <= 1248 &&
					Mob[i].Target.Y >= 1950 && Mob[i].Target.Y <= 2158)
				{
					int cape = Mob[i].Mobs.Player.CapeInfo;
					if (cape != winner)
						DoRecall(i);
					else if (cape == CAPE_BLUE)
					{
						if (!(Rand() % 2))
						{
							posX = 1061 - Rand() % 5;
							posY = 2113 + Rand() % 5;
						}
						else
						{
							posX = 1091 - Rand() % 5;
							posY = 2140 + Rand() % 5;
						}
					}
					else
					{
						if (!(Rand() % 2))
						{
							posX = 1238 + Rand() % 5;
							posY = 1983 + Rand() % 5;
						}
						else
						{
							posX = 1211 + Rand() % 4;
							posY = 1955 + Rand() % 20;
						}
					}

					if (posX != 0 && posY != 0)
					{
						Teleportar(i, posX, posY);
						Log(i, LOG_INGAME, "Teleportado para a base da RvR");
					}
				}
			}

			pMsgSignal p;
			memset(&p, 0, sizeof p);

			p.Header.PacketId = 0x899;
			p.Header.ClientId = SERVER_SIDE;
			p.Header.Size = sizeof p;

			p.Value = winner;

			AddMessageDB((BYTE*)&p, sizeof p);

			if (winner != 0)
			{
				for (int i = 1; i < MAX_PLAYER; i++)
				{
					if (Users[i].Status != USER_PLAY)
						continue;

					if (Mob[i].Mobs.Player.CapeInfo == winner)
					{
						Mob[i].GetCurrentScore(i);
						SendScore(i);
					}
				}
			}
		}
		else if (now.tm_hour == sServer.RvR.Hour && now.tm_min == 35 && sServer.RvR.Status == 0)
		{
			for (int i = 1000; i < MAX_SPAWN_MOB; i++)
			{
				if (Mob[i].GenerateID == RVRSTORE_BLUE || Mob[i].GenerateID == RVRSTORE_RED)
					DeleteMob(i, 1);
			}
		}
	}
	/*
	else if(now.tm_hour == 22 && now.tm_min == 0 && sServer.RvR.Status == 1)
	{
		for(INT32 i = 1000; i < MAX_SPAWN_MOB; i++)
		{
			if(Mob[i].GenerateID == TORRE_RVR || Mob[i].GenerateID == (TORRE_RVR + 1))
				DeleteMob(i, 1);
		}

		if(sServer.RvR.Points[0] > sServer.RvR.Points[1])
		{
			SendNotice("O Reino Blue foi campeão da RvR!");

			Log(SERVER_SIDE, LOG_ADMIN, "O Reino blue foi campeão com %d pontos. Red: %d", sServer.RvR.Points[0], sServer.RvR.Points[1]);

			sServer.RvR.Points[0] = 0;
			sServer.RvR.Points[1] = 0;

			sServer.RvR.Bonus = 1;
		}
		else if(sServer.RvR.Points[1] > sServer.RvR.Points[0])
		{
			SendNotice("O Reino Red foi campeão da RvR!");

			Log(SERVER_SIDE, LOG_ADMIN, "O Reino red foi campeão com %d pontos. Blue: %d", sServer.RvR.Points[1], sServer.RvR.Points[0]);
			sServer.RvR.Points[0] = 0;
			sServer.RvR.Points[1] = 0;

			sServer.RvR.Bonus = 2;
		}
		else
		{
			SendNotice("Nenhum reino foi campeão devido ao empate!");

			Log(SERVER_SIDE, LOG_ADMIN, "Nenhum reino foi campeão. Blue: %d. Red: %d", sServer.RvR.Points[0], sServer.RvR.Points[1]);

			sServer.RvR.Points[0] = 0;
			sServer.RvR.Points[1] = 0;

			sServer.RvR.Bonus = 0;
		}

		sServer.RvR.Status = 0;
		ClearArea(1025, 1920, 1280, 2175);
	}
	*/

	if (!(counter % 120))
	{
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			int fadaId = Mob[i].Mobs.Player.Equip[13].Index;
			if ((fadaId < 3900 || fadaId > 3908) && fadaId != 3914 && fadaId != 3915 && fadaId != 3916 && fadaId != 3917)
				continue;

			if (!Mob[i].IsInsideValley() && (fadaId == 3917 || fadaId == 3916))
				continue;

			st_Item *item = &Mob[i].Mobs.Player.Equip[13];

			int days = item->EFV1;
			int hours = item->EFV2;
			int mins = item->EFV3;

			mins--;
			if (mins <= 0 && (hours > 0 || days > 0))
			{
				hours -= 1;
				mins = 59;
			}

			if (hours <= 0 && days > 0)
			{
				days -= 1;
				hours = 23;
			}

			if (days <= 0)
				days = 0;

			if (days == 0 && mins == 0 && hours == 0)
			{
				SendClientMessage(i, "!Sua fada expirou.");

				Log(i, LOG_INGAME, "Fada %s expirou ... [%d] [%d %d %d %d %d %d]", ItemList[item->Index].Name, item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3);
				LogPlayer(i, "Fada %s expirou...", ItemList[item->Index].Name);

				memset(item, 0, sizeof st_Item);
				SendItem(i, SlotType::Equip, 13, item);

				if (fadaId == 3916 || fadaId == 3917)
				{
					DoRecall(i);

					Log(i, LOG_INGAME, "Removido da área do Vale pela expiração da fada.");
				}

				continue;
			}

			item->EF1 = 106;
			item->EF2 = 107;
			item->EF3 = 108;

			item->EFV1 = days;
			item->EFV2 = hours;
			item->EFV3 = mins;

			SendItem(i, SlotType::Equip, 13, item);
		}
	}

	INT32 LOCAL_108 = counter % 16;
	INT32 LOCAL_109 = counter % 32;
	INT32 LOCAL_110 = 0;

	for (; LOCAL_110 < MAX_PLAYER; LOCAL_110++)
	{
		if ((LOCAL_110 & 0x8000001F) == LOCAL_109 && Users[LOCAL_110].Status != USER_EMPTY && Users[LOCAL_110].Status != USER_SAVING4QUIT)
			CheckIdle(LOCAL_110);

		if ((LOCAL_110 & 0x8000000F) == LOCAL_108 && Mob[LOCAL_110].Mobs.Player.Status.curHP && Users[LOCAL_110].Status == USER_PLAY)
		{
			RegenMob(LOCAL_110);

			ProcessAffect(LOCAL_110);
			LOCAL_106++;
		}
	}

	if (!(counter % 4))
	{
		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY || Users[i].AccessLevel != 0)
				continue;

			INT32 level = Mob[i].Mobs.Player.Status.Level,
				posX = Mob[i].Target.X,
				posY = Mob[i].Target.Y,
				evId = Mob[i].Mobs.Player.Equip[0].EFV2;

			if (posX >= 2380 && posX <= 2425 && posY >= 2078 && posY <= 2132)
			{
				if (level < g_pQuestLevel[0][0] || level > g_pQuestLevel[0][1] || evId >= CELESTIAL || Users[i].QuestAccess != 1)
					DoRecall(i);
			}
			else if (posX >= 2229 && posX <= 2256 && posY >= 1701 && posY <= 1728)
			{
				if (level < g_pQuestLevel[1][0] || level > g_pQuestLevel[1][1] || evId >= CELESTIAL || Users[i].QuestAccess != 2)
					DoRecall(i);
			}
			else if (posX >= 459 && posX <= 500 && posY >= 3886 && posY <= 3917)
			{
				if (level < g_pQuestLevel[2][0] || level > g_pQuestLevel[2][1] || evId >= CELESTIAL || Users[i].QuestAccess != 3)
					DoRecall(i);
			}
			else if (posX >= 655 && posX <= 701 && posY >= 3729 && posY <= 3764)
			{
				if (level < g_pQuestLevel[3][0] || level > g_pQuestLevel[3][1] || evId >= CELESTIAL || Users[i].QuestAccess != 4)
					DoRecall(i);
			}
			else if (posX >= 1308 && posX <= 1350 && posY >= 4023 && posY <= 4057)
			{
				if (level < g_pQuestLevel[4][0] || level > g_pQuestLevel[4][1] || evId >= CELESTIAL)
					DoRecall(i);
			}
			else if (posX >= 2232 && posX <= 2256 && posY >= 1564 && posY <= 1589)
			{
				if (level < 100 || level > 149 || evId > ARCH || Users[i].QuestAccess != 5)
					DoRecall(i);
			}
			else if (posX >= 2639 && posX <= 2670 && posY >= 1966 && posY <= 2004)
			{
				if (level < 69 || level > 74 || evId >= ARCH)
					DoRecall(i);
			}
			else if (posX >= 1949 && posX <= 1989 && posY >= 1586 && posY <= 1615)
			{
				if (level < 119 || level > 124 || evId >= ARCH)
					DoRecall(i);
			}
			else if (posX >= 793 && posX <= 828 && posY >= 4046 && posY <= 4082)
			{
				if (level < 200 || level > 254 || evId >= ARCH)
					DoRecall(i);
			}
			else if (posX >= 2180 && posX <= 2296 && posY >= 1160 && posY <= 1270)
			{
				auto& zakum = sServer.Zakum;
				MapAttribute map = GetAttribute(Mob[i].Target.X, Mob[i].Target.Y);
				if (map.CantSummon && std::find(std::begin(zakum.Users), std::end(zakum.Users), i) == std::end(zakum.Users))
				{
					Log(i, LOG_HACK, "O usuário estava na quest do Zakum sem possuir o registro. Posição: %u %u", Mob[i].Target.X, Mob[i].Target.Y);
					Log(SERVER_SIDE, LOG_HACK, "[%s] O usuário estava na quest do Zakum sem possuir o registro. Posição: %u %u", Users[i].User.Username, Mob[i].Target.X, Mob[i].Target.Y);

					DoRecall(i);
				}
			}
		}
	}

	if (!(counter % 1200))
	{ // Passou um segundo e checa se passou 15 minutos
		ClearArea(2380, 2078, 2425, 2133, "Coveiro"); // Coveiro
		ClearArea(2229, 1701, 2256, 1728, "Jardim"); // Jardim
		ClearArea(459, 3886, 500, 3917, "Kaizen"); // Kaizen
		ClearArea(657, 3728, 704, 3764, "Hidra"); // Hidra 
		ClearArea(1308, 4023, 1350, 4057, "Emblema da Proteção"); // Emblema da Proteção 
		ClearArea(793, 4046, 828, 4080, "Molar de Gárgula"); // Molar 201-255
		ClearArea(2552, 1934, 2545, 1957); // Iniciante
		ClearArea(1718, 1714, 1745, 1738, "Quest da Capa"); // Quest do Reino
		ClearArea(2232, 1564, 2256, 1589, "Capa Verde"); // capa verde
		ClearArea(2639, 1966, 2670, 2004, "Benção de Deus"); // benção de Deus	   70 - 75
		ClearArea(1949, 1586, 1989, 1615, "Equílibrio da Força"); // equilíbrio da força  120 - 125
		ClearArea(301, 407, 330, 487);

		for (auto& user : Users)
		{
			if (user.Status != USER_PLAY)
				continue;

			CMob& mob = Mob[user.clientId];
			if (!mob.Mode)
				continue;

			if (mob.Target.X >= 3600 && mob.Target.X <= 3700 && mob.Target.Y >= 3600 && mob.Target.Y <= 3700)
			{
				Log(user.clientId, LOG_INGAME, "Enviado para a cidade através do ClearArea. Posição: %dx %dy - Quest LanHouse (N)", mob.Target.X, mob.Target.Y);

				user.Times.LastLanHouseN = nowChrono;
				DoRecall(user.clientId);
			}
			else if (mob.Target.X >= 3713 && mob.Target.X <= 3838 && mob.Target.Y >= 3459 && mob.Target.Y <= 3582)
			{
				Log(user.clientId, LOG_INGAME, "Enviado para a cidade através do ClearArea. Posição: %dx %dy - Quest LanHouse (M)", mob.Target.X, mob.Target.Y);
				user.Times.LastLanHouseM = nowChrono;

				DoRecall(user.clientId);
			}
			else if (mob.Target.X >= 3840 && mob.Target.X <= 3967 && mob.Target.Y >= 3584 && mob.Target.Y <= 3712)
			{
				Log(user.clientId, LOG_INGAME, "Enviado para a cidade através do ClearArea. Posição: %dx %dy - Quest LanHouse (A)", mob.Target.X, mob.Target.Y);
				user.Times.LastLanHouseA = nowChrono;

				DoRecall(user.clientId);
			}
		}

		for (INT32 i = 1; i < MAX_PLAYER; i++)
			Users[i].QuestAccess = 0;
	}

	if (!(counter % 3600))
		ClearArea(2177, 3585, 2304, 3839, "Área do Vale"); // Coveiro

	if (!(counter % 10))
	{
		if (sServer.Zakum.IsOperating && std::chrono::steady_clock::now() - sServer.Zakum.StartTime >= 15min)
		{
			ClearArea(0x884, 0x488, 0x8F8, 0x4D4, "Zakum");

			Log(SERVER_SIDE, LOG_INGAME, "Quest zakum foi resetada pelo tempo máximo");
			sServer.Zakum.IsOperating = false;
		}
		else if (sServer.Zakum.IsOperating && sServer.Zakum.Users.size() == 0)
		{
			ClearArea(0x884, 0x488, 0x8F8, 0x4D4, "Zakum");

			Log(SERVER_SIDE, LOG_INGAME, "Quest zakum foi resetada por estar vazia");
			sServer.Zakum.IsOperating = false;
		}

	}
	INT32 LOCAL_113 = counter % 6;
	LOCAL_110 = LOCAL_113 + MAX_PLAYER;
	for (LOCAL_110; LOCAL_110 < MAX_SPAWN_MOB; LOCAL_110 += 6)
	{
		if (LOCAL_110 >= MAX_SPAWN_MOB)
			break;

		INT32 LOCAL_114 = Mob[LOCAL_110].Mode;
		INT32 LOCAL_115 = Mob[LOCAL_110].Mobs.Player.CapeInfo;

		if (LOCAL_114 == 0)
			continue;

		if (LOCAL_114 == 3)
		{
			if (!Mob[LOCAL_110].Mobs.Player.Status.curHP)
			{
				Log(SERVER_SIDE, LOG_ERROR, "StandingByProcessor deletado mob com zero de HP.");
				DeleteMob(LOCAL_110, 1);
				continue;
			}

			ProcessAffect(LOCAL_110);
			g_pMobGrid[Mob[LOCAL_110].Target.Y][Mob[LOCAL_110].Target.X] = LOCAL_110;
		}

		if (LOCAL_114 == 4)
		{
			if (!Mob[LOCAL_110].Mobs.Player.Status.curHP)
			{
				Log(SERVER_SIDE, LOG_ERROR, "standingby processer delete hp zero mob %s %d %dx %dy", Mob[LOCAL_110].Mobs.Player.Name, Mob[LOCAL_110].GenerateID, Mob[LOCAL_110].Target.X, Mob[LOCAL_110].Target.Y);

				DeleteMob(LOCAL_110, 1);
				continue;
			}

			ProcessAffect(LOCAL_110);

			INT32 LOCAL_116 = Mob[LOCAL_110].Segment.Progress;
			if (LOCAL_116 < 0 || LOCAL_116 > 5)
			{
				Mob[LOCAL_110].Segment.Progress = 0;
				LOCAL_116 = 0;
			}

			INT32 LOCAL_117 = Mob[LOCAL_110].StandingByProcessor(); // ECX

			if (LOCAL_117 & 0x10000000)
			{
				INT32 LOCAL_118 = LOCAL_117 & 0x0FFFFFFF;
				SetBattle(LOCAL_110, LOCAL_118);

				UINT32 LOCAL_119 = Mob[LOCAL_110].Leader;

				if (LOCAL_119 <= 0)
					LOCAL_119 = LOCAL_110;

				for (INT32 LOCAL_120 = 0; LOCAL_120 < 12; LOCAL_120++)
				{
					INT32 LOCAL_121 = Mob[LOCAL_119].PartyList[LOCAL_120];
					if (LOCAL_121 < MAX_PLAYER) // MAX_PLAYER
						continue;

					if (Mob[LOCAL_121].Mode == 0 || !Mob[LOCAL_121].Mobs.Player.Status.curHP)
					{
						if (Mob[LOCAL_121].Mode != 0)
							DeleteMob(LOCAL_121, 1);

						Mob[LOCAL_119].PartyList[LOCAL_120] = 0;
					}
					else
						SetBattle(LOCAL_121, LOCAL_118);
				}

				if (LOCAL_118 < MAX_PLAYER) // MAX_PLAYER
				{
					INT32 LOCAL_122 = Mob[LOCAL_118].Leader;
					if (LOCAL_122 <= 0)
						LOCAL_122 = LOCAL_118;

					for (INT32 LOCAL_123 = 0; LOCAL_123 < 12; LOCAL_123++)
					{
						INT32 LOCAL_124 = Mob[LOCAL_122].PartyList[LOCAL_123];
						if (LOCAL_124 < MAX_PLAYER)
							continue;

						if (Mob[LOCAL_124].Mode == 0 || !Mob[LOCAL_124].Mobs.Player.Status.curHP)
						{
							if (Mob[LOCAL_124].Mode != 0)
								DeleteMob(LOCAL_124, 1);

							Mob[LOCAL_122].PartyList[LOCAL_123] = 0;
						}
						else
							SetBattle(LOCAL_124, LOCAL_110);
					}

				}
				continue;
			}

			if (LOCAL_117 & 1)
			{
				p36C LOCAL_137;
				GetAction(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, &LOCAL_137);

				GridMulticast(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, (BYTE*)&LOCAL_137);

				INT32 LOCAL_138 = Mob[LOCAL_110].Segment.Progress;
				INT32 LOCAL_139 = Mob[LOCAL_110].GenerateID;

				if (LOCAL_116 != LOCAL_138 && LOCAL_138 >= 0 && LOCAL_138 < 5 && LOCAL_139 >= 0 && LOCAL_139 < 8192 && mGener.pList[LOCAL_139].SegmentAction[LOCAL_138][0])
					SendSay(LOCAL_110, mGener.pList[LOCAL_139].SegmentAction[LOCAL_138]);
			}

			if (LOCAL_117 & 16)
			{
				INT32 LOCAL_140 = Mob[LOCAL_110].Segment.Progress;
				INT32 LOCAL_141 = Mob[LOCAL_110].GenerateID;

				if (LOCAL_140 < 0 || LOCAL_140 >= 5)
					continue;

				if (LOCAL_140 >= 0 && LOCAL_140 < 5 && LOCAL_141 >= 0 && mGener.pList[LOCAL_141].SegmentAction[LOCAL_140][0])
					SendSay(LOCAL_110, mGener.pList[LOCAL_141].SegmentAction[LOCAL_140]);
			}

			if (LOCAL_117 & 256)
				DeleteMob(LOCAL_110, 3);

			if (LOCAL_117 & 4096)
			{   // 4541A6
				Mob[LOCAL_110].GetRandomPos();

				if (Mob[LOCAL_110].Next.X == Mob[LOCAL_110].Target.X && Mob[LOCAL_110].Next.Y == Mob[LOCAL_110].Target.Y)
					continue;

				p36C LOCAL_154;
				GetAction(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, &LOCAL_154);

				GridMulticast(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, (BYTE*)&LOCAL_154);
			}

			if (LOCAL_117 & 2)
				Teleportar(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y);
		}
	}

	LOCAL_113 = sServer.SecCounter % 4;
	INT32 LOCAL_155 = sServer.SecCounter % 8;

	if (LOCAL_113 == LOCAL_155)
		LOCAL_155 = 1;
	else
		LOCAL_155 = 0;

	for (LOCAL_110 = LOCAL_113 + MAX_PLAYER; LOCAL_110 < MAX_SPAWN_MOB; LOCAL_110 += 4)
	{
		if (LOCAL_110 >= MAX_SPAWN_MOB)
			break;

		if (Mob[LOCAL_110].Mode != 5)
			continue;

		if (Mob[LOCAL_110].Mobs.Player.CapeInfo == 4)
			INT32 LOCAL_156 = 0;

		if (!Mob[LOCAL_110].Mobs.Player.Status.curHP)
		{
			// TODO : error

			DeleteMob(LOCAL_110, 1);
			continue;
		}

		if (Mob[LOCAL_110].Mobs.Player.CapeInfo == 4)
			LOCAL_110 = LOCAL_110;

		if (LOCAL_155 != 0)
			ProcessAffect(LOCAL_110);

		INT32 LOCAL_157 = Mob[LOCAL_110].CurrentTarget;

		// 00454486
		if (LOCAL_157 > 0 && LOCAL_157 < 30000)
		{
			INT32 LOCAL_158 = Mob[LOCAL_110].Leader;
			if (LOCAL_158 == 0)
				LOCAL_158 = LOCAL_110;

			INT32 LOCAL_159 = Mob[LOCAL_157].Leader;
			if (LOCAL_159 == 0)
				LOCAL_159 = LOCAL_157;

			INT32 LOCAL_160 = Mob[LOCAL_110].Mobs.Player.GuildIndex;
			if (Mob[LOCAL_110].GuildDisable)
				LOCAL_160 = 0;

			INT32 LOCAL_161 = Mob[LOCAL_157].Mobs.Player.GuildIndex;
			if (Mob[LOCAL_157].GuildDisable)
				LOCAL_161 = 0;

			if (LOCAL_160 == 0 && LOCAL_161 == 0)
				LOCAL_160 = -1;

			// 00454599
			if (LOCAL_158 == LOCAL_159 || LOCAL_160 == LOCAL_161)
				Mob[LOCAL_110].RemoveEnemyList(LOCAL_157);
		}

		// 004545C9
		INT32 LOCAL_162 = Mob[LOCAL_110].BattleProcessor();
		INT32 LOCAL_163 = Rand() % 100;

		if (LOCAL_162 & 32)
		{ // 0045460B
			DeleteMob(LOCAL_110, 3);
			continue;
		}

		else if ((LOCAL_162 & 65536) && !(Rand() & 0x80000001))
			SendEmotion(LOCAL_110, 14, 1);

		if (LOCAL_162 == 0)
		{
			if (Mob[LOCAL_110].GenerateID == GUARDIAN_TOWER_BLUE ||
				Mob[LOCAL_110].GenerateID == GUARDIAN_TOWER_RED ||
				Mob[LOCAL_110].GenerateID == 8 ||
				Mob[LOCAL_110].GenerateID == 9)
			{
				bool isAlive = false;
				int mobIndex;
				int index;
				int searchedIndex;
				if (Mob[LOCAL_110].GenerateID == 8 || Mob[LOCAL_110].GenerateID == 9)
				{
					index = Mob[LOCAL_110].GenerateID - 8;
					searchedIndex = GUARDIAN_TOWER_BLUE + index;
					isAlive = sServer.KingdomBattle.Info[index].isTowerAlive;
					mobIndex = sServer.KingdomBattle.Info[index].TowerId;
				}
				else
				{
					index = Mob[LOCAL_110].GenerateID - GUARDIAN_TOWER_BLUE;
					searchedIndex = 8 + index;
					isAlive = sServer.KingdomBattle.Info[index].isKingAlive;
					mobIndex = sServer.KingdomBattle.Info[index].KingId;
				}

				// O mob em questão deve estar vivo e com o modo de batalha
				bool status = isAlive && mobIndex > 0 && mobIndex < MAX_SPAWN_MOB && Mob[mobIndex].GenerateID == searchedIndex && Mob[mobIndex].Mode == 5;
				SendKingdomBattleInfo(SERVER_SIDE, CAPE_BLUE + index, status);

				sServer.KingdomBattle.Info[index].Status = status;

				if (!status)
					SendNotice("A paz situou no Reino %s", index == 0 ? "Blue" : "Red");
			}
		}

		if (LOCAL_162 & 0x2000)
		{
			INT32 vision = Mob[LOCAL_110].Mobs.Player.Status.DEX;
			if (vision > 20)
				vision = 20;

			p367 packet{};
			packet.Header.Size = sizeof p367;

			int target[13];
			for (INT32 i = 0; i < 13; i++)
				target[i] = 0;

			INT32 posX = Mob[LOCAL_110].Target.X,
				posY = Mob[LOCAL_110].Target.Y,
				maxX = posX + vision,
				maxY = posY + vision,
				minX = posX - vision,
				minY = posY - vision;

			INT32 count = 0;
			for (INT32 tmpY = minY; tmpY < maxY; tmpY++)
			{
				if (count >= 13)
					break;

				for (INT32 tmpX = minX; tmpX < maxX; tmpX++)
				{
					if (count >= 13)
						break;

					INT32 mobId = g_pMobGrid[tmpY][tmpX];
					if (mobId <= 0 || mobId >= MAX_PLAYER)
						continue;

					INT32 distance = GetDistance(tmpX, tmpY, Mob[mobId].Target.X, Mob[mobId].Target.Y);
					if (!Mob[mobId].Mobs.Player.Status.curHP)
						continue;

					if ((Rand() % 5) >= 3)
						continue;

					target[count] = mobId;
					count++;
				}
			}

			GetMultiAttack(LOCAL_110, target, &packet);

			INT32 damagePet = 0;

			for (INT32 i = 0; i < 13; i++)
			{
				INT32 mobId = packet.Target[i].Index;
				if (mobId <= 0 || mobId >= MAX_SPAWN_MOB)
					continue;

				INT32 damage = packet.Target[i].Damage,
					petId = Mob[mobId].Mobs.Player.Equip[14].Index;

				if (mobId < MAX_PLAYER && damage > 0)
				{
					UINT32 LOCAL_180 = GetParryRate(mobId, LOCAL_113, -2);

					UINT32 LOCAL_181 = Rand() % 1000;
					if (LOCAL_181 < LOCAL_180 && Mob[LOCAL_110].GenerateID != KEFRA)
					{
						packet.Target[i].Damage = -3;
						if (Mob[mobId].Mobs.Player.AffectInfo.Resist && LOCAL_181 < MAX_PLAYER)
							packet.Target[i].Damage = -4;
					}

					if (Mob[LOCAL_110].Mobs.Player.CapeInfo == 4)
						packet.Target[i].Damage = (packet.Target[i].Damage << 1) / 5;

					INT32 auxDam = packet.Target[i].Damage;
					for (int t = 0; t < 32; t++)
					{
						if (Mob[mobId].Mobs.Affects[t].Index == 18)
						{
							INT32 tmpDamage = auxDam * 80 / 100;
							if ((Users[mobId].Potion.CountMp - tmpDamage) >= 300)
							{
								Mob[mobId].Mobs.Player.Status.curMP -= tmpDamage;
								Users[mobId].Potion.CountMp -= tmpDamage;

								SetReqMp(mobId);
								SendScore(mobId);

								auxDam = tmpDamage;
							}
							else
								SendSetHpMp(mobId);

							break;
						}
					}

					damage = auxDam;
				}

				packet.Target[i].Damage = damage;

				UINT32 LOCAL_182 = Mob[mobId].Leader;
				if (packet.Target[i].Damage > 0)
				{
					if (LOCAL_182 <= 0)
						LOCAL_182 = mobId;

					SetBattle(LOCAL_182, LOCAL_110);

					if (Mob[LOCAL_110].Mobs.Player.CapeInfo != 4)
						SetBattle(LOCAL_110, LOCAL_182);

					for (INT32 LOCAL_183 = 0; LOCAL_183 < 12; LOCAL_183++)
					{
						INT32 LOCAL_184 = Mob[LOCAL_182].PartyList[LOCAL_183];
						if (LOCAL_184 < MAX_PLAYER)
							continue;

						if (Mob[LOCAL_184].Mode == 0 || Mob[LOCAL_184].Mobs.Player.Status.curHP <= 0)
						{
							if (Mob[LOCAL_184].Mode == 0)
								DeleteMob(LOCAL_184, 1);

							Mob[LOCAL_182].PartyList[LOCAL_183] = 0;
							continue;
						}

						SetBattle(LOCAL_184, LOCAL_110);
						SetBattle(LOCAL_110, LOCAL_184);
					}

					// 00454A48
					INT32 LOCAL_185 = Mob[LOCAL_110].Summoner;
					/*
					if(Mob[LOCAL_110].Mobs.Player.CapeInfo == 4 && mobId >= MAX_PLAYER && LOCAL_185 > 0 && LOCAL_185 < MAX_PLAYER && Users[LOCAL_185].Status == USER_PLAY)
					{
						INT32 LOCAL_186 = Mob[LOCAL_185].Target.X;
						INT32 LOCAL_187 = Mob[LOCAL_185].Target.Y;

						INT32 LOCAL_188 = 46;

						if(Mob[LOCAL_110].Target.X > (LOCAL_186 - LOCAL_188) && Mob[LOCAL_110].Target.X < (LOCAL_186 + LOCAL_188) && Mob[LOCAL_110].Target.Y > (LOCAL_187 - LOCAL_188)
							&& Mob[LOCAL_110].Target.Y < (LOCAL_187 + LOCAL_188))
						{	// 00454BCE
							INT32 LOCAL_3AE;
							if(Mob[mobId].Mobs.Player.Status.curHP < packet.Target[i].Damage)
								LOCAL_3AE = Mob[mobId].Mobs.Player.Status.curHP;
							else
								LOCAL_3AE = packet.Target[i].Damage;

							INT32 LOCAL_189 = LOCAL_3AE;
							INT32 LOCAL_190 = Mob[mobId].Mobs.Player.Exp * LOCAL_189 / Mob[mobId].Mobs.Player.Status.maxHP;

							if(Mob[LOCAL_185].Mobs.Player.Status.Level != Mob[LOCAL_110].Mobs.Player.Status.Level)
							{
								LOCAL_190 = GetExpApply(LOCAL_190, Mob[LOCAL_110].Mobs.Player.Status.Level, Mob[mobId].Mobs.Player.Status.Level);

								if(Mob[mobId].Mobs.Player.CapeInfo == 4)
									LOCAL_190 = 0;

								Mob[LOCAL_185].Mobs.Player.Exp = Mob[LOCAL_185].Mobs.Player.Exp + LOCAL_190;
							}
						}
					}
				*/
				}

				if (packet.Target[i].Damage > 0 || packet.Target[i].Damage <= -5)
				{
					INT32 LOCAL_191 = packet.Target[i].Damage;
					INT32 LOCAL_192 = 0;

					INT32 LOCAL_193 = Mob[mobId].Mobs.Player.Equip[14].Index;

					if (mobId < MAX_PLAYER)
					{
						if (Mob[mobId].isPetAlive())
						{
							if (Mob[mobId].isNormalPet())
							{
								LOCAL_191 = AbsorveDamageByPet(&Mob[mobId], packet.Target[i].Damage);
								LOCAL_192 = packet.Target[i].Damage - LOCAL_191;
							}

							packet.Target[i].Damage = LOCAL_191;
						}
					}

					INT32 itemId = Mob[mobId].Mobs.Player.Equip[13].Index;
					if (itemId == 786 || itemId == 1936 || itemId == 1937)
					{
						INT32 LOCAL_194 = GetItemSanc(&Mob[mobId].Mobs.Player.Equip[13]);

						if (LOCAL_194 < 2)
							LOCAL_194 = 2;

						INT32 multHP = 1;
						switch (itemId)
						{
						case 1936:
							multHP = 10;
							break;

						case 1937:
							multHP = 1000;
							break;
						}

						multHP *= LOCAL_194;
						Mob[mobId].Mobs.Player.Status.curHP = Mob[mobId].Mobs.Player.Status.curHP - (packet.Target[i].Damage / multHP);
					}
					else
					{
						if (packet.Target[i].Damage > Mob[mobId].Mobs.Player.Status.curHP)
							Mob[mobId].Mobs.Player.Status.curHP = 0;
						else
							Mob[mobId].Mobs.Player.Status.curHP = Mob[mobId].Mobs.Player.Status.curHP - packet.Target[i].Damage;
					}

					if (mobId >= MAX_PLAYER && Mob[mobId].Mobs.Player.CapeInfo == 4)
						LinkMountHp(mobId);

					if (LOCAL_192 > 0)
						ProcessAdultMount(mobId, LOCAL_192);

					if (mobId > 0 && mobId < MAX_PLAYER)
					{
						Users[mobId].Potion.CountHp = Users[mobId].Potion.CountHp - packet.Target[i].Damage;
						SetReqHp(mobId);
					}
				}
			}

			GridMulticast_2(Mob[LOCAL_110].Target.X, Mob[LOCAL_110].Target.Y, (BYTE*)&packet, 0);

			for (INT32 i = 0; i < 13; i++)
			{
				INT32 mobId = packet.Target[i].Index;
				if (mobId <= 0 || mobId >= MAX_SPAWN_MOB)
					continue;

				if (Mob[mobId].Mobs.Player.Status.curHP <= 0)
					MobKilled(mobId, LOCAL_110, 0, 0);
			}
		}

		if (LOCAL_162 & 4096)
		{
			UINT32 LOCAL_164 = Mob[LOCAL_110].CurrentTarget;
			if (LOCAL_164 <= 0 || LOCAL_164 >= MAX_SPAWN_MOB)
				continue;

			p39D LOCAL_175;
			GetAttack(LOCAL_110, LOCAL_164, &LOCAL_175);

			//LOCAL_175.flagLocal = 0;
			LOCAL_164 = LOCAL_175.Target.Index & 0xFFFF;

			INT32 LOCAL_176 = LOCAL_175.skillId;
			if (LOCAL_176 >= 0 && LOCAL_176 < 96 && !(LOCAL_175.skillParm & 0xFF))
			{
				if (LOCAL_176 == 33)
					LOCAL_175.Motion = 0xFD;

				INT32 LOCAL_177 = (((LOCAL_176 % 24) >> 3) + 1);
				if (LOCAL_177 >= 1 && LOCAL_177 <= 3)
				{
					INT32 LOCAL_178 = Mob[LOCAL_110].Mobs.Player.Status.Mastery[LOCAL_177];

					INT32 LOCAL_179 = 0;
					if (SetAffect(LOCAL_164, LOCAL_176, 100, LOCAL_178))
						LOCAL_179 = 1;

					if (SetTick(LOCAL_164, LOCAL_176, 100, LOCAL_178))
						LOCAL_179 = 1;

					if (LOCAL_179 != 0)
						SendScore(LOCAL_164);

					if (LOCAL_164 < MAX_PLAYER)
						SendAffect(LOCAL_164);
					/*
					if(LOCAL_179 != 0)
					{
						p364 packet;
						GetCreateMob(LOCAL_164, (BYTE*)&packet);

						GridMulticast_2(Mob[LOCAL_164].Target.X, Mob[LOCAL_164].Target.Y, (BYTE*)&packet, 0);
					}*/
				}
			}

			if (LOCAL_164 < MAX_PLAYER && LOCAL_175.Target.Damage > 0) // MAX_PLAYER
			{
				UINT32 LOCAL_180 = GetParryRate(LOCAL_110, LOCAL_164, -2);

				UINT32 LOCAL_181 = Rand() % 1000;
				if (LOCAL_181 < LOCAL_180)
				{
					LOCAL_175.Target.Damage = -3;

					if (Mob[LOCAL_164].Mobs.Player.AffectInfo.Resist && LOCAL_164 < MAX_PLAYER)
						LOCAL_175.Target.Damage = -4;
				}

				if (Mob[LOCAL_110].Mobs.Player.CapeInfo == 4)
					LOCAL_175.Target.Damage = (LOCAL_175.Target.Damage << 1) / 5;

				INT32 auxDam = LOCAL_175.Target.Damage;
				if (auxDam > 0)
				{
					for (int i = 0; i < 32; i++)
					{
						if (Mob[LOCAL_164].Mobs.Affects[i].Index == 18)
						{
							INT32 tmpDamage = auxDam * 80 / 100;
							if ((Mob[LOCAL_164].Mobs.Player.Status.curMP - tmpDamage) >= 300)
							{
								if (Users[LOCAL_164].Potion.CountMp - tmpDamage >= 0)
									Users[LOCAL_164].Potion.CountMp -= tmpDamage;
								else
									Users[LOCAL_164].Potion.CountMp = 0;

								if (Mob[LOCAL_164].Mobs.Player.Status.curMP - tmpDamage >= 0)
									Mob[LOCAL_164].Mobs.Player.Status.curMP -= tmpDamage;
								else
									Mob[LOCAL_164].Mobs.Player.Status.curMP = 0;

								auxDam -= tmpDamage;
							}

							SendSetHpMp(LOCAL_164);

							break;
						}
					}
				}

				LOCAL_175.Target.Damage = auxDam;
			}

			INT32 summonerId = Mob[LOCAL_110].Summoner;
			if (Mob[LOCAL_110].Mobs.Player.CapeInfo == 4 && summonerId > 0 && summonerId < MAX_PLAYER && Users[summonerId].Status == USER_PLAY)
			{
				if (Mob[LOCAL_164].GenerateID == TORRE_ERION)
				{
					INT32 idGuild = Mob[summonerId].Mobs.Player.GuildIndex;
					if (idGuild == 0 || (idGuild != 0 && idGuild == Mob[LOCAL_164].Mobs.Player.GuildIndex))
						continue;

					INT32 ally = g_pGuildAlly[idGuild];
					if (ally != 0 && ally == Mob[LOCAL_164].Mobs.Player.GuildIndex)
						continue;
				}

				if (LOCAL_164 < MAX_PLAYER)
				{
					if (!Users[summonerId].AllStatus.PK || !GetAttribute(Mob[LOCAL_110].Target.X, Mob[LOCAL_110].Target.Y).PvP || !GetAttribute(Mob[LOCAL_164].Target.X, Mob[LOCAL_164].Target.Y).PvP)
					{
						Mob[LOCAL_110].RemoveEnemyList(LOCAL_164);

						continue;
					}

					if (Users[summonerId].AllStatus.PK && GetArena(Mob[LOCAL_110].Target.X, Mob[LOCAL_110].Target.Y) == 5)
					{
						int oldGuilty = GetGuilty(summonerId);
						SetGuilty(summonerId, 8);

						if (oldGuilty == 0)
						{
							p364 createMob{};
							GetCreateMob(summonerId, (BYTE*)&createMob);

							GridMulticast_2(Mob[summonerId].Target.X, Mob[summonerId].Target.Y, reinterpret_cast<BYTE*>(&createMob), 0);
						}
					}
				}
			}

			if (Mob[LOCAL_110].GenerateID == TORRE_ERION)
			{
				INT32 attacked = LOCAL_164;
				if (attacked >= MAX_PLAYER || Mob[attacked].Mobs.Player.CapeInfo == 4)
					attacked = Mob[attacked].Summoner;

				if (attacked > 0 && attacked < MAX_PLAYER)
				{
					INT32 idGuild = Mob[attacked].Mobs.Player.GuildIndex;
					if (idGuild != 0 && idGuild == Mob[LOCAL_164].Mobs.Player.GuildIndex)
						continue;

					INT32 ally = g_pGuildAlly[idGuild];
					if (ally != 0 && ally == Mob[LOCAL_164].Mobs.Player.GuildIndex)
						continue;
				}
			}

			UINT32 LOCAL_182 = Mob[LOCAL_164].Leader;
			if (LOCAL_175.Target.Damage > 0)
			{
				if (LOCAL_182 <= 0)
					LOCAL_182 = LOCAL_164;

				SetBattle(LOCAL_182, LOCAL_110);

				if (Mob[LOCAL_110].Mobs.Player.CapeInfo != 4)
					SetBattle(LOCAL_110, LOCAL_182);

				// 00454951

				for (INT32 LOCAL_183 = 0; LOCAL_183 < 12; LOCAL_183++)
				{
					INT32 LOCAL_184 = Mob[LOCAL_182].PartyList[LOCAL_183];
					if (LOCAL_184 < MAX_PLAYER)
						continue;

					if (Mob[LOCAL_184].Mode == 0 || Mob[LOCAL_184].Mobs.Player.Status.curHP <= 0)
					{
						if (Mob[LOCAL_184].Mode == 0)
							DeleteMob(LOCAL_184, 1);

						Mob[LOCAL_182].PartyList[LOCAL_183] = 0;
						continue;
					}

					SetBattle(LOCAL_184, LOCAL_110);
					SetBattle(LOCAL_110, LOCAL_184);
				}

				// 00454A48
				INT32 LOCAL_185 = Mob[LOCAL_110].Summoner;
				/*
				if(Mob[LOCAL_110].Mobs.Player.CapeInfo == 4 && LOCAL_164 >= MAX_PLAYER && LOCAL_185 > 0 && LOCAL_185 < MAX_PLAYER && Users[LOCAL_185].Status == USER_PLAY)
				{
					INT32 LOCAL_186 = Mob[LOCAL_185].Target.X;
					INT32 LOCAL_187 = Mob[LOCAL_185].Target.Y;

					INT32 LOCAL_188 = 46;

					if(Mob[LOCAL_110].Target.X > (LOCAL_186 - LOCAL_188) && Mob[LOCAL_110].Target.X < (LOCAL_186 + LOCAL_188) && Mob[LOCAL_110].Target.Y > (LOCAL_187 - LOCAL_188) && Mob[LOCAL_110].Target.Y < (LOCAL_187 + LOCAL_188))
					{	// 00454BCE
						INT32 LOCAL_3AE;
						if(Mob[LOCAL_164].Mobs.Player.Status.curHP < LOCAL_175.Target.Damage)
							LOCAL_3AE = Mob[LOCAL_164].Mobs.Player.Status.curHP;
						else
							LOCAL_3AE = LOCAL_175.Target.Damage;

						INT32 LOCAL_189 = LOCAL_3AE;
						INT32 LOCAL_190 = Mob[LOCAL_164].Mobs.Player.Exp * LOCAL_189 / Mob[LOCAL_164].Mobs.Player.Status.maxHP;

						if(Mob[LOCAL_185].Mobs.Player.Status.Level != Mob[LOCAL_110].Mobs.Player.Status.Level)
						{
							LOCAL_190 = GetExpApply(LOCAL_190, Mob[LOCAL_110].Mobs.Player.Status.Level, Mob[LOCAL_164].Mobs.Player.Status.Level);

							if(Mob[LOCAL_164].Mobs.Player.CapeInfo == 4)
								LOCAL_190 = 0;

							Mob[LOCAL_185].Mobs.Player.Exp = Mob[LOCAL_185].Mobs.Player.Exp + LOCAL_190;
						}
					}
				}*/
			}

			if (LOCAL_175.Target.Damage > 0 || LOCAL_175.Target.Damage <= -5)
			{
				INT32 LOCAL_191 = LOCAL_175.Target.Damage;
				INT32 LOCAL_192 = 0;

				INT32 LOCAL_193 = Mob[LOCAL_164].Mobs.Player.Equip[14].Index;

				if (LOCAL_164 < MAX_PLAYER && Mob[LOCAL_164].isPetAlive())
				{
					if (Mob[LOCAL_164].isNormalPet())
					{
						LOCAL_191 = AbsorveDamageByPet(&Mob[LOCAL_164], LOCAL_175.Target.Damage);
						LOCAL_192 = LOCAL_175.Target.Damage - LOCAL_191;
					}

					LOCAL_175.Target.Damage = LOCAL_191;
				}

				INT32 itemId = Mob[LOCAL_164].Mobs.Player.Equip[13].Index;
				if (itemId == 786 || itemId == 1936 || itemId == 1937)
				{
					INT32 LOCAL_194 = GetItemSanc(&Mob[LOCAL_164].Mobs.Player.Equip[13]);

					if (LOCAL_194 < 2)
						LOCAL_194 = 2;

					INT32 multHP = 1;
					switch (itemId)
					{
					case 1936:
						multHP = 10;
						break;

					case 1937:
						multHP = 1000;
						break;
					}

					multHP *= LOCAL_194;
					Mob[LOCAL_164].Mobs.Player.Status.curHP = Mob[LOCAL_164].Mobs.Player.Status.curHP - (LOCAL_175.Target.Damage / multHP);
				}
				else
				{
					if (LOCAL_175.Target.Damage > Mob[LOCAL_164].Mobs.Player.Status.curHP)
						LOCAL_175.Target.Damage = Mob[LOCAL_164].Mobs.Player.Status.curHP;

					Mob[LOCAL_164].Mobs.Player.Status.curHP = Mob[LOCAL_164].Mobs.Player.Status.curHP - LOCAL_175.Target.Damage;
				}

				if (LOCAL_164 >= MAX_PLAYER && Mob[LOCAL_164].Mobs.Player.CapeInfo == 4)
					LinkMountHp(LOCAL_164);

				if (LOCAL_192 > 0)
					ProcessAdultMount(LOCAL_164, LOCAL_192);
			}

			GridMulticast_2(Mob[LOCAL_164].Target.X, Mob[LOCAL_164].Target.Y, (BYTE*)&LOCAL_175, 0);

			// 00454ED2
			if (LOCAL_164 > 0 && LOCAL_164 < MAX_PLAYER)
			{
				Users[LOCAL_164].Potion.CountHp = Users[LOCAL_164].Potion.CountHp - LOCAL_175.Target.Damage;
				SetReqHp(LOCAL_164);
			}

			if (Mob[LOCAL_164].Mobs.Player.Status.curHP <= 0)
				MobKilled(LOCAL_164, LOCAL_110, 0, 0);
		}

		if (LOCAL_162 & 256)
		{
			INT32 LOCAL_195 = Mob[LOCAL_110].CurrentTarget;

			Mob[LOCAL_110].GetTargetPosDistance(LOCAL_195); // ECX

			if (Mob[LOCAL_110].Next.X == Mob[LOCAL_110].Target.X && Mob[LOCAL_110].Next.Y == Mob[LOCAL_110].Target.Y)
				continue;

			p36C LOCAL_208;
			GetAction(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, &LOCAL_208);

			LOCAL_208.Command[0] = 0;
			LOCAL_208.MoveType = 0;

			GridMulticast(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, (BYTE*)&LOCAL_208);
		}

		if (LOCAL_162 & 1)
		{
			INT32 LOCAL_209 = Mob[LOCAL_110].CurrentTarget;
			Mob[LOCAL_110].GetCurrentScore(MAX_PLAYER);

			Mob[LOCAL_110].GetTargetPos(LOCAL_209); // ECX

			if (Mob[LOCAL_110].Next.X == Mob[LOCAL_110].Target.X && Mob[LOCAL_110].Next.Y == Mob[LOCAL_110].Target.Y)
				continue;

			p36C LOCAL_222;
			GetAction(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, &LOCAL_222);

			//	LOCAL_222.Command[0] = 0;
			LOCAL_222.MoveType = 0;

			GridMulticast(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, (BYTE*)&LOCAL_222);
		}

		if (LOCAL_162 & 2)
			Teleportar(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y);

		if (LOCAL_162 & 16)
		{
			p36C LOCAL_235;
			GetAction(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, &LOCAL_235);

			LOCAL_235.MoveType = 0;

			GridMulticast(LOCAL_110, Mob[LOCAL_110].Next.X, Mob[LOCAL_110].Next.Y, (BYTE*)&LOCAL_235);
		}
	}

	if (sData.SizeOfData != 0)
	{
		int ret = send(sData.Socket, (char*)sData.sendBuffer, sData.SizeOfData, 0);

		// Enviado com sucesso
		if (ret == sData.SizeOfData)
			sData.SizeOfData = 0;
		else
			Log(SERVER_SIDE, LOG_INGAME, "Falha ao tentar enviar o pacote para a DBSRV. Erro: %d", WSAGetLastError());
	}

	if (!(counter % 2))
	{
		auto nowSteady = std::chrono::steady_clock::now();
		for(auto& scheduled : sServer.Scheduled)
		{
			int day = scheduled.Day;
			int month = scheduled.Month;

			int hour = scheduled.Hour;
			int min = scheduled.Min;

			if (!scheduled.Executed && day == now.tm_mday && month == (now.tm_mon + 1) && hour == now.tm_hour && min == now.tm_min)
			{
				p334 packet{};
				packet.Header.ClientId = SCHEDULE_ID;
				packet.Header.PacketId = 0x334;

				strcpy_s(packet.eCommand, "admin");
				strcpy_s(packet.eValue, scheduled.Command.c_str());

				Users[SCHEDULE_ID].RequestCommand((PacketHeader*)&packet);

				Log(SERVER_SIDE, LOG_ADMIN, "[SISTEMA] Scheduled chamado: %s", scheduled.Command.c_str());

				scheduled.ExecuteTime = nowSteady;
				scheduled.Executed = true;
			}
			else if (scheduled.Executed && nowSteady - scheduled.ExecuteTime >= 1min)
				scheduled.Executed = false;
		}
	}

	// Auto venda premiada
	if (!(counter % 2))
	{
		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			CUser *user = &Users[i];
			if (user->Status != USER_PLAY)
				continue;

			// Checa se está em autovenda
			// Caso não esteja, checa se a loja estava ativa durante os 5 minutos anteriores
			if (!user->IsAutoTrading)
			{
				if (user->PremierStore.Status)
				{
					user->PremierStore.Wait++;

					if (user->PremierStore.Wait >= 300)
					{
						// Desativa tudo
						user->PremierStore.Status = 0;
						user->PremierStore.Time = 0;
						user->PremierStore.Wait = 0;
						user->PremierStore.Count = 0;
					}
				}

				// skipa o código restante pois a loja não está aberta
				continue;
			}

			// Está habilitado a receber a paradita
			if (!user->PremierStore.Status)
				continue;

			user->PremierStore.Time++;

			if (user->PremierStore.Time >= 3600)
			{
				user->PremierStore.Time = 0;

				if (user->PremierStore.Status == 1)
				{
					st_Item item{};
					item.Index = 4545;

					if (PutItem(i, &item))
					{
						Log(i, LOG_INGAME, "Recebido item %s [%d] autovenda 1hora online.", ItemList[item.Index].Name, item.Index);

						SendClientMessage(i, "!Chegou um item: [ %s ]", ItemList[item.Index].Name);
					}
					else
					{
						Log(i, LOG_INGAME, "Não recebeu o item %s [%d] autovenda 1hora online por FALTA DE ESPAÇO.", ItemList[item.Index].Name, item.Index);

						SendClientMessage(i, "!Não recebeu o item [ %s ] por falta de espaço", ItemList[item.Index].Name);
					}

					user->PremierStore.Status = 2;
					user->PremierStore.Time = 0;
				}
				else if (user->PremierStore.Status == 2)
				{
					st_Item item;
					memset(&item, 0, sizeof st_Item);

					item.Index = 4546;

					if (PutItem(i, &item))
					{
						Log(i, LOG_INGAME, "Recebido item %s [%d] autovenda 1hora online.", ItemList[item.Index].Name, item.Index);

						SendClientMessage(i, "!Chegou um item: [ %s ]", ItemList[item.Index].Name);
					}
					else
					{
						Log(i, LOG_INGAME, "Não recebeu o item %s [%d] autovenda 1hora online por FALTA DE ESPAÇO.", ItemList[item.Index].Name, item.Index);

						SendClientMessage(i, "!Não recebeu o item [ %s ] por falta de espaço", ItemList[item.Index].Name);
					}

					user->PremierStore.Count++;
					if (user->PremierStore.Count > 5)
					{
						// Desativa tudo
						user->PremierStore.Status = 0;
						user->PremierStore.Time = 0;
						user->PremierStore.Wait = 0;
						user->PremierStore.Count = 0;
					}
					else
					{
						user->PremierStore.Status = 1;
						user->PremierStore.Time = 0;
						user->PremierStore.Wait = 0;
					}
				}
			}
		}
	}

	//NÃO REMOVER - Se remover eu como um cu ^^
	sServer.SecCounter = counter;
}

void ProcessMinTimer()
{
	int counter = sServer.MinCounter;
	counter++;

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	SaveGuildZone();
	WriteGameConfigv2();
	WriteRvRStore();

	if (sServer.TotalServer == 0)
		sServer.TotalServer = 1;

	// 8BF1838 mincounter
	// 2 = ISDST, 3 = YEAR DAY, 4 = WEEK DAY, 5 = YEAR, 6 = MONTH, 6 = MONTHDAY, 7 = HOUR, 8 = MIN, 9 = SEC
	if ((counter % 5))
	{// 00455BD9
		// Código que faz o canal ser novato ou canal de guerra
		INT32 LOCAL_11 = (now.tm_mday - 1) % sServer.TotalServer;
		if (LOCAL_11 == (sServer.Channel - 1))
			sServer.NoviceChannel = 1;
		else
			sServer.NoviceChannel = 1;

		if (!sServer.NoviceChannel)
		{
			for (INT32 i = 1; i < MAX_PLAYER; i++)
			{
				if (Users[i].Status != USER_PLAY)
					continue;

				if (Users[i].IsAutoTrading)
					RemoveTrade(i);
			}
		}

		int myChannel = sServer.Channel;
		if (now.tm_wday == DOMINGO)
			sServer.WarChannel = 1;
		else
			sServer.WarChannel = 0;

		if (now.tm_hour == 22 && !sServer.NewbieEventServer && sServer.NoviceChannel == 1)
			sServer.NewbieEventServer = 1;
		else if (now.tm_hour != 22 && sServer.NewbieEventServer)
			sServer.NewbieEventServer = 0;
	}

	if (now.tm_hour == 0 && now.tm_min == 0)
	{ // 00:00 - A cada 12 segundos é chamada, então, essa função basicamente
	  // vai ser chamada 5 vezes, o que não importa, porque é só por um minuto
	  // E foda-se, eu que mando nesta merda, filho da putaa! : )

		// Reseta as taxas do dia
		for (int i = 0; i < 5; i++)
			sServer.TaxesDay[i] = 0;
	}

	if (sServer.Zakum.Clear == 1)
	{
		sServer.Zakum.Clear = 2;

		SendNoticeArea(g_pLanguageString[_NN_Zakum_Initialize], 0x880, 0x488, 0x8FC, 0x4FC);
	}
	else if (sServer.Zakum.Clear == 2)
	{
		sServer.Zakum.Clear = 0;
		sServer.Zakum.IsOperating = false;

		ClearArea(0x884, 0x488, 0x8F8, 0x4D4);
	}

	if (sServer.Kingdom1Clear == 1)
		sServer.Kingdom1Clear = 2;
	else if (sServer.Kingdom1Clear == 2)
	{
		sServer.Kingdom1Clear = 0;

		ClearArea(0x68C, 0x718, 0x6F0, 0x764);
	}

	if (sServer.Kingdom2Clear == 1)
		sServer.Kingdom2Clear = 2;
	else if (sServer.Kingdom2Clear == 2)
	{
		sServer.Kingdom2Clear = 0;

		ClearArea(0x68C, 0x718, 0x6F0, 0x764);
	}

	for (int i = 1; i <= mGener.numList; i++)
	{
		stGener *genMob = &mGener.pList[i];
		INT32 LOCAL_28 = genMob->MinuteGenerate;
		if (i == 0 || i == 1 || i == 2)
			continue;

		if (i == 5 || i == 6 || i == 7)
			continue;

		if (LOCAL_28 == -1)
			continue;

		if (LOCAL_28 <= 0)
		{
			// error
			continue;
		}

		INT32 LOCAL_29 = i % LOCAL_28;
		if ((counter % LOCAL_28) == LOCAL_29)
			GenerateMob(i, 0, 0);
	}

	if (!(counter % 16))
	{
		memset(&FailAccount, 0, 256);
		CurrentTime = GetTickCount();
	}

	UINT32 LOCAL_15 = 20;
	// TODO : O 100 é o valor total de portões carregados no InitItem
	for (; LOCAL_15 < sServer.InitCount; LOCAL_15++)
	{
		UINT16 LOCAL_16 = g_pInitItem[LOCAL_15].PosX;
		UINT16 LOCAL_17 = g_pInitItem[LOCAL_15].PosY;

		if (LOCAL_16 < 0 || LOCAL_16 > 4096 || LOCAL_17 < 0 || LOCAL_17 > 4096)
			continue;

		INT32 LOCAL_18 = g_pItemGrid[LOCAL_17][LOCAL_16];
		if (LOCAL_18 < 0 || LOCAL_18 >= 4096)
			continue;

		if (pInitItem[LOCAL_18].Item.Index <= 0 || pInitItem[LOCAL_18].Item.Index >= MAX_ITEMLIST)
			continue;

		st_Item* LOCAL_19 = &pInitItem[LOCAL_18].Item;

		INT32 LOCAL_20 = GetItemAbility(LOCAL_19, EF_KEYID);
		if (LOCAL_20 != 0 && pInitItem[LOCAL_18].Status == 1 && LOCAL_20 < 15)
		{
			if (pInitItem[LOCAL_18].IsOpen == 0)
			{
				pInitItem[LOCAL_18].IsOpen = 1;
				continue;
			}

			int LOCAL_21;
			UpdateItem(LOCAL_18, 3, &LOCAL_21);

			p374 packet;
			packet.Header.ClientId = 0x7530;
			packet.Header.PacketId = 0x374;
			packet.gateId = LOCAL_18 + 10000;

			packet.Header.Size = sizeof p374;
			packet.status = pInitItem[LOCAL_18].Status;
			packet.unknow = LOCAL_21;
			packet.unknow = 0;

			GridMulticast_2(LOCAL_16, LOCAL_17, (BYTE*)&packet, 0);

			pInitItem[LOCAL_18].IsOpen = 0;
		}
	}

	//GuildProcess();

	INT32 LOCAL_37 = Rand() % 0x4B0;
	if (sServer.ForceWeather == -1)
	{
		if (LOCAL_37 >= 0 && LOCAL_37 < 260 && sServer.Weather != 0)
		{
			sServer.Weather = 0;

			SendWeather();
		}
		else if (LOCAL_37 > 30 && LOCAL_37 < 50 && sServer.Weather != 1)
		{
			sServer.Weather = 1;

			SendWeather();
		}
		else if (LOCAL_37 > 55 && LOCAL_37 < 60 && sServer.Weather != 2)
		{
			sServer.Weather = 2;

			SendWeather();
		}
	}
	else
	{
		if (sServer.Weather != sServer.ForceWeather)
		{
			sServer.Weather = sServer.ForceWeather;
			sServer.ForceWeather = -1;

			SendWeather();
		}
	}

	time_point_t nowChrono = std::chrono::steady_clock::now();
	for (const auto& user : Users)
	{
		if (user.Status != USER_PLAY)
			continue;

		if (nowChrono - user.MacIntegrity.loginTime > 5s && !user.MacIntegrity.IsChecked && !user.MacIntegrity.WasWarned)
		{
			Log(user.clientId, LOG_HACK, "O usuário não enviou o pacote de integridade de macaddress");
			Log(SERVER_SIDE, LOG_HACK, "O usuário %s não enviou o pacote de integridade de macaddress", user.User.Username);

			user.MacIntegrity.WasWarned = true;
		}
	}

	//NÃO REMOVER
	sServer.MinCounter = counter;
}

void ProcessHourTimer()
{
	// Cada hora incrementa 1
	// !(counter % 10) quer dizer que passaram-se 10 horas
	int counter = sServer.HourCounter;
	{
		INT32 totalOn = 0,
			on = 0;

		for (INT32 x = 1; x < MAX_PLAYER; x++)
		{
			if (Users[x].Status < USER_SELCHAR)
				continue;

			totalOn++;

			INT32 t = x;
			for (; t < MAX_PLAYER; t++)
			{
				if (Users[t].Status < 11)
					continue;

				if (t == x)
					continue;

				if (!memcmp(&Users[x].MacAddress, &Users[t].MacAddress, 6))
					break;
			}

			if (t != MAX_PLAYER)
				continue;

			on++;
		}

		Log(SERVER_SIDE, LOG_INGAME, "Total de usuários online> %d com %d computadores", totalOn, on);
	}

	std::stringstream str;
	for (int i = 0; i < 5; i++)
	{
		char result[32];
		memset(result, 0, 32);

		sprintf_s(result, "%I64d", g_pCityZone[i].impost);
		NumberFormat(result);

		str << "Imposto na cidade de " << szCitys[i] << " é de " << result << std::endl;
	}

	Log(SERVER_SIDE, LOG_INGAME, "Total de Imposto: %s", str.str().c_str());

	//NÃO REMOVER
	sServer.HourCounter = counter;
}