#include "cServer.h"
#include "CNPCGener.h"
#include "Basedef.h"

CNPCGener mGener;

CNPCGener::CNPCGener()
{
	numList = -1;
}

CNPCGener::~CNPCGener()
{
	numList = -1;
}

bool LoadMob(char *mobName, int mobIndex, bool mob)
{
	char szTMP[1024];
	sprintf_s(szTMP, "npc\\%s", mobName);
	FILE *hFile = NULL;
	fopen_s(&hFile, szTMP, "rb");

	if(hFile != NULL) 
	{
		if(mob)
			fread(&mGener.pList[mobIndex].Leader, 1, sizeof st_Mob, hFile);
		else
			fread(&mGener.pList[mobIndex].Follower, 1, sizeof st_Mob, hFile);

		fclose(hFile);
		return true;
	} 

	return false;
}

bool CNPCGener::ReadNPCGener()
{ 
	FILE *pFile = nullptr;
	fopen_s(&pFile, "NPCGener.txt", "r");

	if(!pFile)
	{
		MessageBoxA(NULL, "Não foi possível encontrar \"NPCGener.txt\"", "Error ReadNPCGener", 4096);

		return false;
	}

	FILE *newFile = nullptr;
	fopen_s(&newFile, "NPCGener_new.txt", "w+");

	if(!newFile)
		MessageBoxA(NULL, "Não será possível gerar \"NPCGener_new.txt\"", "Fail ReadNPCGener", 4096);

	char tmp[1024];

	int index = -1;
    while(fgets(tmp, sizeof(tmp), pFile))
	{
			char cmd[128];
			char val[128];

		if(*tmp == '#')
		{
			index++;

            if(index == MAX_NPCGENERATOR)
            {
                printf("Falha ao obter um índice para o npc '%s'.", val);
                break;
            }

            if(newFile != NULL)
                fprintf(newFile, "#\t[%04d]\n", index);

			for(int i = 0; i < 5; i ++)
				pList[index].SegmentAction[i][0] = 0;

            continue;
        }
	
		if(*tmp == '#' || *tmp == '\n' || *tmp == '/')
		{
			fprintf(newFile, tmp);
			continue;
		}

		int ret = sscanf_s(tmp, " %[^:]: %[^\t\n]", cmd, 127, val, 127);
        if(ret != 2)
            continue;

		if(index != -1 )
		{
			stGener *genMob = &pList[index];

			int wait = 0;
			if(!strcmp(cmd, "MinuteGenerate"))
				genMob->MinuteGenerate = atoi(val);
			else if(!strcmp(cmd, "MaxNumMob"))
				genMob->MaxNumMob = atoi(val);
			else if(!strcmp(cmd, "Formation"))
				genMob->Formation = atoi(val);
			else if(!strcmp(cmd, "MinGroup"))
				genMob->MinGroup = atoi(val);
			else if(!strcmp(cmd, "MaxGroup"))
				genMob->MaxGroup = atoi(val);
			else if(!strcmp(cmd, "RouteType"))
				genMob->RouteType = atoi(val);
			else if(!strcmp(cmd, "StartRange"))
				genMob->SegmentRange[0] = atoi(val);
			else if(!strcmp(cmd, "FightAction1"))
				strncpy_s(genMob->FightAction[0], val, 80);
			else if(!strcmp(cmd, "FightAction2"))
				strncpy_s(genMob->FightAction[1], val, 80);
			else if(!strcmp(cmd, "FightAction3"))
				strncpy_s(genMob->FightAction[2], val, 80);
			else if(!strcmp(cmd, "FightAction4"))
				strncpy_s(genMob->FightAction[3], val, 80);
			else if(!strcmp(cmd, "StartAction"))
				strncpy_s(genMob->SegmentAction[0], val, 80);
			else if(!strcmp(cmd, "Segment1Action"))
				strncpy_s(genMob->SegmentAction[1], val, 80);
			else if(!strcmp(cmd, "Segment2Action"))
				strncpy_s(genMob->SegmentAction[2], val, 80);
			else if(!strcmp(cmd, "Segment3Action"))
				strncpy_s(genMob->SegmentAction[3], val, 80);
			else if(!strcmp(cmd, "DestAction"))
				strncpy_s(genMob->SegmentAction[4], val, 80);
			else if(!strcmp(cmd, "StartWait"))
				genMob->SegmentWait[0] = atoi(val);
			else if(!strcmp(cmd, "DestWait"))
				genMob->SegmentWait[4] = atoi(val);
			else if(!strcmp(cmd, "Leader"))
			{
				if(!LoadMob(val, index, true))
				{
				}
			}
			else if(!strcmp(cmd, "Follower"))
			{
				if(!LoadMob(val, index, false))
				{
				}
			}
			else if(!strcmp(cmd, "Segment1X"))
				genMob->Segment_X[1] = atoi(val);
			else if(!strcmp(cmd, "Segment2X"))
				genMob->Segment_X[2] = atoi(val);
			else if(!strcmp(cmd, "Segment3X"))
				genMob->Segment_X[3] = atoi(val);
			else if(!strcmp(cmd, "Segment1Y"))
				genMob->Segment_Y[1] = atoi(val);
			else if(!strcmp(cmd, "Segment2Y"))
				genMob->Segment_Y[2] = atoi(val);
			else if(!strcmp(cmd, "Segment3Y"))
				genMob->Segment_Y[3] = atoi(val);
			else if(!strcmp(cmd, "Segment1Wait"))
				genMob->SegmentWait[1] = atoi(val);
			else if(!strcmp(cmd, "Segment2Wait"))
				genMob->SegmentWait[2] = atoi(val);
			else if(!strcmp(cmd, "Segment3Wait"))
				genMob->SegmentWait[3] = atoi(val);
			else if(!strcmp(cmd, "Segment1Range"))
				genMob->SegmentRange[1] = atoi(val);
			else if(!strcmp(cmd, "Segment2Range"))
				genMob->SegmentRange[2] = atoi(val);
			else if(!strcmp(cmd, "Segment3Range"))
				genMob->SegmentRange[3] = atoi(val);
			else if(!strcmp(cmd, "RouteType"))
				genMob->RouteType = atoi(val);
			else if(!strcmp(cmd, "StartX"))
				genMob->Segment_X[0] = atoi(val);
			else if(!strcmp(cmd, "StartY"))
				genMob->Segment_Y[0] = atoi(val);
			else if(!strcmp(cmd, "DestX"))
				genMob->Segment_X[4] = atoi(val);
			else if(!strcmp(cmd, "DestY"))
				genMob->Segment_Y[4] = atoi(val);
			else if(!strcmp(cmd, "DestRange"))
				genMob->SegmentRange[4] = atoi(val);
			else if(!strcmp(cmd, "DieAction1"))
				strncpy_s(genMob->DieAction[0], val, 80);
			else if(!strcmp(cmd, "DieAction2"))
				strncpy_s(genMob->DieAction[1], val, 80);
			else if(!strcmp(cmd, "DieAction3"))
				strncpy_s(genMob->DieAction[2], val, 80);
			else if(!strcmp(cmd, "DieAction4"))
				strncpy_s(genMob->DieAction[3], val, 80);
			else
				continue;

			if(newFile)
				fputs(tmp, newFile);
		}

	}

	fclose(pFile);

	if(newFile)
		fclose(newFile);

	numList = index;

	return true;
}