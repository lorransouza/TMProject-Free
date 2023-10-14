#pragma once

#include <fstream>
#include <iterator>
#include <algorithm>

#include <functional>
#include <map>
#include <vector>
#include <tuple>
#include <array>
#include <excpt.h>

#include "NewApp.h"
#include "CPSock.h"
#include "ObjectManager.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "TimerManager.h"
#include "TMScene.h"
#include "RenderDevice.h"
#include "EventTranslator.h"
#include "dsutil.h"
#include "SControl.h"
#include "TMPaths.h"
#include "ResourceControl.h"
#include "Basedef.h"
#include "HudController.h"

class HudElement
{


public:
	struct stObject_Human
	{
		int GroupType;
		

	} Object_Human;


	struct stDropListEvent
	{
		std::vector<stDropList> Drop;

		int DropPage;

		st_DropListMobSelected DropSelected;

	}DropListEvent;

	struct stPersonagem
	{
		
		int ForceMob;
		int ExpBonus;
		int DropBonus;
		int PerfuDamage;
		int AbsDamage;

	}Personagem;

	struct stObject_World
	{

		int* inventarioGrid[4];
		int* LabelExpBonus;
		int* LabelDropBonus;


	}Object_World;

	struct stTorre
	{

		MSG_TowerWar Packet;

	}GuerraTorres;
	
};

class HudController : public HudElement
{

public:
	static void Init();

public:
	static void DiaryMission(int handle);
};

extern HudController _HudControl;
