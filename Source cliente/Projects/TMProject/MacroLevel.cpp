#include "pch.h"
#include "MacroLevel.h"
#include "HudController.h"
#include "TimerManager.h"
#include "TMFieldScene.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "dsutil.h"
#include "DirShow.h"
#include "SControlContainer.h"
#include "SGrid.h"
#include "Mission.h"
#include "MrItemMix.h"
#include "TMGround.h"
#include "TMHuman.h"
#include "TMObjectContainer.h"
#include "TMCamera.h"
#include "TMSun.h"
#include "TMSky.h"
#include "TMSnow.h"
#include "TMRain.h"
#include "TMSkinMesh.h"
#include "TMEffectMesh.h"
#include "TMEffectBillBoard2.h"
#include "TMEffectBillBoard.h"
#include "TMUtil.h"
#include "TMHouse.h"
#include "TMEffectBillBoard4.h"
#include "TMSkillMagicArrow.h"
#include "TMEffectStart.h"
#include "TMSkillJudgement.h"
#include "TMSkillTownPortal.h"
#include "TMEffectLevelUp.h"
#include "TMEffectSkinMesh.h"
#include "SGrid.h"
#include "ItemEffect.h"
#include "TMUtil.h"
#include "TMEffectSWSwing.h"
#include "TMEffectSpark.h"
#include "TMSkillHolyTouch.h"
#include "TMEffectParticle.h"
#include "TMSkillMeteorStorm.h"
#include "TMSkillThunderBolt.h"
#include "TMSkillSlowSlash.h"
#include "TMSkillMagicShield.h"
#include "TMSkillFreezeBlade.h"
#include "TMArrow.h"
#include "TMShade.h"
#include "TMSkillPoison.h"
#include "TMFont3.h"
#include "TMSkillHeavenDust.h"
#include "TMSkillFlash.h"
#include "TMItem.h"
#include "TMCannon.h"
#include "TMEffectCharge.h"
#include "TMSkillExplosion2.h"
#include "TMEffectDust.h"
#include "TMGate.h"
#include <WinInet.h>

Macro::Water_N::Water_N()
{
}

Macro::Water_N::~Water_N()
{
}

void Macro::Water_N::UseItem(const int posX, const int posY, const int level)
{
	int X = posX / 128;
	int Y = posY / 128;

	if (X == 8 && Y == 27 || (posX / 4) == 491 && (posY / 4) == 443)
	{
		// Send UseItem
		struct stUseItem
		{
			MSG_STANDARD Header;
			int SourType;
			int SourPos;
			int DestType;
			int DestPos;
			unsigned short GridX;
			unsigned short GridY;
			unsigned short ItemID;
		};

		bool found = false;
		bool repeat = false;

		stUseItem Packet{};

		Packet.Header.ID = g_pObjectManager->m_dwCharID;
		Packet.Header.Type = 0x373;
		Packet.Header.Size = sizeof(stUseItem);
		Packet.SourType = 1;

		for (int itemId = level; itemId <= lastScroll; itemId++)
		{
			for (int i = 0; i < 4; i++)
			{
				auto Inv = (SGridControl*)_HudControl.Object_World.inventarioGrid[i];

				if (Inv)
				{
					for (int row = 0; row < 3; row++)
					{
						for (int column = 0; column < 5; column++)
						{
							auto ItemST = Inv->GetItem(column, row);

							if (!ItemST || !ItemST->m_pItem)
								continue;

							if (ItemST->m_pItem->sIndex < firstScroll || ItemST->m_pItem->sIndex > lastScroll)
								continue;

							if (ItemST->m_pItem->sIndex != itemId && !repeat)
								continue;

							Packet.SourPos = (column + row * Inv->m_nColumnGridCount) + (i * 15);
							Packet.ItemID = ItemST->m_pItem->sIndex;
							SendOneMessage((char*)&Packet, sizeof(stUseItem));
							//Inv->ClearItem(column, row);
							return;
						}
					}
				}
			}

			if (itemId == lastScroll)
			{
				if (!repeat)
				{
					repeat = true;
					itemId = firstScroll;
				}
				else // Olhou todo o inventário e não encontrou nenhum pergaminho mais.
					break;
			}
		}
	}
}

void Macro::Water_N::DoMove(const int posX, const int posY)
{
	int waterX = 1966 + rand() % 1;
	int waterY = 1775 - rand() % 1;

	struct	stMove
	{
		MSG_STANDARD Header;

		short PosX, PosY;

		int Effect; // 0 = walking, 1 = teleporting
		int Speed;

		char Route[24];

		short TargetX, TargetY;
	};

	stMove Packet{};

	Packet.Header.Type = 0x36C;
	Packet.Header.ID = g_pObjectManager->m_dwCharID;
	Packet.Header.Size = sizeof(stMove);
	Packet.Header.Tick = g_pTimerManager->GetServerTime();


	Packet.PosX = g_pObjectManager->m_stMobData.HomeTownX;
	Packet.PosY = g_pObjectManager->m_stMobData.HomeTownY;

	Packet.TargetX = waterX;
	Packet.TargetY = waterY;

	Packet.Speed = 255;

	SendOneMessage((char*)&Packet, sizeof(stMove));

}

Macro::Water_M::Water_M()
{
}

Macro::Water_M::~Water_M()
{
}

void Macro::Water_M::UseItem(const int posX, const int posY, const int level)
{
	int X = posX / 128;
	int Y = posY / 128;

	if (X == 9 && Y == 28 || (posX / 4) == 491 && (posY / 4) == 443)
	{
		// Send UseItem
		struct stUseItem
		{
			MSG_STANDARD Header;
			int SourType;
			int SourPos;
			int DestType;
			int DestPos;
			unsigned short GridX;
			unsigned short GridY;
			unsigned short ItemID;
		};

		bool found = false;
		bool repeat = false;

		stUseItem Packet{};

		Packet.Header.ID = g_pObjectManager->m_dwCharID;
		Packet.Header.Type = 883;
		Packet.Header.Size = sizeof(stUseItem);
		Packet.SourType = 1;

		for (int itemId = level; itemId <= lastScroll; itemId++)
		{
			for (int i = 0; i < 4; i++)
			{
				auto Inv = (SGridControl*)_HudControl.Object_World.inventarioGrid[i];

				if (Inv)
				{
					for (int row = 0; row < 3; row++)
					{
						for (int column = 0; column < 5; column++)
						{
							auto ItemST = Inv->GetItem(column, row);

							if (!ItemST || !ItemST->m_pItem)
								continue;

							if (ItemST->m_pItem->sIndex < firstScroll || ItemST->m_pItem->sIndex > lastScroll)
								continue;

							if (ItemST->m_pItem->sIndex != itemId && !repeat)
								continue;

							Packet.SourPos = (column + row * Inv->m_nColumnGridCount) + (i * 15);
							Packet.ItemID = ItemST->m_pItem->sIndex;
							SendOneMessage((char*)&Packet, sizeof(stUseItem));

							//Inv->ClearItem(column, row);
							return;
						}
					}
				}
			}

			if (itemId == lastScroll)
			{
				if (!repeat)
				{
					repeat = true;
					itemId = firstScroll;
				}
				else // Olhou todo o inventário e não encontrou nenhum pergaminho mais.
					break;
			}
		}
	}
}

void Macro::Water_M::DoMove(const int posX, const int posY)
{
	int waterX = 1966 + rand() % 1;
	int waterY = 1775 - rand() % 1;

	struct	stMove
	{
		MSG_STANDARD Header;

		short PosX, PosY;

		int Effect; // 0 = walking, 1 = teleporting
		int Speed;

		char Route[24];

		short TargetX, TargetY;
	};

	stMove Packet{};

	Packet.Header.Type = 0x36C;
	Packet.Header.ID = g_pObjectManager->m_dwCharID;
	Packet.Header.Size = sizeof(stMove);
	Packet.Header.Tick = g_pTimerManager->GetServerTime();


	Packet.PosX = (int)g_pObjectManager->m_stMobData.HomeTownX;
	Packet.PosY = (int)g_pObjectManager->m_stMobData.HomeTownY;

	Packet.TargetX = waterX;
	Packet.TargetY = waterY;

	Packet.Speed = 255;
	SendOneMessage((char*)&Packet, sizeof(stMove));

}

Macro::Water_A::Water_A()
{
}

Macro::Water_A::~Water_A()
{
}

void Macro::Water_A::UseItem(const int posX, const int posY, const int level)
{
	int X = posX / 128;
	int Y = posY / 128;

	if (X == 10 && Y == 27 || (posX / 4) == 491 && (posY / 4) == 443)
	{
		// Send UseItem
		struct stUseItem
		{
			MSG_STANDARD Header;
			int SourType;
			int SourPos;
			int DestType;
			int DestPos;
			unsigned short GridX;
			unsigned short GridY;
			unsigned short ItemID;
		};

		bool found = false;
		bool repeat = false;

		stUseItem Packet{};

		Packet.Header.ID = g_pObjectManager->m_dwCharID;
		Packet.Header.Type = 0x373;
		Packet.Header.Size = sizeof(stUseItem);
		Packet.SourType = 1;

		for (int itemId = level; itemId <= lastScroll; itemId++)
		{
			for (int i = 0; i < 4; i++)
			{
				auto Inv = (SGridControl*)_HudControl.Object_World.inventarioGrid[i];

				if (Inv)
				{
					for (int row = 0; row < 3; row++)
					{
						for (int column = 0; column < 5; column++)
						{
							auto ItemST = Inv->GetItem(column, row);

							if (!ItemST || !ItemST->m_pItem)
								continue;

							if (ItemST->m_pItem->sIndex < firstScroll || ItemST->m_pItem->sIndex > lastScroll)
								continue;

							if (ItemST->m_pItem->sIndex != itemId && !repeat)
								continue;

							Packet.SourPos = (column + row * Inv->m_nColumnGridCount) + (i * 15);
							Packet.ItemID = ItemST->m_pItem->sIndex;
							SendOneMessage((char*)&Packet, sizeof(stUseItem));

							//Inv->ClearItem(column, row);
							return;
						}
					}
				}
			}

			if (itemId == lastScroll)
			{
				if (!repeat)
				{
					repeat = true;
					itemId = firstScroll;
				}
				else // Olhou todo o inventário e não encontrou nenhum pergaminho mais.
					break;
			}
		}
	}
}

void Macro::Water_A::DoMove(const int posX, const int posY)
{
	int waterX = 1966 + rand() % 1;
	int waterY = 1775 - rand() % 1;

	struct	stMove
	{
		MSG_STANDARD Header;

		short PosX, PosY;

		int Effect; // 0 = walking, 1 = teleporting
		int Speed;

		char Route[24];

		short TargetX, TargetY;
	};

	stMove Packet{};

	Packet.Header.Type = 0x36C;
	Packet.Header.ID = g_pObjectManager->m_dwCharID;
	Packet.Header.Size = sizeof(stMove);
	Packet.Header.Tick = g_pTimerManager->GetServerTime();

	Packet.PosX = g_pObjectManager->m_stMobData.HomeTownX;
	Packet.PosY = g_pObjectManager->m_stMobData.HomeTownY;

	Packet.TargetX = waterX;
	Packet.TargetY = waterY;

	Packet.Speed = 255;
	SendOneMessage((char*)&Packet, sizeof(stMove));

}
