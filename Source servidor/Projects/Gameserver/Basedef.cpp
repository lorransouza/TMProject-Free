#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <array>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <filesystem>
#include <map>
#include "pugixml.hpp"
#include "UOD_EventManager.h"
#include "UOD_ChristmasMission.h"
#include "UOD_BossEvent.h"
#include "UOD_Mission.h"
#include "UOD_Arena.h"
#include "UOD_PreWarBoss.h"
#include "ItemsAction.h"
#include <tuple>
#include <ctime>
#include <format>


using namespace std::string_literals;
// Declaração das variáveis externs
char g_pLanguageString[MAX_STRING][MAX_MESSAGE];
WORD g_pMobGrid[4096][4096];
char g_pHeightGrid[4096][4096];
MapAttribute g_pAttributeMap[1024][1024];
sItemData ItemList[MAX_ITEMLIST];
sSpellData SkillData[256];
char EffectName[256][32];
stTeleport mTeleport[MAX_TELEPORT];
st_Mob NPCBase[50];
WORD g_pItemGrid[4096][4096];
stInitItem pInitItem[4096];
BASE_InitItem g_pInitItem[4096];
sGuildZone g_pCityZone[5];
int ChargedGuildList[10][5];
DWORD g_pHitRate[1024];
stPista pPista[MAX_ROOM];
char g_pGuildNotice[MAX_GUILD][128];
stGuild g_pGuild[MAX_GUILD];
INT32 g_pGuildPoint [MAX_GUILD];
INT32 g_pGuildAlly[MAX_GUILD];
INT32 g_pGuildWar[MAX_GUILD];
stNPCEvent npcEvent[MAX_NPCEVENTO];
stNPCQuest questNPC[MAX_NPCQUEST];
stPositionCP g_pPositionCP[MAX_MESSAGE];
stDonateStore g_pStore[MAX_STORE][MAX_DONATEITEM];
INT16 g_pBlockedItem[MAX_BLOCKITEM];
stPacItem g_pPacItem[MAX_PACITEM];

std::vector<LojaDonate> ControlDonateLoja = std::vector<LojaDonate>();


int g_pGenerateIndexes[MAX_ROOM] = {LICH_ID, TORRE_ID, VALKY_ID, SULRANG, HELL_BOSS};
int g_pGenerateLoops[MAX_ROOM] = {1, 3, 1, 3, 1, 0};

/* Event variables */
constexpr std::array<std::array<int, 9>, 7> Runes = 
{ {
	{ 5110, 5112, 5115, 5113, 5111, 0, 0, 0, 0 },
	{ 5114, 5113, 5117, 5111, 5115, 5112, 0, 0, 0 },
	{ 5118, 5121, 5122, 5116, 5130, 5119, 0, 0, 0 },
	{ 5122, 5126, 5121, 5116, 5119, 0, 0, 0, 0 },
	{ 5125, 5126, 5124, 5127, 0, 0, 0, 0, 0 },
	{ 5120, 5131, 5118, 5119, 5123, 5132, 5121, 0, 0},
	{ 5130, 5131, 5119, 5133, 5120, 5123, 5132, 5129, 5128 }
	}
};

int g_pTeleBarlog[5][4][8] =
{ // maxX, maxY, minX, minY, centerX, centerY
	{
		{3381, 1168, 3379, 1166, 3431, 1182, 2},
		{3381, 1180, 3379, 1178, 3431, 1182, 2},
	},
	{
		{3342, 1225, 3340, 1223, 3359, 1174, 3},
		{3353, 1226, 3351, 1223, 3359, 1174, 3},
		{3364, 1225, 3362, 1223, 3359, 1174, 3}
	},
	{
		{3380, 1239, 3378, 1237, 3348, 1263, 4},
		{3388, 1239, 3386, 1237, 3348, 1263, 4},
		{3396, 1239, 3394, 1237, 3348, 1263, 4},
		{3404, 1239, 3402, 1237, 3348, 1263, 4}
	},
	{
		{3431, 1199, 3429, 1197, 3422, 1232, 1}
	},
	{
		{0, 0, 0, 0, 3384, 1199, 0}
	}
};

unsigned int g_pPistaCoords[7][6] = {
	{3328,1599,3452,1660,3350,1655},
	{3328,1539,3452,1595,3393,1585},
	{3328,1407,3452,1466,3418,1451},
	{3328,1025,3452,1148,3380,1085},
	{3328,1280,3454,1406,3444,1396},
	{3328,1153,3452,1276,3430,1180},
	{3328,1467,3452,1535,3434,1502}
};

INT32 g_pGroundMask[6][4][6][6] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 
	0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 
	0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 
	0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 
	18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 
	0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 
	18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 
	18, 18, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 
	18, 0, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 18, 18, 18, 18, 18, 0, 0, 0, 0,
	0, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0, 0, 18, 0
};

st_MountData mMont[30] =
{
	{0, 1, 0, 0, 4},// 0 - Porco
	{0, 1, 0, 0, 4}, // 1 - Javali
	{50, 9, 0, 0, 5}, // 2 - Lobo
	{80, 14, 0, 0, 5},// 3 -  Dragão Menor
	{100, 19, 0, 0, 4}, // 4 - Urso
	{150, 23, 0, 0, 5}, // 5 - Dente de Sabre
	{250, 47, 40, 0, 6}, // 6 - S/ Sela N
	{300, 57, 50, 0, 6}, //7 - Fantasma N
	{350, 61, 60, 0, 6}, // 8 - Leve N
	{400, 66, 70, 0, 6}, // 9 - Equip N
	{500, 80, 80, 0, 6}, // 10 - Anda N
 	{250, 47, 0, 16, 6}, // 11 - s/ sela B
	{300, 57, 0, 20, 6}, // 12 - Fantasma B
	{350, 61, 0, 24, 6}, // 13 - Leve B
	{400, 66, 0, 28, 6}, // 14 - Equip B
	{500, 80, 0, 32, 6}, // 15 - Anda B
	{550, 85, 0, 0, 6}, // 16 - Fenrir
	{600, 85, 0, 0, 6},  // 17 - Dragão
	{550, 85, 0, 20, 6}, // 18 - Fenrir das Sombras
	{650, 95, 60, 28, 6}, // 19 - Tigre de Fogo
	{700, 104, 80, 32, 6}, // 20 - DV
	{570, 85, 20, 16, 6}, // 21 - Unicórnio
	{570, 85, 30, 8, 6}, // 22 - Pegasus
	{570, 85, 40, 12, 6}, // 23 - unisus
	{590, 90, 30, 20, 6}, // 24 - Grifo
	{600, 90, 40, 16, 6}, // 25 - HipoGrifo 
	{600, 90, 50, 16, 6}, // 26 - Grifo Sangrento
	{600, 38, 60, 28, 6}, // 27 - Svaldfire
	{300, 90, 60, 28, 6}, // 28 - sleipnir
	{150, 23, 0, 20, 6} // pantera
};

TransBonus pSummonBonus[50] = {
	 { 80, 350, 70, 75, 100, 400, 0, 0, 0, 0, 0, 7, 0, 0, 100, 100, 5 },
	 { 80, 300, 70, 150, 125, 400, 0, 0, 0, 0, 0, 7, 0, 0, 100, 100, 20 },
	 { 80, 450, 70, 125, 125, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 20 },
	 { 80, 400, 70, 200, 150, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 70 },
	 { 80, 550, 70, 175, 150, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 90 },
	 { 80, 450, 70, 250, 175, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 110 },
	 { 100, 550, 60, 250, 174, 400, 0, 0, 0, 0, 0, 6, 0, 0, 100, 100, 140 },
	 { 130, 300, 80, 200, 180, 250, 0, 0, 0, 0, 6, 0, 0, 100, 100, 160, 0 }
 };

int Taxes[64] = {
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 
	900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 
	900, 900, 900
};

unsigned char ItemGrid[8][8] = {
    { 01, 00, 00, 00, 00, 00, 00, 00 }, { 01, 00, 01, 00, 00, 00, 00, 00 },
    { 01, 00, 01, 00, 01, 00, 00, 00 }, { 01, 00, 01, 00, 01, 00, 01, 00 },
    { 01, 01, 00, 00, 00, 00, 00, 00 }, { 01, 01, 01, 01, 00, 00, 00, 00 },
    { 01, 01, 01, 01, 01, 01, 00, 00 }, { 01, 01, 01, 01, 01, 01, 01, 01 }
};

//--------------------------------
// Read messages strings
//--------------------------------
bool ReadLanguageFile()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Language.txt", "rt");
	if (!pFile)
		return false;

	char szTMP[1024];
	while (fgets(szTMP, 1024, pFile))
	{
		if (*szTMP == '#')
			continue;

		int len = strlen(szTMP);
		for (int i = 0; i < len; i++)
		{
			if (szTMP[i] == '\t')
				szTMP[i] = ' ';
		}

		int index = -1;
		char message[128];


		int ret = sscanf_s(szTMP, "%d %*s %[^\n]", &index, message, 128);
		if (ret != 2)
			continue;

		strncpy_s(g_pLanguageString[index], message, 128);
	}

	fclose(pFile);
	return true;
}
/*
void GetCurrentScore_7556(CMob *arg1, st_Affect *arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int* arg8, int arg9, int *arg10)
{
	//0040A29C  |> 8B95 4CFFFFFF  /MOV EDX,[LOCAL.45]
	int LOCAL_45 = 0;
	for (; LOCAL_45 < 16; LOCAL_45++)
	{
		if (arg2 == 0)
			continue;

		int LOCAL_46 = arg2[LOCAL_45].Index;
		if (LOCAL_46 == 0)
			continue;

		int LOCAL_47 = arg2[LOCAL_45].Value;
		int LOCAL_48 = arg2[LOCAL_45].Master;


		int LOCAL_13 = 0; // SPEED
		int LOCAL_14 = 0; // ATTSPEED
		int LOCAL_41 = 0; // Resist1
		int LOCAL_42 = 0; // Resist2
		int LOCAL_43 = 0; // Resist3
		int LOCAL_44 = 0; // Resist4
		int LOCAL_36 = 100; // q
		int LOCAL_2 = 0; // ATTACK
		int LOCAL_1 = 0; // ?
		int LOCAL_3 = 0; // STATUS MAX HP
		int LOCAL_40 = 0;
		int LOCAL_38 = 100; // ? defense? ??
		int LOCAL_39 = 100; // ?
		int LOCAL_12 = 0; // SPECIALALL
		int LOCAL_34 = 0; // ?
		int LOCAL_35 = 0; // ?
		int LOCAL_37 = 100; // ?
		if (LOCAL_46 == 1)
		{
			if (LOCAL_48 == 255)
				LOCAL_13 -= 3;
			else
				LOCAL_13 -= 1;

			LOCAL_14 -= 30;
			continue;
		}

		if (LOCAL_46 == 2)
		{
			LOCAL_13 += LOCAL_47;

			int LOCAL_50 = *arg10;

			*arg10 = LOCAL_50 | 128;
			continue;
		}

		if (LOCAL_46 == 3)
		{
			int LOCAL_51 = LOCAL_47;

			if (arg1->Mobs.Player.Equip[0].Index <= 50)
				LOCAL_51 -= 40;

			if (LOCAL_48 == 255)
				LOCAL_51 += 20;

			LOCAL_41 -= LOCAL_51;
			LOCAL_42 -= LOCAL_51;
			LOCAL_43 -= LOCAL_51;
			LOCAL_44 -= LOCAL_51;

			continue;
		}

		if (LOCAL_46 == 4)
		{
			if (LOCAL_47 == 0)
			{
				LOCAL_36 += 3;
				LOCAL_2 += 20;
				LOCAL_1 += 10;
			}

			else if (LOCAL_47 == 1)
			{
				LOCAL_36 += 5;
				LOCAL_2 += 40;
				LOCAL_1 += 12;
			}

			else if (LOCAL_47 == 2)
			{
				LOCAL_36 += 6;
				LOCAL_2 += 60;
				LOCAL_1 += 16;
			}

			else if (LOCAL_47 == 3)
			{
				LOCAL_36 += 15;
				LOCAL_2 += 80;
				LOCAL_1 += 20;
			}
			continue;
		}

		if (LOCAL_46 == 5)
		{
			//LOCAL_114 
			/*float fValue = (100 - LOCAL_47) / 100.0; // local31

			arg1->Mobs.Player.Status.DEX *= fValue;
			

			int LOCAL_114 = 100 - LOCAL_47;
			float LOCAL_52 = LOCAL_114 / 100.0000f;

			int LOCAL_115 = arg1->Mobs.Player.Status.DEX;
			arg1->Mobs.Player.Status.DEX = LOCAL_115 * LOCAL_52;
			continue;
		}

		if (LOCAL_46 == 6)
		{
			int LOCAL_53 = *arg10;

			*arg10 = LOCAL_53 | 8;

			continue;
		}

		if (LOCAL_46 == 7)
		{
			int LOCAL_54 = (LOCAL_48 / 10) + 10;

			LOCAL_14 -= LOCAL_54;
			if (arg1->Mobs.Player.Equip[0].Index <= 50)
				continue;

			int LOCAL_55 = arg1->Mobs.Player.Status.INT;
			LOCAL_55 = LOCAL_55 - (LOCAL_54 + 10);

			arg1->Mobs.Player.Status.INT = LOCAL_55;

			continue;
		}

		if (LOCAL_46 == 8)
		{
			if (LOCAL_47 & 2)
			{
				int LOCAL_56 = 25;
				LOCAL_41 += LOCAL_56;
				LOCAL_42 += LOCAL_56;
				LOCAL_43 += LOCAL_56;
				LOCAL_44 += LOCAL_56;
			}

			if (LOCAL_47 & 16)
				LOCAL_3 = (arg1->Mobs.Player.Status.Level >> 1) + LOCAL_3 + ((arg1->Mobs.Player.Status.Defense * 20) / 100);

			if (LOCAL_47 & 32)
			{
				LOCAL_36 += 5;
				LOCAL_1 += 4;
				LOCAL_3 = (LOCAL_3 * 107) / 100;
			}

			if (LOCAL_47 & 128)
				LOCAL_40 = 1;

			continue;
		}

		if (LOCAL_46 == 9)
		{
			int LOCAL_57 = (LOCAL_48 / 3) + 15;

			LOCAL_36 += 5;

			if (arg1->Mobs.Player.ClassInfo == 1)
			{
				if (arg1->Mobs.Player.Learn & 0x100000)
				{
					LOCAL_57 *= 5;
					LOCAL_36 += 15;
					if (arg1->Mobs.Player.Learn & 0x800000)
					{
						LOCAL_36 += 7;
					}
				}

				LOCAL_2 += LOCAL_57;
				continue;
			}
		}

		if (LOCAL_46 == 10)
		{
			int LOCAL_58 = (LOCAL_48 / 5) + LOCAL_47;
			int LOCAL_59 = arg1->Mobs.Player.Status.Level;

			if (func_4012DA(arg5))
			{
				if (arg1->Mobs.Player.Learn & 0x40000000)
					LOCAL_59 = 400;
			}

			if (LOCAL_48 == 255)
				LOCAL_58 = LOCAL_58 + LOCAL_59 >> 1;

			LOCAL_2 -= LOCAL_58;
			continue;
		}

		if (LOCAL_46 == 11)
		{
			int LOCAL_60 = (LOCAL_48 / 3) + LOCAL_47;

			if (arg1->Mobs.Player.ClassInfo == 1)
			{
				if (arg1->Mobs.Player.Learn & 0x800000)
					LOCAL_60 += 100;
			}

			arg1->Mobs.Player.Status.Defense += LOCAL_60;

			continue;
		}

		if (LOCAL_46 == 12)
		{
			int LOCAL_116 = 100 - LOCAL_47;
			float LOCAL_61 = LOCAL_116 / 100.0000f;
			int LOCAL_117 = arg1->Mobs.Player.Status.Defense;

			arg1->Mobs.Player.Status.Defense = LOCAL_117 * LOCAL_61;

			continue;
		}

		if (LOCAL_46 == 13)
		{
			int LOCAL_62 = LOCAL_47 + (LOCAL_48 / 10);
			LOCAL_36 += LOCAL_62;

			if (arg1->Mobs.Player.Learn & 0x8000)
				LOCAL_36 += 5;

			LOCAL_38 -= 10;
			continue;
		}

		if (LOCAL_46 == 14)
		{
			int LOCAL_63 = arg1->Mobs.Player.Status.Level;

			if (func_4012DA(arg5))
			{
				if (arg1->Mobs.Player.Learn & 0x40000000)
					LOCAL_63 += 400;
			}

			int LOCAL_64 = LOCAL_47 + LOCAL_48 + ((LOCAL_63 << 2) / 3);

			int LOCAL_65 = arg1->Mobs.Player.Status.CON + LOCAL_64;

			int LOCAL_66 = 0;

			if (arg1->Mobs.Player.Learn & 0x80)
				LOCAL_66 += LOCAL_48;

			arg1->Mobs.Player.Status.CON = LOCAL_65 + LOCAL_66;
			continue;
		}

		if (LOCAL_46 == 15)
		{
			int LOCAL_67 = LOCAL_47 + (LOCAL_48 / 10);
			if (arg1->Mobs.Player.ClassInfo == 1)
			{
				if (arg1->Mobs.Player.Learn & 0x800000)
					LOCAL_67 = (LOCAL_67 * 120) / 100;
			}

			int LOCAL_68 = arg1->Mobs.Player.Status.Mastery[0] + LOCAL_67;

			if (LOCAL_68 > 255)
				LOCAL_68 = 255;

			arg1->Mobs.Player.Status.Mastery[0] = LOCAL_68;

			LOCAL_68 = arg1->Mobs.Player.Status.Mastery[1] + LOCAL_67;

			if (LOCAL_68 > 255)
				LOCAL_68 = 255;

			arg1->Mobs.Player.Status.Mastery[1] = LOCAL_68;


			LOCAL_68 = arg1->Mobs.Player.Status.Mastery[2] + LOCAL_67;

			if (LOCAL_68 > 255)
				LOCAL_68 = 255;

			arg1->Mobs.Player.Status.Mastery[2] = LOCAL_68;


			LOCAL_68 = arg1->Mobs.Player.Status.Mastery[3] + LOCAL_67;

			if (LOCAL_68 > 255)
				LOCAL_68 = 255;

			arg1->Mobs.Player.Status.Mastery[3] = LOCAL_68;

			continue;
		}

		if (LOCAL_46 == 16)
		{
			//0040AB38  |. 8B85 44FFFFFF  |MOV EAX,[LOCAL.47]

			int LOCAL_74 = LOCAL_47 - 1;

			if (LOCAL_74 < 0 || LOCAL_74 >= 5 || arg1->Mobs.Player.ClassInfo != 2)
				continue;

			if (LOCAL_74 != 4)
			{
				arg1->Mobs.Player.Equip[0].Index = LOCAL_74 + 22;
			}
			else
				arg1->Mobs.Player.Equip[0].Index = 32;

			int LOCAL_75 = 0;
			int LOCAL_76 = 0;
			int LOCAL_77 = 0;
			int LOCAL_78 = 0;
			int LOCAL_79 = 0;

			if (arg1->Mobs.Player.Equip[0].Index == 22)
			{
				if (arg1->Mobs.Player.Learn & 0x200000)
				{
					// 59A328

					LOCAL_75 = 10;
					LOCAL_79 = pTransBonus[LOCAL_74].Unknown_48;
					//0040AC04  |. 8B8D D8FEFFFF  |MOV ECX,[LOCAL.74]
					//smaster
					// unk44
					// unk40
					arg1->Mobs.Player.Critical = pTransBonus[LOCAL_74].Unknown_40 + ((arg1->Mobs.Player.Status.Mastery[2] / 50) * pTransBonus[LOCAL_74].Unknown_44);
				}
			}
			else if (arg1->Mobs.Player.Equip[0].Index == 17)
			{
				if (arg1->Mobs.Player.Learn & 0x200000)
				{
					LOCAL_76 = 20;
					LOCAL_78 += 24;
					LOCAL_79 += pTransBonus[LOCAL_79].Unknown_48;
				}
			}
			else if (arg1->Mobs.Player.Equip[0].Index == 24)
			{
				if (arg1->Mobs.Player.Learn & 0x200000)
				{
					LOCAL_75 = 10;
					LOCAL_77 = 5;
					LOCAL_76 = 5;

					LOCAL_78 += 20;

					LOCAL_79 += pTransBonus[LOCAL_79].Unknown_48;
				}
			}

			else if (arg1->Mobs.Player.Equip[0].Index == 25)
			{
				LOCAL_79 += pTransBonus[LOCAL_79].Unknown_48;
			}

			else if (arg1->Mobs.Player.Equip[0].Index == 32)
			{
				LOCAL_78 += 10;
				LOCAL_79 += pTransBonus[LOCAL_79].Unknown_48;
			}

			if (arg1->Mobs.Player.Learn & 0x800000)
			{
				LOCAL_75 += 5;
				LOCAL_77 += 2;
			}

			int LOCAL_80 = 0;

			LOCAL_80 = (LOCAL_80 - pTransBonus[LOCAL_74].Sanc) / 12;

			if (func_4012DA(arg5) && (arg1->Mobs.Player.Learn & 0x40000000))
			{
				LOCAL_80 = 9;
			}
			else
			{
				LOCAL_80 = (LOCAL_12 + (arg1->Mobs.Player.Status.Level * 2)) / 3;
				LOCAL_80 = (LOCAL_80 - pTransBonus[LOCAL_74].Sanc) / 12;
			}

			if (LOCAL_80 < 0)
				LOCAL_80 = 0;

			if (LOCAL_80 > 9)
				LOCAL_80 = 9;

			arg1->Mobs.Player.Equip[0].EF1 = 43;
			arg1->Mobs.Player.Equip[0].EFV1 = LOCAL_80;

			int LOCAL_70 = pTransBonus[LOCAL_74].MinAttack + LOCAL_75;
			int LOCAL_71 = pTransBonus[LOCAL_74].MaxAttack + LOCAL_75;
			int LOCAL_72 = LOCAL_71 - LOCAL_70;

			int LOCAL_73 = LOCAL_70 + ((LOCAL_72 * LOCAL_48) / 200);
			LOCAL_36 = LOCAL_36 + LOCAL_73 - 100;

			LOCAL_70 = pTransBonus[LOCAL_74].MinDefense + LOCAL_77;
			LOCAL_71 = pTransBonus[LOCAL_74].MaxDefense + LOCAL_77;

			LOCAL_72 = LOCAL_70 - LOCAL_71;

			LOCAL_73 = LOCAL_70 + ((LOCAL_72 * LOCAL_48) / 200);
			int LOCAL_69 = arg1->Mobs.Player.Status.Defense;

			LOCAL_69 = (LOCAL_69 * LOCAL_73) / 100;

			if (arg1->Mobs.Player.Equip[0].Index == 22)
				LOCAL_69 += 5;

			arg1->Mobs.Player.Status.Defense = LOCAL_69;

			LOCAL_70 = pTransBonus[LOCAL_74].MinHp + LOCAL_76;
			LOCAL_71 = pTransBonus[LOCAL_74].MaxHp + LOCAL_76;
			LOCAL_72 = LOCAL_71 - LOCAL_70;
			LOCAL_73 = LOCAL_70 + ((LOCAL_72 * LOCAL_48) / 200);

			LOCAL_69 = LOCAL_3;

			LOCAL_69 = (LOCAL_69 * LOCAL_73) / 100;
			LOCAL_3 = LOCAL_69;

			LOCAL_41 = LOCAL_41 + LOCAL_79;
			LOCAL_42 = LOCAL_42 + LOCAL_79;
			LOCAL_43 = LOCAL_43 + LOCAL_79;
			LOCAL_44 = LOCAL_44 + LOCAL_79;

			LOCAL_34 = pTransBonus[LOCAL_74].SpeedAttack + LOCAL_78;
			LOCAL_35 = pTransBonus[LOCAL_74].SpeedMove;

			continue;
		}

		if (LOCAL_46 == 18)
		{
			int LOCAL_81 = *arg10;
			*arg10 = LOCAL_81 | 1;
			continue;
		}

		if (LOCAL_46 == 19)
		{
			int LOCAL_81 = *arg10;
			*arg10 = LOCAL_81 | 2;
			continue;
		}

		if (LOCAL_46 == 21)
		{
			int LOCAL_83 = (LOCAL_48 / 3) + 10;

			arg1->Mobs.Player.Status.Defense -= LOCAL_83;
			int LOCAL_84 = LOCAL_47 + (LOCAL_48 / 10);

			if (arg1->Mobs.Player.Learn & 0x80)
				LOCAL_84 += 5;
			LOCAL_36 += LOCAL_84;

			continue;
		}

		if (LOCAL_46 == 24)
		{
			int LOCAL_85 = 0;
			int LOCAL_86 = 0;
			int LOCAL_87 = 0;
			int LOCAL_88 = *arg10;

			*arg10 = LOCAL_88 | 8;

			LOCAL_85 = LOCAL_48;

			arg1->Mobs.Player.Status.Defense += LOCAL_85;

			int LOCAL_89 = GetItemAbility(&arg1->Mobs.Player.Equip[7], 17);

			if (LOCAL_89 == 128)
			{
				LOCAL_86 += 30;
				LOCAL_37 += 30;
			}

			if (arg1->Mobs.Player.Learn & 0x8000)
				LOCAL_37 += 10;

			LOCAL_41 += LOCAL_86;
			LOCAL_42 += LOCAL_86;
			LOCAL_43 += LOCAL_86;
			LOCAL_44 += LOCAL_86;

			continue;
		}

		if (LOCAL_46 == 25)
		{
			int LOCAL_90 = (LOCAL_48 >> 2) + LOCAL_47;

			int LOCAL_91 = 0;
			LOCAL_90 = LOCAL_90 / 10;

			if (LOCAL_48 == 255)
				LOCAL_90 += 20;

			LOCAL_41 += LOCAL_90;
			LOCAL_42 += LOCAL_90;
			LOCAL_44 += LOCAL_90;
			continue;
		}

		if (LOCAL_46 == 26)
		{
			int LOCAL_92 = *arg10;
			*arg10 = LOCAL_92 | 0x20;
			continue;
		}

		if (LOCAL_46 == 27)
		{
			int LOCAL_92 = *arg10;
			*arg10 = LOCAL_92 | 0x4;
			continue;
		}

		if (LOCAL_46 == 28)
		{
			int LOCAL_92 = *arg10;
			*arg10 = LOCAL_92 | 0x40;
			continue;
		}

		if (LOCAL_46 == 29)
		{
			if (func_4012DA(arg5) && arg1->Mobs.Player.Learn & 0x40000000)
			{
				if (arg9 < 1 || arg9 > 10)
					arg9 = 1;

				int LOCAL_118 = arg9;

				LOCAL_118 -= 1;

				switch (LOCAL_118)
				{
				case 0:
				{
					arg1->Mobs.Player.Status.STR = arg1->Mobs.Player.Status.STR + ((arg1->Mobs.Player.Status.STR * 80) / 100);
					arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 60) / 100);
					break;
				}
				case 1:
				{
					arg1->Mobs.Player.Status.INT = arg1->Mobs.Player.Status.INT + ((arg1->Mobs.Player.Status.INT * 80) / 100);
					arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 60) / 100);
					break;
				}
				case 2:
				{
					arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 80) / 100);
					arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 60) / 100);
					break;
				}

				case 3:
				{
					arg1->Mobs.Player.Status.STR = arg1->Mobs.Player.Status.STR + ((arg1->Mobs.Player.Status.STR * 80) / 100);
					arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 60) / 100);
					break;
				}

				case 4:
				{
					arg1->Mobs.Player.Status.INT = arg1->Mobs.Player.Status.INT + ((arg1->Mobs.Player.Status.INT * 80) / 100);
					arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 60) / 100);
					break;
				}

				case 5:
				{
					arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 80) / 100);
					arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 60) / 100);
					break;
				}

				case 6:
				{
					arg1->Mobs.Player.Status.STR = arg1->Mobs.Player.Status.STR + ((arg1->Mobs.Player.Status.STR * 120) / 100);
					break;
				}

				case 7:
				{
					arg1->Mobs.Player.Status.INT = arg1->Mobs.Player.Status.INT + ((arg1->Mobs.Player.Status.INT * 120) / 100);
					break;
				}

				case 8:
				{
					arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 120) / 100);
					break;
				}

				case 9:
				{
					arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 120) / 100);
					break;
				}
				}
			}
			else
			{
				if (arg1->Mobs.Player.Learn & 0x40000000)
				{
					if (arg9 < 1 || arg9 > 10)
						arg9 = 1;

					int LOCAL_119 = arg9;

					LOCAL_119 -= 1;

					switch (LOCAL_119)
					{
					case 0:
					{
						arg1->Mobs.Player.Status.STR = arg1->Mobs.Player.Status.STR + ((arg1->Mobs.Player.Status.STR * 40) / 100);
						arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 30) / 100);
						break;
					}
					case 1:
					{
						arg1->Mobs.Player.Status.INT = arg1->Mobs.Player.Status.INT + ((arg1->Mobs.Player.Status.INT * 40) / 100);
						arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 30) / 100);
						break;
					}
					case 2:
					{
						arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 40) / 100);
						arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 30) / 100);
						break;
					}
					case 3:
					{
						arg1->Mobs.Player.Status.STR = arg1->Mobs.Player.Status.STR + ((arg1->Mobs.Player.Status.STR * 40) / 100);
						arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 30) / 100);
						break;
					}
					case 4:
					{
						arg1->Mobs.Player.Status.INT = arg1->Mobs.Player.Status.INT + ((arg1->Mobs.Player.Status.INT * 40) / 100);
						arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 30) / 100);
						break;
					}
					case 5:
					{
						// CON E DEX??
						arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.CON * 40) / 100);
						arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 30) / 100);
						break;
					}
					case 6:
					{
						arg1->Mobs.Player.Status.STR = arg1->Mobs.Player.Status.STR + ((arg1->Mobs.Player.Status.STR * 60) / 100);
						break;
					}
					case 7:
					{
						arg1->Mobs.Player.Status.INT = arg1->Mobs.Player.Status.INT + ((arg1->Mobs.Player.Status.INT * 60) / 100);
						break;
					}

					case 8:
					{
						arg1->Mobs.Player.Status.CON = arg1->Mobs.Player.Status.CON + ((arg1->Mobs.Player.Status.CON * 60) / 100);
						break;
					}

					case 9:
					{
						arg1->Mobs.Player.Status.DEX = arg1->Mobs.Player.Status.DEX + ((arg1->Mobs.Player.Status.DEX * 60) / 100);
						break;
					}
					}
				}
				else
				{
					arg1->Mobs.Player.Status.STR = 2000;
					arg1->Mobs.Player.Status.INT = 2000;
					LOCAL_3 = 10000;
				}
				continue;
			}
		}

		if (LOCAL_46 == 31)
		{
			int LOCAL_95 = (LOCAL_48 / 10) + LOCAL_47;
			if (arg1->Mobs.Player.Learn & 0x8000)
				LOCAL_95 += 100;

			arg1->Mobs.Player.Status.Defense += LOCAL_95;
			continue;
		}

		if (LOCAL_46 == 33)
		{
			int LOCAL_96 = 0;

			if (LOCAL_47 == 0)
				LOCAL_96 = 202;

			if (LOCAL_47 == 1)
				LOCAL_96 = 209;

			if (LOCAL_47 == 2)
				LOCAL_96 = 212;

			if (LOCAL_47 == 3)
				LOCAL_96 = 230;

			if (LOCAL_47 == 4)
				LOCAL_96 = 229;

			if (LOCAL_47 == 5)
				LOCAL_96 = 216;

			if (LOCAL_47 == 6)
				LOCAL_96 = 226;

			if (LOCAL_47 == 7)
				LOCAL_96 = 298;

			arg1->Mobs.Player.Equip[0].Index = LOCAL_96;
			continue;
		}

		if (LOCAL_46 == 34)
		{
			LOCAL_36 += 15;
			LOCAL_2 += 80;
			LOCAL_1 += 20;
			LOCAL_38 += 30;
			LOCAL_39 += 30;
			continue;
		}

		if (LOCAL_46 == 35)
		{
			if (arg2[LOCAL_45].Time <= 0xF4240)
			{
				LOCAL_38 += 30;
				LOCAL_39 += 30;
			}
			else
			{
				LOCAL_38 += 10;
				LOCAL_39 += 10;
			}
			continue;
		}

		if (LOCAL_46 == 36)
		{
			int LOCAL_97 = *arg10;
			*arg10 = LOCAL_97 | 0x10;
			continue;
		}

		if (LOCAL_46 == 37)
		{
			*arg8 = ((arg1->Mobs.Player.Status.Mastery[2] / 10) * 3) + 24;

			continue;
		}

		if (LOCAL_46 == 38)
		{
			int LOCAL_98 = arg1->Mobs.Player.Status.Level;

			if (func_4012DA(arg5))
			{
				if (arg1->Mobs.Player.Learn & 0x40000000)
					LOCAL_98 += 400;

				int LOCAL_99 = (arg1->Mobs.Player.Status.maxMP >> 2) + ((arg1->Mobs.Player.Status.Mastery[2] + LOCAL_98) >> 2);

				LOCAL_3 = LOCAL_3 + LOCAL_99 + 750;
				arg1->Mobs.Player.Status.maxMP -= LOCAL_99;
			}
			else
			{
				// MAX MP
				// mastery[2]
				int LOCAL_100 = (arg1->Mobs.Player.Status.maxMP >> 3) + ((arg1->Mobs.Player.Status.Mastery[2] + LOCAL_98) >> 2);

				LOCAL_3 = LOCAL_3 + LOCAL_100 + 750;
				arg1->Mobs.Player.Status.maxMP -= LOCAL_100;
				continue;
			}
		}
	}
}*/

int func_4012DA(int arg)
{
	int local1 = 0;
	if ((arg / 10) > 5)
		return 1;

	return 0;
}
// ----
// Retorna um número aleatório
// ----
int Rand()
{
	static bool started = false;
	if (!started)
	{
		std::srand(std::time(nullptr));

		started = true;
	}

	if (sServer.NewRandomMode)
		return std::rand();

	static long long i = 115;
	i = ((i * 214013) + 253101111);

	return ((i >> 16) & 0x7FFF);
}

// ----
// Lê o arquivo de configuração com o IP/Porta de conexão
// ----
bool LoadConfig()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "serverip.txt", "rt");
	if (!pFile)
		return false;

	char szTMP[256];
	while (fgets(szTMP, 256, pFile))
	{
		if (*szTMP == '#' || *szTMP == '\n')
			continue;

		char cmd[32], val[32];
		int ret = sscanf_s(szTMP, "%[^=]=%[^\n]", cmd, 32, val, 32);
		if (ret != 2)
			continue;

		if (!strcmp(cmd, "Porta"))
			sServer.Config.Porta = atoi(val);
		else if (!strcmp(cmd, "IP"))
			strncpy_s(sServer.Config.IP, 32, val, 32);
	}

	fclose(pFile);
	return true;
}

bool LoadDataServer()
{
	char line[1024];
	FILE *hFile = NULL;

	fopen_s(&hFile, "dataserver.txt", "r");
	
	if (!hFile)
		return false;

	while (fgets(line, sizeof(line), hFile))
	{
		if (*line == '#' || *line == '\n')
			continue;

		char cmd[32], val[32];
		int ret = sscanf_s(line, "%[^=]=%[^\n]", cmd, 32, val, 32);

		if (ret != 2)
			continue;

		if (!strcmp(cmd, "Porta"))
			sServer.Data.Porta = atoi(val);
		else if (!strcmp(cmd, "IP"))
			strncpy_s(sServer.Data.IP, 16, val, 16);
	}

	fclose(hFile);
	return true;
}

bool ReadInitItem()
{
	FILE *pFile = NULL;
	char line[1024];

	fopen_s(&pFile, "InitItem.csv", "r");
	if(pFile)
	{
		while (1)
		{
			char *ret = fgets(line, 1024, pFile);

			if (ret == 0)
				break;

			for (int i = 0; i < 1024; i++)
				if (line[i] == ',')
					line[i] = ' ';

			int index = -1;
			int posX = 0;
			int posY = 0;
			int rotate = 0;

			sscanf_s(line, "%d %d %d %d", &index, &posX, &posY, &rotate);

			if (index == -1)
				continue;

			if(index == 773)
				index = index;

			g_pInitItem[sServer.InitCount].Index = index;
			g_pInitItem[sServer.InitCount].PosX = posX;
			g_pInitItem[sServer.InitCount].PosY = posY;
			g_pInitItem[sServer.InitCount].Rotate = rotate;
			sServer.InitCount++;
		}

		fclose(pFile);
		return true;
	}
	
	return false;
}

bool ReadGuild()
{
	FILE *fp; 
	fopen_s(&fp, "guild_zone.txt", "rt");

    if(fp == NULL)
	{
		memset(g_pCityZone, 0, sizeof(g_pCityZone));
        return false;
    }

    int zone_index, ret;
    sGuildZone zone;
	char tmp[1024];

    while(fgets(tmp, sizeof(tmp), fp))
    {
        if(tmp[0] == '\n' || tmp[0] == '#')
            continue;

        zone_index = -1;
        memset(&zone, 0, sizeof(zone));

		ret = sscanf_s(tmp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%I64d,%d,%d",
				&zone_index, &zone.owner_index, &zone.chall_index, &zone.chall_index_2, &zone.tower_x, &zone.tower_y, &zone.area_guild_x,
				&zone.area_guild_y, &zone.city_x, &zone.city_y, &zone.city_min_x,
				&zone.city_min_y, &zone.city_max_x, &zone.city_max_y, &zone.area_guild_min_x,
				&zone.area_guild_min_y, &zone.area_guild_max_x, &zone.area_guild_max_y, &zone.war_min_x,
				&zone.war_min_y, &zone.war_max_x, &zone.war_max_y, &zone.guilda_war_x,
				&zone.guilda_war_y, &zone.guildb_war_x, &zone.guildb_war_y, &zone.impost, &zone.perc_impost, &zone.win_count);

		if (zone_index >= 0 && zone_index < 5)
		{
			memcpy(&g_pCityZone[zone_index], &zone, sizeof(sGuildZone));

			if (sServer.Channel >= 1 && sServer.Channel <= 10)
				ChargedGuildList[sServer.Channel - 1][zone_index] = zone.owner_index;
		}
    }

	fclose(fp);
	return true;
}

bool LoadGuild()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "Data\\guilds.txt", "rt");

	if (!pFile)
	{
		MessageBox(NULL, "Can't open guilds.txt", "-system", MB_OK | MB_APPLMODAL);
		return false;
	}
	//memset(g_pGuild, 0, sizeof g_pGuild);
	char str[256];
	while(true)
	{
		char *ret = fgets(str, 255, pFile);
		if (ret == NULL)
			break;

		if(str[0] == '\n' || str[0] == '#')
			continue;

		INT32 fameGuild = 0,
			  guildId   = 0,
			  kingdom   = 0,
			  citizen   = 0,
			  wins		= 0;

		char guildName[32] = { 0, },
			 subName  [ 3][32];

		for(INT32 i = 0; i < 3; i++)
			memset(subName, 0, 32 * 3);

		memset(guildName, 0, 32);

		INT32 rtn = sscanf_s(str, "%d, %d, %d, %d, %d, %[^,], %[^,], %[^,], %[^,]", &guildId, &fameGuild, &kingdom, &citizen, &wins,
			guildName, 16, subName[0], 16, subName[1], 16,subName[2], 16);

		if(rtn < 5)
		{
			MessageBoxA(NULL, "Can't parse strings on Guilds.txt", NULL, MB_OK);
			continue;
		}

		if(guildId < 0 || guildId >= MAX_GUILD)
			continue;

		if(fameGuild < 0)
			fameGuild = 0;

		g_pGuild[guildId].Citizen = citizen;
		g_pGuild[guildId].Fame    = fameGuild;
		g_pGuild[guildId].Kingdom = kingdom;
		g_pGuild[guildId].Wins	  = wins;
		
		g_pGuild[guildId].Name = std::string(guildName);

		for(INT32 i = 0; i < 3; i++)
		{
			if(!subName[i][0])
				continue;

			g_pGuild[guildId].SubGuild[i] = std::string(subName[i]);
		}
	}

	fclose(pFile);
	return true;
}

void ApplyAttribute(char *pHeight, int size)
{
	int endx = g_HeightPosX + size;
	int endy = g_HeightPosY + size;

	int xx = 0;
	int yy = 0;

	for(int y = g_HeightPosY; y < endy; y++)
	{
		for(int x = g_HeightPosX; x < endx; x++)
		{
			xx = (x >> 2) & 0x3FF;
			yy = (y >> 2) & 0x3FF;

			MapAttribute att = g_pAttributeMap[yy][xx];

			if (att.Value & 2)
				pHeight[x + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX] = 127;
		}
	}
}

bool ReadHeightMap()
{
	int Handle;
	_sopen_s(&Handle, "./heightmap.dat", _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	if (Handle == -1)
		return false;

	_read(Handle, (void*)g_pHeightGrid, sizeof(g_pHeightGrid));
	_close(Handle);
	return true;
}

bool LoadQuiz()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "Data\\Quiz.txt", "rt");

	if (!pFile)
		return false;

	TOD_QuizQuestions* last = nullptr;

	char szTMP[256] = { 0 };
	while (fgets(szTMP, 256, pFile))
	{
		if (*szTMP == '\n')
			continue;

		if (*szTMP == '#')
		{
			sServer.QuizQuestions.push_back(TOD_QuizQuestions{});
			last = &sServer.QuizQuestions.back();
		}

		char cmd[128] = { 0 }, val[128] = { 0 };
		int ret = sscanf_s(szTMP, "%[^=]=%[^\n]", cmd, 127, val, 127);
		if (ret != 2)
			continue;

		if (last == nullptr)
			continue;

		std::string command{ cmd };

		if (command == "Question")
			last->Question = val;
		else
		{
			int index;
			if (sscanf_s(cmd, "Answer%d", &index) != 1)
				continue;

			if (index >= 0 && index < 4)
				last->Answers[index] = val;
		}
	}

	fclose(pFile);
	return true;
}


bool ReadAttributeMap()
{
    FILE *pFile = NULL; 
	
	fopen_s(&pFile, "AttributeMap.dat", "rb");

    if(!pFile)
		return false;

    fread(g_pAttributeMap, 1024, 1024, pFile);
    fclose(pFile);

    return true;
}

bool ReadGameConfigv2()
{
	const std::string filename = "gameconfig_v2.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	auto configNode = doc.child("configs");

	{
		auto generalNode = configNode.child("general");

		if (generalNode != nullptr)
		{
			sServer.RvR.Hour = generalNode.attribute("rvrHour").as_int();
			sServer.TowerWar.Hour = generalNode.attribute("towerHour").as_int();
			sServer.WeekHour = generalNode.attribute("cityHour").as_int();
			sServer.CastleHour = generalNode.attribute("noatunHour").as_int();
			sServer.MaxWaterEntrance = generalNode.attribute("maxWater").as_int();

			if (generalNode.child_value("cliver") != nullptr)
			{
				if(!std::string(generalNode.child_value("cliver")).empty())
					sServer.CliVer = std::stoi(generalNode.child_value("cliver"));
			}
		}
	}

	{
		auto kefra = configNode.child("kefra");

		sServer.FirstKefra = kefra.attribute("firstKefra").as_int();
		sServer.KefraKiller = kefra.attribute("kefraKiller").as_int();
		sServer.KefraDead = kefra.attribute("kefraDead").as_int();
	}

	{
		auto kingdomBattle = configNode.child("kingdomBattle");
		if (kingdomBattle != nullptr)
			sServer.KingdomBattle.Winner = kingdomBattle.attribute("winner").as_int();
	}

	{
		auto tower = configNode.child("towerWar");
		if (tower != nullptr)
			sServer.TowerWar.Guild = tower.attribute("guild").as_int();
	}

	{
		auto lanhouse = configNode.child("lanhouseN");
		if (lanhouse != nullptr)
			sServer.LanHouseN.TotalToKill = lanhouse.attribute("totalToKill").as_int();
	}

	{
		auto lanhouse = configNode.child("lanhouseM");
		if (lanhouse != nullptr)
			sServer.LanHouseM.TotalToKill = lanhouse.attribute("totalToKill").as_int();
	}

	{
		auto pesaLevel = configNode.child("nightmare");
		if (pesaLevel != nullptr)
			sServer.MaximumPesaLevel = pesaLevel.attribute("maximumPesaLevel").as_int();
	}

	return true;
}

bool WriteGameConfigv2()
{
	pugi::xml_document doc;
	auto configNode = doc.append_child("configs");

	{
		auto generalNode = configNode.append_child("general");

		generalNode.append_attribute("rvrHour").set_value(sServer.RvR.Hour);
		generalNode.append_attribute("towerHour").set_value(sServer.TowerWar.Hour);
		generalNode.append_attribute("noatunHour").set_value(sServer.CastleHour);
		generalNode.append_attribute("cityHour").set_value(sServer.WeekHour);
		generalNode.append_attribute("maxWater").set_value(sServer.MaxWaterEntrance);

		generalNode.append_child("cliver").append_child(pugi::node_pcdata).set_value(std::to_string(sServer.CliVer).c_str());

	}

	{
		auto kefra = configNode.append_child("kefra");

		kefra.append_attribute("firstKefra").set_value(sServer.FirstKefra);
		kefra.append_attribute("kefraKiller").set_value(sServer.KefraKiller);
		kefra.append_attribute("kefraDead").set_value(sServer.KefraDead);
	}

	{
		auto kingdomBattle = configNode.append_child("kingdomBattle");
		kingdomBattle.append_attribute("winner").set_value(sServer.KingdomBattle.Winner);
	}

	{
		auto warTower = configNode.append_child("towerWar");
		warTower.append_attribute("guild").set_value(sServer.TowerWar.Guild);
	}

	{
		auto lanhouse = configNode.append_child("lanhouseN");
		lanhouse.append_attribute("totalToKill").set_value(sServer.LanHouseN.TotalToKill);
	}

	{
		auto lanhouse = configNode.append_child("lanhouseM");
		lanhouse.append_attribute("totalToKill").set_value(sServer.LanHouseM.TotalToKill);
	}

	{
		auto pesaLevel = configNode.append_child("nightmare");
		pesaLevel.append_attribute("maximumPesaLevel").set_value(sServer.MaximumPesaLevel);
	}

	doc.save_file("gameconfig_v2.xml");
	return true;
}

bool ReadGameConfig()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "gameconfig.txt", "r");
	if(pFile)
	{
		bool modeDrop = false;
		bool modeBonusExp = false;
		bool modeRef = false;
		INT32 mode = -1;
		int totalDrop = 0;

		char line[1024];
		while(fgets(line, 1024, pFile))
		{
			if(*line == '#' || *line == '\n')
				continue;

			char cmd[1024];

			int ret = sscanf_s(line, "%[^\n]", cmd, 1024);
			if(!modeDrop && !modeBonusExp && !modeRef)
			{
				if(!strcmp(cmd, "Game server drop"))
					modeDrop = true;
				else if(!strcmp(cmd, "Bônus experiência"))
					modeBonusExp = true;
				else if(!strcmp(cmd, "Rates Refinação Abençoada"))
					modeRef = true;
				else if(!strncmp(cmd, "NewbieZone", 10))
				{
					int level = 0;
					sscanf_s(line, "%*[^=]=%d", &level);
					sServer.NewbieZone = level;
				}
				else if(!strncmp(cmd, "Channel", 7))
				{
					int level = 0;
					sscanf_s(line, "%*[^=]=%d", &level);
					sServer.Channel = level;
				}
				else if(!strncmp(cmd, "TotalChannel", 12))
				{
					int level = 0;
					sscanf_s(line, "%*[^=]=%d", &level);
					sServer.TotalServer = level;
				}
				else if(!strncmp(cmd, "GuildHour", 9))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.GuildHour = aux;
				}
				else if(!strncmp(cmd, "GuildDay", 10))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.GuildDay = aux;
				}
				
				else if(!strncmp(cmd, "NewbieHour", 10))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.NewbieHour = aux;
				}
				else if(!strncmp(cmd, "Coloseum", 8))
				{
					int aux = 0 , aux2 = 0;
					sscanf_s(line, "%*[^=]=%d %d", &aux, &aux2);
					sServer.BRItem = aux;
					sServer.BRHour = aux2;
				}
				else if(!strncmp(cmd, "StatSapphire", 10))
				{
					int aux = 0;
					sscanf_s(line, "%*[^=]=%d", &aux);
					sServer.StatSapphire = aux;
				}
			}
			else
			{
				if(modeDrop)
				{
					int value[10];
					int ret = sscanf_s(line, "%d %d %d %d %d %d %d %d %d %d", &value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7], &value[8], &value[9]);

					for(int i = totalDrop, x = 0; i < (totalDrop + ret) || x < 10; i++, x++)
					{
						if(i > 63)
							break;

						Taxes[i] = value[x];
					}
					totalDrop += ret;

					if(totalDrop >= 63)
					{
						int total = 0;
						for(int i = 0 ; i < 64; i++)
							total += Taxes[i];

						total /= 64;

						modeDrop = false;
					}
				}
				else if(modeBonusExp)
				{
					int val = 0;
					int ret =  sscanf_s(line, "%d", &val);

					if(ret != 1)
					{
						modeBonusExp = false;
						continue;
					}

					sServer.BonusEXP = val;

					modeBonusExp = false;
				}
				else if(modeRef)
				{
					int index = 0, ref = 0;
					int ret = sscanf_s(line, "%d=%d", &index, &ref);

					sServer.RateRef[index] = ref;

					if(index == 4)
						modeRef = false;
				}
			}
		}

		if(sServer.TotalServer == 0)
			sServer.TotalServer = 1;

		fclose(pFile);
		return true;
	}
	
	if(sServer.TotalServer == 0)
		sServer.TotalServer = 1;

	return false;
}

bool ReadNPCBase()
{
    static const char* npc_name[39] = {
        "Condor", "Javali", "Lobo", "Urso", "Tigre",
        "Gorila", "Dragao_Negro", "Succubus", "", "", "Porco",
        "Javali_", "Lobo_", "Dragao_Menor", "Urso_",
        "Dente_de_Sabre", "Sem_Sela", "Fantasma", "Leve",
        "Equipado", "Andaluz", "Sem_Sela_", "Fantasma_",
        "Leve_", "Equipado_", "Andaluz_", "Fenrir", "Dragao",
        "Grande_Fenrir", "Tigre_de_Fogo", "Dragao_Vermelho",
        "Unicornio", "Pegasus", "Unisus", "Grifo", "Hippo_Grifo",
        "Grifo_Sangrento", "Sleipnir", "Svadilfari"
    };
	
	for(int i = 0 ; i < 39; i++) 
	{
		if(!*npc_name[i])
			continue;

		FILE *pFile = NULL;
		
		char szTMP[1024];
		sprintf_s(szTMP, "npc_base/%s", npc_name[i]);

		fopen_s(&pFile, szTMP, "rb");

		if(pFile) 
		{
			fread(&NPCBase[i], 1, sizeof st_Mob, pFile);

			fclose(pFile);
		}
		else 
		{
			sprintf_s(szTMP, "Falha ao ler arquivo: %s", npc_name[i]);

			MessageBoxA(NULL, szTMP, szTMP, MB_OK);
		}
	}

	return true;
}

bool SaveGuildZone()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "guild_zone.txt", "w+");

	if (pFile)
	{
		for(INT32 i = 0; i < 5; i++)
		{
			sGuildZone *zone= &g_pCityZone[i]; 

			fprintf(pFile, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%I64d,%d,%d\n",
				i, ChargedGuildList[sServer.Channel - 1][i], zone->chall_index, zone->chall_index_2, zone->tower_x, zone->tower_y, zone->area_guild_x,
				zone->area_guild_y, zone->city_x, zone->city_y, zone->city_min_x,
				zone->city_min_y, zone->city_max_x, zone->city_max_y, zone->area_guild_min_x,
				zone->area_guild_min_y, zone->area_guild_max_x, zone->area_guild_max_y, zone->war_min_x,
				zone->war_min_y, zone->war_max_x, zone->war_max_y, zone->guilda_war_x,
				zone->guilda_war_y, zone->guildb_war_x, zone->guildb_war_y, zone->impost, zone->perc_impost, zone->win_count);
		}

		fclose(pFile);
		return true;
	}

	return false;
}

int GetEmptyUser()
{
	for (int i = 1; i < MAX_PLAYER; i++)
	{
		if (Users[i].Status == USER_EMPTY)
			return i;
	}

	return NULL;
}

CUser *GetUserBySocket(DWORD socket)
{
	for (int i = 1; i < MAX_PLAYER; i++)
	{
		if (Users[i].Socket.Socket == socket)
			return &Users[i];
	}

	return NULL;
}

bool SendClientMessage(int clientId, const char *msg, ...)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return false;

	/* Arglist */
	char buffer[256];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */

	p101 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.ClientId = 0;
	packet.Header.PacketId = 0x101;
	buffer[127] = '\0';

	strncpy_s(packet.Msg, buffer, 128);

	if(buffer[0] != '.')
		Log(clientId, LOG_INGAME, "[Mensagem do servidor]> %s", buffer);

	return Users[clientId].AddMessage((BYTE*)&packet, sizeof packet);
}

void GridMulticast_2(short posX, short posY, BYTE *sendPak, int Index)
{
	int VisX = VIEWGRIDX, VisY = VIEWGRIDY,
		minPosX = (posX - HALFGRIDX),
		minPosY = (posY - HALFGRIDY);

	if((minPosX + VisX) >= 4096)
		VisX = (VisX - (VisX + minPosX - 4096));

	if((minPosY + VisY) >= 4096)
		VisY = (VisY - (VisY + minPosY - 4096));

    if(minPosX < 0)
	{
		minPosX = 0;
		VisX = (VisX + minPosX);
	}

	if(minPosY < 0)
	{
		minPosY = 0;
		VisY = (VisY + minPosY);
	}

    int maxPosX = (minPosX + VisX),
        maxPosY = (minPosY + VisY);

    for(int nY = minPosY; nY < maxPosY; nY++)
    {
        for(int nX = minPosX; nX < maxPosX; nX++)
        {
            short mobID = g_pMobGrid[nY][nX];
            if(mobID <= 0 || Index == mobID)
                continue;

            if(sendPak == NULL || mobID >= MAX_PLAYER)
				continue;

			if (Mob[mobID].Mode == 0)
				continue;

			if(*(WORD*)&sendPak[4] == 0x338)
			{
				*(INT64*)&sendPak[24] = Mob[mobID].Mobs.Player.Exp;
				*(UINT32*)&sendPak[12] = static_cast<int>(Mob[mobID].Mobs.Hold);
			}

			Users[mobID].AddMessage(sendPak, *(short*)&sendPak[0]);
        }
    }
}

void GridMulticast(int Index, unsigned int posX, unsigned int posY, BYTE *buf)
{
    if(Index <= 0 || Index >= MAX_SPAWN_MOB)
        return ;

	if(Index == 0 || Mob[Index].Target.X == 0)
		return;
	
	CMob *mob = (CMob*)&Mob[Index];

	INT32 mobId = g_pMobGrid[mob->Target.Y][mob->Target.X];

	if(mobId == Index && mobId != 0)
		g_pMobGrid[mob->Target.Y][mob->Target.X] = 0;

	if (g_pMobGrid[posY][posX] != Index && g_pMobGrid[posY][posX] != 0)
		GetEmptyMobGrid(Index, &posX, &posY);

	if (g_pMobGrid[posY][posX] != Index && g_pMobGrid[posY][posX] != 0)
	{
		if (Index < MAX_PLAYER)
			Log(Index, LOG_INGAME, "PC step in other mob's grid", "-system", 0);
	}

	g_pMobGrid[posY][posX] = Index;
		
	int VisX = VIEWGRIDX, VisY = VIEWGRIDY,
        minPosX = (mob->Target.X - HALFGRIDX),
		minPosY = (mob->Target.Y - HALFGRIDY);


	if((minPosX + VisX) >= 4096)
		VisX = (VisX - (VisX + minPosX - 4096));

	if((minPosY + VisY) >= 4096)
		VisY = (VisY - (VisY + minPosY - 4096));

    if(minPosX < 0)
	{
		minPosX = 0;
		VisX = (VisX + minPosX);
	}

	if(minPosY < 0)
	{
		minPosY = 0;
		VisY = (VisY + minPosY);
	}

    int maxPosX = (minPosX + VisX),
        maxPosY = (minPosY + VisY);

	int dVisX = VIEWGRIDX, dVisY = VIEWGRIDY,
		dminPosX = (posX - HALFGRIDX),
		dminPosY = (posY - HALFGRIDY);

	if((dminPosX + dVisX) >= 4096)
		dVisX = (dVisX - (dVisX + dminPosX - 4096));

	if((dminPosY + dVisY) >= 4096)
		dVisY = (dVisY - (dVisY + dminPosY - 4096));

    if(dminPosX < 0)
	{
		dminPosX = 0;
		dVisX = (dVisX + dminPosX);
	}

	if(dminPosY < 0)
	{
		dminPosY = 0;
		dVisY = (dVisY + dminPosY);
	}

    int dmaxPosX = (dminPosX + dVisX),
        dmaxPosY = (dminPosY + dVisY);

    for(int nY = minPosY; nY < maxPosY; nY++)
    {
        for(int nX = minPosX; nX < maxPosX; nX++)
        {
            short mobID = g_pMobGrid[nY][nX];
			if (mobID > 0 && Index != mobID)
			{
				if (buf != NULL && mobID < MAX_PLAYER)
					Users[mobID].AddMessage(buf, *(short*)&buf[0]);

				if (nX < dminPosX || nX >= dmaxPosX ||
					nY < dminPosY || nY >= dmaxPosY)
				{
					if (mobID < MAX_PLAYER)
						SendRemoveMob(mobID, Index, 0, 0); //SendSignalParm(mobID, Index, 0x165, 0);

					if (Index < MAX_PLAYER)
						SendRemoveMob(Index, mobID, 0, 0);
				}
			}

			WORD item = g_pItemGrid[nY][nX];
			if (item != 0)
			{
				if (nX < dminPosX || nX >= dmaxPosX ||
					nY < dminPosY || nY >= dmaxPosY)
				{
					if (item >= 0 && item < 4096 && Index < MAX_PLAYER)
						SendRemoveItem(Index, item, 0);
				}
			}
        }
    }

    for(int nY = dminPosY; nY < dmaxPosY; nY++)
    {
        for(int nX = dminPosX; nX < dmaxPosX; nX++)
        {
            short mobID = g_pMobGrid[nY][nX];
            short initID = g_pItemGrid[nY][nX];

            if(nX < minPosX || nX >= maxPosX ||
			   nY < minPosY || nY >= maxPosY)
            {
                if(mobID > 0 && Index != mobID)
                {
					if(Mob[mobID].Mode == 0)
					{
						g_pMobGrid[nY][nX] = 0;

						Log(SERVER_SIDE, LOG_ERROR, "MOB GRID HAS EMPTY MOB - %s %dx %dy", Mob[mobID].Mobs.Player.Name, Mob[mobID].Target.X, Mob[mobID].Target.Y);
						continue;
					}

                    if(Index < MAX_PLAYER)
						SendCreateMob(Index, mobID);

                    if(mobID < MAX_PLAYER)
						SendCreateMob(mobID, Index);
			
                    if(buf != NULL && mobID < MAX_PLAYER)
						Users[mobID].AddMessage(buf, *(short*)&buf[0]);
                }
				
				if(initID > 0 && Index > 0 && Index < MAX_PLAYER)
					SendCreateItem(Index, initID, 0);
				
				if(initID > 0 && mobID > 0 && mobID < MAX_PLAYER)
					SendCreateItem(mobID, initID, 0);
            }
        }
	}
	
	p36C *LOCAL_85 = (p36C*)buf;
//	memset(LOCAL_85, 0, sizeof p36C);
	
	mob->Last.Time = LOCAL_85->Header.TimeStamp;
	mob->Last.Speed = LOCAL_85->MoveSpeed;
	mob->Last.X = LOCAL_85->LastPos.X;
	mob->Last.Y = LOCAL_85->LastPos.Y;

    mob->Target.X = posX;
    mob->Target.Y = posY;
}

void Teleportar(int clientId, unsigned int posX, unsigned int posY)
{
	INT32 LOCAL_1 = GetEmptyMobGrid(clientId, &posX, &posY);
	if (!LOCAL_1)
	{
		if (clientId < MAX_PLAYER)
			Log(clientId, LOG_INGAME, "Falha ao encontrar espaço vago no mapa para o usuário. Posição: %ux %uy", posX, posY);

		return;
	}

	p36C packet{};
	GetAction(clientId, posX, posY, &packet);

	packet.MoveType = 1;

	if (clientId < MAX_PLAYER)
	{
		Users[clientId].AddMessage((BYTE*)&packet, sizeof packet);
		Log(clientId, LOG_INGAME, "Teleportado para a posição %u %u", posX, posY);
	}

	GridMulticast(clientId, posX, posY, (BYTE*)&packet);
}

short get_effect_index(const char *s)
{
    int i;
    for(i = 0; i < 256; i++)
        if(!strcmp(EffectName[i], s))
            return i;

    return atoi(s);
}

bool ReadItemEffect()
{
	memset(EffectName, 0, sizeof(EffectName));
	FILE *fp;
	fopen_s(&fp, "ItemEffect.h", "rt");
    if(fp == NULL)
        return false;

    int ret, index;
    const char *cmm = "#define";

    char line[1024];
    char val[64];

    while(fgets(line, sizeof(line), fp))
    {
        if(strncmp(line, cmm, strlen(cmm)) == 0)
        {
            index = -1;
            *val = '\0';
            ret = sscanf_s(line, "#define %s %d", val, 31, &index);
            if(ret != 2)
                continue;

            if(index >= 0 && index < 256)
				strcpy_s(EffectName[index], val);
        }
    }

    fclose(fp);
    return true;
}

bool ReadItemList()
{
    FILE *fp = NULL;
	fopen_s(&fp, "ItemList.csv", "rt");

    if(fp == NULL)
    {
        memset(ItemList, 0, sizeof(ItemList));
        return false;
    }

	if(!ReadItemEffect())
	{
		if(fp)
			fclose(fp);

		return false;
	}

    int itemID, ret;
    sItemData item;

    char line[1024];

    memset(ItemList, 0, sizeof(ItemList));
    while(fgets(line, sizeof(line), fp))
    {
        char meshBuf[MAX_MESH_BUFFER];
		char scoreBuf[MAX_SCORE_BUFFER] = { 0, };
        char effBuf[MAX_EFFECT][MAX_EFFECT_NAME];

        if(*line == '\n' || *line == '#')
            continue;

        memset(effBuf, 0, sizeof(effBuf));
        memset(meshBuf, 0, sizeof(meshBuf));
        memset(scoreBuf, 0, sizeof(scoreBuf));
        memset(&item, 0, sizeof(sItemData));

        char *p = line;
        while(*p != '\0')
        {
            if(*p == ',')
                *p = ' ';
            p++;
        }

        ret = sscanf_s(line, "%d %s %s %s %hd %d %hd %hd %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd %s %hd",
                     &itemID, item.Name, 63, meshBuf, MAX_MESH_BUFFER, scoreBuf, MAX_SCORE_BUFFER, &item.Unique, &item.Price, &item.Pos, &item.Extreme, &item.Grade,
                     effBuf[ 0], MAX_EFFECT_NAME, &item.Effect[ 0].Value, effBuf[ 1], MAX_EFFECT_NAME, &item.Effect[ 1].Value, effBuf[ 2], MAX_EFFECT_NAME, &item.Effect[ 2].Value,
                     effBuf[ 3], MAX_EFFECT_NAME, &item.Effect[ 3].Value, effBuf[ 4], MAX_EFFECT_NAME, &item.Effect[ 4].Value, effBuf[ 5], MAX_EFFECT_NAME, &item.Effect[ 5].Value,
                     effBuf[ 6], MAX_EFFECT_NAME, &item.Effect[ 6].Value, effBuf[ 7], MAX_EFFECT_NAME, &item.Effect[ 7].Value, effBuf[ 8], MAX_EFFECT_NAME, &item.Effect[ 8].Value,
                     effBuf[ 9], MAX_EFFECT_NAME, &item.Effect[ 9].Value, effBuf[10], MAX_EFFECT_NAME, &item.Effect[10].Value, effBuf[11], MAX_EFFECT_NAME, &item.Effect[11].Value);

        if(ret < 9 || itemID <= 0 || itemID >= MAX_ITEMLIST)
            continue;

        sscanf_s(meshBuf, "%hd.%d", &item.Mesh1, &item.Mesh2);
        sscanf_s(scoreBuf, "%hd.%hd.%hd.%hd.%hd", &item.Level, &item.Str, &item.Int, &item.Dex, &item.Con);

        int i;
        for(i = 0; i < MAX_EFFECT; i++)
            item.Effect[i].Index = get_effect_index(effBuf[i]);

        memcpy(&ItemList[itemID], &item, sizeof(sItemData));
    }

    fclose(fp);
    return true;
}

bool ReadSkillData()
{
	FILE *fp;
	fopen_s(&fp, "SkillData.csv", "rt");

	if(fp == NULL)
	{
		memset(SkillData, 0, sizeof(SkillData));
		return false;
	}

	char line[1024];
	while(fgets(line, sizeof(line), fp))
	{
		int index;
		sSpellData spell;

		if(*line == '\n' || *line == '#')
			continue;

		index = -1;
		memset(&spell, 0, sizeof(spell));

		char *p = line;
		while(*p != '\0')
		{
			if(*p == ',')
				*p = ' ';
			p++;
		}

		int ret = sscanf_s(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %*s %*s %d %d %d %d %d %d %d %d",
				&index, &spell.Points, &spell.Target, &spell.Mana, &spell.Delay, &spell.Range,
				&spell.InstanceType, &spell.InstanceValue, &spell.TickType, &spell.TickValue,
				&spell.AffectType, &spell.AffectValue, &spell.Time, &spell.InstanceAttribute,
				&spell.TickAttribute, &spell.Aggressive, &spell.Maxtarget, &spell.PartyCheck,
				&spell.AffectResist, &spell.Passive_Check, &spell.ForceDamage);

		if(ret < 19 || index < 0 || index >= 256)
			continue;

		//spell.Time >>= 2;

		memcpy(&SkillData[index], &spell, sizeof(sSpellData));
	}

	fclose(fp);
	return true;
}

bool ReadTeleport()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Teleport.txt", "r");
	if(pFile)
	{
		int index = 0;
		char line[1024];
		while(fgets(line, 1024, pFile))
		{
			if(*line == '#' || *line == '\n')
				continue;

			int value[5] = {0, 0, 0, 0, 0};
			int ret = sscanf_s(line, "%d, %d, %d, %d, %d", &value[0], &value[1], &value[2], &value[3], &value[4]);

			if(ret != 5)
				continue;

			stTeleport *m = &mTeleport[index];
			m->Price = value[4];

			m->SrcPos.X = value[0];
			m->SrcPos.Y = value[1];

			m->DestPos.X = value[2];
			m->DestPos.Y = value[3];

			index++;

			if(index == MAX_TELEPORT)
				break;
		}

		fclose(pFile);
		return true;
	}

	return false;
}

bool CanEquip(st_Item *pItem, st_Mob *mob, int pSlot, int pClass, p376 *p, int classMaster)
{
    if(pItem->Index <= 0 || pItem->Index >= 6500)
    { // Verifica se o id do item eh valido
        return false;
    }
	
    if(pSlot == 15)
    { // Verifica se o client esta tentando trocar a capa
        return false;
    }

    short ItemUnique = ItemList[pItem->Index].Unique;
    if(pSlot != -1)
    { // Verifica o slot do item
        short ItemPos = GetItemAbility(pItem, EF_POS);
        if(((ItemPos >> pSlot) & 1) == 0)
             //return false; // Verifica se pode mover o item para este slot

        if(pSlot == 6 || pSlot == 7)
        { // Slot das armas/escudos
            int SrcSlot;
            if(pSlot == 6)
                SrcSlot = 7;
            else // pSlot = 7
                SrcSlot = 6;

			short SrcItemID = mob->Equip[SrcSlot].Index;
            if(SrcItemID > 0 && SrcItemID < 6500)
            { // Verifica o id da outra arma
                short SrcUnique = ItemList[SrcItemID].Unique;
				short SrcPos = GetItemAbility(&mob->Equip[SrcSlot], EF_POS);

                if(ItemPos == 64 || SrcPos == 64)
                { // Verifica se a arma usada eh de 2 maos

                    if(ItemUnique == 46)
                    { // Armas de arremeco
                        if(SrcPos != 128)
                        { // A segunda arma esta a mao do escudo
                            return false;
                        }
                    }

                    else if(SrcUnique == 46)
                    { // Armas de arremeco
                        if(ItemPos != 128)
                        { // A segunda arma esta a mao do escudo
                            return false;
                        }
                    }
                    else // Outros tipos de armas
                        return false;
                }
            }
        }
    }

	pClass = GetInfoClass(mob->Equip[0].EF2);
	if((mob->Equip[0].Index >= 22 && mob->Equip[0].Index <= 25) || mob->Equip[0].Index == 32)
	{
		pClass = 2;

		if(mob->Equip[0].EFV2 >= ARCH)
			pClass = GetInfoClass(mob->Equip[0].EF2);
	}

    short ItemClass = GetItemAbility(pItem, EF_CLASS);
	short pos = GetItemAbility(pItem, EF_POS);

	if(pos == 0) //evita de equipar o item
	{
		Log(SERVER_SIDE, LOG_ERROR, "Item com POS 0 - ItemId: %d", pItem->Index);
		return false;
	}

	if(ItemClass == 0)
		return false;

	if(mob->Equip[0].EFV2 == 1)
		if(((ItemClass >> pClass) & 1) == 0)
			return false;
		else NULL;
	else 
		if(((ItemClass >> pClass) & 1) == 0 && pos <= 32)
			return false;

    // Dados do requerimento
    short ItemRLevel = GetItemAbility(pItem, EF_LEVEL);
    short ItemRSTR = GetItemAbility(pItem, EF_REQ_STR);
    short ItemRINT = GetItemAbility(pItem, EF_REQ_INT);
    short ItemRDEX = GetItemAbility(pItem, EF_REQ_DEX);
    short ItemRCON = GetItemAbility(pItem, EF_REQ_CON);
    short ItemWType = GetItemAbility(pItem, EF_WTYPE);

    ItemWType %= 10;
    int divItemWType = (ItemWType / 10);

    if(pSlot == 7 && ItemWType != 0)
    {
        int porcDim = 100;
        if(divItemWType == 0 && ItemWType > 1)
            porcDim = 130;
        else if(divItemWType == 6 && ItemWType > 1)
            porcDim = 150;

        ItemRLevel = ((ItemRLevel * porcDim) / 100);
        ItemRSTR = ((ItemRSTR * porcDim) / 100);
        ItemRINT = ((ItemRINT * porcDim) / 100);
        ItemRDEX = ((ItemRDEX * porcDim) / 100);
        ItemRCON = ((ItemRCON * porcDim) / 100);
    }

	int ItemMobType = GetEffectValueByIndex(pItem->Index, EF_MOBTYPE);
	if(ItemMobType != 0)
	{
		if(ItemMobType == 2)
		{
			if(classMaster == MORTAL)
			{
				if(ItemRLevel > static_cast<short>(mob->bStatus.Level))
					return false;
			}
			else 
				return false;
		}
		else if(ItemMobType == CELESTIAL)
		{
			if(classMaster >= CELESTIAL)
				return true;

			return false;
		}
		else if(classMaster != MORTAL)
		{
			if(pSlot == 1)
			{
				if(pItem->Index >= 3500 && pItem->Index <= 3508)
					return true;

				return false;
			}
		} 
		else
			return false;
	}

	if(classMaster >= ARCH)
	{
		if(pSlot == 1)
			if((pItem->Index >= 3500 && pItem->Index <= 3508) || pItem->Index == 747)
				return true;
			else
				return false;
	}

	if(classMaster >= ARCH)
		return true;

    // Verificacao dos atributos do personagem
	if(ItemRSTR <= mob->Status.STR &&
       ItemRINT <= mob->Status.INT &&
       ItemRDEX <= mob->Status.DEX &&
       ItemRCON <= mob->Status.CON &&
       ItemRLevel <= static_cast<int>(mob->Status.Level))
        return true;

    return false;
}

short GetItemAbility(st_Item *itemPtr, int eff)
{
    int result = 0;

    int itemID = itemPtr->Index;

    int unique = ItemList[itemID].Unique;
    int pos = ItemList[itemID].Pos;

    if(eff == EF_DAMAGEADD || eff == EF_MAGICADD)
        if(unique < 41 || unique > 50)
            return 0;

    if(eff == EF_CRITICAL)
        if(itemPtr->Effect[1].Index == EF_CRITICAL2 || itemPtr->Effect[2].Index == EF_CRITICAL2)
            eff = EF_CRITICAL2;

    if(eff == EF_DAMAGE && pos == 32)
        if(itemPtr->Effect[1].Index == EF_DAMAGE2 || itemPtr->Effect[2].Index == EF_DAMAGE2)
            eff = EF_DAMAGE2;

    if(eff == EF_ACADD)
        if(itemPtr->Effect[1].Index == EF_ACADD2 || itemPtr->Effect[2].Index == EF_ACADD2)
            eff = EF_ACADD2;

    if(eff == EF_LEVEL && itemID >= 2330 && itemID < 2360)
        result = (itemPtr->Effect[1].Index - 1);
    else if(eff == EF_LEVEL)
        result += ItemList[itemID].Level;

    if(eff == EF_REQ_STR)
        result += ItemList[itemID].Str;
    if(eff == EF_REQ_INT)
        result += ItemList[itemID].Int;
    if(eff == EF_REQ_DEX)
        result += ItemList[itemID].Dex;
    if(eff == EF_REQ_CON)
        result += ItemList[itemID].Con;

    if(eff == EF_POS)
        result += ItemList[itemID].Pos;

    if(eff != EF_INCUBATE)
    {
        for(int i = 0; i < 12; i++)
        {
            if(ItemList[itemID].Effect[i].Index != eff)
                continue;

            int val = ItemList[itemID].Effect[i].Value;
            if(eff == EF_ATTSPEED && val == 1)
                val = 10;

            result += val;
            break;
        }
    }

	if(itemPtr->Index >= 2330 && itemPtr->Index < 2390)
    {
        if(eff == EF_MOUNTHP)
            return *(WORD*)&itemPtr->Effect[0].Index;

        if(eff == EF_MOUNTSANC)
            return itemPtr->Effect[1].Index;

        if(eff == EF_MOUNTLIFE)
            return itemPtr->Effect[1].Value;

        if(eff == EF_MOUNTFEED)
            return itemPtr->Effect[2].Index;

        if(eff == EF_MOUNTKILL)
            return itemPtr->Effect[2].Value;
		
		if(itemPtr->Index >= 2360 && itemPtr->Index < 2390 && *(short*)&itemPtr->Effect[0].Index > 0)
        {
			int mountIndex = itemPtr->Index - 2360;
			if(mountIndex < 0 || mountIndex > 29)
				return 0;

			st_MountData mont = mMont[mountIndex];

            int ef2 = itemPtr->Effect[1].Index;
            if(eff == EF_DAMAGE)
				return static_cast<short>((mont.atkFisico * (ef2 + 20) / 100));

            if(eff == EF_MAGIC)
				return static_cast<short>((mont.atkMagico * (ef2 + 15) / 100));

            if(eff == EF_PARRY)
				return static_cast<short>(mont.Evasion);

            if(eff == EF_RUNSPEED)
				return static_cast<short>(mont.speedMove);

            if(eff == EF_RESIST1 || eff == EF_RESIST2 ||
				eff == EF_RESIST3 || eff == EF_RESIST4 || eff == EF_RESISTALL)
			   return static_cast<short>(mont.Resist);
        }
		
        return result;
    }
	
	if(itemPtr->Index >= 3980 && itemPtr->Index <= 3999 && eff == EF_RUNSPEED)
		return 6;
	
    for(int i = 0; i < 3; i++)
    {
        if(itemPtr->Effect[i].Index != eff)
            continue;

        int val = itemPtr->Effect[i].Value;
        if(eff == EF_ATTSPEED && val == 1)
            val = 10;

        result += val;
    }

    if(eff == EF_RESIST1 || eff == EF_RESIST2 ||
       eff == EF_RESIST3 || eff == EF_RESIST4)
    {
        for(int i = 0; i < 12; i++)
        {
            if(ItemList[itemID].Effect[i].Index != EF_RESISTALL)
                continue;

            result += ItemList[itemID].Effect[i].Value;
            break;
        }

        for(int i = 0; i < 3; i++)
        {
            if(itemPtr->Effect[i].Index != EF_RESISTALL)
                continue;

            result += itemPtr->Effect[i].Value;
            break;
        }
    }

	int sanc = GetItemSanc(itemPtr);
	if (itemPtr->Index <= 40)
		sanc = 0;

	if (sanc >= 9 && (pos & 0xF00) != 0)
		sanc += 1;

	if (sanc > 15)
		sanc = 15;

	if (sanc != 0 && eff != EF_GRID && eff != EF_CLASS &&
		eff != EF_POS && eff != EF_WTYPE && eff != EF_RANGE &&
		eff != EF_LEVEL && eff != EF_REQ_STR && eff != EF_REQ_INT &&
		eff != EF_REQ_DEX && eff != EF_REQ_CON && eff != EF_VOLATILE &&
		eff != EF_INCUBATE && eff != EF_INCUDELAY && eff != EF_UNKNOW1 &&
		eff != EF_MOBTYPE)
	{
		INT32 value = sanc;

		if (value == 10)
			value = 10;
		else if (value == 11)
			value = 12;
		else if (value == 12)
			value = 15;
		else if (value == 13)
			value = 18;
		else if (value == 14)
			value = 23;
		else if (value == 15)
			value = 28;

		result = result * (value + 10) / 10;
	}

	if (eff == EF_RUNSPEED)
	{
		if (result >= 3)
			result = 2;

		if (result > 0 && sanc >= 9)
			result++;
	}

	if (eff == EF_HWORDGUILD || eff == EF_LWORDGUILD)
	{
		int x = result;
		result = x;
	}

	if (eff == EF_GRID)
		if (result < 0 || result > 7)
			result = 0;

    return result;
}

short GetItemAbilityNoSanc(st_Item *itemPtr, int eff)
{
	int result = 0;

	int itemID = itemPtr->Index;

	int unique = ItemList[itemID].Unique;
	int pos = ItemList[itemID].Pos;

	if (eff == EF_DAMAGEADD || eff == EF_MAGICADD)
		if (unique < 41 || unique > 50)
			return 0;

	if (eff == EF_CRITICAL)
		if (itemPtr->Effect[1].Index == EF_CRITICAL2 || itemPtr->Effect[2].Index == EF_CRITICAL2)
			eff = EF_CRITICAL2;

	if (eff == EF_DAMAGE && pos == 32)
		if (itemPtr->Effect[1].Index == EF_DAMAGE2 || itemPtr->Effect[2].Index == EF_DAMAGE2)
			eff = EF_DAMAGE2;

	if (eff == EF_ACADD)
		if (itemPtr->Effect[1].Index == EF_ACADD2 || itemPtr->Effect[2].Index == EF_ACADD2)
			eff = EF_ACADD2;

	if (eff == EF_LEVEL && itemID >= 2330 && itemID < 2360)
		result = (itemPtr->Effect[1].Index - 1);
	else if (eff == EF_LEVEL)
		result += ItemList[itemID].Level;

	if (eff == EF_REQ_STR)
		result += ItemList[itemID].Str;
	if (eff == EF_REQ_INT)
		result += ItemList[itemID].Int;
	if (eff == EF_REQ_DEX)
		result += ItemList[itemID].Dex;
	if (eff == EF_REQ_CON)
		result += ItemList[itemID].Con;

	if (eff == EF_POS)
		result += ItemList[itemID].Pos;

	if (eff != EF_INCUBATE)
	{
		for (int i = 0; i < 12; i++)
		{
			if (ItemList[itemID].Effect[i].Index != eff)
				continue;

			int val = ItemList[itemID].Effect[i].Value;
			if (eff == EF_ATTSPEED && val == 1)
				val = 10;

			result += val;
			break;
		}
	}

	if (itemPtr->Index >= 2330 && itemPtr->Index < 2390)
	{
		if (eff == EF_MOUNTHP)
			return *(WORD*)&itemPtr->Effect[0].Index;

		if (eff == EF_MOUNTSANC)
			return itemPtr->Effect[1].Index;

		if (eff == EF_MOUNTLIFE)
			return itemPtr->Effect[1].Value;

		if (eff == EF_MOUNTFEED)
			return itemPtr->Effect[2].Index;

		if (eff == EF_MOUNTKILL)
			return itemPtr->Effect[2].Value;

		if (itemPtr->Index >= 2360 && itemPtr->Index < 2390 && *(short*)&itemPtr->Effect[0].Index > 0)
		{
			int mountIndex = itemPtr->Index - 2360;
			if (mountIndex < 0 || mountIndex > 29)
				return 0;

			st_MountData mont = mMont[mountIndex];

			int ef2 = itemPtr->Effect[1].Index;
			if (eff == EF_DAMAGE)
				return static_cast<short>((mont.atkFisico * (ef2 + 20) / 100));

			if (eff == EF_MAGIC)
				return static_cast<short>((mont.atkMagico * (ef2 + 15) / 100));

			if (eff == EF_PARRY)
				return static_cast<short>(mont.Evasion);

			if (eff == EF_RUNSPEED)
				return static_cast<short>(mont.speedMove);

			if (eff == EF_RESIST1 || eff == EF_RESIST2 ||
				eff == EF_RESIST3 || eff == EF_RESIST4 || eff == EF_RESISTALL)
				return static_cast<short>(mont.Resist);
		}

		return result;
	}

	if (itemPtr->Index >= 3980 && itemPtr->Index <= 3999 && eff == EF_RUNSPEED)
		return 6;

	for (int i = 0; i < 3; i++)
	{
		if (itemPtr->Effect[i].Index != eff)
			continue;

		int val = itemPtr->Effect[i].Value;
		if (eff == EF_ATTSPEED && val == 1)
			val = 10;

		result += val;
	}

	if (eff == EF_RESIST1 || eff == EF_RESIST2 ||
		eff == EF_RESIST3 || eff == EF_RESIST4)
	{
		for (int i = 0; i < 12; i++)
		{
			if (ItemList[itemID].Effect[i].Index != EF_RESISTALL)
				continue;

			result += ItemList[itemID].Effect[i].Value;
			break;
		}

		for (int i = 0; i < 3; i++)
		{
			if (itemPtr->Effect[i].Index != EF_RESISTALL)
				continue;

			result += itemPtr->Effect[i].Value;
			break;
		}
	}

	return result;
}

int GetMaxAbility(st_Mob *usr, int eff)
{
    int MaxAbility = 0;

    for(int i = 0; i < 18; i++)
    {
		if(usr->Equip[i].Index == 0)
            continue;

        short ItemAbility = GetItemAbility(&usr->Equip[i], eff);
        if(MaxAbility < ItemAbility)
            MaxAbility = ItemAbility;
    }

    return MaxAbility;
}

int GetMobAbility(stCharInfo* usr, int eff)
{
	int value = GetMobAbility(&usr->Player, eff);
	if (eff == EF_RANGE)
		return value;

	// aqui poderá ficar os effects de outras estruturas (no caso do over, runas, por exemplo)

	if (eff == EF_BONUSEXPIND && IsCostume(&usr->Player.Equip[12]))
	{
		int expBonus = usr->Player.Equip[12].EF1;
		if (expBonus > 0 && expBonus <= 20)
			value += expBonus;
	}

	return value;
}

int GetMobAbility(st_Mob *usr, int eff)
{
    int LOCAL_1 = 0;

    if(eff == EF_RANGE)
    {
        LOCAL_1 = GetMaxAbility(usr, eff);

		int LOCAL_2 = (usr->Equip[0].Index / 10);
        if(LOCAL_1 < 2 && LOCAL_2 == 3)
            if((usr->Learn[0] & 0x40ULL) != 0)
                LOCAL_1 = 2;

        return LOCAL_1;
    }

    int LOCAL_18[18];
    for(int LOCAL_19 = 0; LOCAL_19 < 18; LOCAL_19++)
    {
        LOCAL_18[LOCAL_19] = 0;

		int LOCAL_20 = usr->Equip[LOCAL_19].Index;
        if(LOCAL_20 == 0 && LOCAL_19 != 7)
            continue;

        if(LOCAL_19 >= 1 && LOCAL_19 <= 5)
            LOCAL_18[LOCAL_19] = ItemList[LOCAL_20].Unique;

        if(eff == EF_DAMAGE && LOCAL_19 == 6)
            continue;

        if(eff == EF_MAGIC && LOCAL_19 == 7)
            continue;

		if (LOCAL_19 == 12 && eff != EF_SAVEMANA && eff != EF_AC && eff != EF_BONUSEXPIND)
			continue;

        if(LOCAL_19 == 7 && eff == EF_DAMAGE)
        {
            int dam1 = (GetItemAbility(&usr->Equip[6], EF_DAMAGE) +
                        GetItemAbility(&usr->Equip[6], EF_DAMAGE2));
            int dam2 = (GetItemAbility(&usr->Equip[7], EF_DAMAGE) +
                        GetItemAbility(&usr->Equip[7], EF_DAMAGE2));

			int arm1 = usr->Equip[6].Index;
            int arm2 = usr->Equip[7].Index;

            int unique1 = 0;
            if(arm1 > 0 && arm1 < MAX_ITEMLIST)
                unique1 = ItemList[arm1].Unique;

            int unique2 = 0;
            if(arm2 > 0 && arm2 < MAX_ITEMLIST)
                unique2 = ItemList[arm2].Unique;

            if(unique1 != 0 && unique2 != 0)
            {
                int porc = 0;
                if(unique1 == unique2)
                    porc = 70;
                else
                    porc = 50;

                if(dam1 > dam2)
                    LOCAL_1 = ((LOCAL_1 + dam1) + ((dam2 * porc) / 100));
                else
                    LOCAL_1 = ((LOCAL_1 + dam2) + ((dam1 * porc) / 100));

                continue;
            }

            if(dam1 > dam2)
                LOCAL_1 += dam1;
            else
                LOCAL_1 += dam2;

            continue;
        }

        int LOCAL_28 = GetItemAbility(&usr->Equip[LOCAL_19], eff);
        if(eff == EF_ATTSPEED && LOCAL_28 == 1)
            LOCAL_28 = 10;

        LOCAL_1 += LOCAL_28;
    }

    if(eff == EF_AC && LOCAL_18[1] != 0)
        if(LOCAL_18[1] == LOCAL_18[2] && LOCAL_18[2] == LOCAL_18[3] &&
           LOCAL_18[3] == LOCAL_18[4] && LOCAL_18[4] == LOCAL_18[5])
            LOCAL_1 = ((LOCAL_1 * 105) / 100);

    return LOCAL_1;
}

bool CanCarry(st_Item* Dest, st_Item* Inven, int DestX, int DestY, int* error)
{
	int pItemGrid = GetItemAbility(Dest, EF_GRID);
	
	unsigned char pGridDest[8];
	memcpy(pGridDest, ItemGrid[pItemGrid], 8);

	unsigned char invSlots[MAX_INVEN];
	memset(invSlots, 0, MAX_INVEN);

	for(int i = 0; i < MAX_INVEN; i++)
    {
		if(Inven[i].Index == 0)
            continue;

		pItemGrid = GetItemAbility(Dest, EF_GRID);

		unsigned char pGridInv[8];
	    memcpy(pGridInv, g_pItemGrid[pItemGrid], 8);

		int pInvX = (i % 9);
        int pInvY = (i / 9);
		for(int y = 0; y < 4; y++)
        {
            for(int x = 0; x < 2; x++)
            {
                if(pGridInv[(y * 2) + x] == 0)
                    continue;

                if((y + pInvY) < 0 || (y + pInvY) >= 7)
                    continue;

                if((x + pInvX) < 0 || (x + pInvX) >= 9)
                    continue;

                invSlots[(y + pInvY) * 9 + x + pInvX] = (i + 1);
            }
        }
	}

	for(int y = 0; y < 4; y++)
    {
        for(int x = 0; x < 2; x++)
        {
            if(pGridDest[(y * 2) + x] == 0)
                continue;

            if((y + DestY) <  0 || (x + DestX) <  0 || (y + DestY) >= 7 || (x + DestX) >= 9)
            {
                *error = -1;
                return FALSE;
            }

            if(invSlots[(y + DestY) * 9 + x + DestX] == 0)
                continue;

            *error = invSlots[(y + DestY) * 9 + x + DestX];
            return FALSE;
        }
    }

	return TRUE;
}

bool CanCargo(st_Item *destItem, st_Item *Inventory, int pDestX, int pDestY)
{
    int pGrid = GetItemAbility(destItem, EF_GRID);

	unsigned char pGridDest[8];
	memcpy(pGridDest, g_pItemGrid[pGrid], 8);

	unsigned char invSlots[128];
	memset(invSlots, 0, 128);

	for(int slot = 0; slot < 128; slot++)
    {
        if(Inventory[slot].Index == 0)
            continue;

        pGrid =  GetItemAbility(&Inventory[slot], EF_GRID);

        unsigned char pGridInv[8];
	    memcpy(pGridInv, g_pItemGrid[pGrid], 8);

        int pInvX = (slot % 9);
        int pInvY = (slot / 9);
        for(int y = 0; y < 4; y++)
        {
            for(int x = 0; x < 2; x++)
            {
                if(pGridInv[(y * 2) + x] == 0)
                    continue;

                if((y + pInvY) < 0 || (y + pInvY) >= 14)
                    continue;

                if((x + pInvX) < 0 || (x + pInvX) >= 9)
                    continue;

                invSlots[(y + pInvY) * 9 + x + pInvX] = (slot + 1);
            }
        }
    }

    for(int y = 0; y < 4; y++)
    {
        for(int x = 0; x < 2; x++)
        {
            if(pGridDest[(y * 2) + x] == 0)
                continue;

            if((y + pDestY) <   0 || (x + pDestX) <  0 ||
               (y + pDestY) >= 14 || (x + pDestX) >= 9)
                return false;

            if(invSlots[(y + pDestY) * 9 + x + pDestX] == 0)
                continue;

            return false;
        }
    }

    return true;
}

void CharLogOut(int clientId)
{
	if(Users[clientId].Status != USER_PLAY)
	{
		SendSignal(clientId, clientId, 0x116);

		return;
	}
	
	if(Users[clientId].Trade.ClientId != 0 || Users[clientId].IsAutoTrading)
		RemoveTrade(clientId);

	LogGold(clientId);

	Users[clientId].User.Position.X = 0;
	Users[clientId].User.Position.Y = 0;
	Users[clientId].User.CharSlot = -1;

	// Salva
	SaveUser(clientId, 1);
	DeleteMob(clientId, 2);

	RemoveParty(clientId);

	// Seta como o usuário está na charList
	Users[clientId].Status = USER_SELCHAR;

	// Remove o registro da quest atual
	Users[clientId].QuestAccess = 0;

	if(Mob[clientId].Target.Y > 0 && Mob[clientId].Target.Y < 4096 && Mob[clientId].Target.X > 0 && Mob[clientId].Target.X < 4096)
 		g_pMobGrid[Mob[clientId].Target.Y][Mob[clientId].Target.X] = MOB_EMPTY;

	Mob[clientId].Mode = 0;
	SendSignal(clientId, clientId, 0x116);

	Users[clientId].PremierStore.Status = 0;
	Users[clientId].PremierStore.Time   = 0;
	Users[clientId].PremierStore.Wait   = 0;
	Users[clientId].PremierStore.Count  = 0;

	Users[clientId].aHack.Question = -1;
	Users[clientId].aHack.Response = 0;
	Users[clientId].aHack.Error    = 0;
	Users[clientId].aHack.Next     = 60;
	Users[clientId].aHack.Last     = sServer.SecCounter;

	Users[clientId].MacIntegrity.WasWarned = false;
	Users[clientId].MacIntegrity.IsChecked = false;

	sServer.Zakum.Unregister(clientId);
	std::fill(std::begin(Users[clientId].TimeStamp.Skills), std::end(Users[clientId].TimeStamp.Skills), std::chrono::steady_clock::time_point());
}	

void CheckIdle(int clientId)
{
	INT32 LOCAL_1 = sServer.SecCounter, 
		  LOCAL_2 = Users[clientId].TimeStamp.LastReceiveTime;
		
	if(LOCAL_2 > LOCAL_1)
		Users[clientId].TimeStamp.LastReceiveTime = sServer.SecCounter;

	if(LOCAL_2 < (LOCAL_1 - 1440))
		Users[clientId].TimeStamp.LastReceiveTime = sServer.SecCounter;

	if(LOCAL_2 < (LOCAL_1 - 720))
	{
		Log(clientId, LOG_INGAME, "Desconectado por inatividade... Last: %d. Counter: %d. Status: %d", Users[clientId].TimeStamp.LastReceiveTime, sServer.SecCounter, Users[clientId].Status);
		LogPlayer(clientId, "Desconectado por inatividade");

		CloseUser(clientId);
	}
}

void SetBattle(int arg1, int arg2)
{
	//0x00401014 Lib: Server.obj Class : (null)

    if(arg1 <= 0     || arg2 <= 0 ||
	   arg1 >= 30000 || arg2 >= 30000)
	   return;

	if(!Mob[arg1].Mode)
		return;

	if(!Mob[arg2].Mode)
		return;

	if(arg1 < MAX_PLAYER)
	{
		if(Users[arg1].Status != USER_PLAY)
			return;
	}
	
	if (Mob[arg1].Target.X < (Mob[arg2].Target.X - VIEWGRIDX))
	    return;
		
	if (Mob[arg1].Target.X >(Mob[arg2].Target.X + VIEWGRIDX))
		return;

	if (Mob[arg1].Target.Y < (Mob[arg2].Target.Y - VIEWGRIDY))
		return;

	if (Mob[arg1].Target.Y >(Mob[arg2].Target.Y + VIEWGRIDY))
		return;
	
	INT32 mode = Mob[arg1].Mode;

	Mob[arg1].Mode = 5;
	Mob[arg1].AddEnemyList(arg2);

	int LOCAL_1 = Mob[arg1].GenerateID,
		LOCAL_2 = Rand() & 0x80000003;

	if(mode != 5)
	{
		if (Mob[arg1].GenerateID == GUARDIAN_TOWER_BLUE)
		{
			SendNotice("A Torre Guardião do reino Blue está sendo atacado");
			SendKingdomBattleInfo(SERVER_SIDE, CAPE_BLUE, true);

			sServer.KingdomBattle.Info[0].Status = true;
		}
		else if (Mob[arg1].GenerateID == GUARDIAN_TOWER_RED)
		{
			SendNotice("A Torre Guardião do reino Red está sendo atacado");
			SendKingdomBattleInfo(SERVER_SIDE, CAPE_RED, true);

			sServer.KingdomBattle.Info[1].Status = true;
		}
		else if (Mob[arg1].GenerateID == 8) // rei blue
		{
			SendNotice("O Rei Harabard está sendo atacado");

			SendKingdomBattleInfo(SERVER_SIDE, CAPE_BLUE, true);
			sServer.KingdomBattle.Info[0].Status = true;
		}
		else if (Mob[arg1].GenerateID == 9) // rei blue
		{
			SendNotice("O Rei Glantuar está sendo atacado");
			SendKingdomBattleInfo(SERVER_SIDE, CAPE_RED, true);

			sServer.KingdomBattle.Info[1].Status = true;
		}

		if(LOCAL_1 >= 0 && LOCAL_1 < 8192)
		{
			if((mGener.pList[LOCAL_1].FightAction[LOCAL_2][0]))
				if(!Mob[arg1].Leader)
					SendSay(arg1, mGener.pList[LOCAL_1].FightAction[LOCAL_2]);
		}
	}
}

void SetAffect(int clientId, int skillId, int time)
{
	int tickType = SkillData[skillId].AffectType;
	int affectId = GetEmptyAffect(clientId, tickType);

	if (tickType <= 0 || (affectId < 0 && affectId >= 32))
		return;

	Mob[clientId].Mobs.Affects[affectId].Index = tickType;
	Mob[clientId].Mobs.Affects[affectId].Master = SkillData[skillId].AffectValue;
	Mob[clientId].Mobs.Affects[affectId].Time = time;
	Mob[clientId].Mobs.Affects[affectId].Value = 1;

	Mob[clientId].GetCurrentScore(clientId);
}

int SetAffect(int clientId, int skillId, int delay, int level)
{
	CMob *spw = &Mob[clientId];

	int tickType = SkillData[skillId].AffectType;
	int skillType = SkillData[skillId].Aggressive; // LOCAL2

	if(spw->Mobs.Player.AffectInfo.Resist && skillType != 0)
		return 0;
	
	int affectId = GetEmptyAffect(clientId, tickType);//LOCAL_3
	if(tickType <= 0 || (affectId < 0 && affectId >= 32))
		return 0;

	int local4 = spw->Mobs.Affects[affectId].Index;
	int aux = 1;

	spw->Mobs.Affects[affectId].Index = tickType;
	spw->Mobs.Affects[affectId].Master = SkillData[skillId].AffectValue;
	
	unsigned int time = SkillData[skillId].Time * delay / 100;

	if(local4 == tickType)
	{
		if(time > spw->Mobs.Affects[affectId].Time)
			spw->Mobs.Affects[affectId].Time = time;
	}
	else
	{
		spw->Mobs.Affects[affectId].Time = time;
		aux = 2;
	}

	if(local4 == tickType)
	{
		if(level > spw->Mobs.Affects[affectId].Value)
			spw->Mobs.Affects[affectId].Value = level;
	}
	else
		spw->Mobs.Affects[affectId].Value = level;

	Mob[clientId].GetCurrentScore(clientId);

	return aux;
}

int SetBuff(int clientId, int buffId, int master, int value, int time)
{
	return SetBuff(clientId, buffId, master, value, time, true);
}

int SetBuff(int clientId, int buffId, int master, int value, int time, bool overrideBuffTime)
{
	INT32 LOCAL_3 = GetEmptyAffect(clientId, buffId);

	if(buffId > 0 && LOCAL_3 >= 0 && LOCAL_3 < 32)
	{
		st_Affect *affect = Mob[clientId].Mobs.Affects;

		affect[LOCAL_3].Index = buffId;
		affect[LOCAL_3].Master = master;

		if(affect[LOCAL_3].Time > 0 && overrideBuffTime)
			affect[LOCAL_3].Time += time;
		else
			affect[LOCAL_3].Time = time;

		affect[LOCAL_3].Value = value;

		Mob[clientId].GetCurrentScore(clientId);
		return true;
	}

	return false;
}

int SetTick(int clientId, int skillId, int delay, int level)
{
	CMob *spw = &Mob[clientId];
	if(spw->Mobs.Player.Info.Merchant == 1 && clientId > MAX_PLAYER)
		return false;

	INT32 LOCAL_1 = SkillData[skillId].TickType;
	INT32 LOCAL_2 = SkillData[skillId].Aggressive;

	if((spw->Mobs.AffectInfo & 2) && LOCAL_2 != 0)
		return false;

	INT32 LOCAL_3 = GetEmptyAffect(clientId, LOCAL_1);

	if(LOCAL_1 > 0 && LOCAL_3 >= 0 && LOCAL_3 < 32)
	{
		st_Affect *affect = spw->Mobs.Affects;

		affect[LOCAL_3].Index = LOCAL_1;
		affect[LOCAL_3].Master = SkillData[skillId].TickValue;

		affect[LOCAL_3].Time = (delay * SkillData[skillId].Time) / 100;

		if(delay >= 10000)
			affect[LOCAL_3].Time = 10000;

		affect[LOCAL_3].Value = level;

		Mob[clientId].GetCurrentScore(clientId);
		return true;
	}

	return false;
}

void MobKilled(int arg1, int arg2, int arg3, int arg4)
{
	char temp[128];
	if(arg1 <= 0 || arg1 >= 30000)
		return;

	if(arg2 <= 0 || arg2 >= 30000)
		return;

	if(Mob[arg1].Mode == 0)
		return;

#if defined(_DEBUG)
	if (arg2 < MAX_PLAYER && arg1 > MAX_PLAYER)
		Log(arg2, LOG_INGAME, "Matou o mob %s. GenerID: %d", Mob[arg1].Mobs.Player.Name, Mob[arg1].GenerateID);
#endif

	if (Mob[arg1].GenerateID == TORRE_RVR_BLUE || Mob[arg1].GenerateID == TORRE_RVR_RED)
	{
		if (arg2 == 29999)
		{
			DeleteMob(arg1, 1);

			return;
		}

		int connId = arg2;
		if (connId >= MAX_PLAYER)
		{
			int summonerId = Mob[connId].Summoner;

			if (summonerId > 0 && summonerId < MAX_PLAYER)
				connId = summonerId;
			else
				connId = 0;
		}

		if (connId <= MAX_PLAYER && Users[connId].Status != USER_PLAY)
			connId = 0;

		// Ressuscita a torre
		Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;

		p364 packet;
		GetCreateMob(arg1, (BYTE*)&packet);

		GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&packet, 0);

		int towerId = 0;

		if (Mob[arg1].GenerateID == TORRE_RVR_BLUE)
			towerId = 1;
		else
			towerId = 0;

		sServer.RvR.Points[towerId] += 25;
		Mob[connId].Mobs.RvRPoints += 15;

		for(INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if(Users[i].Status != USER_PLAY)
				continue;

			if(Mob[i].Target.X >= 1041 && Mob[i].Target.X <= 1248 && Mob[i].Target.Y >= 1950 && Mob[i].Target.Y <= 2158)
				SendClientMessage(i, "A torre do reino %s foi destruída por %s. +25 pontos para o reino %s.", (Mob[arg1].GenerateID == TORRE_RVR_BLUE) ? "Blue" : "Red", Mob[connId].Mobs.Player.Name, (towerId == 0) ? "Blue" : "Red");

			// RVR
			if (Mob[i].Target.X >= 1041 && Mob[i].Target.X <= 1248 &&
				Mob[i].Target.Y >= 1950 && Mob[i].Target.Y <= 2158 && sServer.RvR.Status == 1)
			{
				int cape = Mob[i].Mobs.Player.CapeInfo;

				INT32 posX  = 0;
				INT32 posY = 0;

				if (cape == CAPE_BLUE)
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
						posX = 1238 - Rand() % 5;
						posY = 1983 + Rand() % 5;
					}
					else
					{
						posX = 1211 - Rand() % 5;
						posY = 1955 + Rand() % 5;
					}
				}
				if(posX != 0 && posY != 0)
					Teleportar(i, posX, posY);
			}
		}
		
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (Mob[i].Target.X >= 1041 && Mob[i].Target.X <= 1248 &&
				Mob[i].Target.Y >= 1950 && Mob[i].Target.Y <= 2158)
			{
				if (Users[i].Status != USER_PLAY)
					continue;

				// Atualiza o placar de kill
				SendCounterMob(i, sServer.RvR.Points[1], sServer.RvR.Points[0]);
			}
		}

		return;
	}

	if (Mob[arg1].GenerateID == TORRE_ERION)
	{
		if (arg2 == 29999)
		{
			DeleteMob(arg1, 1);

			return;
		}

		int cId = arg2;
		if (cId >= MAX_PLAYER)
		{
			int summonerId = Mob[cId].Summoner;

			if (summonerId > 0 && summonerId < MAX_PLAYER)
				cId = summonerId;
			else
				cId = 0;
		}

		if (cId < MAX_PLAYER && Users[cId].Status != USER_PLAY)
			cId = 0;

		INT32 guildId = Mob[cId].Mobs.Player.GuildIndex;
		sServer.TowerWar.Guild = guildId;
			
		SendNotice("O jogador %s da guild %s derrubou a Torre de Erion.", Mob[cId].Mobs.Player.Name, g_pGuild[guildId].Name.c_str());

		// Ressuscita a torre
		Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;
		Mob[arg1].Mobs.Player.GuildIndex   = sServer.TowerWar.Guild;

		p364 packet;
		GetCreateMob(arg1, (BYTE*)&packet);

		GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&packet, 0);
		
		ClearTowerArea(0);

		Log(SERVER_SIDE, LOG_INGAME, "A Torre foi derrubada pela guild %s (%d)", g_pGuild[guildId].Name.c_str(), guildId);
		return;
	}
	/*
	if(arg1 < MAX_PLAYER)
	{
		Users[arg1].Potion.CountHp = Mob[arg1].Mobs.Player.Status.maxHP;
		SetReqHp(arg1);
		Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;
		SendScore(arg1);
		return;
	}*/

	// 0045AB5D
	st_Item *LOCAL_1 = &Mob[arg1].Mobs.Player.Equip[13];
	if(LOCAL_1->Index == 769)
	{
		INT32 LOCAL_2 = GetItemSanc(LOCAL_1);
		if(LOCAL_2 <= 0)
			memset(LOCAL_1, 0, sizeof LOCAL_1);
		else
		{
			LOCAL_2 = LOCAL_2 - 1;

			SetItemSanc(LOCAL_1, LOCAL_2, 0);
		}

		if(arg1 > 0 && arg1 < MAX_PLAYER)
		{
			SendItem(arg1, SlotType::Equip, 13, LOCAL_1);

			SendEmotion(arg1, 14, 2);

			p364 packet;
			GetCreateMob(arg1, (BYTE*)&packet);

			GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&packet, 0);
		}
		
		SendEquip(arg1);

		Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;
		if(arg1 < MAX_PLAYER)
		{
			SetReqHp(arg1);
			SendScore(arg1);
		}
			
		return;
	}

	INT8 pergaRessuSlot = GetFirstSlot(arg1, 3463);
	if (pergaRessuSlot != -1 && arg1 < MAX_PLAYER)
	{
		// Pergaminho da ressureição
		// Enviar carta avisando que mataram o jovem, e depois ressucitá-lo
		AmountMinus(&Mob[arg1].Mobs.Player.Inventory[pergaRessuSlot]);
		SendItem(arg1, SlotType::Inv, pergaRessuSlot, &Mob[arg1].Mobs.Player.Inventory[pergaRessuSlot]);

		if (arg2 < MAX_PLAYER)
		{
			char szMsg[120];
			memset(szMsg, 0, 120);

			sprintf_s(szMsg, "![%s] te matou em %dx : %dy.", Mob[arg2].Mobs.Player.Name, Mob[arg1].Target.X, Mob[arg1].Target.Y);

			SendClientMessage(arg1, szMsg);

			LogPlayer(arg1, "[%s] te matou em %dx : %dy - Pergaminho da Ressurreição", Mob[arg2].Mobs.Player.Name, Mob[arg1].Target.X, Mob[arg1].Target.Y);
		}
		
		Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;

		p364 packet;
		GetCreateMob(arg1, (BYTE*)&packet);

		GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&packet, 0);

		SetReqHp(arg1);
		SendScore(arg1);
		return;
	}
	
	if(arg1 < MAX_PLAYER && (Mob[arg1].Mobs.Player.bStatus.Level > 500 || (Users[arg1].AccessLevel != 0 && Users[arg1].IsAdmin)))
	{
		Users[arg1].Potion.CountHp = Mob[arg1].Mobs.Player.Status.maxHP;
		Users[arg1].Potion.CountMp = Mob[arg1].Mobs.Player.Status.maxMP;
		SetReqHp(arg1);

		Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;
		Mob[arg1].Mobs.Player.Status.curMP = Mob[arg1].Mobs.Player.Status.maxMP;

		SendScore(arg1);
		return;
	}

	if (arg1 < MAX_PLAYER)
	{
		int killerId = arg2;
		if (killerId >= MAX_PLAYER && Mob[killerId].Summoner < MAX_PLAYER)
			killerId = Mob[arg2].Summoner;

		if(static_cast<TOD_Arena*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::Arena))->MobKilled(arg1, killerId))
			return;
	}

	if ((sServer.WeekMode == 3 || sServer.ForceWeekDay == 3) && arg1 < MAX_PLAYER) // guerra ativa
	{
		int posX = Mob[arg1].Target.X;
		int posY = Mob[arg1].Target.Y;
		for (int i = 0; i < 5; i++)
		{
			int minX = g_pCityZone[i].war_min_x;
			int minY = g_pCityZone[i].war_min_y;
			int maxX = g_pCityZone[i].war_max_x;
			int maxY = g_pCityZone[i].war_max_y;

			if (posX < minX || posY < minY || posX > maxX || posY > maxY)
				continue;

			// identifica se é guild a ou b
			int guildId = Mob[arg1].Mobs.Player.GuildIndex;

			int zonePosX = -1;
			int zonePosY = -1;

			// verifica
			if (guildId != g_pCityZone[i].chall_index && guildId != g_pCityZone[i].chall_index_2 && guildId != ChargedGuildList[sServer.Channel - 1][i])
			{
				Log(SERVER_SIDE, LOG_INGAME, "O usuário %s (%s) foi identificado na área de guild mas não é desafiante nem dono da cidade. GuildId: %d. %dx %dy", Mob[arg1].Mobs.Player.Name,
					Users[arg1].User.Username, guildId, posX, posY);

				break;
			}

			// Remove uma vida do usuário
			--Mob[arg1].Lifes;

			// está dentro da guerra e possui vidas
			if (Mob[arg1].Lifes >= 0)
			{
				Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;
				Mob[arg1].Mobs.Player.Status.curMP = Mob[arg1].Mobs.Player.Status.maxMP;

				Users[arg1].Potion.CountHp = Mob[arg1].Mobs.Player.Status.maxHP;
				Users[arg1].Potion.CountMp = Mob[arg1].Mobs.Player.Status.maxMP;

				SetReqHp(arg1);

				for (int iAffect = 0; iAffect < 32; iAffect++)
				{
					if (Mob[arg1].Mobs.Affects[iAffect].Index == 32)
					{
						memset(&Mob[arg1].Mobs.Affects[iAffect], 0, sizeof st_Affect);

						SendAffect(arg1);
						break;
					}
				}

				SendClientMessage(arg1, "Você possui %d vidas restantes", Mob[arg1].Lifes);
				SendScore(arg1);

				Log(arg1, LOG_INGAME, "Revivido dentro da área da Guerra. Vidas restantes: %d", Mob[arg1].Lifes);
				return;
			}
		}
	}


	if(sServer.CastleState > 1 && Mob[arg1].Target.X == 1046 && Mob[arg1].Target.Y == 1690 && arg1 > 0 && arg1 < MAX_PLAYER)
		Teleportar(arg1, 1057, 1742);
	
	INT32 LOCAL_3 = Mob[arg2].Mobs.Player.Equip[0].Index;

	if(arg2 >= MAX_PLAYER && Mob[arg2].Mobs.Player.CapeInfo == 4 && LOCAL_3 >= 315 && LOCAL_3 <= 345 && arg1 > MAX_PLAYER && Mob[arg1].Mobs.Player.CapeInfo != 4)
	{
		INT32 LOCAL_4 = Mob[arg2].Summoner;

		if(LOCAL_4 > 0 && LOCAL_4 < MAX_PLAYER && Users[LOCAL_4].Status != 0 && Mob[LOCAL_4].Mode != 0)
		{
			st_Item *LOCAL_5 = &Mob[LOCAL_4].Mobs.Player.Equip[14];

			if(LOCAL_5->Index >= 2330 && LOCAL_5->Index < 2360)
			{
				BYTE LOCAL_6 = LOCAL_5->Effect[1].Index; // EBP - 18h
				BYTE LOCAL_7 = LOCAL_5->Effect[2].Value; // EBP - 1Ch
				BYTE LOCAL_8 = (char)((LOCAL_6 & 255)+ 100); // EBP - 20h
				
				if (LOCAL_5->Index == 2330)
					LOCAL_8 = LOCAL_6 + 25;

				else if (LOCAL_5->Index == 2331)
					LOCAL_8 = LOCAL_6 + 35;

				else if (LOCAL_5->Index == 2332)
					LOCAL_8 = LOCAL_6 + 45;

				else if (LOCAL_5->Index == 2333)
					LOCAL_8 = LOCAL_6 + 55;

				else if (LOCAL_5->Index == 2334)
					LOCAL_8 = LOCAL_6 + 65;

				else if (LOCAL_5->Index == 2335)
					LOCAL_8 = LOCAL_6 + 75;

				if(LOCAL_6 < Mob[arg1].Mobs.Player.Status.Level && LOCAL_6 < 100)
				{
					LOCAL_7 = LOCAL_7 + 1;

					if(LOCAL_7 >= LOCAL_8)
					{
						LOCAL_7 = 1;
						LOCAL_6 = LOCAL_6 + 1;

						LOCAL_5->Effect[2].Value = LOCAL_7;
						LOCAL_5->Effect[1].Index = LOCAL_6;

						SendClientMessage(LOCAL_4, "Sua montaria subiu de level.");

						SendItem(LOCAL_4, SlotType::Equip, 14, &Mob[LOCAL_4].Mobs.Player.Equip[14]);
						MountProcess(LOCAL_4, 0);
					}
					else
					{
						LOCAL_5->Effect[2].Value = LOCAL_7;
						SendItem(LOCAL_4, SlotType::Equip, 14, &Mob[LOCAL_4].Mobs.Player.Equip[14]);
					}
				}
			}
		}
	}

	// 0045AF04
	p338 LOCAL_13;
	memset(&LOCAL_13, 0, sizeof(p338));

	LOCAL_13.Header.PacketId = 0x338;
	LOCAL_13.Header.Size = sizeof p338;
	LOCAL_13.Header.ClientId = 30000;

	LOCAL_13.killed = arg1;
	LOCAL_13.killer = arg2;
	LOCAL_13.Hold = static_cast<int>(Mob[arg2].Mobs.Hold);
	LOCAL_13.Exp = Mob[arg2].Mobs.Player.Exp;

	Mob[arg1].Mobs.Player.Status.curHP = 0;


	INT32 LOCAL_14 = Mob[arg2].Leader;
	if(LOCAL_14 == 0)
		LOCAL_14 = arg2;

	// 45AF56

	if(arg2 >= MAX_PLAYER && Mob[arg2].Mobs.Player.CapeInfo == 4)
	{
		INT32 LOCAL_15 = Mob[arg2].Summoner;

		if(LOCAL_15 <= 0 || LOCAL_15 >= MAX_PLAYER || Users[LOCAL_15].Status != 22)
		{
			GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

			if(arg1 >= MAX_PLAYER)
				DeleteMob(arg1, 1);

			return;
		}
		else
			arg2 = LOCAL_15;
	}

	// Soma que matou um mob
	if(arg1 >= MAX_PLAYER && arg2 < MAX_PLAYER)
		Mob[arg2].Mobs.Counters.PvM.Mob ++;
	
	// Mob matou player
	if(arg1 < MAX_PLAYER && arg2 >= MAX_PLAYER)
		Mob[arg1].Mobs.Counters.PvM.Death++;

	// Player matou player
	if(arg1 < MAX_PLAYER && arg2 < MAX_PLAYER)
	{
		// arg2 é quem matou, então soma um no kill
		Mob[arg2].Mobs.Counters.PvP.Kill ++;

		// arg1 é quem morreu, então soma um no Death
		Mob[arg1].Mobs.Counters.PvP.Death++;
	}

	if (arg1 < MAX_PLAYER && arg2 < MAX_PLAYER)
	{
		if (Mob[arg1].Target.X >= 1041 && Mob[arg1].Target.X <= 1248 &&
			Mob[arg1].Target.Y >= 1950 && Mob[arg1].Target.Y <= 2158 && sServer.RvR.Status == 1)
		{
			int capeKiller = Mob[arg2].Mobs.Player.CapeInfo;
			int capeDead = Mob[arg1].Mobs.Player.CapeInfo;

			if ((capeKiller == CAPE_RED || capeKiller	== CAPE_BLUE) &&
			      (capeDead == CAPE_RED || capeDead		== CAPE_BLUE))
			{
				if (capeDead != capeKiller)
				{
					int capeId = capeKiller - 7;
					sServer.RvR.Points[capeId]++;		

					Mob[arg2].Mobs.RvRPoints++;

					for(INT32 i = 1; i < MAX_PLAYER; i++)
					{
						if(Users[i].Status != USER_PLAY)
							continue;

						if (Mob[i].Target.X >= 1041 && Mob[i].Target.X <= 1248 &&
							Mob[i].Target.Y >= 1950 && Mob[i].Target.Y <= 2158)
						{
							SendClientMessage(i, "%s matou %s e pontuou para o reino %s", Mob[arg2].Mobs.Player.Name, Mob[arg1].Mobs.Player.Name, (capeId == 0) ? "Blue" : "Red");
							SendCounterMob(i, sServer.RvR.Points[1], sServer.RvR.Points[0]);
						}
					}
				}
			}
		}
	}

	if(arg1 < MAX_PLAYER && Mob[arg1].Mobs.Player.bStatus.Level <= 399)
	{
		INT32 LOCAL_16 = Mob[arg1].Mobs.Player.bStatus.Level;
		if(LOCAL_16 < 0 || LOCAL_16 > 399)
			return;

		INT32 evId = Mob[arg1].Mobs.Player.Equip[0].EFV2;
		INT64 LOCAL_17 = g_pNextLevel[evId][LOCAL_16];
		INT64 LOCAL_18 = g_pNextLevel[evId][LOCAL_16 + 1];

		INT64 LOCAL_19 = LOCAL_18 - LOCAL_17;
		INT64 LOCAL_20 = LOCAL_19 / 20;

		if(LOCAL_16 >= 30)
			LOCAL_20 = LOCAL_19 / 22;
		
		if(LOCAL_16 >= 40)
			LOCAL_20 = LOCAL_19 / 25;

		if(LOCAL_16 >= 50)
			LOCAL_20 = LOCAL_19 / 30;

		if(LOCAL_16 >= 60)
			LOCAL_20 = LOCAL_19 / 35;

		if(LOCAL_16 >= 70)
			LOCAL_20 = LOCAL_19 / 40;

		if(LOCAL_16 >= 80)
			LOCAL_20 = LOCAL_19 / 45;

		if(LOCAL_16 >= 90)
			LOCAL_20 = LOCAL_19 / 50;

		if(LOCAL_16 >= 100)
			LOCAL_20 = LOCAL_19 / 55;

		if(LOCAL_16 >= 150)
			LOCAL_20 = LOCAL_19 / 70;

		if(LOCAL_16 >= 200)
			LOCAL_20 = LOCAL_19 / 85;

		if(LOCAL_16 >= 250)
			LOCAL_20 = LOCAL_19 / 100;

		// 0045B154
		if(LOCAL_20 < 0)
			LOCAL_20 = 0;

		if(LOCAL_20 > 30000)
			LOCAL_20 = 30000;

		INT32 LOCAL_21 = GetCurKill(arg2);
		INT32 LOCAL_22 = GetTotKill(arg2);
		INT32 LOCAL_23 = GetPKPoint(arg2);
		INT32 LOCAL_24 = GetPKPoint(arg1);
		INT32 LOCAL_25 = GetGuilty(arg1);
		INT32 LOCAL_26 = GetCurKill(arg1);
		
		INT32 LOCAL_27 = Mob[arg1].Mobs.Player.CapeInfo;
		INT32 LOCAL_28 = Mob[arg2].Mobs.Player.CapeInfo; 

		INT32 LOCAL_29 = 0;
		INT32 LOCAL_30 = 0;

		if(LOCAL_24 <= 0)
			LOCAL_20 = LOCAL_20 * 5;
		else if(LOCAL_24 <= 25)
			LOCAL_20 = LOCAL_20 * 3;

		INT32 LOCAL_31 = Mob[arg1].Target.X >> 7;
		INT32 LOCAL_32 = Mob[arg1].Target.Y >> 7;

		if(arg2 < MAX_PLAYER)
		{
			LOCAL_20 = LOCAL_20 / 6;

			if((LOCAL_27 == 7 && LOCAL_28 == 8) || (LOCAL_27 == 8 && LOCAL_28 == 7))
				LOCAL_29 = 1;

			INT32 LOCAL_33 = Mob[arg1].Mobs.Player.GuildIndex;
			INT32 LOCAL_34 = Mob[arg2].Mobs.Player.GuildIndex;

			INT32 LOCAL_35 = MAX_GUILD;

			//if(LOCAL_33 > 0 && LOCAL_33 < LOCAL_35 && LOCAL_34 > 0 && LOCAL_34 < LOCAL_35 && g_pGuildWar[LOCAL_33] == LOCAL_34 && g_pGuildWar[LOCAL_34] == LOCAL_33)
			//	LOCAL_30 = 1;

			if(sServer.CastleState != 0 && LOCAL_31 == 8 && LOCAL_32 == 13)
				LOCAL_30 = 1;

			if (static_cast<TOD_PreWarBoss*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::PreWarBoss))->GetStatus() && LOCAL_31 == 8 && LOCAL_32 == 13)
				LOCAL_30 = 1;
		}

		Log(arg1, LOG_INGAME, "Morreu para %s %dx %dy", Mob[arg2].Mobs.Player.Name, Mob[arg2].Target.X, Mob[arg2].Target.Y);
		Log(arg2, LOG_INGAME, "Matou %s em %dx %dy", Mob[arg1].Mobs.Player.Name, Mob[arg1].Target.X, Mob[arg1].Target.Y);

		INT32 LOCAL_36 = GetArena(Mob[arg1].Target.X, Mob[arg1].Target.Y);
		INT32 LOCAL_37 = GetVillage(Mob[arg1].Target.X, Mob[arg1].Target.Y);
		INT32 warArea = -1;

		for(INT32 i = 0 ; i < 5; i++)
		{
			if(Mob[arg1].Target.X >= g_pCityZone[i].war_min_x && Mob[arg1].Target.X <= g_pCityZone[i].war_max_x && Mob[arg1].Target.Y >= g_pCityZone[i].war_min_y && g_pCityZone[i].war_max_y)
			{
				warArea = i;
				break;
			}
		}

		INT32 LOCAL_38 = 0;

		if(LOCAL_31 == 0 && LOCAL_32 == 31)
			LOCAL_38 = 1;

		if(LOCAL_36 == 5 && LOCAL_37 == 5 && LOCAL_38 == 0)
		{
			if(LOCAL_29 != 0)
				LOCAL_20 = LOCAL_20 + ((LOCAL_20 * LOCAL_26) >> 2);

			if(LOCAL_20 > 150000)
				LOCAL_20 = 150000;
			
			// 0045B47B
			if(LOCAL_20 < 0)
				LOCAL_20 = 0;
			
			if(Mob[arg1].Target.X >= 2447 && Mob[arg1].Target.X <= 2545 && Mob[arg1].Target.Y >= 1851 && Mob[arg1].Target.Y <= 1924)
				LOCAL_20 = 0;

			if (!sServer.KefraDead && Mob[arg1].Target.X >= 2176 && Mob[arg1].Target.X <= 2559 && Mob[arg1].Target.Y >= 3840 && Mob[arg1].Target.Y <= 4096)
				LOCAL_20 = 0;

			if (Mob[arg1].Target.X >= 1041 && Mob[arg1].Target.X <= 1248 && Mob[arg1].Target.Y >= 1950 && Mob[arg1].Target.Y <= 2158)
				LOCAL_20 = 0;

			if(LOCAL_30 != 0)
				LOCAL_20 = 0;

            if (Mob[arg1].Mobs.Info.LvBlocked)
                LOCAL_20 = 0;

			if(LOCAL_16	>= 20 || Mob[arg1].Mobs.Player.Equip[0].EFV2 >= ARCH)
			{
				if(arg2 < MAX_PLAYER)
					Mob[arg1].Mobs.Player.Exp = Mob[arg1].Mobs.Player.Exp - LOCAL_20;
				else if(sServer.DeadPoint != 0)
					Mob[arg1].Mobs.Player.Exp = Mob[arg1].Mobs.Player.Exp - LOCAL_20;

				if(Mob[arg1].Mobs.Player.Exp < 0)
					Mob[arg1].Mobs.Player.Exp = 0;

				if(sServer.DeadPoint != 0 || arg2 < MAX_PLAYER)
				{
					sprintf_s(temp, "Você perdeu %I64d de experiência", LOCAL_20);
					SendClientMessage(arg1, temp);

					SendEtc(arg1);

					Log(arg1, LOG_INGAME, "Perdeu %I64d de experiência.", LOCAL_20);
				}

				while (Mob[arg1].Mobs.Player.bStatus.Level > 0 && Mob[arg1].Mobs.Player.Exp < g_pNextLevel[Mob[arg1].Mobs.Player.Equip[0].EFV2][Mob[arg1].Mobs.Player.bStatus.Level - 1])
					Mob[arg1].Mobs.Player.bStatus.Level--;

				SendEtc(arg1);
			}
			else
				SendClientMessage(arg1, "Level abaixo de 20 não perde experiência."); // No caso, MeessageString, ou seilá o nome

			if(LOCAL_16 >= 99 && arg2 < MAX_PLAYER && LOCAL_30 == 0)
			{ // Nível acima de 100
				INT64 holdValue = 0;
				INT32 classMaster = Mob[arg1].Mobs.Player.Equip[0].EFV2;
				if(classMaster == 1 || classMaster == 2)
					holdValue = 15000;
				else 
					holdValue = 50000;
				
				if(Mob[arg1].Target.X >= 2447 && Mob[arg1].Target.X <= 2545 && Mob[arg1].Target.Y >= 1850 && Mob[arg1].Target.Y <= 1921)
					holdValue = 0;

				if (Mob[arg1].Target.X >= 1041 && Mob[arg1].Target.X <= 1248 && Mob[arg1].Target.Y >= 1950 && Mob[arg1].Target.Y <= 2158)
					holdValue = 0;

				for (INT32 g = 0; g < MAX_MESSAGE; g++)
				{
					if (Mob[arg1].Target.X >= g_pPositionCP[g].Min.X && Mob[arg1].Target.X <= g_pPositionCP[g].Max.X && Mob[arg1].Target.Y >= g_pPositionCP[g].Min.Y && Mob[arg1].Target.Y <= g_pPositionCP[g].Max.Y)
					{
						holdValue = 0;

						break;
					}
				}

				if (Mob[arg1].Target.X >= 2180 && Mob[arg1].Target.X <= 2558 && Mob[arg1].Target.Y >= 3837 && Mob[arg1].Target.Y <= 4096)
					holdValue = 0;

				if (Mob[arg1].Target.X >= 2448 && Mob[arg1].Target.X <= 2545 && Mob[arg1].Target.Y >= 1850 && Mob[arg1].Target.Y <= 1921 && sServer.TowerWar.Status)
					holdValue = 0;

				if (LOCAL_30 == 0)
					holdValue = 0;

				if(holdValue + Mob[arg1].Mobs.Hold > (g_pNextLevel[classMaster][LOCAL_16]))
					Mob[arg1].Mobs.Hold = g_pNextLevel[classMaster][LOCAL_16];
				else
					Mob[arg1].Mobs.Hold += holdValue;

				LOCAL_13.Hold = static_cast<int>(Mob[arg1].Mobs.Hold);
			}

			if(LOCAL_29 != 0 && warArea == -1)
			{
				SetCurKill(arg2, LOCAL_21 + 1);
				SetTotKill(arg2, LOCAL_22 + 1);

				SendItem(arg2, SlotType::Inv, 63, &Mob[arg2].Mobs.Player.Inventory[63]);

				if(LOCAL_30 == 0 && LOCAL_25 == 0)
				{
					INT32 LOCAL_42 = (-(LOCAL_24 / 20)) * 3;

					if(LOCAL_42 < -3)
						LOCAL_42 = -3;

					if(LOCAL_42 > 0)
						LOCAL_42 = 0;

					if(LOCAL_25 > 0)
						LOCAL_42 = 0;
					
					for(INT32 g = 0; g < MAX_MESSAGE; g++)
					{
						if(Mob[arg1].Target.X >= g_pPositionCP[g].Min.X && Mob[arg1].Target.X <= g_pPositionCP[g].Max.X && Mob[arg1].Target.Y >= g_pPositionCP[g].Min.Y && Mob[arg1].Target.Y <= g_pPositionCP[g].Max.Y)
						{
							LOCAL_42 = 0;

							break;
						}
					}
 
					if(Mob[arg1].Target.X >= 2180 && Mob[arg1].Target.X <= 2558 && Mob[arg1].Target.Y >= 3837 && Mob[arg1].Target.Y <= 4096)
						LOCAL_42 = 0;

					if (Mob[arg1].Target.X >= 2448 && Mob[arg1].Target.X <= 2545 && Mob[arg1].Target.Y >= 1850 && Mob[arg1].Target.Y <= 1924 && sServer.TowerWar.Status)
						LOCAL_42 = 0;
						
					Log(arg2, LOG_INGAME, "CP %d diminuiu em %d", LOCAL_23 - 75, LOCAL_42);
					LogPlayer(arg2, "CP %d foi diminuído em %d", LOCAL_23 - 75, LOCAL_42);

					LOCAL_23 = LOCAL_23 + LOCAL_42;

					SetPKPoint(arg2, LOCAL_23);

					sprintf_s(temp, "CP %d diminuiu em %d", LOCAL_23 - 75, LOCAL_42);
					SendClientMessage(arg2, temp);
				}
			}
			else if(LOCAL_30 == 0 && arg2 < MAX_PLAYER && warArea == -1) // 0045B74F
			{
				INT32 LOCAL_43 = (-(LOCAL_24 / 25) * 3);

				if(LOCAL_43 < -3)
					LOCAL_43 = -3;

				if(LOCAL_43 > 0)
					LOCAL_43 = 0;

				if(LOCAL_25 > 0)
					LOCAL_43 = 0;

				if(Mob[arg1].Mobs.Player.Equip[15].Index == 548 || Mob[arg1].Mobs.Player.Equip[15].Index == 549 || Mob[arg2].Mobs.Player.Equip[15].Index == 548 || Mob[arg2].Mobs.Player.Equip[15].Index == 549)
					LOCAL_43 = LOCAL_43 * 3;
	
				if(Mob[arg1].Target.X >= 2180 && Mob[arg1].Target.X <= 2541 && Mob[arg1].Target.Y >= 3858 && Mob[arg1].Target.Y <= 4051)
					LOCAL_43 = 0;
				
				for(INT32 g = 0; g < MAX_MESSAGE; g++)
				{
					if(Mob[arg1].Target.X >= g_pPositionCP[g].Min.X && Mob[arg1].Target.X <= g_pPositionCP[g].Max.X && Mob[arg1].Target.Y >= g_pPositionCP[g].Min.Y && Mob[arg1].Target.Y <= g_pPositionCP[g].Max.Y)
					{
						LOCAL_43 = 0;

						break;
					}
				}

				if (Mob[arg1].Target.X >= 2448 && Mob[arg1].Target.X <= 2545 && Mob[arg1].Target.Y >= 1850 && Mob[arg1].Target.Y <= 1921 && sServer.TowerWar.Status)
					LOCAL_43 = 0;

				LOCAL_23 = LOCAL_23 + LOCAL_43;

				SetPKPoint(arg2, LOCAL_23);

				if(LOCAL_43 != 0)
				{
					sprintf_s(temp, "CP %d diminuiu em %d", LOCAL_23 - 75, -LOCAL_43);

					SendClientMessage(arg2, temp);
					Log(arg2, LOG_INGAME, "CP %d diminui em %d", LOCAL_23 - 75, -LOCAL_43);
				}
				
				/*
				if(LOCAL_24 <= 60)
				{
					INT32 LOCAL_44 = (75 - LOCAL_24) / 10;
					INT32 LOCAL_45 = 0;
					for(INT32 LOCAL_46 = 0; LOCAL_46 < 63; LOCAL_46 ++)
					{
						if(Rand() % 5)
							continue;
						INT32 LOCAL_47 = Mob[arg1].Target.X;
						INT32 LOCAL_48 = Mob[arg1].Target.Y;
							
						st_Item *LOCAL_49 = &Mob[arg1].Mobs.Player.Inventory[LOCAL_46];
						if(LOCAL_49->Index <= 0 || LOCAL_49->Index > MAX_ITEMLIST)
							continue;
						if(LOCAL_49->Index == 508 || LOCAL_49 ->Index == 509 || LOCAL_49->Index == 522 || LOCAL_49->Index == 531 || LOCAL_49->Index == 446)
							continue;
						INT32 LOCAL_50 = Rand() & 0x80000003;
						INT32 LOCAL_51 = CreateItem(LOCAL_47, LOCAL_48, LOCAL_49, 1, 1);
						if(LOCAL_51 >= 5000 || LOCAL_51 <= 0)
							continue;
							
						memset(LOCAL_49, 0, 8);
						// INICIO DO BUFFER -> EBP - 108h
						char buffer[28];
						*(WORD*)&buffer[4] = 0x175;
						*(WORD*)&buffer[0] = 28;
						*(DWORD*)&buffer[12] = 1;
						*(DWORD*)&buffer[16] = LOCAL_46;
						*(DWORD*)&buffer[20] = LOCAL_50;
						*(WORD*)&buffer[24] = LOCAL_47;
						*(WORD*)&buffer[26] = LOCAL_48;
						GridMulticast_2(Mob[arg2].Target.X, Mob[arg2].Target.Y, (BYTE*)&buffer, 28);
						LOCAL_45 = LOCAL_45 + 1;
						// Revisar esse RAND \/
						if(!Rand() & 0x80000001)
							break;
						if(LOCAL_45 >= LOCAL_44)
							break;
					}
				}
					
				if(LOCAL_24 <= 35)
				{
					INT32 LOCAL_67 = (LOCAL_24 + 10) / 10;
					if(LOCAL_67 <= 0)
						LOCAL_67 = 1;
					INT32 LOCAL_68 = Rand() % LOCAL_67;
					//0045BB71
					INT32 LOCAL_69;
					// Não entendi direito essa parte, tá muito confuso
					while(!LOCAL_68)
					{ 
						LOCAL_69 = Rand() % 14 + 1;
						if(LOCAL_69 != 12)
						{
							INT32 LOCAL_70 = Mob[arg1].Target.X;
							INT32 LOCAL_71 = Mob[arg1].Target.Y;
							st_Item *LOCAL_72 = &Mob[arg1].Mobs.Player.Equip[LOCAL_69];
							if(LOCAL_72->Index <= 0  || LOCAL_72->Index > MAX_ITEMLIST)
								break;
								
							if(LOCAL_72->Index == 508 || LOCAL_72->Index == 509 || LOCAL_72->Index == 522 || LOCAL_72->Index == 531 || LOCAL_72->Index == 446)
								break;
							INT32 LOCAL_73 = Rand() & 0x80000003;
							INT32 LOCAL_74 = CreateItem(LOCAL_70, LOCAL_71, LOCAL_72, 1, 1);
							if(LOCAL_74 >= 5000 || LOCAL_74 <= 0)
								break;
							char buffer[28];
							*(WORD*)&buffer[4] = 0x175;
							*(WORD*)&buffer[0] = 28;
							*(DWORD*)&buffer[12] = 0;
							*(DWORD*)&buffer[16] = LOCAL_69;
							*(DWORD*)&buffer[20] = LOCAL_73;
							*(WORD*)&buffer[24] = LOCAL_70;
							*(WORD*)&buffer[26] = LOCAL_71;
								
							GridMulticast_2(Mob[arg2].Target.X, Mob[arg2].Target.Y, (BYTE*)&buffer, 28);
						}
					}
				}
					*/
				p364 LOCAL_116;
				GetCreateMob(arg2, (BYTE*)&LOCAL_116);

				GridMulticast_2(Mob[arg2].Target.X, Mob[arg2].Target.Y, (BYTE*)&LOCAL_116, 0);

				if(LOCAL_29 != 0)
					SetCurKill(arg1, 0);
			}
		}
		else
		{
			//0045BF43 - Você não pode perder experiência nesta área
			SendClientMessage(arg1, "Você não pode perder experiência nesta área");
		}

		if(Mob[arg1].Mobs.Player.Equip[13].Index == 753 || Mob[arg1].Mobs.Player.Equip[13].Index == 1726)
		{
			const auto& familiarItem = Mob[arg1].Mobs.Player.Equip[13];
			INT32 LOCAL_123 = GetItemSanc(&Mob[arg1].Mobs.Player.Equip[13]);

			if (LOCAL_123 <= 0)
			{
				Log(arg1, LOG_INGAME, "%s %s foi destruído", ItemList[familiarItem.Index].Name, familiarItem.toString().c_str());
				memset(&Mob[arg1].Mobs.Player.Equip[13], 0, 8);
			}
			else
			{
				LOCAL_123 --;
				if(Mob[arg1].Mobs.Player.Equip[13].Effect[0].Index == EF_SANC)
					Mob[arg1].Mobs.Player.Equip[13].Effect[0].Value = LOCAL_123;
				else if(Mob[arg1].Mobs.Player.Equip[13].Effect[1].Index == EF_SANC)
					Mob[arg1].Mobs.Player.Equip[13].Effect[1].Value = LOCAL_123;
				else if(Mob[arg1].Mobs.Player.Equip[13].Effect[2].Index == EF_SANC)
					Mob[arg1].Mobs.Player.Equip[13].Effect[2].Value = LOCAL_123;

				Log(arg1, LOG_INGAME, "%s %s perdeu uma refinação. Refinações restantes: %d", ItemList[familiarItem.Index].Name, familiarItem.toString().c_str(), LOCAL_123);
			}

			SendItem(arg1, SlotType::Equip, 13, &Mob[arg1].Mobs.Player.Equip[13]);
		}
		
		GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);
		return;
	}

	//0045C208
	if(arg2 >= MAX_PLAYER || Mob[arg1].Mobs.Player.CapeInfo == 4)
	{
		GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

		DeleteMob(arg1, 1);
		return;
	}

	//0045C267
	auto LOCAL_124 = GetExpApply(Mob[arg1].Mobs.Player.Exp, arg2, arg1);
	INT32 LOCAL_125 = 0;
	/*
	if(Mob[arg2].ExpBonus > 0 && Mob[arg2].ExpBonus < 20)
		LOCAL_125 = LOCAL_124 * Mob[arg2].ExpBonus / 100;
		*/
	UINT32 LOCAL_126 = 30;
	UINT32 LOCAL_127 = 0;
	UINT32 LOCAL_128 = 0;
	UINT32 LOCAL_129 = Mob[arg2].Target.X;
	UINT32 LOCAL_130 = Mob[arg2].Target.Y;
	UINT32 LOCAL_131;
	UINT32 LOCAL_132 = 0;

	for(; LOCAL_132 < 13; LOCAL_132++)
	{
		if(LOCAL_132 == 12)
			LOCAL_131 = LOCAL_14;
		else
			LOCAL_131 = Mob[LOCAL_14].PartyList[LOCAL_132];

		if(LOCAL_131 <= 0 || LOCAL_131 >= MAX_PLAYER)
			continue;

		if (LOCAL_129 < (Mob[LOCAL_131].Target.X - HALFGRIDX))
			continue;
		if (LOCAL_129 >(Mob[LOCAL_131].Target.X + HALFGRIDX))
			continue;
		if (LOCAL_130 < (Mob[LOCAL_131].Target.Y - HALFGRIDY))
			continue;
		if (LOCAL_130 >(Mob[LOCAL_131].Target.Y + HALFGRIDY))
			continue;

		LOCAL_127 = Mob[LOCAL_131].Mobs.Player.Status.Level +  LOCAL_126 + LOCAL_127;
		LOCAL_128 ++;
	}

	if(LOCAL_128 <= 0 || LOCAL_128 > 13 || LOCAL_127 <= 0)
	{
		GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

		DeleteMob(arg1, 1);
		return;
	}
	
	static INT32 PARTYBONUS [13] = {100, 100, 200, 300, 400, 500, 500, 500, 500, 500, 500, 500, 500};
	static INT32 g_EmptyMob = MAX_PLAYER;
	static INT32 DOUBLEMODE = 0; // 758FAC8

	//0045C4D2
	// TODO : Aqui na tm tem um provável erro
	// Ele acessa EmptyMob como array e o PARTYBONUS como valor 
	long long LOCAL_133 = PARTYBONUS[LOCAL_128];
	if(LOCAL_128 > 1)
		LOCAL_133 = LOCAL_133 + g_EmptyMob - 100;

	long long LOCAL_134 = LOCAL_124;
	long long LOCAL_135 = LOCAL_124 * LOCAL_133 / 25;

	for(LOCAL_132 = 0; LOCAL_132 < 13; LOCAL_132 ++)
	{
		if(LOCAL_132 == 0)
			LOCAL_131 = LOCAL_14;
		else
			LOCAL_131 = Mob[LOCAL_14].PartyList[LOCAL_132 - 1];

		if(LOCAL_131 <= 0 || LOCAL_131 >= MAX_PLAYER)
			continue;

		if(!Mob[LOCAL_131].Mobs.Player.Status.curHP)
			continue;
		
		if((Mob[LOCAL_131].Target.X < 1152 || Mob[LOCAL_131].Target.X > 1282 || Mob[LOCAL_131].Target.Y < 130 || Mob[LOCAL_131].Target.Y > 217) &&
			(Mob[LOCAL_131].Target.X < 1049 || Mob[LOCAL_131].Target.X > 1130 || Mob[LOCAL_131].Target.Y < 272 || Mob[LOCAL_131].Target.Y > 334))
		{
			if(LOCAL_129 < (Mob[LOCAL_131].Target.X - HALFGRIDX))
				continue;
			if (LOCAL_129 > (Mob[LOCAL_131].Target.X + HALFGRIDX))
				continue;
			if (LOCAL_130 < (Mob[LOCAL_131].Target.Y - HALFGRIDY))
				continue;
			if (LOCAL_130 > (Mob[LOCAL_131].Target.Y + HALFGRIDY))
				continue;
		}

		long long LOCAL_136 = ((Mob[LOCAL_131].Mobs.Player.Status.Level + LOCAL_126) * LOCAL_135) / LOCAL_127;
		if(LOCAL_136 < 0 || LOCAL_136 > 500000000)
			LOCAL_136 = 500000000;

		LOCAL_136 = LOCAL_136 * 6 / 10;
		if(LOCAL_136 > LOCAL_134)
			LOCAL_136 = LOCAL_134;

		LOCAL_136 += LOCAL_126;
		LOCAL_136 = GetExpApply_2(LOCAL_136, LOCAL_131, arg2, true);

		INT64 hold = Mob[LOCAL_131].Mobs.Hold;
		if(hold > 0)
		{
			if(hold > LOCAL_136)
			{
				hold -= LOCAL_136;
				LOCAL_136 = 0;
			}
			else
			{
				LOCAL_136 -= static_cast<int>(hold);
				hold = 0;
			}

			Mob[LOCAL_131].Mobs.Hold = hold;
		}

		Mob[LOCAL_131].CheckQuarter(LOCAL_136);
		Mob[LOCAL_131].Mobs.Player.Exp += LOCAL_136;
		Mob[LOCAL_131].CheckGetLevel();
	}

	auto christmasMission = static_cast<TOD_ChristmasMission*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::ChristmasMission));
	if(christmasMission != nullptr)
		christmasMission->RefreshChristmasMission(arg2, arg1);

	auto mission = static_cast<TOD_Mission*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::Mission));
	if (mission != nullptr)
		mission->MobKilled(arg2, arg1);

	auto preWar = static_cast<TOD_PreWarBoss*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::PreWarBoss));
	if (preWar != nullptr)
		preWar->MobKilled(arg2, arg1);

	INT32 LOCAL_137 = Mob[arg1].GenerateID;
	INT32 LOCAL_138 = Rand() & 0x80000003;

	if(LOCAL_137 >= 0 && mGener.pList[LOCAL_137].DieAction[LOCAL_138][0] && Mob[arg1].Leader == 0)
		SendSay(arg1, mGener.pList[LOCAL_137].DieAction[LOCAL_138]);

	GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, (BYTE*)&LOCAL_13, 0);

	// 0045C809
	INT32 LOCAL_139 = Mob[arg2].Mobs.Player.CapeInfo;
	if(LOCAL_139 != 0 && LOCAL_139 != 4 && LOCAL_139 != CAPE_BLUE && LOCAL_139 != CAPE_RED)
	{
		DeleteMob(arg1, 1);
		return;
	}

	INT32 LOCAL_140 = arg3;
	if(LOCAL_140 == 0)
		LOCAL_140 = Mob[arg1].Target.X;

	INT32 LOCAL_141 = arg4;
	if(LOCAL_141 == 0)
		LOCAL_141 = Mob[arg1].Target.Y;

	INT32 LOCAL_142 = Mob[arg1].GenerateID;
	if(Mob[arg1].Mobs.Player.Equip[0].Index == 219 && LOCAL_142 != GUARDIAN_TOWER_BLUE && LOCAL_142 != GUARDIAN_TOWER_RED)
	{
		if(Mob[arg1].Target.X < 0 || Mob[arg1].Target.X >= 4096 || Mob[arg1].Target.Y < 0 || Mob[arg1].Target.Y >= 4096)
		{
			Log(SERVER_SIDE, LOG_ERROR, "Nenhum portão do castelo para abrir na posição.");
			// 0045C958

			return;
		}
		
		INT32 LOCAL_143 = g_pItemGrid[Mob[arg1].Target.Y][Mob[arg1].Target.X];
		if(LOCAL_143 < 0 || LOCAL_143 >= 5000 || pInitItem[LOCAL_143].Open == 0)
		{
			Log(SERVER_SIDE, LOG_ERROR, "Nenhum portão do castelo para abrir.");
			return;
		}

		int LOCAL_144;
		INT32 LOCAL_145 = UpdateItem(LOCAL_143, 1, &LOCAL_144);

		//
		if(LOCAL_145 != 0)
		{
			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_143 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = pInitItem[LOCAL_143].Status;
			LOCAL_150.unknow = 0;

			GridMulticast_2(pInitItem[LOCAL_143].PosX, pInitItem[LOCAL_143].PosY, (BYTE*)&LOCAL_150, 0);

			pInitItem[LOCAL_143].IsOpen = 0;
		}
	}
	// 0045CE87
	if(LOCAL_142 == 3 && Mob[arg1].Mobs.Player.Equip[0].Index == 258)
	{
		Log(arg2, LOG_INGAME, "Derrotou Lorde Zakum");

		char szTMP[256];

		sprintf_s(szTMP, g_pLanguageString[_SN_Zakum_Killed], Mob[arg2].Mobs.Player.Name);

		//0045CEAF
		SendNoticeArea(szTMP, 2176, 1160, 2300, 1276);

		sServer.Zakum.Clear = 1;
		INT32 LOCAL_170 = Mob[arg1].Target.X,
			  LOCAL_171 = Mob[arg1].Target.Y;

		st_Item LOCAL_173;
		memset(&LOCAL_173, 0, 8);

		LOCAL_173.Index = 753;
		
		INT32 LOCAL_174 = (Rand() % 5) - 1;
		INT32 LOCAL_175 = (Rand() % 5) - 1;
		INT32 LOCAL_176 = 0;
		INT32 LOCAL_177 = 174 + LOCAL_175 + LOCAL_176;
		INT32 LOCAL_178 = 0;

		if(LOCAL_177 < 0)
			LOCAL_177 = 0;

		if(LOCAL_177 < 9)
			LOCAL_178 = Rand() % 10;
		
		LOCAL_173.Effect[0].Index = EF_SANC;
		SetItemSanc(&LOCAL_173, LOCAL_177, LOCAL_178);

		for (INT32 LOCAL_780 = 1; LOCAL_780 <= 2; LOCAL_780++)
		{
			INT32 LOCAL_781 = Rand() & 0x80000007;
			if (LOCAL_781 == 0)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_HP;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() % 41) + 20;
			}
			else if (LOCAL_781 == 1)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_DAMAGE;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() % 21) + 5;
			}
			else if (LOCAL_781 == 2)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_ATTSPEED;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() % 11) + 5;
			}
			else if (LOCAL_781 == 3)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_MP;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() % 51) + 20;
			}
			else if (LOCAL_781 == 4)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_MAGIC;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() % 7) + 2;
			}
			else if (LOCAL_781 == 5)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_STR;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() & 0x8000000F) + 5;
			}
			else if (LOCAL_781 == 6)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_INT;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() & 0x8000000F) + 5;
			}
			else if (LOCAL_781 == 7)
			{
				LOCAL_173.Effect[LOCAL_780].Index = EF_DEX;
				LOCAL_173.Effect[LOCAL_780].Value = (Rand() & 0x8000000F) + 5;
			}
		}

		INT32 slotId = GetFirstSlot(arg2, 0);
		if(slotId != -1)
		{
			memset(&Mob[arg2].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);
			memcpy(&Mob[arg2].Mobs.Player.Inventory[slotId], &LOCAL_173, sizeof st_Item);

			SendItem(arg2, SlotType::Inv, slotId, &LOCAL_173);

			Log(arg2, LOG_INGAME, "Item IMP adicionado corretamente ao inventário [%d] [%d %d %d %d %d %d]", LOCAL_173.Index, LOCAL_173.EF1, LOCAL_173.EFV1, LOCAL_173.EF2,
				LOCAL_173.EFV2, LOCAL_173.EF3, LOCAL_173.EFV3);

			LogPlayer(arg2, "Recebido IMP na quest Lorde Zakum");
		}
		//CreateItem(LOCAL_170, LOCAL_171, &LOCAL_173, Rand() & 0x080000003, 1);
	}
	else if(LOCAL_142 >= TORRE_NOATUN && LOCAL_142 < TORRE_NOATUN + 3)
	{// 0045D057
		INT32 LOCAL_179 = LOCAL_142 - TORRE_NOATUN;
		sServer.LiveTower[LOCAL_179] = 0;

		for(INT32 LOCAL_180 = 0 ; LOCAL_180 < 3; LOCAL_180++)
		{
			if(sServer.LiveTower[LOCAL_180] != 0)
				break;
		}
	}
	// 0045D0C6
	else if(LOCAL_142 == 8 || LOCAL_142 == 9)
	{
		int index = LOCAL_142 - 8;

		if (sServer.KingdomBattle.Info[index].isTowerAlive)
		{
			SendClientMessage(arg2, "Para derrotar o Rei, é necessário passar pela Torre Guardiã!");

			Mob[arg1].Mobs.Player.Status.curHP = Mob[arg1].Mobs.Player.Status.maxHP;

			p364 packet{};
			GetCreateMob(arg1, reinterpret_cast<BYTE*>(&packet));
			
			GridMulticast_2(Mob[arg1].Target.X, Mob[arg1].Target.Y, reinterpret_cast<BYTE*>(&packet), 0);
			return;
		}
	
		if (LOCAL_142 == 8)
			sServer.KingdomBattle.Winner = CAPE_RED;
		else
			sServer.KingdomBattle.Winner = CAPE_BLUE;

		SendNotice("O Reino %s conquistou o reino %s", LOCAL_142 == 8 ? "Red" : "Blue", LOCAL_142 == 8 ? "Blue" : "Red");

		Log(SERVER_SIDE, LOG_INGAME, "O Reino %s conquistou o reino %s", LOCAL_142 == 8 ? "Red" : "Blue", LOCAL_142 == 8 ? "Blue" : "Red");

		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			Mob[i].GetCurrentScore(i);
			SendScore(i);
		}
		
		sServer.KingdomBattle.Info[index].Status = false;

		SendKingdomBattleInfo(SERVER_SIDE, CAPE_BLUE, sServer.KingdomBattle.Info[0].Status);
		SendKingdomBattleInfo(SERVER_SIDE, CAPE_RED, sServer.KingdomBattle.Info[1].Status);
	}
	else if (LOCAL_142 == GUARDIAN_TOWER_BLUE || LOCAL_142 == GUARDIAN_TOWER_RED)
	{
		SendNotice("A Torre Guardiã do Reino %s foi derrotada", LOCAL_142 == GUARDIAN_TOWER_BLUE ? "Blue" : "Red");

		sServer.KingdomBattle.Info[LOCAL_142 - GUARDIAN_TOWER_BLUE].isTowerAlive = false;
		sServer.KingdomBattle.Info[LOCAL_142 - GUARDIAN_TOWER_BLUE].TowerId = 0;
	}
	else if(LOCAL_142 == 0 || LOCAL_142 == 1 || LOCAL_142 == 2)
	{
		INT32 LOCAL_181 = Rand() % 14;
		INT32 LOCAL_182 = 0;
	
		if(LOCAL_181 == 0)
			LOCAL_182 = 420;
		
		if(LOCAL_181 == 1)
			LOCAL_182 = 421;

		if(LOCAL_182 != 0)
		{
			st_Item LOCAL_184;
			memset(&LOCAL_184, 0, 8);

			LOCAL_184.Index = LOCAL_182;

			SetItemBonus(&LOCAL_184, 0, 0, 0);

			PutItem(arg2, &LOCAL_184);
		}
	}
	else if(LOCAL_142 == 5 || LOCAL_142 == 6 || LOCAL_142 == 7)
	{
		INT32 LOCAL_185 = Rand() % 14,
			  LOCAL_186 = 0;

		if(LOCAL_185 == 0)
			LOCAL_186 = Rand() % 7 + 421;

		if(LOCAL_186 != 0)
		{
			st_Item LOCAL_188;
			memset(&LOCAL_188, 0, 8);

			LOCAL_188.Index = LOCAL_186;
			
			SetItemBonus(&LOCAL_188, 0, 0, 0);
			
			PutItem(arg2, &LOCAL_188);
				//CreateItem(LOCAL_140, LOCAL_141, &LOCAL_188, Rand() & 0x80000003, 1);
		}
	}
	else if(LOCAL_142 == 3)
	{
		INT32 LOCAL_189 = Rand() % 7;
		INT32 LOCAL_190 = 0;

		if(LOCAL_189 == 0)
			LOCAL_190 = 1106;
		if(LOCAL_189 == 1)
			LOCAL_190 = 1256;
		if(LOCAL_189 == 2)
			LOCAL_190 = 1418;
		if(LOCAL_189 == 3)
			LOCAL_190 = 1568;

		if(LOCAL_190 != 0)
		{
			st_Item LOCAL_192;
			memset(&LOCAL_192, 0, sizeof st_Item);

			LOCAL_192.Index = LOCAL_190;

			SetItemBonus(&LOCAL_192, 75, 1, 0);
			
			PutItem(arg2, &LOCAL_192);
		}
	}

	else if(LOCAL_142 == VALKY_ID  || LOCAL_142 == HELL_BOSS || LOCAL_142 == COELHO_ID || LOCAL_142 == BARLOG)
	{
		INT32 pistaId = 2;
		if(LOCAL_142 == HELL_BOSS)
			pistaId = 4;
		else if(LOCAL_142 == COELHO_ID)
			pistaId = 6;
		else if(LOCAL_142 == BARLOG)
			pistaId = 5;

		// Teleporta para o boss
		stPista *pista = &pPista[pistaId];

		int party = -1;
		for(int i = 0; i < MAX_PARTYPISTA;i ++)
		{
			for(int x = 0; x < 13;x++)
			{
				if(pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

        static std::map<int, std::string> bossName
        {
            { VALKY_ID, "Valkyria" },
            { HELL_BOSS, "Hell" },
            { COELHO_ID, "Coelho" },
            { BARLOG, "Barlog" },
        };

		if (party != -1)
		{
			GiveRuna(pistaId, party);
		
			for (int i = 0; i < MAX_PARTYPISTA; ++i)
			{
				if (i == party)
					continue;

                LogRune(pistaId, i, "O boss "s + bossName[LOCAL_142] + "foi morto pelo grupo "s + std::to_string(party) + ". Este grupo não receberá a premiação. Morto por "s + Mob[arg2].Mobs.Player.Name);
                MessageRune(pistaId, i, "O boss foi derrotado por outro grupo.");
			}

			LogRune(pistaId, party, "O Boss "s + bossName[LOCAL_142] + " foi morto pelo jogador "s + Mob[arg2].Mobs.Player.Name + " do grupo.");
		}
		else
		{
            for (int i = 0; i < MAX_PARTYPISTA; ++i)
                LogRune(pistaId, i, "O Boss "s + bossName[LOCAL_142] + " foi morto pelo jogador "s.append(Mob[arg2].Mobs.Player.Name).append(" que não estava registrado na pista."));
		}
	}
	else if(LOCAL_142 >= KALINTZ_MAGE && LOCAL_142 <= KALINTZ_MAGE +7 )
	{
		stPista *pista = &pPista[3];

		int party = -1;
		for(int i = 0; i < MAX_PARTYPISTA;i ++)
		{
			for(int x = 0; x < 13;x++)
			{
				if(pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

		if(party != -1)
		{
			pista->inSec.Points[party] ++;

			int generateMob = SULRANG + Rand() % 3;
			if(generateMob > 189)
				generateMob --;

			GenerateMob(generateMob, 0, 0);
			LogRune(3, party, "Derrotou o KALINTZ e adquiriu 1 ponto para a equipe. Total de pontos: "s + (std::to_string(pista->inSec.Points[party])));
		}
	}
	else if(LOCAL_142 >= SULRANG && LOCAL_142 < SULRANG + 2)
	{
		int _rand = Rand () % 8;
		int generateIdd = KALINTZ_MAGE + _rand;

		GenerateMob(generateIdd, 0, 0);

		const stPista *pista = &pPista[3];
		int party = -1;
		for (int i = 0; i < MAX_PARTYPISTA; i++)
		{
			for (int x = 0; x < 13; x++)
			{
				if (pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

		if (party != -1)
			Log(3, party, "Derrotou o SULRANG. Renascendo spot de Kalintz");
	}
	else if(LOCAL_142 == LICH_ID)
	{
		stPista *pista = &pPista[0];

		int party = -1;
		for(int i = 0; i < MAX_PARTYPISTA;i ++)
		{
			for(int x = 0; x < 13;x++)
			{
				if(pista->Clients[i][x] == arg2)
				{
					party = i;
					break;
				}
			}
		}

		bool reborn = true;
		if(party != -1)
		{
			int _rand = rand() % 100;
			if(_rand <= 30)
			{
				GiveRuna(0, party);
				reborn = false;
			}
		}

		for(int i = 1000; i < 30000; i++)
		{
			if(Mob[i].GenerateID == LICH_ID)
				DeleteMob(i, 1);
		}

		if(reborn)
			GenerateMob(LICH_ID, 0, 0);
	}
	else if (LOCAL_142 >= TORRE_ID && LOCAL_142 < TORRE_ID + 3)
	{
		int groupIndex = LOCAL_142 - TORRE_ID;

		const auto pista = &pPista[1];
		for (int i = 0; i < 13; ++i)
		{
			auto memberId = pista->Clients[groupIndex][i];
			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			Log(memberId, LOG_INGAME, "A Torre do grupo da Pista +1 morreu.");
			SendClientMessage(memberId, "Você não defendeu sua Torre");
		}
	}
	else if(LOCAL_142 >= HELL && LOCAL_142 < (HELL + 47))
	{
		if(LOCAL_142 != HELL_BOSS)
		{
			int indexRoom = LOCAL_142 - HELL;
			stPista *pista = &pPista[4];

			int party = -1;
			for (int i = 0; i < MAX_PARTYPISTA; i++)
			{
				for (int x = 0; x < 13; x++)
				{
					if (pista->Clients[i][x] == arg2)
					{
						party = i;

						break;
					}
				}
			}

			if(pista->inSec.RoomSorted[indexRoom] && mGener.pList[LOCAL_142].MobCount <= 1)
			{
				for(int i = 0; i < 13; i++)
				{
					int memberId = pista->Clients[party][i];
					if(memberId <= 0 || memberId >= MAX_PLAYER)
						continue;

					if(Mob[memberId].Target.X < g_pPistaCoords[4][0] || Mob[memberId].Target.X > g_pPistaCoords[4][2] || Mob[memberId].Target.Y < g_pPistaCoords[4][1] || Mob[memberId].Target.Y > g_pPistaCoords[4][3])
						continue;
					
					Teleportar(memberId, 3358, 1340);
					SendClientMessage(memberId, "Teleportado para a sala do Boss");

					Log(memberId, LOG_INGAME, "Teleportado para a área do boss +4.");
				} 

				for (int i = 0; i < MAX_PARTYPISTA; i++)
				{
					if (i == party)
						continue;

					for (int t = 0; t < 13; ++t)
					{
						int memberId = pista->Clients[i][t];
						if (memberId <= 0 || memberId >= MAX_PLAYER)
							continue;

						SendClientMessage(memberId, "A sala sorteada foi derrotada por outro grupo. Você falhou na refinação!");
					}

					LogRune(4, i, "A sala sorteada foi derrotada por outro grupo.");
				}

				if(!pista->inSec.Born)
				{
					// Gera o BOSS no local
					GenerateMob(HELL_BOSS, 0, 0);

					pista->inSec.Born = true;
				
					for(int i = 1000; i < 30000;i++)
					{
						if(Mob[i].GenerateID == HELL_BOSS)
						{
							pista->inSec.BossID = i;
							break;
						}
					}
				}
			}
			else
			{
				if (party != -1)
				{
					std::stringstream str;
					str << "Derrotou a sala " << indexRoom << ". Sala não sorteada. Mobs restantes: " << mGener.pList[LOCAL_142].MobCount - 1;

					LogRune(4, party, str.str());
				}
			}
		}
	}
	else if (LOCAL_142 == KEFRA)
	{
		if (arg2 == 29999)
		{
			DeleteMob(arg1, 1);

			return;
		}

		int killerId = arg2;
		if (arg2 >= MAX_PLAYER)
		{
			killerId = Mob[arg2].Summoner;

			if (killerId > 0 && killerId < MAX_PLAYER && Users[killerId].Status != USER_PLAY)
				killerId = 0;
		}
		
		Log(SERVER_SIDE, LOG_INGAME, "KEFRA - Kefra derrotado por %s - GuildId: %d.", Mob[killerId].Mobs.Player.Name, Mob[killerId].Mobs.Player.GuildIndex);

		if (killerId)
		{
			if (Mob[killerId].Mobs.Player.GuildIndex)
				SetGuildFame(Mob[killerId].Mobs.Player.GuildIndex, g_pGuild[Mob[killerId].Mobs.Player.GuildIndex].Fame + 100);

			_MSG_NOTIFY_KEFRA_DEATH packet;
			memset(&packet, 0, sizeof _MSG_NOTIFY_KEFRA_DEATH);

			packet.Header.ClientId = 0;
			packet.Header.PacketId = MSG_NOTIFY_KEFRA_DEATH;
			packet.Header.Size	   = sizeof _MSG_NOTIFY_KEFRA_DEATH;

			strncpy_s(packet.Name, Mob[killerId].Mobs.Player.Name, 16);

			AddMessageDB((BYTE*)&packet, sizeof _MSG_NOTIFY_KEFRA_DEATH);

			if(Mob[killerId].Mobs.Player.GuildIndex != 0)
				sServer.KefraKiller = Mob[killerId].Mobs.Player.GuildIndex;
			else
				sServer.KefraKiller = 0;
		}
		
		for(INT32 i = GUARDAS_KEFRA; i < GUARDAS_KEFRA + 18; i++)
		{
			// Faz o mob não ficar nascendo após morrer 
			mGener.pList[i].MinuteGenerate = -1;
		}

		for(INT32 i = 1000; i < MAX_SPAWN_MOB; i++)
		{
			if(Mob[i].GenerateID < GUARDAS_KEFRA || Mob[i].GenerateID > GUARDAS_KEFRA + 18)
				continue;

			MobKilled(i, i, 0, 0);
		}

		sServer.KefraDead = true;

		if (sServer.KefraKiller != 0)
		{
			SendNotice("O jogador [%s] da guild [%s] derrotou Kefra", Mob[killerId].Mobs.Player.Name, g_pGuild[sServer.KefraKiller].Name.c_str());

		}
		else
			SendNotice(g_pLanguageString[_NN_Kefra_PlayerKill]);
	}
	else if(LOCAL_142 >= COLOSSEUM_ID && LOCAL_142 < (COLOSSEUM_ID + 4))
	{
		int totalPL = 0;
		int totalPO = 0;
		for(INT32 i = 1;i < MAX_PLAYER; i++)
		{
			if(Users[i].Status != USER_PLAY || Mob[i].Mobs.Player.Status.curHP <= 0)
				continue;
				
			if(Mob[i].Target.X >= 2604 && Mob[i].Target.Y >= 1708 && Mob[i].Target.X <= 2648 && Mob[i].Target.Y <= 1744)
			{
				for(INT32 t = 0; t < 64;t++)
				{
					st_Item *item = &Mob[arg1].Mobs.Player.Inventory[t];
					if(item->Index <= 0 || item->Index >= MAX_ITEMLIST)
						continue;

					INT32 _rand = Rand () % 100;
					INT32 rate = 40;

					if (sServer.Colosseum.Type == TOD_Colosseum_Type::Mystic)
						rate = 75;

					if(_rand < rate)
					{
						Log(i, LOG_INGAME, "Recebeu item %s %s no Coliseu", ItemList[item->Index].Name, item->toString());

						PutItem(i, item);
						if (item->Index == 413)
							totalPL++;
						else if (item->Index == 412)
							totalPO++;
					}
				}
			}
		}

		Log(SERVER_SIDE, LOG_INGAME, "Coliseu: Total de PLs %d. Total de POs %d", totalPL, totalPO);
	}

	if (Mob[arg1].BossInfoId != -1 && Mob[arg1].BossInfoId < sServer.Boss.size())
	{
		int connId = arg2;
		if (connId > MAX_PLAYER)
		{
			int summonerId = Mob[connId].Summoner;

			if (summonerId > 0 && summonerId < MAX_PLAYER)
				connId = summonerId;
			else
				connId = 0;
		}

		auto& boss = sServer.Boss[Mob[arg1].BossInfoId];
		INT32 guildId = Mob[connId].Mobs.Player.GuildIndex;
		if (guildId != 0)
		{
			SendNotice("O jogador %s da guild %s derrotou %s", Mob[connId].Mobs.Player.Name, g_pGuild[guildId].Name.c_str(), Mob[arg1].Mobs.Player.Name);

			if (boss.Fame != 0)
				SetGuildFame(guildId, g_pGuild[guildId].Fame + boss.Fame);

			Log(SERVER_SIDE, LOG_INGAME, "%s foi derrotado por %s (%s-%d). Fame: %d. Fame ganha: %d", Mob[arg1].Mobs.Player.Name, Mob[connId].Mobs.Player.Name, g_pGuild[guildId].Name.c_str(), guildId, g_pGuild[guildId].Fame, boss.Fame);
		}
		else
		{
			SendNotice("O jogador %s derrotou o %s", Mob[arg2].Mobs.Player.Name, Mob[arg1].Mobs.Player.Name);
			Log(SERVER_SIDE, LOG_INGAME, "%s foi derrotado por %s, SEM GUILD", Mob[arg1].Mobs.Player.Name, Mob[connId].Mobs.Player.Name);
		}

		boss.GenerGenerated = 0;
		boss.LastUpdate = std::chrono::steady_clock::now();
	}

	INT32 LOCAL_193 = Mob[arg1].Mobs.Player.Gold;
	INT32 rate = 6;
	INT32 level = Mob[arg2].Mobs.Player.bStatus.Level;
	switch (Mob[arg2].Mobs.Player.Equip[0].EFV2)
	{
	case 1:
		if (level < 100)
			rate = 2;
		else if (level < 200)
			rate = 3;
		else if (level < 300)
			rate = 5;

		break;
	case 2:
		if (level < 100)
			rate = 2;
		else if (level < 200)
			rate = 3;
		else if (level < 300)
			rate = 5;
		break;
	}

	int mobLevel = Mob[arg1].Mobs.Player.Status.Level;
	if (mobLevel >= 250 && LOCAL_193 == 0)
	{
		LOCAL_193 = 500 + ((mobLevel - 250) * 5);
		if (LOCAL_193 > 1500)
			LOCAL_193 = 1500;
	}

	if ((Mob[arg2].Target.X >= 3600 && Mob[arg2].Target.X <= 3700 && Mob[arg2].Target.Y >= 3600 && Mob[arg2].Target.Y <= 3700) ||
		(Mob[arg2].Target.X >= 3732 && Mob[arg2].Target.X <= 3816 && Mob[arg2].Target.Y >= 3476 && Mob[arg2].Target.Y <= 3562))
		rate = 3;

	if(LOCAL_193 != 0 && (!(Rand() % rate)))
	{/*
		INT32 LOCAL_195 = LOCAL_193 >> 2;
		LOCAL_193 = LOCAL_193 + LOCAL_195 + (Rand() % LOCAL_195);
		LOCAL_193 <<= 2;
		// Parte não descompilada
		// porque o gold não dropa mais no chão, hehe
		// 0045D59E -
	*/
		INT32 gold = LOCAL_193;
		
		INT32 aux = gold >> 1;
		if(aux <= 0)
			aux = 1;

		gold = gold + aux + (Rand() % aux);
		if(sServer.GoldBonus != 0)
			gold += ((gold * sServer.GoldBonus) / 100);

		unsigned int xgold = Mob[arg2].Mobs.Player.Gold + gold;
		if(xgold > 2000000000)
		{
			int zgold = 2000000000 - Mob[arg2].Mobs.Player.Gold;
			if(zgold > 0)
			{
				if(gold > zgold)
					gold = zgold;
			}
		}

		if(gold != 0)
		{
			// Adiciona o gold ao usuário
			Mob[arg2].Mobs.Player.Gold += gold;

			if(arg2 > 0 && arg2 < MAX_PLAYER)
			{
				Users[arg2].Gold += gold;
				Users[arg2].GoldCount ++;

				if(Users[arg2].GoldCount >= 10)
					LogGold(arg2);
			}

			if(Mob[arg2].Mobs.Player.Gold == 2000000000)
				SendClientMessage(arg2, "Não possui espaço para guardar gold.");

			// Envia a atualização de gold
			SendSignalParm(arg2, arg2, 0x3AF, Mob[arg2].Mobs.Player.Gold);
		}
	}

	if (sServer.LanHouseN.MobId == arg1)
	{
		sServer.LanHouseN.MobId = 0;
		sServer.LanHouseN.Count = 0;
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			if (Mob[i].Target.X >= 3600 && Mob[i].Target.X <= 3700 && Mob[i].Target.Y >= 3600 && Mob[i].Target.Y <= 3700)
				SendClientMessage(i, "O boss da LanHouse foi morto por %s", Mob[arg2].Mobs.Player.Name);
		}
	}

	auto event = static_cast<TOD_BossEvent*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::BossEvent));
	if (event != nullptr)
	{
		// boss morreu
		if (strcmp(Mob[arg1].Mobs.Player.Name, event->GetBossName().c_str()) == 0)
		{
			event->Reset();

			Log(SERVER_SIDE, LOG_INGAME, "O boss do evento foi morto por %s", Mob[arg2].Mobs.Player.Name);
		}
	}

	if (Mob[arg2].Target.X >= 3600 && Mob[arg2].Target.X <= 3700 && Mob[arg2].Target.Y >= 3600 && Mob[arg2].Target.Y <= 3700 && Mob[arg2].Mobs.Player.CapeInfo != 4)
	{
		if (++sServer.LanHouseN.Count >= sServer.LanHouseN.TotalToKill && sServer.LanHouseN.MobId == 0)
		{
			sServer.LanHouseN.Count = 0;

			std::array<st_Position, 4> positions =
			{
				{
					{3610,3685},
					{3610, 3610},
					{3685, 3610},
					{3685, 3683}
				}
			};

			auto randomPos = select_randomly(std::begin(positions), std::end(positions));
			sServer.LanHouseN.MobId = CreateMob("DarkCaveira", randomPos->X, randomPos->Y, "npc");

			Log(SERVER_SIDE, LOG_INGAME, "O boss da área da LanHouse nasceu em %hux %huy", randomPos->X, randomPos->Y);
			SendNoticeArea("O boss da LanHouse (N) nasceu", 3600, 3600, 3700, 3700);
		}
	}

	if (sServer.LanHouseM.MobId == arg1)
	{
		sServer.LanHouseM.MobId = 0;
		sServer.LanHouseM.Count = 0;
		for (int i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			if (Mob[i].Target.X >= 3732 && Mob[i].Target.X <= 3816 && Mob[i].Target.Y >= 3476 && Mob[i].Target.Y <= 3562)
				SendClientMessage(i, "O boss da LanHouse foi morto por %s", Mob[arg2].Mobs.Player.Name);
		}
	}

	if (Mob[arg2].Target.X >= 3732 && Mob[arg2].Target.X <= 3816 && Mob[arg2].Target.Y >= 3476 && Mob[arg2].Target.Y <= 3562 && Mob[arg2].Mobs.Player.CapeInfo != 4)
	{
		if (++sServer.LanHouseM.Count >= sServer.LanHouseM.TotalToKill && sServer.LanHouseM.MobId == 0)
		{
			sServer.LanHouseM.Count = 0;
			std::array<st_Position, 4> positions =
			{
				{
					{3813, 3557},
					{3813, 3482},
					{3738, 3482},
					{3738, 3557}
				}
			};

			auto randomPos = select_randomly(std::begin(positions), std::end(positions));
			sServer.LanHouseM.MobId = CreateMob("PerGorila", randomPos->X, randomPos->Y, "npc");

			Log(SERVER_SIDE, LOG_INGAME, "O boss da área da LanHouse (M) nasceu em %hux %huy", randomPos->X, randomPos->Y);
			SendNoticeArea("O boss da LanHouse (M) nasceu", 3732, 3476, 3816, 3562);
		}
	}

	// 0045D646
	INT32 LOCAL_198 = Mob[arg1].Mobs.Player.Status.Level;
	static const int FixDropTax[64] = 
	{
		400,  400,  400,  400, 400, 400, 400,  400,  4,  4,  4,  4, 900,  900,  900,  900,  20000, 20000, 20000, 20000, 20000, 20000, 
		35, 35, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 
		1500, 1500, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 1, 35, 500, 2500, 5000, 5000, 10000, 20000
	};

	// 0045D857 testando
	for(LOCAL_132 = 0; LOCAL_132 < 64; LOCAL_132++)
	{
		st_Item *LOCAL_207 = (st_Item*)&Mob[arg1].Mobs.Player.Inventory[LOCAL_132];
		if(!Mob[arg1].Mobs.Player.Inventory[LOCAL_132].Index)		
			continue;
		
		INT32 t = 0;
		for(; t < MAX_BLOCKITEM; t++)
		{
			if(Mob[arg1].Mobs.Player.Inventory[LOCAL_132].Index == g_pBlockedItem[t])
				break;
		}

		if(t != MAX_BLOCKITEM)
			continue;

		// Liberado drop dos itens abaixo somente com o Kefra morto
		if((LOCAL_207->Index >= 674 && LOCAL_207->Index <= 677) || LOCAL_207->Index == 770)
		{
			if(!sServer.KefraDead)
				continue;
		}

		INT32 LOCAL_203 = FixDropTax[LOCAL_132]; // 4C90B8
		INT32 LOCAL_204 = Taxes[LOCAL_132]; //Taxes[LOCAL_132] + mobKiller->Game.Bonus.DropBonus;

		if(LOCAL_204 != 100 && LOCAL_204 != 0)
		{
			LOCAL_204 = 10000 / LOCAL_204;
			LOCAL_203 = LOCAL_203 *		 LOCAL_204 / 100;
		}
		
		int bonusDrop = 0;
		for (int i = 0; i < 3; i++)
		{
			if (LOCAL_207->Effect[i].Index == EF_DROPBONUS)
			{
				bonusDrop = LOCAL_207->Effect[i].Value;
				break;
			}
		}

		if (bonusDrop > 0 && bonusDrop < 100)
			LOCAL_203 -= (LOCAL_203 * bonusDrop / 100);

		INT32 fada = Mob[arg2].Mobs.Player.Equip[13].Index;
		if(fada == 3901 || fada == 3904 || fada == 3905 || fada == 3907 || fada == 3908 || fada == 3914)
			LOCAL_203 -= (LOCAL_203 *  32 / 100);
	
		if (fada == 3915)
			LOCAL_203 -= (LOCAL_203 * 40 / 100);

		if(fada == 3902)
			LOCAL_203 -= (LOCAL_203 *  16 / 100);

		if(Mob[arg2].DropBonus > 0 && Mob[arg2].DropBonus < 100)
			LOCAL_203 -= (LOCAL_203 * Mob[arg2].DropBonus / 100);

		if(LOCAL_203 <= 0)
			LOCAL_203 = 1;

		INT32 LOCAL_205 = LOCAL_132 >> 3;
		if(LOCAL_132 >= 60) 
		{// 0045D929  |. 81BD E8FCFFFF >|CMP [LOCAL.198],0AA
			if(LOCAL_198 < 170)
				LOCAL_203 = LOCAL_203 * 90 / 100;
			else if(LOCAL_198 < 200)	
				LOCAL_203 = LOCAL_203 * 60 / 100;
			else if(LOCAL_198 < 230)
				LOCAL_203 = LOCAL_203 * 50 / 100;
			else if(LOCAL_198 < 255)
				LOCAL_203 = LOCAL_203 * 43 / 100;
			else
				LOCAL_203 = LOCAL_203 * 38 / 100;
		}
		else
		{
			if(LOCAL_205 == 0 || LOCAL_205 == 1 || LOCAL_205 == 2)
			{
				if(LOCAL_198 < 10)
					LOCAL_203 = LOCAL_203 * 3 / 10; 
				else if(LOCAL_198 < 20)
					LOCAL_203 = (LOCAL_203 << 2) / 10;
				else if(LOCAL_198 < 30)
					LOCAL_203 = LOCAL_203 * 5 / 10;
				else if(LOCAL_198 < 40)
					LOCAL_203 = LOCAL_203 * 6 / 10;
				else if(LOCAL_198 < 50)
					LOCAL_203 = LOCAL_203 * 7 / 10;
				else if(LOCAL_198 < 60)
					LOCAL_203 = (LOCAL_203 << 3) / 10;
			}
		}
			
		if(LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
			LOCAL_203 = 4;

		if(LOCAL_203 > 32000)
			LOCAL_203 = 32000;

		int realRate = LOCAL_203;
		if(LOCAL_203 > 0)
			LOCAL_203 = Rand () % LOCAL_203;
		else
			LOCAL_203 = 0;

		if(LOCAL_203 != 0 && LOCAL_132 != 11)
			continue;

		if(LOCAL_207->Index <= 0 || LOCAL_207->Index > 6500)
			continue;

		if(ItemList[LOCAL_207->Index].Level > 140)
			if((LOCAL_203 & 0x80000001) == 1)
				continue;

		int LOCAL_209 = 0;
		if(arg2 > 0 && arg2 < MAX_PLAYER)
			LOCAL_209 = Mob[arg2].DropBonus;

		// Fada dourada / Fada Prateada
		bool isValley = Mob[arg2].IsInsideValley();
		if (fada == 3914 || fada == 3915 || (isValley && fada == 3917))
		{
			constexpr std::array agruppedItems
			{ 
				412, 413, 419, 420,
				2390, 2391, 2392, 2393, 2394, 2395, 2396, 2397, 2398, 2399, 2400, 2401, 2402,
				2403, 2404, 2405, 2406, 2407, 2408, 2409, 2410, 2411, 2412, 2413, 2414, 2415,
				2416, 2417, 2418, 2419, 4016, 4017, 4018, 4019, 4020, 4850
			};

			std::vector<uint16_t> excludedItems
			{
				3140, 
				1774
			};

			if (isValley)
			{
				excludedItems.push_back(540);
				excludedItems.push_back(541);
			}

			auto itemId = LOCAL_207->Index;
			bool agroup = std::find(agruppedItems.begin(), agruppedItems.end(), LOCAL_207->Index) != agruppedItems.end();
			if (agroup)
			{
				bool bag1 = TimeRemaining(Mob[arg2].Mobs.Player.Inventory[60].EFV1, Mob[arg2].Mobs.Player.Inventory[60].EFV2, Mob[arg2].Mobs.Player.Inventory[60].EFV3 + 1900) > 0.0f;
				bool bag2 = TimeRemaining(Mob[arg2].Mobs.Player.Inventory[61].EFV1, Mob[arg2].Mobs.Player.Inventory[61].EFV2, Mob[arg2].Mobs.Player.Inventory[61].EFV3 + 1900) > 0.0f;

				int max = GetMaxAmountItem(LOCAL_207);
				int slotId = -1;
				for (int i = 0; i < 60; i++)
				{
					if (i >= 30 && !bag1 && !bag2)
						break;

					if (i >= 30 && i <= 44 && !bag1)
						continue;

					if (i >= 45 && i <= 59 && !bag2)
						continue;

					if (Mob[arg2].Mobs.Player.Inventory[i].Index == LOCAL_207->Index)
					{
						if (GetItemAmount(&Mob[arg2].Mobs.Player.Inventory[i]) < max)
						{
							slotId = i;

							break;
						}
					}
				}

				if (slotId != -1)
				{
					// fazemos a cópia para 
					st_Item temporaryItem = *LOCAL_207;
					st_Item* dstItem = &Mob[arg2].Mobs.Player.Inventory[slotId];
					if (AgroupItem(arg2, &temporaryItem, dstItem))
					{
						Log(arg2, LOG_INGAME, "Item %s foi agrupado pela %s", ItemList[LOCAL_207->Index].Name, ItemList[fada].Name);

						SendItem(arg2, SlotType::Inv, slotId, dstItem);

						// Parte não feita - 0045DCF2
						if (LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
							LOCAL_132 = 11;
						continue;
					}
				}
			}
			else if (fada == 3915 && (itemId == 4010 || itemId == 4011 || (itemId >= 4026 && itemId <= 4029)))
			{
				// fazemos a cópia para não apagarmos o item do inventário do mob
				st_Item temporaryItem = *LOCAL_207;

				if (GoldBar(arg2, SlotType::Inv, 0, &temporaryItem))
					Log(arg2, LOG_INGAME, "Barra de gold utilizado pela fada dourada");

				// Parte não feita - 0045DCF2
				if (LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
					LOCAL_132 = 11;

				continue;
			}
			else if(std::find(excludedItems.begin(), excludedItems.end(), LOCAL_207->Index) == excludedItems.end())
			{
				// fazemos a cópia para não apagarmos o item do inventário do mob
				st_Item temporaryItem = *LOCAL_207;

				auto sellResult = SellItem(arg2, &temporaryItem);
				if (sellResult == TOD_SellItemResult::Success)
				{
					Log(arg2, LOG_INGAME, "Item %s vendido pela %s - [%d] [%d %d %d %d %d %d] em %dx %dy - %s", ItemList[LOCAL_207->Index].Name, ItemList[fada].Name, LOCAL_207->Index, LOCAL_207->EF1, LOCAL_207->EFV1, LOCAL_207->EF2,
						LOCAL_207->EFV2, LOCAL_207->EF3, LOCAL_207->EFV3, Mob[arg2].Target.X, Mob[arg2].Target.Y, Mob[arg1].Mobs.Player.Name);

					// Parte não feita - 0045DCF2
					if (LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
						LOCAL_132 = 11;
					continue;
				}
				else
					Log(arg2, LOG_INGAME, "Item não vendido pela fada pelo motivo de %d", (int)sellResult);
			}
		}

		SetItemBonus(LOCAL_207, Mob[arg1].Mobs.Player.Status.Level, 0, LOCAL_209);

		INT32 slotId = GetFirstSlot(arg2, 0);
		if(slotId == -1)
		{
			Log(arg2, LOG_INGAME, "Não dropou o item %s por falta de espaço [%d] [%d %d %d %d %d %d] em %dx %dy - %s (1/%d)", ItemList[LOCAL_207->Index].Name, LOCAL_207->Index, LOCAL_207->EF1, LOCAL_207->EFV1, LOCAL_207->EF2,
				LOCAL_207->EFV2, LOCAL_207->EF3, LOCAL_207->EFV3, Mob[arg2].Target.X, Mob[arg2].Target.Y, Mob[arg1].Mobs.Player.Name, realRate);

			LogPlayer(arg2, "Não dropou o item %s no mob %s por falta de espaço no inventário", ItemList[LOCAL_207->Index].Name, Mob[arg1].Mobs.Player.Name);
			SendClientMessage(arg2, "Seu inventário está cheio");
		}
		else
		{
			memcpy(&Mob[arg2].Mobs.Player.Inventory[slotId], LOCAL_207, 8);
			SendItem(arg2, SlotType::Inv, slotId, LOCAL_207);

			Log(arg2, LOG_INGAME, "Dropado item %s [%d] [%d %d %d %d %d %d] em %dx %dy - %s (1/%d)", ItemList[LOCAL_207->Index].Name, LOCAL_207->Index, LOCAL_207->EF1, LOCAL_207->EFV1, LOCAL_207->EF2,
				LOCAL_207->EFV2, LOCAL_207->EF3, LOCAL_207->EFV3, Mob[arg2].Target.X, Mob[arg2].Target.Y, Mob[arg1].Mobs.Player.Name, realRate);

			LogPlayer(arg2, "Dropado o item %s no mob %s", ItemList[LOCAL_207->Index].Name, Mob[arg1].Mobs.Player.Name);

			auto& user = Users[arg2];
			auto found = std::find_if(user.Dropped.Items.begin(), user.Dropped.Items.end(), [](const DroppedItem& dropped) {
				return dropped.Item.Index <= 0 || dropped.Item.Index >= MAX_ITEMLIST;
			});

			if (found == user.Dropped.Items.end())
			{
				if (++user.Dropped.LastIndex >= user.Dropped.Items.size())
					user.Dropped.LastIndex = 0;

				found = user.Dropped.Items.begin() + user.Dropped.LastIndex;
			}

			// Fazemos uma cópia para cá
			found->Item = *LOCAL_207;
			found->SlotId = slotId;
			found->Time = std::chrono::steady_clock::now();
		}

		// Parte não feita - 0045DCF2
		if(LOCAL_132 == 8 || LOCAL_132 == 9 || LOCAL_132 == 10)
			LOCAL_132 = 11;
	}

	// 0045DE03
	if(arg2 < MAX_PLAYER)
	{
		if(Mob[arg1].Mobs.Player.Equip[0].Index == 239 || Mob[arg1].Mobs.Player.Equip[0].Index == 241)
		{
			if(!(Rand() % 20))
			{
				INT32 LOCAL_278 = 1;
				INT32 LOCAL_279 = 1;
				INT32 LOCAL_280 = 2;

				LOCAL_278 = LOCAL_278 << (char)LOCAL_280;
				LOCAL_279 = LOCAL_279 << (char)((LOCAL_280 + 1));

				if((Mob[arg2].Mobs.Player.QuestInfo.Mystical_GetQuest) && !(Mob[arg2].Mobs.Player.QuestInfo.Mystical_GetAmuleto))
				{
					SendClientMessage(arg2, g_pLanguageString[_NN_Watching_Town_Success]);

					Mob[arg2].Mobs.Player.QuestInfo.Mystical_CanGetAmuleto = 1;
				}
			}
		}
	}

	if(Mob[arg2].Mobs.Player.AffectInfo.DrainHP)
	{
		INT32 LOCAL_281 = Mob[arg1].Mobs.Player.Status.maxHP;
		INT32 LOCAL_282 = Mob[arg1].Mobs.Player.Status.Mastery[3];

		LOCAL_281 = LOCAL_281 / 30 + LOCAL_282;

		INT32 LOCAL_283 = Mob[arg2].Mobs.Player.Status.curHP;

		Mob[arg2].Mobs.Player.Status.curHP += LOCAL_283;
		if(Mob[arg2].Mobs.Player.Status.curHP > Mob[arg2].Mobs.Player.Status.maxHP)
			Mob[arg2].Mobs.Player.Status.curHP = Mob[arg2].Mobs.Player.Status.maxHP;

		if(LOCAL_283 != Mob[arg2].Mobs.Player.Status.curHP)
		{
			if(arg2 > 0 && arg2 < MAX_PLAYER)
			{
				SetReqHp(arg2);
				SetReqMp(arg2);
			}

			p18A LOCAL_287{};
			LOCAL_287.Header.PacketId = 0x18A;
			LOCAL_287.Header.Size = sizeof p18A;
			LOCAL_287.Header.ClientId = arg2;

			LOCAL_287.CurHP = Mob[arg2].Mobs.Player.Status.curHP;
			LOCAL_287.Incress = LOCAL_287.CurHP - LOCAL_283;

			INT32 LOCAL_288 = Mob[arg2].Target.X;
			INT32 LOCAL_289 = Mob[arg2].Target.Y;

			GridMulticast_2(LOCAL_288, LOCAL_289, (BYTE*)&LOCAL_287, 0);
		}
	}

	if (sServer.DropArea.Status && Users[arg2].DropEvent.IsValid)
	{
		bool hasPotion = false;
		for (int i = 0; i < 32; i++)
		{
			if (Mob[arg2].Mobs.Affects[i].Index == 59)
			{
				hasPotion = true;

				break;
			}
		}

		bool isPesa = false;
		for (int i = 1; i < 3; i++)
		{
			if (Mob[arg2].Target.X >= g_pPesaArea[i][0] && Mob[arg2].Target.X <= g_pPesaArea[i][2] && Mob[arg2].Target.Y >= g_pPesaArea[i][1] && Mob[arg2].Target.Y <= g_pPesaArea[i][3])
				isPesa = true;
		}

		bool isWater = false;
		for (int i = 0; i < 3; ++i)
		{
			for (int x = 0; x < 9; ++x)
			{
				if (Mob[arg2].Target.X >= waterMaxMin[i][x][0] && Mob[arg2].Target.X <= waterMaxMin[i][x][2] && Mob[arg2].Target.Y >= waterMaxMin[i][x][1] && Mob[arg2].Target.Y <= waterMaxMin[i][x][3])
					isWater = true;
			}

			if (isWater)
				break;
		}

		if (!isPesa && !isWater)
		{
			for (const auto& area : sServer.DropArea.areas)
			{
				if (area.Limit != 0 && area.Dropped >= area.Limit)
					continue;

				if (Mob[arg2].Target.X < area.Min.X || Mob[arg2].Target.X > area.Max.X || Mob[arg2].Target.Y < area.Min.Y || Mob[arg2].Target.Y > area.Max.Y)
					continue;

				int rate = area.Rate;

				int fada = Mob[arg2].Mobs.Player.Equip[13].Index;
				if (fada == 3901 || fada == 3904 || fada == 3905 || fada == 3907 || fada == 3908 || fada == 3914)
					rate -= (rate * 15 / 100);

				if (fada == 3915)
					rate -= (rate * 18 / 100);

				if (fada == 3902)
					rate -= (rate * 10 / 100);

				if (hasPotion)
					rate -= (rate * 30 / 100);

				int rand = Rand() % rate;
				if (rand != 0)
					break;

				const st_Item& item = area.Item;
				int slotId = GetFirstSlot(arg2, 0);
				if (slotId == -1)
				{
					SendClientMessage(arg2, "Falta espaço no inventário");

					Log(arg2, LOG_INGAME, "Não dropou o item do evento %s por falta de espaço no inventário", item.toString().c_str());
					break;
				}

				area.Dropped++;

				Users[arg2].DropEvent.Dropped++;

				if (sServer.DropArea.Message)
				{
					if (area.Limit == 0)
						SendNotice("%s dropou o item %s. Total dropado: %d", Mob[arg2].Mobs.Player.Name, ItemList[item.Index].Name, area.Dropped);
					else
						SendNotice("%s dropou o item %s. Total dropado: %d de %d", Mob[arg2].Mobs.Player.Name, ItemList[item.Index].Name, area.Dropped, area.Limit);
				}

				Log(arg2, LOG_INGAME, "Foi dropado o item do evento %s. Rate: %d. Gerado: %d. Total de dropados: %d", item.toString().c_str(), area.Rate, rand, Users[arg2].DropEvent.Dropped);

				Mob[arg2].Mobs.Player.Inventory[slotId] = item;
				SendItem(arg2, SlotType::Inv, slotId, &Mob[arg2].Mobs.Player.Inventory[slotId]);
				break;
			}
		}
	}

	DeleteMob(arg1, 1);
}

void ClearAreaTeleport(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int DestX, unsigned int DestY, const char* areaName = "")
{	//0x00401596
	int LOCAL_1 = 1;

	for(; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if(Users[LOCAL_1].Status != USER_PLAY)
			continue;

		if(!Mob[LOCAL_1].Mode)
			continue;

		if(Mob[LOCAL_1].Target.X >= x1 && Mob[LOCAL_1].Target.X <= x2 &&
			Mob[LOCAL_1].Target.Y >= y1 && Mob[LOCAL_1].Target.Y <= y2)
		{
			if(Mob[LOCAL_1].Mobs.Player.Status.curHP <= 0)
			{
				Mob[LOCAL_1].Mobs.Player.Status.curHP = 1;
				SendScore(LOCAL_1);
			}
		
			Log(LOCAL_1, LOG_INGAME, "Enviado para a cidade através do ClearAreaTeleport. Posição: %ux %uy. Teleportado para \"%s\"", Mob[LOCAL_1].Target.X, Mob[LOCAL_1].Target.Y, areaName);
			Teleportar(LOCAL_1, DestX, DestY);
		}
	}
}

void ClearArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{	//0x00401596
	int LOCAL_1 = 0;

	for (; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if (Users[LOCAL_1].Status != USER_PLAY)
			continue;

		if (!Mob[LOCAL_1].Mode)
			continue;

		if (Mob[LOCAL_1].Target.X < x1 || Mob[LOCAL_1].Target.X > x2 ||
			Mob[LOCAL_1].Target.Y < y1 || Mob[LOCAL_1].Target.Y > y2)
			continue;

		if (Mob[LOCAL_1].Mobs.Player.Status.curHP <= 0)
		{
			Mob[LOCAL_1].Mobs.Player.Status.curHP = 1;
			SendScore(LOCAL_1);
		}

		Log(LOCAL_1, LOG_INGAME, "Enviado para a cidade através do ClearArea. Posição: %dx %dy", Mob[LOCAL_1].Target.X, Mob[LOCAL_1].Target.Y);
		DoRecall(LOCAL_1);
	}
}

void ClearArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, const char* questName)
{	//0x00401596
	int LOCAL_1 = 0;

	for (; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if (Users[LOCAL_1].Status != USER_PLAY)
			continue;

		if (!Mob[LOCAL_1].Mode)
			continue;

		if (Mob[LOCAL_1].Target.X < x1 || Mob[LOCAL_1].Target.X > x2 ||
			Mob[LOCAL_1].Target.Y < y1 || Mob[LOCAL_1].Target.Y > y2)
			continue;

		Log(LOCAL_1, LOG_INGAME, "Enviado para a cidade através do ClearArea. Posição: %dx %dy - Quest %s", Mob[LOCAL_1].Target.X, Mob[LOCAL_1].Target.Y, questName);
		DoRecall(LOCAL_1);
	}
}

void DoRemoveHide(INT32 clientId)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	for (INT32 i = 0; i < 32; i++)
	{
		if (Mob[clientId].Mobs.Affects[i].Index == 28)
		{
			memset(&Mob[clientId].Mobs.Affects[i], 0, sizeof st_Affect);

			Mob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
		}
	}
}

void DoRemovePossuido(INT32 clientId)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	for (INT32 i = 0; i < 32; i++)
	{
		if (Mob[clientId].Mobs.Affects[i].Index == 24)
		{
			memset(&Mob[clientId].Mobs.Affects[i], 0, sizeof st_Affect);

			Mob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
		}
	}
}

int CreateItem(int posX, int posY, st_Item *item, int unknow, int arg4, int status)
{
	if(item->Index <= 0 || item->Index >= MAX_ITEMLIST)
		return 0;

	GetEmptyItemGrid(&posX, &posY);

	// Checagem de terreno não feita
	if(g_pItemGrid[posY][posX])
		return 0;

	INT32 LOCAL_1 = GetEmptyItem();
	if(LOCAL_1 == 0)
		return 0;

	pInitItem[LOCAL_1].Open = 1;
	pInitItem[LOCAL_1].PosX = posX;
	pInitItem[LOCAL_1].PosY = posY;

	memcpy(&pInitItem[LOCAL_1].Item, item, sizeof st_Item);

	pInitItem[LOCAL_1].Rotation		= unknow;
	pInitItem[LOCAL_1].Status		= status;
	pInitItem[LOCAL_1].IsOpen		= 90;
	pInitItem[LOCAL_1].Unknow_36	= 0;
	pInitItem[LOCAL_1].CanRun		= GetItemAbility(item, EF_GROUND);

	g_pItemGrid[posY][posX]			= LOCAL_1;
	pInitItem[LOCAL_1].HeightGrid	= g_pHeightGrid[posY][posX];
	
	p26E packet;
	GetCreateItem(LOCAL_1, &packet);

	GridMulticast_2(posX, posY, (BYTE*)&packet, 0);
	return LOCAL_1;
}

void SetCurKill(int Index, int cFrag)
{
	if(cFrag < 0)
		cFrag = 0;
	else if(cFrag > 200) 
		cFrag = 200;

	Mob[Index].Mobs.Player.Inventory[63].EFV1 = cFrag;
}

void SetGuilty(int Cid, int arg_2)
{//0x00401488
	if(Cid <= 0 || Cid >= MAX_PLAYER)
	    return;

	if(arg_2 < 0)
		arg_2 = 0;
	else if(arg_2 > 50)
		arg_2 = 50;

	int LOCAL_1 = arg_2;
	Mob[Cid].Mobs.Player.Inventory[63].EF2 = LOCAL_1;
}

void SetTotKill(int Index, int tFrag)
{
	if (tFrag < 0)
		tFrag = 0;

	if (tFrag > 32767)
		tFrag = 32767;

	Mob[Index].Mobs.Player.Inventory[63].EFV2 = tFrag % 256;
	Mob[Index].Mobs.Player.Inventory[63].EFV3 = tFrag >> 8;
}

void SetItemBonus(st_Item* item)
{
	int LOCAL_4 = item->Index;
	int LOCAL_5 = ItemList[item->Index].Unique;
	int LOCAL_7 = ItemList[item->Index].Pos;

	int LOCAL_9 = 0;

	INT32 ability = GetItemAbility(item, EF_MOBTYPE);
	if ((LOCAL_7 & 254) && LOCAL_7 != 128)
	{
		int LOCAL_10 = 59;
		int LOCAL_11 = 0;
		int LOCAL_12 = 0;

		int LOCAL_13 = Rand() % 101;

		int LOCAL_14 = 100;

		LOCAL_14 = LOCAL_13 % 3;

		if (LOCAL_7 == 2)
		{
			LOCAL_10 = EF_ATTSPEED;
			LOCAL_12 = 3;

			if (LOCAL_14 == 1)
			{
				LOCAL_10 = EF_MAGIC;
				LOCAL_12 = 2;
			}
		}
		else if (LOCAL_7 == 4 || LOCAL_7 == 8)
		{
			LOCAL_10 = EF_CRITICAL2;
			LOCAL_12 = 10;
			LOCAL_11 = 1;

			if (LOCAL_14 == 2)
			{
				LOCAL_10 = EF_AC;
				LOCAL_12 = 5;
				LOCAL_11 = 0;
			}
		}
		else if (LOCAL_7 == 16)
		{
			LOCAL_10 = EF_ACADD2;
			LOCAL_12 = 5;
		}
		else if (LOCAL_7 == 32)
		{
			LOCAL_10 = EF_DAMAGE2;
			LOCAL_12 = 6;
			LOCAL_11 = -1;
		}
		else if (LOCAL_7 == 64 || LOCAL_7 == 192)
		{
			if (LOCAL_5 == 44 || LOCAL_5 == 47)
			{
				switch (LOCAL_14)
				{
				case 0:
					LOCAL_10 = EF_MAGIC;
					LOCAL_12 = 4;
					LOCAL_11 = -1;
					break;
				case 1:
					LOCAL_10 = EF_SPECIALALL;
					LOCAL_12 = 3;
					LOCAL_11 = -1;
					break;
				}
			}
			else if (LOCAL_14 == 0)
			{
				LOCAL_10 = EF_ATTSPEED;
				LOCAL_12 = 3;
				LOCAL_11 = 1;
			}
			else if (LOCAL_14 == 1)
			{
				LOCAL_10 = EF_DAMAGE;
				LOCAL_12 = 9;
				LOCAL_11 = -1;
			}
			else if (LOCAL_14 == 2)
			{
				LOCAL_10 = EF_SPECIALALL;
				LOCAL_12 = 3;
			}
		}

		int LOCAL_15 = Rand() % 100;
		int LOCAL_16 = LOCAL_15 % 3;

		int LOCAL_17 = 59;
		int LOCAL_18 = 0;
		int LOCAL_19 = 0;

		switch (LOCAL_7)
		{
		case 2:
			LOCAL_17 = EF_HP;
			LOCAL_18 = 10;

			if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_AC;
				LOCAL_18 = 5;
				LOCAL_19 = -1;
			}
			break;
		case 4:
		case 8:
			if (LOCAL_16 == 2)
			{
				LOCAL_17 = EF_AC;
				LOCAL_18 = 5;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 0)
			{
				LOCAL_17 = EF_MAGIC;
				LOCAL_18 = 2;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_DAMAGE;
				LOCAL_18 = 6;
				LOCAL_19 = -1;
			}
			break;
		case 16:
			if (LOCAL_16 == 0)
			{
				LOCAL_17 = EF_MAGIC;
				LOCAL_18 = 2;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_DAMAGE;
				LOCAL_18 = 6;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 2)
			{
				LOCAL_17 = EF_SPECIALALL;
				LOCAL_18 = 3;
			}
			break;
		case 32:
			if (LOCAL_16 == 0)
			{
				LOCAL_17 = EF_MAGIC;
				LOCAL_18 = 2;
				LOCAL_19 = -1;
			}
			else if (LOCAL_16 == 1)
			{
				LOCAL_17 = EF_SPECIALALL;
				LOCAL_18 = 3;
			}
			break;
		case 64:
		case 128:
		case 192:
			if (LOCAL_5 == 44 || LOCAL_5 == 47)
			{
				if (LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 4;
					LOCAL_19 = -1;
				}
				else if (LOCAL_16 == 1)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
					LOCAL_19 = 1;
				}
			}
			else
			{
				if (LOCAL_16 == 0)
				{
					LOCAL_17 = EF_ATTSPEED;
					LOCAL_18 = 3;
					LOCAL_19 = 1;
				}
				else if (LOCAL_16 == 1)
				{
					LOCAL_17 = EF_DAMAGE;
					LOCAL_18 = 9;
					LOCAL_19 = -1;
				}
				else if (LOCAL_16 == 2)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
					LOCAL_19 = 0x01;
				}
			}
			break;
		}

		int LOCAL_20 = 0;
		LOCAL_13 = Rand() % 100;

		if (LOCAL_13 <= 20)
			LOCAL_20 = 6;
		else if (LOCAL_13 < 60)
			LOCAL_20 = 5;
		else if (LOCAL_13 < 80)
			LOCAL_20 = 4;
		else
			LOCAL_20 = 3;

		if (LOCAL_9 != 0 && LOCAL_20 < 4)
			LOCAL_20 = 4;

		LOCAL_20 += LOCAL_11;

		if (ability == 1 && LOCAL_20 >= 3)
			LOCAL_20 = 2;

		if (!item->Effect[1].Index && LOCAL_20 > 0)
		{
			// EBP - 28 = LOCAL_10
			item->Effect[1].Index = LOCAL_10;
			item->Effect[1].Value = LOCAL_20 * LOCAL_12;
		}
		else if (!item->Effect[1].Index && LOCAL_20 <= 0 && LOCAL_7 == 32)
		{
			item->Effect[1].Index = LOCAL_10;
			item->Effect[1].Value = 0;
		}
		else if (!item->Effect[1].Index)
		{
			item->Effect[1].Index = EF_UNIQUE;
			item->Effect[1].Value = Rand() % 127;
		}

		int LOCAL_21 = 0;
		LOCAL_15 = Rand() % 0x64;

		if (LOCAL_15 <= 20) // 2
			LOCAL_21 = 6;
		else if (LOCAL_15 < 60) // 9
			LOCAL_21 = 5;
		else if (LOCAL_13 < 80)
			LOCAL_21 = 4;
		else
			LOCAL_21 = 3;

		if (LOCAL_9 != 0 && LOCAL_21 < 3)
			LOCAL_21 = 0x03;

		LOCAL_21 += LOCAL_19;

		if (ability == 1 && LOCAL_21 >= 3)
			LOCAL_21 = 2;

		// Limita o adicional dano para 45 + 27
		if (LOCAL_7 >= 64 && LOCAL_17 == EF_DAMAGE && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		// Limita o adicional dano para 45 + 27
		if (LOCAL_7 >= 64 && LOCAL_17 == EF_MAGIC && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		if (LOCAL_21 > 0x00 && !item->Effect[2].Index)
		{
			item->Effect[2].Index = LOCAL_17;
			item->Effect[2].Value = LOCAL_21 * LOCAL_18;
		}

		if (!(Rand() % 80))
		{
			int LOCAL_22 = Rand() % 0x064;

			int LOCAL_23 = 6;
			int LOCAL_24 = 35;
			int LOCAL_25 = 85;
			int LOCAL_26 = 100;

			LOCAL_23 = 6;
			LOCAL_24 = 35;
			LOCAL_25 = 70;
			LOCAL_25 = 85;
			LOCAL_26 = 100;

			if (LOCAL_22 < LOCAL_26)
			{ // Aqui ele gera adicionais tipo: Inteligência, força e tal ^^ 
				for (int i = 0; i < 3; i++)
				{
					int LOCAL_28 = Rand() % 0x0A;
					int LOCAL_29 = dwValue[LOCAL_28];
					int LOCAL_30 = 2;
					int LOCAL_31 = dwValue_02[LOCAL_28][LOCAL_30][0];
					int LOCAL_32 = dwValue_02[LOCAL_28][LOCAL_30][1];

					int LOCAL_33 = LOCAL_32 + 1 - LOCAL_31;

					if (LOCAL_33 == 0)
						LOCAL_33 = 1;

					int LOCAL_34 = Rand() % LOCAL_33 + LOCAL_31;

					item->Effect[i].Index = LOCAL_29;
					item->Effect[i].Value = LOCAL_34;
				}
			}
		}
	}
}

void SetItemBonus(st_Item *item, int level, int sanc, int bonus)
{
	INT32 LOCAL_1 = bonus >> 3;
	
	if(LOCAL_1 < 0)
		LOCAL_1 = 0;

	if(LOCAL_1 > 2)
		LOCAL_1 = 2;

	INT32 LOCAL_2 = -1;
	INT32 LOCAL_3 = -1;

	if(item->Effect[0].Index >= 100 && item->Effect[0].Index <= 105)
	{
		LOCAL_2 = item->Effect[0].Index - 100;
		LOCAL_3 = item->Effect[0].Value;

		item->Effect[0].Index = 0;
		item->Effect[0].Value = 0;
	}
	
	if(sanc == 0 && level >= 210)
		level -= 47;

	int LOCAL_4 = item->Index;
	int LOCAL_5 = ItemList[item->Index].Unique;
	int LOCAL_6 = ItemList[item->Index].Level; // LEVEL
	int LOCAL_7 = ItemList[item->Index].Pos;

	int LOCAL_8 = (level - LOCAL_6) / 25;
	if(LOCAL_2 != -1)
		LOCAL_8 = LOCAL_2;

	//0044443D  |. A3 B4695C01    MOV DWORD PTR DS:[15C69B4],EAX -> Reparei que não é utilizado ^^ 

	int LOCAL_9 = 0;

	if(LOCAL_8 >= 4)
		LOCAL_9 = 1;

	if(LOCAL_8 < 0)
		LOCAL_8 = 0;

	if(LOCAL_8 > 3)
		LOCAL_8 = 3;

	if(sanc != 0)
		if(LOCAL_8 >= 3)
			LOCAL_8 = 3;
	
	INT32 ability = GetItemAbility(item, EF_MOBTYPE);
	if((LOCAL_7 & 254) && !item->Effect[0].Index && LOCAL_7 != 128)
	{
		int LOCAL_10 = 59;
		int LOCAL_11 = 0;
		int LOCAL_12 = 0;
			
		int LOCAL_13 = Rand() % 101;

		int LOCAL_14 = 100;

		// Não original, não sei o que fazem aqui ^^ 
		if(LOCAL_1 == 0)
			LOCAL_1 = 1;

		if(LOCAL_13 == 0)
			LOCAL_13 = 1;

		if(sanc != 0)
			LOCAL_14 = LOCAL_13 % 3;
		else if(LOCAL_8 == 0)
			LOCAL_14 = LOCAL_13 % (7 - LOCAL_1);
		else if(LOCAL_8 == 1)
			LOCAL_14 = LOCAL_13 % (5 - LOCAL_1);
		else if(LOCAL_8 == 2)
			LOCAL_14 = LOCAL_13 % (5 - LOCAL_1);
		else if(LOCAL_8 >= 3)
			LOCAL_14 = LOCAL_13 & 0x80000003;

		if(LOCAL_7 == 2)
		{
			if(LOCAL_14 == 0)
			{
				LOCAL_10 = EF_ATTSPEED; 
				LOCAL_12 = 3;
			}
			else if(LOCAL_14 == 1)
			{
				LOCAL_10 = EF_MAGIC;
				LOCAL_12 = 2;
			}
		}
		else if(LOCAL_7 == 4 || LOCAL_7 == 8)
		{
			LOCAL_10 = EF_CRITICAL2;
			LOCAL_12 = 10;
			LOCAL_11 = 1;

			if(LOCAL_14 == 2)
			{
				LOCAL_10 = EF_AC;
				LOCAL_12 = 5;
				LOCAL_11 = 0;
			}
		}
		else if(LOCAL_7 == 16)
		{
			LOCAL_10 = EF_ACADD2;
			LOCAL_12 = 5;
		}
		else if(LOCAL_7 == 32)
		{
			LOCAL_10 = EF_DAMAGE2;
			LOCAL_12 = 6;
			LOCAL_11 = -1;
		}
		else if(LOCAL_7 == 64 || LOCAL_7 == 192)
		{
			if(LOCAL_5 == 44 || LOCAL_5 == 47)
			{
				switch(LOCAL_14)
				{
					case 0:
						LOCAL_10 = EF_MAGIC;
						LOCAL_12 = 4;
						LOCAL_11 = -1;
						break;
					case 1:
						LOCAL_10 = EF_SPECIALALL;
						LOCAL_12 = 3;
						LOCAL_11 = -1;
						break;
				}
			}
			else if(LOCAL_14 == 0)
			{
				LOCAL_10 = EF_ATTSPEED;
				LOCAL_12 = 3;
				LOCAL_11 = 1;
			}
			else if(LOCAL_14 == 1) 
			{
				LOCAL_10 = EF_DAMAGE;
				LOCAL_12 = 9;
				LOCAL_11 = -1;
			}
			else if(LOCAL_14 == 2)
			{
				LOCAL_10 = EF_SPECIALALL;
				LOCAL_12 = 3;
			}
		}

		int LOCAL_15 = Rand() % 100;
		if(sanc != 0)
			LOCAL_15 = (LOCAL_15 << 1) / 3;
	
		int LOCAL_16 = 100;
		if(sanc != 0)
			LOCAL_16 = LOCAL_15 & 0x80000003;
		else if(LOCAL_8 == 0)
			LOCAL_16 = LOCAL_15 & 0x80000007;
		else if(LOCAL_8 == 1 || LOCAL_8 == 2)
			LOCAL_16 = LOCAL_15 % 6;
		else if(LOCAL_8 >= 3)
			LOCAL_16 = LOCAL_15 & 0x80000003;

		int LOCAL_17 = 59;
		int LOCAL_18 = 0;
		int LOCAL_19 = 0;

		switch(LOCAL_7)
		{
			case 2:
				if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_HP;
					LOCAL_18 = 10;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_AC;
					LOCAL_18 = 5;
					LOCAL_19 = -1;
				}
				break;
			case 4:
			case 8:
				if(LOCAL_16 == 2)
				{
					LOCAL_17 = EF_AC;
					LOCAL_18 = 5;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 2;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_DAMAGE;
					LOCAL_18 = 6;
					LOCAL_19 = -1;
				}
				break;
			case 16:
				if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 2;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_DAMAGE;
					LOCAL_18 = 6;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 2)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
				}
				else if(LOCAL_16 == 3)
				{
					LOCAL_17 = EF_RESISTALL;
					LOCAL_18 = 3;
				}
				break;
			case 32:
				if(LOCAL_16 == 0)
				{
					LOCAL_17 = EF_MAGIC;
					LOCAL_18 = 2;
					LOCAL_19 = -1;
				}
				else if(LOCAL_16 == 1)
				{
					LOCAL_17 = EF_SPECIALALL;
					LOCAL_18 = 3;
				}
				break;
			case 64:
			case 128:
			case 192:
				if(LOCAL_5 == 44 || LOCAL_5 == 47)
				{
					if(LOCAL_16 == 0)
					{
						LOCAL_17 = EF_MAGIC;
						LOCAL_18 = 4;
						LOCAL_19 = -1;
					}
					else if(LOCAL_16 == 1)
					{
						LOCAL_17 = EF_SPECIALALL;
						LOCAL_18 = 3;
						LOCAL_19 = 1;
					}
				}
				else
				{
					if(LOCAL_16 == 0)
					{
						LOCAL_17 = EF_ATTSPEED;
						LOCAL_18 = 3;
						LOCAL_19 = 1;
					}
					else if(LOCAL_16 == 1)
					{
						LOCAL_17 = EF_DAMAGE;
						LOCAL_18 = 9;
						LOCAL_19 = -1;
					}
					else if(LOCAL_16 == 2)
					{
						LOCAL_17 = EF_SPECIALALL;
						LOCAL_18 = 3;
						LOCAL_19 = 0x01;
					}
				}
				break;
		}

		int LOCAL_20 = 0;
		LOCAL_13 = Rand () % 100;

		if(LOCAL_8 == 0)
		{
			if(LOCAL_13 < 2)
				LOCAL_20 = 4;
			else if(LOCAL_13 < 6)
				LOCAL_20 = 3;
			else if(LOCAL_13 < 24)
				LOCAL_20 = 2;
			else if(LOCAL_13 < 55)
				LOCAL_20 = 1;
			else 
				LOCAL_20 = 0;
		}
		else if(LOCAL_8 == 1)
		{
			if(LOCAL_13 < 3)
				LOCAL_20 = 5;
			else if(LOCAL_13 < 12)
				LOCAL_20 = 4;
			else if(LOCAL_13 < 24)
				LOCAL_20 = 3;
			else if(LOCAL_13 < 65)
				LOCAL_20 = 2;
			else 
				LOCAL_20 = 1;
		}
		else if(LOCAL_8 == 2)
		{
			if(LOCAL_13 < 5)
				LOCAL_20 = 5;
			else if(LOCAL_13 < 20)
				LOCAL_20 = 4;
			else if(LOCAL_13 < 60)
				LOCAL_20 = 3;
			else 
				LOCAL_20 = 2;
		}
		else if(LOCAL_8 == 3)
		{
			if(LOCAL_13 <= 15)
				LOCAL_20 = 6;
			else if(LOCAL_13 < 40)
				LOCAL_20 = 5;
			else if(LOCAL_13 < 80)
				LOCAL_20 = 4;
			else
				LOCAL_20 = 3;
		}
		else if(LOCAL_8 >= 4)
		{
			if(LOCAL_13 < 2)
				LOCAL_20 = 7;
			else if(LOCAL_13 < 6)
				LOCAL_20 = 6;
			else if(LOCAL_13 < 17)
				LOCAL_20 = 5;
			else
				LOCAL_20 = 4;
		}

		if(LOCAL_9 != 0 && LOCAL_20 < 4)
			LOCAL_20 = 4;

		LOCAL_20 += LOCAL_11;

		if(sanc != 0 && LOCAL_20 == 0)
			LOCAL_20 = 1;

		if(ability == 1 && LOCAL_20 >= 3)
			LOCAL_20 = 2;

		if(!item->Effect[1].Index && LOCAL_20 > 0)
		{
			// EBP - 28 = LOCAL_10
			item->Effect[1].Index = LOCAL_10;
			item->Effect[1].Value = LOCAL_20 * LOCAL_12;
		}
		else if(!item->Effect[1].Index && LOCAL_20 <= 0 && LOCAL_7 == 32)
		{
			item->Effect[1].Index = LOCAL_10;
			item->Effect[1].Value = 0;
		}
		else if(!item->Effect[1].Index)
		{
			item->Effect[1].Index = EF_UNIQUE;
			item->Effect[1].Value = Rand() % 127;
		}

		int LOCAL_21 = 0;

		LOCAL_15 = Rand() % 0x64;

		if(LOCAL_8 == 0)
		{
			if(LOCAL_15 < 0x02)
				LOCAL_21 = 4;
			else if(LOCAL_15 < 0x06)
				LOCAL_21 = 3;
			else if(LOCAL_15 < 0x18)
				LOCAL_21 = 2;
			else if(LOCAL_15 < 0x037)
				LOCAL_21 = 1;
			else 
				LOCAL_21 = 0;
		}
		else if(LOCAL_8 == 1)
		{
			if(LOCAL_15 < 3)
				LOCAL_21 = 5;
			else if(LOCAL_15 < 12)
				LOCAL_21 = 4;
			else if(LOCAL_15 < 24)
				LOCAL_21 = 3;
			else if(LOCAL_15 < 65)
				LOCAL_21 = 2;
			else 
				LOCAL_21 = 1;
		}
		else if(LOCAL_8 == 2)
		{
			if(LOCAL_15 < 7)
				LOCAL_21 = 5;
			else if(LOCAL_15 < 20) // 16
				LOCAL_21 = 4;
			else if(LOCAL_15 < 60)
				LOCAL_21 = 3;
			else 
				LOCAL_21 = 2;
		}
		else if(LOCAL_8 == 3)
		{
			if(LOCAL_15 <= 15) // 2
				LOCAL_21 = 6;
			else if(LOCAL_15 < 40) // 9
				LOCAL_21 = 5;
			else if(LOCAL_13 < 80)
				LOCAL_21 = 4;
			else
				LOCAL_21 = 3;
		}
		else if(LOCAL_8 >= 4)
		{
			if(LOCAL_15 < 2)
				LOCAL_21 = 7;
			else if(LOCAL_15 < 7)
				LOCAL_21 = 6;
			else if(LOCAL_15 < 17)
				LOCAL_21 = 5;
			else if(LOCAL_15 < 75)
				LOCAL_21 = 4;
			else
				LOCAL_21 = 3;
		}

		if(LOCAL_9 != 0 && LOCAL_21 < 3)
			LOCAL_21 = 0x03;

		if(sanc != 0 && LOCAL_21 >= 0x05)
			LOCAL_21 = 0x04;

		LOCAL_21 += LOCAL_19;

		if(LOCAL_1 != 0 && LOCAL_21 == 0)
			LOCAL_21 = LOCAL_1;

		if(sanc != 0 && LOCAL_21 == 0)
			LOCAL_21 = 0x01;
		
		if(ability == 1 && LOCAL_21 >= 3)
			LOCAL_21 = 2;

		// Limita o adicional dano para 45 + 27
		if(LOCAL_7 >= 64 && LOCAL_17 == EF_DAMAGE && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		// Limita o adicional dano para 45 + 27
		if(LOCAL_7 >= 64 && LOCAL_17 == EF_MAGIC && ((LOCAL_21 >= 4 && LOCAL_20 >= 5) || (LOCAL_21 >= 5 && LOCAL_20 >= 4)))
			LOCAL_21 = 3;

		if(LOCAL_21 > 0x00 && !item->Effect[2].Index)
		{
			item->Effect[2].Index = LOCAL_17;
			item->Effect[2].Value = LOCAL_21 * LOCAL_18;
		}
		else if(!item->Effect[2].Index)
		{
			item->Effect[2].Index = EF_UNIQUE;
			item->Effect[2].Value = Rand() % 0x7F;
		}
			
		if(!item->Effect[0].Index)
		{
			int LOCAL_22 = Rand() % 0x064;

			if(sanc != 0)
				LOCAL_22 >>= 1;

			int LOCAL_23 = 1;
			int LOCAL_24 = 12;
			int LOCAL_25 = 45;
			int LOCAL_26 = 70;

			if(LOCAL_8 >= 0x03 || LOCAL_8 == 0x02)
			{
				LOCAL_23 = 6;
				LOCAL_24 = 35;
				LOCAL_25 = 70;
				LOCAL_25 = 85;
				LOCAL_26 = 100;
			}

			if(LOCAL_8 == 0x01 || LOCAL_8 == 0x0)
			{
				LOCAL_23 = 6;
				LOCAL_24 = 2;
				LOCAL_25 = 55;
				LOCAL_25 = 75;
				LOCAL_26 = 90;
			}

			if(LOCAL_22 < LOCAL_23)
			{
				item->Effect[0].Index = 43;
				item->Effect[0].Value = 2;

				if(LOCAL_3 > 0x02)
				{
					int LOCAL_27 = Rand() % 100;

					if(LOCAL_3 == 3)
					{
						if(LOCAL_27 < 30)
							item->Effect[0].Value = 3;
					}
					else if(LOCAL_3 == 4)
					{
						if(LOCAL_27 < 10)
							item->Effect[0].Value = 4;
						else if(LOCAL_27 < 40)
							item->Effect[0].Value = 3;
					}
					else if(LOCAL_3 == 5)
					{
						if(LOCAL_27 < 10)
							item->Effect[0].Value = 5;
						else if(LOCAL_27 < 30)
							item->Effect[0].Value = 4;
						else if(LOCAL_27 < 60)
							item->Effect[0].Value = 3;
					}
					else if(LOCAL_3 == 6)
					{
						if(LOCAL_27 < 10)
							item->Effect[0].Value  = 6;
						else if(LOCAL_27 < 20)
							item->Effect[0].Value = 5;
						else if(LOCAL_27 < 30)
							item->Effect[0].Value = 4;
						else if(LOCAL_27 < 60)
							item->Effect[0].Value = 3;
					}
					else if(LOCAL_3 == 7)
					{
						if(LOCAL_27 < 4)
							item->Effect[0].Value = 7;
						else if(LOCAL_27 < 10)
							item->Effect[0].Value = 6;
						else if(LOCAL_27 < 20)
							item->Effect[0].Value = 5;
						else if(LOCAL_27 < 35)
							item->Effect[0].Value = 4;
						else if(LOCAL_27 < 60)
							item->Effect[0].Value = 3;
					}
				}
			}
			else if(LOCAL_22 < LOCAL_24)
			{
				item->Effect[0].Index = EF_SANC;
				item->Effect[0].Value = 1;
			}
			else if(LOCAL_22 < LOCAL_25)
			{
				item->Effect[0].Index = EF_SANC;
				item->Effect[0].Value = 0;
			}
			else if(LOCAL_22 < LOCAL_26)
			{ // Aqui ele gera adicionais tipo: Inteligência, força e tal ^^ 
				int LOCAL_28 = Rand() % 0x0A;
				int LOCAL_29 = dwValue[LOCAL_28];
				int LOCAL_30 = LOCAL_8;
				if(LOCAL_30 >= 2)
					LOCAL_30 = 2;
				int LOCAL_31 = dwValue_02[LOCAL_28][LOCAL_30][0];
				int LOCAL_32 = dwValue_02[LOCAL_28][LOCAL_30][1];

				int LOCAL_33 = LOCAL_32 + 1 - LOCAL_31;

				if(LOCAL_33 == 0)
					LOCAL_33 = 1;

				int LOCAL_34 = Rand() % LOCAL_33 + LOCAL_31;

				item->Effect[0].Index = LOCAL_29;
				item->Effect[0].Value = LOCAL_34;
			}
			else
			{
				item->Effect[0].Index = 0x3B;
				item->Effect[0].Value = Rand() % 0x7F;
			}
		}
	}

	int LOCAL_35 = 0;
	for(LOCAL_35 = 0; LOCAL_35 < 12; LOCAL_35++)
	{
		int LOCAL_36 = ItemList[LOCAL_4].Level;

		switch(ItemList[LOCAL_4].Effect[LOCAL_35].Index)
		{
			case EF_SANC:
				item->Effect[0].Index = EF_SANC;
				item->Effect[0].Value = static_cast<BYTE>(ItemList[LOCAL_4].Effect[LOCAL_35].Value);
				break;
			case EF_AMOUNT:
				item->Effect[0].Index = EF_AMOUNT;
				item->Effect[0].Value = static_cast<BYTE>(ItemList[LOCAL_4].Effect[LOCAL_35].Value);
				break;
			case EF_INCUBATE:
				item->Effect[0].Index = EF_INCUBATE;

				BYTE LOCAL_37 = Rand() % 4 + ItemList[LOCAL_4].Effect[LOCAL_35].Value;
				if(LOCAL_37 > 9)
					LOCAL_37 = 9;

				item->Effect[0].Value = static_cast<BYTE>(LOCAL_37);
				break;
		}
	}

	// item->Index == 412 || item->Index == 413 || 
	if(item->Index == 419 || item->Index == 420 || item->Index == 753)
	{
		if(!item->Effect[0].Index)
		{
			item->Effect[0].Index = EF_UNIQUE;
			item->Effect[0].Value = Rand() % 0x7F;
		}
		if(!item->Effect[1].Index)
		{
			item->Effect[1].Index = EF_UNIQUE;
			item->Effect[1].Value = Rand() % 0x7F;
		}
		if(!item->Effect[2].Index)
		{
			item->Effect[2].Index = EF_UNIQUE;
			item->Effect[2].Value = Rand() % 0x7F;
		}
	}

	if((item->Index >= 447 && item->Index <= 450) || (item->Index >= 692 && item->Index <= 695))
	{
		if(!item->Effect[0].Index)
		{
			item->Effect[0].Index = EF_UNIQUE;
			item->Effect[0].Value = Rand() % 256;
		}
		if(!item->Effect[1].Index)
		{
			item->Effect[1].Index = EF_UNIQUE;
			item->Effect[1].Value = Rand() % 256;
		}
		if(!item->Effect[2].Index)
		{
			item->Effect[2].Index = EF_UNIQUE;
			item->Effect[2].Value = Rand() % 256;
		}
	}
}

bool SetItemSanc(st_Item *item, int sanc, int arg3)
{
	if(sanc > 15 || sanc < 0)
		return false;

	if(arg3 > 20)
		arg3 = 20;
	if(arg3 < 0)
		arg3 = 0;

	int i = 0;
	int actualSanc = 0;
	for (; i < 3; i++)
	{
		if (item->Effect[i].Index == 43 || (item->Effect[i].Index >= 116 && item->Effect[i].Index <= 125))
		{
			actualSanc = item->Effect[i].Value;
			break;
		}
	}

	if(sanc > 9)
	{
		// se o item já possuir uam refinação maior que +11, respeitamos o adicional  de gema que
		// ele já possui
		int value = 0;
		if (actualSanc > 230)
			value = (actualSanc - 230) % 4;

		sanc = 230 + ((sanc - 10) * 4) + value;
		if(sanc > 250)
			sanc = 250;
	}
    
	int	sc = sanc + (10 * arg3);

	if(i == 3)
	{
		for(i = 0;i<3;i++)
		{
			if(item->Effect[i].Index == 0)
			{
				item->Effect[i].Index = EF_SANC;
				item->Effect[i].Value = sc;
				return true;
			}
		}
		// Impossível de refinar
		if( i == 3 )
			return false;
	}
	else
	{
		item->Effect[i].Value = sc;
		return true;
	}

	return false;
}

void SetPKPoint(int clientId,int points)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER) 
		return;

	if(points < 1) 
		points = 1;

	if(points > 150) 
		points = 150;

	unsigned char cv = points;
	Mob[clientId].Mobs.Player.Inventory[63].EF1 = cv;
}

void SetReqHp(int clientId)
{//0x00401492
    if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;
 
    if(Mob[clientId].Mobs.Player.Status.curHP > Mob[clientId].Mobs.Player.Status.maxHP)
		Mob[clientId].Mobs.Player.Status.curHP = Mob[clientId].Mobs.Player.Status.maxHP;
 
    if(Users[clientId].Potion.CountHp < Mob[clientId].Mobs.Player.Status.curHP)
		Users[clientId].Potion.CountHp = Mob[clientId].Mobs.Player.Status.curHP;
}

void SetReqMp(int clientId)
{
    if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;
	 
    if(Mob[clientId].Mobs.Player.Status.curMP > Mob[clientId].Mobs.Player.Status.maxMP)
		Mob[clientId].Mobs.Player.Status.curMP = Mob[clientId].Mobs.Player.Status.maxMP;
 
    if(Users[clientId].Potion.CountMp < Mob[clientId].Mobs.Player.Status.curMP)
		Users[clientId].Potion.CountMp = Mob[clientId].Mobs.Player.Status.curMP;
}

 int MountCon[30] = 
 { 
	 0,	 800,	 900,	 -800,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 600,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0
 };


int GenerateSummon(int User,int SummonId, st_Item *Item)
{
	int Leader = Mob[User].Leader;
	if(Leader <= 0)
		Leader = User;

	int mobId = mGener.GetEmptyNPCMob();
	if(mobId == 0)
	{
		SendClientMessage(User,"No EmpytMobNPC");
		return 0;
	}

	if(SummonId < 0 || SummonId >= 50)
		return 0;
	
	int summonFaceId = NPCBase[SummonId].Equip[0].Index;
	int LOCAL4 = 0;
	int ptCont = 0;

	int idxPt;
	for(idxPt = 0;idxPt < 12; idxPt++)
	{
		int idx = Mob[Leader].PartyList[idxPt];
		if(idx == 0)
			break;
	}

	if(idxPt >= 12)
	{
		SendClientMessage(User,"Não é possivel envocar com grupo cheio");
		return 0;
	}
	
	memset(&Mob[mobId].PartyList, 0, 24);
	memcpy(&Mob[mobId].Mobs.Player, &NPCBase[SummonId], sizeof st_Mob);
	Mob[mobId].Mobs.Player.bStatus.Level  = Mob[User].Mobs.Player.bStatus.Level;
	Mob[mobId].Mobs.Player.Status.Level  = Mob[User].Mobs.Player.bStatus.Level;
	strcat_s(Mob[mobId].Mobs.Player.Name,"^");

	for(int i=0; i<16; i++)
		if(Mob[mobId].Mobs.Player.Name[i] == 95)
			Mob[mobId].Mobs.Player.Name[i] = 32;

	memset(&Mob[mobId].Mobs.Affects, 0, sizeof(st_Affect) * 32);
	
	int sINT = Mob[User].Mobs.Player.Status.INT + (Mob[User].Mobs.Player.Status.INT * 15 / 100);
	int sMast = Mob[User].Mobs.Player.Status.Mastery[2] + 50;

	if (Item == nullptr && Mob[User].Mobs.Player.ClassInfo == 2)
	{
		if (Mob[User].Mobs.Player.Learn[0] & (1 << 15)) // se tiver oitava skill succubus
		{
			sINT += ceil((static_cast<double>(sINT) * 0.105));
			sMast += 50;
		}
	}

	int calcInt = pSummonBonus[SummonId].MinAttack * sINT / 100;
	int calcMast = pSummonBonus[SummonId].MaxAttack * sMast / 100;

	Mob[mobId].Mobs.Player.bStatus.Attack = Mob[mobId].Mobs.Player.bStatus.Attack + calcInt + calcMast;

	calcInt = pSummonBonus[SummonId].MinDefense * sINT / 100;
	calcMast = pSummonBonus[SummonId].MaxDefense * sMast / 100;

	Mob[mobId].Mobs.Player.bStatus.Defense = Mob[mobId].Mobs.Player.bStatus.Defense + calcInt + calcMast;

	calcInt = pSummonBonus[SummonId].MinHp * sINT / 100;
	calcMast = pSummonBonus[SummonId].MaxHp * sMast / 100;

	Mob[mobId].Mobs.Player.Status.maxHP = Mob[mobId].Mobs.Player.bStatus.maxHP + calcInt + calcMast;
	
	Mob[mobId].Mobs.Player.bStatus.maxHP = Mob[mobId].Mobs.Player.Status.maxHP;
	Mob[mobId].GenerateID = -1;
	Mob[mobId].Formation = 5;
	Mob[mobId].RouteType = 5;
	Mob[mobId].Mode = 4;
	Mob[mobId].Segment.Progress = 0;
	Mob[mobId].Segment.Direction = 0;
	Mob[mobId].Summoner = User;
	Mob[mobId].clientId = mobId;

	memset(Mob[mobId].Segment.ListX, 0 , 5 * sizeof(int));
	memset(Mob[mobId].Segment.ListY, 0 , 5 * sizeof(int));

	Mob[mobId].Leader = Leader;
	Mob[mobId].Last.Time = CurrentTime;

	if(Item != 0)
	{
		int ret = GetItemAbility(Item, EF_MOUNTSANC); //LOCAL_30

		if(ret >= 100)
			ret = 100; 

		int faceId = Mob[mobId].Mobs.Player.Equip[0].Index;		// LOCAL_31
		INT16 Con = 0; // LOCAL_32

		if(faceId >= 315 && faceId < 345)
		{
			int result = faceId - 315;
			int valCon = MountCon[result]; // LOCAL_33
			int calc1 = (valCon / 2) - 1000; // LOCAL_34
			int calc2 = valCon - calc1; // LOCAL_35
	
			Con = (calc1 + ((calc2 * ret) / 100)); // LOCAL_32

			Mob[mobId].Mobs.Player.bStatus.CON = Con;
			Mob[mobId].Mobs.Player.Status.CON = Con;

			int retn = GetItemAbility(Item,EF_MOUNTSANC);

			Mob[mobId].Mobs.Player.bStatus.Attack = Mob[mobId].Mobs.Player.bStatus.Attack + ((retn & 255) * 6);
			Mob[mobId].Mobs.Player.Status.Attack = Mob[mobId].Mobs.Player.bStatus.Attack;
		}
	}

	if(Mob[User].GuildDisable == 0)
	{
		Mob[mobId].Mobs.Player.GuildIndex		= Mob[User].Mobs.Player.GuildIndex;
		Mob[mobId].Mobs.Player.GuildMemberType  = 0;
	}

	Mob[mobId].Mobs.Player.Status.curHP = Mob[mobId].Mobs.Player.Status.maxHP;

	if(true) //?
	{
		if(Mob[User].Mobs.Player.Equip[15].Index == 543 || Mob[User].Mobs.Player.Equip[15].Index == 545)
		{
			memset(&Mob[mobId].Mobs.Player.Equip[15], 0, sizeof(st_Item));
			Mob[mobId].Mobs.Player.Equip[15].Index = 734;
		}

		if(Mob[User].Mobs.Player.Equip[15].Index == 544 || Mob[User].Mobs.Player.Equip[15].Index == 546)
		{
			memset(&Mob[mobId].Mobs.Player.Equip[15], 0, sizeof(st_Item));
			Mob[mobId].Mobs.Player.Equip[15].Index = 735;
		}

		if(Mob[User].Mobs.Player.Equip[15].Index == 548 || Mob[User].Mobs.Player.Equip[15].Index == 549)
		{
			memset(&Mob[mobId].Mobs.Player.Equip[15], 0, sizeof(st_Item));
			Mob[mobId].Mobs.Player.Equip[15].Index = 550;
		}
	}
	
	Mob[mobId].Mobs.Player.CapeInfo = 4;
	
	Mob[mobId].GetCurrentScore(MAX_PLAYER);
	
	memset(&Mob[mobId].EnemyList, 0, sizeof(short) * 4);

	unsigned int X = Mob[User].Target.X;
	unsigned int Y = Mob[User].Target.Y;

	int retMobGrid = GetEmptyMobGrid(mobId,&X,&Y);
	if(retMobGrid == 0)
	{
		Mob[mobId].Mode = 0;
		return 0;
	}

	Mob[mobId].Target.X = X;
	Mob[mobId].Last.X = X;

	Mob[mobId].Target.Y = Y;
	Mob[mobId].Last.Y = Y;

	if(Item == NULL)
	{
		Mob[mobId].Mobs.Affects[0].Index = 24;
		Mob[mobId].Mobs.Affects[0].Master = 0;
		Mob[mobId].Mobs.Affects[0].Value = 0;
		Mob[mobId].Mobs.Affects[0].Time = 30;

		if(SummonId >= 28 && SummonId <= 37)
			Mob[mobId].Mobs.Affects[0].Time = 128;
	}
	
	if(Item != NULL)
	{
		if(*(short*)&Item->EF1 > Mob[mobId].Mobs.Player.Status.maxHP)
			*(short*)&Item->EF1 = Mob[mobId].Mobs.Player.Status.maxHP;
	
		Mob[mobId].Mobs.Player.bStatus.curHP = *(short*)&Item->EF1;
		Mob[mobId].Mobs.Player.Status.curHP = Mob[mobId].Mobs.Player.bStatus.curHP;
	}

	if (Mob[idxPt].Mobs.Player.Equip[0].Index != summonFaceId || SummonId >= 50)
	{
		RemoveSummonerParty(User);
	}

	g_pMobGrid[Y][X] = mobId;

	Mob[mobId].SpawnType = 2;
	SendGridMob(mobId);
	Mob[mobId].SpawnType = 1;

	int MobId = mobId;

	Mob[Leader].PartyList[idxPt] = MobId;
	Mob[MobId].Leader = Leader;

	//if(ptCont == 0)
	//	SendAddParty(Leader,Leader, 1);

	//SendAddParty(MobId,Leader, 1);

	//if(ptCont == 0)
	//	SendAddParty(MobId,MobId, 0);

	//SendAddParty(Leader,MobId, 0);

	//for(int i=0; i<12; i++)
	//{
	//	int partyID = Mob[Leader].PartyList[i];

	//	if(partyID == 0)
	//		continue;

	//	if(partyID != MobId)
	//		SendAddParty(partyID,MobId,0);

	//	SendAddParty(MobId,partyID,0);
	//}
   Mob[mobId].SpawnType = User;//

	return 1;
}

// Boa parte feita, mas tem que revisar bem, principalmente a parte das fadas e etc.

int ProcessAffect(int clientId)
{
	INT32 LOCAL_1 = 0;
	INT32 LOCAL_2 = 0;
	INT32 LOCAL_3 = 0;
	INT32 LOCAL_4 = 0;
	INT32 LOCAL_6 = 0;
	INT32 LOCAL_5 = Mob[clientId].Mobs.Player.Equip[13].Index;
	if(clientId < MAX_PLAYER && (LOCAL_5 == 754 || LOCAL_5 == 769 || LOCAL_5 == 1726))
	{
		INT32 LOCAL_6 = Mob[clientId].CurrentTarget;

		if(LOCAL_6)
		{
			if(LOCAL_6 != clientId)
			{
				if(LOCAL_6 <= 0 || LOCAL_6 > 30000)
					Mob[clientId].CurrentTarget = 0;
				else
				{
					if(Mob[clientId].Mode == 0)
						Mob[clientId].CurrentTarget = 0;
					else
					{
						MapAttribute LOCAL_7 = GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y); //GET ATTRIBUTE
						MapAttribute targetAttribute = GetAttribute(Mob[LOCAL_6].Target.X, Mob[LOCAL_6].Target.Y); //GET ATTRIBUTE
						if(LOCAL_7.PvP)
						{
							if((LOCAL_6 < MAX_PLAYER || Mob[LOCAL_6].Mobs.Player.CapeInfo == 4) && !Users[clientId].AllStatus.PK)
								Mob[clientId].CurrentTarget = 0;
						}

						if (LOCAL_6 < MAX_PLAYER && !targetAttribute.PvP)
							Mob[clientId].CurrentTarget = 0;
					}
				}

				if (Mob[clientId].CurrentTarget != 0)
				{
					INT32 LOCAL_8 = Mob[clientId].Leader;
					if (LOCAL_8 == 0)
						LOCAL_8 = clientId;

					INT32 LOCAL_9 = Mob[clientId].Leader;
					if (LOCAL_9 == 0)
						LOCAL_9 = LOCAL_6;

					INT32 LOCAL_10 = Mob[clientId].Mobs.Player.GuildIndex;
					if (Mob[clientId].GuildDisable != 0)
						LOCAL_10 = 0;

					INT32 LOCAL_11 = Mob[LOCAL_6].Mobs.Player.GuildIndex;
					if (Mob[LOCAL_6].GuildDisable != 0)
						LOCAL_11 = 0;

					if (LOCAL_10 == 0 && LOCAL_11 == 0)
						LOCAL_10 = -1;

					if (LOCAL_8 == LOCAL_9 || LOCAL_10 == LOCAL_11)
						Mob[clientId].CurrentTarget = 0;
					//4503AC
					INT32 LOCAL_12 = 0;
					INT32 LOCAL_13 = Mob[clientId].Mobs.Player.CapeInfo;
					INT32 LOCAL_14 = Mob[LOCAL_6].Mobs.Player.CapeInfo;

					if ((LOCAL_13 == 7 && LOCAL_14 == 7) || (LOCAL_13 == 8 && LOCAL_14 == 8))
						LOCAL_12 = 1;


					if ((LOCAL_12 != 1 || Users[clientId].AllStatus.PK != 0) || (LOCAL_12 != 1 || LOCAL_6 < MAX_PLAYER))
					{
						UINT32 LOCAL_15 = Mob[clientId].Target.X;
						UINT32 LOCAL_16 = Mob[clientId].Target.Y;

						if (Mob[LOCAL_6].Target.X >= (LOCAL_15 - VIEWGRIDX) && Mob[LOCAL_6].Target.X <= (LOCAL_15 + VIEWGRIDX) &&
							Mob[LOCAL_6].Target.Y >= (LOCAL_16 - VIEWGRIDY) && Mob[LOCAL_6].Target.Y <= (LOCAL_16 + VIEWGRIDY))
						{
							p39D packet;
							memset(&packet, 0, sizeof packet);

							packet.Header.PacketId = 0x39D;
							packet.Header.ClientId = clientId;
							packet.Header.Size = sizeof packet;
							packet.Header.TimeStamp = 0x0E0A1ACA;

							packet.attackerPos.X = Mob[clientId].Target.X;
							packet.attackerPos.Y = Mob[clientId].Target.Y;

							packet.attackerId = clientId;
							packet.Motion = 0xFE;

							packet.skillId = 32;
							if (LOCAL_5 == 769)
								packet.skillId = 34;
							else if (LOCAL_5 == 1726)
								packet.skillId = 36;

							packet.Target.Index = LOCAL_6;
							packet.Target.Damage = -1;

							// 004505C9

							Users[clientId].TimeStamp.TimeStamp = 0x0E0A1ACA;
							Users[clientId].PacketControl((BYTE*)& packet, sizeof p39D);
							Users[clientId].TimeStamp.TimeStamp = 0;
						}
						else
							Mob[clientId].CurrentTarget = 0;
					}
				}
			}
			else
				Mob[clientId].CurrentTarget = 0;
		}
	}

	for(INT32 LOCAL_28 = 0; LOCAL_28 < 32; LOCAL_28 ++)
	{
		INT32 LOCAL_29 = Mob[clientId].Mobs.Affects[LOCAL_28].Index;
		if(LOCAL_29 <= 0)
			continue;

		INT32 LOCAL_30 = Mob[clientId].Mobs.Player.Status.maxHP;
		INT32 LOCAL_31 = Mob[clientId].Mobs.Player.Status.curHP;
		INT32 LOCAL_32 = Mob[clientId].Mobs.Affects[LOCAL_28].Master;
		INT32 LOCAL_33 = Mob[clientId].Mobs.Affects[LOCAL_28].Value;
		INT32 LOCAL_34 = Mob[clientId].Mobs.Player.Status.Level;

		if(Mob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			LOCAL_34 += 400;

		if(LOCAL_29 == 17) // AURA DA VIDA
		{
			INT32 LOCAL_35 = (LOCAL_33 << 1) + LOCAL_32 + 50;
			LOCAL_31 = LOCAL_31 + LOCAL_35;

			if(LOCAL_31 < 1)
				LOCAL_31 = 1;

			if(LOCAL_31 > LOCAL_30)
				LOCAL_31 = LOCAL_30;

			if(Mob[clientId].Mobs.Player.Status.curHP != LOCAL_31)
				LOCAL_1 = 1;

			LOCAL_4 = LOCAL_31 - Mob[clientId].Mobs.Player.Status.curHP;

			Mob[clientId].Mobs.Player.Status.curHP = LOCAL_31;

			LOCAL_3 = 1;
		}
		else if (LOCAL_29 == 56)
		{
			auto event = static_cast<TOD_BossEvent*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::BossEvent));

			if (clientId >= MAX_PLAYER && strcmp(Mob[clientId].Mobs.Player.Name, event->GetBossName().c_str()) != 0)
			{
				// value (seria a refinação)
				int percent = 5 + LOCAL_33;
				int damage = -(LOCAL_30 * percent / 100);

				int removedDamage = GetDamageByJewel(clientId, damage);

				int remainingHp = LOCAL_31 + removedDamage;
				if (remainingHp < 1)
					remainingHp = 1;

				Mob[clientId].Mobs.Player.Status.curHP = remainingHp;
				if (clientId < MAX_PLAYER)
					Users[clientId].Potion.CountHp += removedDamage;

				if (LOCAL_31 != remainingHp)
					LOCAL_1 = 1;

				if (clientId >= MAX_PLAYER && Mob[clientId].Mobs.Player.CapeInfo == 4)
					LinkMountHp(clientId);

				LOCAL_4 = damage;
				LOCAL_3 = 1;
			}
		}
		else if(LOCAL_29 == 20) // VENENO 
		{
			auto event = static_cast<TOD_BossEvent*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::BossEvent));

			if (clientId < MAX_PLAYER || (clientId >= MAX_PLAYER && strcmp(Mob[clientId].Mobs.Player.Name, event->GetBossName().c_str()) != 0))
			{
				LOCAL_32 = 100;
				if (clientId >= MAX_PLAYER)
					LOCAL_32 = LOCAL_32 - Mob[clientId].Mobs.Player.Learn[0];

				LOCAL_32 = LOCAL_32 / 10;

				float LOCAL_164 = (100.0f - LOCAL_32) / 100.0f;
				INT32 LOCAL_37 = 1000;

				if (clientId >= MAX_PLAYER)
					LOCAL_37 = (100 - Mob[clientId].Mobs.Player.Learn[0]) * 10;

				INT32 LOCAL_38 = (LOCAL_31 - LOCAL_37);
				LOCAL_31 = static_cast<float>(LOCAL_31) * LOCAL_164;

				if (LOCAL_31 < LOCAL_38)
					LOCAL_31 = LOCAL_38;

				if (LOCAL_31 < 1)
					LOCAL_31 = 1;

				if (LOCAL_31 > LOCAL_30)
					LOCAL_31 = LOCAL_30;

				if (Mob[clientId].Mobs.Player.Status.curHP != LOCAL_31)
					LOCAL_1 = 1;

				LOCAL_4 = LOCAL_31 - Mob[clientId].Mobs.Player.Status.curHP;
				if (clientId >= MAX_PLAYER)
				{
					INT32 itemId = Mob[clientId].Mobs.Player.Equip[13].Index;
					if (itemId == 786 || itemId == 1936 || itemId == 1937)
					{
						INT32 _sanc = GetItemSanc(&Mob[clientId].Mobs.Player.Equip[13]); // local209
						if (_sanc < 2)
							_sanc = 2;

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

						multHP *= _sanc;
						INT32 damage = -LOCAL_4 / multHP;

						LOCAL_31 = Mob[clientId].Mobs.Player.Status.curHP - damage;
					}
				}

				Mob[clientId].Mobs.Player.Status.curHP = LOCAL_31;

				// Aqui ele adiciona a parada na struct
				if (clientId > 0 && clientId < MAX_PLAYER)
					Users[clientId].Potion.CountHp += LOCAL_4;

				LOCAL_3 = 1;

				if (clientId >= MAX_PLAYER && Mob[clientId].Mobs.Player.CapeInfo == 4)
					LinkMountHp(clientId);
			}
		}
		/*
		else if(LOCAL_29 == 21)
		{
			INT32 LOCAL_39 = LOCAL_30;
				
			LOCAL_39 = LOCAL_39 * LOCAL_32 / 100;

			LOCAL_31 = LOCAL_31 + LOCAL_39;

			if(LOCAL_31 < 1)
				LOCAL_31 = 1;

			if(LOCAL_31 > LOCAL_30)
				LOCAL_31 = LOCAL_30;
				
			if(Mob[clientId].Mobs.Player.Status.curHP != LOCAL_31)
				LOCAL_1 = 1;

			LOCAL_4 = Mob[clientId].Mobs.Player.Status.curHP - LOCAL_31;
				
			Mob[clientId].Mobs.Player.Status.curHP = LOCAL_31;
		}*/
		else if(LOCAL_29 == 23) // AURA BESTIAL
		{ // 00451A99
			if(clientId >= MAX_PLAYER)
				goto check;

			MapAttribute LOCAL_108 = GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y);
			INT32 LOCAL_109 = Mob[clientId].Mobs.Player.CapeInfo;
			if(LOCAL_108.Village)
				goto check;

			p367 LOCAL_132;
			memset(&LOCAL_132, 0, sizeof p367);

			INT32 LOCAL_133 = Mob[clientId].CurrentTarget;
			if(LOCAL_133 != 0)
			{
				INT32 LOCAL_134 = 0;
				INT32 LOCAL_135 = Mob[clientId].Target.X;
				INT32 LOCAL_136 = Mob[LOCAL_133].Target.X;

				if(LOCAL_133 == clientId)
					Mob[clientId].CurrentTarget = 0;
				else
				{
					if(LOCAL_133 <= 0 || LOCAL_133 >= 30000)
						Mob[clientId].CurrentTarget = 0;
					else
					{
						if(Mob[LOCAL_133].Mode == 0)
							Mob[clientId].CurrentTarget = 0;
						else
						{
							MapAttribute LOCAL_137 = GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y);
							INT32 LOCAL_138 = Mob[clientId].Leader;
							if(LOCAL_138 == 0)
								LOCAL_138 = clientId;

							INT32 LOCAL_139 = Mob[LOCAL_133].Leader;
							if(LOCAL_139 == 0)
								LOCAL_139 = LOCAL_133;

							INT32 LOCAL_140 = Mob[clientId].Mobs.Player.GuildIndex;
							if(Mob[clientId].GuildDisable != 0)
								LOCAL_140 = 0;

							INT32 LOCAL_141 = Mob[LOCAL_133].Mobs.Player.GuildIndex;
							if(Mob[LOCAL_133].GuildDisable != 0)
								LOCAL_141 = 0;

							INT32 LOCAL_142 = g_pGuildAlly[LOCAL_140];
							if(LOCAL_142 == 0)
								LOCAL_142 = -2;

							if(LOCAL_140 == 0 && LOCAL_141 == 0)
								LOCAL_140 = -1;

							// 00451D27
							if(LOCAL_138 == LOCAL_139)
								goto label;

							if(LOCAL_140 == LOCAL_141)
								goto label;

							if(LOCAL_142 == LOCAL_141)
								goto label;

							// é um mob.
							if (LOCAL_133 >= MAX_PLAYER)
							{
								// PK desativado e é uma evocação.
								if (!Users[clientId].AllStatus.PK && Mob[LOCAL_133].Summoner)
									goto label;
							}
							if(LOCAL_133 < MAX_PLAYER)
							{
								if(!Users[clientId].AllStatus.PK)
								{
									// essa verificação sempre vai entrar lol
									if(LOCAL_133 < MAX_PLAYER || LOCAL_136 == 4 || LOCAL_134 == 1)
										goto label;
								}

								if(!LOCAL_137.PvP)
								{
									if(LOCAL_133 < MAX_PLAYER || LOCAL_136 ==4 || LOCAL_134 == 1)
										goto label;
								}

									
								if(Mob[LOCAL_133].Mobs.Player.AffectInfo.Snoop || Mob[LOCAL_133].Mobs.Player.Info.Merchant & 1)
								{
									Mob[clientId].CurrentTarget = 0;
									goto label;
								}

								if(Mob[LOCAL_133].Mobs.Player.AffectInfo.SlowMov)
									continue;
							}

							if((LOCAL_135 == 7 && LOCAL_136 == 7) || (LOCAL_135 == 8 && LOCAL_136 == 8))
								LOCAL_134 = 1;

							if(clientId < MAX_PLAYER)
							{
								if(LOCAL_134 == 1 && !Users[clientId].AllStatus.PK) 
									goto label;

								if(!Users[clientId].AllStatus.PK && LOCAL_133 < MAX_PLAYER)
								{
									Mob[clientId].CurrentTarget = 0;

									goto label;
								}
							}

							if(LOCAL_134 == 1 && LOCAL_133 >= MAX_PLAYER)
								goto label;

							UINT32 LOCAL_143 = Mob[clientId].Target.X;
							UINT32 LOCAL_144 = Mob[clientId].Target.Y;

							if (Mob[LOCAL_133].Target.X < LOCAL_143 - VIEWGRIDX || Mob[LOCAL_133].Target.X > LOCAL_143 + VIEWGRIDX || Mob[LOCAL_133].Target.Y < LOCAL_144 - VIEWGRIDY || Mob[LOCAL_133].Target.Y > LOCAL_144 + VIEWGRIDY)
							{
								Mob[clientId].CurrentTarget = 0;

								goto label;
							}

							LOCAL_132.Target[0].Index = LOCAL_133;
							LOCAL_132.Target[0].Damage = -1;
						}
					}
				}
			}
	label:
			INT32 LOCAL_145 = 0;
			INT32 LOCAL_146 = Mob[clientId].Target.Y - 1;
			INT32 LOCAL_147 = Mob[clientId].Target.X - 1;

			for(INT32 LOCAL_148 = LOCAL_146; LOCAL_148 <= LOCAL_146 + 2; LOCAL_148++)
			{
				if(LOCAL_148 < 0 || LOCAL_148 >= 4096)
					continue;

				for(INT32 LOCAL_149 = LOCAL_147; LOCAL_149 <= LOCAL_147 + 2; LOCAL_149++)
				{
					INT32 LOCAL_150 = g_pMobGrid[LOCAL_148][LOCAL_149];
					if(LOCAL_150 <= 0 || LOCAL_150 > 30000)
						continue;

					if(Mob[LOCAL_150].Mode == 0)
						continue;

					INT32 LOCAL_151 = Mob[LOCAL_150].Mobs.Player.CapeInfo;
					INT32 LOCAL_152 = 0;

					// 004520BE

					if((LOCAL_109 == 7 && LOCAL_151 == 7) || (LOCAL_109 == 8 && LOCAL_151 == 8))
						LOCAL_152 = 1;

					if(LOCAL_152 == 1 && LOCAL_150 >= MAX_PLAYER)
						continue;

					if(LOCAL_151 == 6)
						continue;

					if(LOCAL_150 == clientId)
						continue;

					INT32 LOCAL_153 = Mob[clientId].Leader;
					if(LOCAL_153 == 0)
						LOCAL_153 = clientId;

					INT32 LOCAL_154 = Mob[LOCAL_150].Leader;
					if(LOCAL_154 == 0)
						LOCAL_154 = LOCAL_150;

					INT32 LOCAL_155 = Mob[clientId].Mobs.Player.GuildIndex;
					if(Mob[clientId].GuildDisable != 0)
						LOCAL_155 = 0;

					INT32 LOCAL_156 = Mob[LOCAL_150].Mobs.Player.GuildIndex;
					if(Mob[LOCAL_150].GuildDisable != 0)
						LOCAL_156 = 0;

					INT32 LOCAL_157 =  g_pGuildAlly[LOCAL_155];
					if(LOCAL_157 == 0)
						LOCAL_157 = -2;

					if(LOCAL_155 == 0 && LOCAL_156 == 0)
						LOCAL_155 = -1;

					if(LOCAL_153 == LOCAL_154)
						continue;
						
					if(LOCAL_155 == LOCAL_156)
						continue;

					if(LOCAL_157 == LOCAL_156)
						continue;

					//0045226B
					if(clientId < MAX_PLAYER)
					{
						if(!Users[clientId].AllStatus.PK)
						{
							// Verifica se o alvo é um player ou se é uma evocação.
							// Se tiver com PK ativo, não vai atacar.
							if(LOCAL_150 < MAX_PLAYER || Mob[LOCAL_150].Summoner)// || LOCAL_152 == 1)
								continue;
						}

						if(!LOCAL_108.PvP)
						{
							if(LOCAL_150 < MAX_PLAYER)// || LOCAL_152 == 1)
								continue;
						}

						if(Mob[LOCAL_150].Mobs.Player.AffectInfo.Snoop || Mob[LOCAL_150].Mobs.Player.Info.Merchant & 1)
							continue;
					}
					else
					{
						if(LOCAL_150 >= MAX_PLAYER && LOCAL_151 != 4)
							continue;
					}

					LOCAL_132.Target[LOCAL_145].Index = LOCAL_150;
					LOCAL_132.Target[LOCAL_145].Damage = -1;

					LOCAL_145++;
				}
			}

			if(!LOCAL_132.Target[0].Index)
				continue;

			LOCAL_132.Header.PacketId = 0x367;
			LOCAL_132.Header.ClientId = clientId;
			LOCAL_132.Header.Size = sizeof p367;
			LOCAL_132.Header.TimeStamp = 0xE0A1ACA;
				
			LOCAL_132.attackerPos.X = Mob[clientId].Target.X;
			LOCAL_132.attackerPos.Y = Mob[clientId].Target.Y;

			LOCAL_132.Motion = -2;
			LOCAL_132.attackerId = clientId;

			LOCAL_132.skillId = 52;

			Users[clientId].TimeStamp.TimeStamp = 0xE0A1ACA;
			Users[clientId].PacketControl((BYTE*)&LOCAL_132, sizeof p367);
			Users[clientId].TimeStamp.TimeStamp = 0;

		}
		else if(LOCAL_29 == 22)
		{ // 00450943
			INT32 LOCAL_40 = 0;
			INT32 LOCAL_41 = 0;
			INT32 LOCAL_42 = 0;
			INT32 LOCAL_43 = 0;
			INT32 LOCAL_44 = 0;
			INT32 LOCAL_45 = 0;
			MapAttribute LOCAL_46 = GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y);
			INT32 LOCAL_47 = Mob[clientId].Mobs.Player.CapeInfo;

			if(LOCAL_46.Village)
				goto check;

			//004509E4
			INT32 LOCAL_48 = Mob[clientId].Target.Y - 1;
			INT32 LOCAL_49 = Mob[clientId].Target.X - 1;
			INT32 LOCAL_50 = Mob[clientId].Leader;

			if(LOCAL_50 <= 0)
				LOCAL_50 = clientId;

			for(INT32 LOCAL_51 = LOCAL_48; LOCAL_51 <= (LOCAL_48 + 2); LOCAL_51++)
			{
				if(LOCAL_51 < 0 || LOCAL_51 >= 4096)
					continue;

				for(INT32 LOCAL_52 = LOCAL_49; LOCAL_52 <= (LOCAL_49 + 2); LOCAL_52++)
				{
					if(LOCAL_52 < 0 || LOCAL_52 >= 4096)
						continue;

					INT32 LOCAL_53 = g_pMobGrid[LOCAL_51][LOCAL_52];
					if(LOCAL_53 <= 0 || LOCAL_53 >= 30000)
						continue;

					if(Mob[LOCAL_53].Mode == 0)
						continue;

					if(Mob[LOCAL_53].Mobs.Player.Status.curHP <= 0)
						continue;

					if(LOCAL_50 == Mob[LOCAL_53].Leader)
						continue;

					INT32 LOCAL_54 = Mob[LOCAL_53].Mobs.Player.CapeInfo;
					INT32 LOCAL_55 = 0;

					if((LOCAL_47 == 7 && LOCAL_54 == 7) || (LOCAL_47 == 8 && LOCAL_54 == 8))
						LOCAL_55 = 1;

					//if(Mob[LOCAL_53].Mobs.Player.AffectInfo.SlowMov)
					//	continue;
					//00450C12  |. 85C0           ||TEST EAX,EAX
					if(clientId < MAX_PLAYER)
					{
						if(!Users[clientId].AllStatus.PK)
						{
							if(LOCAL_53 < MAX_PLAYER || LOCAL_54 == 4) //||// LOCAL_55 == 1)
								continue;
						}

						if(!LOCAL_46.PvP && (LOCAL_53 < MAX_PLAYER || LOCAL_54 == 4))// || LOCAL_55 == 1))
							continue;

						if(Mob[LOCAL_53].Mobs.Player.AffectInfo.Snoop || Mob[LOCAL_53].Mobs.Player.Info.Merchant & 1)
							continue;
					}
					else
					{
						if(LOCAL_53 >= MAX_PLAYER && LOCAL_54 == 4)
							continue;
					}

					if(LOCAL_55 == 1 && LOCAL_53 >= MAX_PLAYER)
						continue;

					if(LOCAL_54 == 6)
						continue;

					if(LOCAL_53 == clientId)
						continue;

					INT32 LOCAL_56 = Mob[clientId].Leader;
					if(LOCAL_56 == 0)
						LOCAL_56 = clientId;

					// 00450CCB
					INT32 LOCAL_57 = Mob[LOCAL_53].Leader;
					if(LOCAL_57 == 0)
						LOCAL_57 = LOCAL_53;

					INT32 LOCAL_58 = Mob[clientId].Mobs.Player.GuildIndex;
					if(Mob[clientId].GuildDisable != 0)
						LOCAL_58 = 0;

					INT32 LOCAL_59 = Mob[LOCAL_53].Mobs.Player.GuildIndex;
					if(Mob[LOCAL_53].GuildDisable != 0)
						LOCAL_59 = 0;

					INT32 LOCAL_60 = g_pGuildAlly[LOCAL_58];
					if(LOCAL_60 == 0)
						LOCAL_60 = -2;

					if(LOCAL_58 == 0 && LOCAL_59 == 0)
						LOCAL_58 = -1;

					// 00450D6E - GuildAlly não feito
					if(LOCAL_56 == LOCAL_57)
						continue;

					if(LOCAL_58 == LOCAL_59)
						continue;

					//
					if(LOCAL_60 == LOCAL_59)
						continue;

					if(LOCAL_40 == 0)
					{
						LOCAL_40 = LOCAL_53;
						continue;
					}

					if(LOCAL_40 == LOCAL_53)
						continue;

					if(LOCAL_41 == 0)
					{
						LOCAL_41 = LOCAL_53;
						continue;
					}

					if(LOCAL_41 == LOCAL_53)
						continue;

					// 00450E3E
					if(LOCAL_42 == 0)
					{
						LOCAL_42 = LOCAL_53 ;
						continue;
					}

					if(LOCAL_42 == LOCAL_53)
						continue;

					if(LOCAL_43 == 0)
					{
						LOCAL_43 = LOCAL_53;
						continue;
					}

					if(LOCAL_43 == LOCAL_53)
						continue;

					if(LOCAL_44 == 0)
					{
						LOCAL_44 = LOCAL_53;
						continue;
					}

					if(LOCAL_44 == LOCAL_53)
						continue;

					if(LOCAL_45 == 0)
					{
						LOCAL_45 = LOCAL_53;
						break;
					}
				}

				if(LOCAL_45 != 0)
					break;
			}

			if(LOCAL_42 == 0 || LOCAL_43 == 0 || LOCAL_44 == 0 || LOCAL_45 == 0)
			{
				INT32 LOCAL_61 = Mob[clientId].Target.Y - 2;
				INT32 LOCAL_62 = Mob[clientId].Target.X - 2;

				for(INT32 LOCAL_63 = LOCAL_61; LOCAL_63 <= LOCAL_61 + 4; LOCAL_63++)
				{
					if(LOCAL_63 < 0 || LOCAL_63 >= 4096)
						continue;

					for(INT32 LOCAL_64 = LOCAL_62; LOCAL_64 <= LOCAL_62 + 4; LOCAL_64++)
					{
						if(LOCAL_64 < 0 || LOCAL_64 >= 4096)
							continue;

						INT32 LOCAL_65 = g_pMobGrid[LOCAL_63][LOCAL_64];
						if(LOCAL_65 <= 0 || LOCAL_65 >= MAX_SPAWN_MOB)
							continue;

						if(Mob[LOCAL_65].Mode == 0)
							continue;

						if(Mob[LOCAL_65].Mobs.Player.Status.curHP <= 0)
							continue;

						// 00451041
						if(LOCAL_50 == Mob[LOCAL_65].Leader)
							continue;

						INT32 LOCAL_66 = Mob[LOCAL_65].Mobs.Player.CapeInfo;
						INT32 LOCAL_67 = 0;
						if((LOCAL_47 == 7 && LOCAL_66 == 7) || (LOCAL_47 == 8 && LOCAL_66 == 8))
							LOCAL_67 = 1;

						if(LOCAL_67 == 1 && LOCAL_65 >= MAX_PLAYER)
							continue;

						//if(Mob[LOCAL_65].Mobs.Player.AffectInfo.SlowMov)
						//	continue;

						if(clientId < MAX_PLAYER)
						{
							if(!Users[clientId].AllStatus.PK)
							{
								if(LOCAL_65 < MAX_PLAYER || LOCAL_66 == 4 || LOCAL_67 == 1)
									continue;
							}

							if(!LOCAL_46.PvP)
							{
								if(LOCAL_65 < MAX_PLAYER || LOCAL_66 == 4  || LOCAL_67 == 1)
									continue;
							}

							if((Mob[LOCAL_65].Mobs.AffectInfo & 0x40) || Mob[LOCAL_65].Mobs.Player.Info.Merchant & 1)
								continue;
						}
						else
						{
							if(LOCAL_65 >= MAX_PLAYER && LOCAL_66 == 4)
								continue;
						}

						if(LOCAL_65 == clientId)
							continue;

						if(LOCAL_66 == 6)
							continue;

						INT32 LOCAL_68 = Mob[clientId].Leader;
						if(LOCAL_68 == 0)
							LOCAL_68 = clientId;

						// 004511D4
						INT32 LOCAL_69 = Mob[LOCAL_65].Leader;
						if(LOCAL_69 == 0)
							LOCAL_69 = LOCAL_65;

						INT32 LOCAL_70 = Mob[clientId].Mobs.Player.GuildIndex;
						if(Mob[clientId].GuildDisable != 0)
							continue;

						INT32 LOCAL_71 = Mob[LOCAL_65].Mobs.Player.GuildIndex;
						if(Mob[LOCAL_65].GuildDisable != 0)
							LOCAL_71 = 0;

						INT32 LOCAL_72 = g_pGuildAlly[LOCAL_70]; // 00451277 - GuildAlly
						if(LOCAL_72 == 0)
							LOCAL_72 = -2;

						if(LOCAL_70 == 0 && LOCAL_71 == 0)
							LOCAL_70 = -1;

						if(LOCAL_68 == LOCAL_69)
							continue;

						if(LOCAL_70 == LOCAL_71)
							continue;

						if(LOCAL_72 == LOCAL_71)
							continue;

						if(LOCAL_40 == 0)
						{
							LOCAL_40 = LOCAL_65;

							continue;
						}

						if(LOCAL_40 == LOCAL_65)
							continue;

						if(LOCAL_41 == 0)
						{
							LOCAL_41 = LOCAL_65;
							continue;
						}

						if(LOCAL_41 == LOCAL_65)
							continue;

						if(LOCAL_42 == 0)
						{
							LOCAL_42 = LOCAL_65;

							continue;
						}

						if(LOCAL_42 == LOCAL_65)
							continue;

						if(LOCAL_43 == 0)
						{
							LOCAL_43 = LOCAL_65;
							continue;
						}

						if(LOCAL_43 == LOCAL_65)
							continue;

						if(LOCAL_44 == 0)
						{
							LOCAL_44 = LOCAL_65;

							continue;
						}

						if(LOCAL_44 == LOCAL_65)
							continue;

						if(LOCAL_45 == 0)
						{
							LOCAL_45 = LOCAL_65;
							break;
						}
					}

					if(LOCAL_45 != 0)
						break;
				}
			}

			if(LOCAL_40 != 0)
			{
				p367 LOCAL_95;
				memset(&LOCAL_95, 0, sizeof p367);

				LOCAL_95.Header.PacketId = 0x367;
				LOCAL_95.Header.ClientId = clientId;
				LOCAL_95.Header.Size = sizeof p367;

				LOCAL_95.Header.TimeStamp = 0x0E0A1ACA;
				LOCAL_95.attackerPos.X = Mob[clientId].Target.X;
				LOCAL_95.attackerPos.Y = Mob[clientId].Target.Y;

				LOCAL_95.Motion = -2;

				if(Mob[clientId].Mobs.Player.Equip[0].Index == 219)
					LOCAL_95.Motion = -4;

				LOCAL_95.attackerId = clientId;
				LOCAL_95.skillId = 33;

				INT32 LOCAL_96 = Rand() % 100;

				INT32 LOCAL_97 = LOCAL_33 + LOCAL_34  + LOCAL_96;

				LOCAL_95.Target[0].Index = LOCAL_40;
				LOCAL_95.Target[0].Damage = -1;

				if(LOCAL_97 > 300)
				{
					LOCAL_95.Target[1].Index = LOCAL_41;
					LOCAL_95.Target[1].Damage = -1;
				}
				if(LOCAL_97 > 350)
				{
					LOCAL_95.Target[2].Index = LOCAL_42;
					LOCAL_95.Target[2].Damage = -1;
				}
				if(LOCAL_97 > 400)
				{
					LOCAL_95.Target[3].Index = LOCAL_43;
					LOCAL_95.Target[3].Damage = -1;
				}
				if(LOCAL_97 > 450)
				{
					LOCAL_95.Target[4].Index = LOCAL_44;
					LOCAL_95.Target[4].Damage = -1;
				}
				if(LOCAL_97 > 500)
				{
					LOCAL_95.Target[5].Index = LOCAL_45;
					LOCAL_95.Target[5].Damage = -1;
				}

				if(clientId < MAX_PLAYER)
				{
					Users[clientId].TimeStamp.TimeStamp = 0x00E0A1ACA;
					Users[clientId].PacketControl((BYTE*)&LOCAL_95, sizeof p367);
				}
				else
				{
					//004515B2
					INT32 LOCAL_98 = 0;

					for(;  LOCAL_98 < 13; LOCAL_98 ++)
					{
						INT32 LOCAL_99 = LOCAL_95.Target[LOCAL_98].Index;
						INT32 LOCAL_100 = Mob[clientId].Mobs.Player.bStatus.Attack;
						LOCAL_100 = (LOCAL_100 * ((Rand() & 0x80000007) + 5)) / 10;
						LOCAL_100 = LOCAL_100 - (Mob[clientId].Mobs.Player.Status.Defense >> 1);

						if(LOCAL_100 < 0)
							LOCAL_100 = Rand() % 100;

						LOCAL_95.Target[LOCAL_98].Damage = LOCAL_100;
						if(LOCAL_99 <= 0 || LOCAL_99 >= 30000)
							continue;

						INT32 LOCAL_101 = Mob[LOCAL_99].Leader;
						if(LOCAL_95.Target[LOCAL_98].Damage > 0)
						{
							if(LOCAL_101 <= 0)
								LOCAL_101 = LOCAL_99;

							SetBattle(LOCAL_101, LOCAL_28);
							SetBattle(LOCAL_28, LOCAL_101);

							for(INT32 LOCAL_102 = 0; LOCAL_102 < 12;  LOCAL_102++)
							{
								INT32 LOCAL_103 = Mob[LOCAL_101].PartyList[LOCAL_102];

								if(LOCAL_103 < MAX_PLAYER)
									continue;

								if(Mob[LOCAL_103].Mode == 0 || !Mob[LOCAL_103].Mobs.Player.Status.curHP)
								{
									if(Mob[LOCAL_103].Mode != 0)
										DeleteMob(LOCAL_103, 1);

									Mob[LOCAL_103].PartyList[LOCAL_102] = 0;
									continue;
								}

								SetBattle(LOCAL_103, LOCAL_28);
								SetBattle(LOCAL_28, LOCAL_103);
							}
						}

						if(LOCAL_95.Target[0].Damage > 0 || LOCAL_95.Target[0].Damage <= -5)
						{
							INT32 LOCAL_104 = LOCAL_95.Target[0].Damage;
							INT32 petDamage = 0;
							INT32 petId = Mob[LOCAL_99].Mobs.Player.Equip[14].Index;

							if(Mob[LOCAL_99].isPetAlive())
							{
								if (Mob[LOCAL_99].isNormalPet())
								{
									LOCAL_104 = AbsorveDamageByPet(&Mob[LOCAL_99], LOCAL_95.Target[0].Damage);
									petDamage = LOCAL_95.Target[0].Damage - LOCAL_104;
								}
								LOCAL_95.Target[0].Damage = LOCAL_104;
							}
							
							INT32 itemId = Mob[LOCAL_99].Mobs.Player.Equip[13].Index;
							if(itemId == 786 || itemId == 1936 || itemId == 1937)
							{
								INT32 LOCAL_107 = GetItemSanc(&Mob[LOCAL_99].Mobs.Player.Equip[13]);
								if(LOCAL_107 < 2)
									LOCAL_107 = 2;
								
								INT32 multHP = 1;
								switch(itemId)
								{
									case 1936:
										multHP = 10;
										break;

									case 1937:
										multHP = 1000;
										break;
								}

								multHP *= LOCAL_107;
								Mob[LOCAL_99].Mobs.Player.Status.curHP -= (LOCAL_95.Target[0].Damage / multHP);
							}
							else
								Mob[LOCAL_99].Mobs.Player.Status.curHP -= LOCAL_95.Target[0].Damage;

							if(LOCAL_99 >= MAX_PLAYER && Mob[LOCAL_99].Mode == 4)
								LinkMountHp(LOCAL_99);

							if(petDamage > 0)
								ProcessAdultMount(LOCAL_99 , petDamage);
						}

						if(LOCAL_99 > 0 && LOCAL_99 < MAX_PLAYER)
						{
							Users[LOCAL_99].Potion.CountHp -= LOCAL_95.Target[0].Damage;

							SetReqHp(LOCAL_99);
						}

						if(Mob[LOCAL_99].Mobs.Player.Status.curHP <= 0)
							MobKilled(LOCAL_99, clientId, 0, 0);
					}

					GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&LOCAL_95, 0);
				}
			}
		}

check:
		Mob[clientId].Mobs.Affects[LOCAL_28].Time -= 1;

		if((INT32)Mob[clientId].Mobs.Affects[LOCAL_28].Time <= 0)
		{
			if(Mob[clientId].Mobs.Affects[LOCAL_28].Index == 16)
			{
				LOCAL_2 = 1;

				Mob[clientId].Mobs.Player.Equip[0].Index = Mob[clientId].Mobs.Player.Equip[0].EF2;
			}

			if(Mob[clientId].Mobs.Affects[LOCAL_28].Index == 8)
				Mob[clientId].Jewel = 0;

			Mob[clientId].Mobs.Affects[LOCAL_28].Index = 0;
			Mob[clientId].Mobs.Affects[LOCAL_28].Time = 0;
			Mob[clientId].Mobs.Affects[LOCAL_28].Master = 0;
			Mob[clientId].Mobs.Affects[LOCAL_28].Value = 0;

			LOCAL_1 = 1;
		}

	}

	if(LOCAL_3 != 0)
	{
		if(clientId > 0 && clientId < MAX_PLAYER)
		{	
			SetReqHp(clientId);
		
			SetReqMp(clientId); 
		}

		p18A packet;
		packet.Header.PacketId = 0x18A;
		packet.Header.Size = 18;
		packet.Header.ClientId = clientId;

		packet.CurHP = Mob[clientId].Mobs.Player.Status.curHP;
		packet.Incress = LOCAL_4;

		INT32 LOCAL_162 = Mob[clientId].Target.X;
		INT32 LOCAL_163 = Mob[clientId].Target.Y;

		GridMulticast_2(LOCAL_162, LOCAL_163, (BYTE*)&packet, 0);
	}

	if(LOCAL_6 != 0)
	{
	//	p364 packet;

	//	GetCreateMob(clientId, (BYTE*)&packet);
	//	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y,  (BYTE*)&packet, 0);
	}
	
	if(LOCAL_1 != 0)
	{
		Mob[clientId].GetCurrentScore(clientId);

		SendScore(clientId);

		if(LOCAL_2 != 0)
			SendEquip(clientId);

		//p364 packet;
		//GetCreateMob(clientId, (BYTE*)&packet);

		//GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);

		return 1;
	}

	return 0;
}

int ApplyHp(int clientId)
{
	//0x00401253
	if(Users[clientId].Potion.CountHp > Mob[clientId].Mobs.Player.Status.maxHP)
		Users[clientId].Potion.CountHp = Mob[clientId].Mobs.Player.Status.maxHP;
	
	int LOCAL_1 = Mob[clientId].Mobs.Player.Status.curHP,
		LOCAL_2 = Users[clientId].Potion.CountHp;

	if(LOCAL_2 <= LOCAL_1)
		return 0;

	INT32 level =  Mob[clientId].Mobs.Player.Status.Level;
	if(Mob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
		level += 400;

	int LOCAL_3 = level + 100;
	Mob[clientId].Mobs.Player.Status.curHP += LOCAL_3;
		
	if(Mob[clientId].Mobs.Player.Status.curHP > LOCAL_2)
		Mob[clientId].Mobs.Player.Status.curHP = LOCAL_2;
		
	return 1;
}

int ApplyMp(int clientId)
{
	//0x00401253
	if(Users[clientId].Potion.CountMp > Mob[clientId].Mobs.Player.Status.maxMP)
		Users[clientId].Potion.CountMp = Mob[clientId].Mobs.Player.Status.maxMP;
	
	int LOCAL_1 = Mob[clientId].Mobs.Player.Status.curMP,
		LOCAL_2 = Users[clientId].Potion.CountMp;

	if(LOCAL_2 <= LOCAL_1)
		return 0;
	
	INT32 level =  Mob[clientId].Mobs.Player.Status.Level;
	if(Mob[clientId].Mobs.Player.Equip[0].EFV2>= CELESTIAL)
		level += 400;

	int LOCAL_3 = level + 100;
		
	Mob[clientId].Mobs.Player.Status.curMP += LOCAL_3;
		
	if(Mob[clientId].Mobs.Player.Status.curMP > LOCAL_2)
		Mob[clientId].Mobs.Player.Status.curMP = LOCAL_2;
		
	return 1;
}

void AmountMinus(st_Item *item)
{
	int index = 0;
	int amount = 0;
	for(int i = 0;i < 3;i++)
	{
		if(item->Effect[i].Index == EF_AMOUNT)
		{
			index = i;
			amount = item->Effect[i].Value;

			break;
		}
	}

	if(amount <= 1)
		memset(item, 0, sizeof st_Item);
	else
		item->Effect[index].Value --;
}

void MountProcess(int clientId, st_Item *item)
{
	st_Item *LOCAL_1 = &Mob[clientId].Mobs.Player.Equip[14];

	INT32 LOCAL_2 = 1;
	if(item != nullptr)
		memcpy(item, LOCAL_1, 8);

	if(LOCAL_2 == 0)
		return;

	INT32 LOCAL_3 = Mob[clientId].Leader;
	if(LOCAL_3 == 0)
		LOCAL_3 = clientId;

	for(INT32 LOCAL_4 = 0; LOCAL_4 < 12; LOCAL_4 ++)
	{
		INT32 LOCAL_5 = Mob[LOCAL_3].PartyList[LOCAL_4];

		if(LOCAL_5 <= 0 || LOCAL_5 > 30000)
			continue;

		INT32 LOCAL_6 = Mob[LOCAL_5].Mobs.Player.Equip[0].Index;

		if(Mob[LOCAL_5].Summoner == clientId && LOCAL_6 >= 315 && LOCAL_6 < 345)
			DeleteMob(LOCAL_5, 3);
	}

	INT32 LOCAL_7 = LOCAL_1->Index - 2320;
	if(LOCAL_7 >= 10 && LOCAL_7 < 40)
	{
		INT32 LOCAL_8 = GetItemAbility(LOCAL_1, 80);

		if(LOCAL_8 >= 0)
			GenerateSummon(clientId, LOCAL_7, LOCAL_1);
	}
}

void RemoveParty(INT32 clientId)
{
	CMob *player = &Mob[clientId];

	int leader = player->Leader;
	if (leader < 0 || leader >= MAX_SPAWN_MOB)
		return;

	if (leader && leader < MAX_PLAYER)
	{
		if (Users[leader].Status != USER_PLAY)
		{
			player->Leader = 0;
			return;
		}
	}

	if (clientId > 0 && clientId < MAX_PLAYER)
		SendRemoveParty(clientId, 0);

	if (leader)
	{
		SendRemoveParty(leader, clientId);

		CMob *Leader = &Mob[leader];
		player->Leader = 0;

		for (int i = 0; i < 12; i++)
		{
			int partyMob = Leader->PartyList[i];

			if (!partyMob)
				continue;

			// Remove o clientid que saiu do grupo da array do lider
			if (partyMob == clientId)
				Leader->PartyList[i] = 0;

			// Remove os nomes da janela do client que saiu do grupo
			if (partyMob > 0 && partyMob < MAX_PLAYER)
				if (Users[partyMob].Status == USER_PLAY)
					SendRemoveParty(partyMob, clientId);
		}

	}
	else // Lider saindo do grupo
	{
		int groupCount = 0;
		// Encontra o número total de players no grupo
		for (INT8 i = 0; i < 12; i++)
			if (player->PartyList[i] < MAX_PLAYER && Users[player->PartyList[i]].Status == USER_PLAY)
				groupCount++;

		bool isClueLeader{ false };
		int clueSanc{ -1 };
		int clueParty{ -1 };

		// itera sobre todas as pistas
		for (int iPista = 0; iPista < 10; ++iPista)
		{
			// itera sobre todos os grupos
			for (int iParty = 0; iParty < MAX_PARTYPISTA; ++iParty)
			{
				auto& pista = pPista[iPista].Clients[iParty];

				int memberId = pista[12];
				if (memberId > 0 && memberId < MAX_PLAYER && clientId == memberId)
				{
					clueParty = iParty;
					clueSanc = iPista;

					isClueLeader = true;
					break;
				}
			}

			if (isClueLeader)
				break;
		}

		int newLeader = 0;
		for (int i = 0; i < 12; ++i)
		{
			if (player->PartyList[i] <= 0 || player->PartyList[i] >= MAX_PLAYER)
				continue;

			newLeader = player->PartyList[i];
			break;
		}

		// Caso o primeiro indice seja menor que 750 e o grupo tenha 2 jogadores ou mais.
		if (newLeader != 0 && groupCount >= 2)
		{
			// O primeiro indice vai ser o novo lider.
			CMob *newLider = &Mob[newLeader];

			for (int i = 0, count = 0; i < 12; ++i)	
			{
				if (player->PartyList[i] != 0 && player->PartyList[i] != newLeader)
					newLider->PartyList[count++] = player->PartyList[i];
			}

			// Informa o novo lider de sua condição
			newLider->Leader = 0;

			// Remove o cara 
			SendRemoveParty(newLeader, clientId);

			// Adiciona ele de novo como lider, provavelmente.
			SendAddParty(newLeader, newLeader, 1);

			if (isClueLeader)
			{
				auto& pista = pPista[clueSanc].Clients[clueParty];
				pista[12] = newLeader;

				for (int i = 0; i < 12; ++i)
				{
					if (pPista[clueSanc].Clients[clueParty][i] == newLeader)
					{
						pPista[clueSanc].Clients[clueParty][i] = 0;

						break;
					}
				}

				for (int i = 0; i < 13; ++i)
				{
					int memberId = pista[i];
					if (memberId <= 0 || memberId >= MAX_PLAYER)
						continue;

					SendClientMessage(memberId, "O líder do grupo da pista foi passado para %s", newLider->Mobs.Player.Name);
					Log(memberId, LOG_INGAME, "Líder passado para o usuário %s (%s)", newLider->Mobs.Player.Name, Users[newLeader].User.Username);
				}
			}		
		}
		else // Remove todo mundo do grupo
		{
			for (int i = 0; i < 12; i++)
			{
				int partyMob = player->PartyList[i];

				if (partyMob <= 0 || partyMob > MAX_SPAWN_MOB)
					continue;

				player->PartyList[i] = 0;

				CMob *Member = &Mob[partyMob];
				Member->Leader = 0;

				if (partyMob <= 0 || partyMob >= MAX_PLAYER)
					continue;

				if (Users[partyMob].Status == 22)
					SendRemoveParty(partyMob, 0);
			}

			return;
		}

		for (int iterator = 0; iterator < 12; iterator++)
		{
			int partyMob = player->PartyList[iterator];
			if (partyMob <= 0 || partyMob > MAX_SPAWN_MOB || partyMob == newLeader)
				continue;

			CMob *mob = &Mob[partyMob];
			mob->Leader = newLeader;

			SendRemoveParty(partyMob, clientId);
			SendAddParty(partyMob, player->PartyList[0], 1);
		}

		memset(player->PartyList, 0, sizeof INT16 * 12);
	}
}

//void RemoveParty(int clientId, int targetId)
//{
//	// Se targetId != 0 quer dizer que o líder tentou usar o sistema de kikar usuário
//	if(targetId != 0)
//	{
//		CMob *spw = &Mob[clientId];
//		// Checa se o usuário é líder
//		// Se for = 0, é possível retirar o usuário
//		if(spw->Leader != 0)
//		{
//			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);
//
//			return;
//		}
//	}
//
//	if(targetId == 0)
//		targetId = clientId;
//	
//	int leader = Mob[targetId].Leader;
//	if(leader == 0)
//		leader = targetId;
//
//	
//
//	// Retira quem foi retirado do líder do grupo
//	SendRemoveParty(leader, targetId);
//	SendRemoveParty(targetId, leader);
//
//	if(targetId > 0 && targetId < MAX_PLAYER)
//		SendRemoveParty(targetId, 0);
//
//	if(clientId > 0 && clientId < MAX_PLAYER)
//		SendRemoveParty(clientId, 0);
//
//	// Quem foi retirado fica sem liderença
//	Mob[targetId].Leader = 0;
//
//	int count = 0;
//	for (int i = 0; i < 12; i++)
//		if (Mob[leader].PartyList[i] != 0)
//			count++;
//
//	if(count >= 2)
//	{ // Grupo ainda há outras pessoas
//		if(leader == targetId)
//		{
//			int secondUser = 0;
//			int tmpSecondUser = 0;
//
//			for(int i = 0 ;i < 12; i ++)
//			{
//				int mobId = Mob[leader].PartyList[i];
//				if(mobId <= 0 || mobId >= MAX_SPAWN_MOB)
//					continue;
//
//				tmpSecondUser = mobId;
//				break;
//			}
//
//			for(int i = 0 ;i < 12; i++)
//			{ // Procura o segundo usuário possível do grupo
//				int memberId = Mob[leader].PartyList[i];
//				if(memberId <= 0 || memberId > MAX_PLAYER)
//					continue;
//
//				if(tmpSecondUser == memberId)
//					continue;
//
//				secondUser = tmpSecondUser;
//				break;
//			}
//			
//			// Caso não exista um segundo usuário (caso de evocações apenas no grupo), simplesmente o desfaz assim como a TMsrv faz
//			if(secondUser == 0)
//			{
//				Mob[leader].Leader = 0;
//
//				SendRemoveParty(leader, 0);
//				for(int i = 0; i < 12; i++)
//				{
//					int mobId = Mob[leader].PartyList[i];
//					if(mobId <= 0 || mobId >= MAX_SPAWN_MOB)
//						continue;
//
//					Mob[leader].PartyList[i] = 0;
//					Mob[mobId].Leader = 0;
//
//					if(mobId < MAX_PLAYER)
//						SendRemoveParty(mobId, 0);
//				}
//			}
//			else
//			{
//				// Seto o nvoo líder como 'líder de algum grupo'
//				Mob[secondUser].Leader = 0;
//
//				SendRemoveParty(secondUser, targetId);
//				SendAddParty(secondUser, secondUser, 1);
//
//				for(int x = 0; x < 12; x++)
//				{
//					int mobId = Mob[leader].PartyList[x];
//					if(mobId <= 0 || mobId >= MAX_SPAWN_MOB)
//						continue;
//
//					if(mobId == secondUser)
//					{
//						Mob[leader].PartyList[x] = 0;
//						continue;
//					}
//
//					Mob[mobId].Leader = secondUser;
//
//					SendRemoveParty(mobId, targetId);
//					SendAddParty(mobId, secondUser, 1);
//				}
//
//				memcpy(Mob[secondUser].PartyList, Mob[leader].PartyList, sizeof Mob[secondUser].PartyList);
//
//				memset(Mob[leader].PartyList, 0, sizeof Mob[leader].PartyList);
//			}
//		} 
//		else
//		{
//			SendRemoveParty(leader, targetId);
//
//			for(int i = 0 ; i < 12; i ++)
//			{
//				int mobId = Mob[leader].PartyList[i];
//				if(mobId <= 0 || mobId >= MAX_SPAWN_MOB)
//					continue;
//
//				if(mobId == targetId)
//					Mob[leader].PartyList[i] = 0;
//
//				if(mobId < MAX_PLAYER)
//					SendRemoveParty(mobId, targetId);
//			}
//		}
//	}
//	else
//	{ // Grupo será totalmente desfeito
//		Mob[leader].Leader = 0;
//
//		SendRemoveParty(leader, 0);
//
//		for(int i = 0; i < 12; i++)
//		{
//			int mobId = Mob[leader].PartyList[i];
//			if(mobId <= 0 || mobId >= MAX_SPAWN_MOB)
//				continue;
//
//			Mob[leader].PartyList[i] = 0;
//			Mob[mobId].Leader = 0;
//
//			if(mobId < MAX_PLAYER)
//				SendRemoveParty(mobId, 0);
//		}
//	}
//}

void RegenMob(int User)
{
	if (User <= 0 || User >= MAX_PLAYER)
		return;

	int GuildID = Mob[User].Mobs.Player.GuildIndex;

	Users[User].CharLoginTime++;

	if (!(Users[User].CharLoginTime % 450))
	{/*
		int Frag = GetPKPoint(User);
		if (Frag < 75)
		{
			Frag++;

			SetPKPoint(User, Frag);
			SendClientMessage(User, g_pLanguageString[_DD_PKPointPlus], Frag - 75, 1);
		}*/

		st_Item *MountBufferItem = &Mob[User].Mobs.Player.Equip[14];
		if (MountBufferItem->Index >= 2300 && MountBufferItem->Index < 2330)
		{
			int Delay = GetItemAbility(MountBufferItem, EF_INCUDELAY);
			if (Delay > 0)
			{
				Delay--;
				MountBufferItem->EFV3 = Delay;

				SendItem(User, SlotType::Equip, 14, &Mob[User].Mobs.Player.Equip[14]);

				SendClientMessage(User, g_pLanguageString[_NN_Incu_Proceed]);

				LogPlayer(User,  "Perdeu 1 incubação do ovo %s. Incubação atual: %d", ItemList[MountBufferItem->Index].Name, Delay);
				Log(User, LOG_INGAME, "Perdeu 1 incubação do ovo %s [%d]. Incubação atual: %d", ItemList[MountBufferItem->Index].Name, MountBufferItem->Index, Delay);
			}
		}

		if (MountBufferItem->Index >= 2330 && MountBufferItem->Index < 2390 && *(short*)&MountBufferItem->EF1 > 0)
		{
			int v44 = MountBufferItem->EF3;

			int v48 = (MountBufferItem->Index - 2330) % 30;

			if (v48 <= 15)
				v44 -= 2;
			else
				v44 -= 4;

			bool revi = false;
			for(INT32 i = 0; i < 32; i++)
			{
				if(Mob[User].Mobs.Affects[i].Index == 51)
				{
					revi = true;
					break;
				}
			}

			if(!revi)
			{
				if (v44 <= 1)
				{
					*(short*)&MountBufferItem->EF1 = 0;
					MountBufferItem->EF3 = 4;

					SendClientMessage(User, g_pLanguageString[_NN_Mount_died]);

					Log(User, LOG_INGAME, "Montaria %s %s morreu por falta de ração", ItemList[MountBufferItem->Index].Name, MountBufferItem->toString().c_str());
					LogPlayer(User, "Montaria %s morreu por falta de ração. Alimite a montaria constantemente", ItemList[MountBufferItem->Index].Name);

					if (MountBufferItem->Index >= 2360 && MountBufferItem->Index < 2390)
						ProcessAdultMount(User, 0);
					if (MountBufferItem->Index < 2360)
						MountProcess(User, nullptr);
					else
						Mob[User].GetCurrentScore(User);
				}
				else
					MountBufferItem->EF3 = v44;
			}

			SendItem(User, SlotType::Equip, 14, &Mob[User].Mobs.Player.Equip[14]);
		}
	}

	int Guilty = GetGuilty(User);
	if (Guilty > 0)
	{
		Guilty--;
		SetGuilty(User, Guilty);

		if (Guilty == 0)
		{
			p364 m;
			GetCreateMob(User, (BYTE*) &m);

			GridMulticast_2(Mob[User].Target.X, Mob[User].Target.Y, (BYTE*) &m, 0);

			for (int s = MAX_PLAYER; s < 30000; s++)
			{
				if (Mob[s].Mode != 5)
					continue;

				if (Mob[s].Mobs.Player.CapeInfo != 4)
					continue;

				if (Mob[s].CurrentTarget == User)
				{
					Mob[s].CurrentTarget = 0;
					Mob[s].Mode = 4;
				}

				for (int l = 0; l < 4; l++)
				{
					if (Mob[s].EnemyList[l] != User)
						continue;

					Mob[s].EnemyList[l] = 0;
					Mob[s].Mode = 4;
				}
			}
		}
	}

	int regenBase = 100;
	MapAttribute vD4 = GetAttribute(Mob[User].Target.X, Mob[User].Target.Y);

	if (vD4.Village)
		regenBase = 500;

	if (vD4.Guild)
		regenBase = 1000;

#pragma region 7556
	if (Mob[User].Mode && Mob[User].Mobs.Player.Status.curHP && User < MAX_PLAYER && Users[User].Status == USER_PLAY)
	{
		int currentHp = Mob[User].Mobs.Player.Status.curHP;
		int currentMp = Mob[User].Mobs.Player.Status.curMP;
		int maxHp = Mob[User].Mobs.Player.Status.maxHP;
		int maxMp = Mob[User].Mobs.Player.Status.maxMP;
		int LOCAL74 = currentHp;
		int regen = Mob[User].RegenHP;
		// LOCAL68 = vCC

		int newHp = (regenBase + regen) * 5;
		currentHp = currentHp + newHp;

		if (currentHp > maxHp)
			currentHp = maxHp;

		Mob[User].Mobs.Player.Status.curHP = currentHp;

		regen = Mob[User].RegenMP;
		
		int newMp = (regenBase + regen) * 5;
		currentMp = currentMp + newMp;

		if (currentMp > maxMp)
			currentMp = maxMp;

		Mob[User].Mobs.Player.Status.curMP = currentMp;

		p181 packet;
		packet.Header.PacketId = 0x181;
		packet.Header.ClientId = User;
		packet.Header.Size = sizeof p181;

		packet.curHP = currentHp;
		packet.curMP = currentMp;

		SetReqHp(User);
		SetReqMp(User);

		if (User > 0 && User < MAX_PLAYER)
		{
			packet.maxHP = Users[User].Potion.CountHp;
			packet.maxMP = Users[User].Potion.CountMp;
		}

		int TargetX = Mob[User].Target.X;
		int TargetY = Mob[User].Target.Y;

		GridMulticast_2(TargetX, TargetY, (BYTE*)&packet, 0); // maybe

		SendSetHpMp(User);
	}
#pragma endregion

	/*if (Mob[User].Mode && Mob[User].Mobs.Player.Status.curHP && User >= MAX_PLAYER || Users[User].Status == USER_PLAY)
	{
		int maxHP = Mob[User].Mobs.Player.Status.maxHP;
		int maxMP = Mob[User].Mobs.Player.Status.maxMP;
		int curHP = Mob[User].Mobs.Player.Status.curHP;
		int curMP = Mob[User].Mobs.Player.Status.curMP;

		int calcRegenHPMP = Mob[User].Mobs.Player.RegenHP;
		calcRegenHPMP = calcRegenHPMP * vCC / 100;

		int calcRegenHP = maxHP * calcRegenHPMP / 120 + vD0;
		curHP = curHP + calcRegenHP;

		if (curHP > maxHP)
			curHP = maxHP;

		Mob[User].Mobs.Player.Status.curHP = curHP;

		calcRegenHPMP = Mob[User].Mobs.Player.RegenMP;

		calcRegenHPMP = calcRegenHPMP * vCC / 100;

		int calcRegenMP = maxMP * calcRegenHPMP / 120 + vD0;

		curMP = curMP + calcRegenMP;

		if (curMP > maxMP)
			curMP = maxMP;

		Mob[User].Mobs.Player.Status.curMP = curMP;

		p181 packet;
		packet.Header.PacketId = 0x181;
		packet.Header.ClientId = User;
		packet.Header.Size = sizeof p181;

		packet.curHP = curHP;
		packet.curMP = curMP;

		SetReqHp(User);
		SetReqMp(User);

		packet.maxHP = Users[User].Potion.CountHp;
		packet.maxMP = Users[User].Potion.CountMp;

		int TargetX = Mob[User].Target.X;
		int TargetY = Mob[User].Target.Y;

		GridMulticast_2(TargetX, TargetY, (BYTE*) &packet, 0); // maybe

	}*/
}

int UpdateItem(int arg1, int arg2, int* arg3)
{ // ADDRBASE = 0x8B9E778
	INT32 LOCAL_1 = pInitItem[arg1].Rotation;

	INT32 LOCAL_2 = UpdateItem2(pInitItem[arg1].CanRun, pInitItem[arg1].Status, arg2, pInitItem[arg1].PosX, pInitItem[arg1].PosY, LOCAL_1, arg3);
	if(LOCAL_2 == 0)
		return 0;

	pInitItem[arg1].HeightGrid = *arg3;

	INT32 LOCAL_3 = pInitItem[arg1].CanRun;
	INT32 LOCAL_4 = pInitItem[arg1].Status;

	pInitItem[arg1].IsOpen = 0;

	if(LOCAL_3 >= 6 || LOCAL_3 < 0)
		return 1;

	if(LOCAL_4 != arg2)
	{
		INT32 LOCAL_5 = pInitItem[arg1].PosX;
		INT32 LOCAL_6 = pInitItem[arg1].PosY;

		for(INT32 LOCAL_7 = 0; LOCAL_7 <= 4; LOCAL_7 ++)
		{
			for(INT32 LOCAL_8 = 0; LOCAL_8 <= 4; LOCAL_8 ++)
			{// 0045F785
				INT32 LOCAL_9 = g_pGroundMask[LOCAL_3][LOCAL_1][LOCAL_7][LOCAL_8];
				UINT32 LOCAL_10 = LOCAL_5 + LOCAL_8 - 2;
				UINT32 LOCAL_11 = LOCAL_6 + LOCAL_7 - 2;

				if(LOCAL_10 < 1 || LOCAL_11 < 1 || LOCAL_10 > 4094 || LOCAL_11 > 4094)
					continue;

				if(LOCAL_9 == 0)
					continue;

				INT32 LOCAL_12 = g_pMobGrid[LOCAL_11][LOCAL_10];
				if(LOCAL_12 == 0)
					continue;

				if(Mob[LOCAL_12].Mobs.Player.Equip[0].Index == 220)
				{
					DeleteMob(LOCAL_12, 3);

					continue;
				}

				INT32 LOCAL_13 = GetEmptyMobGrid(LOCAL_12, &LOCAL_10, &LOCAL_11);
				if(LOCAL_13 != 0)
				{
					Mob[LOCAL_12].Route[0] = 0;

					p36C LOCAL_26;
					GetAction(LOCAL_12, LOCAL_10, LOCAL_11, &LOCAL_26);

					LOCAL_26.MoveSpeed = 20;
					LOCAL_26.MoveType = 0;

					GridMulticast_2(LOCAL_10, LOCAL_11, (BYTE*)&LOCAL_26, 0);

					if(LOCAL_12 < MAX_PLAYER)
						Users[LOCAL_12].AddMessage((BYTE*)&LOCAL_26, sizeof p36C);
				}
			}
		}
	}

	INT32 LOCAL_27 = GetItemAbility(&pInitItem[arg1].Item, EF_KEYID);

	//if(LOCAL_27 == 15 && LOCAL_4 == 1 && arg2 == 3)
	//	CreateMob("GATE", pInitItem[arg1].PosX, pInitItem[arg1].PosY, "npc");

	pInitItem[arg1].Status = arg2;
	return 1;
}

int UpdateItem2(int arg1,int arg2,int arg3,int arg4,int arg5,int arg7,int *arg8)
{
	*arg8 = 0;

	INT32 LOCAL_1 = 0;
	if(arg1 >= 6 || arg1 < 0)
		return 0;

	if(arg2 == 1 && (arg3 == 3 || arg3 == 2))
		LOCAL_1 = 1;

	if(arg3 == 1 && (arg2 == 3 || arg2 == 2))
		LOCAL_1 = -1;

	if(LOCAL_1 == 0)
		return 0;
	
	INT32 LOCAL_2 = 0;
	for( ; LOCAL_2 <= 5; LOCAL_2 ++)
	{
		INT32 LOCAL_3 = 0;
		for( ; LOCAL_3 <= 5; LOCAL_3++)
		{
			INT32 LOCAL_4 = g_pGroundMask[arg1][arg7][LOCAL_2][LOCAL_3];
			INT32 LOCAL_5 = LOCAL_3 + arg4 - 2;
			INT32 LOCAL_6 = LOCAL_2 + arg5 - 2;

			if(LOCAL_4 == 0)
				continue;

			LOCAL_4 = LOCAL_4 * LOCAL_1;

			// 0040BCCB
			if(LOCAL_5 - g_HeightPosX < 1)
				break;

			if(LOCAL_6 - g_HeightPosY < 1)
				break;

			if(LOCAL_5 - g_HeightPosX > g_HeightWidth - 2)
				break;

			if(LOCAL_5 - g_HeightPosY > g_HeightHeight - 2)
				break;

			INT32 LOCAL_7 = g_pHeightGrid[LOCAL_6 - g_HeightPosY][LOCAL_5 - g_HeightPosX] + LOCAL_4;
			if(LOCAL_7 > 255)
				LOCAL_7 = 255;

			if(LOCAL_7 < 0)
				LOCAL_7 = 0;

			if(g_pGroundMask[arg1][arg7][LOCAL_2][LOCAL_3] != 0)
				*arg8 = LOCAL_7;

			g_pHeightGrid[LOCAL_6 - g_HeightPosY][LOCAL_5 - g_HeightPosX] = LOCAL_7;
		}
	}

	return 1;
}

bool ReadMob(st_Mob *mob, const char *folder)
{
	FILE *pFile = NULL;
		
	char szTMP[1024];
	sprintf_s(szTMP, "%s/%s", folder, mob->Name);

	fopen_s(&pFile, szTMP, "rb");
	if(pFile) 
	{
		fread(mob, 1, sizeof st_Mob, pFile);		
		fclose(pFile);
		return true;
	}

	return false;
}

INT32 CreateMob(const char *mob, int posX, int posY, const char *folder)
{
	INT32 LOCAL_1 = mGener.GetEmptyNPCMob();
	if(LOCAL_1 == 0)
		return -1;

	Mob[LOCAL_1] = CMob{};
	Mob[LOCAL_1].BossInfoId = MAXUINT32;
	Mob[LOCAL_1].clientId = LOCAL_1;
	strncpy_s(Mob[LOCAL_1].Mobs.Player.Name, mob, 16);

	memset(&Mob[LOCAL_1].PartyList, 0, sizeof INT16 * 12);

	INT32 LOCAL_2 = ReadMob(&Mob[LOCAL_1].Mobs.Player, folder);
	if(LOCAL_2 == 0)
		return false;

	Mob[LOCAL_1].Mobs.Player.Name[15] = 0;

	INT32 LOCAL_3;
	for(LOCAL_3 = 0; LOCAL_3 < 16;LOCAL_3 ++)
	{
		if(Mob[LOCAL_1].Mobs.Player.Name[LOCAL_3] == '_')
			Mob[LOCAL_1].Mobs.Player.Name[LOCAL_3] = ' ';
		if (Mob[LOCAL_1].Mobs.Player.Name[LOCAL_3] == '@')
			Mob[LOCAL_1].Mobs.Player.Name[LOCAL_3] = ' ';
	}

	memset(&Mob[LOCAL_1].Mobs.Affects, 0, sizeof st_Affect * 32);

	for(LOCAL_3 = 0; LOCAL_3 < 5; LOCAL_3 ++)
	{
		if(Mob[LOCAL_1].Mobs.Player.Equip[0].Index == 220 || Mob[LOCAL_1].Mobs.Player.Equip[0].Index == 219 || Mob[LOCAL_1].Mobs.Player.Equip[0].Index == 358)
		{
			Mob[LOCAL_1].Segment.ListX[LOCAL_3] = posX;
			Mob[LOCAL_1].Segment.ListY[LOCAL_3] = posY;
		}
		else
		{
			Mob[LOCAL_1].Segment.ListX[LOCAL_3] = posX + (Rand() % 5) - 2;
			Mob[LOCAL_1].Segment.ListY[LOCAL_3] = posY + (Rand() % 5) - 2;
		}
	}

	Mob[LOCAL_1].GenerateID = -1;
	Mob[LOCAL_1].Formation = 0;
	Mob[LOCAL_1].RouteType = 0;
	Mob[LOCAL_1].Mode = 4;
	Mob[LOCAL_1].Segment.Progress = 0;
	Mob[LOCAL_1].Leader = 0;
	Mob[LOCAL_1].WaitSec = 10;

	Mob[LOCAL_1].clientId = LOCAL_1;

	Mob[LOCAL_1].GetCurrentScore(MAX_PLAYER);

	Mob[LOCAL_1].Mobs.Player.Status.curHP = Mob[LOCAL_1].Mobs.Player.Status.maxHP;
	Mob[LOCAL_1].Segment.Direction = 0;

	memset(&Mob[LOCAL_1].EnemyList, 0, 8);
	
	UINT32 LOCAL_5 = Mob[LOCAL_1].Segment.ListX[0];
	UINT32 LOCAL_6 = Mob[LOCAL_1].Segment.ListY[0];

	INT32 LOCAL_7 = GetEmptyMobGrid(LOCAL_1, &LOCAL_5, &LOCAL_6);
	if(LOCAL_7 == 0)
	{
		Mob[LOCAL_1].Mode = 0;
		Mob[LOCAL_1].Mobs.Player.Name[0] = 0;
		Mob[LOCAL_1].GenerateID = -1;

		return -2;
	}

	Mob[LOCAL_1].Last.Time = clock();

	Mob[LOCAL_1].Segment.X = LOCAL_5;
	Mob[LOCAL_1].Target.X = LOCAL_5;
	Mob[LOCAL_1].Last.X = LOCAL_5;

	Mob[LOCAL_1].Segment.Y = LOCAL_6;
	Mob[LOCAL_1].Target.Y = LOCAL_6;
	Mob[LOCAL_1].Last.Y = LOCAL_6;

	INT32 LOCAL_17 = Mob[LOCAL_1].Mobs.Player.bStatus.maxMP;
	if(LOCAL_17 != 0)
	{
		SetAffect(LOCAL_1, LOCAL_17, 30000, 200);
		SetTick(LOCAL_1, LOCAL_17, 30000, 200);
	}

	g_pMobGrid[LOCAL_6][LOCAL_5] = LOCAL_1;
	
	Mob[LOCAL_1].SpawnType = 2;
	SendGridMob(LOCAL_1);
	Mob[LOCAL_1].SpawnType = 0;

	return LOCAL_1;
} 

void DoRecall(int clientId)
{
	UINT32 LOCAL_1 = 0;
	UINT32 LOCAL_2 = 0;
	INT32 LOCAL_3 = Mob[clientId].Mobs.Player.Info.CityID;
	
	LOCAL_1 = g_pCityZone[LOCAL_3].city_x + (Rand() % 15);
	LOCAL_2 = g_pCityZone[LOCAL_3].city_y + (Rand() % 15);

	INT32 LOCAL_4 = Mob[clientId].Mobs.Player.GuildIndex;
	if(LOCAL_4 > 0)
	{
		for(INT32 LOCAL_5 = 0; LOCAL_5 < 5; LOCAL_5++)
		{
			if(ChargedGuildList[sServer.Channel - 1][LOCAL_5] == LOCAL_4)
			{
				LOCAL_1 = g_pCityZone[LOCAL_5].area_guild_x;
				LOCAL_2 = g_pCityZone[LOCAL_5].area_guild_y;

				break;
			}
		}
	}

	// RVR
	if (Mob[clientId].Target.X >= 1041 && Mob[clientId].Target.X <= 1248 &&
		Mob[clientId].Target.Y >= 1950 && Mob[clientId].Target.Y <= 2158 && sServer.RvR.Status == 1)
	{
		int cape = Mob[clientId].Mobs.Player.CapeInfo;

		if (cape == CAPE_BLUE)
		{
			if (!(Rand() % 2))
			{
				LOCAL_1 = 1061 - Rand() % 5;
				LOCAL_2 = 2113 + Rand() % 5;
			}
			else
			{
				LOCAL_1 = 1091 - Rand() % 5;
				LOCAL_2 = 2140 + Rand() % 5;
			}
		}
		else
		{
			if (!(Rand() % 2))
			{
				LOCAL_1 = 1238 + Rand() % 5;
				LOCAL_2 = 1983 + Rand() % 5;
			}
			else
			{
				LOCAL_1 = 1211 + Rand() % 5;
				LOCAL_2 = 1955 + Rand() % 5;
			}
		}
	}

	bool LOCAL_6 = GetEmptyMobGrid(clientId, &LOCAL_1, &LOCAL_2);
	if (LOCAL_6 == false)
	{
		if (clientId < MAX_PLAYER)
			Log(clientId, LOG_INGAME, "DoRecall - Falha ao encontrar espaço vago no mapa para o usuário. Posição: %ux %uy", LOCAL_1, LOCAL_2);

		return;
	}

	p36C LOCAL_19;
	memset(&LOCAL_19, 0, sizeof p36C);

	GetAction(clientId, LOCAL_1, LOCAL_2, &LOCAL_19);

	LOCAL_19.MoveType = 1;
	if (clientId < MAX_PLAYER)
		Users[clientId].AddMessage((BYTE*)&LOCAL_19, sizeof p36C);

	GridMulticast(clientId, LOCAL_1, LOCAL_2, (BYTE*)&LOCAL_19);

	Log(clientId, LOG_INGAME, "DoRecall - Enviado para cidade: %ux %uy", LOCAL_1, LOCAL_2);
}

void BASE_InitializeHitRate()
{
	memset(g_pHitRate, 0, 4096);

	INT32 LOCAL_1 = 512;
	INT32 LOCAL_2 = 0;
	INT32 LOCAL_3 = 0;

	while(true)
	{
		INT32 LOCAL_4 = 0;

		for(;LOCAL_4 < 1024; LOCAL_4 ++)
		{
			if(g_pHitRate[LOCAL_4] != 0)
				continue;

			if(LOCAL_3 == 0)
				g_pHitRate[LOCAL_4] = LOCAL_2;
			else if(LOCAL_3 == 1)
				g_pHitRate[LOCAL_4 ] = 512 - LOCAL_2;
			else if(LOCAL_3 == 2)
				g_pHitRate[LOCAL_4] = LOCAL_2 + 512;
			else
				g_pHitRate[LOCAL_4] = 1024 - LOCAL_2;

			if(g_pHitRate[LOCAL_4] > 999)
				g_pHitRate[LOCAL_4] = 999;

			LOCAL_3 ++;
			if(LOCAL_3 >= 4)
				LOCAL_3 = 0;
			if(LOCAL_3 == 0)
				LOCAL_2++;
		}

		LOCAL_1 /= 2;

		if(LOCAL_1 == 0)
			break;
	}
	
	g_pHitRate[0] = 512;
}

void ProcessAdultMount(int clientId, int damage)
{
	st_Item *LOCAL_1 = &Mob[clientId].Mobs.Player.Equip[14];

	if(LOCAL_1->Index < 2360 || LOCAL_1->Index >= 2390)
		return;

	// Checa se possui a porra do Poção Revigorante xD
	for(INT32 i = 0 ; i < 32 ;i ++ )
	{
		if(Mob[clientId].Mobs.Affects[i].Index == 51)
			return;
	}

	INT32 LOCAL_2 = LOCAL_1->Index - 2360;
	INT32 LOCAL_3 = NPCBase[LOCAL_2 + 10].Status.maxHP;
	INT32 LOCAL_4 = LOCAL_1->Effect[2].Index;
	
	if(LOCAL_4 <= 0)
	{
		if(LOCAL_1->Effect[1].Index)
		{
			*(short*)&LOCAL_1->Effect[0].Index = 0;
			LOCAL_4 = 0;
		}
	}

	INT32 LOCAL_5 = *(short*)&LOCAL_1->Effect[0].Index;
	INT32 LOCAL_6 = *(short*)&LOCAL_1->Effect[0].Index - damage; // EBP - 18h

	if(LOCAL_6 >= LOCAL_3)
		LOCAL_6 = LOCAL_3;

	INT32 LOCAL_7 = LOCAL_6;

	*(short*)&LOCAL_1->Effect[0].Index = LOCAL_7;

	if(LOCAL_6 <= 0)
		LOCAL_1->Effect[2].Index = 0;

	// Pet morreu
	if(LOCAL_5 > 0 && LOCAL_6 <= 0)
	{
		SendEquip(clientId); // 401069

		Log(clientId, LOG_INGAME, "Pet %s [%d] [%d %d %d %d %d %d] morreu. Hit: %d", ItemList[LOCAL_1->Index].Name,
			LOCAL_1->Index, LOCAL_1->Effect[0].Index, LOCAL_1->Effect[0].Value, LOCAL_1->Effect[1].Index, LOCAL_1->Effect[1].Value, LOCAL_1->Effect[2].Index, LOCAL_1->Effect[2].Value,
			damage);

		LogPlayer(clientId, "Pet %s morreu levando um hit de %d. HP antes do hit: %d", ItemList[LOCAL_1->Index].Name, damage, LOCAL_5);
	}

	if(LOCAL_5 != LOCAL_6 && clientId < MAX_PLAYER)
		SendItem(clientId, SlotType::Equip, 14, LOCAL_1); 
}

void LinkMountHp(int arg1)
{
	if(arg1 < MAX_PLAYER || arg1 >= 30000)
		return;

	if(Mob[arg1].Mobs.Player.CapeInfo != 4)
		return;

	INT32 LOCAL_1 = Mob[arg1].Mobs.Player.Equip[0].Index;
	if(LOCAL_1 < 315 || LOCAL_1 >= 345)
		return;

	INT32 LOCAL_2 = Mob[arg1].Summoner;
	if(Mob[LOCAL_2].Mode == 0 || Users[LOCAL_2].Status != USER_PLAY)
		return;

	INT16 LOCAL_3 = Mob[LOCAL_2].Mobs.Player.Equip[14].Index - 2330;
	INT16 LOCAL_4 = LOCAL_1 - 315;

	if(LOCAL_3 != LOCAL_4)
		return;

	INT16 LOCAL_5 = *(short*)&Mob[LOCAL_2].Mobs.Player.Equip[14].Effect[0].Index;
	INT16 LOCAL_6 = Mob[arg1].Mobs.Player.Status.curHP;
	if(LOCAL_5 != LOCAL_6)
	{
		*(short*)&Mob[LOCAL_2].Mobs.Player.Equip[14].Effect[0].Index = LOCAL_6;

		SendItem(LOCAL_2, SlotType::Equip, 14, &Mob[LOCAL_2].Mobs.Player.Equip[14]);
	}
}

float TimeRemaining(int dia, int mes, int ano)
{
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	int month  = now.tm_mon; //0 Janeiro, 1 Fev
	int day    = now.tm_mday;
	int year   = now.tm_year;

	struct std::tm a = {0,0,0, day, month, year};
	struct std::tm b = {0,0,0, dia, mes - 1, ano-1900};

	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	if ( x != (std::time_t)(-1) && y != (std::time_t)(-1) )
	{
		double difference = (std::difftime(y, x) / (60 * 60 * 24));
		return static_cast<float>(difference);
	}

	return 0;
}

void BASE_GetFirstKey(const char * source, char * dest)
{
	if ((source[0] >= 'A' && source[0] <= 'Z') || (source[0] >= 'a' && source[0] <= 'z'))
	{
		dest[0] = source[0];
		dest[1] = 0;

		return;
	}

	strcpy_s(dest, 4, "etc");
}

INT32 GetFirstSlot(int clientId, int itemId)
{
	st_Item *item = Mob[clientId].Mobs.Player.Inventory;
	for(int i = 0; i < 30; i++)
	{
		if(item[i].Index == itemId)
			return i;
	}

	if(item[60].Index == 3467)
	{
		float remainig = TimeRemaining(item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
		if(remainig > 0.0f)
		{
			for(int i = 30; i < 45; i++)
			{
				if(item[i].Index == itemId)
					return i;
			}
		}
		else
		{
			Log(clientId, LOG_INGAME, "Bolsa do Andarilho acabou. Slot 60. %02d/%02d/%04d", item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
			LogPlayer(clientId, "Bolsa do andarilho acabou. Data de finalização: %02d/%02d/%04d", item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);

			memset(&item[60], 0, sizeof st_Item);
			
			// Atualiza o inventário
			SendItem(clientId, SlotType::Inv, 60, &item[60]);
		}
	}
	
	if(item[61].Index == 3467)
	{
		float remainig = TimeRemaining(item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
		if(remainig > 0.0f)
		{
			for(int i = 45; i < 60; i++)
			{
				if(item[i].Index == itemId)
					return i;
			}
		}
		else
		{
			Log(clientId, LOG_INGAME, "Bolsa do Andarilho acabou. Slot 61. %02d/%02d/%04d", item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
			LogPlayer(clientId, "Bolsa do andarilho acabou. Data de finalização: %02d/%02d/%04d", item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
			memset(&item[61], 0, sizeof st_Item);
			
			// Atualiza o inventário
			SendItem(clientId, SlotType::Inv, 61, &item[61]);
		}
	}

	return -1;
}

void RemoveTrade(int clientId)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	memset(&Users[clientId].Trade, 0, sizeof (p383));

	for(int i = 0; i < 15; i++)
		Users[clientId].Trade.Slot[i] = -1;

	memset(&Users[clientId].AutoTrade, 0, sizeof (Users[0].AutoTrade));

	for(int i = 0; i < 12; i++)
		Users[clientId].AutoTrade.Slots[i] = -1;

	if(Users[clientId].Status != USER_PLAY)
		return;

	SendSignal(clientId, clientId, 0x384);

	if(Users[clientId].IsAutoTrading)
	{
		int TargetX = Mob[clientId].Target.X;
		int TargetY = Mob[clientId].Target.Y;

		p364 sm;
		GetCreateMob(clientId,(BYTE*)&sm);

		GridMulticast_2(TargetX, TargetY, (BYTE*)&sm,0);

		Users[clientId].IsAutoTrading = 0;
	}
}

std::vector<int> GetMobInArea(int minPosX, int minPosY, int maxPosX, int maxPosY)
{
	auto Mobs = std::vector<int>();

	for (int i = 1000; i < 25000; i++)
	{
		auto wMob = &Mob[i];

		if (Mob[i].Target.X > static_cast<int>(minPosX) && Mob[i].Target.X < static_cast<int>(maxPosX) && Mob[i].Target.Y > static_cast<int>(minPosY) && Mob[i].Target.Y < static_cast<int>(maxPosY))
		{
			if (wMob->Mode == 4 || wMob->Mode == 5)
			{
				Mobs.push_back(static_cast<uint16_t>(i));
			}
		}

	}

	return Mobs;
}

bool ReadStoreNew()
{
	FILE* fp;

	fp = fopen("Donate.txt", "rb");

	if (fp == NULL)
	{
		MessageBoxA(0, "Missing Donate.txt file", "Error", MB_ICONERROR | MB_OK);
		fclose(fp);
		return false;
	}

	int QuizLineCounter = -1;

	ControlDonateLoja.clear();


	while (true)
	{
		char Buffer[1024];

		char* ptr = fgets((char*)Buffer, 1024, fp);

		if (ptr == NULL) break;

		for (int o = 0; o < 1024; o++)
		{
			if (ptr[o] == ',') { ptr[o] = ' '; }
			if (ptr[o] == '[') { break; }
		}

		if (ptr[0] == '#') continue;

		bool FlagTemp = false;

		int type,
			page,
			item,
			add1,
			eff1,
			add2,
			eff2,
			add3,
			eff3,
			price,
			stock,
			slot;


		if (sscanf(Buffer, "%d %d %d %d %d %d %d %d %d %d %d %d", &type, &page, &item, &add1, &eff1, &add2, &eff2, &add3, &eff3, &price, &stock, &slot))
		{
			st_Item Item;
			memset(&Item, 0, sizeof(st_Item));

			Item.Index = item;
			Item.EF1 = add1;
			Item.EFV1 = eff1;
			Item.EF2 = add2;
			Item.EFV2 = eff2;
			Item.EF3 = add3;
			Item.EFV3 = eff3;

			auto Temp = LojaDonate(
				type,
				page,
				Item,
				price,
				stock,
				slot
			);

			ControlDonateLoja.push_back(Temp);
			QuizLineCounter++;
		}
		else
		{
			MessageBoxA(0, "Missing data in quiz", "Error", MB_ICONERROR | MB_OK);
			fclose(fp);
			return false;
		}
	}

	fclose(fp);
	return true;
}

void SendDropList(int conn)
{
	if (conn <= 0 || conn >= MAX_PLAYER)
		return;

	Users[conn].LastTimeRequestDrop = CurrentTime;

	auto player = &Mob[conn];

	if (player == NULL)
		return;

	p2568 Packet;
	memset(&Packet, 0, sizeof(Packet));

	auto wMobs = GetMobInArea(Mob[conn].Target.X - 50, Mob[conn].Target.Y - 50, Mob[conn].Target.X + 50, Mob[conn].Target.Y + 50);
	if (wMobs.size() > 0)
	{
		for (auto& i : wMobs)
		{
			auto mob = &Mob[i];

			if (mob->Mobs.Player.Info.Merchant != 0)
				continue;


			if (Packet.amount == 10)
				break;

			auto exists = false;

			for (int x = 0; x < Packet.amount; x++)
			{
				if (!strcmp(Packet.Drop[x].name, mob->Mobs.Player.Name))
				{
					exists = true;
					break;
				}
			}
			if (exists)
				continue;

			strcpy_s(Packet.Drop[Packet.amount].name, mob->Mobs.Player.Name);

			Packet.Drop[Packet.amount].X = Mob[conn].Target.X;
			Packet.Drop[Packet.amount].Y = Mob[conn].Target.Y;

			Packet.Drop[Packet.amount].exp = (int)mob->Mobs.Player.Exp;
			Packet.Drop[Packet.amount].gold = mob->Mobs.Player.Gold;

			for (int a = 0; a < 64; a++)
				memcpy(&Packet.Drop[Packet.amount].carry[a], &mob->Mobs.Player.Inventory[a], sizeof(st_Item));

			Packet.amount++;
		}
	}

	Packet.Header.Size = sizeof(p2568);
	Packet.Header.PacketId = 2568;
	Packet.Header.ClientId = conn;

	Users[conn].AddMessage((BYTE*)&Packet, sizeof p2568);
	Users[conn].SendMessageA();
}

void ProcessDecayItem()
{
	for(INT32 LOCAL_1 = 0; LOCAL_1 < 8; LOCAL_1 ++)
	{
		sServer.ItemCount ++;

		if(sServer.ItemCount >= 4096)
			sServer.ItemCount = (sServer.InitCount + 1);

		if(pInitItem[sServer.ItemCount].Item.Index == 1727 || (pInitItem[sServer.ItemCount].Item.Index >= 3145 && pInitItem[sServer.ItemCount].Item.Index <= 3149) || (pInitItem[sServer.ItemCount].Item.Index >= 794 && pInitItem[sServer.ItemCount].Item.Index <= 798))
			continue;

		if(pInitItem[sServer.ItemCount].Item.Index == 4922 || pInitItem[sServer.ItemCount].Item.Index == 4923 || pInitItem[sServer.ItemCount].Item.Index == 4143 || pInitItem[sServer.ItemCount].Item.Index == 471)
			continue;

		if(pInitItem[sServer.ItemCount].Open == 0)
			continue;

		if(pInitItem[sServer.ItemCount].Unknow_36 == 0)
		{
			pInitItem[sServer.ItemCount].Unknow_36 = 1;

			continue;
		}
			
		INT32 LOCAL_2 = pInitItem[sServer.ItemCount].PosX;
		INT32 LOCAL_3 = pInitItem[sServer.ItemCount].PosY;

		memset(&pInitItem[sServer.ItemCount].Item, 0, sizeof st_Item);

		g_pItemGrid[LOCAL_3][LOCAL_2] = 0;

		pInitItem[sServer.ItemCount].Open = 0;

		p16F packet;
		packet.Header.PacketId = 0x16F;
		packet.Header.Size = sizeof p16F;
		packet.Header.ClientId = 0x7530;

		packet.initID = sServer.ItemCount + 10000;

		GridMulticast_2(LOCAL_2, LOCAL_3, (BYTE*)&packet, 0);
	}
}

void SummonGuild(int arg1, int arg2, int arg3, int arg4, int arg5)
{
	INT32 LOCAL_1 = 0;
	if(arg1 <= 0)
		return;

	for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
	{
		if(Users[LOCAL_2].Status != USER_PLAY)
			continue;

		if(Mob[LOCAL_2].Mode == 0)
			continue;

		if(Mob[LOCAL_2].Mobs.Player.GuildIndex != arg1)
			continue;
		
		INT32 LOCAL_3 = GetVillage(Mob[LOCAL_2].Target.X, Mob[LOCAL_2].Target.Y);
		if(LOCAL_3 != arg5)
			continue;

		if(arg5 == 2 && Mob[LOCAL_2].Mobs.Player.bStatus.Level > 100000)
		{
			SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);

			continue;
		}

		Teleportar(LOCAL_2, arg2, arg3);

		LOCAL_1 ++;
		if(LOCAL_1 == 30)
		{
			arg2 ++;
			arg3 ++;
		}

		if(LOCAL_1 == 45)
		{
			arg2 -= 2;
			arg3 -= 2;
		}

		if(LOCAL_1 >= arg4)
			break;
	}
}

void SummonGuild(int arg1, int arg2, int arg3, int arg4)
{
	INT32 LOCAL_1 = 0;
	if(arg1 <= 0)
		return;

	for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
	{
		if(Users[LOCAL_2].Status != USER_PLAY)
			continue;

		if(Mob[LOCAL_2].Mode == 0)
			continue;

		if(Mob[LOCAL_2].Mobs.Player.GuildIndex != arg1)
			continue;
		
		Teleportar(LOCAL_2, arg2, arg3);

		LOCAL_1 ++;
		if(LOCAL_1 == 30)
		{
			arg2 ++;
			arg3 ++;
		}

		if(LOCAL_1 == 45)
		{
			arg2 -= 2;
			arg3 -= 2;
		}

		if(LOCAL_1 >= arg4)
			break;
	}
}

void DoRanking(int arg1, int arg2, int arg3)
{
	if(arg1 == 0)
	{
		INT32 LOCAL_1 = 121;

		if(arg2 <= 0 || arg2 >= MAX_PLAYER)
			return;

		if(arg3 <= 0 || arg3 >= MAX_PLAYER)
			return;
		
		strncpy_s(sServer.Challanger.RankingName[0], 32, Mob[arg2].Mobs.Player.Name, 16);
		strncpy_s(sServer.Challanger.RankingName[1], 32, Mob[arg3].Mobs.Player.Name, 16);
		strncpy_s(sServer.Challanger.RankingName[2], 32, Mob[arg2].Mobs.Player.Name, 16);
		strncpy_s(sServer.Challanger.RankingName[3], 32, Mob[arg3].Mobs.Player.Name, 16);
		
		sServer.Challanger.RankingLevel[0] = Mob[arg2].Mobs.Player.Status.Level;
		sServer.Challanger.RankingLevel[1] = Mob[arg3].Mobs.Player.Status.Level;

		// 004617DA
		Teleportar(arg2, 147, 4045);
		SendClientMessage(arg2, g_pLanguageString[_NN_Battle_Started]);

		Teleportar(arg3, 189, 4045);
		SendClientMessage(arg3, g_pLanguageString[_NN_Battle_Started]);
		
		SendSignalParm(arg2, 0x7530, 0x3A1, LOCAL_1 + LOCAL_1 - 2);
		SendSignalParm(arg3, 0x7530, 0x3A1, LOCAL_1 + LOCAL_1 - 2);

		sServer.Challanger.RankingProcess = arg1 + 1;
		sServer.Challanger.Challanger1 = arg2;
		sServer.Challanger.Challanger2 = arg3;
		sServer.Challanger.Timer = LOCAL_1;

		return;
	}

	INT32 LOCAL_2 = 301;
	INT32 LOCAL_3 = Mob[arg2].Mobs.Player.GuildIndex;
	INT32 LOCAL_4 = Mob[arg3].Mobs.Player.GuildIndex;

	if(LOCAL_3 <= 0 || LOCAL_4 <= 0)
		return;
		
	sServer.Challanger.RankingProcess = arg1 + 1;
	sServer.Challanger.Challanger1 = LOCAL_3;
	sServer.Challanger.Challanger2 = LOCAL_4;
	sServer.Challanger.Timer = LOCAL_2;
	
	strncpy_s(sServer.Challanger.RankingName[0], 32, Mob[arg2].Mobs.Player.Name, 16);
	strncpy_s(sServer.Challanger.RankingName[1], 32, Mob[arg3].Mobs.Player.Name, 16);
	strncpy_s(sServer.Challanger.RankingName[2], 32, Mob[arg2].Mobs.Player.Name, 16);
	strncpy_s(sServer.Challanger.RankingName[3], 32, Mob[arg3].Mobs.Player.Name, 16);

	sServer.Challanger.RankingLevel[0] = Mob[arg2].Mobs.Player.Status.Level;
	sServer.Challanger.RankingLevel[1] = Mob[arg3].Mobs.Player.Status.Level;

	INT32 LOCAL_5 = 5;
	INT32 LOCAL_6 = 5;

	if(arg1 == 2)
	{
		LOCAL_5 = 10;
		LOCAL_6 = 10;
	}
	else if(arg1 == 3)
	{
		LOCAL_5 = MAX_PLAYER;
		LOCAL_6 = MAX_PLAYER;
	}

	if(arg1 == 3)
	{
		SummonGuild(LOCAL_3, 147, 4045, MAX_PLAYER, 3);
		SummonGuild(LOCAL_4, 189, 4045, MAX_PLAYER, 3);

		return;
	}

	for(INT32 LOCAL_7 = 1688; LOCAL_7 <= 1714; LOCAL_7++)
	{
		for(INT32 LOCAL_8 = 2564; LOCAL_8 <= 2579; LOCAL_8++)
		{
			INT32 LOCAL_9 = g_pMobGrid[LOCAL_7][LOCAL_8];
			if(LOCAL_9 <= 0 || LOCAL_9 >= MAX_PLAYER)
				continue;

			if(LOCAL_5 > 0 && Mob[LOCAL_9].Mobs.Player.GuildIndex == LOCAL_3)
			{
				LOCAL_5 --;

				Teleportar(LOCAL_9, 147, 4045);
				SendClientMessage(LOCAL_9, g_pLanguageString[_NN_Battle_Started]);

				SendSignalParm(LOCAL_9, 0x7530, 0x3A1, (LOCAL_2 + LOCAL_2 - 2));
			}

			if(LOCAL_6 > 0 &&  Mob[LOCAL_9].Mobs.Player.GuildIndex == LOCAL_4)
			{
				LOCAL_6 --;

				Teleportar(LOCAL_9, 189, 4045);
				SendClientMessage(LOCAL_9, g_pLanguageString[_NN_Battle_Started]);

				SendSignalParm(LOCAL_9, 0x7530, 0x3A1, (LOCAL_2 + LOCAL_2 - 2));
			}

			if(LOCAL_5 <= 0 && LOCAL_6 <= 0)
				break;
		}
	}
}

void ProcessRanking()
{
	if(sServer.Challanger.Timer > 0)
	{
		sServer.Challanger.Timer --;

		if(!(sServer.Challanger.Timer % 5))
		{
			INT32 LOCAL_1 = 0,
				  LOCAL_2 = 0;

			for(INT32 LOCAL_3 = 1; LOCAL_3 < MAX_PLAYER; LOCAL_3++)
			{
				if(Users[LOCAL_3].Status != USER_PLAY)
					continue;

				if(Mob[LOCAL_3].Mobs.Player.Status.curHP <= 0)
					continue;

				INT32 LOCAL_4 = Mob[LOCAL_3].Target.X;
				INT32 LOCAL_5 = Mob[LOCAL_3].Target.Y;
				
				if(LOCAL_4 < 142 || LOCAL_4 > 195 || LOCAL_5 < 4007 || LOCAL_5 > 4082)
					continue;

				if(sServer.Challanger.RankingProcess == 1)
				{
					if(LOCAL_3 == sServer.Challanger.Challanger1)
						LOCAL_1++;

					if(LOCAL_3 == sServer.Challanger.Challanger2)
						LOCAL_2 ++;
				}
				else
				{
					if(Mob[LOCAL_3].Mobs.Player.GuildIndex == sServer.Challanger.Challanger1)
						LOCAL_1 ++;

					if(Mob[LOCAL_3].Mobs.Player.GuildIndex == sServer.Challanger.Challanger2)
						LOCAL_2 ++;
				}
			}

			if(LOCAL_1 == 0 || LOCAL_2 == 0)
				sServer.Challanger.Timer = 0;
		}

		if(sServer.Challanger.Timer < 60)
		{
			SendDamage(0x8E, 0xFA7, 0xC3, 0xFCA);
			SendDamage(0x8E, 0xFCE, 0xC3, 0xFF2);

			SendEnvEffect(0x8E, 0xFA7, 0xA8, 0xFB2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFE7, 0xA8, 0xFF2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFB3, 0xA8, 0xFBE, 0x20, 0);
			SendEnvEffect(0x8E, 0xFDB, 0xA8, 0xFE6, 0x20, 0);
			SendEnvEffect(0x8E, 0xFBF, 0xA8, 0xFCA, 0x20, 0);
			SendEnvEffect(0x8E, 0xFCF, 0xA8, 0xFDA, 0x20, 0);
			SendEnvEffect(0xA8, 0xFA7, 0xC3, 0xFB2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFE7, 0xC3, 0xFF2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFB3, 0xC3, 0xFBE, 0x20, 0);
			SendEnvEffect(0xA8, 0xFDB, 0xC3, 0xFE6, 0x20, 0);
			SendEnvEffect(0xA8, 0xFBF, 0xC3, 0xFCA, 0x20, 0);
			SendEnvEffect(0xA8, 0xFCF, 0xC3, 0xFDA, 0x20, 0);
		}
		else if(sServer.Challanger.Timer < 120)
		{
			SendDamage(0x8E, 0xFA7, 0xC3, 0xFC2);
			SendDamage(0x8E, 0xFD7, 0xC3, 0xFF2);

			SendEnvEffect(0x8E, 0xFA7, 0xA8, 0xFB2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFE7, 0xA8, 0xFF2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFB3, 0xA8, 0xFBE, 0x20, 0);
			SendEnvEffect(0x8E, 0xFDB, 0xA8, 0xFE6, 0x20, 0);
			SendEnvEffect(0xA8, 0xFA7, 0xC3, 0xFB2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFE7, 0xC3, 0xFF2, 0x20, 0);
			SendEnvEffect(0xA8, 0xFB3, 0xC3, 0xFBE, 0x20, 0);
			SendEnvEffect(0xA8, 0xFDB, 0xC3, 0xFE6, 0x20, 0);
		}
		else if(sServer.Challanger.Timer < 180)
		{
			SendDamage(0x8E, 0xFA7, 0xC3, 0xFB3);
			SendDamage(0x8E, 0xFE6, 0xCE, 0xFF2);

			SendEnvEffect(0x8E, 0xFA7, 0xA8, 0xFB2, 0x20, 0);
			SendEnvEffect(0x8E, 0xFE7, 0xA8, 0xFF2, 0x20, 0);
			SendEnvEffect(0xA9, 0xFA7, 0xC3, 0xFB2, 0x20, 0);
			SendEnvEffect(0xA9, 0xFE7, 0xC3, 0xFF2, 0x20, 0);
		}
		
		if(sServer.Challanger.Timer <= 0)
		{
			INT32 LOCAL_6 = 0,
				  LOCAL_7 = 0;

			for(INT32 LOCAL_8 = 1; LOCAL_8 < MAX_PLAYER; LOCAL_8 ++)
			{
				if(Users[LOCAL_8].Status != USER_PLAY)
					continue;;

				if(Mob[LOCAL_8].Mobs.Player.Status.curHP <= 0)
					continue;
				
				INT32 LOCAL_9 = Mob[LOCAL_8].Target.X;
				INT32 LOCAL_10 = Mob[LOCAL_8].Target.Y;
				
				if(LOCAL_9 < 142 || LOCAL_9 > 195 || LOCAL_10 < 4007 || LOCAL_10 > 4082)
					continue;

				if(sServer.Challanger.RankingProcess == 1)
				{
					if(LOCAL_8 == sServer.Challanger.Challanger1)
						LOCAL_6++;

					if(LOCAL_8 == sServer.Challanger.Challanger2)
						LOCAL_7 ++;
				}
				else
				{
					if(Mob[LOCAL_8].Mobs.Player.GuildIndex == sServer.Challanger.Challanger1)
						LOCAL_6 ++;

					if(Mob[LOCAL_8].Mobs.Player.GuildIndex == sServer.Challanger.Challanger2)
						LOCAL_7 ++;
				}
			}
			
			sServer.Challanger.RankingName[0][15] = 0;
			sServer.Challanger.RankingName[0][14] = 0;
			sServer.Challanger.RankingName[1][15] = 0;
			sServer.Challanger.RankingName[1][14] = 0;
			sServer.Challanger.RankingName[2][15] = 0;
			sServer.Challanger.RankingName[2][14] = 0;
			sServer.Challanger.RankingName[3][15] = 0;
			sServer.Challanger.RankingName[3][14] = 0;

			char szTMP[128];
			if(LOCAL_6 == LOCAL_7)
			{
				sprintf_s(szTMP, g_pLanguageString[_SS_S_S_Draw], sServer.Challanger.RankingName[0], sServer.Challanger.RankingName[1]);
				
				SendNoticeArea(szTMP, 0xA04, 0x698, 0xA13, 0x6B2);
				SendNoticeArea(szTMP, 0x8E, 0xFA7, 0xC3, 0xFF2);
			}
			else
			{
				if(LOCAL_6 > LOCAL_7)
					sprintf_s(szTMP, g_pLanguageString[_SS_S_WinBy_S],sServer.Challanger.RankingName[0], sServer.Challanger.RankingName[1]);
				else	
					sprintf_s(szTMP, g_pLanguageString[_SS_S_WinBy_S],sServer.Challanger.RankingName[1], sServer.Challanger.RankingName[0]);

				SendNoticeArea(szTMP, 0xA04, 0x698, 0xA13, 0x6B2);
				SendNoticeArea(szTMP, 0x8E, 0xFA7, 0xC3, 0xFF2);
			}

			sServer.Challanger.RankingProcess = 0;
			sServer.Challanger.Timer = 0;
			sServer.Challanger.Challanger1 = 0;
			sServer.Challanger.Challanger2 = 0;
			sServer.Challanger.RankingName[0][0] = 0;
			sServer.Challanger.RankingName[1][0] = 0;

			ClearAreaTeleport(0x8E, 0xFA7, 0xC3, 0xFF2, 0xA0C, 0x6D8);
		}
	}
}

void SetItemAmount(st_Item *item, int amount)
{
	int i = 0;
	for(; i < 3; i++)
	{
		if(item->Effect[i].Index == EF_AMOUNT)
		{
			item->Effect[i].Value = amount;
			break;
		}
	}

	if(i == 3)
	{
		for(i = 0; i < 3; i++)
		{
			if(item->Effect[i].Index == 0)
			{
				item->Effect[i].Index = EF_AMOUNT;
				item->Effect[i].Value = amount;

				break;
			}
		}
	}
}
int CombineJeffi(int clientId, int searched, int earned)
{
	if (clientId < 0 || clientId >= MAX_PLAYER)
		return -1;

	int invAmount = GetInventoryAmount(clientId, searched);

	if (invAmount < 10)
		return -1;

	float bolsa1 = TimeRemaining(Mob[clientId].Mobs.Player.Inventory[60].EFV1, Mob[clientId].Mobs.Player.Inventory[60].EFV2, Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900);
	float bolsa2 = TimeRemaining(Mob[clientId].Mobs.Player.Inventory[61].EFV1, Mob[clientId].Mobs.Player.Inventory[61].EFV2, Mob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900);
	
	int totalPacks = 0;
	int aux = invAmount / 10; // pra cada PO/PL se usa 10 restos
	return 0;
}

void Combine(int clientId, int searched, int earned)
{
	INT32 LOCAL_1 = 0;
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	INT32 LOCAL_2 = 0;
	
	LOCAL_1 = GetInventoryAmount(clientId, searched);

	if(LOCAL_1 < 10)
		return;

	INT32 LOCAL_4 = 0;
	for(LOCAL_2 = 0; LOCAL_2 < 60; LOCAL_2 ++)
	{
		INT32 LOCAL_5 = Mob[clientId].Mobs.Player.Inventory[LOCAL_2].Index;
		if(LOCAL_5 != searched)
			continue;

		while(Mob[clientId].Mobs.Player.Inventory[LOCAL_2].Index == searched)
		{
			AmountMinus(&Mob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_4++;

			if(LOCAL_4 >= 10)
				break;
		}

		if(LOCAL_4 >= 10)
			break;
	}

	INT32 added = false;
	for(INT32 i = 0; i < 60; i++) 
	{
		st_Item *item = &Mob[clientId].Mobs.Player.Inventory[i];
		if(item->Index == earned)
		{
			INT32 amount = GetItemAmount(item);
			if(amount >= 120)
				continue;

			amount ++;
			int i = 0;
			for(; i < 3; i++)
			{
				if(item->Effect[i].Index == EF_AMOUNT)
				{
					item->Effect[i].Value = amount;
					break;
				}
			}

			if(i == 3)
			{
				for(i = 0; i < 3; i++)
				{
					if(item->Effect[i].Index == 0)
					{
						item->Effect[i].Index = EF_AMOUNT;
						item->Effect[i].Value = amount;

						break;
					}
				}
			}

			if(i != 3)
			{
				SendItem(clientId, SlotType::Inv, i, &Mob[clientId].Mobs.Player.Inventory[i]);
				
				added = true;
				break;	
			}
		}
	}

	if(!added) 
	{
		st_Item LOCAL_7;
		memset(&LOCAL_7, 0, sizeof st_Item);
		LOCAL_7.Index = earned;

		INT32 LOCAL_8 = GetFirstSlot(clientId, 0);

		if(LOCAL_8 == -1)
		{
			INT32 LOCAL_13 = Mob[clientId].Target.X;
			INT32 LOCAL_14 = Mob[clientId].Target.Y;

			Log(clientId, LOG_COMP, "O item %s não foi adicionado por falta de espaço [%d]", ItemList[earned].Name, earned);
		}
		else
		{
			Mob[clientId].Mobs.Player.Inventory[LOCAL_8] = LOCAL_7;

			SendItem(clientId, SlotType::Inv, LOCAL_8, &LOCAL_7);
		}
	}
}

float TimeRemaining(stDate date)
{
	time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	int month  = now.tm_mon; //0 Janeiro, 1 Fev
	int day    = now.tm_mday;
	int year   = now.tm_year;

	int dia_ = 0, mes_ = 0, ano_ = 0, hora_ = 0, min_ = 0, sec_;

	ano_  = date.Ano;
	dia_  = date.Dia;
	mes_  = date.Mes;

	hora_ = date.Hora;
	min_  = date.Minuto;
	sec_  = date.Segundo;

	struct std::tm a = {now.tm_sec, now.tm_min,now.tm_hour, day, month, year};
	struct std::tm b = {sec_, min_, hora_, dia_, mes_ - 1, ano_ - 1900};
	
	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	double timeDiv = 0;
	if ( x != (std::time_t)(-1) && y != (std::time_t)(-1) )
		timeDiv = (INT32)(std::difftime(y, x) / 8);

	return static_cast<float>(timeDiv);
}

int RandomItemEffect()
{
	INT32 rnd = Rand() % 105;
	if (rnd <= 35)
		return 0;
	else if (rnd > 35 && rnd <= 70)
		return 1;

	return 2;
}

void DoNightmare()
{
	for(INT32 i = 0 ; i < 3; i++)
	{
		// Checa se está ligado - 0 = Desligado. 1 = 4 min de espera. 2 = em funcionamento
		if(sServer.Nightmare[i].Status == 0)
			continue;

		INT32 timer = sServer.Nightmare[i].TimeLeft - 2;

		// Checa se o tempo já acabou e se acabou a 1° etapa
		// de preparação
		if(timer <= 0 && sServer.Nightmare[i].Status == 1)
		{
			for(int x = 0; x < 8; x++)
			{
				for(int y = 1 ; y < 4; y++)
				{
					int spawnId = g_pPesaGenerate[i][x][y];
					if(spawnId <= 0 || spawnId >= 8032)
						continue;

					GenerateMob(spawnId, 0, 0);
				}
			}

			for(INT32 x = 0; x < 40; x++)
			{

				INT32 memberId = sServer.Nightmare[i].Members[x];
				if(memberId <= 0 || memberId >= MAX_PLAYER)
					continue;

				if (Users[memberId].Status != USER_PLAY)
				{
					sServer.Nightmare[i].Members[x] = 0;
					continue;
				}

				SendSignalParm(memberId, memberId, 0x3B0, 8);
				SendSignalParm(memberId, memberId, 0x3A1, 15 * 60);
			} 

			// Em processo normal de up
			sServer.Nightmare[i].Status = 2; 

			// Seta o novo tempo
			timer = 15 * 60;
		}

		if(sServer.Nightmare[i].Status == 2)
		{
			INT32 alive = 0;
			for(INT32 x = 0; x < 8; x++)
			{
				if(sServer.Nightmare[i].Alive[x])
					continue;

				int npcId = g_pPesaGenerate[i][x][0];
				if(npcId <= 0 || npcId > MAX_NPCGENERATOR)
					continue;

				if(mGener.pList[npcId].MobCount == 0)
				{
					sServer.Nightmare[i].Alive[x] = true;

					continue;
				}
					
				alive ++;

				for(int y = 1 ; y < 4; y++)
				{
					int spawnId = g_pPesaGenerate[i][x][y];
					if(spawnId <= 0 || spawnId > MAX_NPCGENERATOR)
						continue;

					GenerateMob(spawnId, 0, 0);
				}				
			}

			for(INT32 x = 0; x < 40; x++)
			{
				INT32 memberId = sServer.Nightmare[i].Members[x];
				if(memberId <= 0 || memberId >= MAX_PLAYER)
					continue;

				if(Users[memberId].Status != USER_PLAY)
				{
					sServer.Nightmare[i].Members[x] = 0;

					continue;
				}

				SendSignalParm(memberId, memberId, 0x3B0, alive);
				if (Mob[memberId].Mobs.Player.Equip[0].EFV2 >= static_cast<int>(CELESTIAL) && Mob[memberId].Mobs.Player.bStatus.Level >= sServer.MaximumPesaLevel)
				{
					Log(memberId, LOG_INGAME, "Removido da área do pesadelo por atingir o limite de nível");

					DoRecall(memberId);
					sServer.Nightmare[i].Members[x] = 0;
					sServer.Nightmare[i].MembersName[x].clear();
				}
			} 

			sServer.Nightmare[i].NPCsLeft = alive;
		}

		if(timer <= 0)
		{
			for(int x = 1000; x < MAX_SPAWN_MOB; x++)
			{
				if(!Mob[x].Mode)
					continue;

				if(Mob[x].Target.X >= g_pPesaArea[i][0] && Mob[x].Target.X <= g_pPesaArea[i][2] && Mob[x].Target.Y >= g_pPesaArea[i][1] && Mob[x].Target.Y <= g_pPesaArea[i][3])
					DeleteMob(x, 1);
			}

			ClearArea(g_pPesaArea[i][0], g_pPesaArea[i][1], g_pPesaArea[i][2], g_pPesaArea[i][3]);

			sServer.Nightmare[i] = TOD_Nightmare{};
			timer = 0;
		}
	
		sServer.Nightmare[i].TimeLeft = timer;
	}

}

void DoWater()
{
	static const INT32 waterId[3] = {3174, 778, 3183};
	
	for(INT32 i = 0; i < 3; i++)
	{
		stWater *water = sServer.pWater[i];
		for(INT32 x = 0; x < 9; x++)
		{
			INT32 timer = water[x].Time;
			if(timer == -1)
				continue;

			INT32 initial = PERGA_A;
			if(i == 0)
				initial = PERGA_N;
			else if(i == 1)
				initial = PERGA_M;

			// O tempo da água acabou
			if(timer <= 0 && water[x].Mode != 2)
			{
				for(INT32 t = 1000; t < 30000; t++)
				{
					if(Mob[t].Target.X >= waterMaxMin[i][x][0] && Mob[t].Target.X <= waterMaxMin[i][x][2] && Mob[t].Target.Y >= waterMaxMin[i][x][1] && Mob[t].Target.Y <= waterMaxMin[i][x][3])
						MobKilled(t, t, 0, 0);
				}
				
				// Reseta a área e teleporta todos para fora da Zona Elemental da água...
				ClearAreaTeleport(waterMaxMin[i][x][0], waterMaxMin[i][x][1], waterMaxMin[i][x][2], waterMaxMin[i][x][3], 1965, 1770);
				
				INT32 leader = water[x].Leader;
				
				if(x != 8 && leader != 0 && leader > 0 && leader < MAX_PLAYER && Users[leader].Status == 22)
				{
					INT32 slotId = GetFirstSlot(leader, 0);
					if(slotId != -1)
					{
						memset(&Mob[leader].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

						Mob[leader].Mobs.Player.Inventory[slotId].Index = waterId[i] + x;
						SendItem(leader, SlotType::Inv, slotId, &Mob[leader].Mobs.Player.Inventory[slotId]);
					}
				}

				if(x == 8)
				{
					mGener.pList[initial + 8].MobCount = 0;
					mGener.pList[initial + 9].MobCount = 0;
					mGener.pList[initial + 10].MobCount = 0;
					mGener.pList[initial + 11].MobCount = 0;
				}
				else
					mGener.pList[initial + x].MobCount = 0;

				water[x].Time = -1;
				water[x].Mode = 0;
				continue;
			}

			if(timer <= 0 && water[x].Mode == 2)
			{
				// Reseta a área e teleporta todos para fora da Zona Elemental da água...
				ClearAreaTeleport(waterMaxMin[i][x][0], waterMaxMin[i][x][1], waterMaxMin[i][x][2], waterMaxMin[i][x][3], 1965, 1770);

				water[x].Mode = 0;
				water[x].Leader = 0;
				water[x].Time = -1;
			}

			// Checa agora se o matou todos os mobs
			INT32 mobCount = mGener.pList[initial + x].MobCount;
			if(x == 8)
				mobCount = mGener.pList[initial + 8].MobCount + mGener.pList[initial + 9].MobCount + mGener.pList[initial + 10].MobCount + mGener.pList[initial + 11].MobCount;

			if(mobCount <= 1 && water[x].Mode == 1)
			{
				water[x].Mode = 2;
				water[x].Time = 15;
				timer = 18;

				INT32 leader = water[x].Leader;
				if(leader > 0 && leader < MAX_PLAYER && Users[leader].Status == USER_PLAY)
				{
					for(INT32 p = 0; p < 12; p++)
					{
						INT32 party = Mob[leader].PartyList[p];
						if(party <= 0 || party >= MAX_PLAYER)
							continue;

						SendSignalParm(party, 0x7530, 0x3A1, 15);
					}
					
					INT32 slotId = GetFirstSlot(leader, 0);
					if(slotId != -1 && x != 8)
					{
						memset(&Mob[leader].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

						Mob[leader].Mobs.Player.Inventory[slotId].Index = waterId[i] + x;
						SendItem(leader, SlotType::Inv, slotId, &Mob[leader].Mobs.Player.Inventory[slotId]);
					}
				}
				
				SendSignalParm(leader, SERVER_SIDE, 0x3A1, 15);
			}
			else
			{
				INT32 leader = water[x].Leader;
				if(leader > 0 && leader < MAX_PLAYER && Users[leader].Status == 22)
				{
					for(INT32 p = 0; p < 12; p++)
					{
						INT32 party = Mob[leader].PartyList[p];
						if(party <= 0 || party >= MAX_PLAYER)
							continue;

						SendSignalParm(party, SERVER_SIDE, 0x3B0, mobCount);
					}
					
					if(Users[leader].Status == USER_PLAY)
						SendSignalParm(leader, SERVER_SIDE, 0x3B0, mobCount);
					else
						water[x].Leader = 0;
				}
			}

			if(water[x].Time != -1)
				water[x].Time = timer - 3;
		}
	}
}

void LogRune(int sala, int party, std::string message)
{
    if (sala < 0 || sala >= 10)
        return;

    if (party < 0 || party >= 3)
        return;

    const stPista *pista = &pPista[sala];
    for (int i = 0; i < 13; ++i)
    {
        int memberId = pista->Clients[party][i];
        if (memberId <= 0 || memberId >= MAX_PLAYER)
            continue;

        Log(memberId, LOG_INGAME, message.c_str());
    }
}

void MessageRune(int sala, int party, std::string message)
{
    if (sala < 0 || sala >= 10)
        return;

    if (party < 0 || party >= 3)
        return;

    const stPista *pista = &pPista[sala];
    for (int i = 0; i < 13; ++i)
    {
        int memberId = pista->Clients[party][i];
        if (memberId <= 0 || memberId >= MAX_PLAYER)
            continue;

        Log(memberId, LOG_INGAME, message.c_str());
    }
}

void GiveRuna(int sala, int party)
{
	stPista *pista = &pPista[sala];

	for (int iRunes = 0; iRunes < sServer.RunesPerSanc; iRunes++)
	{
		std::vector<const CUser*> receivedUsers;
		for (int i = 0; i < 13; i++)
		{
			int memberId = pista->Clients[party][i];
			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (Mob[memberId].Target.X < g_pPistaCoords[sala][0] || Mob[memberId].Target.X > g_pPistaCoords[sala][2] || Mob[memberId].Target.Y < g_pPistaCoords[sala][1] || Mob[memberId].Target.Y > g_pPistaCoords[sala][3])
			{
				Log(memberId, LOG_INGAME, "Não recebeu a runa pois estava fora da área. Sala %d. %ux %uy", sala, Mob[memberId].Target.X, Mob[memberId].Target.Y);
				LogPlayer(memberId, "Não recebeu a Runa da sala %d pois estava fora da área", sala);

				continue;
			}

			if (Mob[memberId].Mobs.Player.Status.curHP <= 0)
			{
				Log(memberId, LOG_INGAME, "Não recebeu a runa pois estava morto na área");
				LogPlayer(memberId, "Não recebeu a Runa da sala %d pois estava fora da área", sala);

				continue;
			}

			int slotId = GetFirstSlot(memberId, 0);
			if (slotId == -1)
			{
				SendClientMessage(memberId, "Sem espaço no inventário para receber a Runa");

				Log(memberId, LOG_INGAME, "Não recebeu runas pois estavam sem espaço");
				LogPlayer(memberId, "Não recebeu a Runa da sala %d por não possuir espaço no inventário", sala);
				continue;
			}

			const CUser* user = &Users[memberId];
			auto isSameUser = [user](const CUser* receivedUser) {
				return memcmp(user->MacAddress, receivedUser->MacAddress, 8) == 0;
			};

			auto totalFound = std::count_if(std::begin(receivedUsers), std::end(receivedUsers), isSameUser);
			if (totalFound >= 2)
			{
				std::stringstream str;
				str << "Recebido runas pelas contas " << std::endl;
				for (auto receivedUser : receivedUsers)
				{
					if (isSameUser(receivedUser))
						str << receivedUser->User.Username << std::endl;
				}

				Log(user->clientId, LOG_INGAME, str.str().c_str());
				continue;
			}

			st_Item *item = &Mob[memberId].Mobs.Player.Inventory[slotId];

			auto runeRoom = Runes[sala];
			int itemIndex = 0;
			int tries = 0;
			while (tries++ < 30 && (itemIndex <= 0 || itemIndex >= MAX_ITEMLIST))
				itemIndex = *select_randomly(runeRoom.begin(), runeRoom.end());

			if (itemIndex <= 0 || itemIndex > 6500)
				itemIndex = runeRoom[0];

			memset(item, 0, sizeof st_Item);

			item->Index = itemIndex;

			SendItem(memberId, SlotType::Inv, slotId, item);

			Log(memberId, LOG_INGAME, "Recebeu a runa %s. Sala %d ( %hu )", ItemList[item->Index].Name, sala, item->Index);
			LogPlayer(memberId, "Recebeu a runa %s. Sala %d.", ItemList[item->Index].Name, sala);

			receivedUsers.push_back(user);
		}
	} 

	int leaderId = pista->Clients[party][12];
	int slotId = GetFirstSlot(leaderId, 0);

	if (Mob[leaderId].Target.X < g_pPistaCoords[sala][0] || Mob[leaderId].Target.X > g_pPistaCoords[sala][2] || Mob[leaderId].Target.Y < g_pPistaCoords[sala][1] || Mob[leaderId].Target.Y > g_pPistaCoords[sala][3])
	{
		Log(leaderId, LOG_INGAME, "Não recebeu a Pista de Runas pois estava fora da área. Sala %d. %ux %uy", sala, Mob[leaderId].Target.X, Mob[leaderId].Target.Y);
		LogPlayer(leaderId, "Não recebeu a Pista de Runas pois estava fora da área. Sala: %d", sala + 1);
		return;
	}

	if(sala == 6)
		return;



	if(slotId == -1)
	{
		SendClientMessage(leaderId, "!Não foi possível receber a Pista de Runas. Falta espaço no inventário");
		Log(leaderId, LOG_INGAME, "Não foi possível receber a pista de runas pois não possuía espaço no inventário. Sala: %d", sala);
	}
	else
	{
		memset(&Mob[leaderId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

		Mob[leaderId].Mobs.Player.Inventory[slotId].Index = 5134;

		Mob[leaderId].Mobs.Player.Inventory[slotId].EF1 = EF_SANC;
		Mob[leaderId].Mobs.Player.Inventory[slotId].EFV1 = sala + 1;

		SendItem(leaderId, SlotType::Inv, slotId, &Mob[leaderId].Mobs.Player.Inventory[slotId]);
	}
}	

void DoRune()
{
	constexpr std::array positions =
	{
		st_Position { 3299, 1687 },
		st_Position { 3299, 1701 }
	};

	size_t positionIndex = 0;
	size_t totalReseted = 0;
	for(INT32 i = 0 ; i < MAX_ROOM; i++)
	{
		stPista *pista = &pPista[i];
		if(!pista->Status)
			continue;

		if(pista->Timer - 2 <= 0)
		{
			switch(i)
			{
				case 1: // 
				{
					//bool giveRuna = true;
					bool giveRuna[3] = { false, false, false};
					for (int x = 0; x < 3; x++)
					{
						if (mGener.pList[TORRE_ID + x].MobCount != 0)
							giveRuna[x] = true;

					}

					for (int x = 0; x < 3; x++) //Max party = 3
					{
						if (giveRuna[x])
							GiveRuna(i, x);
					}
				}
				break;
				case 3:
				{
					int morePoints[2] = {-1, -1};
					for(int i = 0 ; i < MAX_PARTYPISTA;i ++)
					{
						if(pista->inSec.Points[i] > morePoints[1])
						{
							morePoints[1] = pista->inSec.Points[i];
							morePoints[0] = i;
						}
					}

					if (morePoints[0] != -1)
						GiveRuna(i, morePoints[0]);
				}
				break;
			}

			auto minX = g_pPistaCoords[i][0];
			auto minY = g_pPistaCoords[i][1];
			auto maxX = g_pPistaCoords[i][2];
			auto maxY = g_pPistaCoords[i][3];

			// Limpa os mobs da área
			for(int x = 1000; x < 30000; x++)
			{
				if(Mob[x].Target.X >= minX && Mob[x].Target.X <= maxX && Mob[x].Target.Y >= minY && Mob[x].Target.Y <= maxY)
					DeleteMob(x, 1);
			}

			// Limpa a array de players e teleporta para o HallKefra
			for(int y = 0;y < MAX_PARTYPISTA; y++)
			{
				for(int z = 0;z<13;z++)
				{
					if(pista->Clients[y][z] == 0)
						continue;
					
					INT32 mobId = pista->Clients[y][z];
					if (Mob[mobId].Target.X >= minX && Mob[mobId].Target.X <= maxX && Mob[mobId].Target.Y >= minY && Mob[mobId].Target.Y <= maxY)
					{
						if (totalReseted != 0 && (totalReseted++ % 30) == 0)
							positionIndex++;

						if (positionIndex >= positions.size())
							positionIndex = 0;

						Teleportar(pista->Clients[y][z], positions[positionIndex].X, positions[positionIndex].Y);
					}
					else 
						Log(mobId, LOG_INGAME, "Não retornou a área do Uxmal por não estar dentro da área da pista. %u %u", Mob[mobId].Target.X, Mob[mobId].Target.Y);

					pista->Clients[y][z] = 0;

					totalReseted++;
				}
			}

			// Reseta por completo a área.
			std::string pistaName{ "Pista +" + std::to_string(i + 1) };
			ClearAreaTeleport(minX, minY, maxX, maxY, 3299, 1687, pistaName.c_str());

			*pista = stPista{};
		}
		else
		{
			pista->Timer -= 2;

			if(i == 2 || i == 6)
			{
				if(pista->Timer <= 600 && !pista->inSec.Born)
				{
					INT32 mobId = VALKY_ID;
					if(i == 6)
						mobId = COELHO_ID;
						
					GenerateMob(mobId, 0, 0);

					for(int y = 1000;y < 30000;y++)
					{
						if(Mob[y].GenerateID == mobId)
						{
							pista->inSec.BossID = y;

							break;
						}
					}

					pista->inSec.Born = true;

					for(int x = 0; x < 3; x++)
					{
						for(int y = 0; y < 13; y++)
						{
							int cId = pista->Clients[x][y];
							if(Users[cId].Status != USER_PLAY)
							{
								pista->Clients[x][y] = 0;

								continue;
							}

							SendClientMessage(cId, "O Boss nasceu.");
						}
					}
				}
			}
			else if(i == 5)
			{
				for(INT32 room = 0; room < 4; room++)
				{
					for(int tele = 0; tele < g_pTeleBarlog[room][0][6];tele ++)
					{
						for(int posY = g_pTeleBarlog[room][tele][3]; posY <= g_pTeleBarlog[room][tele][1]; posY++)
						{
							for(int posX = g_pTeleBarlog[room][tele][2]; posX <= g_pTeleBarlog[room][tele][0]; posX++)
							{
								int mobId = g_pMobGrid[posY][posX];
								if(mobId == 0)
									continue;

								int party = -1;

								for(int liderId = 0; liderId < MAX_PARTYPISTA; liderId ++)
								{
									if(mobId == pista->Clients[liderId][12])
									{
										party = liderId;

										break;
									}
								}

								if(party == -1)
									continue;

								int semente = GetFirstSlot(mobId, 4032);
								if(semente == -1)
								{
									SendClientMessage(mobId, "Necessário Semente de Cristal para avançar a sala");

									break;
								}

								int _rand = Rand() % 100;

								if(_rand <= 70)
								{
									for(int x = 0; x < 13; x++)
									{
										int memberId = pista->Clients[party][x];
										if(memberId <= 0 || memberId >= MAX_PLAYER)
											continue;

										Teleportar(memberId, g_pTeleBarlog[pista->inSec.Room[party] + 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] + 1][0][5]);
									}

									Teleportar(mobId, g_pTeleBarlog[pista->inSec.Room[party] + 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] + 1][0][5]);

									pista->inSec.Room[party] ++;

									if(pista->inSec.Room[party] == 4)
										GenerateMob(BARLOG, 0, 0);

									memset(&Mob[mobId].Mobs.Player.Inventory[semente], 0, sizeof st_Item);

									SendItem(mobId, SlotType::Inv, semente, &Mob[mobId].Mobs.Player.Inventory[semente]);

									Log(mobId, LOG_INGAME, "Teleportado BARLOG para sala %d", pista->inSec.Room[party]);
								}
								else if(_rand <= 90 && pista->inSec.Room[party] > 0)
								{
									for(int x = 0; x < 13; x++)
									{
										int memberId = pista->Clients[party][x];
										if(memberId <= 0 || memberId >= MAX_PLAYER)
											continue;

										Teleportar(memberId, g_pTeleBarlog[pista->inSec.Room[party] - 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] - 1][0][5]);
									}

									Teleportar(mobId, g_pTeleBarlog[pista->inSec.Room[party] - 1][0][4], g_pTeleBarlog[pista->inSec.Room[party] - 1][0][5]);
								
									memset(&Mob[mobId].Mobs.Player.Inventory[semente], 0, sizeof st_Item);

									SendItem(mobId, SlotType::Inv, semente, &Mob[mobId].Mobs.Player.Inventory[semente]);

									Log(mobId, LOG_INGAME, "Teleportado BARLOG para sala %d - Voltou uma sala", pista->inSec.Room[party]);

									pista->inSec.Room[party] --;
								}
								else
								{
									memset(&Mob[mobId].Mobs.Player.Inventory[semente], 0, sizeof st_Item);
									SendItem(mobId, SlotType::Inv, semente, &Mob[mobId].Mobs.Player.Inventory[semente]);

									SendClientMessage(mobId, "Nada aconteceu, tente novamente...");

									Log(mobId, LOG_INGAME, "Tentou teleporte barlog. SALA %d", pista->inSec.Room[party]);
								}
							}
						}
					}
				}
			}
		}
	}
	
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	if(!(now.tm_min % 20))
	{
		for(INT32 i = 0 ; i < MAX_AVAIBLE_ROOM; i++)
		{
			stPista *pista = &pPista[i];
			if(pista->Status)
				continue;

			int x;
			for(x = 0; x < 3; x++)
				if(pista->Clients[x][12] > 0)
					break;

			if(x == 3)
			{
				pista->Status = false;
				pista->Timer = 0;

				continue;
			}

			for(x = 1000; x < 30000; x++)
			{
				if(Mob[x].Target.X >= g_pPistaCoords[i][0] && Mob[x].Target.X <= g_pPistaCoords[i][2] && Mob[x].Target.Y >= g_pPistaCoords[i][1] && Mob[x].Target.Y <= g_pPistaCoords[i][3])
					DeleteMob(x, 1);
			}

			std::stringstream str;
			if(i == 4)
			{
				int roomSorted = -1;
				for(int i = HELL; i < (HELL + 47); i++)
				{
					if(i == HELL_BOSS)
						continue;

					GenerateMob(i, 0, 0);
				}

				roomSorted = Rand() % 47;

				if (roomSorted >= 0 && roomSorted < 47)
				{
					pista->inSec.RoomSorted[roomSorted] = true;
					str << "Sala sorteada para ir ao boss: " << roomSorted << std::endl;
				}
				else
				{
					pista->inSec.RoomSorted[30] = true;
					str << "Sala sorteada para ir ao boss: " << 30 << std::endl;
				}
			}

			if(i != 2 && i != 4 && i != 5 && i != 6)
			{
				for(int x = 0 ; x < g_pGenerateLoops[i]; x++)
				{
					int generate = g_pGenerateIndexes[i] + x;
					if(generate <= 0 || generate >= MAX_NPCGENERATOR)
						continue;

					GenerateMob(generate, 0 ,0);
				}
			}

			for(x = 0; x < MAX_PARTYPISTA;x ++)
			{
				std::stringstream partyStr;
				partyStr << str.str();
				for(int y = 0; y < 13; y++)
				{
					int memberId = pista->Clients[x][y];
					if(memberId == 0)
						continue;

					// 3317 1674 3275 1712
					if(Mob[memberId].Target.X >= 3275 && Mob[memberId].Target.X <= 3317 && Mob[memberId].Target.Y >= 1674 && Mob[memberId].Target.Y <= 1712)
					{
						if (i == 1) // Pista + 1
						{
							// Teleporta o gp para a respectiva coordenada da torre
							// caso a torre que esteja do lado dele cair, ele se fode
							if (x == 0)
								Teleportar(memberId, 3387, 1554);
							else if (x == 1)
								Teleportar(memberId, 3418, 1577);
							else if (x == 2)
								Teleportar(memberId, 3358, 1578);
						}
						else
							Teleportar(memberId, g_pPistaCoords[i][4], g_pPistaCoords[i][5]);

						partyStr << "Grupo com " << Mob[memberId].Mobs.Player.Name << "(" << Users[memberId].User.Username << ") para a pista +" << i << std::endl;

						SendSignalParm(memberId, memberId, 0x3A1, 900);
						Log(memberId, LOG_INGAME, "Teleportado para pista %d - %ux %ux", i, g_pPistaCoords[i][4], g_pPistaCoords[i][5]);
					}
					else
						Log(memberId, LOG_INGAME, "Não foi teleportado para pista pois estava fora da área de Kefra %hux %huy", Mob[memberId].Target.X, Mob[memberId].Target.Y);
				}

				for (int y = 0; y < 13; y++)
				{
					int memberId = pista->Clients[x][y];
					if (memberId == 0)
						continue;

					Log(memberId, LOG_INGAME, partyStr.str().c_str());
				}
			}

			pista->Timer = 900;
			pista->Status = true;
		}
	}
}

void LogPlayer(INT32 clientId, const char *msg, ...)
{
	return;
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	FILE *pFile = NULL;
	
	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	char first[16];
	BASE_GetFirstKey(Users[clientId].User.Username, first);
	
	char szFileName[512];
	sprintf_s(szFileName, "..\\Logs\\Site\\%s\\%s - %02d-%02d-%02d.txt", first, Users[clientId].User.Username, now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));
			
	// Abre o arquivo de log 
	fopen_s(&pFile, szFileName, "a+");

	if(pFile)
	{
		// Inicia a lista de argumentos
		va_list arglist;

		if(Users[clientId].Status == USER_PLAY)
		{
			// Insere a hora no arquivo
			fprintf(pFile, "\n%02d:%02d:%02d %s : ",
					now.tm_hour, now.tm_min, now.tm_sec , Mob[clientId].Mobs.Player.Name);
		}
		else
		{
			// Insere a hora no arquivo
			fprintf(pFile, "\n%02d:%02d:%02d : ",
					now.tm_hour, now.tm_min, now.tm_sec);
		}
		
		va_start(arglist, msg);
		vfprintf(pFile, msg, arglist);

		va_end(arglist);
		fclose(pFile);
	}
}

void Log(INT32 clientId, INT32 type, const char *msg, ...)
{
	try 
	{
		time_t rawnow = time(NULL);
		struct tm now; localtime_s(&now, &rawnow);

		char szFileName[512] = { 0 };
		if (clientId == SERVER_SIDE)
		{
			FILE *pFile = nullptr;

			std::lock_guard<std::recursive_mutex> mutex{ sServer.logMutex };

			if(type == LOG_HACK)
				sprintf_s(szFileName, "..\\Logs\\Servidor_%d\\Hack_%02d-%02d-%02d.txt", sServer.Channel, now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));
			else
				sprintf_s(szFileName, "..\\Logs\\Servidor_%d\\%02d-%02d-%02d.txt", sServer.Channel, now.tm_mday, (now.tm_mon + 1), (now.tm_year - 100));

			// Abre o arquivo de log 
			fopen_s(&pFile, szFileName, "at+");

			if (pFile)
			{
				// Inicia a lista de argumentos
				va_list arglist;

				// Insere a hora no arquivo
				fprintf(pFile, "\n%02d:%02d:%02d : ",
					now.tm_hour, now.tm_min, now.tm_sec);

				// Insere o log em si
				va_start(arglist, msg);
				vfprintf(pFile, msg, arglist);
				va_end(arglist);

				// Fecha o arquivo
				fclose(pFile);
			}

#if defined(_DEBUG)
			// Inicia a lista de argumentos
			va_list arglist;
			va_start(arglist, msg);

			printf("\n%02d:%02d:%02d : ", now.tm_hour, now.tm_min, now.tm_sec);
			vprintf(msg, arglist);
			printf("\n");

			va_end(arglist);
#endif
		}
		else if (clientId < MAX_PLAYER && clientId > 0)
		{
			if (!Users[clientId].User.Username[0])
				return;

			std::lock_guard<std::recursive_mutex> mutex{ Users[clientId].logMutex };
			if (type == LOG_HACK)
			{
				// Inicia a lista de argumentos
				va_list arglist;
				va_start(arglist, msg);

				std::string message;

				if (Users[clientId].Status == USER_PLAY)
					message += "["s + Mob[clientId].Mobs.Player.Name + "] - "s;

				message += msg;

				if (Users[clientId].HackLog != nullptr)
					Users[clientId].HackLog->Log(message.c_str(), arglist);

				va_end(arglist);
			}

			if (Users[clientId].NormalLog != nullptr)
			{
				// Inicia a lista de argumentos
				va_list arglist;
				va_start(arglist, msg);

				std::string message;

				if (Users[clientId].Status == USER_PLAY)
					message += "["s + Mob[clientId].Mobs.Player.Name + "] - "s;

				message += msg;

				Users[clientId].NormalLog->Log(message.c_str(), arglist);
				va_end(arglist);
			}
#if defined(_DEBUG)
			// Inicia a lista de argumentos
			va_list arglist;
			va_start(arglist, msg);

			printf("\n%02d:%02d:%02d %s : ", now.tm_hour, now.tm_min, now.tm_sec, Mob[clientId].Mobs.Player.Name);
			vprintf(msg, arglist);
			printf("\n");

			va_end(arglist);
#endif
		}
	}
	catch (std::exception& e)
	{
		static bool isOnError = false;

		if (!isOnError)
		{
			isOnError = true;
			::Log(SERVER_SIDE, LOG_INGAME, "Erro estranho no log. Mensagem: %s", e.what());
			isOnError = false;
		}
	}
	catch (...)
	{
		static bool isOnError = false;

		if (!isOnError)
		{
			isOnError = true;
			::Log(SERVER_SIDE, LOG_INGAME, "Erro estranho no log. Sem mensagme de erro. GetLastError() %d. errno %d", GetLastError(), errno);
			isOnError = false;
		}
	}
}

INT32 GetGuild(st_Item *item)
{
	return 0;
}

void DoSummon(int arg1, unsigned int arg2, unsigned int arg3)
{
	INT32 LOCAL_1 = GetEmptyMobGrid(arg1, &arg2, &arg3);
	if (!LOCAL_1)
	{
		if (arg1 < MAX_PLAYER)
			Log(arg1, LOG_INGAME, "DoSummon - Falha ao encontrar espaço vago no mapa para o usuário. Posição: %ux %uy", arg2, arg3);

		return;
	}

	p36C LOCAL_14;
	GetAction(arg1, arg2, arg3, &LOCAL_14);

	LOCAL_14.MoveType = 1;

	if(arg1 < MAX_PLAYER)
		Users[arg1].AddMessage((BYTE*)&LOCAL_14, sizeof p36C);

	GridMulticast(arg1, arg2, arg3, (BYTE*)&LOCAL_14);
}

bool AddCrackError(int arg1, int point, int type)
{
	if(type!= 3 && type != 8 && type != 15)
		Log(arg1, LOG_HACK, "CrackError - Points: %d type: %d", point, type);

	Users[arg1].CrackCount += point;

	if(Users[arg1].CrackCount >= 30)
	{
		Log(arg1, LOG_HACK, "Desconectado - CrackCount: %d - point: %d - type: %d", Users[arg1].CrackCount, point, type);

		Users[arg1].CrackCount = 0;

		SendClientMessage(arg1, g_pLanguageString[_NN_Bad_Network_Packets]);

		if(Users[arg1].Status != USER_SELCHAR)
		{
			CharLogOut(arg1);

			Log(arg1, LOG_HACK, "Enviado para a tela de personagem. CrackError atingiu o limite");
		}

		return true;
	}

	return false;
}

BOOL CheckPacket(PacketHeader *Header)
{
	if(Header == NULL)
		return false;

	// Código - 1 = erro - 0 = ok
	BOOL code = TRUE;

	INT32 packetId = Header->PacketId;
	INT32 size = Header->Size;

	if(packetId == 0x20D && size != sizeof p20D)
		code = FALSE;
	else if(packetId == 0x20F && size != sizeof p20F)
		code = FALSE;
	else if(packetId == 0x36C && size != sizeof p36C)
		code = FALSE;
	else if(packetId == 0x366 && size != sizeof p36C)
		code = FALSE;
	else if(packetId == 0x368 && size != sizeof p36C)
		code = FALSE;
	else if(packetId == 0x37A && size != sizeof p37A)
		code = FALSE;
	else if(packetId == 0x39E && (size != sizeof p39D && size != sizeof p367 && size != sizeof p39D + sizeof st_Target))
		code = FALSE;
	else if(packetId == 0x39D && (size != sizeof p39D && size != sizeof p367 && size != sizeof p39D + sizeof st_Target))
		code = FALSE;
	else if(packetId == 0x367 && (size != sizeof p39D && size != sizeof p367 && size != sizeof p39D + sizeof st_Target))
		code = FALSE;
	else if(packetId == 0x215 && size != sizeof PacketHeader)
		code = FALSE;
	else if(packetId == 0x333 && size != sizeof p333)
		code = FALSE;
	else if(packetId == 0x334 && size != sizeof p334)
		code = FALSE;
	else if(packetId == 0x376 && size != sizeof p376)
		code = FALSE;
	else if(packetId == 0x397 && size != sizeof p397)
		code = FALSE;
	else if(packetId == 0x398 && size != sizeof p398)
		code = FALSE;
	else if(packetId == 0x39A && size != sizeof p39A)
		code = FALSE;
	else if(packetId == 0x384 && size != 12)
		code = FALSE;
	else if(packetId == 0x291 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x277 && size != sizeof p277)
		code = FALSE;
	else if(packetId == 0x27B && size != sizeof p27B)
		code = FALSE;
	else if(packetId == 0x379 && size != sizeof p379)
		code = FALSE;
	else if(packetId == 0x374 && size != sizeof p374)
		code = FALSE;
	else if((packetId == 0x3A6 || packetId == 0x2C3 || packetId == 0x2D2 || packetId == 0x2D3 || packetId == 0x2C4 || packetId == 0x3BA || packetId == 0x3B5 || packetId == 0x3C0) && size != sizeof pCompor)
		code = FALSE;
	else if(packetId == 0x3D5 && size != sizeof pMsgSignal2)
		code = FALSE;
	else if(packetId == 0x28C && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x378 && size != sizeof p378)
		code = FALSE;
	else if(packetId == 0x37F && size != sizeof p37F)
		code = FALSE;
	else if(packetId == 0x3AB && size != sizeof p3AB)
		code = FALSE;
	else if(packetId == 0x37E && size != sizeof p37E)
		code = FALSE;
	else if(packetId == 0x272 && size != sizeof p272)
		code = FALSE;
	else if(packetId == 0x2E4 && size != sizeof p2E4)
		code = FALSE;
	else if(packetId == 0x270 && size != sizeof p270)
		code = FALSE;
	else if(packetId == 0x399 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x383 && size != sizeof p383)
		code = FALSE;
	else if(packetId == 0x387 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x388 && size != sizeof pMsgSignal)
		code = FALSE;
	else if(packetId == 0x39F && size != sizeof p39F)
		code = FALSE;
	else if(packetId == 0x28B && size != sizeof p28B)
		code = FALSE;
	else if(packetId == 0x2E5 && size != sizeof p2E5)
		code = FALSE;
	else if (packetId == 0xE12 && size != sizeof pE12)
		code = FALSE;
	else if (packetId == 0x36A && size != sizeof p36A)
		code = FALSE;

	return code;
}

void DoAlly(INT32 guild, INT32 ally)
{
	if(guild <= 0 || ally < 0 || guild >= MAX_GUILD || ally >= MAX_GUILD)
		return;

	if (IsWarTime())
		return;

	INT32 allyId = g_pGuildAlly[guild]; // local 8
	if(allyId < 0 || allyId >= MAX_GUILD)
		allyId = 0;

	INT32 allyAlly = g_pGuildAlly[ally]; // LOCAL_9
	if(ally == 0)
	{
		allyId = g_pGuildAlly[guild];

		if(allyId > 0 && allyId < MAX_GUILD)
		{
			SendGuildNotice(guild, g_pLanguageString[_SS_Ally_Canceled], g_pGuild[guild].Name.c_str() , g_pGuild[allyId].Name.c_str());
			SendGuildNotice(allyId, g_pLanguageString[_SS_Ally_Canceled], g_pGuild[guild].Name.c_str(), g_pGuild[allyId].Name.c_str());
			
			g_pGuildAlly[guild] = 0;
		}
	}
	else if(allyId == 0 && ally != 0)
	{
		SendGuildNotice(guild, g_pLanguageString[_SS_Ally_Declared], g_pGuild[guild].Name.c_str(), g_pGuild[ally].Name.c_str());
		SendGuildNotice(ally, g_pLanguageString[_SS_Ally_Declared], g_pGuild[guild].Name.c_str(), g_pGuild[ally].Name.c_str());

		g_pGuildAlly[guild] = ally;
	}

	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(Users[i].Status != USER_PLAY)
			continue;

		if(Mob[i].Mobs.Player.GuildIndex == guild)
			SendWarInfo(i, sServer.CapeWin);
	}
}

INT32 CombineTreasureMap(INT32 clientId)
{
	for(INT32 LOCAL_1 = 0; LOCAL_1 < 60; LOCAL_1++)
	{
		if(Mob[clientId].Mobs.Player.Inventory[LOCAL_1].Index == 788)
		{
			INT32 LOCAL_2 = LOCAL_1;
			LOCAL_2 = LOCAL_1 + 1;

			if(LOCAL_2 >= 60)
				continue;

			if(Mob[clientId].Mobs.Player.Inventory[LOCAL_2].Index != 789)
				continue;

			LOCAL_2 = LOCAL_1 + 9;
			if(LOCAL_2 >= 60)
				break;

			if(Mob[clientId].Mobs.Player.Inventory[LOCAL_2].Index != 790)
				continue;

			LOCAL_2 = LOCAL_1 + 10;
			if(LOCAL_2 >= 60)
				continue;

			if(Mob[clientId].Mobs.Player.Inventory[LOCAL_2].Index != 791)
				continue;

			// 00462654
			LOCAL_2 = LOCAL_1 + 18;
			if(LOCAL_2 >= 60)
				break;

			if(Mob[clientId].Mobs.Player.Inventory[LOCAL_2].Index != 792)
				continue;

			LOCAL_2 = LOCAL_1 + 19;
			if(LOCAL_2 >= 60)
				break;

			if(Mob[clientId].Mobs.Player.Inventory[LOCAL_2].Index != 793)
				continue;

			// envia pacote para DB - 004626BD
			LOCAL_2 = LOCAL_1;

			if(LOCAL_2 >= 60)
				continue;

			memset(&Mob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &Mob[clientId].Mobs.Player.Inventory[LOCAL_2]);

			LOCAL_2 = LOCAL_1 + 1;
			if(LOCAL_2 >= 60)
				continue;

			memset(&Mob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &Mob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 9;
			if(LOCAL_2 >= 60)
				continue;

			memset(&Mob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &Mob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 10;
			if(LOCAL_2 >= 60)
				continue;

			memset(&Mob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &Mob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 18;
			if(LOCAL_2 >= 60)
				continue;

			memset(&Mob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &Mob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			LOCAL_2 = LOCAL_1 + 19;
			if(LOCAL_2 >= 60)
				continue;

			memset(&Mob[clientId].Mobs.Player.Inventory[LOCAL_2], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, LOCAL_2, &Mob[clientId].Mobs.Player.Inventory[LOCAL_2]);
			
			SendClientMessage(clientId, g_pLanguageString[_NN_Treasure_Map]);
			return 1;
		}

		INT32 LOCAL_70 = 0;
		for(LOCAL_1 = 0; LOCAL_1 < 60 ;LOCAL_1++)
		{
			if(Mob[clientId].Mobs.Player.Inventory[LOCAL_1].Index >= 788 && Mob[clientId].Mobs.Player.Inventory[LOCAL_1].Index <= 793)
			{
				memset(&Mob[clientId].Mobs.Player.Inventory[LOCAL_1], 0, sizeof st_Item); // n tem

				Mob[clientId].Mobs.Player.Inventory[LOCAL_1].Index = 485;
				SendItem(clientId, SlotType::Inv, LOCAL_1, &Mob[clientId].Mobs.Player.Inventory[LOCAL_1]);

				LOCAL_70++;
			}
		}

		if(LOCAL_70 > 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Congratulations]);

			return 1;
		}
		
		return 0;
	}

	return true;
}

INT32 PutItemArea(st_Item *item, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
	int count = 0;
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		if (Users[i].Status != USER_PLAY)
			continue;

		if (Mob[i].Target.X >= x1 && Mob[i].Target.Y >= y1 &&
			Mob[i].Target.X <= x2 && Mob[i].Target.Y <= y2)
		{
			count++;
			PutItem(i, item);
		}
	}

	return count;
}

INT32 PutItem(INT32 clientId, st_Item *item)
{
	if(clientId <= 0 || clientId >= MAX_PLAYER)
		return 0;

	if(Users[clientId].Status != USER_PLAY)
		return 0;

	INT32 slot = GetFirstSlot(clientId, 0);
	if(slot != -1)
	{
		memcpy(&Mob[clientId].Mobs.Player.Inventory[slot], item, sizeof st_Item);

		SendItem(clientId, SlotType::Inv, slot, item);
	}
	else
		return 0;

	return 1;
}

void CloseUser(INT32 clientId)
{
	INT32 LOCAL_1 = 0,
	      LOCAL_2 = 0;

	if(Users[clientId].Status == USER_PLAY)
	{
		if(Mob[clientId].Target.X >= 0 && Mob[clientId].Target.X < 4096 && Mob[clientId].Target.Y >= 0 && Mob[clientId].Target.Y < 4096)
			g_pMobGrid[Mob[clientId].Target.Y][Mob[clientId].Target.X] = 0;
	}
	else
	{
		if(Mob[clientId].Target.X >= 0 && Mob[clientId].Target.X < 4096 && Mob[clientId].Target.Y >= 0 && Mob[clientId].Target.Y < 4096 && g_pMobGrid[Mob[clientId].Target.Y][Mob[clientId].Target.X] == clientId)
			g_pMobGrid[Mob[clientId].Target.Y][Mob[clientId].Target.X] = 0;
	}

	//7B3236C = 0; Provavelmente algo admin
	//7B322DC = 0; IsBillConnected

	Users[clientId].AccessLevel				= 0;
	Users[clientId].TokenOk					= 0;
	Users[clientId].inGame.incorrectNumeric = 0;
	Users[clientId].DropEvent.IsValid = false;
	Users[clientId].DropEvent.Dropped = 0; 
	Users[clientId].EventAutoTrade.IsValid = false;
	Users[clientId].Arena.GroupIndex = MAXUINT32;

	// Remove o registro da quest atual
	Users[clientId].QuestAccess = 0;

	std::fill(Users[clientId].Repurchase.Items.begin(), Users[clientId].Repurchase.Items.end(), st_Item{});
	Users[clientId].Repurchase.LastIndex = 0;

	std::fill(Users[clientId].Dropped.Items.begin(), Users[clientId].Dropped.Items.end(), DroppedItem{});
	Users[clientId].Dropped.LastIndex = 0;

	std::fill(Users[clientId].AutoParty.Nicknames.begin(), Users[clientId].AutoParty.Nicknames.end(), "");
	Users[clientId].AutoParty.EnableAll = true;
	Users[clientId].AutoParty.Password = "";
	
	sServer.Zakum.Unregister(clientId);
	std::fill(std::begin(Users[clientId].TimeStamp.Skills), std::end(Users[clientId].TimeStamp.Skills), std::chrono::steady_clock::time_point());

	Users[clientId].invitedUsers.clear();

	Users[clientId].CloseSocket();

	TOD_EventManager::GetInstance().GetEventItem(TOD_EventType::Arena)->Unregister(Users[clientId]);
	TOD_EventManager::GetInstance().GetEventItem(TOD_EventType::HappyHarvest)->Unregister(Users[clientId]);

	INT32 LOCAL_3 = Users[clientId].Status;
	if(LOCAL_3 == USER_EMPTY || LOCAL_3 == USER_ACCEPT)
	{
		Log(clientId, LOG_INGAME, "Usuário desconectado com status %d", LOCAL_3);
		Users[clientId].CloseUser_OL1();

		Users[clientId].NormalLog.reset();
		Users[clientId].HackLog.reset();
		Users[clientId].User.Username[0] = '\0';
		return;
	}
	else if(LOCAL_3 == USER_PLAY || LOCAL_3 == USER_SAVING4QUIT)
	{
		Log(clientId, LOG_INGAME, "Personagem foi desconectado. Permaneceu %02d:%02d:%02d minutos online",
			(Users[clientId].Time / 3600) % 24, (Users[clientId].Time / 60) % 60, Users[clientId].Time % 60);

		LogPlayer(clientId, "Personagem desconectado. Permaneceu %02d:%02d:%02d online",
			(Users[clientId].Time / 3600) % 24, (Users[clientId].Time / 60) % 60, Users[clientId].Time % 60);

		LogGold(clientId);

		INT32 LOCAL_4 = Users[clientId].Trade.ClientId;
		if(LOCAL_4 > 0 && LOCAL_4 < MAX_PLAYER && Users[LOCAL_4].Status == USER_PLAY && Users[LOCAL_4].Trade.ClientId == clientId)
			RemoveTrade(LOCAL_4);

		Users[clientId].Trade.ClientId = 0;

		stSaveChar packet;
		memset(&packet, 0, sizeof packet);

		if (clientId <= 0 || clientId >= MAX_PLAYER)
		{
			Users[clientId].NormalLog.reset();
			Users[clientId].HackLog.reset();
			return;
		}
		
		packet.CharSlot = Users[clientId].inGame.CharSlot;
		if (packet.CharSlot < 0 || packet.CharSlot >= 4)
		{
			Users[clientId].NormalLog.reset();
			Users[clientId].HackLog.reset();
			return;
		}
		
		packet.Header.PacketId = 0x806;
		memcpy(&packet.Storage, &Users[clientId].User.Storage.Item, sizeof st_Item * 128);
		memcpy(packet.SkillBar, Mob[clientId].Mobs.Player.SkillBar1, 4);
		memcpy(&packet.SkillBar[4], Mob[clientId].Mobs.SkillBar, 16);

		strncpy_s(packet.Pass, Users[clientId].User.Block.Pass, 16);
		strncpy_s(packet.User, Users[clientId].User.Username, 16);
		packet.Header.ClientId = clientId;
		
		packet.BanType = Users[clientId].User.BanType;
		memcpy(&packet.Ban, &Users[clientId].User.Ban, sizeof stDate);

		packet.Coin = Users[clientId].User.Storage.Coin;
		packet.Cash = Users[clientId].User.Cash;
		packet.Blocked = Users[clientId].User.Block.Blocked;

		memcpy(&packet.Mob, &Mob[clientId].Mobs, sizeof stCharInfo);
		memcpy(&packet.Friends, &Users[clientId].User.Friends, 30 * 16);
		
		packet.Insignia = Users[clientId].User.Insignias.Value;

		packet.Slot = -1;

		packet.Unique = Users[clientId].User.Unique.Value;
		packet.Position.X = Mob[clientId].Target.X;
		packet.Position.Y = Mob[clientId].Target.Y;
		
		packet.Daily.WeekYear  = Users[clientId].User.Daily.WeekYear;
		
		packet.Water.Day = Users[clientId].User.Water.Day;
		packet.Water.Total = Users[clientId].User.Water.Total;

		packet.Divina = Users[clientId].User.Divina;
		packet.Sephira = Users[clientId].User.Sephira;

		packet.SingleGift = Users[clientId].User.SingleGift;
		memcpy(&packet.Daily.Received[0], Users[clientId].User.Daily.Received, sizeof(Users[clientId].User.Daily.Received));

		AddMessageDB((BYTE*)&packet, sizeof stSaveChar);
		Users[clientId].Status = USER_SAVING4QUIT;

		DeleteMob(clientId, 2);
		RemoveParty(clientId);
	}
	else
	{
		Log(clientId, LOG_INGAME, "Personagem foi desconectado. Permaneceu %02d:%02d:%02d minutos online",
			(Users[clientId].Time / 3600) % 24, (Users[clientId].Time / 60) % 60, Users[clientId].Time % 60);

		LogPlayer(clientId, "Personagem desconectado. Permaneceu %02d:%02d:%02d online",
			(Users[clientId].Time / 3600) % 24, (Users[clientId].Time / 60) % 60, Users[clientId].Time % 60);

		pMsgSignal packet;
		packet.Header.PacketId = 0x805;
		packet.Header.ClientId = clientId;

		AddMessageDB((BYTE*)&packet, 12);

		Mob[clientId].Mode = 0;
		Users[clientId].CloseUser_OL1();
	}

	Users[clientId].IsBanned = false;
	Users[clientId].IsAutoTrading				= false;
	
	Users[clientId].Socket.Error				= 0;
	Users[clientId].AccessLevel					= 0;

	// Zera o acesso a quest
	Users[clientId].QuestAccess					= 0;
	Users[clientId].WolfEggEquipedTime			= 0;
	Users[clientId].WolfEquipedTime				= 0;

	// Apaga o SND do usuário
	Users[clientId].SNDMessage[0]				= 0;

	// Apaga a última pessoa que o usuário conversou
	Users[clientId].LastWhisper					= 0;
	Mob[clientId].Tab[0]						= 0;

	// Zera o tempo que o usuário está online
	Users[clientId].CharLoginTime				= 0;

	Users[clientId].TimeStamp.LastReceiveTime   = 0;
	Users[clientId].TimeStamp.LastAttack		= 0;
	Users[clientId].TimeStamp.TimeStamp			= 0;
	Users[clientId].inGame.incorrectNumeric		= 0;
	Users[clientId].hashIncrement				= 0;

	Users[clientId].Time = 0;

	memset(Users[clientId].Keys, 0, 16);
	memset(Users[clientId].MacAddress, 0, 8);

	Users[clientId].WolfEggEquipedTime			= 0;
	Users[clientId].WolfEquipedTime				= 0;
	Users[clientId].WolfTotalTime				= 0;
	Users[clientId].AlphaPotionRewardCounter	= 0;

	// :) (: 
	Users[clientId].User.BanType				= 0;
	Users[clientId].User.Unique.Value			= 0;

	memset(&Users[clientId].User.Ban, 0, sizeof stDate);
	
	Users[clientId].aHack.Response = 0; // aguardando
	Users[clientId].aHack.Question = 0;
	Users[clientId].aHack.Next     = 0;
	Users[clientId].aHack.Last     = 0;
	Users[clientId].aHack.Error    = 0;

	Users[clientId].PremierStore.Status = 0;
	Users[clientId].PremierStore.Time   = 0;
	Users[clientId].PremierStore.Wait   = 0;
	Users[clientId].PremierStore.Count  = 0;

	Users[clientId].User.Daily.WeekYear  = 0;

	memset(Users[clientId].User.Daily.Received, 0, sizeof Users[clientId].User.Daily.Received);

	Users[clientId].User.Water.Day		= 0;
	Users[clientId].User.Water.Total	= 0;

	Users[clientId].MacIntegrity.IsChecked = false;
	Users[clientId].MacIntegrity.WasWarned = false;

	Users[clientId].User = stAccount{};

	auto colosseum = std::find(sServer.Colosseum.clients.begin(), sServer.Colosseum.clients.end(), clientId);
	if (colosseum != sServer.Colosseum.clients.end())
		sServer.Colosseum.clients.erase(colosseum);

	Users[clientId].Christmas.Mission.MissionId = -1;
	std::fill(std::begin(Users[clientId].Christmas.Mission.Count), std::end(Users[clientId].Christmas.Mission.Count), 0);
	Users[clientId].Christmas.Mission.Status = TOD_ChristmasMission_Status::WaitingNextRound;

	TOD_EventManager::GetInstance().GetEventItem(TOD_EventType::Arena)->Unregister(Users[clientId]);

	for (int i = 0; i < MAX_PLAYER; i++)
	{
		if (sServer.Zombie.Registered[i] == clientId)
		{
			sServer.Zombie.Registered[i] = 0;
			break;
		}
	}

	for(int i = 0; i < 3; ++i)
	{
		auto it = std::find(std::begin(sServer.Nightmare[i].Members), std::end(sServer.Nightmare[i].Members), clientId);
		if (it != std::end(sServer.Nightmare[i].Members))
		{
			*it = 0;
			Log(clientId, LOG_INGAME, "Removido do registro do Pesadelo");
		}
	}

	// Remove o usuário da pista se estiver cadastrado
	for (int ref = 0; ref < 6; ref++)
	{
		stPista *pista = &pPista[ref];

		for (int party = 0; party < MAX_PARTYPISTA; party++)
		{
			for (int member = 0; member < 13; member++)
			{
				if (pista->Clients[party][member] == clientId)
				{
					pista->Clients[party][member] = 0;

					party = MAX_PARTYPISTA;
					ref = 6;

					Log(clientId, LOG_INGAME, "O usuário foi removido da pista + %d pois foi desconectado", ref);
					break;
				}
			}
		}
	}

	Users[clientId].NormalLog.reset();
	Users[clientId].HackLog.reset();
}

void FinishCastleWar()
{
	Log(SERVER_SIDE, LOG_INGAME, "Guerra de Noatun finalizada. CastleState: 4");

	SetCastleDoor(1);
	ClearAreaGuild(0x40C, 0x688, 0x478, 0x6E4, ChargedGuildList[sServer.Channel - 1][4]);

	// Anteriormente o valor era 0
	// Agora seta como 4 para ser possível o recolhimento dos impostos
	sServer.CastleState = 4;

	INT32 LOCAL_1 = 1;
	for(; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if(Users[LOCAL_1].Status != USER_PLAY)
			continue;

		SendSignal(LOCAL_1, 0x7530, 0x3AC);
	}

	for(LOCAL_1 = 1; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if(Users[LOCAL_1].Status != USER_PLAY)
			continue;

		SendSignalParm(LOCAL_1, 0x7530, 0x3AC, 0); // 0 = sServer.CastleState
	}

	for(LOCAL_1 = MAX_PLAYER; LOCAL_1 < MAX_SPAWN_MOB; LOCAL_1 ++)
	{
		if(Mob[LOCAL_1].Mode == 0)
			continue;

		if(Mob[LOCAL_1].Mobs.Player.Equip[0].Index == 219)
			DeleteMob(LOCAL_1, 2);
	}

	sServer.AltarId = 0;
	ClearArea(0x40C, 0x688, 0x478, 0x6E4);
	GuildZoneReport();
	
	for(INT32 i = 0; i < 4096; i++)
	{
		if(pInitItem[i].Item.Index <= 0 || pInitItem[i].Item.Index >= MAX_ITEMLIST)
			continue;

		if(pInitItem[i].Item.Index >= 3145 && pInitItem[i].Item.Index <= 3149)
		{
			pInitItem[i + 4].Item.Index = 3145 + g_pCityZone[4].win_count;
			
			pInitItem[i + 4].Item.EF1 = 56;
			pInitItem[i + 4].Item.EFV1 = ChargedGuildList[sServer.Channel - 1][4] / 257;

			pInitItem[i + 4].Item.EF2 = 57;
			pInitItem[i + 4].Item.EFV2 = ChargedGuildList[sServer.Channel - 1][4];

			pInitItem[i + 4].Item.EF3 = 59;
			pInitItem[i + 4].Item.EFV3 = Rand() % 255;

			Log(SERVER_SIDE, LOG_INGAME, "Setado o indice da torre de Noatun para %d. Index: %hu", i + 4, pInitItem[i + 4].Item.Index);
			break;
		}
	}

	// Reseta a guerra de torres : ) 
	sServer.TowerWar.Guild = 0;

	for(INT32 i = 0; i < MAX_GUILD; i++)
	{
		if(g_pGuild[i].Name.empty())
			continue;

		SetGuildWin(i, 0);
	}
}

void DecideChallenger()
{
	for (auto cityIt = std::begin(sServer.ChallengerMoney); cityIt != std::end(sServer.ChallengerMoney); ++cityIt)
	{
		int cityIndex = std::distance(std::begin(sServer.ChallengerMoney), cityIt);
		auto& cityZone = g_pCityZone[cityIndex];
		auto& city = *cityIt;

		if (ChargedGuildList[sServer.Channel - 1][cityIndex] != 0)
		{
			auto& city = *cityIt;
			auto maxIt = std::max_element(std::begin(city), std::end(city), [](const ChallengeInfo& a, const ChallengeInfo& b) {
				return a.Value < b.Value;
			});

			if (maxIt == std::end(city))
				Log(SERVER_SIDE, LOG_INGAME, "Não houve desafiantes na cidade de %s", szCitys[cityIndex]);
			else
			{
				const auto& guild = *maxIt;
				g_pCityZone[cityIndex].chall_index = guild.GuildId;

				Log(SERVER_SIDE, LOG_INGAME, "%s será o desafiante da cidade %s", g_pGuild[guild.GuildId].Name.c_str(), szCitys[cityIndex]);
			}
		}
		else
		{
			for (int i = 0; i < 2; ++i)
			{
				auto maxIt = std::max_element(std::begin(city), std::end(city), [](const ChallengeInfo& a, const ChallengeInfo& b) {
					return a.Value < b.Value;
				});

				if (maxIt == std::end(city))
				{
					if(i == 0)
						Log(SERVER_SIDE, LOG_INGAME, "Não houve desafiantes na cidade de %s", szCitys[cityIndex]);
					else 
						Log(SERVER_SIDE, LOG_INGAME, "Não houve um segundo desafiante na cidade de %s", szCitys[cityIndex]);

					continue;
				}

				const auto& guild = *maxIt;

				if (i == 0)
					g_pCityZone[cityIndex].chall_index = guild.GuildId;
				else
					g_pCityZone[cityIndex].chall_index_2 = guild.GuildId;

				Log(SERVER_SIDE, LOG_INGAME, "%s será o desafiante da cidade %s (%d)", g_pGuild[guild.GuildId].Name.c_str(), szCitys[cityIndex], i);
				city.erase(maxIt);
			}
		}
	}
}

void GuildProcess()	
{
	DoColosseum();

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);
	
	if(now.tm_wday >= SEGUNDA && now.tm_wday <= SEXTA && (now.tm_hour == 20 || now.tm_hour == 15) && sServer.NoviceChannel)
	{
		if((now.tm_min == 0 || now.tm_min == 30) && !sServer.Colosseum.inSec.closedGate)
		{
			sServer.Colosseum.Type = now.tm_min == 0 ? TOD_Colosseum_Type::Normal : TOD_Colosseum_Type::Mystic;

			sServer.Colosseum.inSec.closedGate = true;
			SetColoseumDoor(1);
			
			if(sServer.Colosseum.Type == TOD_Colosseum_Type::Normal)
				SendNotice("!Coliseu Normal será iniciado em 3 minutos. Disponível apenas para mortais.");
			else if (sServer.Colosseum.Type == TOD_Colosseum_Type::Mystic)
				SendNotice("!Coliseu Místico será iniciado em 3 minutos. Disponível para Arch ou superior");

			SendNotice("!Localização: Cidade de Arzan. 2615x 1724y");

			Log(SERVER_SIDE, LOG_INGAME, "Coliseu será iniciado em 3 minutos");
		}
		else if((now.tm_min == 3 || now.tm_min == 33) && !sServer.Colosseum.inSec.closedWalls)
		{
			Log(SERVER_SIDE, LOG_INGAME, "Coliseu foi iniciado");
			sServer.Colosseum.inSec.closedWalls = true;

			// Seta como o 1° nível
			sServer.Colosseum.level = 1;
			
			SetColoseumDoor(3);
			SetColoseumDoor2(1);
			
			// Seta 5 minutos
			sServer.Colosseum.time = 300; 
			sServer.Colosseum.inSec.wasBorn = true;

			for(int i = 0; i < 9; i++)
				GenerateMob(COLOSSEUM_ID + 4 + i, 0, 0);

			for(int i = 1; i < MAX_PLAYER; i++)
			{
				if(Users[i].Status != USER_PLAY)
					continue;

				if (Mob[i].Target.X >= 2604 && Mob[i].Target.X <= 2650 && Mob[i].Target.Y >= 1708 && Mob[i].Target.Y <= 1748)
				{
					if (Mob[i].Mobs.Player.Equip[0].EFV2 != MORTAL && sServer.Colosseum.Type == TOD_Colosseum_Type::Normal)
					{
						DoRecall(i);

						SendClientMessage(i, "Somente mortais podem participar do Coliseu");
						Log(i, LOG_INGAME, "Removido da área de Coliseu por não ser mortal");
						continue;
					}
					
					if (Mob[i].Mobs.Player.Equip[0].EFV2 < ARCH && sServer.Colosseum.Type == TOD_Colosseum_Type::Mystic)
					{
						DoRecall(i);

						SendClientMessage(i, "Somente archs ou superior podem participar do Coliseu");
						Log(i, LOG_INGAME, "Removido da área de Coliseu por não ser Arch");
						continue;
					}

					int sameComputerId = -1;
					for (const auto clientId : sServer.Colosseum.clients)
					{
						if (memcmp(&Users[clientId].MacAddress, Users[i].MacAddress, 8) == 0)
						{
							sameComputerId = i;
							break;
						}
					}
					
					if (sameComputerId != -1)
					{
						SendClientMessage(i, "Somente uma conta por computador");

						Log(i, LOG_INGAME, "O usuário não foi registrado no Coliseu por já ter uma conta registrada. Conta: %s", Users[sameComputerId].User.Username);
						continue;
					}

					SendClientMessage(i, "Coliseu foi iniciado");
					SendSignalParm(i, SERVER_SIDE, 0x3A1, sServer.Colosseum.time);

					sServer.Colosseum.clients.push_back(i);

					Log(i, LOG_INGAME, "Registrado no coliseu. %ux %uy.", Mob[i].Target.X, Mob[i].Target.Y);
				}
			}
		}
		else if (sServer.Colosseum.level != 0)
		{
			for (int i = 1; i < MAX_PLAYER; i++)
			{
				if (Users[i].Status != USER_PLAY || Users[i].IsAdmin)
					continue;

				if (Mob[i].Target.X >= 2608 && Mob[i].Target.X <= 2647 && Mob[i].Target.Y >= 1708 && Mob[i].Target.Y <= 1748)
				{
					if (std::find(sServer.Colosseum.clients.cbegin(), sServer.Colosseum.clients.cend(), i) == sServer.Colosseum.clients.cend())
						DoRecall(i);
				}
			}
		}
	}

	if(sServer.WarChannel == 1 && now.tm_hour == sServer.CastleHour && now.tm_wday == DOMINGO)// && now.tm_wday == 0) // now.tm_hour == 19
	{
		if(sServer.CastleState == 0 && now.tm_min == 0)
		{
			SendNotice(g_pLanguageString[_DN_Castle_will_be_open]);

			sServer.CastleState = 1;
			for(INT32 LOCAL_13 = 1; LOCAL_13 < MAX_PLAYER; LOCAL_13++)
			{
				if(Users[LOCAL_13].Status != USER_PLAY)
					continue;

				SendSignalParm(LOCAL_13, 0x7530, 0x3AC, sServer.CastleState);
			}

			Log(SERVER_SIDE, LOG_INGAME, "Portões vão fechar em 5 minutos. CastleState: 1");
		}
		else if(sServer.CastleState == 1 && now.tm_min == 5)
		{
			ClearAreaGuild(0x40C, 0x688, 0x478, 0x6E4, ChargedGuildList[sServer.Channel - 1][4]);

			ClearAreaTeleport(0x0469, 0x6A9, 0x469, 0x6AD, 0x421, 0x6CE);
			ClearAreaTeleport(0x045C, 0x6A9, 0x45C, 0x6AD, 0x421, 0x6CE);
			ClearAreaTeleport(0x0446, 0x698, 0x446, 0x69C, 0x421, 0x6CE);
			ClearAreaTeleport(0x043F, 0x649, 0x43F, 0x6B1, 0x421, 0x6CE);
			ClearAreaTeleport(0x041A, 0x69A, 0x41A, 0x69A, 0x421, 0x6CE);
			ClearAreaTeleport(0x0416, 0x69A, 0x417, 0x49B, 0x421, 0x6CE);
			ClearAreaTeleport(0x0464, 0x6AC, 0x464, 0x6AC, 0x421, 0x6CE);

			SetCastleDoor(3);

			for(INT32 LOCAL_14 = 0; LOCAL_14 < 3; LOCAL_14 ++)
			{
				GenerateMob(LOCAL_14 + TORRE_NOATUN, 0, 0);

				sServer.LiveTower[LOCAL_14] = 1;
			}
			
			SendNotice(g_pLanguageString[_DN_Castle_opened]);
			sServer.CastleState = 2;
			
			Log(SERVER_SIDE, LOG_INGAME, "Guerra iniciada. CastleState: 2");
		}
		else if(sServer.CastleState == 2 && now.tm_min == 50)
		{
			SendNotice(g_pLanguageString[_DN_Castle_will_be_closed]);

			sServer.CastleState = 3;
			Log(SERVER_SIDE, LOG_INGAME, "Guerra irá acabar em 5minutos. CastleState: 3");
		}
		else if(sServer.CastleState == 3 && now.tm_min >= 55)
		{
			SendNotice(g_pLanguageString[_DN_Castle_closed]);

			FinishCastleWar();
		}

		if((sServer.CastleState == 2 || sServer.CastleState == 3) && now.tm_min < 55)
		{
			INT32 User = g_pMobGrid[1690][1046];
			
			if(User != sServer.AltarId)
			{
				if (User > 0 && User < MAX_PLAYER && Mob[User].Mobs.Player.GuildMemberType == 9)
				{
					INT32 guildId = Mob[User].Mobs.Player.GuildIndex;

					bool any = false;
					for(INT32 i = 0; i < 4; i++)
					{
						if(ChargedGuildList[sServer.Channel - 1][i] == guildId)
							any = true;
					}

					if(ChargedGuildList[sServer.Channel - 1][4] == guildId)
						SendClientMessage(User, "Você já é dono do Castelo");
					else if(any)
					{
						if (sServer.AltarId > 0 && sServer.AltarId < MAX_PLAYER)
							Users[sServer.AltarId].TimerCount = 0;

						sServer.AltarId = User;
						Users[User].TimerCount = 0;
					}
					else
						SendClientMessage(User, "Para conquistar o Castelo você deve possuir pelo menos uma cidade");
				}
				else if(Mob[User].Mobs.Player.GuildMemberType != 9 && User > 0 && User < MAX_PLAYER)
				{
					SendClientMessage(User, "Somente o líder da guilda pode conquistar o Castelo");
					sServer.AltarId = 0;
				}
				else
					sServer.AltarId = 0;
			}

			if(sServer.AltarId != 0)
			{
				if(Users[User].TimerCount == 0)
					SendNotice(g_pLanguageString[_SN_S_is_charging_castle], Mob[User].Mobs.Player.Name);

				p3AD LOCAL_5;
				LOCAL_5.Header.PacketId = 0x3AD;
				LOCAL_5.Header.Size = 16;
				LOCAL_5.User = User;
				LOCAL_5.Unknow = 1;

				GridMulticast_2(Mob[User].Target.X, Mob[User].Target.Y, (BYTE*)&LOCAL_5, 0);

				Users[User].TimerCount ++;

				INT32 GuildID = Mob[User].Mobs.Player.GuildIndex;
				if(Users[User].TimerCount > 180)
				{
					SendNotice(g_pLanguageString[_SN_S_charge_castle], Mob[User].Mobs.Player.Name);

					// Seta o novo líder da guild
					ChargedGuildList[sServer.Channel - 1][4] = GuildID;
					
					// Como houve uma nova pessoa conquistando, a contagem de Vitórias 
					// é zerada no ato ^^^ 
					g_pCityZone[4].win_count = 0;
					
					sServer.CapeWin = Mob[User].Mobs.Player.CapeInfo;

					FinishCastleWar();

					INT32 LOCAL_6 = 1;
					for(; LOCAL_6 < MAX_PLAYER; LOCAL_6++)
						ClearCrown(LOCAL_6); //remove a Coroa de quem estiver online do jogo
				}
			}
		}
	}

	// Ao terminar a guerra, sServer.CastleState é setado como 4
	// Quando está setado como 4, o líder da guild dono de noatun pode efetuar
	// o recolhimento dos impostos. Caso continue 4 as 21h quer dizer que o mesmo
	// não recolheu e será setado automaticamente como 0
	if(sServer.WarChannel == 1 && now.tm_hour == sServer.CastleHour + 1 && now.tm_min >= 30 && now.tm_wday == DOMINGO && sServer.CastleState != 0)
	{
		sServer.CastleState = 0;

		Log(SERVER_SIDE, LOG_INGAME, "Setado CastleState = 0");
	}

	// 004585E6
	if(now.tm_hour == sServer.NewbieHour && now.tm_min == 54)
		sServer.PotionReady = 0;
	
	if(now.tm_hour == sServer.NewbieHour && now.tm_min == 55 && sServer.PotionReady == 0)
	{
		GenerateMob(22, 0, 0);

		sServer.PotionReady = 1;
	}

	INT32 LOCAL_24 = 5;
	// 0045864E
	//if(sServer.ForceWeekDay != -1)
	//	now.tm_wday = -1;

	if(sServer.WeekMode == 0)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour) || sServer.ForceWeekDay == 1)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice1]);

			sServer.WeekMode = 1;
			return;
		}
	}

	// 04586A2
	if(sServer.WeekMode == 1)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour && now.tm_min == 3) || sServer.ForceWeekDay == 2)
		{
			ClearGuildPKZone();
			sServer.WeekMode = 2;

			INT32 LOCAL_25 = 0;
			for(; LOCAL_25 < 5; LOCAL_25 ++)
			{
				if(LOCAL_25 == 4)
					continue;

				if(ChargedGuildList[sServer.Channel - 1][LOCAL_25] == 0 && g_pCityZone[LOCAL_25].chall_index_2 == 0)
					continue;

				INT32 guildId = ChargedGuildList[sServer.Channel - 1][LOCAL_25];
				if (guildId == 0)
					guildId = g_pCityZone[LOCAL_25].chall_index_2;

				INT32 total = 0;
				for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
				{
					if(Users[LOCAL_2].Status != USER_PLAY || Users[LOCAL_2].IsAutoTrading)
						continue;

					if(Mob[LOCAL_2].Mode == 0)
						continue;

					if(Mob[LOCAL_2].Mobs.Player.GuildIndex != guildId)
						continue;
		
					INT32 LOCAL_3 = GetVillage(Mob[LOCAL_2].Target.X, Mob[LOCAL_2].Target.Y);
					if (LOCAL_3 != LOCAL_25)
					{
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por estar fora da cidade. Posição: %u %u", szCitys[LOCAL_25], Mob[LOCAL_2].Target.X, Mob[LOCAL_2].Target.Y);

						continue;
					}

					if ((LOCAL_25 == 0 || LOCAL_25 == 1) && Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por não ser celestial", szCitys[LOCAL_25]);

						continue;
					}
					if (LOCAL_25 == 2 && Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != MORTAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por estar fora não ser mortal", szCitys[LOCAL_25]);

						continue;
					}

					if (LOCAL_25 == 3 && Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != ARCH)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por estar fora não ser arch", szCitys[LOCAL_25]);

						continue;
					}

					Mob[LOCAL_2].Lifes = 5;

					total++;
					Teleportar(LOCAL_2, g_pCityZone[LOCAL_25].guilda_war_x, g_pCityZone[LOCAL_25].guilda_war_y);
					SendScore(LOCAL_2);

					Log(SERVER_SIDE, LOG_GUILD, "Teleportado %s (%u-%hhu) para a guerra de %s - Contagem: %d - DEFENSORA", Mob[LOCAL_2].Mobs.Player.Name, Mob[LOCAL_2].Mobs.Player.bStatus.Level,
						Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2, szCitys[LOCAL_25], total);

					if(total >= 26)
						break;
				}
			}

			for(LOCAL_25 = 0; LOCAL_25 < 5; LOCAL_25 ++)
			{
				if(LOCAL_25 == 4)
					continue;

				if(g_pCityZone[LOCAL_25].chall_index == 0)
					continue;

				INT32 guildId = g_pCityZone[LOCAL_25].chall_index;
				INT32 total = 0;
				for(INT32 LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
				{
					if(Users[LOCAL_2].Status != USER_PLAY || Users[LOCAL_2].IsAutoTrading)
						continue;

					if(Mob[LOCAL_2].Mode == 0)
						continue;

					if(Mob[LOCAL_2].Mobs.Player.GuildIndex != guildId)
						continue;
		
					INT32 LOCAL_3 = GetVillage(Mob[LOCAL_2].Target.X, Mob[LOCAL_2].Target.Y);
					if (LOCAL_3 != LOCAL_25)
					{
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por estar fora da cidade. Posição: %u %u", szCitys[LOCAL_25], Mob[LOCAL_2].Target.X, Mob[LOCAL_2].Target.Y);

						continue;
					}

					if ((LOCAL_25 == 0 || LOCAL_25 == 1) && Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por não ser celestial", szCitys[LOCAL_25]);

						continue;
					}

					if (LOCAL_25 == 2 && Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != MORTAL)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por estar fora não ser mortal", szCitys[LOCAL_25]);

						continue;
					}

					if (LOCAL_25 == 3 && Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2 != ARCH)
					{
						SendClientMessage(LOCAL_2, g_pLanguageString[_NN_3rd_village_limit]);
						Log(LOCAL_2, LOG_INGAME, "Não teleportado para guerra de %s por estar fora não ser arch", szCitys[LOCAL_25]);

						continue;
					}

					Mob[LOCAL_2].Lifes = 5;

					total++;
					Teleportar(LOCAL_2, g_pCityZone[LOCAL_25].guildb_war_x, g_pCityZone[LOCAL_25].guildb_war_y);

					Log(SERVER_SIDE, LOG_GUILD, "Teleportado %s (%u-%hhu) para a guerra de %s - Contagem: %d - DESAFIADOR", Mob[LOCAL_2].Mobs.Player.Name, Mob[LOCAL_2].Mobs.Player.bStatus.Level,
						Mob[LOCAL_2].Mobs.Player.Equip[0].EFV2, szCitys[LOCAL_25], total);

					if(total >= 26)
						break;
				}
			}

			return;
		}
	}
	else if(sServer.WeekMode == 2)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour && now.tm_min == 6) || sServer.ForceWeekDay == 3)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice3]);

			sServer.WeekMode = 3;
			SetArenaDoor(1);
		}
	}
	else if(sServer.WeekMode == 3)
	{
		if((now.tm_wday == DOMINGO && now.tm_hour >= sServer.WeekHour && now.tm_min >= 30) || sServer.ForceWeekDay == 4)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice4]);

			DecideWinner();
			ClearGuildPKZone();
			ClearChallanger();
			SaveGuildZone();
			SetArenaDoor(3);

			sServer.WeekMode = 4;
			GuildZoneReport();
		}
	}
	else if(sServer.WeekMode == 4)
	{
		if(now.tm_wday == SABADO || sServer.ForceWeekDay == 5)
		{
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice5]);

			sServer.WeekMode = 5;
		}
	}
	else if(sServer.WeekMode == 5)
	{
		if(now.tm_wday == DOMINGO || sServer.ForceWeekDay == 0)
		{
			DecideChallenger();
			SendNotice(g_pLanguageString[_NN_Guild_Battle_Notice6]);

			sServer.WeekMode = 0;
		}
	}
}

void ClearAreaGuild(unsigned int minPosX, unsigned int minPosY, unsigned int maxPosX, unsigned int maxPosY, int guildId)
{
	for(size_t i = 1; i < MAX_PLAYER; i++)
	{
		if(Users[i].Status != USER_PLAY)
			continue;

		if(Mob[i].Target.X < minPosX || Mob[i].Target.X > maxPosX || Mob[i].Target.Y < minPosY || Mob[i].Target.Y > maxPosY)
			continue;

		if(Mob[i].Mobs.Player.GuildIndex == guildId && guildId == 0)
			continue;

		DoRecall(i);
	}
}

void SetCastleDoor(int mode)
{
	for(INT32 i = 0; i < 4; i++)
	{
		INT32 LOCAL_2 = i + 33;

		if(pInitItem[LOCAL_2].Item.Index <= 0 || pInitItem[LOCAL_2].Item.Index >= MAX_ITEMLIST)
			continue;

		if(pInitItem[LOCAL_2].Status != mode)
		{
			INT32 LOCAL_3;
			UpdateItem(LOCAL_2, mode, &LOCAL_3);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_2 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = pInitItem[LOCAL_2].Status;
			LOCAL_150.unknow = LOCAL_3;

			GridMulticast_2(pInitItem[LOCAL_2].PosX, pInitItem[LOCAL_2].PosY, (BYTE*)&LOCAL_150, 0);

			pInitItem[LOCAL_2].IsOpen = 0;
		}
	}
}

void SetColoseumDoor(int mode)
{
	for(INT32 i = 0; i < 2; i++)
	{
		INT32 LOCAL_2 = i + 13;

		if(pInitItem[LOCAL_2].Item.Index <= 0 || pInitItem[LOCAL_2].Item.Index >= MAX_ITEMLIST)
			continue;

		if(pInitItem[LOCAL_2].Status != mode)
		{
			INT32 LOCAL_3;
			UpdateItem(LOCAL_2, mode, &LOCAL_3);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_2 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = pInitItem[LOCAL_2].Status;
			LOCAL_150.unknow = (mode == 3) ? 18 : 0;

			GridMulticast_2(pInitItem[LOCAL_2].PosX, pInitItem[LOCAL_2].PosY, (BYTE*)&LOCAL_150, 0);

			pInitItem[LOCAL_2].IsOpen = 0;
		}
	}
}

void SetColoseumDoor2(int mode)
{
	for(INT32 i = 0; i < 5; i++)
	{
		INT32 LOCAL_2 = i + 15;

		if(pInitItem[LOCAL_2].Item.Index <= 0 || pInitItem[LOCAL_2].Item.Index >= MAX_ITEMLIST)
			continue;

		if(pInitItem[LOCAL_2].Status != mode)
		{
			INT32 LOCAL_3;
			UpdateItem(LOCAL_2, mode, &LOCAL_3);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_2 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = pInitItem[LOCAL_2].Status;
			LOCAL_150.unknow = (mode == 3) ? 18 : 0;

			GridMulticast_2(pInitItem[LOCAL_2].PosX, pInitItem[LOCAL_2].PosY, (BYTE*)&LOCAL_150, 0);

			pInitItem[LOCAL_2].IsOpen = 0;
		}
	}
}

void ClearAreaLevel(unsigned int minPosX, unsigned int minPosY, unsigned int maxPosX, unsigned int maxPosY, unsigned int min_level, unsigned int max_level)
{
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(Users[i].Status != USER_PLAY)
			continue;

		if(Mob[i].Mode == 0)
			continue;

		if(Mob[i].Target.X < minPosX || Mob[i].Target.X > maxPosX || Mob[i].Target.Y < minPosY || Mob[i].Target.Y > maxPosY)
			continue;

		if(Mob[i].Mobs.Player.bStatus.Level < static_cast<int>(min_level) || Mob[i].Mobs.Player.bStatus.Level > static_cast<int>(max_level))
			continue;

		if (Mob[i].Mobs.Player.bStatus.Level >= 1010 || Users[i].AccessLevel != 0)
			continue;

		DoRecall(i);
	}
}

void GenerateColoseum(int i)
{
	INT32 LOCAL_1 = Rand() & 0x80000003 + 4;

	for(INT32 i = 0; i < LOCAL_1; i++)
		GenerateMob(i, 0, 0);
}

void ClearGuildPKZone()
{
	UINT32 LOCAL_1 = 0x80,
		  LOCAL_2 = 0x80,
		  LOCAL_3 = 0x100,
		  LOCAL_4 = 0x100,
		  i = 0x1;

	for(; i < MAX_PLAYER; i++)
	{
		if(Users[i].Status != USER_PLAY)
			continue;

		if(Mob[i].Mode == 0)
			continue;

		if(Mob[i].Target.X < LOCAL_1 || Mob[i].Target.X > LOCAL_3 || Mob[i].Target.Y < LOCAL_2 || Mob[i].Target.Y > LOCAL_4)
			continue;

		DoRecall(i);
	}
}

void SetArenaDoor(int mode)
{
	for(INT32 i = 0 ; i < 5; i++)
	{
		if(i == 4)
			continue;

		for(INT32 x = 0; x < 3; x++)
		{
			INT32 LOCAL_3 = i * 3 + x + 1;

			if(pInitItem[LOCAL_3].Item.Index <= 0 || pInitItem[LOCAL_3].Item.Index >= MAX_ITEMLIST)
				continue;

			INT32 LOCAL_4;
			UpdateItem(LOCAL_3, mode, &LOCAL_4);

			p374 LOCAL_150;
			LOCAL_150.Header.PacketId = 0x374;
			LOCAL_150.Header.ClientId = 0x7530;
			LOCAL_150.gateId = LOCAL_3 + 10000;
			LOCAL_150.Header.Size = 20;
			LOCAL_150.status = pInitItem[LOCAL_3].Status;
			LOCAL_150.unknow = LOCAL_4;

			GridMulticast_2(pInitItem[LOCAL_3].PosX, pInitItem[LOCAL_3].PosY, (BYTE*)&LOCAL_150, 0);

			pInitItem[LOCAL_3].IsOpen = 0;
		}
	}
}

void DecideWinner()
{
	for(INT32 i = 0; i < 5; i++)
	{
		if(i == 4)
			continue;

		INT32 LOCAL_2 = 0,
			  LOCAL_3 = 0,
			  LOCAL_4 = ChargedGuildList[sServer.Channel - 1][i],
			  LOCAL_5 = g_pCityZone[i].chall_index;

		if (LOCAL_4 == 0)
			LOCAL_4 = g_pCityZone[i].chall_index_2;

		if(LOCAL_5 == 0)
		{
			g_pCityZone[i].win_count++;

			if (g_pCityZone[i].win_count >= 5)
				g_pCityZone[i].win_count = 4;
			continue;
		}

		std::stringstream strOwner;
		strOwner << "[" << szCitys[i] << "] - Guild campeã informações:\n";

		std::stringstream strChall;
		strChall << "[" << szCitys[i] << "] - Guild desafiadora informações:\n";

		for(UINT32 LOCAL_134 = g_pCityZone[i].war_min_y; LOCAL_134 < g_pCityZone[i].war_max_y; LOCAL_134++)
		{
			for(UINT32 LOCAL_135 = g_pCityZone[i].war_min_x; LOCAL_135 < g_pCityZone[i].war_max_x; LOCAL_135++)
			{
				INT32 mobId = g_pMobGrid[LOCAL_134][LOCAL_135]; // LOCAL_136
				if(mobId <= 0 || mobId >= MAX_PLAYER)
					continue;

				if(Mob[mobId].Mode == 0 || Mob[mobId].Mobs.Player.Status.curHP <= 0)
					continue;

				INT32 LOCAL_137 = Mob[mobId].Mobs.Player.GuildIndex;
				UINT32 LOCAL_138 = Mob[mobId].Mobs.Player.Status.Level + 1;

				if (Mob[mobId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
					LOCAL_138 += 400;

				if(LOCAL_137 == 0)
					continue;

				int lifesMultiplier = 1;
				if (Mob[mobId].Lifes > 0)
				{
					switch (Mob[mobId].Lifes)
					{
					case 5:
						lifesMultiplier = 10;
						break;
					case 4:
						lifesMultiplier = 8;
						break;
					case 3:
						lifesMultiplier = 5;
						break;
					case 2:
						lifesMultiplier = 3;
						break;
					case 1:
						break;
					}
				}

				int totalPoints = LOCAL_138 * lifesMultiplier;
				if (LOCAL_137 == LOCAL_4)
				{
					LOCAL_2 += LOCAL_138;
					strOwner << "Player " << Mob[mobId].Mobs.Player.Name << " com " << Mob[mobId].Lifes << " totalizou " << totalPoints << " pontos. Total: " << LOCAL_2 << "\n";
				}
				else if (LOCAL_137 == LOCAL_5)
				{
					LOCAL_3 += LOCAL_138;
					strChall << "Player " << Mob[mobId].Mobs.Player.Name << " com " << Mob[mobId].Lifes << " totalizou " << totalPoints << " pontos. Total: " << LOCAL_3 << "\n";
				}
			}
		}

		if(LOCAL_2 >= LOCAL_3)
		{
			SendGuildNotice(LOCAL_4, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_4].Name.c_str(), g_pGuild[LOCAL_5].Name.c_str(), LOCAL_2, LOCAL_3);
			SendGuildNotice(LOCAL_5, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_4].Name.c_str(), g_pGuild[LOCAL_5].Name.c_str(), LOCAL_2, LOCAL_3);
			
			if (ChargedGuildList[sServer.Channel - 1][i] == 0)
				g_pCityZone[i].win_count = -1;

			g_pCityZone[i].win_count ++;
			if(g_pCityZone[i].win_count >= 5)
				g_pCityZone[i].win_count = 4;

			ChargedGuildList[sServer.Channel - 1][i] = LOCAL_4;
		}
		else 
		{
			SendGuildNotice(LOCAL_4, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_5].Name.c_str(), g_pGuild[LOCAL_4].Name.c_str(), LOCAL_3, LOCAL_2);
			SendGuildNotice(LOCAL_5, g_pLanguageString[_SSNN_GuildWarResult], g_pGuild[LOCAL_5].Name.c_str(), g_pGuild[LOCAL_4].Name.c_str(), LOCAL_3, LOCAL_2);
			
			ChargedGuildList[sServer.Channel - 1][i] = LOCAL_5;

			g_pCityZone[i].win_count = 0;			
		}

		Log(SERVER_SIDE, LOG_INGAME, strOwner.str().c_str());
		Log(SERVER_SIDE, LOG_INGAME, strChall.str().c_str());

		Log(SERVER_SIDE, LOG_INGAME, "Guerra cidade %d terminada. %s %d %s %d", i, g_pGuild[LOCAL_4].Name.c_str(), LOCAL_2, g_pGuild[LOCAL_5].Name.c_str(), LOCAL_3);

		g_pCityZone[i].chall_index = 0;
		g_pCityZone[i].chall_index_2= 0;
	}
		
	INT32 castle = ChargedGuildList[sServer.Channel - 1][4];

	// Checa se há algum dono de castelo
	if(castle != 0)
	{
		// Caso tenha um dono, checa se possui peolo menos uma cidade
		bool has = false;
		for(INT32 i = 0; i < 4; i++)
		{
			// Caso encontre uma cidade, a variável HAS é setada como true
			if(ChargedGuildList[sServer.Channel - 1][i] == castle)
				has = true;
		}

		// CAso ainda esteja false, quer dizer que nenhuma cidade foi encontrada
		// Ou seja, Noatun será removida
		if(!has)
		{
			ChargedGuildList[sServer.Channel - 1][4] = 0;
			g_pCityZone[4].win_count   = 0;
			castle = 0;

			Log(SERVER_SIDE, LOG_GUILD, "Rei do Castelo perdeu o Castelo por não possuir nenhuma cidade");
		}
	}

	// Seta como true que tem todas as cidades
	bool hasAll = true;
	for(INT32 i = 1; i < 4; i++)
	{
		// Checa se o dono da cidade em questão é igual ao dono
		// da cidade de armia
		// Caos não seja, a variável é setada como false e não será entregue
		if(ChargedGuildList[sServer.Channel - 1][i] != ChargedGuildList[sServer.Channel - 1][0])
			hasAll = false;
	}

	// Caso a guilda seja dono de todas as cidades E o dono do castelo seja diferente do
	if(hasAll)
	{
		bool newOwner = ChargedGuildList[sServer.Channel - 1][4] != ChargedGuildList[sServer.Channel - 1][0];
		ChargedGuildList[sServer.Channel - 1][4] = ChargedGuildList[sServer.Channel - 1][0];

		if(newOwner)
			g_pCityZone[4].win_count = 0;

		Log(SERVER_SIDE, LOG_GUILD, "Entregue o Castelo para %d por possuir todas as cidades", ChargedGuildList[sServer.Channel - 1][4]);
	}

	UpdateCityTowers();
	SaveGuildZone();
}

void UpdateCityTowers()
{
	std::stringstream str;
	for (INT32 i = 0; i < 4096; i++)
	{
		if (pInitItem[i].Item.Index <= 0 || pInitItem[i].Item.Index >= MAX_ITEMLIST)
			continue;

		if (pInitItem[i].Item.Index >= 3145 && pInitItem[i].Item.Index <= 3149)
		{
			str << "Encontrado primeira torre em " << i << "\n";
			for (INT32 t = 0; t < 5; t++)
			{
				pInitItem[i + t].Item.Index = 3145 + g_pCityZone[t].win_count;

				pInitItem[i + t].Item.EF1 = 56;
				pInitItem[i + t].Item.EFV1 = ChargedGuildList[sServer.Channel - 1][t] / 257;

				pInitItem[i + t].Item.EF2 = 57;
				pInitItem[i + t].Item.EFV2 = ChargedGuildList[sServer.Channel - 1][t];

				pInitItem[i + t].Item.EF3 = 59;
				pInitItem[i + t].Item.EFV3 = Rand() % 255;

				str << "Torre com id " << (i + t) << " alterado para index " << pInitItem[i + t].Item.Index << "\n";
			}

			break;
		}
	}

	Log(SERVER_SIDE, LOG_INGAME, str.str().c_str());
}

void ClearChallanger()
{
	for(INT32 i = 0; i < 5; i++)
	{
		g_pCityZone[i].chall_index = 0;
		g_pCityZone[i].chall_index_2 = 0;

		for (auto& challenger : sServer.ChallengerMoney)
			challenger.clear();
	}
}

INT32 Challange(INT32 clientId, INT32 mobId, INT32 value)
{
	if(mobId < MAX_PLAYER || mobId >= MAX_SPAWN_MOB)
		return 0;

	INT32 cityId = Mob[mobId].Mobs.Player.bStatus.Level,
		  guildId = Mob[clientId].Mobs.Player.GuildIndex;

	if(cityId < 0 || cityId > 5)
		return 0;

	if(cityId == 4)
		return 0;

	if(Mob[clientId].Mobs.Player.GuildMemberType != 9)
	{
		SendSay(mobId, g_pLanguageString[_NN_Only_Guild_Master_can]);

		return 0;
	}

	INT32 wins = g_pGuild[guildId].Wins;
	if(wins == 0)
	{
		SendSay(mobId, "Para apostar você deve ter conquistado a Torre de Erion uma vez pelo menos");

		return 0;
	}

	if (g_pGuild[guildId].Fame < value || g_pGuild[guildId].Fame < 100)
	{
		SendSay(mobId, g_pLanguageString[_NN_Havent_Money_So_Much]);

		return 0;
	}
	
	for (int serverId = 0; serverId < 10; ++serverId)
	{
		if (serverId == sServer.Channel - 1)
			continue;

		for (int cityId = 0; cityId < 5; cityId++)
		{
			if (ChargedGuildList[serverId][cityId] == guildId)
			{
				SendSay(mobId, "Não é possível apostar enquanto possuir cidade em outro servidor");

				return 0;
			}
		}
	}

	if(value < 100)
	{
		SendSay(mobId, g_pLanguageString[_NN_Need_1000000_For_Challange]);

		return 0;
	}

	if(ChargedGuildList[sServer.Channel - 1][cityId] == guildId)
	{
		SendSay(mobId, g_pLanguageString[_NN_Champions_Cant_Challange]);

		return 0;
	}

	INT32 success = 1,
		  LOCAL_4 = sServer.ServerGroup,
		  LOCAL_5 = guildId >> 12,
		  LOCAL_6 = guildId % 0xFFF;

	if(LOCAL_6 < 0 || LOCAL_4 >= 10 || LOCAL_5 < 0 || LOCAL_5 >= 16 || LOCAL_6 < 0 || LOCAL_6 > 4096)
		success = 0;
	else if(!g_pGuild[guildId].Name[0])
		success = 0;

	if(success == 0)
	{
		SendSay(mobId, g_pLanguageString[_NN_Only_Named_Guild]);

		return 0;
	}

	value = g_pGuild[guildId].Fame;
	SetGuildFame(guildId, value - 100);

	SetGuildWin(guildId, wins - 1);
	g_pGuild[guildId].Wins--;

	SendClientMessage(clientId, "Aposta realizada.");

	Log(SERVER_SIDE, LOG_INGAME, "Guild %s realizou aposta na cidade %d. Valor: %d", g_pGuild[guildId].Name.c_str(), cityId, value);

	SendEtc(clientId);

	auto& cityChallenger = sServer.ChallengerMoney[cityId];
	auto guildIt = std::find_if(std::begin(cityChallenger), std::end(cityChallenger), [guildId](const ChallengeInfo& info) {
		return info.GuildId == guildId;
	});

	ChallengeInfo* guild = nullptr;
	if (guildIt == std::end(cityChallenger))
		guild = &cityChallenger.emplace_back();
	else
		guild = &*guildIt;

	if (guild == nullptr)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Falha ao encontrar ponteiro de guild. GuildId: %d", guildId);

		return 0;
	}

	guild->GuildId = guildId;
	guild->Value = value;

	return 1;
}

void DoWar(int arg1, int arg2)
{
	INT32 LOCAL_1 = 65535;
	if(arg1 <= 0 || arg2 < 0 ||arg1 >= LOCAL_1 || arg2 >= LOCAL_1)
		return;

	INT32 LOCAL_8 = g_pGuildWar[arg1];
	if(LOCAL_8 < 0 || LOCAL_8 >= LOCAL_1)
		LOCAL_8 = 0;

	INT32 LOCAL_9 = g_pGuildWar[arg2];
	if(arg2 == 0)
	{
		if(LOCAL_8 <= 0 || LOCAL_8 >= LOCAL_1)
			return;

		LOCAL_9 = g_pGuildWar[LOCAL_8];
		if(LOCAL_9 == arg1)
		{
			g_pGuildWar[LOCAL_8] = 0;
			g_pGuildWar[arg1] = 0;

			for(INT32 LOCAL_10 = 1; LOCAL_10 < MAX_PLAYER; LOCAL_10++)
			{
				if(Users[LOCAL_10].Status != USER_PLAY)
					continue;

				if(Mob[LOCAL_10].Mobs.Player.GuildIndex != arg1 && Mob[LOCAL_10].Mobs.Player.GuildIndex == LOCAL_8)
					continue;

				SendWarInfo(LOCAL_10, sServer.CapeWin);
			}
		}
		else
		{
			SendGuildNotice(arg1, g_pLanguageString[_SS_War_declare_canceled], g_pGuild[arg1].Name.c_str(), g_pGuild[arg2].Name.c_str());

			g_pGuildWar[arg1] = 0;
		}

		SendGuildNotice(arg1, g_pLanguageString[_SS_War_Canceled], g_pGuild[arg1].Name.c_str(), g_pGuild[arg2].Name.c_str());
		SendGuildNotice(LOCAL_8, g_pLanguageString[_SS_War_Canceled], g_pGuild[arg1].Name.c_str(), g_pGuild[arg2].Name.c_str());
	}
	else if(LOCAL_8 == 0 && arg2 != 0 && LOCAL_9 != arg1)
	{
		SendGuildNotice(arg1, g_pLanguageString[_SS_War_Declared], g_pGuild[arg1].Name.c_str(), g_pGuild[arg2].Name.c_str());
		SendGuildNotice(arg2, g_pLanguageString[_SS_War_Declared], g_pGuild[arg1].Name.c_str(), g_pGuild[arg2].Name.c_str());

		g_pGuildWar[arg1] = arg2;
	}
	else if(LOCAL_8 == 0 && arg2 != 0 && LOCAL_9 == arg1)
	{
		SendNotice(g_pLanguageString[_SS_War_Started], g_pGuild[arg1].Name.c_str(), g_pGuild[arg2].Name.c_str());

		g_pGuildWar[arg1] = arg2;

		for(INT32 LOCAL_11 = 1; LOCAL_11 < MAX_PLAYER; LOCAL_11++)
		{
			if(Users[LOCAL_11].Status != USER_PLAY)
				continue;
			
			if(Mob[LOCAL_11].Mobs.Player.GuildIndex != arg1 && Mob[LOCAL_11].Mobs.Player.GuildIndex == arg2)
				continue;

			SendWarInfo(LOCAL_11, sServer.CapeWin);
		}
	}
}

void SetGuildFame(INT32 guildId, INT32 fame)
{
	MSG_ADDGUILD packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = MSG_ADDGUILD_OPCODE;
	packet.Header.Size = sizeof packet;

	packet.Type = 0;
	packet.Value = fame;
	packet.guildIndex = guildId;

	AddMessageDB((BYTE*)&packet, sizeof packet);
}

void SetGuildWin(INT32 guildId, INT32 win)
{
	MSG_ADDGUILD packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = MSG_ADDGUILD_OPCODE;
	packet.Header.Size = sizeof packet;

	packet.Type = 2;
	packet.Value = win;
	packet.guildIndex = guildId;

	AddMessageDB((BYTE*)&packet, sizeof packet);
}

void SaveUser(INT32 clientId, INT32 arg2)
{
	stSaveChar packet{};
	packet.Header.PacketId = 0x807;
	packet.Header.Size = sizeof(stSaveChar);

	packet.CharSlot = Users[clientId].inGame.CharSlot;
	packet.Mob = Mob[clientId].Mobs;

	memcpy(packet.Storage, Users[clientId].User.Storage.Item, 1024);

	packet.Coin = Users[clientId].User.Storage.Coin;
	packet.Header.ClientId = clientId;
	packet.Arg2 = arg2;

	strncpy_s(packet.User, Users[clientId].User.Username, 16);
	strncpy_s(packet.Pass, Users[clientId].User.Block.Pass, 16);

	packet.Blocked = Users[clientId].User.Block.Blocked;

	memcpy(packet.SkillBar, Mob[clientId].Mobs.Player.SkillBar1, 4);
	memcpy(&packet.SkillBar[4], Mob[clientId].Mobs.SkillBar, 16);

	packet.Cash = Users[clientId].User.Cash;

	packet.BanType = Users[clientId].User.BanType;
	memcpy(&packet.Ban, &Users[clientId].User.Ban, sizeof stDate);

	memcpy(&packet.Friends, &Users[clientId].User.Friends, 30 * 16);
	packet.Insignia = Users[clientId].User.Insignias.Value;

	packet.Position.X = Mob[clientId].Target.X;
	packet.Position.Y = Mob[clientId].Target.Y;

	packet.Unique = Users[clientId].User.Unique.Value;
	packet.Slot = Users[clientId].User.CharSlot;

	packet.Daily.WeekYear  = Users[clientId].User.Daily.WeekYear;
	memcpy(packet.Daily.Received, Users[clientId].User.Daily.Received, sizeof Users[clientId].User.Daily.Received);

	packet.Water.Day = Users[clientId].User.Water.Day;
	packet.Water.Total = Users[clientId].User.Water.Total;

	packet.Divina = Users[clientId].User.Divina;
	packet.Sephira = Users[clientId].User.Sephira;
	packet.SingleGift = Users[clientId].User.SingleGift;

	AddMessageDB((BYTE*)&packet, sizeof stSaveChar);
}

void RemoveDefaultADD(st_Item *item, int index)
{
	__try
	{
		if(ItemList[item->Index].Pos >= 64)
			return;

		for(int i = 0; i < 3; i++)
		{
			switch(item->Effect[i].Index)
			{
				case 43:
				case 0:
				case 116:
				case 117:
				case 118:	
				case 119:
				case 120:
				case 121:
				case 122:
				case 123:
				case 124:
				case 125:
					continue;
			}

			int value = GetEffectValueByIndex(item->Index, item->Effect[i].Index); // 45
			int originalValue = GetEffectValueByIndex(index, item->Effect[i].Index); // 35

			item->Effect[i].Value = (item->Effect[i].Value + originalValue) - value;
		}
	}
	__except(1)
	{
	}
}

void FormatIntToTime(int time, char *string)
{
	sprintf_s(string, 16, "%02d:%02d:%02d", time / 3600, (time / 60) % 60, time % 60);
}

INT32 ReadNPCQuest()
{
	// Busca por arquivos no diretorio dir
	HANDLE handle;
	WIN32_FIND_DATA win32_find_data;
	handle = FindFirstFile("Quests\\*.c", &win32_find_data);

	if(handle == INVALID_HANDLE_VALUE)
		return false;

	// Limpa a array
	memset(questNPC, 0, sizeof questNPC);

	while(true)
	{
		char *fileName = win32_find_data.cFileName;
		if(fileName[0] == '.')
		{
			if(FindNextFile(handle, &win32_find_data))
				break;

			continue;
		}

		char tmp[256];
		sprintf_s(tmp, "Quests\\%s", fileName);
		printf("Lendo arquivo: %s\n", fileName);

		FILE *pFile = nullptr;
		fopen_s(&pFile, tmp, "r");

		if(!pFile)
		{
			if(!FindNextFile(handle, &win32_find_data))
				break;

			continue;
		}

		while(fgets(tmp, 256, pFile))
		{
			char cmd1[96],
				 cmd2[96],
				 cmd3[96],
				 cmd4[96],
				 cmd5[96],
				 cmd6[96];

			INT32 ret = sscanf_s(tmp, "%s %s %s %s %s %s", cmd1, 96, cmd2, 96, cmd3, 96, cmd4, 96, cmd5, 96, cmd6, 96);

			INT32 questId = -1;

			if(sscanf_s(cmd1, "QUEST%d", &questId))
			{
				if(questId < 0 || questId >= MAX_NPCQUEST)
					questId = -1;
			}

			if(questId == -1)
				continue;

			stNPCQuest *npc = &questNPC[questId];
			if(!_strnicmp(cmd2, "TAB", 3))
				strncpy_s(npc->Tab, cmd3, 28);
			else if(!_strnicmp(cmd2, "CONDITION", 9))
			{
				INT32 condition = -1;

				sscanf_s(cmd2, "CONDITION-%d", &condition);
				if(condition < 0 || condition >= MAX_NPCQUEST_CONDITION)
					continue;
				
				if(!_strnicmp(cmd3, "SPEECH", 6) && sscanf_s(cmd4, "%s", npc->Condition[condition].Speech, 96))
					continue;
				else if(!_strnicmp(cmd3, "LEVEL", 5) && sscanf_s(cmd4, "%d", &npc->Condition[condition].minLevel) && sscanf_s(cmd5, "%d", &npc->Condition[condition].maxLevel))
					continue;
				else if(!_strnicmp(cmd3, "EVOLUTION", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Evolution))
					continue;
				else if(!_strnicmp(cmd3, "EXP", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Exp))
					continue;
				else if(!_strnicmp(cmd3, "GOLD", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Gold))
					continue;
				else if(!_strnicmp(cmd3, "CLASS", 9) && sscanf_s(cmd4, "%d", &npc->Condition[condition].Class))
					continue;
				else if(!_strnicmp(cmd3, "EQITEM", 6))
				{
					INT32 pos = -1, 
						  itemId = -1;
					
					if(!sscanf_s(cmd4, "%d", &pos) || !sscanf_s(cmd5, "%d", &itemId))
						continue;
					
					npc->Condition[condition].Equip.Slot = pos;
					npc->Condition[condition].Equip.ItemID = itemId;
				}
				else if(!_strnicmp(cmd3, "ITEM", 4))
				{
					INT32 itemId = -1,
						  amount = -1;

					if(!sscanf_s(cmd4, "%d", &amount) || !sscanf_s(cmd5, "%d", &itemId))
						continue;

					if(itemId <= 0 || itemId >= 6500 || amount <= 0 || amount >= 200)
						continue;

					npc->Condition[condition].Item.Item = itemId;
					npc->Condition[condition].Item.Amount = amount;
				}
			}
			else if(!_strnicmp(cmd2, "REWARD", 6))
			{
				INT32 reward = -1;

				sscanf_s(cmd2, "REWARD-%d", &reward);
				if(reward < 0 || reward >= MAX_NPCQUEST_REWARD)
					continue;
				
				if(!_strnicmp(cmd3, "SPEECH", 6) && sscanf_s(cmd4, "%s", npc->Reward[reward].Speech, 96))
					continue;
				else if(!_strnicmp(cmd3, "EXP", 6) && sscanf_s(cmd4, "%d", &npc->Reward[reward].Exp))
					continue;
				else if(!_strnicmp(cmd3, "LEVEL", 6) && sscanf_s(cmd4, "%d", &npc->Reward[reward].Level))
					continue;
				else if(!_strnicmp(cmd3, "GOLD", 6) && sscanf_s(cmd4, "%d", &npc->Reward[reward].Gold))
					continue;
				else if(!_strnicmp(cmd3, "EQUIP", 6))
				{
					INT32 pos = -1;
					st_Item item;
					
					memset(&item, 0, sizeof st_Item);
					if(!sscanf_s(tmp, "%*s %*s %*s %d %hd %hhu %hhu %hhu %hhu %hhd %hhu", &pos, &item.Index, &item.EF1, &item.EFV1, &item.EF2, &item.EFV2, &item.EF3, &item.EFV3))
						continue;
					
					npc->Reward[reward].Equip.Slot = pos;
					memcpy(&npc->Reward[reward].Equip.Item, &item, sizeof st_Item);
				}
				else if(!_strnicmp(cmd3, "TELEPORT", 6))
				{
					INT32 posX = 0,
						  posY = 0;
				
					if(!sscanf_s(cmd4, "%d", &posX) || !sscanf_s(cmd5, "%d", &posY))
						continue;

					if(posX < 0 || posX >= 4096 || posY < 0 || posY >= 4096)
					{
						npc->Reward[reward].Teleport.X = 0;
						npc->Reward[reward].Teleport.Y = 0;
					
						printf("Fail : TELEPORT QUEST %d - %dx %dy\n", questId, posX, posY);
					}
					else
					{
						npc->Reward[reward].Teleport.X = posX;
						npc->Reward[reward].Teleport.Y = posY;
					}
					continue;
				}
				else if(!_strnicmp(cmd3, "REMOVEGOLD", 10))
				{
					INT32 gold = 0;

					if(!sscanf_s(cmd4, "%d", &gold))
						continue;

					npc->Remove.Gold = gold;
				}
				else if(!_strnicmp(cmd3, "REMOVEEXP", 10))
				{
					INT32 exp = 0;

					if(!sscanf_s(cmd4, "%d", &exp))
						continue;

					npc->Remove.Exp = exp;
				}
				else if(!_strnicmp(cmd3, "DELETEITEM", 10))
				{
					INT32 amount = 0,
						  itemId = 0,
						  slot   = 0;
				
					if(!sscanf_s(cmd4, "%d", &slot) || !sscanf_s(cmd5, "%d", &amount) || !sscanf_s(cmd6, "%d", &itemId))
						continue;

					npc->Remove.Item[slot].Item = itemId;
					npc->Remove.Item[slot].Amount = amount;
				}
				else if(!_strnicmp(cmd3, "EQDELETE", 8))
				{
					INT32 slotId = 0,
						  itemId = 0;
				
					if(!sscanf_s(cmd4, "%d", &slotId) || !sscanf_s(cmd5, "%d", &itemId))
						continue;

					npc->Remove.Equip.Slot = slotId;
					npc->Remove.Equip.Item = itemId;
				}
				else if(!_strnicmp(cmd3, "ITEM", 4))
				{
					INT32 slot = -1;

					sscanf_s(cmd3, "ITEM-%d", &slot);
					if(slot < 0 || slot >= MAX_NPCQUEST_REWARD_ITEM)
						continue;

					st_Item item;
					memset(&item, 0, sizeof st_Item);

					INT32 amount = -1;
					sscanf_s(tmp, "%*s %*s %*s %d %hd %hhu %hhu %hhu %hhu %hhu %hhu", &amount, &item.Index, &item.EF1, &item.EFV1, &item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

					if(amount <= 0 || amount >= 100 || item.Index <= 0 || item.Index >= 6500)
						continue;

					memcpy(&npc->Reward[reward].Item[slot].Item, &item, sizeof st_Item);
					npc->Reward[reward].Item[slot].Amount = amount;
				}
			}
		}

		fclose(pFile);
		
		if(!FindNextFile(handle, &win32_find_data))
			break;
	}

	FindClose(handle);
	return true;
}

INT32 ReadMessages()
{
	char line[1024];
	memset(sServer.Messages, 0 , sizeof sServer.Messages);

	FILE *pFile;
	fopen_s(&pFile, "Messages.txt", "r");

	if(pFile) 
	{
		int index = 0;

		while((fscanf_s(pFile, "%[^\n]", line, 1024)) != EOF)
		{
			fgetc(pFile);

			if(*line == '#')
				continue;

			sscanf_s(line, "%hd %hd %hd %hd %[^\n]", &sServer.Messages[index].Hour, &sServer.Messages[index].Min, &sServer.Messages[index].Interval, &sServer.Messages[index].Repeat, sServer.Messages[index].msg, 128);

			index++;

			if(index == MAX_MESSAGE)
				break;
		}

		return true;
	}

	return false;
}

INT32 ReadNoCP( )
{
	FILE *pFile = nullptr;

	char szTMP[1024];
	sprintf_s(szTMP, "Data\\NonCP.txt");

	fopen_s(&pFile, szTMP, "r");
	if(!pFile)
		return false;
	
	memset(g_pPositionCP, 0, sizeof g_pPositionCP);
	int count = 0;
	while(fgets(szTMP, sizeof szTMP, pFile))
	{
		if (szTMP[0] == '#')
			continue;

		INT32 minPosX = 0,
			  maxPosX = 0,
			  minPosY = 0,
			  maxPosY = 0;

		INT32 ret = sscanf_s(szTMP, "%d %d %d %d", &minPosX, &minPosY, &maxPosX, &maxPosY);	

		if(minPosX < 0 || minPosY < 0 || minPosX >= 4096 || minPosY >= 4096 || maxPosX < 0 || maxPosY < 0 || maxPosX >= 4096 || maxPosY >= 4096)
			return false;
		
		g_pPositionCP[count].Min.X = minPosX;
		g_pPositionCP[count].Min.Y = minPosY;
		
		g_pPositionCP[count].Max.X = maxPosX;
		g_pPositionCP[count].Max.Y = maxPosY;

		count++;

		if(count >= MAX_MESSAGE)
			break;
	}

	fclose(pFile);
	return true;
}

INT32 ReadScheduled ( )
{
	const std::string filename = "Data\\Scheduled.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::stringstream str;
		str << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		str << "Error description: " << result.description() << "\n";
		str << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		Log(SERVER_SIDE, LOG_INGAME, "Falha ao ler arquivo Scheduled.xml. Erro:\n%s", str.str().c_str());
		return false;
	}

	sServer.Scheduled.clear();
	auto scheduledNode = doc.child("scheduled");
	for (auto commandNode = scheduledNode.child("command"); commandNode; commandNode = commandNode.next_sibling("command"))
	{
		auto& scheduled = sServer.Scheduled.emplace_back();
		scheduled.Month = std::stoi(commandNode.child_value("month"));
		scheduled.Day = std::stoi(commandNode.child_value("day"));
		scheduled.Hour = std::stoi(commandNode.child_value("hour"));
		scheduled.Min = std::stoi(commandNode.child_value("min"));
		scheduled.Command = commandNode.child_value("commandline");
	}

	return true;
}

bool ReadRvRStore()
{
	const std::string filename = "Data\\RvRStore.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	std::fill(sServer.RvR.Items.begin(), sServer.RvR.Items.end(), TOD_RvRStore_Item{});

	auto rvrNode = doc.child("rvr");
	for (auto itemsNode = rvrNode.child("items"); itemsNode; itemsNode = itemsNode.next_sibling("items"))
	{
		TOD_RvRStore_Item rvrItem{};

		auto itemNode = itemsNode.child("item");
		auto index = std::stoul(itemsNode.child_value("index"));
		if (index >= sServer.RvR.Items.size())
			continue;

		st_Item item{};
		item.Index = itemNode.attribute("itemId").as_int(0);
		if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
			continue;

		for (int i = 0; i < 3; i++)
		{
			std::string childEffectName = "ef" + std::to_string(i);
			std::string childEffectValueName = "efv" + std::to_string(i);

			item.Effect[i].Index = itemNode.attribute(childEffectName.c_str()).as_int(0);
			item.Effect[i].Value = itemNode.attribute(childEffectValueName.c_str()).as_int(0);
		}

		rvrItem.Price = std::stoi(itemsNode.child_value("price"));
		rvrItem.Available = std::stoi(itemsNode.child_value("available"));
		rvrItem.Item = item;

		sServer.RvR.Items[index] = rvrItem;
	}

	return true;
}

void WriteRvRStore()
{
	pugi::xml_document doc;
	auto rvrNode = doc.append_child("rvr");

	int index = 0;
	for (const auto& items : sServer.RvR.Items)
	{
		if (items.Item.Index <= 0 || items.Item.Index >= MAX_ITEMLIST)
			continue;

		auto itemsNode = rvrNode.append_child("items");
		itemsNode.append_child("index").append_child(pugi::node_pcdata).set_value(std::to_string(index).c_str());

		auto itemNode = itemsNode.append_child("item");
		itemNode.append_attribute("itemId").set_value(items.Item.Index);
		itemNode.append_attribute("ef0").set_value(items.Item.Effect[0].Index);
		itemNode.append_attribute("ef1").set_value(items.Item.Effect[1].Index);
		itemNode.append_attribute("ef2").set_value(items.Item.Effect[2].Index);
		itemNode.append_attribute("efv0").set_value(items.Item.Effect[0].Value);
		itemNode.append_attribute("efv1").set_value(items.Item.Effect[1].Value);
		itemNode.append_attribute("efv2").set_value(items.Item.Effect[2].Value);

		itemsNode.append_child("price").append_child(pugi::node_pcdata).set_value(std::to_string(items.Price).c_str());
		itemsNode.append_child("available").append_child(pugi::node_pcdata).set_value(std::to_string(items.Available).c_str());
		index++;
	}

	doc.save_file("Data\\RvRStore.xml");
}

bool ReadMissions()
{
	const std::string filename = "Data\\Missions.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	sServer.Missions.clear();

	auto missionsNode = doc.child("missions");
	for (auto missionNode = missionsNode.child("mission"); missionNode; missionNode = missionNode.next_sibling("mission"))
	{
		TOD_MissionInfo mission{};
		mission.QuestId = std::stoi(missionNode.child_value("id"));
		mission.QuestName = missionNode.child_value("name");
		mission.Type = static_cast<TOD_DailyQuestInfo_Type>(std::stoi(missionNode.child_value("id")));

		{
			auto reqNode = missionNode.child("reqs");
		
			{
				auto classesNode = reqNode.child("classes");
				for (auto classNode = classesNode.child("class"); classNode; classNode = classNode.next_sibling("class"))
					mission.ClassMaster.push_back(std::stoi(classNode.child_value()));
			}
			{
				std::string minLevel = reqNode.child_value("minLevel");
				std::string maxLevel = reqNode.child_value("maxLevel");

				if (!minLevel.empty() && !maxLevel.empty())
				{
					mission.MinLevel = std::stoi(minLevel.c_str());
					mission.MaxLevel = std::stoi(maxLevel.c_str());
				}
			}
		}

		{
			auto questNode = missionNode.child("quest");

			for (auto mobNode = questNode.child("mob"); mobNode; mobNode = mobNode.next_sibling("mob"))
			{
				auto index = mobNode.attribute("index").as_uint();
				if (index >= mission.Mob.mobName.size())
					continue;

				mission.Mob.mobName[index] = mobNode.attribute("name").as_string();
				mission.Mob.Amount[index] = mobNode.attribute("total").as_int();
			}

			for (auto dropNode = questNode.child("drop"); dropNode; dropNode = dropNode.next_sibling("drop"))
			{
				auto index = dropNode.attribute("index").as_uint();
				if (index >= mission.Mob.mobName.size())
					continue;

				mission.Drop.Item[index] = dropNode.attribute("id").as_int();
				mission.Drop.Amount[index] = dropNode.attribute("total").as_int();
			}
		}
		{
			auto rewardNode = missionNode.child("rewards");

			auto readReward = [](pugi::xml_node node, std::array<TOD_DailyQuestInfo_RewardItem, 6>& reward) {
				auto items = node.child("items");
				if (items != nullptr)
				{
					for (auto itemNode = items.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
					{
						auto index = itemNode.attribute("index").as_uint();
						if (index >= reward.size())
							continue;

						st_Item item{};
						item.Index = itemNode.attribute("itemId").as_int(0);
						if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
							continue;

						for (int i = 0; i < 3; i++)
						{
							std::string childEffectName = "ef" + std::to_string(i);
							std::string childEffectValueName = "efv" + std::to_string(i);

							item.Effect[i].Index = itemNode.attribute(childEffectName.c_str()).as_int(0);
							item.Effect[i].Value = itemNode.attribute(childEffectValueName.c_str()).as_int(0);
						}

						reward[index].Amount = itemNode.attribute("amount").as_int(1);
						reward[index].Item = item;
					}
				}
			};

			readReward(rewardNode.child("free"), mission.FreeReward);
			readReward(rewardNode.child("pass"), mission.BattlePassReward);

			std::string gold = rewardNode.child_value("gold");
			if (!gold.empty())
				mission.Gold = std::stoi(gold.c_str());

			std::string exp = rewardNode.child_value("exp");
			if (!exp.empty())
				mission.Exp = std::stoi(exp.c_str());

			sServer.Missions.push_back(mission);
		}
	}

	return true;
}

bool ReadOverStore()
{
	const std::string filename = "Data\\DonateStore.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	auto& donateStore = sServer.DonateStore;
	donateStore.Categories.clear();

	auto storeNode = doc.child("store");

	{
		auto& categories = donateStore.Categories;
		auto categoriesNode = storeNode.child("categories");

		for (auto categoryNode = categoriesNode.child("category"); categoryNode; categoryNode = categoryNode.next_sibling("category"))
		{
			auto& category = categories.emplace_back();

			category.Name = categoryNode.child_value("name");
			
			auto& items = category.Items;
			auto itemsNode = categoryNode.child("items");
			for (auto itemNode = itemsNode.child("itemInfo"); itemNode; itemNode = itemNode.next_sibling("itemInfo"))
			{

				st_Item item{};
				{
					auto miniItemNode = itemNode.child("item");
					item.Index = miniItemNode.attribute("itemId").as_int();
					if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
						continue;

					for (int i = 0; i < 3; i++)
					{
						std::string childEffectName = "ef" + std::to_string(i);
						std::string childEffectValueName = "efv" + std::to_string(i);

						item.Effect[i].Index = miniItemNode.attribute(childEffectName.c_str()).as_int();
						item.Effect[i].Value = miniItemNode.attribute(childEffectValueName.c_str()).as_int();
					}
				}

				TOD_OverStore_ItemInfo itemInfo{};
				itemInfo.Item = item;
				itemInfo.Price = std::stoi(itemNode.child_value("price"));

				if (!std::string(itemNode.child_value("available")).empty())
					itemInfo.Available = std::stoi(itemNode.child_value("available"));

				items.push_back(itemInfo);
			}
		}
	}

	return true;
}

bool ReadArenaConfig()
{
	const std::string filename = "Data\\arenaconfig.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	auto configNode = doc.child("arena");

	sServer.Arena.HoursAllowed.clear();
	sServer.Arena.ParticipantRewards.Items.clear();
	sServer.Arena.TopKillRewards.Items.clear();
	sServer.Arena.WinnerRewards.Items.clear();

	sServer.Arena.MaxKills = configNode.attribute("maxKills").as_int();
	if (sServer.Arena.MaxKills <= 0)
		sServer.Arena.MaxKills = 150;

	{
		auto hours = configNode.child("hours");

		for (auto hour = hours.child("hour"); hour; hour = hour.next_sibling("hour"))
		{
			int value = std::stoi(hour.child_value());
			if (std::find(std::begin(sServer.Arena.HoursAllowed), std::end(sServer.Arena.HoursAllowed), value) != std::end(sServer.Arena.HoursAllowed))
				continue;

			sServer.Arena.HoursAllowed.push_back(value);
		}
	}

	auto readReward = [](pugi::xml_node rewardNode, ArenaReward& rewards)
	{
		if (rewardNode == nullptr)
			return;

		{
			auto goldNode = rewardNode.child_value("gold");

			if (goldNode != nullptr && !std::string(goldNode).empty())
				rewards.Gold = std::stoi(goldNode);

			if (rewardNode != nullptr)
			{
				{
					auto exps = rewardNode.child("experiences");

					for (auto exp = exps.child("experience"); exp; exp = exp.next_sibling("experience"))
					{
						auto ev = exp.attribute("evolution").as_uint() - 1;
						auto value = exp.attribute("value").as_ullong();
						auto maxLevel = exp.attribute("maxlevel").as_uint();

						if (ev >= rewards.Experience.size())
							continue;

						auto& expInfo = rewards.Experience[ev].emplace_back();
						expInfo.Experience = value;
						expInfo.MaximumLevel = maxLevel;
					}
				}

				{
					auto items = rewardNode.child("items");

					if (items != nullptr)
					{
						for (auto itemNode = items.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
						{
							st_Item item{};
							item.Index = itemNode.attribute("itemId").as_int();

							if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
								continue;

							for (int i = 0; i < 3; i++)
							{
								std::string childEffectName = "ef" + std::to_string(i);
								std::string childEffectValueName = "efv" + std::to_string(i);

								item.Effect[i].Index = itemNode.attribute(childEffectName.c_str()).as_int();
								item.Effect[i].Value = itemNode.attribute(childEffectValueName.c_str()).as_int();
							}

							rewards.Items.push_back(item);
						}
					}
				}
			}
		}
	};

	auto rewards = configNode.child("rewards");
	readReward(configNode.child("rewards"), sServer.Arena.WinnerRewards);
	readReward(configNode.child("participant"), sServer.Arena.ParticipantRewards);
	readReward(configNode.child("topKill"), sServer.Arena.TopKillRewards);
	return true;
}

void ClearCrown(int conn)
{
	if (Users[conn].Status != USER_PLAY)
		return;

	int guildMemberType = Mob[conn].Mobs.Player.GuildMemberType;
	int guildId = Mob[conn].Mobs.Player.GuildIndex;

	if(guildId == 0)
		return;

	int isCrownGuild = FALSE;

	int i = 0;

	for (i = 0; i < 10; i++)
	{
		int crownGuild = ChargedGuildList[i][4];

		if ((guildId != 0) && (crownGuild != 0) && (crownGuild == guildId && guildMemberType == 9))
			isCrownGuild = TRUE;
	}

	if (isCrownGuild == 1)
	{
		int haveCrown = 0;
		int haveDrag = 0;

		for (i = 0; i < MAX_EQUIP; i++)
		{
			if (Mob[conn].Mobs.Player.Equip[i].Index == 747)
				haveCrown = 1;

			if (Mob[conn].Mobs.Player.Equip[i].Index != 3993 && Mob[conn].Mobs.Player.Equip[i].Index != 3994) // 747 = Crown on itemlist
				continue;

			haveDrag = 1;
		}

		for (i = 0; i < 64; i++)
		{
			if (Mob[conn].Mobs.Player.Inventory[i].Index == 747)
				haveCrown = 1;

			if (Mob[conn].Mobs.Player.Inventory[i].Index != 3993 && Mob[conn].Mobs.Player.Inventory[i].Index != 3994) // 747 = Crown on itemlist
				continue;

			haveDrag = 1;
		}

		for (i = 0; i < MAX_CARGO; i++)
		{
			if (Users[conn].User.Storage.Item[i].Index == 747)
				haveCrown = 1;

			if (Users[conn].User.Storage.Item[i].Index != 3993 && Users[conn].User.Storage.Item[i].Index != 3994) // 747 = Crown on itemlist
				continue;

			haveDrag = 1;
		}

		if (haveCrown == 0)
		{
			st_Item Item;
			memset(&Item, 0, sizeof(st_Item));

			Item.Index = 747;
			Item.Effect[0].Index = 43;
			Item.Effect[0].Value = 9;

			UINT32 guildFame = g_pGuild[guildId].Fame;
			if (guildFame >= 1000)
			{
				if (guildFame < 1500)
					Item.Effect[0].Value = 233;
				else if (guildFame < 3000)
					Item.Effect[0].Value = 237;
				else if (guildFame < 5000) 
					Item.Effect[0].Value = 241;
				else if (guildFame < 8000)
					Item.Effect[0].Value = 245;
				else if (guildFame < 11000)
					Item.Effect[0].Value = 249;
				else
					Item.Effect[0].Value = 253;
			}

			PutItem(conn, &Item);
		}

		if (haveDrag == 0)
		{
			st_Item Item;
			memset(&Item, 0, sizeof(st_Item));

			if (Mob[conn].Mobs.Player.CapeInfo == CAPE_RED)
				Item.Index = 3993;
			else if (Mob[conn].Mobs.Player.CapeInfo == CAPE_BLUE)
				Item.Index = 3994;


			PutItem(conn, &Item);
		}

		return;
	}

	for (i = 0; i < MAX_EQUIP; i++)
	{
		if (Mob[conn].Mobs.Player.Equip[i].Index != 747 && Mob[conn].Mobs.Player.Equip[i].Index != 3993 && Mob[conn].Mobs.Player.Equip[i].Index != 3994) // 747 = Crown on itemlist
			continue;

		//sprintf_s(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
		//Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		memset(&Mob[conn].Mobs.Player.Equip[i], 0, sizeof(st_Item));
		SendItem(conn, SlotType::Equip, i, &Mob[conn].Mobs.Player.Equip[i]);
	}

	for (i = 0; i < MAX_INVEN; i++)
	{
		if (Mob[conn].Mobs.Player.Inventory[i].Index != 747 && Mob[conn].Mobs.Player.Inventory[i].Index != 3993 && Mob[conn].Mobs.Player.Inventory[i].Index != 3994) // 747 = Crown on itemlist
			continue;

		memset(&Mob[conn].Mobs.Player.Inventory[i], 0, sizeof(st_Item));
		SendItem(conn, SlotType::Inv, i, &Mob[conn].Mobs.Player.Inventory[i]);
	}

	for (i = 0; i < MAX_CARGO; i++)
	{
		if (Users[conn].User.Storage.Item[i].Index != 747 && Users[conn].User.Storage.Item[i].Index != 3993 && Users[conn].User.Storage.Item[i].Index != 3994) // 747 = Crown on itemlist
			continue;

		//sprintf_s(temp, "etc,crown guild:%d level:%d charge:%d name:%s", Guild, GLevel, ChargedGuildList[ServerIndex][4], pMob[conn].MOB.MobName);
		//Log(temp, pUser[conn].AccountName, pUser[conn].IP);

		memset(&Users[conn].User.Storage.Item[i], 0, sizeof(st_Item));

		SendItem(conn, SlotType::Storage, i, &Users[conn].User.Storage.Item[i]);
	}
}

void GuildZoneReport()
{	
	MSG_GuildZoneReport sm;
	memset(&sm, 0, sizeof(MSG_GuildZoneReport));

	sm.Header.PacketId = _MSG_GuildZoneReport;
	sm.Header.Size = sizeof(sm);
	sm.Header.ClientId = sServer.Channel - 1;

	for (int i = 0; i < 5; i++)
		sm.Guild[i] = ChargedGuildList[sServer.Channel - 1][i];
	
	AddMessageDB((BYTE*)&sm, sizeof(sm));
}

void ClearTowerArea(BYTE Citizenship)
{
	// Citizenship:
	// 0 -> Guerra iniciando (teleporta todos na área)
	// 1 -> Deixar somente cidadania 1
	// 2 -> Deixar somente cidadania 2
		
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		CMob *player = &Mob[i];

		if (player->Target.X < 2447 || player->Target.X > 2545 ||
			player->Target.Y < 1857 || player->Target.Y > 1919)
			continue;

		DoRecall(i);
	}
}

//void InitializeTowerWar(stTowerWar* war)
void InitializeTowerWar()
{
	sServer.TowerWar.Status = 1;
	
	GenerateMob(TORRE_ERION, 0, 0);
	ClearTowerArea(0);
	
	SendNotice("A batalha do acampamento avançado do reino foi iniciada.");

	/*
	memcpy(sServer.TowerWar, war, sizeof stTowerWar * 10);

	ClearTowerArea(0);
	GenerateMob(TORRE_ERION, 0, 0);

	SendNotice("A batalha do acampamento avançado do reino foi iniciada.");*/
}

//void FinalizeTowerWar(stTowerWar* war)
void FinalizeTowerWar()
{
	sServer.TowerWar.Status = 0;
	
	for (int i = MAX_PLAYER; i < MAX_SPAWN_MOB; i++)
	{
		if (Mob[i].GenerateID == TORRE_ERION)
		{
			// Mata a torre de thor de forma definitiva
			MobKilled(i, 29999, Mob[i].Target.X, Mob[i].Target.Y);

			break;
		}
	}
	
	INT32 guildId = sServer.TowerWar.Guild;
	if(guildId != 0)
	{
		INT32 fame = g_pGuild[guildId].Fame + 100;
		INT32 win = g_pGuild[guildId].Wins + 1;

		SetGuildFame(guildId, fame);
		Log(SERVER_SIDE, LOG_INGAME, "Guild %s [%d] conquistou a Torre de Erion - Fame: %d - Wins: %d", g_pGuild[guildId].Name.c_str(), guildId, fame, win);

		SetGuildWin(guildId, win);
	}

	SendNotice("A batalha de acampamento avançado do reino foi finalizada");
	
	ClearTowerArea(0);
	/*
	memcpy(sServer.TowerWar, war, sizeof stTowerWar * 10);

	for (int i = MAX_PLAYER; i < MAX_SPAWN_MOB; i++)
	{
		if (Mob[i].GenerateID == TORRE_ERION)
		{
			// Mata a torre de thor de forma definitiva
			MobKilled(i, 29999, Mob[i].Target.X, Mob[i].Target.Y);

			break;
		}
	}

	SendNotice("A batalha de acampamento avnçado do reino foi finalizada");*/
}

void ResultTowerWar(INT32 winner)
{/*
	INT32 conn = sServer.Channel - 1;
	if(conn != winner && winner != -1)
	{
		_MSG_REWARDWARTOWER packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = MSG_REWARDWARTOWER_OPCODE;
		packet.Header.Size = sizeof _MSG_REWARDWARTOWER;
		
		packet.Server = winner;

		INT64 gold = g_pCityZone[4].impost * 10 / 100;
		packet.Gold = gold;

		g_pCityZone[4].impost -= gold;

		INT32 exp = sServer.CitizenEXP.Bonus;
		INT32 removed = 30;
		if(exp > 30)
			exp -= 30;
		else
		{
			removed = exp;
			exp = 0;
		}
		
		sServer.CitizenEXP.Bonus = exp;

		packet.Taxe = removed;
		AddMessageDB((BYTE*)&packet, sizeof _MSG_REWARDWARTOWER);

		Log(SERVER_SIDE, LOG_INGAME, "Canal %d foi vencedor e será entregue %I64d de impostos e %d de exp/drop", winner, gold, removed);
	}*/
}

void UpdateTowerWar(stTowerWar* war)
{
//	memcpy(sServer.TowerWar, war, sizeof stTowerWar * 10);
}

void UpdateWarDeclaration(BYTE Info)
{/*
	_MSG_UPDATEWARDECLARATION packet;
	memset(&packet, 0, sizeof _MSG_UPDATEWARDECLARATION);

	packet.Header.ClientId = 0;
	packet.Header.PacketId = MSG_UPDATEWARDECLARATION;
	packet.Header.Size = sizeof _MSG_UPDATEWARDECLARATION;

	// arg1 = 1: indica que está sendo declarado
	// arg1 = 0: indica que está sendo recusado
	packet.newInfo = Info;

	AddMessageDB((BYTE*)&packet, sizeof _MSG_UPDATEWARDECLARATION);*/
}

void RequestUpdateFriendList(INT32 clientId)
{
	PacketHeader packet;
	memset(&packet, 0, sizeof packet);

	packet.PacketId = 0x906;
	packet.ClientId = clientId;
	packet.Size = sizeof PacketHeader;

	AddMessageDB((BYTE*)&packet, sizeof PacketHeader);
}

INT32 ReadNPCDonate()
{
	FILE *pFile = nullptr;
	fopen_s(&pFile, "..\\DBsrv\\Donate.txt", "r");

	memset(g_pStore, 0, sizeof g_pStore);

	if(pFile)
	{
		char line[1024];

		while(fgets(line, sizeof line, pFile))
		{
			// Comentário
			if(*line == '#' || *line == '\n')
				continue; 

			INT32 store = 0,
				  index = 0,
				  avaible = -1,
				  price = 0,
				  loop = 0,
				  itemId = 0, ef1 = 0, ef2 = 0, ef3 = 0,
				  efv1 = 0, efv2 = 0, efv3 = 0;

			INT32 ret = sscanf_s(line, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", &store, &index, &avaible, &price, &loop,
				&itemId, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);

			if(ret < 6)
			{
				MessageBoxA(NULL, line, "ERROR", NULL);

				break;
			}

			if(store < 0 || store >= 5 || index >= 27 || index < 0)
				continue;

			g_pStore[store][index].Avaible = avaible;
			g_pStore[store][index].Loop = loop;
			g_pStore[store][index].Price = price;

			g_pStore[store][index].item.Index = itemId;
			g_pStore[store][index].item.EF1 = ef1;
			g_pStore[store][index].item.EF2 = ef2;
			g_pStore[store][index].item.EF3 = ef3;
			g_pStore[store][index].item.EFV1 = efv1;
			g_pStore[store][index].item.EFV2 = efv2;
			g_pStore[store][index].item.EFV3 = efv3;
		}

		fclose(pFile);
	}
	else
		return false;

	return true;
}

INT32 ReadPacItens()
{
	FILE *pFile;
	fopen_s(&pFile, "..\\DBsrv\\PacItem.txt", "r");
	memset(g_pPacItem, 0, sizeof g_pPacItem);

	if(!pFile)
		return false;

	char line[1024];
	while(fgets(line, sizeof line, pFile))
	{
		if(*line == '#' || *line == '\n')
			continue;

		st_Item item = {0, };
		INT32 index = -1;
		INT32 amount = 0;
		INT32 itemId = 0;
		INT32 ret = sscanf_s(line, "%d, %d, %d, %hd, %hhu, %hhu, %hhu, %hhu, %hhu, %hhu", &itemId, &index, &amount, &item.Index, &item.EF1, &item.EFV1, 
			&item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

		if(ret < 3)
			continue;

		if(index < 0 || index >= MAX_PACITEM)
			continue;
		
		g_pPacItem[index].ItemId = itemId;

		INT32 t = 0;
		for(; t < 8; t++)
			if(g_pPacItem[index].Item[t].Index == 0)
				break;
		
		if(t == 8)
			continue;

		memcpy(&g_pPacItem[index].Item[t], &item, sizeof st_Item);
		g_pPacItem[index].Amount[t] = amount;
	}

	fclose(pFile);
	return true;
}

INT32 ReadBossQuest()
{
	memset(sServer.QuestsBosses, 0, sizeof Quests * 5);

	for (int i = 0; i < 5; i++)
	{
		FILE *fs = NULL;

		char Directory[1024];
		sprintf_s(Directory, "Data\\BOSS_QUEST_%d.txt", i);

		fopen_s(&fs, Directory, "rt");

		if (!fs)
			continue;

		char line[1024];
		int e = 0;

		while (fgets(line, sizeof(line), fs))
		{
			if (*line == '\n' || *line == '#')
				continue;

			if (e == 0)
				sscanf_s(line, "%d,%d", &sServer.QuestsBosses[i].Boss.CountToBorn, &sServer.QuestsBosses[i].Boss.gennerId);
			else if (e < 11)
			{
				int count = e - 1;
				sscanf_s(line, "%hud,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhd", &sServer.QuestsBosses[i].Boss.Gifts[count].Index,
					&sServer.QuestsBosses[i].Boss.Gifts[count].Effect[0].Index, &sServer.QuestsBosses[i].Boss.Gifts[count].Effect[0].Value,
					&sServer.QuestsBosses[i].Boss.Gifts[count].Effect[1].Index, &sServer.QuestsBosses[i].Boss.Gifts[count].Effect[1].Value,
					&sServer.QuestsBosses[i].Boss.Gifts[count].Effect[2].Index, &sServer.QuestsBosses[i].Boss.Gifts[count].Effect[2].Value,
					&sServer.QuestsBosses[i].Boss.Chances[count]);
			}
			else 
				break;

			e++;
		}

		fclose(fs);
	}

	return true;
}

INT32 ReadBlockedItem()
{
	FILE *pFile;
	fopen_s(&pFile, "BlockDropItem.txt", "r");
	if(!pFile)
		return false;

	char line[1024] = { 0 };
	for (int i = 0; i < MAX_BLOCKITEM; ++i)
		g_pBlockedItem[i] = 0;

	INT32 x = 0;
	while(fgets(line, sizeof line, pFile))
	{
		// Comentário
		if(*line == '#' || *line == '\n')
			continue; 

		INT32 itemId = -1;
		INT32 ret = sscanf_s(line, "%d,%*[^\n]", &itemId);
		if(ret != 1)
			continue;

		if(itemId <= 0 || itemId >= MAX_ITEMLIST)
			continue;

		g_pBlockedItem[x++] = itemId;
		if (x >= MAX_BLOCKITEM)
			break;
	}

	fclose(pFile);
	return true;
}

void RebornKefra()
{
	Log(SERVER_SIDE, LOG_INGAME, "Recebido sinal para renascimento do Kefra");

	if(!sServer.KefraDead)
		return;

	for(INT32 i = GUARDAS_KEFRA; i < GUARDAS_KEFRA + 18; i++)
	{
		// Faz o mob não ficar nascendo após morrer 
		mGener.pList[i].MinuteGenerate = 4;

		// Gera os mobs
		GenerateMob(i, 0, 0);
	}

	// Limpa as variáveis
	sServer.KefraDead = FALSE;
	sServer.KefraKiller = 0;

	// Gera o Kefra no devido local
	GenerateMob(KEFRA, 0, 0);

	// Envia mensagem para o canal completo com o renascimento do Kefra
	SendNotice(g_pLanguageString[_NN_Kefra_Reborn]);
		
	Log(SERVER_SIDE, LOG_ADMIN, "Kefra renasceu");

	// Limpa a área
	ClearArea(3200, 1664, 3328, 1791);
}

void AcceptDailyQuest(int clientId)
{
	if (!Mob[clientId].Mobs.DailyQuest.IsAccepted)
	{
		Mob[clientId].Mobs.DailyQuest.IsAccepted = true;
		SendClientMessage(clientId, "Missão diária aceita!");

		SendSignal(clientId, clientId, 0x705);
	}
	else
	{
		SendClientMessage(clientId, "Você aceitou essa missão.");
		SendSignal(clientId, clientId, 0x705);
	}
}

bool LoadNPCEvento() 
{ 
	FILE *pFile = nullptr;
	fopen_s(&pFile, "Data\\NPCEvento.txt", "r");

	// Apaga totalmente o NPC de evento
	memset(npcEvent, 0, sizeof npcEvent);

	if(pFile != NULL)
	{
		char Text[1024];

		// Contagem dos NPCs
		int npcId = -1;

		while(fgets(Text, 1024, pFile)) 
		{
			if(Text[0] == '#')
			{
				npcId ++;
				continue;
			}

			if(Text[0] == '/' && Text[1] == '/')
				continue;

			char cmd[32], val[512];
			// Comments
			int ret = sscanf_s(Text, "%[^=]=%[^\n]", cmd, 32, val, 512);

			if(ret != 2)
				continue;

			stNPCEvent *ev = &npcEvent[npcId];
			if(!_strnicmp("itemRequired_", cmd, 13))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "itemRequired_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #1", "Falha do GM #1", 4096);

					continue;
				}

				st_Item *item = &ev->itemRequired[itemR];
				sscanf_s(val, "%hd %hhu %hhu %hhu %hhu %hhu %hhu", &item->Index, &item->EF1, &item->EFV1, &item->EF2, &item->EFV2, &item->EF3, &item->EFV3);
			}
			else if(!_strnicmp("goldEarned", cmd, 10))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "goldEarned %d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #1", "Falha do GM #1", 4096);

					continue;
				}
			}
			else if(!_strnicmp("amountRequired_", cmd, 15))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "amountRequired_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #1", "Falha do GM #1", 4096);

					continue;
				}

				ev->amountRequired[itemR] = atoi(val);
			}
			else if(!_strnicmp("goldRequired", cmd, 12))
				ev->goldRequired = atoi(val);

			else if(!_strnicmp("itemEarned_", cmd, 11))
			{
				int itemR = 0;
				int itemR2 = 0;
				int ret = sscanf_s(cmd, "itemEarned_%d_%d", &itemR, &itemR2);
				if(ret != 1 && ret != 2)
				{
					MessageBoxA(NULL, "Falha do GM #2", "Falha do GM #2", 4096);

					continue;
				}

				st_Item *item = &ev->itemEarned[itemR][itemR2];
				sscanf_s(val, "%hd %hhu %hhu %hhu %hhu %hhu %hhu", &item->Index, &item->EF1, &item->EFV1, &item->EF2, &item->EFV2, &item->EF3, &item->EFV3);
			}
			else if(!_strnicmp("itemRate_", cmd, 9))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "itemRate_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #3", "Falha do GM #3", 4096);

					continue;
				}

				ev->Rates[itemR] = atoi(val);
			}
			else if(!_strnicmp("teleport_", cmd, 9))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "teleport_%d", &itemR);
				if(ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #4", "Falha do GM #4", 4096);

					continue;
				}

				sscanf_s(val, "%hud %hud", &ev->Pos[itemR].X, &ev->Pos[itemR].Y);
			}
			else if (!_strnicmp("teleMsg_", cmd, 8))
			{
				int itemR = 0;
				int ret = sscanf_s(cmd, "teleMsg_%d", &itemR);
				if (ret != 1)
				{
					MessageBoxA(NULL, "Falha do GM #5", "Falha do GM #5", 4096);

					continue;
				}

				strncpy_s(ev->msg[itemR], val, 108);
			}
			else if (!_strnicmp("npcName", cmd, 8))
				strncpy_s(ev->npcId, val, 108);
		}

		fclose(pFile);
	}
	else 
		return false;

	return true;
}

void NumberFormat(char*result)
{
	char st[32];
	memset(st, 0, sizeof st);

	strncpy_s(st, result, 32);

	int len = strlen(st);
	int sum = ((len - 1) / 3);
	
	for(int i = (len - 1), count = 0, index = (len - 1) + sum; i >= 0; i--, count++)
	{
		if(!(count % 3) && count != 0)
		{
			result[index] = ',';
			index--;
		}

		result[index] = st[i];

		count++;
		index--;
	}

	if(len + sum < 32)
		result[len + sum] = 0;
}

void FinishColosseum()
{
	sServer.Colosseum.level = 0;
	sServer.Colosseum.inSec.boss = false;
	sServer.Colosseum.inSec.closedGate = false;
	sServer.Colosseum.inSec.closedWalls = false;
	sServer.Colosseum.inSec.wasBorn = false;

	sServer.Colosseum.npc = 0;
	sServer.Colosseum.time = 0;

	sServer.Colosseum.clients.clear();

	// Portões de entrada
	SetColoseumDoor(1);

	// Portões do meio ^^
	SetColoseumDoor2(3);

	for(INT32 i = MAX_PLAYER; i < MAX_SPAWN_MOB; i++)
	{
		if(Mob[i].Target.X >= 2608 && Mob[i].Target.X <= 2647  && Mob[i].Target.Y >= 1708 && Mob[i].Target.Y <= 1748)
			DeleteMob(i, 1);
	}

	ClearArea(2608, 1708, 2647, 1748);
}

void DoColosseum()
{
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);

	// Coliseu não está ativo ainda
	if(sServer.Colosseum.level == 0)
		return;

	// Checa se as muralhas já desceram ou não
	if(sServer.Colosseum.inSec.closedWalls == 0) 
		return;

	char szTMP[256];

	sServer.Colosseum.time -= 1;

	if(sServer.Colosseum.time <= 0)
	{
		ClearArea(2647, 1748, 2608, 1708);
		memset(&sServer.Colosseum, 0, sizeof sServer.Colosseum);

		SendNotice("Coliseu finalizado. Não foi derrotado todos os monstros dentro do tempo");

		FinishColosseum();
		Log(SERVER_SIDE, LOG_INGAME, "Coliseu finalizado. Não foi derrotado todos os monstros dentro do tempo");
		return;
	}

	switch(sServer.Colosseum.level)
	{
		// Nível 1 do Coliseu
		case 1:
		// Nível 2 do Coliseu
		case 2:
		// Nível 3 do Coliseu
		case 3:
		{
			// Começa depois de todos os boss
			int BASE_MOB = (COLOSSEUM_ID + 4) + (9 * (sServer.Colosseum.level - 1));
			if(!sServer.Colosseum.inSec.wasBorn)
			{
				// TODO : GenerateMob dos mobs
				sServer.Colosseum.inSec.wasBorn = true;

				for(int i = 0; i < 9; i++)
					GenerateMob(BASE_MOB + i, 0, 0);
			}

			int totalMob = 0;
			// Alteração do tamanho do loop para adequar-se a quantidade de mobs gerados no local
			if(!sServer.Colosseum.inSec.boss)
				for(int i = 0; i < 9; i++)
					totalMob += mGener.pList[BASE_MOB + i].MobCount;
			else
				totalMob = mGener.pList[COLOSSEUM_ID + (sServer.Colosseum.level - 1)].MobCount; 

			// TODO : Alterar o valor do '4' para o n° do loop pois cada vez que um grupo de mob acaba totalmente fica o valor '1'
			if((sServer.Colosseum.inSec.boss && totalMob == 0) || (!sServer.Colosseum.inSec.boss && totalMob == 0))
			{	
				if(sServer.Colosseum.inSec.boss)
				{
					if(sServer.Colosseum.level != 3)
						sprintf_s(szTMP, "Boss derrotado. Nível %d alcançado...", (sServer.Colosseum.level + 1));
					else
						strncpy_s(szTMP, "Proteja Tyr para continuar sua missão! Caso ele morra, o Coliseu acaba!", 108);

					sServer.Colosseum.time = 300;
					sServer.Colosseum.level += 1;

					for(int i = 1; i < MAX_PLAYER ; i ++)
					{
						if(Users[i].Status != 22)
							continue;
						
						if(Mob[i].Target.X >= 2608 && Mob[i].Target.X <= 2647 && Mob[i].Target.Y >= 1708 && Mob[i].Target.Y <= 1748) 
						{
							SendClientMessage(i, szTMP);
							SendSignalParm(i, SERVER_SIDE, 0x3A1, sServer.Colosseum.time);
						}
					}

					sServer.Colosseum.inSec.boss = false;
					
					// TODO : GenerateMob do próximo nível
					int baseMob = (COLOSSEUM_ID + 4) + (sServer.Colosseum.level - 1) * 9;
					for(int i = 0; i < 9; i++)
						GenerateMob(baseMob + i, 0, 0);

					if(sServer.Colosseum.level == 4)
						GenerateMob(COLOSSEUM_TYR, 0, 0);
				}
				else
				{
					sServer.Colosseum.time = 300;
					sServer.Colosseum.inSec.boss = true;

					for(int i = 0 ; i < MAX_PLAYER ; i ++)
					{
						if(Users[i].Status != USER_PLAY)
							continue;
						
						if(Mob[i].Target.X >= 2608 && Mob[i].Target.X <= 2647 && Mob[i].Target.Y >= 1708 && Mob[i].Target.Y <= 1748) 
							SendClientMessage(i, "Boss nasceu!");
					}

					GenerateMob(COLOSSEUM_ID + (sServer.Colosseum.level - 1), 0, 0);
				}
			}
		}
		break;
		case 4:
		{
			if(mGener.pList[COLOSSEUM_TYR].MobCount == 0)
			{
				for(int i = 0 ; i < MAX_PLAYER ; i ++)
				{
					if(Users[i].Status != USER_PLAY)
						continue;
						
					if(Mob[i].Target.X >= 2608 && Mob[i].Target.X <= 2647 && Mob[i].Target.Y >= 1708 && Mob[i].Target.Y <= 1748) 
						SendClientMessage(i, "Coliseu finalizado. Tyr foi derrotado!!");
				}
				
				FinishColosseum();
				return;
			}

			// Começa depois de todos os boss
			int BASE_MOB = (COLOSSEUM_ID + 4) + (9 * (sServer.Colosseum.level - 1));

			int totalMob = 0;
			// Alteração do tamanho do loop para adequar-se a quantidade de mobs gerados no local
			if(!sServer.Colosseum.inSec.boss)
				for(int i = 0; i < 9; i++)
					totalMob += mGener.pList[BASE_MOB + i].MobCount;
			else
				totalMob = mGener.pList[COLOSSEUM_ID + (sServer.Colosseum.level - 1)].MobCount; 
			
			// TODO : Alterar o valor do '4' para o n° do loop pois cada vez que um grupo de mob acaba totalmente fica o valor '1'
			if((sServer.Colosseum.inSec.boss && totalMob == 0) || (!sServer.Colosseum.inSec.boss && totalMob == 0))
			{
				if(sServer.Colosseum.inSec.boss)
				{
					sServer.Colosseum.time = 300;
					sServer.Colosseum.level += 1;

					sServer.Colosseum.inSec.boss = false;
					FinishColosseum();
				}
				else
				{
					sServer.Colosseum.time = 300;
					sServer.Colosseum.inSec.boss = true;

					for(int i = 1 ; i < MAX_PLAYER; i ++)
					{
						if(Users[i].Status != 22)
							continue;
						
						if(Mob[i].Target.X >= 2608 && Mob[i].Target.X <= 2647 && Mob[i].Target.Y >= 1708 && Mob[i].Target.Y <= 1748) 
							SendClientMessage(i, "Último Boss nasceu!");
					}

					GenerateMob(COLOSSEUM_ID + (sServer.Colosseum.level - 1), 0, 0);
				}
			}

		}
		break;
	}
}

void DropEventOnHit(INT32 clientId, INT32 targetIdx) 
{
	if(!sServer.BossEvent.Status)
		return;
	
	INT32 rate  = sServer.BossEvent.Rate,
		  bonus = sServer.BossEvent.Bonus;

	for(INT32 i = 0; i < 32; i++)
	{
		if(Mob[clientId].Mobs.Affects[i].Index == 56)
		{
			rate += bonus;

			break;
		}
	}

	INT32 _rand = Rand() % 15000;
	if(_rand >= rate)
		return ;

	INT32 slotId = GetFirstSlot(clientId, 0);
	if(slotId == -1)
	{
		SendClientMessage(clientId, "!Sem espaço para receber o item do evento");

		return;
	}

	if(PutItem(clientId, &sServer.BossEvent.item))
	{
		Log(clientId, LOG_INGAME, "Recebeu o [%s] [%d] [%d %d %d %d %d %d] do evento. %d/%d", ItemList[sServer.BossEvent.item.Index].Name,
			sServer.BossEvent.item.Index, sServer.BossEvent.item.EF1, sServer.BossEvent.item.EFV1, sServer.BossEvent.item.EF2, sServer.BossEvent.item.EFV2,
			sServer.BossEvent.item.EF3, sServer.BossEvent.item.EFV3, _rand, rate);
	}
	else
	{
		SendClientMessage(clientId, "!Sem espaço para receber o item do evento");

		return;
	}
	
	sServer.BossEvent.Count ++;
}

void PremiumTime() 
{
	if(sServer.PremiumTime.Second <= 0 || sServer.PremiumTime.Interval <= 0)
		return;

	sServer.PremiumTime.Second ++;
	if(sServer.PremiumTime.Second - sServer.PremiumTime.Last < sServer.PremiumTime.Interval)
		return;

	sServer.PremiumTime.Last = sServer.PremiumTime.Second;

	INT16 players[MAX_PLAYER];
	memset(&players[0], 0, sizeof players);

	INT32 count = 0;
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(Users[i].Status != USER_PLAY)
			continue;

		players[count] = i;
		count++;

		if(count >= MAX_PLAYER)
			break;
	}

	INT32 totalLoop = 0;
	while(true)
	{
		if(count <= 0)
			break;

		totalLoop ++;
		if(totalLoop >= 500)
			break;

		INT32 _rand = Rand() % (count + 1);
		if(_rand >= count)
			_rand = Rand() % count;

		INT32 clientId = players[_rand];
		if(Users[clientId].Status != USER_PLAY)
			continue;

		st_Mob *player = &Mob[clientId].Mobs.Player;
		if(sServer.PremiumTime.Gold != 0)
		{
			if(sServer.PremiumTime.Gold + player->Gold > 2000000000)
				continue;
		}
				
		INT32 slotId = -1;
		if(sServer.PremiumTime.Item.Index != 0)
		{
			slotId = GetFirstSlot(clientId, 0);
			if(slotId == -1)
				continue;
		}

		char szTMP[256];

		player->Gold += sServer.PremiumTime.Gold;
		SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

		if(slotId != -1)
		{
			memset(&player->Inventory[slotId], 0, sizeof st_Item);
			memcpy(&player->Inventory[slotId], &sServer.PremiumTime.Item, sizeof st_Item);
					
			SendItem(clientId, SlotType::Inv, slotId, &sServer.PremiumTime.Item);
		}

		if(sServer.PremiumTime.Gold != 0 && sServer.PremiumTime.Item.Index != 0)
			sprintf_s(szTMP, "O usuário %s ganhou %d de Gold e o item %s", player->Name, sServer.PremiumTime.Gold, ItemList[sServer.PremiumTime.Item.Index].Name);
		else if(sServer.PremiumTime.Gold != 0)
			sprintf_s(szTMP, "O usuário %s ganhou %d de Gold", player->Name, sServer.PremiumTime.Gold);
		else if(sServer.PremiumTime.Item.Index != 0)
			sprintf_s(szTMP, "O usuário %s ganhou o item %s", player->Name, ItemList[sServer.PremiumTime.Item.Index].Name);

		SendNotice(szTMP);
		Log(SERVER_SIDE, LOG_INGAME, szTMP);
		break;
	}
}

void AnswerClient_aHack( INT32 counter )
{
	for(INT32 i = 1; i < MAX_PLAYER; i++ )
	{
		CUser *user = &Users[i];
		if(user->Status != USER_PLAY)
			continue;

		INT32 last			= user->aHack.Last,
			  next			= user->aHack.Next,
			  response		= user->aHack.Response,
			  question		= user->aHack.Question,
			  error			= user->aHack.Error;

		if(error >= 3)
		{
			Log(i, LOG_INGAME, "ANTIHACK - Desconectado por máximo de erros atingido. Errors: %d", error);
			Log(SERVER_SIDE, LOG_INGAME, "ANTIHACK - %s : Desconectado por máximo de erros atingido. Errors: %d", user->User.Username, error);

			//CloseUser(i);
			continue;
		}

		// Checa se está na fase de solicitação de algo para o cliente
		if(response == 0) 
		{
			// CHeca se há necessidade de solicitação de um novo pacote
			if(counter - last < next)
				continue;

			INT32 requested = Rand() % 2;
			
			pMsgSignal packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0x555;
			packet.Header.Size     = sizeof packet;
			packet.Header.ClientId = i;

			packet.Value = requested;

			Users[i].AddMessage((BYTE*)&packet, sizeof packet);

			//
			user->aHack.Response = 1; // aguardando
			user->aHack.Question = requested;
			user->aHack.Next     = 20;
			user->aHack.Last     = counter;
		}
		// Checa se está na fase de checar o tempo de resposta do cliente
		// Para saber se deu tudo certo, se respondeu e tal : ) 
		else if(response == 1)
		{
			if(counter - last < next)
				continue;

			// 20 segundos depois não respondeu vai levar um fucking DC, que se foda
			Log(i, LOG_INGAME, "Não respondeu ao pacote de solicitação. Requested: %d", user->aHack.Question);

			//
			user->aHack.Error ++;
			user->aHack.Response = 0; // aguardando
			user->aHack.Question = -1;
			user->aHack.Next     = 15;
			user->aHack.Last     = counter;
		}
	}
}

INT32 RemoveAmount(INT32 clientId, INT32 itemId, INT32 amount) 
{
	INT32 retn = 0;
	for(INT32 LOCAL_832 = 0; LOCAL_832 < 60 ; LOCAL_832++)
	{
		if(Mob[clientId].Mobs.Player.Inventory[LOCAL_832].Index == itemId)
		{
			int totalRemoved = 0;
			while(Mob[clientId].Mobs.Player.Inventory[LOCAL_832].Index == itemId)
			{
				AmountMinus(&Mob[clientId].Mobs.Player.Inventory[LOCAL_832]);
				totalRemoved++;

				amount--;
				retn  ++;
				if(amount <= 0)
					break;
			}

			SendItem(clientId, SlotType::Inv, LOCAL_832, &Mob[clientId].Mobs.Player.Inventory[LOCAL_832]);
			Log(clientId, LOG_INGAME, "Removido %d unidade(s) de %s %s no slot %d", totalRemoved, ItemList[itemId].Name, Mob[clientId].Mobs.Player.Inventory[LOCAL_832].toString().c_str(), LOCAL_832);
		}

		if(amount <= 0)
			break;
	}

	return retn;
}

BOOL ReadMerchantStore()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Data\\Store.txt", "r");
	memset(&sServer.PremierStore, 0, sizeof sServer.PremierStore);

	if (pFile)
	{
		char tmp[256];
		INT32 count = 0;
		while (fgets(tmp, sizeof(tmp), pFile))
		{
			if (tmp[0] == '#' || tmp[0] == '\n')
				continue;

			if (count >= MAX_PREMIERSTORE)
				break;

			st_Item item;
			memset(&item, 0, sizeof st_Item);

			INT32 price = 0;
			INT32 ret = sscanf_s(tmp, R"(%d,%hd,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)", &price, &item.Index, &item.EF1, &item.EFV1,
				&item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

			if (price <= 0 || price >= 10000)
				continue;

			if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
				continue;

			memcpy(&sServer.PremierStore.item[count], &item, sizeof st_Item);
			sServer.PremierStore.Price[count] = price;

			count++;
		}

		fclose(pFile);
	}

	return false;
}

BOOL ReadArenaStore()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Data\\ArenaStore.txt", "r");
	memset(&sServer.ArenaStore, 0, sizeof sServer.ArenaStore);

	if (pFile)
	{
		char tmp[256];
		INT32 count = 0;
		while (fgets(tmp, sizeof(tmp), pFile))
		{
			if (tmp[0] == '#' || tmp[0] == '\n')
				continue;

			if (count >= MAX_PREMIERSTORE)
				break;

			st_Item item;
			memset(&item, 0, sizeof st_Item);

			INT32 price = 0;
			INT32 ret = sscanf_s(tmp, R"(%d,%hd,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)", &price, &item.Index, &item.EF1, &item.EFV1,
				&item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

			if (price <= 0 || price >= 10000)
				continue;

			if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
				continue;

			memcpy(&sServer.ArenaStore.item[count], &item, sizeof st_Item);
			sServer.ArenaStore.Price[count] = price;

			count++;
		}

		fclose(pFile);
	}

	return false;
}

void LevelItem(INT32 clientId) 
{
	INT32 level = Mob[clientId].Mobs.Player.Status.Level; 
	INT32 evolution = Mob[clientId].Mobs.Player.Equip[0].EFV2;

	for(auto const item : sServer.levelItem)
	{
		auto itemL = &item;
		if(itemL->item.Index <= 0 || itemL->item.Index >= MAX_ITEMLIST || itemL->Level != level || evolution != itemL->Evolution)
			continue;

		INT32 type   = itemL->Type,
			  classe = itemL->Classe; 

		if (evolution >= ARCH)
		{
			// Se for != -1, quer dizer que ele checa a classe
			if (type != -1 && classe != GetInfoClass(Mob[clientId].Mobs.Player.Equip[0].EF2))
				continue;
		}
		else
		{
			// Se for != -1, quer dizer que ele checa a classe
			if (type != -1 && classe != Mob[clientId].Mobs.Player.ClassInfo)
				continue;
		}
		if(type >= 1  && type <= 3)
		{
			if(type == 1)
			{
				if((Mob[clientId].Mobs.Player.Status.STR + Mob[clientId].Mobs.Player.Status.DEX) <= Mob[clientId].Mobs.Player.Status.INT)
					continue;
			}
			else if(type == 2)
			{
				if(Mob[clientId].Mobs.Player.Status.INT <= Mob[clientId].Mobs.Player.Status.STR + Mob[clientId].Mobs.Player.Status.DEX)
					continue;
			}
			else if(type == 3)
			{
				if(Mob[clientId].Mobs.Player.Status.DEX <= Mob[clientId].Mobs.Player.Status.STR)
					continue;
			}
		}

		INT32 slotId = GetFirstSlot(clientId, 0);
		bool isBank = false;
		if (slotId == -1)
		{
			isBank = true;

			for (int i = 0; i < 120; ++i) 
			{
				if (Users[clientId].User.Storage.Item[i].Index <= 0)
				{
					slotId = i;

					break;
				}
			}
		}

		if(slotId == -1)
		{
			SendClientMessage(clientId, "!Sem espaço no banco para receber o item!");

			Log(clientId, LOG_INGAME, "Não recebeu o item por level por falta de espaço no banco. ItemID: %s.", itemL->item.toString().c_str());
			continue;
		}

		Log(clientId, LOG_INGAME, "Recebeu o item %s %s. STR: %d. INT: %hu. DEX: %hu. CON: %hu",
			ItemList[itemL->item.Index].Name,
			itemL->item.toString().c_str(),
			Mob[clientId].Mobs.Player.Status.STR,
			Mob[clientId].Mobs.Player.Status.INT,
			Mob[clientId].Mobs.Player.Status.DEX,
			Mob[clientId].Mobs.Player.Status.CON
		);

		if (!isBank)
		{
			Mob[clientId].Mobs.Player.Inventory[slotId] = itemL->item;
			SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);

			LogPlayer(clientId, "Recebeu o item %s por chegar ao nível %d", ItemList[itemL->item.Index].Name, level + 1);
		}
		else
		{
			Users[clientId].User.Storage.Item[slotId] = itemL->item;
			SendItem(clientId, SlotType::Storage, slotId, const_cast<st_Item*>(&itemL->item));

			LogPlayer(clientId, "Recebeu o item %s por chegar ao nível %d (recompensa chegou ao baú)", ItemList[itemL->item.Index].Name, level + 1);
		}

		SendClientMessage(clientId, "Chegou um item: [ %s ]", ItemList[itemL->item.Index].Name);
	}
}

BOOL ReadLevelItem() 
{
	FILE *pFile = NULL;
	std::fill(std::begin(sServer.levelItem), std::end(sServer.levelItem), stLevelItem{});

	fopen_s(&pFile, "Data//LevelItem.txt", "r");
	if(pFile)
	{
		char tmp[256];

		while(fgets(tmp, sizeof tmp, pFile))
		{
			if(tmp[0] == '#' || tmp[0] == '\n')
				continue;

			INT32 evolution     = -1,
				  type   = -1,
				  classe = -1,
				  level  = -1,
				  itemId = -1,
				  ef1 = 0, efv1 = 0, ef2 = 0, efv2 = 0, ef3 = 0, efv3 = 0;

			st_Item item{};

			INT32 ret = sscanf_s(tmp, "%d %d %d %d %d %d %d %d %d %d %d", &evolution, &level, &classe, &type,
				&itemId, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);//&item.Index, &item.EF1, &item.EFV1, &item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

			if(ret < 5)
				continue;

			sServer.levelItem.push_back(stLevelItem{});

			auto& levelItem = sServer.levelItem.back();
			levelItem.Classe = classe;
			levelItem.Level  = level;
			levelItem.Type   = type;
			levelItem.Evolution   = evolution;
			
			item.Index					 = itemId;
			item.EF1					 = ef1;
			item.EFV1					 = efv1;
			item.EF2					 = ef2;
			item.EFV2					 = efv2;
			item.EF3					 = ef3;
			item.EFV3					 = efv3;

			levelItem.item = item;
		}

		fclose(pFile);
		return true;
	}

	return false;
}

BOOL ReadDailyReward()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "Data//DailyReward.txt", "r");
	if(pFile)
	{
		char tmp[256];

		while(fgets(tmp, sizeof tmp, pFile))
		{
			if(tmp[0] == '#' || tmp[0] == '\n')
				continue;

			INT32 day	   = -1,
				  line	   = -1,
				  itemId = -1,
				  ef1 = 0, efv1 = 0, ef2 = 0, efv2 = 0, ef3 = 0, efv3 = 0;

			st_Item item;
			memset(&item, 0, sizeof st_Item);
			
			INT32 ret = sscanf_s(tmp, "%d %d %d %d %d %d %d %d %d", &day, &line,
				&itemId, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);//&item.Index, &item.EF1, &item.EFV1, &item.EF2, &item.EFV2, &item.EF3, &item.EFV3);

			if (ret < 3)
				continue;
			
			item.Index					 = itemId;
			item.EF1					 = ef1;
			item.EFV1					 = efv1;
			item.EF2					 = ef2;
			item.EFV2					 = efv2;
			item.EF3					 = ef3;
			item.EFV3					 = efv3;

			memcpy(&sServer.Daily.Item[day][line], &item, sizeof st_Item);
		}

		fclose(pFile);
		return true;
	}

	return false;

}

void SendDailyRewardInfo(INT32 clientId)
{
	stAccount *account = &Users[clientId].User;
	
	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);
	
	INT32 weekYear  = getWeek(now.tm_mday, now.tm_mon, now.tm_year);

	if(weekYear != account->Daily.WeekYear)
	{
		memset(&account->Daily.Received[0], 0, sizeof(bool) * 7);

		account->Daily.WeekYear	= weekYear;
	}

	MSG_DAILYREWARDINFO packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = DailyRewardInfoPacket;
	packet.Header.ClientId = clientId;
	packet.Header.Size     = sizeof packet;

	memcpy(&packet.Received[0], &account->Daily.Received[0], sizeof(bool) * 7);
	memcpy(&packet.Item, &sServer.Daily.Item, sizeof st_Item * 7 * 4);

	packet.Day = now.tm_wday;

	Users[clientId].AddMessage((BYTE*)&packet, sizeof packet);
}

bool ReadBoss()
{
	const std::string filename = "Data\\Boss.xml";
	if (!std::filesystem::exists(filename))
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_full);
	if (!result)
	{
		std::cout << "parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (doc.text().as_string() + result.offset) << "]\n\n";

		return false;
	}

	sServer.Boss.fill(TOD_MobBoss{});

	auto bossesNode = doc.child("bosses");

	{
		int count = 0;
		for (auto bossNode = bossesNode.child("boss"); bossNode; bossNode = bossNode.next_sibling("boss"))
		{
			auto& boss = sServer.Boss[count];

			boss.Index = count++;
			boss.Fame = std::stoi(bossNode.child_value("fame"));
			boss.TimeToReborn = std::stoi(bossNode.child_value("timeToReborn"));
			boss.MaxTimeIngame = std::stoi(bossNode.child_value("maxTimeIngame"));

			for (auto genersNode = bossNode.child("geners"); genersNode; genersNode = genersNode.next_sibling("geners"))
				boss.Geners.emplace_back(std::stoi(genersNode.child_value("gener")));

			if (count >= MaxBoss)
				break;
		}
	}

	return true;
}

void LogGold(INT32 clientId)
{
	if(clientId > 0 && clientId < MAX_PLAYER)
	{
		if(Users[clientId].Gold == 0)
			return;;

		INT32 total = Users[clientId].GoldCount;

		Log(clientId, LOG_INGAME, "Recebeu %d de gold em %d mobs no total. Total de gold: %d", Users[clientId].Gold, total, Mob[clientId].Mobs.Player.Gold);
		LogPlayer(clientId, "Recebeu %d de gold matando %d mobs.", Users[clientId].Gold, total);

		Users[clientId].Gold = 0;
		Users[clientId].GoldCount = 0;
	}
}

void GroupTransfer(INT32 clientId, INT32 mobId)
{
	CMob *newLider = &Mob[mobId];

	int clueParty{ 0 };
	int clueSanc{ 0 };
	bool isClueLeader{ false };
	// itera sobre todas as pistas
	for (int iPista = 0; iPista < 10; ++iPista)
	{
		// itera sobre todos os grupos
		for (int iParty = 0; iParty < MAX_PARTYPISTA; ++iParty)
		{
			auto& pista = pPista[iPista].Clients[iParty];

			int memberId = pista[12];
			if (memberId > 0 && memberId < MAX_PLAYER && clientId == memberId)
			{
				clueParty = iParty;
				clueSanc = iPista;

				isClueLeader = true;
				break;
			}
		}

		if (isClueLeader)
			break;
	}

	if (isClueLeader)
	{
		auto& pista = pPista[clueSanc].Clients[clueParty];
		pista[12] = mobId;

		for (int i = 0; i < 12; ++i)
		{
			if (pPista[clueSanc].Clients[clueParty][i] == mobId)
			{
				pPista[clueSanc].Clients[clueParty][i] = clientId;

				break;
			}
		}

		for (int i = 0; i < 13; ++i)
		{
			int memberId = pista[i];
			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			SendClientMessage(memberId, "O líder do grupo da pista foi passado para %s", newLider->Mobs.Player.Name);
			Log(memberId, LOG_INGAME, "Líder passado para o usuário %s (%s)", newLider->Mobs.Player.Name, Users[mobId].User.Username);
		}

        Log(clientId, LOG_INGAME, "O líder da pista foi passado para o usuário %s (%s)", newLider->Mobs.Player.Name, Users[mobId].User.Username);
	}

	// Salva a info do grupo no novo líder
	memcpy(newLider->PartyList, &Mob[clientId].PartyList[0], sizeof INT16 * 12);

	for(INT32 i = 0; i < 12; i++)
	{
		if(mobId == newLider->PartyList[i])
		{
			newLider->PartyList[i] = clientId;

			break;
		}
	}	

	// Apaga o grupo
	memset(Mob[clientId].PartyList, 0, sizeof INT16 * 12);

	// Informa o novo lider de sua condição
	newLider->Leader = 0;
	
	// Informa ao usuário ex-líder de quem é o novo líder
	Mob[clientId].Leader = mobId;

	// Adiciona ele de novo como lider, provavelmente.
	SendAddParty(mobId, mobId, 1);

	// Adiciona o antigo líder como membro normal no grupo
	SendAddParty(clientId, clientId, 0);
	SendAddParty(clientId, mobId, 1);
	SendAddParty(mobId, clientId, 0);
	for (INT32 iterator = 0; iterator < 12; iterator++)
	{
		int partyMob = newLider->PartyList[iterator];
		if (partyMob <= 0 || partyMob > MAX_SPAWN_MOB)
			continue;

		CMob *mob = &Mob[partyMob];

		mob->Leader = mobId;

		SendAddParty(partyMob, clientId, 0);
		SendAddParty(partyMob, mobId, 1);
	}
}

TOD_SellItemResult SellItem(int clientId, st_Item* item)
{
	bool canSell = true;
	switch (item->Index)
	{
	case 509:
	case 3993:
	case 3994:
	case 747:
		canSell = false;
		break;
	}

	bool sellAmount = false;
	switch (item->Index)
	{
	case 419:
	case 420:
	case 412:
	case 413:
		sellAmount = true;
		break;
	}

	auto _volatile = GetItemAbility(item, EF_VOLATILE);
	if (!canSell || _volatile == 1 || _volatile == 191)
		return TOD_SellItemResult::BlockedItem;

	INT32 cityZone = GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y);
	if (cityZone == 5)
		cityZone = 4;

	INT32 perc_impost = g_pCityZone[cityZone].perc_impost;
	INT32 impost = 0;

	sItemData* rItem = &ItemList[item->Index];
	INT32 itemPrice = (rItem->Price / 4);

	if (itemPrice >= 5001 && itemPrice <= 10000)
		itemPrice = itemPrice * 2 / 3;
	else if (itemPrice > 10000)
		itemPrice /= 2;

	if (sellAmount)
		itemPrice *= GetItemAmount(item);

	if (perc_impost != 0)
	{
		if (sellAmount)
			perc_impost /= 2;

		impost = (itemPrice * perc_impost / 100);
		itemPrice = (itemPrice - impost);
	}

	st_Mob* mob = &Mob[clientId].Mobs.Player;
	INT64 totalGold = static_cast<INT64>(mob->Gold) + static_cast<INT64>(itemPrice);
	if (totalGold > 2000000000 || totalGold < 0)
		return TOD_SellItemResult::GoldLimit;

	// Arrecada o imposto da cidade
	g_pCityZone[cityZone].impost += impost;

	mob->Gold += itemPrice;

	Log(clientId, LOG_INGAME, "Vendeu o item %s [%d] [%d %d %d %d %d %d] por %d. Gold atual: %d. Valor do imposto: %d", rItem->Name, item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, itemPrice, Mob[clientId].Mobs.Player.Gold, impost);
	LogPlayer(clientId, "Vendeu o item %s na loja %s por %d", rItem->Name, mob->Name, itemPrice, impost);

	memset(item, 0, sizeof st_Item);

	if (impost >= 2000000)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Imposto maior que 2milhões.");
		Log(SERVER_SIDE, LOG_INGAME, "%s - Vendeu o item %s [%d] [%d %d %d %d %d %d] por %d. Gold atual: %d",Users[clientId].User.Username, rItem->Name, item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, itemPrice, Mob[clientId].Mobs.Player.Gold);
	}

	SendSignalParm(clientId, clientId, 0x3AF, Mob[clientId].Mobs.Player.Gold);
	return TOD_SellItemResult::Success;
}

bool AgroupItem(int clientId, st_Item* srcItem, st_Item* dstItem)
{
	if (srcItem == NULL || dstItem == NULL || srcItem == dstItem)
		return false;

	if (srcItem->Index != dstItem->Index)
		return false;

	if (srcItem->Index == 4685)
	{
		dstItem->Index = 4641;

		Log(clientId, LOG_INGAME, "Juntou 2 %s por %s", ItemList[4685].Name, ItemList[4641].Name);
		*srcItem = st_Item{};
		return true;
	}

	int max = GetMaxAmountItem(srcItem);
	if (max <= 0)
		return false;

	if (srcItem->Index >= 2390 && srcItem->Index <= 2419)
	{
		for (int i = 0; i < 3; i++)
		{
			if (srcItem->Effect[i].Index == 210)
				return false;

			if (dstItem->Effect[i].Index == 210)
				return false;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (srcItem->Effect[i].Index == 200)
			return false;
		if (dstItem->Effect[i].Index == 200)
			return false;
	}

	int amountSrc = 0;
	for (int i = 0; i < 3; i++)
		if (srcItem->Effect[i].Index == EF_AMOUNT)
			amountSrc += srcItem->Effect[i].Value;

	int amountDst = 0;
	for (int i = 0; i < 3; i++)
		if (dstItem->Effect[i].Index == EF_AMOUNT)
			amountDst += dstItem->Effect[i].Value;

	if (amountSrc >= max || amountDst >= max)
		return false;

	Log(clientId, LOG_INGAME, "Agrupando o item %s %s com o item %s %s", ItemList[srcItem->Index].Name, srcItem->toString().c_str(), ItemList[dstItem->Index].Name, dstItem->toString().c_str());

	if (amountSrc == 0)
		amountSrc = 1;

	if (amountDst == 0)
		amountDst = 1;

	int itemIndex = dstItem->Index;
	if ((amountSrc + amountDst) <= max)
	{
		memset(srcItem, 0, sizeof st_Item);

		st_Item item;
		memset(&item, 0, sizeof st_Item);

		item.Index = itemIndex;

		for (int i = 0; i < 3; i++)
		{
			if (dstItem->Effect[i].Index == EF_AMOUNT)
				continue;
			if (dstItem->Effect[i].Index == EF_UNIQUE)
				continue;

			item.Effect[i].Index = dstItem->Effect[i].Index;
			item.Effect[i].Value = dstItem->Effect[i].Value;
		}
		int i = 0;
		for (; i < 3; i++)
		{
			if (item.Effect[i].Index != 0)
				continue;

			item.Effect[i].Index = EF_AMOUNT;
			item.Effect[i].Value = (amountSrc + amountDst);
			break;
		}

		if (i == 3)
			return false;

		memcpy(dstItem, &item, sizeof st_Item);
	}
	else
	{
		int amount = max - amountDst;
		amountSrc -= amount;

		for (int i = 0; i < 3; i++)
		{
			if (dstItem->Effect[i].Index == EF_AMOUNT)
			{
				dstItem->Effect[i].Value = max;
				break;
			}
		}

		int i = 0;
		for (i = 0; i < 3; i++)
		{
			if (srcItem->Effect[i].Index == EF_AMOUNT)
			{
				srcItem->Effect[i].Value = amountSrc;
				break;
			}
		}

		if (i == 3)
		{
			for (i = 0; i < 3; i++)
			{
				if (srcItem->Effect[i].Index == 59)
				{
					srcItem->Effect[i].Index = EF_AMOUNT;
					srcItem->Effect[i].Value = amountSrc;
					break;
				}
			}
		}
	}

	return true;
}

int AbsorveDamageByPet(CMob* player, int damage) 
{
	if (!player->isPetAlive())
		return damage;

	const st_Item& item = player->Mobs.Player.Equip[14];
	int absPerc = 25;

	if (item.Index == 2376) // fenrir
		absPerc = 27;
	else if (item.Index == 2377) // dragão feio
		absPerc = 34;
	else if (item.Index == 2379) // tf
		absPerc = 35;
	else if (item.Index == 2380) // dragão verm.
		absPerc = 35;
	else if (item.Index == 2381 || item.Index == 2382 || item.Index == 2383) // unisus, uni, pegasus
		absPerc = 29;
	else if (item.Index == 2384 || item.Index == 2385 || item.Index == 2386)
		absPerc = 31;
	else if (item.Index == 2387 || item.Index == 2388)
		absPerc = 28;

	int level = item.Effect[1].Index;
	if (level >= 120)
		absPerc += (level - 120);

	auto damageWithAbs = damage - (damage * absPerc / 100);
	if (damageWithAbs < 1)
		damageWithAbs = 1;

	return damageWithAbs;
}

bool LoadChristmasMission()
{
	FILE *pFile = NULL;
	fopen_s(&pFile, "Data\\ChristmasMission.txt", "rt");

	if (!pFile)
		return false;

	TOD_ChristmasMissionInfo* last = nullptr;
	sServer.Christmas.Missions.clear();

	char szTMP[256] = { 0 };
	while (fgets(szTMP, 256, pFile))
	{
		if (*szTMP == '\n')
			continue;

		if (*szTMP == '#')
		{
			sServer.Christmas.Missions.push_back(TOD_ChristmasMissionInfo{});
			last = &sServer.Christmas.Missions.back();
		}

		char cmd[128] = { 0 }, val[128] = { 0 };
		int ret = sscanf_s(szTMP, "%[^=]=%[^\n]", cmd, 127, val, 127);
		if (ret != 2)
			continue;

		if (last == nullptr)
			continue;

		std::string command{ cmd };

		int index;
		if (sscanf_s(cmd, "MobName_%d", &index) == 1)
		{
			if (index >= 0 && index < 4)
				last->MobName[index] = val;
		}
		else if (sscanf_s(cmd, "MobTotal_%d", &index) == 1)
		{
			if (index >= 0 && index < 4)
				last->Count[index] = std::stoi(val);
		}
		else if (sscanf_s(cmd, "Reward_%d", &index) == 1)
		{
			if (index >= 0 && index < 4)
			{
				st_Item item{};
				int retnItem = sscanf_s(val, "%hu %hhu %hhu %hhu %hhu %hhu %hhu",
					&item.Index,
					&item.Effect[0].Index, &item.Effect[0].Value,
					&item.Effect[1].Index, &item.Effect[1].Value,
					&item.Effect[2].Index, &item.Effect[2].Value);

				if (retnItem == 0)
					continue;

				memcpy(&last->Reward[index], &item, sizeof st_Item);
			}
		}
		else if (command == "Id")
			last->Id = std::stoi(val);
		else if (command == "Title")
			last->Title = val;
	}

	fclose(pFile);
	return true;
}

void EnergizeEmptyRune(int clientId, const p376 *packet)
{
	st_Item* srcItem = GetItemPointer(clientId, packet->SrcType, packet->SrcSlot);
	st_Item* dstItem = GetItemPointer(clientId, packet->DstType, packet->DstSlot);

	if (srcItem == nullptr || dstItem == nullptr)
		return;

	if (srcItem->Index < 5110 || srcItem->Index > 5133 || dstItem->Index != 4854)
		return;

	int runeIndex = srcItem->Index - 5109;
	int learn = *(int*)&dstItem->Effect[1].Index;
	if (learn & (1 << runeIndex))
	{
		SendClientMessage(clientId, "Esta Runa já foi utilizada para energização");

		return;
	}

	int energy = 0;
	for (INT8 i = 0; i < 3; i++)
	{
		if (dstItem->Effect[i].Index == EF_AMOUNT)
		{
			energy = dstItem->Effect[i].Value;

			break;
		}
	}

	energy += 5;
	SetItemAmount(dstItem, energy);

	*(int*)&dstItem->Effect[1].Index |= (1 << runeIndex);

	Log(clientId, LOG_INGAME, "Energizou a %s com a %s. Energia total: %d", ItemList[dstItem->Index].Name, ItemList[srcItem->Index].Name, energy);
	*srcItem = st_Item{};

	SendItem(clientId, (SlotType)packet->SrcType, packet->SrcSlot, srcItem);
	SendItem(clientId, (SlotType)packet->DstType, packet->DstSlot, dstItem);
}