#include "cServer.h"
#include "Basedef.h"
#include "GetFunc.h"
#include <sstream>
#include "UOD_Arena.h"
#include "UOD_DroplistGenerator.h"

typedef struct
{
	char Name[16];
	st_Position Position;

	INT32 Level;
	INT32 MaxHp;
	INT64 Exp;

	//
	st_Item item[30];
} stDropList;

INT32 CheckName(stDropList *dropList, char *name, INT32 total)
{
	for(INT32 i = 0; i < total; i++)
	{
		if(!_stricmp(name, dropList[i].Name))
			return true;
	}

	return false;
}

void Organize(stDropList *dropList, INT32 total)
{
	stDropList *newDropList = (stDropList*)malloc(sizeof stDropList * 5000);
	memset(newDropList, 0, sizeof stDropList * 5000);

	INT32 count = 0;
	INT32 index = -1;

	for(INT32 i = 0; i < total; i++)
	{
		index = i;

		if(!dropList[index].Name[0] || strlen(dropList[index].Name) <= 2)
			continue;

		stDropList list;
		memcpy(&list, &dropList[index], sizeof stDropList);

		for(INT32 t = (i + 1); t < total; t++)
		{
			if(!dropList[t].Name[0] || strlen(dropList[t].Name) <= 2)
				continue;

			if(strcoll(dropList[index].Name, dropList[t].Name) > 0)
			{
				memcpy(&list, &dropList[t], sizeof stDropList);
				index = t;
			}
		}
		
		memcpy(&newDropList[count], &list, sizeof stDropList);
		memset(&dropList[index], 0, sizeof stDropList);

		count ++;
		i = 0;
	}

	memcpy(dropList, newDropList, sizeof stDropList * total);
}

void GenerateDropList() 
{
	stDropList *droplist = (stDropList*)malloc(sizeof stDropList * 5000);
	memset(droplist, 0, sizeof stDropList * 5000);
	INT32 total = 0;
	FILE *pFile = NULL;
	fopen_s(&pFile, "droplist.bin", "wb+");

	if(pFile)
	{
		for(INT32 i = 0; i < mGener.numList; i++)
		{
			char name[16];
			strncpy_s(name, mGener.pList[i].Leader.Name, 16);
			
			for(INT32 u = 0; u < 16; u++)
				if(name[u] == '\'')
					name[u] = ' ';

			name[15] = '\0';

			INT32 len = strlen(name);
			for(INT32 u = len; u > 0; u--)
			{
				if(isalnum(name[u]) || name[u] == '-' || name[u] == '[' || name[u] == ']')
					break;

				if(name[u] == '_')
					name [u] = 0;
			}
			
			if(mGener.pList[i].MinuteGenerate == -1)
				continue;

			if(CheckName(droplist, name, total))
				continue;
		
			INT32 slot = 0;
			st_Mob *mob = &mGener.pList[i].Leader;
			for(INT32 t = 0; t < 64; t++)
			{
				if(mob->Inventory[t].Index <= 0 || mob->Inventory[t].Index >= MAX_ITEMLIST)
					continue;

				INT32 p;
				for(p = 0; p < slot; p++)
				{
					if(droplist[total].item[p].Index == mob->Inventory[t].Index)
						break;
				}

				if(p != slot)
					continue;

				for(p=0 ; p < MAX_BLOCKITEM; p++)
				{
					if(g_pBlockedItem[p] == mob->Inventory[t].Index)
						break;
				}

				if(p != MAX_BLOCKITEM)
					continue;

				memcpy(&droplist[total].item[slot], &mob->Inventory[t], sizeof st_Item);
				slot++;
			}

			if(slot <= 0)
			{
				memset(&droplist[total], 0, sizeof stDropList);

				continue;
			}
			
			droplist[total].Level = mob->bStatus.Level;
			droplist[total].MaxHp = mob->bStatus.maxHP;
			
			INT32 itemId = mob->Equip[13].Index;
			if(itemId == 786 || itemId == 1936 || itemId == 1937)
			{
				INT32 hpItemSanc = GetItemSanc(&mob->Equip[13]); // local342
				if(hpItemSanc < 2)
					hpItemSanc = 2;
					
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

				droplist[total].MaxHp *= (hpItemSanc * multHP);
			}

			droplist[total].Exp		   = mob->Exp;
			droplist[total].Position.X = mGener.pList[i].Segment_X[0];
			droplist[total].Position.Y = mGener.pList[i].Segment_Y[0];

			strncpy_s(droplist[total].Name, name, 16);

			total ++;
		}

		printf("Finalizado com %d adicionados \n", total);
		Organize(droplist, total);


		FILE* pFile = nullptr;
		fopen_s(&pFile, "droplist.txt", "w+");

		if (pFile)
		{
			for (int i = 0; i < total; i++)
			{
				auto& item = droplist[i];
				std::stringstream str;

				str << "<hr/>";
				str << "<p><strong><span style=\"font-size:16px;\">" << item.Name << "</span></strong></p>";
				str << "<strong>HP:</strong> " << item.MaxHp;

				str << "<p><strong>Drops:</strong><br/>";

				for (int slot = 0; slot < 30; slot++)
				{
					st_Item* slotItem = &item.item[slot];
					if (slotItem->Index == 0)
						continue;

					str << ItemList[slotItem->Index].Name << "</br>";
				}

				str << "</p>";

				fprintf(pFile, str.str().c_str());
			}

			fclose(pFile);
		}

		//fwrite((void*)droplist, sizeof stDropList * total, 1, pFile);
		//fclose(pFilGetVillagee);
	}

	free(droplist);
}

void CreateGUI()
{
	WNDCLASSEX wcex;
	const char* szWindowClass = "Destiny - GameServer";
	const char* szWindowTitle = "Destiny - GameServer";

	HINSTANCE hInstance = GetModuleHandle(NULL);
	bool Sucessful = true;

	wcex.cbSize = sizeof WNDCLASSEX;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(105));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(105));

	if (!(GUI.Class = RegisterClassEx(&wcex)))
	{
		MessageBoxA(0, "RegisterClassEx error", "Server::RegisterClassEx", 4096);
		Sucessful = false;
	}

	else if (!(GUI.hGUI = CreateWindow(szWindowClass, szWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 50, 50, NULL, NULL, hInstance, NULL)))
	{
		int err = GetLastError();
		MessageBoxA(0, "CreateWindow error", "Server::CreateWindow", 4096);

		Sucessful = false;
	}
	
	else if (!(GUI.Font = CreateFont(8, NULL, NULL, NULL, FW_DONTCARE, false, false, false, DEFAULT_CHARSET, NULL, NULL, DEFAULT_QUALITY, FF_DONTCARE, "MS Sans Serif")))
	{
		MessageBoxA(0, "CreateFont error", "Server::CreateFont", 4096);
		Sucessful = false;
	}

	if (!Sucessful)
		PostQuitMessage(1);

	SendMessageA(GUI.hButton[0], WM_SETFONT, (WPARAM)GUI.Font, NULL);
	SendMessageA(GUI.hLabel[0], WM_SETFONT, (WPARAM)GUI.Font, NULL);
	SendMessageA(GUI.hLabel[1], WM_SETFONT, (WPARAM)GUI.Font, NULL);
	SendMessageA(GUI.hLabel[2], WM_SETFONT, (WPARAM)GUI.Font, NULL);
	
	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);
	int guiWidth = 600;
	int guiHeight = 420;
	SetWindowPos(GUI.hGUI, NULL, 0, 0, guiWidth, guiHeight, SWP_NOMOVE | SWP_NOZORDER);
	ShowWindow(GUI.hGUI, SW_SHOW);
	UpdateWindow(GUI.hGUI);

	SetConsoleTitle(szWindowTitle);
	
	bool success = true;

	if (!LoadConfig())
		success = false;
	else if (!LoadDataServer())
		success = false;
/*	else if (!ReadMySQL())
		success = false;
	else if (!ConnectSQL())
	{
		MessageBox(NULL, "Falha na conexão com MYSQL", "error : mysql_connect", MB_OK);
		success = false;
	}*/
	else if (!CreateServer())
	{
		MessageBox(NULL, "Falha na criação do servidor", "error : CreateServer", MB_OK);
		success = false;
	}
	else if (!ConnectDB())
	{
		MessageBox(NULL, "Falha na conexão com DBSRV", "error : connectDB", MB_OK);
		success = false;
	}

	if (success)
	{
		memset(g_pGuildAlly, 0, sizeof g_pGuildAlly);
		memset(&sServer.TowerWar, 0, sizeof stTowerWar);

		BASE_InitializeHitRate();
		ReadAttributeMap();
		ReadHeightMap();
		ApplyAttribute((char*)g_pHeightGrid, 4096);
		ReadLanguageFile();
		ReadItemList();
		ReadSkillData();
		ReadTeleport();
		ReadGameConfig();
		ReadGameConfigv2();
		ReadNPCBase();
		ReadGuild();
		ReadInitItem();
		LoadGuild();
		ReadNPCQuest();
		ReadMessages();
		ReadNPCDonate();
		ReadNoCP();
		ReadBlockedItem();
		ReadPacItens();
		LoadNPCEvento();
		LoadChristmasMission();
		ReadMerchantStore();
		ReadArenaStore();
		ReadDailyReward();
		ReadLevelItem();
		ReadBoss();
		ReadScheduled();
		LoadQuiz();
		ReadMissions();
		ReadRvRStore();
		ReadArenaConfig();
		ReadOverStore();
		ReadStoreNew();

		for(size_t LOCAL_5 = 0; LOCAL_5 < sServer.InitCount;LOCAL_5 ++)
		{
			st_Item LOCAL_7;
			memset(&LOCAL_7, 0, sizeof st_Item);

			LOCAL_7.Index = g_pInitItem[LOCAL_5].Index;

			INT32 LOCAL_8 = CreateItem(g_pInitItem[LOCAL_5].PosX, g_pInitItem[LOCAL_5].PosY, &LOCAL_7, g_pInitItem[LOCAL_5].Rotate, 1);
			if(LOCAL_8 >= 4096 || LOCAL_8 <= 0)
				continue;

			INT32 LOCAL_9 = GetItemAbility(&LOCAL_7, EF_KEYID);
			if(LOCAL_9 != 0 && LOCAL_9 < 15)
			{
				int LOCAL_10;
				UpdateItem(LOCAL_8, 3, &LOCAL_10);
			}
			/*
			if(LOCAL_7.Index == 746)
			{
				pInitItem[LOCAL_8].IsOpen = 1;
				pInitItem[LOCAL_8].Status = 1;
			}*/
		}

		// Cria as torres doidonas
		for(INT32 i = 0; i < 5; i++)
		{
			sGuildZone *zone = &g_pCityZone[i];

			st_Item item; 
			memset(&item, 0, sizeof st_Item);

			item.Index = 3145 + (zone->win_count);
						
			if(zone->owner_index != 0)
			{
				item.EF1 = 56;
				item.EFV1 = zone->owner_index / 257;

				item.EF2 = 57;
				item.EFV2 = zone->owner_index;

				item.EF3 = 59;
				item.EFV3 = Rand() % 255;
			}

			CreateItem(zone->tower_x, zone->tower_y, &item, 3, 0);
		}

		SetTimer(GUI.hGUI, TIMER_SEND, 100, NULL);
		SetTimer(GUI.hGUI, TIMER_SEC, 500, NULL);
		SetTimer(GUI.hGUI, TIMER_MIN, 12000, NULL);
		SetTimer(GUI.hGUI, TIMER_HOUR, (60000 * 60), NULL);
			
		//SetArenaDoor(3);
		SetCastleDoor(1);
		SetColoseumDoor(1);
		SetColoseumDoor2(3);
		
		sServer.ForceWeather = -1;
		sServer.WeekMode = 4;
		sServer.ForceWeekDay = -1;

		// Lê os arquivos de itens
		mGener.ReadNPCGener();
		TOD_DroplistGenerator::GetInstance().Generate();

		sServer.Staff = 1;

		pMsgSignal2 LOCAL_810;
		LOCAL_810.Header.PacketId = 0x80F;
		LOCAL_810.Header.Size = sizeof pMsgSignal2;
		LOCAL_810.Header.ClientId = 0;

		LOCAL_810.Value = 1;
		LOCAL_810.Value2 = 1;

		AddMessageDB((BYTE*)&LOCAL_810, sizeof pMsgSignal2);

		sServer.ServerTime = GetTickCount();

		GuildZoneReport(); 
		for (int i = 1; i < MAX_PLAYER; i++)
			Users[i].clientId = i;

		for (int i = 0; i < MAX_SPAWN_MOB; ++i)
			Mob[i].clientId = i;

		printf("Servidor iniciado com sucesso");
	}
	else
	{
		printf("Servidor não foi possível de ser ligado\n\n");
		if (sServer.Socket != INVALID_SOCKET)
			closesocket(sServer.Socket);

		WSACleanup();
	}


}