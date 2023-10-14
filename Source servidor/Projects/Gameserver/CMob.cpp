#include "cServer.h"
#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "UOD_BossEvent.h"
#include "UOD_EventManager.h"
#include <algorithm>

CMob Mob[MAX_SPAWN_MOB];

int BaseSIDCHM[4][6] = {
	{ 8, 4,  7, 6, 80, 45 }, // Transknight
	{ 5, 8,  5, 5, 60, 65 }, // Foema
	{ 6, 6,  9, 5, 70, 55 }, // BeastMaster
	{ 8, 9, 13, 6, 75, 60 }  // Hunter
};

TransBonus pTransBonus[5] =
{
	{	110, 130,	 95, 105,	95, 105,	1, 0,	20, 0,	100, 100,	100, 100,	100, 100,	15},
	{	80, 100,	100, 110,	110, 140,	0, 0,	0, 0,	100, 100,	100, 100,	100, 100,	60},
	{	100, 120,	105, 115,	100, 120,	1, 0,	20, 0,	100, 100,	100, 100,	100, 100,	115},
	{	90, 110,	110, 125,	105, 110,	0, 0,	20, 0,	100, 100,	100, 100,	100, 100,	155 },
	{	105, 120,	110, 120,	105, 115,	3, 0,	20, 0,	100, 100,	100, 100,	100, 100,	155 }
};

INT32 g_pClanTable[9][9] = {
	1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1,
	1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
	1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1,
	1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
	1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1,
	0, 1, 0, 1
};

INT32 CMob::GetCurrentHP()
{
	st_Mob *usr = &Mobs.Player;

	static const int HPIncrementPerLevel[4] = {
		3, // Transknight
		1, // Foema
		1, // BeastMaster
		2  // Hunter
	};

	int hp_inc = GetMobAbility(&Mobs, EF_HP);

	int hp_perc = GetMobAbility(&Mobs, EF_HPADD);
	hp_perc += GetMobAbility(&Mobs, EF_HPADD2);

	int mult = HPIncrementPerLevel[usr->ClassInfo];
	switch (Mobs.Player.Equip[0].EFV2)
	{
	case ARCH:
		mult++;
		break;
	case CELESTIAL:
	case SUBCELESTIAL:
		mult += 2;
		break;
	}

	hp_inc += (usr->Status.CON * 2);
	hp_inc += ((hp_inc * hp_perc) / 100);

	return hp_inc;
}

INT32 CMob::GetCurrentMP()
{
	st_Mob *usr = &Mobs.Player;

	static const int MPIncrementPerLevel[4] = {
		1, // Transknight
		1, // Foema
		2, // BeastMaster
		1  // Hunter
	};

	int mp_inc = GetMobAbility(&Mobs, EF_MP) + 50,
		mp_perc = GetMobAbility(&Mobs, EF_MPADD);

	mp_inc += BaseSIDCHM[usr->ClassInfo][5];

	int mult = MPIncrementPerLevel[usr->ClassInfo];
	switch (Mobs.Player.Equip[0].EFV2)
	{
	case ARCH:
		mult++;
		break;
	case CELESTIAL:
	case SUBCELESTIAL:
		mult += 2;
		break;
	}

	mp_inc += (mult * usr->Status.Level);
	mp_inc += (usr->Status.INT * 2);
	mp_inc += ((mp_inc * mp_perc) / 100);

	if (mp_inc > 64000)
		mp_inc = 64000;
	else if (mp_inc <= 0)
		mp_inc = 1;

	return mp_inc;
}

void CMob::GetStatusBaseScore()
{
	if (clientId <= 0 || clientId >= MAX_PLAYER || Mobs.Player.ClassInfo < 0 || Mobs.Player.ClassInfo >= 4)
		return;

	static const int BaseADDHPMP[4][2] = {
		{3, 1},
		{1, 3},
		{1, 2},
		{2, 1}
	};

	INT8 mult = 1;
	switch (this->Mobs.Player.Equip[0].EFV2)
	{
	case MORTAL:
		mult = 1;
		break;
	case ARCH:
		mult = 2;
		break;
	case CELESTIAL:
	case SUBCELESTIAL:
		mult = 3;
		break;
	}

	INT32 level = Mobs.Player.bStatus.Level;
	if (Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
		level += 400;

	INT32 attack = 5;
	attack += (mult * level);
	INT32 defense = 4;
	defense += (mult * level);

	Mobs.Player.bStatus.Attack = attack;
	Mobs.Player.bStatus.Defense = defense;

	int hp = BaseADDHPMP[Mobs.Player.ClassInfo][0] * level + BaseSIDCHM[Mobs.Player.ClassInfo][4];
	int mp = BaseADDHPMP[Mobs.Player.ClassInfo][1] * level + BaseSIDCHM[Mobs.Player.ClassInfo][5];

	Mobs.Player.bStatus.maxHP = hp;
	Mobs.Player.bStatus.maxMP = mp;

	GetScorePoint();
	GetSkillPoint();
	GetMasterPoint();
}


void CMob::GetScorePoint()
{
	if (clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	st_Mob *player = &Mobs.Player;

	INT32 classMaster = Mobs.Player.Equip[0].EFV2;
	INT32 pnts = 0;
	INT32 level = player->bStatus.Level;
	if (classMaster == MORTAL)
	{
		if (level < 255)
			pnts += level * 5;
		else if (level < 301)
		{
			pnts += 1270;
			pnts += (level - 254) * 10;
		}
		else if (level < 354)
		{
			pnts += 1270;
			pnts += 450;
			pnts += (level - 300) * 20;
		}
		else
		{
			pnts += 1270;
			pnts += 450;
			pnts += 1080;
			pnts += (level - 354) * 12;
		}
	}
	else if (classMaster == ARCH)
	{
		int mortalSlot = Mobs.MortalSlot;
		int levelMortal = 399;

		if (mortalSlot != -1 && mortalSlot >= 0 && mortalSlot <= 3)
			levelMortal = Users[clientId].CharList.Status[Mobs.MortalSlot].Level;

		pnts += (levelMortal - 300 + 1) * 6;
		if (level < 255)
			pnts += level * 5;
		else if (level < 301)
		{
			pnts += 1270;
			pnts += (level - 254) * 10;
		}
		else if (level < 354)
		{
			pnts += 1270;
			pnts += 450;
			pnts += (level - 300) * 20;
		}
		else
		{
			pnts += 1270;
			pnts += 450;
			pnts += 1080;
			pnts += (level - 354) * 12;
		}

		if (level >= 369)
			if (player->Equip[0].EFV3 >= 4)
				pnts += 300;
	}
	else if (classMaster == CELESTIAL || classMaster == SUBCELESTIAL)
	{
		int levelSub = -1;
		int totalResets = 0;
		if (classMaster == CELESTIAL && Mobs.Sub.Status == 1)
		{
			levelSub = Mobs.Sub.SubStatus.Level;
		}
		else if (classMaster == SUBCELESTIAL)
		{
			levelSub = level;
			level = Mobs.Sub.SubStatus.Level;

			Mobs.Info.Level355 = Mobs.Sub.Info.Level355;
			Mobs.Info.Level370 = Mobs.Sub.Info.Level370;
			Mobs.Info.Level380 = Mobs.Sub.Info.Level380;
			Mobs.Info.Level398 = Mobs.Sub.Info.Level398;
			Mobs.Info.Level399 = Mobs.Sub.Info.Level399;
			Mobs.Info.Elime = Mobs.Sub.Info.Elime;
			Mobs.Info.Sylphed = Mobs.Sub.Info.Sylphed;
			Mobs.Info.Noas = Mobs.Sub.Info.Noas;
			Mobs.Info.Thelion = Mobs.Sub.Info.Thelion;
		}

		if (levelSub != -1)
		{
			if (Mobs.Info.Reset_1 || Mobs.Sub.Info.Reset_1)
				totalResets++;
			if (Mobs.Info.Reset_2 || Mobs.Sub.Info.Reset_2)
				totalResets++;
			if (Mobs.Info.Reset_3 || Mobs.Sub.Info.Reset_3)
				totalResets++;
		}

		stQuestInfo info = Mobs.Info;
		if (info.Level355)
			pnts += 100;
		else if (info.Level370)
			pnts += 300;
		else if (info.Level380)
			pnts += 600;
		else if (info.Level398)
			pnts += 900;
		else if (info.Level399)
			pnts += 1200;
		else
			pnts += 1200;

		pnts += (totalResets * 200);

		if (info.Elime)
			pnts += 100;
		if (info.Sylphed)
			pnts += 100;
		if (info.Noas)
			pnts += 100;
		if (info.Thelion)
			pnts += 100;

		pnts += 1001;
		pnts += level * 6;

		if (level >= 150)
			pnts += ((level - 149) * 4);

		if (levelSub != -1)
		{
			pnts += (levelSub * 6);
			if (levelSub >= 150)
				 pnts += ((levelSub - 149) * 4);
		}
	}

	int classindex = player->ClassInfo;
	int str = player->bStatus.STR - BaseSIDCHM[classindex][0];
	int _int = player->bStatus.INT - BaseSIDCHM[classindex][1];
	int dex = player->bStatus.DEX - BaseSIDCHM[classindex][2];
	int con = player->bStatus.CON - BaseSIDCHM[classindex][3];
	int total = str + _int + dex + con;

	int otherPnts = pnts;
	pnts -= total;

	bool needScore = false;
	if (pnts < 0)
	{
		needScore = true;
		for (; pnts < 0; pnts++)
		{
			if (player->bStatus.STR > BaseSIDCHM[classindex][0])
				player->bStatus.STR--;
			else if (player->bStatus.INT > BaseSIDCHM[classindex][1])
				player->bStatus.INT--;
			else if (player->bStatus.DEX > BaseSIDCHM[classindex][2])
				player->bStatus.DEX--;
			else if (player->bStatus.CON > BaseSIDCHM[classindex][3])
				player->bStatus.CON--;
		}
	}

	otherPnts -= total;
	player->StatusPoint = otherPnts;

	if (needScore)
	{
		SendScore(clientId);
		SendEtc(clientId);
	}
}

void CMob::GetSkillPoint()
{
	st_Mob *player = &Mobs.Player;

	INT32 classMaster = Mobs.Player.Equip[0].EFV2;
	INT32 pnts = 0;
	INT32 level = player->bStatus.Level;

	if (classMaster == MORTAL)
		pnts += level * 3;
	else if (classMaster == ARCH)
	{
		INT32 levelMortal = Users[clientId].CharList.Status[Mobs.MortalSlot].Level;

		pnts = ((levelMortal - 300 + 1) * 6);

		if (level < 255)
			pnts += level * 4;
		else if (level < 301)
		{
			pnts += 1016;
			pnts += (level - 254) * 3;
		}
		else if (level < 354)
		{
			pnts += 1016;
			pnts += 137;
			pnts += (level - 300) * 4;
		}
		else
		{
			pnts += 1016;
			pnts += 137;
			pnts += 212;
			pnts += (level - 353) * 4;
		}
	}
	else if (classMaster == CELESTIAL || classMaster == SUBCELESTIAL)
	{
		stQuestInfo info = Mob[clientId].Mobs.Info;

		if (info.Level355)
			pnts += 600;
		else if (info.Level370)
			pnts += 800;
		else if (info.Level380)
			pnts += 1000;
		else if (info.Level398)
			pnts += 1200;
		else if (info.Level399)
			pnts += 1600;
		else
			pnts += 1600;

		pnts += level * 4;
	}

	if (Mobs.Info.Pilula)
		pnts += 9;

	int totalPnts = 0;
	int initial = player->ClassInfo * 24;
	for (int i = initial; i < initial + 24; i++)
	{
		int has = (player->Learn[0] & (1 << (i % 24)));
		if (has != 0)
			totalPnts += SkillData[i].Points;
	}

	int otherPnts = pnts;
	pnts -= totalPnts;

	if (player->SkillPoint != pnts)
	{
		player->SkillPoint = pnts;

		SendScore(clientId);
	}
}

void CMob::GetMasterPoint()
{
	st_Mob *player = &Mobs.Player;

	INT32 classMaster = Mobs.Player.Equip[0].EFV2;
	INT32 pnts = 0;
	INT32 level = player->bStatus.Level;

	if (classMaster == MORTAL)
		pnts = level * 2;
	else if (classMaster == ARCH)
	{
		level += 50;
		pnts = (level * 2);
	}
	else if (classMaster == CELESTIAL || classMaster == SUBCELESTIAL)
		pnts = 855;//500 + (level * 3);

	int otherPnts = pnts;
	pnts -= player->bStatus.Mastery[0];
	pnts -= player->bStatus.Mastery[1];
	pnts -= player->bStatus.Mastery[2];
	pnts -= player->bStatus.Mastery[3];

	if (player->MasterPoint != pnts)
	{
		player->MasterPoint = pnts;

		SendScore(clientId);
	}
}

void CMob::GetCurrentScore(int clientId)
{
	if (clientId < MAX_PLAYER)
		Mobs.Player.CapeInfo = 0;

	if (clientId < MAX_PLAYER && Mobs.Player.bStatus.Level < 1011)
	{
		Mobs.Player.Resist.Fogo = GetMobAbility(&this->Mobs, EF_RESIST1);
		Mobs.Player.Resist.Gelo = GetMobAbility(&this->Mobs, EF_RESIST2);
		Mobs.Player.Resist.Sagrado = GetMobAbility(&this->Mobs, EF_RESIST3);
		Mobs.Player.Resist.Trovao = GetMobAbility(&this->Mobs, EF_RESIST4);

		Mobs.Player.Equip[0].Effect[0].Index = 0;
		Mobs.Player.Equip[0].Effect[0].Value = 0;
		Mobs.Player.AffectInfo.Value = 0;
		Mobs.AffectInfo = 0;

		GetStatusBaseScore();
	}
	else if (clientId >= MAX_PLAYER)
	{
		INT32 LOCAL_2 = GenerateID;
		if (LOCAL_2 > 0 && LOCAL_2 < MAX_NPCGENERATOR)
		{
			Mobs.Player.Resist.Fogo = mGener.pList[LOCAL_2].Leader.Resist.Fogo;
			Mobs.Player.Resist.Gelo = mGener.pList[LOCAL_2].Leader.Resist.Gelo;
			Mobs.Player.Resist.Sagrado = mGener.pList[LOCAL_2].Leader.Resist.Sagrado;
			Mobs.Player.Resist.Trovao = mGener.pList[LOCAL_2].Leader.Resist.Trovao;
		}
	}

	Parry = GetMobAbility(&this->Mobs, EF_PARRY);
	HitRate = GetMobAbility(&this->Mobs, EF_HITRATE);
	
	if (clientId < MAX_PLAYER)
	{
		INT32 range = GetMobAbility(&this->Mobs, EF_RANGE);
		if (Mobs.Player.Learn[0] & 0x20000000)
			range++;

		if (Mobs.Player.Learn[0] & 0x80000)
			range++;

		Users[clientId].Range = range;
	}

	ExpBonus = 0;
	DropBonus = 0;
	ForceDamage = 0;
	ReflectDamage = 0;
	PotionBonus = 0;
	LifeSteal = 0;
	Vampirism = 0;
	IndividualExpBonus = 0;
	IgnoreResistance = 0;
	ResistanceChance = 0;
	GetCurScore(this, Mobs.Affects);

	//-------------
	// 00412589 até 0041261E não descompilado, é a parte de HP, mas já tem a função GetCurrentHP(); e MP();
	if (Mobs.Player.Status.curHP > Mobs.Player.Status.maxHP)
		Mobs.Player.Status.curHP = Mobs.Player.Status.maxHP;

	if (Mobs.Player.Status.curMP > Mobs.Player.Status.maxMP)
		Mobs.Player.Status.curMP = Mobs.Player.Status.maxMP;

	INT32 _weaponDamage = GetItemAbility(&Mobs.Player.Equip[6], EF_DAMAGE);
	INT32 _shieldDamage = GetItemAbility(&Mobs.Player.Equip[7], EF_DAMAGE);

	if (_weaponDamage > _shieldDamage)
		WeaponDamage = _weaponDamage + (_shieldDamage / 3);
	else
		WeaponDamage = _shieldDamage + (_weaponDamage / 3);

	INT32 weaponId = Mobs.Player.Equip[6].Index;
	INT32 weaponPos = ItemList[weaponId].Pos;

	if ((weaponId >= 0 || weaponId < MAX_ITEMLIST) && (weaponPos == 64 || weaponPos == 192))
	{
		INT32 sanc = GetItemSanc(&Mobs.Player.Equip[6]);
		if (sanc == 9)
			WeaponDamage += 40;
	}

	INT32 shieldId = Mobs.Player.Equip[7].Index;
	INT32 shieldPos = ItemList[shieldId].Pos;

	if ((shieldId > 0 || shieldId < MAX_ITEMLIST) && (shieldPos == 64 || shieldPos == 192))
	{
		INT32 sanc = GetItemSanc(&Mobs.Player.Equip[7]);
		if (sanc == 9)
			WeaponDamage += 40;
	}

	for (int i = 1; i < 8; i++)
	{
		st_Item *item = &Mobs.Player.Equip[i];
		if (!item->Index)
			continue;

		int sanc = GetItemSanc(item);
		if ((ItemList[item->Index].Grade >= 5 && ItemList[item->Index].Grade <= 8) || sanc >= 10)
		{ // Item com ancient
			if (ItemList[item->Index].Grade == 5)
			{ // Drop bonus
				if (sanc <= 9)
					DropBonus += 2;
				else if (sanc <= 15)
					DropBonus += 4;
			}
			else if (ItemList[item->Index].Grade == 6)
			{
				if (sanc <= 9)
					ForceDamage += 40;
				else if (sanc <= 15)
					ForceDamage += 80 + (80 * (sanc - 10));
			}
			else if (ItemList[item->Index].Grade == 7)
			{
				if (sanc <= 9)
					ExpBonus += 2;
				else if (sanc <= 15)
					ExpBonus += 4;
			}
			else if (ItemList[item->Index].Grade == 8)
			{
				if (sanc <= 9)
					ReflectDamage += 40;
				else if (sanc <= 15)
					ReflectDamage += 80 + (80 * (sanc - 10));
			}
			else if (sanc >= 10)
			{
				INT32 value = -1;
				for (INT32 p = 0; p < 3; p++)
				{
					if (item->Effect[p].Index == 43 || (item->Effect[p].Index >= 116 && item->Effect[p].Index <= 125))
					{
						value = item->Effect[p].Value;
						break;
					}
				}

				if (value != -1)
				{
					value = (value - 230) % 4;

					if (value == 0)
						DropBonus += 2;
					else if (value == 1)
					{
						if (sanc <= 9)
							ForceDamage += 20;
						else if (sanc <= 15)
							ForceDamage += (40 * (sanc - 9));
					}
					else if (value == 2)
						ExpBonus += 2;
					else if (value == 3)
					{
						if (sanc <= 9)
							ReflectDamage += 40;
						else if (sanc <= 15)
							ReflectDamage += (40 * (sanc - 9));
					}
				}
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int abs = 0;
		int perf = 0;

		st_Item *item = &Mobs.Player.Equip[8 + i];
		if (!item->Index)
			continue;

		switch (item->Index)
		{
		case 540:
			perf = 10;
			break;
		case 541:
			abs = 10;
			break;
		case 631:
			perf = 15;
			break;
		case 632:
			abs = 15;
			break;
		case 633:
			abs = 15;
			perf = 15;
			break;
		case 567:
		case 568:
		case 569:
		case 570:
			abs = 15;
			perf = 15;
			break;
		}

		int sanc = GetItemSanc(item);
		if (sanc == 0)
			sanc = 1;

		abs += (sanc * 2);
		perf += (sanc * 2);

		ForceDamage += perf;
		ReflectDamage += abs;
	}

	ExpBonus += GetMobAbility(&Mobs, EF_BONUSEXP);
	IndividualExpBonus += GetMobAbility(&Mobs, EF_BONUSEXPIND);
}

bool CMob::isNormalPet() const {
	const st_Item& item = Mobs.Player.Equip[14];

	return item.Index >= 2360 && item.Index <= 2389;
}

bool CMob::isPetAlive() const
{
	const st_Item& item = Mobs.Player.Equip[14];
	if (item.Index == 0)
		return false;

	if (isNormalPet() && *(short*)&item.Effect[0].Index > 0)
		return true;

	if (item.Index >= 3980 && item.Index <= 3999)
		return true;

	return false;
}

void GetCurScore(CMob *player, st_Affect *affect)
{
	INT32 clientId = ((UINT32)player - (UINT32)&Mob[0]) / sizeof CMob;
	st_Mob *mob = &player->Mobs.Player;

	player->Mobs.Player.AffectInfo.Value = 0;
	player->Mobs.AffectInfo = 0;

	INT32	curHP = player->Mobs.Player.Status.curHP,// EBP - 04h - LOCAL_1
			curMP = player->Mobs.Player.Status.curMP;// EBP - 08h - LOCAL_1

	memcpy(&player->Mobs.Player.Status, &player->Mobs.Player.bStatus, sizeof st_Status);

	player->Mobs.Player.Status.curHP = curHP;
	player->Mobs.Player.Status.curMP = curMP;

	INT32 _critical = static_cast<int>(((GetMobAbility(&player->Mobs, EF_CRITICAL) / 10.0F) * 2.5F));
	INT32 attack = mob->bStatus.Attack + GetMobAbility(&player->Mobs, EF_DAMAGE);
	INT32 defense = mob->bStatus.Defense + GetMobAbility(&player->Mobs, EF_AC) + GetMobAbility(&player->Mobs, EF_ACADD);
	INT32 maxHP = mob->bStatus.maxHP + GetMobAbility(&player->Mobs, EF_HP);
	INT32 maxMP = mob->bStatus.maxMP + GetMobAbility(&player->Mobs, EF_MP);
	INT32 mastery[4] = { 0, 0, 0, 0 };

	INT32 _str = mob->Status.STR + GetMobAbility(&player->Mobs, EF_STR),
		_des = mob->Status.DEX + GetMobAbility(&player->Mobs, EF_DEX),
		_int = mob->Status.INT + GetMobAbility(&player->Mobs, EF_INT),
		_con = mob->Status.CON + GetMobAbility(&player->Mobs, EF_CON);

	INT32 resist[5] = { 0, 0, 0, 0, 0 };
	for (INT32 i = 0; i < 4; i++)
		resist[i] = GetMobAbility(&player->Mobs, EF_RESIST1 + i);

	INT32 masteryAll = GetMobAbility(&player->Mobs, EF_SPECIALALL);
	for (INT32 i = 0; i < 4; i++)
	{
		INT32 tmp = mob->Status.Mastery[i] + GetMobAbility(mob, EF_SPECIAL1 + i) + masteryAll;
		if (tmp > 255)
			tmp = 255;

		mastery[i] = tmp;
	}

	INT32 speed[2];
	speed[0] = GetMobAbility(&player->Mobs, EF_RUNSPEED) + 2; // local9
	speed[1] = ((mob->bStatus.Move.Speed >> 4) * 10) + GetMobAbility(&player->Mobs, EF_ATTSPEED); // local10

	if (speed[0] > 6)
		speed[0] = 6;

	INT32 saveMana = GetMobAbility(&player->Mobs, EF_SAVEMANA);
	INT32 magic = GetMobAbility(&player->Mobs, EF_MAGIC) / 3;
	INT32 regenHP = GetMobAbility(&player->Mobs, EF_REGENHP) / 2;
	INT32 regenMp = GetMobAbility(&player->Mobs, EF_REGENMP) / 2;

	int lifeSteal = GetMobAbility(&player->Mobs, EF_LIFESTEAL);
	int vampirism = GetMobAbility(&player->Mobs, EF_VAMPIRISM);
	int potionBonus = GetMobAbility(&player->Mobs, EF_POTIONHEAL);
	int ignoreResistance = GetMobAbility(&player->Mobs, EF_IGNORERESISTANCE);
	int slowChance = GetMobAbility(&player->Mobs, EF_SLOW);
	int resistanceChance = GetMobAbility(&player->Mobs, EF_RESISTANCE);

	INT32 body = mob->Equip[0].Index / 10;
	if (body < 4)
	{
		mob->Equip[0].Effect[0].Index = EF_SANC;
		mob->Equip[0].Effect[0].Value = 0;

		switch (mob->CapeInfo)
		{
		case 7:
		case 8:
		case 9:
			mob->CapeInfo = 0;
			break;
		}
	}

	INT32 cape = mob->Equip[15].Index;
	if (mob->CapeInfo != 4)
	{
		for (size_t index = 0; index < 2; index++)
		{
			for (size_t capeIndex = 0; capeIndex < g_pCapesID[index].size(); capeIndex++)
			{
				if (g_pCapesID[index][capeIndex] == cape)
				{
					mob->CapeInfo = CAPE_BLUE + static_cast<char>(index);

					// para o loop
					index = g_pCapesID.size();
					break;
				}
			}
		}
	}

	INT32 incHp = GetMobAbility(&player->Mobs, EF_HPADD) + 100 + GetMobAbility(&player->Mobs, EF_HPADD2),
		incMp = GetMobAbility(&player->Mobs, EF_MPADD) + 100 + GetMobAbility(&player->Mobs, EF_MPADD2),
		incDamage = 100,
		incAc = 100,
		level = mob->bStatus.Level;

	INT32 local20 = 0;
	INT32 local21 = 0;
	INT32 transformation = -1;
	INT32 magicJewel = 0;
	if (player->Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
		level += 400;

	for (INT8 i = 0; i < 32; i++)
	{
		INT32 index = affect[i].Index; // local24
		if (!index)
			continue;

		INT32 master = affect[i].Master; // local25
		INT32 value = affect[i].Value; // local26

		switch (index)
		{
		case 1: // lentidao
		{
			if (master == 255)
				speed[0] -= 3;
			else if (master >= 200)
				speed[0] -= 2;
			else
				speed[0] -= 1;

			speed[1] -= 30;

			break;
		}
		case 2: // velocidade
		{
			speed[0] += master;

			player->Mobs.AffectInfo |= 0x80;
			break;
		}
		case 3: // resistencia
		{
			INT32 tmpResist = 0; // local29
			INT32 resistDec = master; // local30

			if (mob->Equip[0].Index <= 50)
				resistDec >>= 1;
			else
				resistDec -= 10;

			for (INT8 i = 0; i < 4; i++)
			{
				tmpResist = resist[i] - resistDec;
				if (tmpResist < 0)
					tmpResist = 0;

				if (tmpResist > 100)
					tmpResist = 100;

				resist[i] = tmpResist;
			}
			break;
		}
		case 55:
		{
			incHp += 10;
			incMp += 10;
		}
		break;
		case 4:
		{
			switch (value)
			{
			case 4:
				incDamage += 13;
				incHp += 15;
				incMp += 10;
				magic += 5;
				break;

			case 5:
				incDamage += 7;
				incHp += 10;
				magic += 3;
				break;

			case 6: // poção kappa
				attack += 75;
				magic += 1;
				break;

			case 7:
				attack += 120;
				magic += 2;
				break;
			case 8:
				attack += 140;
				magic += 3;
				break;

			default:
				incDamage += 3;
				attack += 20;

				magic += 5;
				break;
			}
			break;
		}
		case 5: // evasao
		{
			float fValue = (100.0f - master) / 100.0f; // local31

			_des *= static_cast<INT32>(fValue);
			break;
		}
		case 6: // poção atk
		{
			float fValue = (100.0f + master) / 100.0f; // local32

			_des *= fValue;
			break;
		}
		case 7: // velocidade
		{
			INT32 speedDec = (value / 10) + 10; // local33
			speed[1] -= speedDec;
			speed[0] -= 1;

			if (mob->Equip[0].Index <= 50)
				continue;

			_int -= (speedDec + 10);
			break;
		}
		case 8: // add
		{
			//	speed[1] += (value / 10) + 10; // local35

			if (value & 2)
				resist[4] += 25;

			if (value & 8)
				player->ReflectDamage += 300;

			if (value & 16)
			{
				incAc += 10;
				incHp += 10;
			}

			if (value & 32)
			{
				incHp += 10;
				incDamage += 7;
			}

			if (value & 128)
				magicJewel = 1;
		}
		break;

		case 9: // buff attack
		{
			INT32 atk_bonus = ((((value * 5) / 20) + master) * 3) >> 1; // local36

			incDamage += 5;

			if (mob->ClassInfo == 1)
			{
				// Checa se possui a habilidade Arma mágica
				if ((mob->Learn[0] & 100000))
				{
					atk_bonus *= 4;
					incDamage += 10;
				}
			}

			attack += atk_bonus;
			break;
		}
		case 10: // decrease attack
		{
			attack -= (value / 3) + master; // local37
			break;
		}
		case 11: // magic shield
		{
			defense += (value / 3) + master; // local38
			break;
		}
		case 12: // defense n
		{
			float fValue = (100.0f - master) / 100.0; // local39

			defense = mob->Status.Defense * fValue;
			break;
		}
		case 13: // assalto
		{
			incDamage += (value / 10) + master; // local40

			incAc -= 5;
			break;
		}
		case 14: // possuido
		{
			// 7556
			if (mob->Learn[0] & (1 << 7))
				_con += ((value * 4) + (master * 2));
			else
				_con += ((value * 3)) + master; // local41 - local42
			break;
		}
		case 15: // skills - fm
		{
			INT32 masterInc = (value / 10) + master; // local43

			for (INT8 i = 0; i < 4; i++)
				mastery[i] += masterInc;

			break;
		}
		case 16: // transformação
		{
			INT32 transInfo = master - 1; // local50
			transformation = transInfo;

			if (transInfo < 0 || transInfo >= 5) // TODO: implementar eden
				continue;

			if (mob->ClassInfo != 2)
				continue;

			if (transInfo == 4)
				mob->Equip[0].Index = 32;
			else
				mob->Equip[0].Index = 22 + transInfo; // 22 = face lobisomem

			INT32 atkBonus = 0; // local51
			INT32 hpBonus = 0; // local52
			INT32 acInc = 0; // local53
			INT32 speedInc = 0; // local54
			INT32 LOCAL_55 = mob->Learn[0] & (1 << 17);
			INT32 LOCAL_56 = mob->Learn[0] & (1 << 19); //local56
			INT32 metamorfose = mob->Learn[0] & (1 << (69 % 24)); //local57
			INT32 resistTrans = 0; //local58

			if (mob->Equip[0].Index == 22)
			{
				atkBonus += 13;
				resistTrans += 15;
			}
			else if (mob->Equip[0].Index == 23)
			{
				hpBonus += 150;
				resistTrans += 32;
				speedInc += 20;
			}
			else if (mob->Equip[0].Index == 24)
			{
				acInc += 8;
				atkBonus += 8;
			}
			else if (mob->Equip[0].Index == 25)
				resistTrans += 20;
			else if (mob->Equip[0].Index == 32)
			{
				atkBonus += 7;
				hpBonus += 10;
				acInc += 4;
				speedInc += 20;
				resistTrans += 25;
			}

			if (metamorfose)
			{
				atkBonus += 3;
				hpBonus += 25;
			}

			INT32 levelAux = mob->Status.Level;
			INT32 sanc = ((((levelAux * 2) + mastery[3]) / 3) - pTransBonus[transInfo].Sanc) / 12; // local59
			if (sanc < 0)
				sanc = 0;

			if (sanc > 9)
				sanc = 9;

			if (player->Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
				sanc = 9;

			mob->Equip[0].Effect[0].Index = 43;
			mob->Equip[0].Effect[0].Value = sanc & 255;

			INT32 auxStatus = 0; // local45
			INT32 min = 0; // local46
			INT32 max = 0; // local47
			INT32 max_min = 0; // local48
			INT32 auxValue = 0; // local49

			min = pTransBonus[transInfo].MinAttack + atkBonus; // local46
			max = pTransBonus[transInfo].MaxAttack + atkBonus; // local47
			max_min = max - min; // local48

			auxValue = ((max_min * value) / 300) + min;

			auxStatus = attack; // local45

			if (mob->Equip[0].Index == 22)
				auxStatus += 25;

			attack = auxStatus;
			incDamage += auxValue - 100; // local22

			min = pTransBonus[transInfo].MinDefense + acInc;
			max = pTransBonus[transInfo].MaxDefense + acInc;
			max_min = max - min;

			auxValue = ((max_min * value) / 300) + min;

			auxStatus = defense;
			auxStatus = (auxStatus * auxValue) / 100;

			if (mob->Equip[0].Index == 22)
				auxStatus += 5;

			defense = auxStatus;

			//0040738A
			min = pTransBonus[transInfo].MinHp + hpBonus;
			max = pTransBonus[transInfo].MaxHp + hpBonus;
			max_min = (max - min) + 25;

			auxValue = ((max_min * value) / 200) + min;

			auxStatus = maxHP;
			auxStatus = (auxStatus * auxValue) / 100;

			maxHP = auxStatus;

			for (INT8 i = 0; i < 4; i++)
			{
				int tmpResist = resist[i] + resistTrans;
				if (tmpResist < -50)
					tmpResist = -50;

				if (tmpResist > MAX_RESIST)
					tmpResist = MAX_RESIST;

				resist[i] = tmpResist;
			}

			if (transInfo == 0)
				_critical += 50;

			local20 = pTransBonus[transInfo].SpeedAttack + speedInc;
			local21 = pTransBonus[transInfo].SpeedMove;
			break;
		}
		case 38: // controle de mana - 18
		{ // alterado para a skill Troca de Espíritos
			INT32 mp = ((mob->Status.Level + value) >> 1) + maxMP; // local61

			maxHP += mp;
			maxMP -= (mp / 3);
			break;
		}
		case 19: // imunidade
		{
			player->Mobs.AffectInfo |= 0x02;
			break;
		}
		case 21: // meditação
		{
			defense -= (value / 3) + 10; // local62

			incDamage += (value / 10) + master; // local63
			break;
		}
		case 24: // samaritano
		{
			int acIncress = ((((value * 5) / 15) + master) * 3) >> 1;
			int itemId = player->Mobs.Player.Equip[7].Index;

			if (itemId > 0 && itemId < MAX_ITEMLIST)
			{
				if (ItemList[itemId].Unique == 51)
				{
					acIncress += (acIncress / 3);

					for (int i = 0; i < 4; ++i)
						resist[i] += 15;
				}
			}

			defense += acIncress;
		}
		break;
		case 25: // proteção elemental
		{
			INT32 sum = (value >> 2) + master;
			if ((player->Mobs.Player.Learn[0] & 128))
				sum = (value >> 1) + master + 75;

			defense += sum;
			for (int i = 0; i < 4; i++)
				resist[i] += 22;
		}
		break;
		case 36: // veneno
		{
			player->Mobs.Player.AffectInfo.Value |= 0x10;
		}
		break;
		case 26: // evasão
		{
			player->Mobs.Player.AffectInfo.Value |= 0x20;
		}
		break;
		case 28: // invisibilidade
		{
			player->Mobs.Player.AffectInfo.Value |= 0x40;
		}
		break;
		case 27: // congelar da HT
		{
			// SlowMov, tá ligado, brother manolo doidera
			player->Mobs.Player.AffectInfo.Value |= 0x04;
		}
		break;
		case 29:
		{
			INT32 soul = player->Mobs.Soul;
			if (player->Mobs.Affects[i].Value == 99 && player->Mobs.Player.Equip[0].EFV2 == MORTAL)
			{
				maxHP = 3000;
				maxMP = 3000;

				_str = 1000;
				_int = 1000;
				continue;
			}

			INT32 val = 120;
			if (player->Mobs.Player.Equip[0].EFV2 == MORTAL)
				val = 70;

			INT32 midVal = 80,
				midVal2 = 40;

			if (player->Mobs.Player.Equip[0].EFV2 == MORTAL)
			{
				midVal = 40;
				midVal2 = 30;
			}

			if (soul == 0) // 0 FOR + CONS 
			{
				INT32 aux = _str * midVal / 100;
				_str += aux;

				aux = (_con * midVal2 / 100);
				_con += aux;
			}
			else if (soul == 1) // 1 INT + CONS
			{
				INT32 aux = _int * midVal / 100;
				_int += aux;

				aux = (_con * midVal2 / 100);
				_con += aux;
			}
			else if (soul == 2) // 2 DESTREZA + CONS
			{
				INT32 aux = _des * midVal / 100;
				_des += aux;

				aux = (_con * midVal2 / 100);
				_con += aux;
			}
			else if (soul == 3) // 3 FORÇA + DESTREZA
			{
				INT32 aux = _con * midVal / 100;
				_str += aux;

				aux = (_str * midVal2 / 100);
				_des += aux;
			}
			else if (soul == 4) // 4 INT + DESTREZA
			{
				INT32 aux = _con * midVal / 100;
				_int += aux;

				aux = (_int * midVal2 / 100);
				_des += aux;
			}
			else if (soul == 5) // 5 CONS + DESTREZA
			{
				INT32 aux = _con * midVal / 100;
				_con += aux;

				aux = (_des * midVal2 / 100);
				_des += aux;
			}
			else if (soul == 6)
			{
				INT32 aux = _str * val / 100;
				_str += aux;
			}
			else if (soul == 7)
			{
				INT32 aux = _int * val / 100;
				_int += aux;
			}
			else if (soul == 8)
			{
				INT32 aux = _con * val / 100;
				_con += aux;
			}
			else if (soul == 9)
			{
				INT32 aux = _des * val / 100;
				_des += aux;
			}
		}
		break;
		case 31:
		{
			int acInc = (value >> 1) + master;
			defense += acInc;
			break;
		}
		case 34: // divina
		{
			// 13% de dano extra
			incDamage += 13;

			// 10 de pontos mágicos
			magic += 15;

			incHp += 30;
			incMp += 20;
		}
		break;
		case 35: // Poção do Vigor
		{
			if (value == 2)
			{
				incHp += 15;
				incMp += 15;
			}
			else if (value == 1)
			{
				incHp = 20;
				incMp = 20;
			}
		}
		case 56:
		{
			incAc -= (5 + value);
		}
		break;
		}
	}

	for (int i = 1; i <= 7; i++)
	{// i = local66
		int itemIndex = mob->Equip[i].Index; // LOCAL67
		if (itemIndex <= 0 || itemIndex > MAX_ITEMLIST)
			continue;

		int sanc = GetItemSanc(&mob->Equip[i]);
		if (sanc >= 9)
		{
			int itemPos = ItemList[itemIndex].Pos; //local69
			switch (itemPos)
			{
			case 4:
			case 8:
				defense += 25;
				break;
			case 16:
				player->Mobs.Player.AffectInfo.SkillDelay = 1;
				break;

			case 128:
				defense += 25;
				break;

			case 64:
			case 192:
			{
				int unique = ItemList[itemIndex].Unique; //local72;
				if (unique == 47)
					magic += 4;
				else
					attack += 40;
			}
			break;
			}
		}
	}

	if ((mob->Equip[0].Index >= 22 && mob->Equip[0].Index <= 25) || mob->Equip[0].Index == 32)
	{
		INT32 t = 0;
		for (; t < 32; t++)
		{
			if (affect[t].Index == 16)
				break;
		}

		if (t == 32)
		{
			mob->Equip[0].Index = mob->Equip[0].EF2;

			SendEquip(player->clientId);
		}
	}

	// Skill passivas
	int _classInfo = mob->ClassInfo;
	switch (_classInfo)
	{
	case 0:
	{  // TK
		incAc += 10;

		if (mob->Learn[0] & (1 << ((9 % 24)))) // mestre das armas
		{
			if (mob->Equip[6].Index != 0 && mob->Equip[7].Index != 0 && ItemList[mob->Equip[7].Index].Pos != 128)
				incDamage += 2;
		}

		//if((mob->Learn[0] & ( 1 << ((14 % 24)) )))
		//	attack += mob->Status.Mastery[2];

		//Bônus oitava skill
		if ((mob->Learn[0] & (1 << (7 % 24))))
			magic += 10;
		else if ((mob->Learn[0] & (1 << (15 % 24))))
		{
			incAc += 7;
			_critical += 25;
			lifeSteal += 5;
		}
		else if ((mob->Learn[0] & (1 << (23 % 24))))
			magic += 10;
	}
	break;
	case 1:
	{ // foema
		// Bônus oitava skill
		if ((mob->Learn[0] & (1 << (31 % 24))))
			maxHP += (maxHP * 10 / 100);
		else if ((mob->Learn[0] & (1 << (39 % 24))))
			magic += 5;
		else if ((mob->Learn[0] & (1 << (47 % 24))))
		{
			attack += 250;
			incDamage += 10;

			if (_des + _str > _int)
				incHp += 10;
		}
	}
	break;
	case 2:
	{ // beastmaster
		short SrcPos = GetItemAbility(&mob->Equip[7], EF_POS);
		if (SrcPos == 128) // Escudo
		{
			if ((mob->Learn[0] & (1 << (67 % 24))))
			{
				incAc += 2;
				defense += mob->Status.Mastery[3] / 2;
			}
		}

		if ((mob->Learn[0] & (1 << (65 % 24))))
		{
			player->ReflectDamage += 100;
			if (SrcPos == 128 || (mob->Learn[0] & (1 << (71 % 24))))
				player->ReflectDamage += 75;
		}

		// Bônus oitava skill
		if ((mob->Learn[0] & (1 << (55 % 24))))
		{
			incAc += 3;
			magic += 8;
		}
		else if ((mob->Learn[0] & (1 << (63 % 24))))
		{
			incAc += 5;
			defense += 200;
		}
		else if ((mob->Learn[0] & (1 << (71 % 24))))
		{
			incHp += 8;
			maxHP += 700;
			attack += 200;
		}
	}
	break;
	case 3:
	{
		int unique = ItemList[mob->Equip[6].Index].Unique;

		if ((mob->Learn[0] & (1 << (74 % 24))) && (unique == 42 || unique == 43))
			attack += (mob->Status.Mastery[1] + 10);

		if ((mob->Learn[0] & (1 << (82 % 24))))
		{
			if (mob->Equip[6].Index != 0 && mob->Equip[7].Index != 0 && GetItemAbility(&mob->Equip[7], EF_POS) != 128)
				attack += (mob->Status.Mastery[2] * 2);
		}

		if ((mob->Learn[0] & (1 << (90 % 24))))
			_critical += (_des / 80);

		if ((mob->Learn[0] & (1 << ((94 % 24)))))
			if (unique == 43)
				defense += ((mob->Status.Mastery[3] * 3) / 5);

		// Bônus Oitava skill
		if ((mob->Learn[0] & (1 << (79 % 24))) != 0)
		{
			short wType = GetEffectValueByIndex(mob->Equip[6].Index, EF_WTYPE);

			if (wType == 101)
			{// Arco
				attack += 60;;
				incDamage += 3;
			}
		}

		else if ((mob->Learn[0] & (1 << (87 % 24))) != 0)
			incHp += 7;
		else if ((mob->Learn[0] & (1 << (95 % 24))) != 0)
			incAc += 10;
	}
	break;
	}

	if (player->Mobs.Player.Equip[0].EFV2 >= CELESTIAL && clientId < MAX_PLAYER)
	{
		if (player->Mobs.Player.ClassInfo != 1 || (_int < _str + _des && player->Mobs.Player.ClassInfo == 1))
			maxHP += 1000;
	}

	if (player->Target.X >= 3449 && player->Target.X <= 3979 && player->Target.Y >= 2673 && player->Target.Y <= 3221)
	{
		if (!player->isPetAlive())
			local21 -= 2;
	}

	speed[1] = speed[1] + local20 + (_des / 5);
	speed[0] += local21;

	if (speed[0] > 6)
		speed[0] = 6;

	if (speed[0] <= 0)
		speed[0] = 0;

	if (clientId > 0 && clientId < MAX_PLAYER && Users[clientId].IsAdmin)
		speed[0] = 7;

	INT32 tmpspeed = speed[1];
	if (tmpspeed < 0)
		tmpspeed = 0;

	if (tmpspeed > 150)
		tmpspeed = 150;

	player->Mobs.Player.Status.Move.Value = (tmpspeed << 4) + speed[0];

	if (body < 4)
	{
		attack += (_str / 3) + mastery[0] + (_des / 4);
		//	defense += level;

		maxHP += (_con * 2);
		maxMP += (_int * 2);
	}

	// Bonus ao conquistar cidades
	for (int i = 0; i < 5; i++)
	{
		if (!ChargedGuildList[sServer.Channel - 1][i])
			continue;

		// 1% de bonus de ataque para cada cidade conquistada.
		if (player->Mobs.Player.GuildIndex == ChargedGuildList[sServer.Channel - 1][i])
		{
			maxHP += 50;
			maxMP += 50;
		}
	}

	if (clientId < MAX_PLAYER && Mob[clientId].Mobs.Player.Equip[0].EFV2 == ARCH)
	{
		if (player->Mobs.Info.Elime)
			maxMP += 80;

		if (player->Mobs.Info.Sylphed)
			defense += 30;

		if (player->Mobs.Info.Thelion)
			maxHP += 80;

		if (player->Mobs.Info.Noas)
		{
			maxHP += 60;
			maxMP += 60;
			defense += 20;
		}
	}

	if (sServer.AnnubisBonus != 0)
	{
		if (mob->GuildIndex == sServer.AnnubisBonus)
		{
			incAc += 3;
			incDamage += 3;
			magic += 2;
		}
	}

	if (sServer.RvR.Bonus != 0)
	{
		if (sServer.RvR.Bonus == mob->CapeInfo)
		{
			incHp += 2;
			incMp += 2;
		}
	}

	if (sServer.KingdomBattle.Winner != 0 && sServer.KingdomBattle.Winner == mob->CapeInfo)
	{
		incHp += 2;
		incMp += 2;
	}

	INT32 pkPoint = player->Mobs.Player.Inventory[63].Effect[0].Index - 75;
	if (pkPoint < 0 && clientId < MAX_PLAYER)
	{
		pkPoint = -pkPoint;

		maxHP -= ((maxHP * (pkPoint + 15)) / 100);
		maxMP -= ((maxMP * (pkPoint + 15)) / 100);

		attack -= ((attack * pkPoint) / 100);
		defense -= ((defense * pkPoint) / 100);

		magic -= ((magic * pkPoint / 100));

		if (pkPoint < -50)
			speed[0] = 3;
	}

	if (player->Mobs.Player.CapeInfo == 4 && player->Summoner > 0 && player->Summoner < MAX_PLAYER)
	{
		pkPoint = Mob[player->Summoner].Mobs.Player.Inventory[63].Effect[0].Index - 75;
		if (pkPoint < 0)
		{
			pkPoint = -pkPoint;

			maxHP -= ((maxHP * (pkPoint + 10)) / 100);

			attack -= ((attack * ((pkPoint + 15)) / 100));
			defense -= ((defense * ((pkPoint + 15)) / 100));
		}
	}
	
	defense += GetBonusSet(&player->Mobs.Player, defense);

	if (incDamage != 100)
		attack = attack * incDamage / 100;

	if (incHp != 100)
		maxHP = maxHP * incHp / 100;

	if (incMp != 100)
		maxMP = maxMP * incMp / 100;

	if (incAc != 100)
		defense = defense * incAc / 100;

	regenHP += 5;
	if (regenHP < 5)
		regenHP = 5;
	player->RegenHP = regenHP;

	regenMp += 5;
	if (regenMp < 5)
		regenMp = 5;
	player->RegenMP = regenMp;

	for (int i = 0; i < 4; i++)
	{
		if (mastery[i] > 255)
			mastery[i] = 255;

		mob->Status.Mastery[i] = mastery[i];

		resist[i] += resist[4];

		if (resist[i] > MAX_RESIST)
			resist[i] = MAX_RESIST;

		*(BYTE*)((INT32)&mob->Resist.Fogo + i) = resist[i];
	}

	if (magicJewel)
	{
		INT32 removed = (maxMP / 2);
		maxMP -= removed;

		maxHP += removed;
	}

	if (attack > MAX_STATS  && clientId < MAX_PLAYER)
		attack = MAX_STATS;

	if (defense >= MAX_STATS && clientId < MAX_PLAYER)
		defense = MAX_STATS;

	if (_str > MAX_POINTS && clientId < MAX_PLAYER)
		_str = MAX_POINTS;

	if (_int > MAX_POINTS && clientId < MAX_PLAYER)
		_int = MAX_POINTS;

	if (_con > MAX_POINTS && clientId < MAX_PLAYER)
		_con = MAX_POINTS;

	if (_des > MAX_POINTS && clientId < MAX_PLAYER)
		_des = MAX_POINTS;

	if (magic > MAX_MAGICINCREMENT)
		magic = MAX_MAGICINCREMENT;

	if (maxHP > MAX_HPMP && clientId < MAX_PLAYER)
		maxHP = MAX_HPMP;

	if (maxMP > MAX_HPMP && clientId < MAX_PLAYER)
		maxMP = MAX_HPMP;

	mob->Status.Attack = attack;
	mob->Status.Defense = defense;
	mob->Status.STR = _str;
	mob->Status.INT = _int;
	mob->Status.CON = _con;
	mob->Status.DEX = _des;
	player->MagicIncrement = magic;
	/*
	INT32 index = ((UINT32)player - (UINT32)&Mob[0]) / sizeof CMob;

	if(index < MAX_PLAYER)
		Users[index].inGame.MagicIncrement = magic;*/

	if (mob->Status.curHP > maxHP)
		mob->Status.curHP = maxHP;

	mob->Status.maxHP = maxHP;

	if (mob->Status.curMP > maxMP)
		mob->Status.curMP = maxMP;

	mob->Status.maxMP = maxMP;

	if (_critical > 255)
		_critical = 255;

	mob->Critical = _critical;

	player->AttackSpeed = speed[1] + 100;
	player->PotionBonus = potionBonus;
	player->LifeSteal = lifeSteal;
	player->Vampirism = vampirism;
	player->IgnoreResistance = ignoreResistance;
	player->SlowChance = slowChance;
	player->ResistanceChance = resistanceChance;
}

INT32 CNPCGener::GetEmptyNPCMob()
{
	static int i = 1000;

	if (i >= MAX_SPAWN_MOB)
		i = 1000;

	INT32 startIndex = 29900;
	INT32 mobId = i;

	for (int x = 0; x < startIndex; x++, mobId++)
	{
		if (mobId >= MAX_SPAWN_MOB)
		{
			mobId = 1000;

			continue;
		}

		if (Mob[mobId].Mode == 0)
		{
			i = mobId;

			return mobId;
		}
	}

	return 0;
}

void GenerateMob(int arg1, int arg2, int arg3)
{
	INT32 LOCAL_1 = arg1;
	INT32 LOCAL_2 = mGener.pList[arg1].MinuteGenerate;
	INT32 LOCAL_3 = 0, LOCAL_4 = 0,
		LOCAL_5 = 0, LOCAL_6 = 0;

	static char g_pFormation[5][12][2] =
	{
		1, 1,   -1, 1,   1, -1,   -1, -1,   1, 0,
		1, 0,   -1, 0,   2,  0,   -2,  0,   3, 0,
		1, 1,   -1, 1,   1, -1,   -1, -1,   1, 0,
		1, 0,   -1, 0,   2,  0,   -2,  0,   3, 0,
		2, 0,    0, 2,   1, 1,     0, 1,    1, 0,
		1, 0,    0, 0,   0, 0,     0, 0,    1, 0,
		1, 0,    0, 0,   1, 0,     0, 0,    1, 0,
		0, 0,    0, 0,   1, 0,     0, 0,    1, 0,
		1, 0,    0, 0,   1, 0,     0, 0,    1, 0,
		1, 0,    0, 0,   1, 0,     0, 0,    1, 0,
		1, 0,    0, 0,   1, 0,     0, 0,    0, 0,
		1, 0,    0, 0,   1, 0,     0, 0,    1, 0
	};

	if (LOCAL_2 >= 500 && true)
	{
		LOCAL_3 = mGener.pList[arg1].Segment_X[0];
		LOCAL_4 = mGener.pList[arg1].Segment_Y[0];
		LOCAL_5 = LOCAL_4 - LOCAL_3;

		if (LOCAL_5 <= 0)
		{
			Log(SERVER_SIDE, LOG_ERROR, "Fim do index < que o inicio - GenerateMOB");
			return;
		}

		INT32 LOCAL_6 = LOCAL_3 + (Rand() % LOCAL_5);
		/*
		if (LOCAL_6 < 0 || LOCAL_6 < *(DWORD*)(0x15C69B0))
			Log(SERVER_SIDE, LOG_ERROR, "Relocação de indice incorreta.");
		*/
		arg1 = LOCAL_6;
	}

	INT32 LOCAL_7 = mGener.pList[LOCAL_1].Formation,
		LOCAL_8 = mGener.pList[LOCAL_1].MinGroup,
		LOCAL_9 = mGener.pList[LOCAL_1].MaxGroup - mGener.pList[LOCAL_1].MinGroup + 1;

	LOCAL_8 = LOCAL_8 + (Rand() % LOCAL_9);
	// 4478C1

	if (mGener.pList[LOCAL_1].MobCount >= mGener.pList[LOCAL_1].MaxNumMob)
		return;

	INT32 LOCAL_10 = mGener.GetEmptyNPCMob();
	if (LOCAL_10 == 0)
		return;

	Mob[LOCAL_10] = CMob{};
	Mob[LOCAL_10].BossInfoId = MAXUINT32;
	Mob[LOCAL_10].clientId = LOCAL_10;

	INT32 LOCAL_11 = LOCAL_10;
	memset(&Mob[LOCAL_10].PartyList, 0, 24);

	memcpy(&Mob[LOCAL_10].Mobs.Player, &mGener.pList[LOCAL_1].Leader, sizeof st_Mob);
	Mob[LOCAL_10].clientId = LOCAL_10;
	Mob[LOCAL_10].Mobs.Player.Name[15] = '\0';
	Mob[LOCAL_10].Mobs.Player.bStatus.Merchant.Direction = 0;

	strncpy_s(Mob[LOCAL_10].Mobs.Player.Name, mGener.pList[LOCAL_1].Leader.Name, 16);

	for (INT32 LOCAL_12 = 0; LOCAL_12 < 16; LOCAL_12++)
	{
		if (Mob[LOCAL_10].Mobs.Player.Name[LOCAL_12] == 0x5F)
			Mob[LOCAL_10].Mobs.Player.Name[LOCAL_12] = 0x20;
		if (Mob[LOCAL_10].Mobs.Player.Name[LOCAL_12] == '@')
			Mob[LOCAL_10].Mobs.Player.Name[LOCAL_12] = 0x20;
	}

	memset(Mob[LOCAL_10].Mobs.Affects, 0, sizeof st_Affect * 32);

	// 00447a14
	for (INT32 LOCAL_12 = 0; LOCAL_12 < 5; LOCAL_12++)
	{
		if (mGener.pList[arg1].Segment_X[LOCAL_12] == 0)
		{
			Mob[LOCAL_10].Segment.ListX[LOCAL_12] = 0;
			Mob[LOCAL_10].Segment.ListY[LOCAL_12] = 0;

			continue;
		}

		if (mGener.pList[arg1].SegmentRange[LOCAL_12] != 0)
		{
			Mob[LOCAL_10].Segment.ListX[LOCAL_12] = (mGener.pList[arg1].Segment_X[LOCAL_12] - mGener.pList[arg1].SegmentRange[LOCAL_12]) + ((Rand() % (mGener.pList[arg1].SegmentRange[LOCAL_12] + 1)) * 2);
			Mob[LOCAL_10].Segment.ListY[LOCAL_12] = (mGener.pList[arg1].Segment_Y[LOCAL_12] - mGener.pList[arg1].SegmentRange[LOCAL_12]) + ((Rand() % (mGener.pList[arg1].SegmentRange[LOCAL_12] + 1)) * 2);
		}
		else
		{
			Mob[LOCAL_10].Segment.ListX[LOCAL_12] = mGener.pList[arg1].Segment_X[LOCAL_12];
			Mob[LOCAL_10].Segment.ListY[LOCAL_12] = mGener.pList[arg1].Segment_Y[LOCAL_12];
		}

		Mob[LOCAL_10].Segment.Wait[LOCAL_12] = mGener.pList[arg1].SegmentWait[LOCAL_12];
	}

	if (arg2 != 0 && arg3 != 0)
	{
		for (INT32 LOCAL_12 = 0; LOCAL_12 < 5; LOCAL_12++)
		{
			Mob[LOCAL_10].Segment.ListX[LOCAL_12] = arg2 + (Rand() % 5) - 2;
			Mob[LOCAL_10].Segment.ListY[LOCAL_12] = arg3 + (Rand() % 5) - 2;
		}
	}

	if (Mob[LOCAL_10].Mobs.Player.Equip[0].Index == 220 || Mob[LOCAL_10].Mobs.Player.Equip[0].Index == 219)
	{/*
		INT32 LOCAL_13 = *(DWORD*)(0x008BF1858);
		if(LOCAL_13 > 0 && LOCAL_13 < MAX_PLAYER && Users[LOCAL_13].Status == 22)
		{
			memcpy(&Mob[LOCAL_10].Mobs.Player.Equip[14], &Mob[LOCAL_13].Mobs.Player.Equip[14], 8);

			if(Mob[LOCAL_10].Mobs.Player.Equip[0].Index == 219)
			{
				memcpy(&Mob[LOCAL_10].Mobs.Player.Equip[14], &Mob[LOCAL_13].Mobs.Player.Equip[12], 8);

/*				if(Mob[LOCAL_10].Mobs.MedalId == 509)
					Mob[LOCAL_10].Mobs.MedalId = 508;

			}
		}*/
	}

	//
	Mob[LOCAL_10].GenerateID = arg1;
	Mob[LOCAL_10].Formation = mGener.pList[LOCAL_1].Formation;
	Mob[LOCAL_10].RouteType = mGener.pList[LOCAL_1].RouteType;
	Mob[LOCAL_10].Mode = 4;
	Mob[LOCAL_10].Segment.Progress = 0;
	Mob[LOCAL_10].Leader = 0;
	Mob[LOCAL_10].WaitSec = Mob[LOCAL_10].Segment.Wait[0];
	Mob[LOCAL_10].Last.Time = CurrentTime;


	Mob[LOCAL_10].GetCurrentScore(MAX_PLAYER);
	Mob[LOCAL_10].clientId = LOCAL_10;
	Mob[LOCAL_10].Mobs.Player.Status.curHP = Mob[LOCAL_10].Mobs.Player.Status.maxHP;

	if (sServer.NewbieEventServer != 0 && Mob[LOCAL_10].Mobs.Player.Status.Level < 150) // NewbieEventServer
		Mob[LOCAL_10].Mobs.Player.Status.curHP = (Mob[LOCAL_10].Mobs.Player.Status.curHP >> 2) * 3;

	Mob[LOCAL_10].Segment.Direction = 0;
	memset(&Mob[LOCAL_10].EnemyList, 0, MAX_ENEMY * sizeof(UINT16));

	if (Mob[LOCAL_10].Mobs.Player.CapeInfo == 1 && (Rand() % 10) == 1)
		Mob[LOCAL_10].Mobs.Player.CapeInfo = 2;

	UINT32 LOCAL_14 = Mob[LOCAL_10].Segment.ListX[0];
	UINT32 LOCAL_15 = Mob[LOCAL_10].Segment.ListY[0];

	INT32 LOCAL_16 = GetEmptyMobGrid(LOCAL_10, &LOCAL_14, &LOCAL_15);
	if (LOCAL_16 == 0)
	{
		// error
		Mob[LOCAL_10].Mode = 0;
		Mob[LOCAL_10].Mobs.Player.Name[0] = '\0';
		Mob[LOCAL_10].GenerateID = 0;

		return;
	}

	if (arg1 == GUARDIAN_TOWER_BLUE || arg1 == GUARDIAN_TOWER_RED)
	{
		int index = arg1 - GUARDIAN_TOWER_BLUE;

		int kingIndex = sServer.KingdomBattle.Info[index].KingId;
		if (kingIndex != 0 && Mob[kingIndex].GenerateID == (8 + index))
		{
			if (Mob[kingIndex].Mode == 5)
			{
				Mob[LOCAL_10].Mode = 0;
				Mob[LOCAL_10].Mobs.Player.Name[0] = '\0';
				Mob[LOCAL_10].GenerateID = 0;

				// O rei está sendo atacado, não queremos que a torre nasça
				return;
			}
		}

		sServer.KingdomBattle.Info[index].TowerId = LOCAL_10;
		sServer.KingdomBattle.Info[index].isTowerAlive = true;
		sServer.KingdomBattle.Info[index].Status = false;

		SendNotice("A Torre Guardiã do reino %s nasceu", arg1 == GUARDIAN_TOWER_BLUE ? "blue" : "red");
	}
	else if (arg1 == 8 || arg1 == 9)
	{
		int index = arg1 - 8;

		sServer.KingdomBattle.Info[index].KingId = LOCAL_10;
		sServer.KingdomBattle.Info[index].isKingAlive = true;

		int towerIndex = sServer.KingdomBattle.Info[index].TowerId;
		if (Mob[towerIndex].GenerateID != (GUARDIAN_TOWER_BLUE + index))
		{
			GenerateMob(GUARDIAN_TOWER_BLUE + index, 0, 0);

			sServer.KingdomBattle.Info[index].Status = false;
		}
	}

	Mob[LOCAL_10].Segment.X = LOCAL_14;
	Mob[LOCAL_10].Target.X = LOCAL_14;
	Mob[LOCAL_10].Last.X = LOCAL_14;

	Mob[LOCAL_10].Segment.Y = LOCAL_15;
	Mob[LOCAL_10].Target.Y = LOCAL_15;
	Mob[LOCAL_10].Last.Y = LOCAL_15;
	/*
	if (arg1 == BALMUS_ID || arg1 == KARA_ID || arg1 == COMB_JUDITH_ID ||
		arg1 == EMPIS_ID || arg1 == BRUCE_ID)
	{
		int cityId = GetVillage(LOCAL_14, LOCAL_15);

		if (cityId != 5)
		{
			if (g_pCityZone[cityId].owner_index != 0)
			{
				Mob[LOCAL_10].Mobs.Player.GuildIndex = g_pCityZone[cityId].owner_index;
				Mob[LOCAL_10].Mobs.Player.GuildMemberType = 1;
			}
		}
	}
	*/

	if (mGener.pList[LOCAL_1].MobCount < 0)
		mGener.pList[LOCAL_1].MobCount = 0;

	mGener.pList[LOCAL_1].MobCount++;

	INT32 LOCAL_17 = mGener.pList[LOCAL_1].Leader.bStatus.maxMP;
	if (LOCAL_17 != 0)
	{
		SetAffect(LOCAL_10, LOCAL_17, 30000, 200);
		SetTick(LOCAL_10, LOCAL_17, 30000, 200);
	}

	//p364 LOCAL_44;
	//GetCreateMob(LOCAL_10, (BYTE*)&LOCAL_44);

	// não entendi - 004480BB -> Ele muda o spawn para 2 para aparecer nascendo

	g_pMobGrid[LOCAL_15][LOCAL_14] = LOCAL_10;

	Mob[LOCAL_10].SpawnType = 2;

	SendGridMob(LOCAL_10);

	//GridMulticast_2(LOCAL_14, LOCAL_15, (BYTE*)&LOCAL_44, 0);
	// 004480ED

	Mob[LOCAL_10].SpawnType = 0;

	//LOCAL_44.Spawn.Type = 0;
	for (INT32 LOCAL_12 = 0; LOCAL_12 < LOCAL_8; LOCAL_12++)
	{
		INT32 LOCAL_45 = mGener.GetEmptyNPCMob();
		if (LOCAL_45 <= 0)
		{
			// error - 0044811F
			continue;
		}

		Mob[LOCAL_45] = CMob{};
		Mob[LOCAL_45].BossInfoId = MAXUINT32;
		Mob[LOCAL_45].clientId = LOCAL_45;

		memset(&Mob[LOCAL_45].PartyList, 0, 24);

		Mob[LOCAL_11].PartyList[LOCAL_12] = LOCAL_45;
		memcpy(&Mob[LOCAL_45].Mobs.Player, &mGener.pList[LOCAL_1].Follower, sizeof st_Mob);

		Mob[LOCAL_45].Mobs.Player.bStatus.Merchant.Direction = 0;

		strncpy_s(Mob[LOCAL_45].Mobs.Player.Name, mGener.pList[LOCAL_1].Follower.Name, 16);

		for (INT32 LOCAL_46 = 0; LOCAL_46 < 16; LOCAL_46++)
		{
			if (Mob[LOCAL_45].Mobs.Player.Name[LOCAL_46] == 0x5F)
				Mob[LOCAL_45].Mobs.Player.Name[LOCAL_46] = 0x20;
		}

		memset(Mob[LOCAL_45].Mobs.Affects, 0, sizeof st_Affect * 32);

		for (INT32 LOCAL_47 = 0; LOCAL_47 < 5; LOCAL_47++)
		{
			if (mGener.pList[arg1].Segment_X[LOCAL_47] == 0)
			{
				Mob[LOCAL_45].Segment.ListX[LOCAL_47] = 0;
				Mob[LOCAL_45].Segment.ListY[LOCAL_47] = 0;

				continue;
			}

			Mob[LOCAL_45].Segment.ListX[LOCAL_47] = Mob[LOCAL_11].Segment.ListX[LOCAL_47] + g_pFormation[LOCAL_47][LOCAL_12][0];
			Mob[LOCAL_45].Segment.ListY[LOCAL_47] = Mob[LOCAL_11].Segment.ListY[LOCAL_47] + g_pFormation[LOCAL_47][LOCAL_12][1];
			Mob[LOCAL_45].Segment.Wait[LOCAL_47] = mGener.pList[LOCAL_1].SegmentWait[LOCAL_47];
		}

		if (arg2 != 0 && arg3 != 0)
		{
			for (INT32 LOCAL_12 = 0; LOCAL_12 < 5; LOCAL_12++)
			{
				Mob[LOCAL_45].Segment.ListX[LOCAL_12] = arg2 + (Rand() % 5) - 2;
				Mob[LOCAL_45].Segment.ListY[LOCAL_12] = arg3 + (Rand() % 5) - 2;
			}
		}

		Mob[LOCAL_45].clientId = LOCAL_45;
		Mob[LOCAL_45].GenerateID = arg1;
		Mob[LOCAL_45].Formation = mGener.pList[LOCAL_1].Formation;
		Mob[LOCAL_45].RouteType = mGener.pList[LOCAL_1].RouteType;
		Mob[LOCAL_45].Mode = 4;
		Mob[LOCAL_45].Segment.Progress = 0;

		Mob[LOCAL_45].Leader = LOCAL_11;
		Mob[LOCAL_10].Last.Time = CurrentTime;

		Mob[LOCAL_45].GetCurrentScore(MAX_PLAYER);
		Mob[LOCAL_45].clientId = LOCAL_45;
		Mob[LOCAL_45].Mobs.Player.Status.curHP = Mob[LOCAL_45].Mobs.Player.Status.maxHP;

		if (sServer.NewbieEventServer && Mob[LOCAL_45].Mobs.Player.Status.Level < 150) // NewbieEventServer
			Mob[LOCAL_45].Mobs.Player.Status.curHP = (Mob[LOCAL_45].Mobs.Player.Status.curHP >> 2) * 3;

		Mob[LOCAL_45].WaitSec = Mob[LOCAL_45].Segment.Wait[0];
		Mob[LOCAL_45].Segment.Direction = 0;

		memset(&Mob[LOCAL_45].EnemyList, 0, MAX_ENEMY * 2);

		if (Mob[LOCAL_45].Mobs.Player.CapeInfo == 1 && Rand() % 10 == 1)
			Mob[LOCAL_10].Mobs.Player.CapeInfo = 2;

		UINT32 LOCAL_48 = Mob[LOCAL_45].Segment.ListX[0];
		UINT32 LOCAL_49 = Mob[LOCAL_45].Segment.ListY[0];

		INT32 LOCAL_50 = GetEmptyMobGrid(LOCAL_45, &LOCAL_48, &LOCAL_49);
		if (LOCAL_50 == 0)
			LOCAL_50 = GetEmptyMobGrid(LOCAL_45, &LOCAL_48, &LOCAL_49);

		if (LOCAL_50 == 0)
		{
			Mob[LOCAL_45].Mode = 0;
			Mob[LOCAL_45].GenerateID = -1;
			Mob[LOCAL_45].Mobs.Player.Name[0] = '\0';
			Mob[LOCAL_45].PartyList[LOCAL_12] = 0;

			//error
			// 004486F1
			continue;
		}

		Mob[LOCAL_45].Segment.X = LOCAL_48;
		Mob[LOCAL_45].Target.X = LOCAL_48;
		Mob[LOCAL_45].Last.X = LOCAL_48;

		Mob[LOCAL_45].Segment.Y = LOCAL_49;
		Mob[LOCAL_45].Target.Y = LOCAL_49;
		Mob[LOCAL_45].Last.Y = LOCAL_49;

		INT32 LOCAL_17 = mGener.pList[LOCAL_1].Follower.bStatus.maxMP;
		if (LOCAL_17 != 0)
		{
			SetAffect(LOCAL_10, LOCAL_17, 30000, 200);
			SetTick(LOCAL_10, LOCAL_17, 30000, 200);
		}

		//p364 LOCAL_78;
		//GetCreateMob(LOCAL_45, (BYTE*)&LOCAL_78);

		Mob[LOCAL_45].SpawnType = 2;
		SendGridMob(LOCAL_45);

		Mob[LOCAL_45].SpawnType = 0;
		// n entendi - 00448839
		g_pMobGrid[LOCAL_49][LOCAL_48] = LOCAL_45;

		//	GridMulticast_2(LOCAL_48, LOCAL_49, (BYTE*)&LOCAL_78, 0);

		if (mGener.pList[LOCAL_1].MobCount < 0)
			mGener.pList[LOCAL_1].MobCount = 0;

		mGener.pList[LOCAL_1].MobCount++;
	}
}

void CMob::GetRandomPos()
{
	if (!Mobs.Player.bStatus.Move.Speed)
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		return;
	}

	INT32 LOCAL_2 = GetSpeed(&Mobs.Player.Status);
	INT32 LOCAL_3 = (LOCAL_2 * 8) / 4;

	if (LOCAL_3 >= 24)
		LOCAL_3 = 23;

	Last.X = Target.X;
	Last.Y = Target.Y;

	Next.X = Last.X + ((Rand() % 7) - 3);
	Next.Y = Last.Y + ((Rand() % 7) - 3);

	GetEmptyMobGrid(0, &Next.X, &Next.Y);

	INT32 LOCAL_4;
	for (LOCAL_4 = LOCAL_3; LOCAL_3 >= 0; LOCAL_3--)
	{
		GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_4);//, (char*)g_pHeightGrid);

		if (g_pMobGrid[Next.Y][Next.X] == 0)
			break;

		if (LOCAL_4 != LOCAL_3)
		{
			GetEmptyMobGrid(0, &Next.X, &Next.Y);

			GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_4); //, (char*)g_pHeightGrid);
			if (g_pMobGrid[Next.Y][Next.X] == 0)
				break;
		}
	}

	if (LOCAL_4 == -1 || !Route[0])
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		Route[0] = 0;
	}
}

void CMob::AddEnemyList(int enemyId)
{
	if (enemyId <= 0)
		return;

	if (enemyId < MAX_PLAYER && Mob[enemyId].Mobs.AffectInfo & 0x40)
		return;

	if (enemyId < MAX_PLAYER && Mob[enemyId].Mobs.Player.Info.Merchant)
		return;

	INT32 LOCAL_2 = 0;
	for (; LOCAL_2 < MAX_ENEMY; LOCAL_2++)
	{
		if (this->EnemyList[LOCAL_2] == enemyId)
			return;
	}

	for (LOCAL_2 = 0; LOCAL_2 < MAX_ENEMY; LOCAL_2++)
	{
		if (!this->EnemyList[LOCAL_2])
			break;
	}

	if (LOCAL_2 == 4)
		return;

	this->EnemyList[LOCAL_2] = enemyId;

	if (GenerateID == KEFRA)
	{
		for (LOCAL_2 = 1; LOCAL_2 < MAX_PLAYER; LOCAL_2++)
		{
			if (Users[LOCAL_2].Status != USER_PLAY || Mob[LOCAL_2].Mode == USER_EMPTY)
				continue;

			if (Mob[LOCAL_2].Target.X < (Target.X - 30) || Mob[LOCAL_2].Target.X >(Target.X + 30) || Mob[LOCAL_2].Target.Y < (Target.Y - 30) || Mob[LOCAL_2].Target.Y >(Target.Y + 30))
				continue;

			INT32 t = 0;
			for (; t < MAX_ENEMY; t++)
				if (EnemyList[t] == enemyId)
					return;

			for (t = 0; t < MAX_ENEMY; t++)
				if (EnemyList[t] == 0)
					break;

			if (t == MAX_ENEMY)
				break;

			EnemyList[t] = enemyId;
		}
	}
}

void CMob::RemoveEnemyList(int enemyId)
{
	if (!enemyId)
		return;

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (EnemyList[i] == enemyId)
		{
			EnemyList[i] = 0;
			break;
		}
	}
}

INT32 CMob::StandingByProcessor()
{
	INT32 LOCAL_2 = 0;

	if (RouteType == 5)
	{
		if (Mobs.Player.Equip[0].Index == 358)
		{
			if (Mobs.Affects[0].Index != 24)
				return 256;

			return 0;
		}

		if (Leader <= 0 || Leader >= MAX_PLAYER)
		{
			LOCAL_2 |= 256;

			return LOCAL_2;
		}

		INT32 LOCAL_3 = Mobs.Player.Equip[0].Index;
		if (Mobs.Affects[0].Index != 24 && (LOCAL_3 < 315 || LOCAL_3 >= 345))
		{
			LOCAL_2 |= 256;

			return LOCAL_2;
		}

		INT32 LOCAL_4 = Summoner;
		if (LOCAL_4 <= 0 && LOCAL_4 >= MAX_PLAYER)
		{
			LOCAL_2 |= 256;
			return LOCAL_2;
		}

		INT32 LOCAL_5 = 0;
		if (Leader == LOCAL_4)
			LOCAL_5 = 1;

		for (INT32 LOCAL_6 = 0; LOCAL_6 < 12; LOCAL_6++)
		{
			if (Mob[Leader].PartyList[LOCAL_6] == LOCAL_4)
				LOCAL_5 = 1;
		}

		if (LOCAL_5 == 0)
		{
			LOCAL_2 |= 256;
			return LOCAL_2;
		}

		if (Users[LOCAL_4].Status == USER_PLAY)
		{
			INT32 LOCAL_7 = GetDistance(Target.X, Target.Y, Mob[LOCAL_4].Target.X, Mob[LOCAL_4].Target.Y);

			if (LOCAL_7 >= 20)
			{
				Next.X = Mob[LOCAL_4].Target.X;
				Next.Y = Mob[LOCAL_4].Target.Y;

				LOCAL_2 |= (char)2;
				return LOCAL_2;
			}
			else if (LOCAL_7 > 2 && LOCAL_7 < 20)
			{
				Segment.X = Mob[LOCAL_4].Target.X;
				Segment.Y = Mob[LOCAL_4].Target.Y;

				GetNextPos(0);
				LOCAL_2 |= (char)1;
				return LOCAL_2;
			}
			
			return LOCAL_2;
		}

		LOCAL_2 |= 256;
		return LOCAL_2;
	}

	if (Leader == 0)
	{
		INT32 LOCAL_8 = GetEnemyFromView();

		if (LOCAL_8 != 0 && Target.X < Segment.X + 12 && Target.X > Segment.X - 12 && Target.Y < Segment.Y + 12 && Target.Y > Segment.Y - 12)
			return LOCAL_8 | 0x10000000;
	}

	if (RouteType == 6 && Target.X == Segment.X && Target.Y == Segment.Y)
		return 0;

	if (Segment.X == Target.X && Segment.Y == Target.Y)
	{
		if (WaitSec > 0 && WaitSec != 6)
		{
			WaitSec -= 6;

			if (WaitSec > 0)
			{
				if (RouteType == 0 && Target.X == Segment.ListX[0] && Target.Y == Segment.ListY[0])
					return 0;

				if (Mobs.Player.Status.Move.Speed == 0)
					return 0;

				LOCAL_2 = LOCAL_2 | 4096;
				return LOCAL_2;
			}

			WaitSec = 0;
		}
		else
		{
			int LOCAL_9 = Segment.Wait[Segment.Progress];
			if (LOCAL_9 > 0)
			{
				WaitSec = LOCAL_9;

				return LOCAL_2;
			}
		}

		INT32 LOCAL_10 = SetSegment();
		if (LOCAL_10 == 1)
			return LOCAL_2;

		if (LOCAL_10 == 2)
		{
			LOCAL_2 = 256;
			return LOCAL_2;
		}
	}

	// 004112D1
	GetNextPos(0);

	if (Next.X == Target.X && Next.Y == Target.Y)
	{
		INT32 LOCAL_11 = SetSegment();
		return LOCAL_2;
	}

	LOCAL_2 = LOCAL_2 ^ (char)0x01;
	return LOCAL_2;
}

void CMob::GetNextPos(int battle)
{
	if (!Mobs.Player.bStatus.Move.Speed)
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		return;
	}

	if (Mobs.Player.Equip[0].Index == 219 || Mobs.Player.Equip[0].Index == 220 || GenerateID == KEFRA)
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		return;
	}

	int speed = GetSpeed(&Mobs.Player.Status); //LOCAL_2
	int LOCAL_3 = speed * 8 / 4;
	if (LOCAL_3 >= 24)
		LOCAL_3 = 23;

	int LOCAL_4;
	if (battle)
	{
		LOCAL_4 = (Mobs.Player.bStatus.STR + 1) >> 1;

		if (LOCAL_3 > LOCAL_4)
			LOCAL_3 = LOCAL_4;
	}
	else
	{
		if (LOCAL_3 > Mobs.Player.bStatus.STR)
			LOCAL_3 = Mobs.Player.bStatus.STR;
	}

	Last.X = Target.X;
	Last.Y = Target.Y;

	Next.X = Segment.X;
	Next.Y = Segment.Y;

	int LOCAL_5 = LOCAL_3;
	for (; LOCAL_5 >= 0; LOCAL_5--)
	{// 413949 
		GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_5);//, (char*)g_pHeightGrid); //

		if (!g_pMobGrid[Next.Y][Next.X])
			break;

		if (LOCAL_5 != LOCAL_3)
		{
			GetEmptyMobGrid(0, &Next.X, &Next.Y);

			GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_5); //, (char*)g_pHeightGrid); //(char*)g_pHeightGrid);
			if (!g_pMobGrid[Next.Y][Next.X])
				break;
		}
	}

	if (LOCAL_5 < 0 || !Route[0])
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		Route[0] = 0;
		//INT32 LOCAL_6 = 0;
		return;
	}

	//if(g_pMobGrid[Next.Y][Next.X])
	//	int LOCAL_7 = 0;
}

INT32 CMob::GetEnemyFromView()
{//0x0040151E
	int LOCAL_2 = 0x9, //MOV [LOCAL.2],9
		LOCAL_3 = 0x9, //MOV [LOCAL.3],9
		LOCAL_4 = this->Target.X - 4,
		LOCAL_5 = this->Target.Y - 4;

	if (Mobs.Player.CapeInfo == 7 || Mobs.Player.CapeInfo == 8)
	{
		LOCAL_3 = 16;
		LOCAL_2 = 16;

		LOCAL_4 = Target.X - 6;
		LOCAL_5 = Target.Y - 6;
	}

	int LOCAL_6 = LOCAL_4,
		LOCAL_7 = LOCAL_5,
		LOCAL_8 = LOCAL_4 + LOCAL_3,
		LOCAL_9 = LOCAL_5 + LOCAL_2,
		LOCAL_10 = LOCAL_7;

	for (; LOCAL_10 < LOCAL_9; LOCAL_10++)
	{
		int LOCAL_11 = LOCAL_6;

		for (; LOCAL_11 < LOCAL_8; LOCAL_11++)
		{
			if (LOCAL_11 == Target.X || LOCAL_10 == Target.Y)
				continue;

			if (LOCAL_11 < 0 || LOCAL_10 < 0 || LOCAL_11 > 4096 || LOCAL_10 > 4096)
				continue;

			int LOCAL_12 = g_pMobGrid[LOCAL_10][LOCAL_11];
			if (LOCAL_12 == 0)
				continue;

			if (!Mob[LOCAL_12].Mobs.Player.Status.curHP)
				continue;

			if (Mob[LOCAL_12].Mode == 0)
				continue;

			if (LOCAL_12 < MAX_PLAYER && ((Mob[LOCAL_12].Mobs.AffectInfo) & 0x40 || (Mob[clientId].Mobs.Player.Info.Merchant & 1)))
				continue;

			INT32 LOCAL_13 = Mob[LOCAL_12].Mobs.Player.CapeInfo;
			if (Mobs.Player.CapeInfo < 0 || Mobs.Player.CapeInfo >= 9 || LOCAL_13 < 0 || LOCAL_13 >= 9)
			{
				Log(SERVER_SIDE, LOG_ERROR, "clan out or range %d %d [%d] [%s] [%s]", Mobs.Player.CapeInfo, LOCAL_13, Mob[LOCAL_12].GenerateID, Mob[LOCAL_12].Mobs.Player.Name, Mobs.Player.Name);

				return 0;
			}

			if (g_pClanTable[Mobs.Player.CapeInfo][LOCAL_13] <= 0)
				return LOCAL_12;
		}
	}

	return 0;
}

INT32 CMob::SetSegment()
{
	if (RouteType == 6) // 320h - 800
	{
		Segment.Progress = 0; // 3BC
		Segment.Direction = 0; // 3B8

		Segment.X = Segment.ListX[Segment.Progress];
		Segment.Y = Segment.ListY[Segment.Progress];

		WaitSec = 0;

		return 0;
	}

	if (RouteType < 0 || RouteType > 4)
	{
		Log(SERVER_SIDE, LOG_ERROR, "Wront SetSegment - RouteTpe %d - GenerID %d", RouteType, GenerateID);

		return 0;
	}

	INT32 LOCAL_2 = 0;
	for (INT32 i = 0; i < 1000; i++)
	{
		if (Segment.Direction == 0)
			Segment.Progress++;
		else
			Segment.Progress--;

		if (Segment.Progress <= -1)
		{
			if (RouteType == 0)
			{
				Segment.Progress = 0; // 3BC
				Segment.Direction = 0; // 3B8

				LOCAL_2 = 2;
				break;
			}
			else if (RouteType == 1)
			{
				Segment.Progress = 0; // 3BC
				Segment.Direction = 0; // 3B8

				break;
			}
			else if (RouteType == 2)
			{
				Segment.Progress = 0; // 3BC
				Segment.Direction = 0; // 3B8

				break;
			}
			else if (RouteType == 3)
			{
				LOCAL_2 = 2;
				break;
			}
			else if (RouteType == 4)
			{
				Log(SERVER_SIDE, LOG_ERROR, "SetSegment SegmentProgress -1 but route type 4 - %s %d", Mobs.Player.Name, 0);

				break;
			}
		}
		else if (Segment.Progress >= 5)
		{
			if (RouteType == 0)
			{
				Segment.Progress = 4;
				Mode = 4;

				Mobs.Player.bStatus.Merchant.Merchant = Mobs.Player.Info.Merchant;
				INT32 LOCAL_3 = strlen((char*)Route);

				char LOCAL_4 = 0; //EBP-10;
				if (LOCAL_3 > 0)
				{
					LOCAL_4 = Route[(LOCAL_3 - 1)]; // No caso, ele usa o final da Next, não sei como ele faz isso, mas enfim!
					LOCAL_4 = LOCAL_4 - 48;

					if (LOCAL_4 >= 1 && LOCAL_4 <= 9)
					{
						LOCAL_4 = LOCAL_4 << 4;

						Mobs.Player.bStatus.Merchant.Direction = Mobs.Player.bStatus.Merchant.Direction | LOCAL_4;
					}

				}

				GetCurrentScore(MAX_PLAYER); // Na verdade, PUSH 064h, MOV ECX
				LOCAL_2 = 1;

				break;
			}
			else if (RouteType == 1)
			{
				LOCAL_2 = 2;
				break;
			}
			else if (RouteType == 2 || RouteType == 3)
			{
				Segment.Progress = 4; // 3BC
				Segment.Direction = 1; // 3B8

				break;
			}
			else if (RouteType == 4)
			{
				Segment.Progress = -1;
				break;
			}
		}
		else
		{
			if (Segment.ListX[Segment.Progress] == 0)
				continue;

			LOCAL_2 = 0;
			break;
		}
	}

	Segment.X = Segment.ListX[Segment.Progress];
	Segment.Y = Segment.ListY[Segment.Progress];

	WaitSec = 0;

	return LOCAL_2;
}

INT32 CMob::BattleProcessor()
{
	if (Mobs.Player.Equip[0].Index == 362)
		return 0x2000;

	SelectTargetFromEnemyList();

	if (CurrentTarget == 0)
	{ // Não encontrou ninguém para atacar
		Mode = 4;

		if (BossInfoId < sServer.Boss.size())
			sServer.Boss[BossInfoId].LastUpdate = std::chrono::steady_clock::now();

		return 0;
	}

	if (RouteType == 5)
	{
		if (Mobs.Player.Equip[0].Index == 358)
		{
			if (Mobs.Affects[0].Index != 24)
				return 32;

			return 0;
		}

		if (Leader <= 0 || Leader >= MAX_PLAYER)
			return 32;

		INT32 LOCAL_2 = Summoner;
		if (LOCAL_2 <= 0 || LOCAL_2 >= MAX_PLAYER)
			return 256;

		INT32 LOCAL_3 = 0;
		if (Leader == LOCAL_2)
			LOCAL_3 = 1;

		INT32 LOCAL_4 = 0;
		for (; LOCAL_4 < 12; LOCAL_4++)
		{
			if (Mob[Leader].PartyList[LOCAL_4] == LOCAL_2)
				LOCAL_3 = 1;
		}

		if (LOCAL_3 == 0)
			return 256;

		INT32 LOCAL_5 = GetDistance(Target.X, Target.Y, Mob[LOCAL_2].Target.X, Mob[LOCAL_2].Target.Y);

		if (LOCAL_5 >= 20)
		{
			Next.X = Mob[LOCAL_2].Target.X;
			Next.Y = Mob[LOCAL_2].Target.Y;

			return 2;
		}
	}

	INT32 LOCAL_6 = Mobs.Player.bStatus.INT;
	if (LOCAL_6 < Rand() % 100)
		return 0x10000;

	INT32 LOCAL_7 = Mobs.Player.bStatus.DEX;
	INT32 LOCAL_8 = Mob[CurrentTarget].Target.X;
	INT32 LOCAL_9 = Mob[CurrentTarget].Target.Y;
	INT32 LOCAL_10 = GenerateID == KEFRA ? 25 : GetMobAbility(&Mobs.Player, 27);
	INT32 LOCAL_11 = GetDistance(Target.X, Target.Y, LOCAL_8, LOCAL_9);

	if (RouteType != 5)
	{
		if (Target.X > Segment.X + 30 || Target.X < Segment.X - 30 || Target.Y > Segment.Y + 30 || Target.Y < Segment.Y - 30)
		{
			CurrentTarget = 0;
			for (INT32 LOCAL_12 = 0; LOCAL_12 < MAX_ENEMY; LOCAL_12++)
				EnemyList[LOCAL_12] = 0;

			Mode = 4;

			GetNextPos(1);
			if (Next.X == Target.X && Next.Y == Target.Y)
				return 0;

			return 0x10;
		}
	}

	if (LOCAL_11 <= LOCAL_10)
	{
		INT32 LOCAL_13 = Rand() % 100;

		if (LOCAL_10 >= 4 && LOCAL_11 <= 2 && LOCAL_13 > LOCAL_7)
			return 0x100;

		INT32 LOCAL_14 = LOCAL_8;
		INT32 LOCAL_15 = LOCAL_9;

		// 004117B5
		GetHitPosition(Target.X, Target.Y, &LOCAL_8, &LOCAL_9);

		if (LOCAL_8 != LOCAL_14 || LOCAL_9 != LOCAL_15)
			return 0x100;

		return 0x1000;
	}

	return 0x01;
}

void CMob::SelectTargetFromEnemyList()
{// 00411AD0
	CurrentTarget = 0;

	INT32 LOCAL_2[MAX_ENEMY];
	INT32 LOCAL_6 = 0;
	for (; LOCAL_6 < MAX_ENEMY; LOCAL_6++)
		LOCAL_2[LOCAL_6] = MAX_PLAYER;

	INT32 LOCAL_7 = 6;

	if (Mobs.Player.CapeInfo == 4 || Mobs.Player.CapeInfo == 7 || Mobs.Player.CapeInfo == 8)
		LOCAL_7 = 12;

	if (Mobs.Player.Equip[0].Index == 362) // kefra face
		LOCAL_7 = 12;

	INT32 LOCAL_12;
	if ((Target.X >> 7) < 12 && (Target.Y << 7) > 25)
		LOCAL_12 = 1;
	else
		LOCAL_12 = 0;

	if (LOCAL_12 != 0)
		LOCAL_7 = 8;

	for (LOCAL_6 = 0; LOCAL_6 < MAX_ENEMY; LOCAL_6++)
	{
		INT16 LOCAL_8 = EnemyList[LOCAL_6]; // EBP - 20h

		if (LOCAL_8 <= 0 || LOCAL_8 >= MAX_SPAWN_MOB)
			continue;

		if (Mob[LOCAL_8].Mode == 0)
		{
			EnemyList[LOCAL_6] = 0;

			continue;
		}
		// 00411BF7
		if (!Mob[LOCAL_8].Mobs.Player.Status.curHP)
		{
			EnemyList[LOCAL_6] = 0;

			continue;
		}

		auto event = static_cast<TOD_BossEvent*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::BossEvent));
		if (strcmp(Mob[LOCAL_8].Mobs.Player.Name, event->GetBossName().c_str()) == 0)
		{
			EnemyList[LOCAL_6] = 0;

			continue;
		}

		if (LOCAL_8 < MAX_PLAYER)
		{
			if (Mob[LOCAL_8].Mobs.AffectInfo & 0x40)
			{
				EnemyList[LOCAL_6] = 0;

				continue;
			}

			if (Mob[LOCAL_8].Mobs.Player.Status.Level >= 400 && Mob[LOCAL_8].Mobs.Player.AffectInfo.SlowMov)
			{
				EnemyList[LOCAL_6] = 0;

				continue;
			}
		}

		if (Mob[LOCAL_8].Target.X < Target.X - LOCAL_7 || Mob[LOCAL_8].Target.X > Target.X + LOCAL_7 ||
			Mob[LOCAL_8].Target.Y < Target.Y - LOCAL_7 || Mob[LOCAL_8].Target.Y > Target.Y + LOCAL_7)
		{
			EnemyList[LOCAL_6] = 0;

			continue;
		}

		LOCAL_2[LOCAL_6] = GetDistance(Target.X, Target.Y, Mob[LOCAL_8].Target.X, Mob[LOCAL_8].Target.Y);

		if (LOCAL_8 >= MAX_PLAYER)
			LOCAL_2[LOCAL_6] += 2;
	}

	INT32 LOCAL_9 = MAX_PLAYER;
	INT32 LOCAL_10 = 0;

	for (LOCAL_6 = 0; LOCAL_6 < 4; LOCAL_6++)
	{
		INT16 LOCAL_11 = EnemyList[LOCAL_6];
		if (!LOCAL_11)
			continue;

		if (LOCAL_9 > LOCAL_2[LOCAL_6])
		{
			LOCAL_10 = LOCAL_11;

			LOCAL_9 = LOCAL_2[LOCAL_6];
		}
	}

	if (LOCAL_9 == MAX_PLAYER)
	{
		CurrentTarget = 0;

		return;
	}

	CurrentTarget = LOCAL_10;
}

void CMob::GetTargetPos(int arg1)
{
	if (!Mobs.Player.bStatus.Move.Speed)
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		return;
	}

	INT32 LOCAL_2 = GetSpeed(&Mobs.Player.Status);
	INT32 LOCAL_3 = LOCAL_2 * 8 / 4;

	if (LOCAL_3 >= 24)
		LOCAL_3 = 23;

	Last.X = Target.X;
	Last.Y = Target.Y;

	Next.X = Mob[arg1].Target.X;
	Next.Y = Mob[arg1].Target.Y;

	GetEmptyMobGrid(0, &Next.X, &Next.Y);

	INT32 LOCAL_4 = LOCAL_3;
	for (; LOCAL_4 >= 0; LOCAL_4--)
	{
		GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_4); //, (char*)g_pHeightGrid);
		if (!g_pMobGrid[Next.Y][Next.X])
			break;

		if (LOCAL_4 != LOCAL_3)
		{
			GetEmptyMobGrid(0, &Next.X, &Next.Y);

			GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_4); //, (char*)g_pHeightGrid);
			if (!g_pMobGrid[Next.Y][Next.X])
				break;
		}
	}

	if (LOCAL_4 == -1 || !Route[0])
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		Route[0] = 0;
	}
}

void CMob::GetTargetPosDistance(int arg1)
{
	if (!Mobs.Player.bStatus.Move.Speed)
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		return;
	}

	INT32 LOCAL_2 = GetSpeed(&Mobs.Player.Status);
	INT32 LOCAL_3 = LOCAL_2 << 2 << 1 >> 2;

	if (LOCAL_3 >= 24)
		LOCAL_3 = 23;

	Last.X = Target.X;
	Last.Y = Target.Y;

	Next.X = Mob[arg1].Target.X;
	Next.Y = Mob[arg1].Target.Y;

	if (Mob[arg1].Last.X > Next.X)
		Next.X = Next.X - (1 + Rand() % 2);
	else if (Mob[arg1].Last.X < Next.X)
		Next.X = Next.X + (Rand() % 2 + 1);

	if (Mob[arg1].Last.Y > Next.Y)
		Next.Y = Next.Y - (1 + Rand() % 2);
	else if (Mob[arg1].Last.Y < Next.Y)
		Next.Y = Next.Y + (Rand() % 2 + 1);

	GetEmptyMobGrid(0, &Next.X, &Next.Y);

	INT32 LOCAL_4 = LOCAL_3;
	for (; LOCAL_4 >= 0; LOCAL_4--)
	{
		GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_4); //, (char*)g_pHeightGrid);

		if (!g_pMobGrid[Next.Y][Next.X])
			break;

		if (LOCAL_4 != LOCAL_3)
		{
			GetEmptyMobGrid(0, &Next.X, &Next.Y);
			GetRoute(Last.X, Last.Y, &Next.X, &Next.Y, (char*)Route, LOCAL_4); //, (char*)g_pHeightGrid);

			if (!g_pMobGrid[Next.Y][Next.X])
				break;
		}
	}

	if (LOCAL_4 == 1 || !Route[0])
	{
		Next.X = Target.X;
		Next.Y = Target.Y;

		Route[0] = 0;
	}
}

bool CMob::IsInsideValley() const
{
	return (Target.X >= 2177 && Target.X <= 2304 && Target.Y >= 3584 && Target.Y <= 3839);
}

INT32 CMob::CheckGetLevel()
{
	static const unsigned char szAdds[4][2] =
	{
		{30, 40},
		{35, 50},
		{45, 60},
		{50, 70}
	};

	INT32 evId = Mobs.Player.Equip[0].EFV2;
	INT32 level = Mobs.Player.Status.Level;
	if ((evId <= ARCH && level >= MAX_LEVEL) || (evId >= CELESTIAL && level >= MAX_LEVEL_CELESTIAL))
		return false;

	INT64 exp = Mobs.Player.Exp;
	INT64 nextExp = g_pNextLevel[evId][level];

	if (evId == ARCH && Mobs.Info.LvBlocked)
	{
		Mobs.Info.LvBlocked = 0;
		Log(clientId, LOG_INGAME, "O arch estava bloqueado. Efetuando desbloqueio");
	}

	if (evId == CELESTIAL)
	{
		if (Mobs.Info.LvBlocked && level != 39 && level != 89)
		{
			Mobs.Info.LvBlocked = 0;
			Log(clientId, LOG_INGAME, "O celestial estava bloqueado sem estar no nível 40 ou 90. Efetuando desbloqueio.");
		}
	}

	if (evId >= TOD_Class::Celestial && level >= 199 && !Mob[clientId].Mobs.Info.Unlock200)
		return false;

	// O Usuário tem uma experiência maior
	if (exp >= nextExp)
	{
		// Adiciona o level no usuário
		Mobs.Player.Status.Level++;
		Mobs.Player.bStatus.Level++;
		level++;

		// Se está upando pro 221 e tem uma guild
		// damos a capa gratuitamente para o usuário
		if ((evId == MORTAL || evId == ARCH) && level == 219 && Mobs.Player.GuildIndex != 0 && Mobs.Player.CapeInfo == 0)
		{
			int guildIndex = Mobs.Player.GuildIndex;
			int guildCape = g_pGuild[guildIndex].Kingdom;

			Mobs.Player.Equip[15] = st_Item{};
			Mobs.Player.Equip[15].Index = 545 + (guildCape - CAPE_BLUE);

			SendItem(clientId, SlotType::Equip, 14, &Mobs.Player.Equip[15]);
			Log(clientId, LOG_INGAME, "Recebeu a capa %hu por subir de nível 221 estando na guild %s (%d)", Mobs.Player.Equip[15].Index, g_pGuild[guildIndex].Name.c_str(), guildIndex);
		}

		/*if(evId == ARCH)
		{
			if((level == 354 && !Mob[clientId].Mobs.Info.Unlock354) || (level == 369 && !Mob[clientId].Mobs.Info.Unlock369))
			{
				Mobs.Player.Exp = g_pNextLevel[evId][level - 1];
				Mobs.Info.LvBlocked = 1;

				SendClientMessage(clientId, g_pLanguageString[_NN_Level_Blocked]);

				SendEtc(clientId);
			}
			else if(level == 354 && Mob[clientId].Mobs.Info.Unlock354)
				Log(clientId, LOG_INGAME, "Não travou o nível 355 por já ter travado uma vez!");
			else if(level == 369 && Mob[clientId].Mobs.Info.Unlock369)
				Log(clientId, LOG_INGAME, "Não travou o nível 370 por já ter travado uma vez!");

		}*/

		bool block = false;
		if (evId == CELESTIAL)
			block = ((level == 39 && !Mobs.Info.Unlock39) || (level == 89 && !Mobs.Info.Unlock89));

		if (evId == CELESTIAL || evId == SUBCELESTIAL)
			block = block || (((level == 199 && !Mobs.Info.Unlock200 && !Mobs.Sub.Info.Unlock200) || (level == 209 && !Mobs.Info.Unlock210 && !Mobs.Sub.Info.Unlock210)) && Mob[clientId].Mobs.GetTotalResets() == 3);

		if (block)
		{
			Mobs.Player.Exp = g_pNextLevel[evId][level - 1];
			Mobs.Info.LvBlocked = 1;

			SendClientMessage(clientId, g_pLanguageString[_NN_Level_Blocked]);

			SendEtc(clientId);
		}
		else if (level == 199 && Mob[clientId].Mobs.GetTotalResets() < 3)
		{
			Mobs.Player.Exp = g_pNextLevel[evId][level - 1];
			SendEtc(clientId);
		}

		// Atribui uma nova pontuação de CP para o usuário
		INT32 pkPoint = GetPKPoint(clientId);
		if ((pkPoint - 75) < 75)
		{
			SetPKPoint(clientId, GetPKPoint(clientId) + 3);

			SendCreateMob(clientId, clientId); 
			Log(clientId, LOG_INGAME, "Pontos CP subiu em 3. CP novo: %d", GetPKPoint(clientId));
			LogPlayer(clientId, "Pontos CP subiu em 3. CP novo: %d", GetPKPoint(clientId) - 75);
		}

		Mobs.Player.Status.curHP = Mobs.Player.Status.maxHP;
		Mobs.Player.Status.curMP = Mobs.Player.Status.maxMP;

		Users[clientId].Potion.CountHp = Mobs.Player.Status.maxHP;
		Users[clientId].Potion.CountMp = Mobs.Player.Status.maxMP;

		SendSetHpMp(clientId);

		GetCurrentScore(clientId);

		SendScore(clientId);
		SendEtc(clientId);

		SendEmotion(clientId, 14, 3);

		SendClientMessage(clientId, "+ + + LEVEL UP + + +");

		Log(clientId, LOG_INGAME, "Level UP para level %d. Ev: %d - %dx %dy", level, evId, Mob[clientId].Target.X, Mob[clientId].Target.Y);
		LogPlayer(clientId, "Level UP para level %d", level + 1);
		LevelItem(clientId);

		if (Mobs.Player.Equip[0].EFV2 == SUBCELESTIAL)
		{
			stQuestInfo quest = Mobs.Info;

			INT32 i = -1;
			switch (Mobs.Player.bStatus.Level)
			{
			case 120:
				if (quest.Add120 == 0)
					i = 0;
				break;
			case 150:
				if (quest.Add151 == 0)
					i = 1;
				break;
			case 180:
				if (quest.Add180 == 0)
					i = 2;
				break;
			case 190:
			case 199:
				if (quest.Add199 == 0)
					i = 3;
				break;
			}

			if (i != -1)
			{
				st_Item *cyt = &Mobs.Player.Equip[1];
				if (cyt->Index >= 3500 && cyt->Index <= 3507)
				{
					Log(clientId, LOG_INGAME, "%s com adicional do level %d. Antigo adicional: [%d] [%d %d %d %d %d %d]", ItemList[cyt->Index].Name, Mobs.Player.bStatus.Level, cyt->Index, cyt->EF1, cyt->EFV1, cyt->EF2, cyt->EFV2, cyt->EF3, cyt->EFV3);
					INT32 ref = 0,
						add = 0;

					for (INT32 i = 0; i < 3; i++)
					{
						if (cyt->Effect[i].Index >= 43 || (cyt->Effect[i].Index >= 116 && cyt->Effect[i].Index <= 125))
						{
							ref = cyt->Effect[i].Value;
							add = cyt->Effect[i].Index;

							break;
						}
					}

					cyt->Effect[0].Index = add;
					cyt->Effect[0].Value = ref;

					cyt->Effect[1].Index = EF_HP;
					cyt->Effect[1].Value = szAdds[i][1];

					cyt->Effect[2].Index = EF_AC;
					cyt->Effect[2].Value = szAdds[i][0];

					if (i == 0)
						Mobs.Info.Add120 = 1;
					else if (i == 1)
						Mobs.Info.Add151 = 1;
					else if (i == 2)
						Mobs.Info.Add180 = 1;
					else if (i == 3)
						Mobs.Info.Add199 = 1;

					SendItem(clientId, SlotType::Equip, 1, cyt);
				}
				else
					Log(clientId, LOG_INGAME, "%s não recebeu o adicional do level %d por não ter cythera equipada no personagem", ItemList[cyt->Index].Name, Mobs.Player.bStatus.Level);
			}
		}

		return true;
	}

	return false;
}

INT32 CMob::CheckQuarter(long long expEarned)
{
	int evId = Mobs.Player.Equip[0].EFV2;
	int level = Mobs.Player.Status.Level;

	if ((evId <= ARCH && level >= MAX_LEVEL) || (evId >= CELESTIAL && level >= MAX_LEVEL_CELESTIAL))
		return true;

	long long exp = Mobs.Player.Exp;
	long long actualExp = g_pNextLevel[evId][level];
	long long nextExp = g_pNextLevel[evId][level] - g_pNextLevel[evId][level - 1];
	long long previousExp = 0;

	if(level > 0)
		previousExp = g_pNextLevel[evId][level - 1];

	nextExp /= 4;

	BOOL needRefresh = FALSE;
	for (unsigned long long i = 3; i > 0; i--)
	{
		if (exp < (previousExp + nextExp * i))
		{
			if ((exp + expEarned) >= previousExp + nextExp * i)
			{
				needRefresh = TRUE;

				SendClientMessage(clientId, "Adquiriu %d/4 de bônus.", i);
				break;
			}
		}
	}

	if (needRefresh)
	{
		Mobs.Player.Status.curHP = Mobs.Player.Status.maxHP;
		Mobs.Player.Status.curMP = Mobs.Player.Status.maxMP;

		Users[clientId].Potion.CountHp = Mobs.Player.Status.maxHP;
		Users[clientId].Potion.CountMp = Mobs.Player.Status.maxMP;

		SendSetHpMp(clientId);

		GetCurrentScore(clientId);

		SendScore(clientId);
		SendEtc(clientId);
	}

	return needRefresh;
}

bool CMob::isBagActive(TOD_Bag bag) const
{
	if (bag == TOD_Bag::FirstBag)
	{
		if (Mob[clientId].Mobs.Player.Inventory[60].Index != 3467)
			return false;

		return TimeRemaining(Mob[clientId].Mobs.Player.Inventory[60].EFV1, Mob[clientId].Mobs.Player.Inventory[60].EFV2, Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) > 0.0f;
	}

	if (bag == TOD_Bag::SecondBag)
	{
		if (Mob[clientId].Mobs.Player.Inventory[61].Index != 3467)
			return false;

		return TimeRemaining(Mob[clientId].Mobs.Player.Inventory[61].EFV1, Mob[clientId].Mobs.Player.Inventory[61].EFV2, Mob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900) > 0.0f;
	}

	throw std::exception("Invalid bag type");
}

int stCharInfo::GetTotalResets() const
{
	int totalResets = 0;
	if (Info.Reset_1)
		totalResets++;
	if (Info.Reset_2)
		totalResets++;
	if (Info.Reset_3)
		totalResets++;

	return totalResets;
}