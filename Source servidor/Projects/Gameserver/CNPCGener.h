#ifndef __CNPCGENER_H__
#define __CNPCGENER_H__

#include <Windows.h>
#include <array>

#define	MAX_NPCGENERATOR 8192

struct stGener
{
	INT32 Mode; // 0 - 3
	INT32 MinuteGenerate; // 4 - 7

	INT32 MaxNumMob; // 8 - 11

	INT32 MobCount; // 12 - 15
	
	INT32 MinGroup; // 16 - 19
	INT32 MaxGroup; // 20 - 23
	
	INT32 Segment_X[5]; //24 - 43
	INT32 Segment_Y[5]; // 44 - 63

	INT32 SegmentRange[5]; // 64 - 83

	INT32 SegmentWait[5]; // 84 - 103
	
	char SegmentAction[5][80]; // 104 - 503
	char FightAction[4][80]; // 504 - 823
	char FleeAction[4][80]; // 824 - 1143
	char DieAction[4][80]; // 1144 - 1463
	
	INT32 Formation; // 1464 - 1467
	INT32 RouteType; // 1468 - 1471 

	UINT32 Unknow; // 1472 - 1475 -> Ele pega o valor do MinuteGenerate (?)

	st_Mob Leader; // 1476 - 2231	
	st_Mob Follower; // 2232 - 2987
};

class CNPCGener
{
public:
	CNPCGener();
	~CNPCGener();

	std::array<stGener, MAX_NPCGENERATOR> pList;

	INT32 numList;

	bool ReadNPCGener();
	INT32 GetEmptyNPCMob();
};

extern CNPCGener mGener;

#endif