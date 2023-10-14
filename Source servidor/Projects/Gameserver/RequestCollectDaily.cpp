#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"

bool CUser::RequestCollectDaily(PacketHeader* Header)
{
	static const char* const weekName[] =
	{
		"domingo", "segunda-feira",
		"terça-feira", "quarta-feira", "quinta-feira",
		"sexta-feira", "sábado"
	};

	stAccount* account = &Users[clientId].User;

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	INT32 weekYear = getWeek(now.tm_mday, now.tm_mon, now.tm_year);

	if (weekYear != account->Daily.WeekYear)
	{
		memset(&account->Daily.Received[0], 0, sizeof(bool) * 7);

		account->Daily.WeekYear = weekYear;
	}

	INT32 day = now.tm_wday;
	if (account->Daily.Received[day] != 0)
	{
		SendClientMessage(clientId, "Você já recebeu esta premiação!");

		return true;
	}

	switch (day)
	{
		case 0: // Domingo
		{
			SetBuff(clientId, 39, 0, 0, 450);
		}
		break;
		case 1: // Segunda-feira
		{
			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(Mob[clientId].Mobs.DailyQuest.BattlePass.GetTMStruct(), now_time_t);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + 1;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			auto battlePass = Mob[clientId].Mobs.DailyQuest.BattlePass;
			if (diffTime > 0.0)
			{
				month = battlePass.Mes; //0 Janeiro, 1 Fev
				day = battlePass.Dia + 1;
				year = battlePass.Ano ;
				hour = battlePass.Hora;
				min = battlePass.Minuto;
				sec = battlePass.Segundo;
			}

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			Mob[clientId].Mobs.DailyQuest.BattlePass.Dia = day;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Mes = month;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Ano = year;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Hora = hour;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Minuto = min;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Segundo = sec;

			SendMissionInfo(clientId);
		}
		break;
		case 2: // Terça-feira
		{
			int slot = GetEmptyAffect(clientId, 55);
			if (slot == -1)
			{
				SendClientMessage(clientId, "Você não tem slot de buff vago.");

				return true;
			}

			Mob[clientId].Mobs.Affects[slot] = st_Affect{};
			Mob[clientId].Mobs.Affects[slot].Index = 55;
			Mob[clientId].Mobs.Affects[slot].Time = 1800;
			Mob[clientId].Mobs.Affects[slot].Value = 1;

			p364 packet{};
			GetCreateMob(clientId, (BYTE*)&packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);

			SendAffect(clientId);
		}
		break;
		case 3:
		{
			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon; //0 Janeiro, 1 Fev
			int day = now.tm_mday;
			int year = now.tm_year;

			int mes = 0, dia = 0, ano = 0;
			int days = 1;

			if (TimeRemaining(Mob[clientId].Mobs.Player.Inventory[60].EFV1, Mob[clientId].Mobs.Player.Inventory[60].EFV2, Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) > 0.0f)
			{
				dia = Mob[clientId].Mobs.Player.Inventory[60].EFV1;
				mes = Mob[clientId].Mobs.Player.Inventory[60].EFV2;
				ano  = Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900;
			}
			else
			{
				mes = month + 1;
				dia = day;
				ano = year + 1900;
			}

			dia += days;
			if (dia > dias_mes[mes])
			{
				dia -= dias_mes[mes];
				mes += 1;
			}

			if (mes > 12)
			{
				mes -= 12;
				ano += 1;
			}

			Mob[clientId].Mobs.Player.Inventory[60].Index = 3467;
			Mob[clientId].Mobs.Player.Inventory[60].EF1 = 106;
			Mob[clientId].Mobs.Player.Inventory[60].EFV1 = dia;
			Mob[clientId].Mobs.Player.Inventory[60].EF3 = 109;
			Mob[clientId].Mobs.Player.Inventory[60].EFV3 = (ano - 1900);
			Mob[clientId].Mobs.Player.Inventory[60].EF2 = 110;
			Mob[clientId].Mobs.Player.Inventory[60].EFV2 = mes;

			st_Item emptyItem{ };
			SendItem(clientId, SlotType::Inv, 60, &emptyItem);
			SendItem(clientId, SlotType::Inv, 60, &Mob[clientId].Mobs.Player.Inventory[60]);
		}
		break;
		case 4:	// quinta
			SetBuff(clientId, 39, 0, 0, 900);
			break;
		case 5: // sexta-feira
		{
			st_Affect* affects = Mob[clientId].Mobs.Affects;

			int affectIndex = -1;
			for (int i = 0; i < 32; i++)
			{
				if (affects[i].Index == 4 && (affects[i].Value == 4 || affects[i].Value == 5))
					continue;

				if (affects[i].Index == 4)
				{
					affectIndex = i;
					break;
				}
			}

			if (affectIndex == -1)
				affectIndex = GetEmptyAffect(clientId, 0);

			if (affectIndex == -1)
			{
				SendClientMessage(clientId, "Não possui espaço nos buffs para receber");
				return true;
			}

			affects[affectIndex].Index = 4;
			affects[affectIndex].Value = 7;
			affects[affectIndex].Time = 480;

			SendScore(clientId);
		}
		break;
		case 6: // sabado
			SetBuff(clientId, 30, 0, 1, 450);
			break;
	}

	Log(clientId, LOG_INGAME, "Recebeu a Premiação diária referente ao dia: %s.", weekName[day]);

	SendClientMessage(clientId, "Coletado com sucesso referente ao dia de %s!!", weekName[day]);

	account->Daily.Received[day] = 1;

	SendDailyRewardInfo(clientId);

	Mob[clientId].GetCurrentScore(clientId);

	SendEtc(clientId);
	SendScore(clientId);
	return true;
}