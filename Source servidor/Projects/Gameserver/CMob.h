#ifndef __CMOB_H__
#define __CMOB_H__

#include <Windows.h>

#include "stBase.h"
#include "cServer.h"

#include <array>
#include <chrono>

using time_point = std::chrono::time_point<std::chrono::steady_clock>;

#define MOB_EMPTY       0 
#define MOB_USERDOCK    1  .
#define MOB_USER        2
#define MOB_PEACE  4
#define MOB_COMBAT      5
#define MOB_RETURN		6
#define MOB_FLEE		7
#define MOB_ROAM		8

#define MAX_ENEMY 13
#define MAX_SPAWN_MOB 30000

enum class TOD_Bag
{
	FirstBag,
	SecondBag
};

struct TransBonus
{
	INT32 MinAttack; // Unknown_00; // Min Attack
	INT32 MaxAttack; // Unknown_04; // Max Attack
	INT32 MinDefense; // Unknown_08; // Min Defense
	INT32 MaxDefense; // Unknown_12; // Max Defense
	INT32 MinHp; // Unknown_16; // Min Hp
	INT32 MaxHp; // Unknown_20; // Max Hp
	INT32 SpeedMove;
	INT32 Unknown_28;
	INT32 SpeedAttack; // 32
	INT32 Unknow_36; // 36
	INT32 Unknown_40;
	INT32 Unknown_44;
	INT32 Unknown_48;
	INT32 Unknown_52;
	INT32 Unknown_56;
	INT32 Unknown_60;

	INT32 Sanc; // 64
}; // 68

class CMob
{
public:
	stCharInfo Mobs;

	UINT32 Mode;			// 788 a 791

	UINT32 Leader; // 792 a 795
	UINT32 Formation; // 796 a 799
	UINT32 RouteType; // 800 a 803

	struct
	{
		UINT32 X; // 804 a 807
		UINT32 Y; // 808 a 811
		UINT32 Time; // 812 a 815
		UINT32 Speed; // 816 a 819
	} Last; //

	struct
	{
		UINT32 X; // 820 a 823
		UINT32 Y; // 824 a 827
	} Target;

	struct
	{
		UINT32 X; // 828 a 831
		UINT32 Y; // 832 a 835
		UINT32 Action; // 836 a 839
	} Next;

	UINT8 Route[24]; // 840 a 863

	INT32 WaitSec; // 864 a 867

	struct
	{
		UINT32 X; // 868 a 871
		UINT32 Y; // 872 a 875
	} Position;

	struct
	{
		UINT32 Seg; // 876 a 879
		UINT32 Unknown_0; // 880 a 883
		UINT32 X; // 884 a 887
		UINT32 Y; // 888 a 891
		UINT32 ListX[5]; // 892 a 911
		UINT32 ListY[5]; // 912 a 931
		UINT32 Wait[5];  // 932 a 951
		UINT32 Direction; // 952 a 955
		INT32 Progress; // 956 a 959
	} Segment;

	INT32 GenerateID{ -1 }; // 960 a 963
	UINT16 CurrentTarget; // 964 a 965
	UINT16 EnemyList[MAX_ENEMY]; // 966 a 973
	INT16 PartyList[12]; // 974 a 997

	UINT16 Unknown_1; // 998 a 999

	INT32 LifeSteal;
	INT32 Vampirism;
	INT32 PotionBonus;

	UINT32 WeaponDamage; // 1000 a 1003
	UINT32 Summoner; // 1004 a 1007
	UINT32 PotionCount; // 1008 a 1011
	UINT32 Parry; // 1012 a 1015
	UINT32 GuildDisable; // 1016 a 1019
	UINT32 DropBonus; // 1020 a 1023
	UINT32 ExpBonus; // 1024 a 1027
	UINT32 ForceDamage; // 1028 a 1031
	UINT32 ReflectDamage; // 1032 a 1035
	
	UINT32 Unknown_2; // 1036 a 1039
	UINT32 IndividualExpBonus;

	INT32 HitRate; 

	UINT32 SpawnType; 
	UINT32 Range;

	UINT32 Motion; 
	UINT32 Jewel; // jóias cash

	INT32 QuestId{ -1 };

	INT32 IgnoreResistance;
	INT32 ResistanceChance;
	INT32 SlowChance;
	INT32 RegenHP;
	INT32 RegenMP;
	INT32 MagicIncrement;
	INT32 clientId;
	INT32 AttackSpeed;

	INT32 Lifes; 

	char Tab[26];
	INT16 SummonerParty[12];
	struct
	{
		std::array<time_point, 10> SkillTick;
	} MeridaInfo;

	// Functions
	INT32 GetCurrentMP();
	INT32 GetCurrentHP();
	
	void GetCurrentScore(int clientId);
	void GetStatusBaseScore();
	void GetScorePoint();
	void GetSkillPoint();
	void GetMasterPoint();

	void GetRandomPos();
	void AddEnemyList(int enemyId);
	void RemoveEnemyList(int enemyId);
	void GetNextPos(int battle);
	void SelectTargetFromEnemyList();
	void GetTargetPos(int arg1);
	void GetTargetPosDistance(int arg1);
	
	INT32 CheckGetLevel();
	INT32 CheckQuarter(long long expEarned);
	INT32 StandingByProcessor();
	INT32 GetEnemyFromView();
	INT32 SetSegment();
	INT32 BattleProcessor();

	bool isNormalPet() const;
	bool isPetAlive() const;

	bool isBagActive(TOD_Bag bag) const;

	bool IsInsideValley() const;
	size_t BossInfoId{ MAXUINT32 };
};

void GenerateMob(int arg1, int arg2, int arg3);
void GetCurScore(CMob *mob, st_Affect *affect);

extern CMob Mob[MAX_SPAWN_MOB];
extern TransBonus pTransBonus[5];
extern st_Mob NPCBase[50];
#endif