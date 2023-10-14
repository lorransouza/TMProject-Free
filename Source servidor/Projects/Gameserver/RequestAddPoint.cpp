#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"


#pragma optimize( "", off )
bool CUser::RequestAddPoint(PacketHeader *header)
{
	//00436263 

	p277 *p = (p277*)header;
	if (p->Header.Size != sizeof p277)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Error: p is size different from p277 %hu", p->Header.Size);

		return true;
	}
	if (header->ClientId <= 0 || header->ClientId >= MAX_PLAYER)
		return true;

	int clientId = p->Header.ClientId;

	if (Mob[clientId].Mobs.Player.Status.curHP == 0 || Users[clientId].Status != USER_PLAY)
	{
		SendHpMode(clientId);
		AddCrackError(clientId, 10, 20);

		return true;
	}

	if (p == nullptr)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Error: p is null %s %s", __FILE__, __FUNCTION__);

		return true;
	}

	if (p->Mode == 0)
	{
		if (Mob[clientId].Mobs.Player.StatusPoint <= 0)
		{
			SendEtc(clientId);
			return true;
		}

		if (p->Info < 0 || p->Info > 3)
		{
			Log(clientId, LOG_ERROR, "Error no ApplyBonus, info fora de indice");

			return true;
		}

		int addPoints = 1;
		if (Mob[clientId].Mobs.Player.StatusPoint > 200)
			addPoints = 100;

		Mob[clientId].Mobs.Player.StatusPoint -= addPoints;

		if (p->Info == 0)
			Mob[clientId].Mobs.Player.bStatus.STR += addPoints;

		if (p->Info == 1)
			Mob[clientId].Mobs.Player.bStatus.INT += addPoints;

		if (p->Info == 2)
			Mob[clientId].Mobs.Player.bStatus.DEX += addPoints;

		if (p->Info == 3)
			Mob[clientId].Mobs.Player.bStatus.CON += addPoints;

		Mob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);
		SendEtc(clientId);
		return true;
	}
	else if (p->Mode == 1)
	{
		if (Mob[clientId].Mobs.Player.MasterPoint <= 0)
		{
			SendEtc(clientId);

			return true;
		}

		if (p->Info < 0 || p->Info > 3)
		{
			Log(clientId, LOG_ERROR, "ApplyBonus Special, Info fora de indice.");

			return true;
		}

		// info mastery[0] level
		if (Mob[clientId].Mobs.Player.bStatus.Mastery[p->Info] >= (((Mob[clientId].Mobs.Player.bStatus.Level + 1) * 3) >> 1) && Mob[clientId].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Maximum_Point_Now]);
			return true;
		}

		int max = 0;
		if (Mob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			max = 200;

		int has = Mob[clientId].Mobs.Player.Learn[0] & (1 << (8 * p->Info - 1));
		max = (has) ? 255 : 200;

		if (Mob[clientId].Mobs.Player.bStatus.Mastery[p->Info] >= max)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Maximum_Point_200_Now]);
			return true;
		}

		Mob[clientId].Mobs.Player.MasterPoint -= 1;
		Mob[clientId].Mobs.Player.bStatus.Mastery[p->Info] += 1;

		Mob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);
		SendEtc(clientId);
		return true;
	}

	else if (p->Mode == 2)
	{
		int classInfo = Mob[clientId].Mobs.Player.ClassInfo;

		int skillClass = (p->Info - 5000) / 24;
		int skillPos = (p->Info - 5000) % 24;

		int mobId = p->unk;

		if (p->Info < 5000 || p->Info > 5095)
		{
			Log(clientId, LOG_ERROR, "ApplyPoint RequestSkill out of bound");
			return true;
		}

		if (mobId < 0 || mobId >= 30000)
		{
			Log(clientId, LOG_ERROR, "ApplyBonus RequestTargetId is out of bound");
			return true;
		}

		if (classInfo != skillClass)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Learn_Other_Class_Skill]);

			return true;
		}

		int skillBonus = SkillData[skillClass * 24 + skillPos].Points;
		if (skillBonus > Mob[clientId].Mobs.Player.SkillPoint)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Enough_Skill_Point]);
			return true;
		}

		unsigned int learned = 1 << skillPos;
		if ((learned & Mob[clientId].Mobs.Player.Learn[0]) != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Already_Learned_It]);
			return true;
		}

		unsigned int level = ItemList[p->Info].Level;
		if (level > Mob[clientId].Mobs.Player.Status.Level && Mob[clientId].Mobs.Player.Equip[0].EFV2 < static_cast<int>(CELESTIAL))
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Need_More_Level_To_Learn]);
			return true;
		}

		if (Mob[clientId].Mobs.Player.Status.Mastery[1] < ItemList[p->Info].Int ||
			Mob[clientId].Mobs.Player.Status.Mastery[2] < ItemList[p->Info].Dex ||
			Mob[clientId].Mobs.Player.Status.Mastery[3] < ItemList[p->Info].Con)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Need_More_Mastery_To_Learn]);
			return true;
		}

		static int gold[4] =
		{
			5000000,
			10000000,
			50000000,
			50000000,
		};

		if ((skillPos % 24) % 8 == 7)
		{
			for (int i = 1; i < 8; i++)
			{
				int have = Mob[clientId].Mobs.Player.Learn[0] & (1 << (skillPos - i));
				if (!have)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Need_All_Skill_Lineage]);

					return true;
				}
			}

			int skillDiv = ((p->Info - 5000) % 24 / 8) + 1;
			for (int i = 1; i < 4; i++)
			{
				if (i == skillDiv)
					continue;

				int have = Mob[clientId].Mobs.Player.Learn[0] & (1 << ((i * 8) - 1));
				if (have)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Only_A_8th_Per_Char]);

					return true;
				}
			}

			if (Mob[clientId].Mobs.Player.Gold < gold[Mob[clientId].Mobs.Player.Equip[0].EFV2 - 1])
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Need_X_Gold_To_Buy_Skill]);

				return true;
			}
		}

		Log(clientId, LOG_INGAME, "Comprou uma skill %s. Learn antigo: %llu, Learn atual: %llu. Learn da skill: %d.", ItemList[p->Info].Name,
			Mob[clientId].Mobs.Player.Learn, Mob[clientId].Mobs.Player.Learn[0] | learned, learned);

		Mob[clientId].Mobs.Player.Learn[0] = Mob[clientId].Mobs.Player.Learn[0] | learned;
		Mob[clientId].Mobs.Player.SkillPoint -= skillBonus;

		if ((skillPos % 24) % 8 == 7)
			Mob[clientId].Mobs.Player.Gold -= gold[Mob[clientId].Mobs.Player.Equip[0].EFV2 - 1];

		Mob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);
		SendEtc(clientId);
		return true;
	}
	else
	{
		Log(clientId, LOG_ERROR, "ApplyBonus out of range.");

		return true;
	}
	return true; 
}
#pragma optimize("", on)

