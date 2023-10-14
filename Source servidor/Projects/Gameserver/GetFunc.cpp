#include "cServer.h"
#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include <array>
#include "UOD_Arena.h"

int	g_pDistanceTable[8][8]=
{	0,1,2,3,4,5,6,7,
	1,1,2,3,4,5,6,7,
	2,2,3,4,4,5,6,7,
	3,3,4,4,5,5,6,7,
	4,4,4,5,5,5,6,7,
	5,5,5,5,5,6,6,7,
	6,6,6,6,6,6,7,7,
	7,7,7,7,7,7,7,7
};

unsigned int g_pArenaArea[5][4] = 
{
	{2052, 2171, 2052, 2163},
	{2432, 2675, 1672, 1767},
	{2448, 2476, 1966, 2024},
	{1070, 1110, 1130, 1158},
	{1036, 1072, 1700, 1760}
};

void GetCreateMob(int clientId, BYTE *bufPak)
{
	memset(bufPak, 0x00, sizeof p364);

	CMob *mob = (CMob*)&Mob[clientId];

	p364 *p = (p364*)(bufPak);

	p->Header.ClientId = 0x7530;
	p->Header.PacketId = 0x364;
	p->Header.Size = sizeof p364;

	memcpy(&p->Status, &mob->Mobs.Player.Status, sizeof st_Status);

	int len = strlen(mob->Mobs.Player.Name);
	if(clientId >= MAX_PLAYER)
	{
		if(len >= 16)
			len = 15; 

		p364_Mob *packetMob = reinterpret_cast<p364_Mob*>(bufPak);
		strncpy_s(packetMob->Name, 16, mob->Mobs.Player.Name, 16);

		for(INT32 i = len; i > 0; i--)
		{
			if(packetMob->Name[i] != ' ' && packetMob->Name[i] != '_' && packetMob->Name[i] != 0)
				break;

			packetMob->Name[i] = 0;
		}

		packetMob->Name[len] = 0;

		if(Mob[clientId].Mobs.Player.CapeInfo == 4)
			packetMob->Status.Defense = 0;
		else
			packetMob->Status.Defense = 1;
	}
	else
	{
		if (len >= 12)
		{
			p364_Mob* packetMob = reinterpret_cast<p364_Mob*>(bufPak);

			strncpy_s(packetMob->Name, 16, mob->Mobs.Player.Name, 16);
		}
		else
			strncpy_s(p->Name, 12, mob->Mobs.Player.Name, 16);
	}
	p->Spawn.Type = mob->SpawnType;
	
	if(Mob[clientId].Motion != 0)
		p->Spawn.Type |= (Mob[clientId].Motion << 4);

	p->Index = clientId;

	p->Current.X = Mob[clientId].Target.X;
	p->Current.Y = Mob[clientId].Target.Y;
	p->Header.TimeStamp = CurrentTime;

	bool isUsingCostume = Mob[clientId].Mobs.Player.Equip[12].Index != 0;
	for(int i = 0; i < 17; i++)//alterado
	{
		short effValue = 0;
		st_Item eqItem = mob->Mobs.Player.Equip[i];

		if(i == 14)
		{
			if(eqItem.Index >= 2360 && eqItem.Index <= 2390)
			{
				if(*(INT16*)&eqItem.Effect[0].Index <= 0)
				{
					p->Item_Refine[i] = 0;
					p->pAnctCode[i] = 0;
					
					continue;
				}
			}
		}

		if (i == 0)
		{
			// se tiver um traje equipado
			if (Mob[clientId].Mobs.Player.Equip[12].Index != 0 && Mob[clientId].Mobs.Player.ClassInfo == 2)
				eqItem.Index = eqItem.EF2;
		}
		p->pAnctCode[i] = GetAnctCode(&eqItem, isUsingCostume);
		p->Item_Refine[i] = GetItemIDAndEffect(&eqItem, i, isUsingCostume);

	}

	for(int i = 0 ; i < 32; i++)
	{
		if(mob->Mobs.Affects[i].Index == 0 || mob->Mobs.Affects[i].Time <= 0)
		{
			p->Affect[i].Index = 0;
			p->Affect[i].Time = 0;
			continue;
		}

		p->Affect[i].Index = mob->Mobs.Affects[i].Index & 255;
		p->Affect[i].Time = mob->Mobs.Affects[i].Time & 255;
	}
	
	p->GuildIndex = mob->Mobs.Player.GuildIndex;
	p->GuildMemberType = mob->Mobs.Player.GuildMemberType;

	if(Mob[clientId].GuildDisable == 1)
	{
		p->GuildMemberType = 0;
		p->GuildIndex = 0;
	}

	if (mob->Mobs.Player.GuildMemberType == 9)
		p->Spawn.Type |= 0x80;

	else if (mob->Mobs.Player.GuildMemberType >= 6)
		p->Spawn.Type |= 0x40;

	if(clientId < MAX_PLAYER)
	{
		p->ChaosPoints = GetPKPoint(clientId);
		p->CurrentKill = GetCurKill(clientId);
		p->TotalKill = GetTotKill(clientId);

		if(GetGuilty(clientId) > 0)
			p->ChaosPoints = 0;

		if(Mob[clientId].Mobs.Player.Info.Merchant & 1)
			p->Status.Merchant.Merchant = 1;

		p->Life = Mob[clientId].Lifes;
	}
	
//#if !defined(_DEBUG)
//	if((Mob[clientId].Mobs.Player.Status.Level >= 69 && Mob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL) || Mob[clientId].Mobs.Player.Equip[0].EFV2 >= ARCH)
//		strncpy_s(p->pTab, mob->Tab, 26);
//#else
//	sprintf_s(p->pTab, "index = %d", clientId);
//#endif

	if(Mob[clientId].Target.X >= 2604 && Mob[clientId].Target.Y >= 1708 && Mob[clientId].Target.X <= 2648 && Mob[clientId].Target.Y <= 1744)
	{
		p->GuildIndex = 0;
		p->GuildMemberType = 0;

		p->Item_Refine[15] = 0;
		p->pAnctCode[17]   = 0;
	}

	if (clientId < MAX_PLAYER && Users[clientId].Arena.GroupIndex != -1 && Mob[clientId].Target.X >= 143 && Mob[clientId].Target.Y >= 546 && Mob[clientId].Target.X <= 195 && Mob[clientId].Target.Y <= 625)
	{
		p->GuildIndex = 0;
		p->GuildMemberType = 0;

		p->Item_Refine[15] = 0;
		p->pAnctCode[17] = 0;

		sprintf_s(p->pTab, "Grupo %s (%d)", GroupsName[Users[clientId].Arena.GroupIndex], Users[clientId].Arena.Kills);
	}
}

void GetCreateMobTrade(int clientId, BYTE *bufPak)
{
	memset(bufPak, 0x00, sizeof p363);

	CMob *mob = (CMob*)&Mob[clientId];
	p363 *p = (p363*)(bufPak);

	p->Header.Size = sizeof p363;
	p->Header.ClientId = 0x7530;
	p->Header.PacketId = 0x363;

	memcpy(&p->Status, &mob->Mobs.Player.Status, sizeof st_Status);
	
	for (int i = 0; i < 12; i++)
		p->Name[i] = mob->Mobs.Player.Name[i];

	p->Index = clientId;
	
	p->Spawn.Type = mob->SpawnType;
	
	p->GuildIndex = mob->Mobs.Player.GuildIndex;
	p->Unknow = mob->Mobs.Player.GuildMemberType;

	p->Current.X = mob->Target.X;
	p->Current.Y = mob->Target.Y;

	bool isUsingCostume = Mob[clientId].Mobs.Player.Equip[12].Index != 0;
	for(int i = 0;i < 17; i++)
	{
		short effValue = 0;
		st_Item eqItem{ mob->Mobs.Player.Equip[i] };

		if(i == 14)
			if(eqItem.Index >= 2360 && eqItem.Index <= 2390)
				if(*(short*)&eqItem.Effect[0].Index <= 0)
					eqItem.Index = 0;

		p->pAnctCode[i] = GetAnctCode(&eqItem, isUsingCostume);
		p->Item_Refine[i] = GetItemIDAndEffect(&eqItem, i, isUsingCostume);
	}

	for(int i = 0 ; i < 32; i++)
	{
		if(mob->Mobs.Affects[i].Index == 0 || mob->Mobs.Affects[i].Time <= 0)
		{
			p->Affect[i].Index = 0;
			p->Affect[i].Time = 0;
			continue;
		}
		
		p->Affect[i].Index = mob->Mobs.Affects[i].Index;
		p->Affect[i].Time = mob->Mobs.Affects[i].Index & 255;
	}

	p->ChaosPoints = GetPKPoint(clientId);
	p->CurrentKill = GetCurKill(clientId);
	p->TotalKill = GetTotKill(clientId);

	if (GetGuilty(clientId) > 0)
		p->ChaosPoints = 0;

	if (Mob[clientId].Mobs.Player.Info.Merchant & 1)
		p->Status.Merchant.Merchant = 1;
	
	strncpy_s(p->pTab, mob->Tab, 26);
	strncpy_s(p->StoreName, Users[clientId].AutoTradeName, 24);
}

int GetAnctCode(st_Item *item, bool usingCostume)
{
	int value = 0;

	if(item->Index >= 2360 && item->Index < 2389)
	{
		if(item->EFV3 >= 11)
			return item->EFV3;
		return 0;
	}

    if(item->EF1 == 43)
		value = item->EFV1;
    else if(item->EF2 == 43)
		value = item->EFV2;
    else if(item->EF3 == 43)
		value = item->EFV3;

	else if (item->EF1 >= 116 && item->EF1 <= 125)
		return value = item->EF1 - 3;
	else if(item->EF2 >= 116 && item->EF2 <=125)
		return value = item->EF2 - 3;
	else if(item->EF3 >= 116 && item->EF3 <=125)
		return value = item->EFV3 - 3;

    if(value == 0)
		return 0;
    if(value < 230)
		return 43;

    switch(value % 4)
    {
        case 0:  
			return 0x30;
        case 1:  
			return 0x40;
        case 2:  
			return 0x10;
        case 3:  
			return 0x20;
		default: 
			return 0x20;
    }
}

int GetItemIDAndEffect(st_Item* Item, int mnt, bool usingCostume)
{
	int value;
	bool colored = false;

	if(mnt == 14)
	{
		return Item->Index | (Item->EF2 / 10 * 4096);
	}
	else
	{
		if (Item->EF1 >= 116 && Item->EF1 <=  125)
		{
			if (!usingCostume)
			{
				value = Item->EF1;
				colored = true;
			}
			else
				value = Item->EFV1;
		}
		else if (Item->EF2 >= 116 && Item->EF2 <=  125)
		{
			if (!usingCostume)
			{
				value = Item->EF2;
				colored = true;
			}
			else
				value = Item->EFV2;
		}
		else if (Item->EF3 >= 116 && Item->EF3 <=  125)
        {
			if (!usingCostume)
			{
				value = Item->EF3;
				colored = true;
			}
			else
				value = Item->EFV3;
		}
		else if (Item->EF1 == 43)
			value = Item->EFV1;
		else if (Item->EF2 == 43)
			value = Item->EFV2;
		else if (Item->EF3 == 43)
			value = Item->EFV3;
		else
			return Item->Index;
	}

	if(value > 9 && mnt != 14 && !colored)
	{
		if (value < 234)
			value = 10;
		else if (value < 238)
			value = 11;
		else if (value < 242)
			value = 12;
		else if (value < 246)
			value = 13;
		else if (value < 250)
			value = 14;
		else if (value < 254)
			value = 15;
		else
			value = 16;
    }
	else if(colored)
		if(value >= 9) 
			value = 9;
	
	return Item->Index | (value * 0x1000);
}

bool GetEmptyMobGrid(int Index, unsigned int *posX, unsigned int *posY)
{
	if (*posX < 0 || *posX >= 4096 || *posY < 0 || *posY >= 4096)
		return false;

	if (g_pMobGrid[*posY][*posX] == Index)
		return true;
	
	int LOCAL1 = g_pMobGrid[*posY][*posX];

	if (LOCAL1 < 0 || LOCAL1 >= 30000)
	{
		g_pMobGrid[*posY][*posX] = 0;
		LOCAL1 = 0;
	}

	if (Mob[LOCAL1].Mode == 0)
	{
		g_pMobGrid[*posY][*posX] = 0;
		LOCAL1 = 0;
	}

	if (LOCAL1 == 0)
	{
		if (g_pHeightGrid[*posY][*posX] != 127)
			return true;
	}

	for (unsigned int nY = (*posY - 1); nY <= (*posY + 1); nY++)
	{
		for (unsigned int nX = (*posX - 1); nX <= (*posX + 1); nX++)
		{
			if (nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 2); nY <= (*posY + 2); nY++)
	{
		for (unsigned int nX = (*posX - 2); nX <= (*posX + 2); nX++)
		{
			if (nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 3); nY <= (*posY + 3); nY++)
	{
		for (unsigned int nX = (*posX - 3); nX <= (*posX + 3); nX++)
		{
			if (nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 4); nY <= (*posY + 4); nY++)
	{
		for (unsigned int nX = (*posX - 4); nX <= (*posX + 4); nX++)
		{
			if (nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	for (unsigned int nY = (*posY - 5); nY <= (*posY + 5); nY++)
	{
		for (unsigned int nX = (*posX - 5); nX <= (*posX + 5); nX++)
		{
			if (nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
				continue;

			if (g_pMobGrid[nY][nX] == 0)
			{
				if (g_pHeightGrid[nY][nX] != 127)
				{
					*posX = nX;
					*posY = nY;
					return true;
				}
			}
		}
	}

	return false;
}

void GetAction(int clientId, short posX, short posY, void *buf)
{
	p36C *p = (p36C*)buf;

	CMob *mob = (CMob*)(&Mob[clientId]);

	p->Header.ClientId = clientId;
	p->Header.PacketId = 0x36C;
	p->Header.Size = sizeof p36C;

	p->LastPos.X = mob->Target.X;
	p->LastPos.Y = mob->Target.Y;

	p->Destiny.X = posX;
	p->Destiny.Y = posY;

	p->MoveSpeed = GetSpeed(&mob->Mobs.Player.Status);
	p->MoveType = 0;
	
	memcpy(&p->Command[0], &Mob[clientId].Route, 24);
}

INT32 GetInView(INT32 a, INT32 b)
{
	if (Mob[a].Target.X - VIEWGRIDX > Mob[b].Target.X)
		return false;

	if (Mob[a].Target.X + VIEWGRIDX < Mob[b].Target.X)
		return false;

	if (Mob[a].Target.Y - VIEWGRIDX > Mob[b].Target.Y)
		return false;

	if (Mob[a].Target.Y + VIEWGRIDX < Mob[b].Target.Y)
		return false;

	return true;
}

int GetItemSanc(st_Item *item)
{
    int value = 0;

    if(item->Index >= 2360 && item->Index <= 2389)
    {
        //Montarias.
        value = (item->Effect[2].Index / 10);

        if(value > 9)
            value = 9;

        return value;
    }

    if(item->Index >= 2330 && item->Index <= 2359)
    {
        //Crias.
        return 0;
    }

    if(item->Effect[0].Index == 43)
        value = item->Effect[0].Value;
    else if(item->Effect[1].Index == 43)
        value = item->Effect[1].Value;
    else if(item->Effect[2].Index == 43)
        value = item->Effect[2].Value;
	else
	{
		for(INT32 i = 0; i < 3; i++)
		{
			if(item->Effect[i].Index >= 116 && item->Effect[i].Index <= 125)
			{
				value = item->Effect[i].Value;

				break;
			}
		}
	}

    if(value >= 230)
    {
        value = 10 + ((value - 230) / 4);
        if(value > 15)
            value = 15;
    }
    else
        value %= 10;

    return value;
}

bool IsImpossibleToRefine(st_Item* item)
{
	if (item->Index <= 0 || item->Index >= MAX_ITEMLIST)
		return false;

	bool haveRef = false;
	for (int i = 0; i < 3; i++)
	{
		if (item->Effect[i].Index == 0)
			return false;

		if (item->Effect[i].Index == 43 || (item->Effect[i].Index >= 116 && item->Effect[i].Index <= 125))
			return false;
	}

	return true;
}

int GetInfoClass (int face) 
{
	if(face < 10)
		return 0;
	else if(face < 20)
		return 1;
	else if(face < 30 || face == 32)
		return 2;
	else if(face < 40)
		return 3;

	return 4;
}

short GetEffectValueByIndex(int ItemID, int Index)
{
	sItemData *item = &ItemList[ItemID];
	for(int i = 0; i < 12; i++)
	{
		if(item->Effect[i].Index == Index)
			return item->Effect[i].Value;
	}
	return 0;
}

MapAttribute GetAttribute(int posX, int posY)
{
	if (posX < 0 || posX > 4096 || posY < 0 || posY > 4096)
		return g_pAttributeMap[0][0];

	int gridX = (posX >> 2) & 0x3FF;
	int gridY = (posY >> 2) & 0x3FF;

	return g_pAttributeMap[gridY][gridX];
}

int GetSpeed(st_Status *status)
{
	int speed = status->Move.Speed;

	if(speed < 1)
		speed = 1;
	else if(speed > 6)
		speed = 6;

	return speed;
}

int GetDistance(int x1,int y1,int x2,int y2)
{
	int x, y;
	if(x1 > x2)
		x = x1 - x2;
	else
		x = x2 - x1;

	if(y1 > y2)
		y = y1 - y2;
	else
		y = y2 - y1;

	if(x <= 7 && y <= 7)
		return g_pDistanceTable[y][x]; // array com distâncias

	if(x > y)
		return x + 1;

	return y + 1;
}

INT32 GetRoute(unsigned int x, unsigned int y, unsigned int *targetX, unsigned int *targetY, char *route, int distance)
{
	unsigned int lastX = x; //local1
	unsigned int lastY = y; //local2
	unsigned int tX = *targetX; //local3
	unsigned int tY = *targetY; //local4

	memset(route, 0, 24);

	int i = 0; // LOCAL5
	for (; i < distance || i < 23; i++)
	{
		if ((x - g_HeightPosX) < 1 || (y - g_HeightPosY) < 1 || (x - g_HeightPosX) > (g_HeightWidth - 2) || (y - g_HeightPosY) > (g_HeightHeight - 2))
		{
			route[i] = 0;

			break;
		}

		// 0040C544
		INT32 cul = g_pHeightGrid[y - g_HeightPosY][x - g_HeightPosX], // local6
			  kk  = 0; // local7

		if (cul > 8)
			kk = 0;

		INT32 nw = g_pHeightGrid[(y - g_HeightPosY - 1)][x - g_HeightPosX - 1];	  //nw
		INT32 n = g_pHeightGrid [(y - g_HeightPosY - 1)][x - g_HeightPosX    ];   // n
		INT32 ne = g_pHeightGrid[(y - g_HeightPosY - 1)][x - g_HeightPosX + 1];   // ne
		INT32 w = g_pHeightGrid [(y - g_HeightPosY)]    [x - g_HeightPosX - 1];   // w
		INT32 e = g_pHeightGrid [(y - g_HeightPosY)]    [x - g_HeightPosX + 1];   // e
		INT32 sw = g_pHeightGrid[(y - g_HeightPosY + 1)][x - g_HeightPosX - 1];   // sw
		INT32 s = g_pHeightGrid [(y - g_HeightPosY + 1)][x - g_HeightPosX    ];   // s
		INT32 se = g_pHeightGrid[(y - g_HeightPosY + 1)][x - g_HeightPosX + 1];   // se LOCAL15

		if (tX == x && tY == y)
		{
			route[i] = 0; 
			break;
		}
		if (tX == x && tY < y && n < cul + 8 && n > cul - 8) 
		{
			route[i] = '2'; 
			y--; 
			continue;
		}
		if (tX > x && tY < y && ne < cul + 8 && ne > cul - 8) 
		{
			route[i] = '3'; 
			x++; 
			y--; 
			continue;
		}
		if (tX > x && tY == y && e < cul + 8 && e > cul - 8) 
		{
			route[i] = '6'; 
			x++; 
			continue;
		}
		if (tX > x && tY > y && se < cul + 8 && se > cul - 8) 
		{
			route[i] = '9';
			x++;	
			y++;	
			continue;
		}
		if (tX == x && tY > y && s <cul + 8 && s > cul - 8) 
		{
			route[i] = '8';
			y++; 
			continue;
		}
		if (tX < x && tY > y && sw < cul + 8 && sw > cul - 8) 
		{
			route[i] = '7';
			x--;	
			y++; 
			continue;
		}
		if (tX < x && tY == y && w < cul + 8 && w > cul - 8) 
		{
			route[i] = '4'; 
			x--; 
			continue;
		}
		if (tX < x && tY < y && nw < cul + 8 && nw > cul - 8) 
		{
			route[i] = '1';
			x--; 
			y--; 
			continue;
		}
		if (tX > x && tY < y && e < cul + 8 && e > cul - 8) 
		{
			route[i] = '6'; 
			x++; 
			continue;
		}
		if (tX > x && tY < y && n < cul + 8 && n > cul - 8) 
		{
			route[i] = '2'; 
			y--; 
			continue;
		}
		if (tX > x && tY > y && e < cul + 8 && e > cul - 8) 
		{
			route[i] = '6'; 
			x++; 
			continue;
		}
		if (tX > x && tY > y && s < cul + 8 && s > cul - 8) 
		{
			route[i] = '8'; 
			y++; 
			continue;
		}
		if (tX < x && tY > y && w < cul + 8 && w > cul - 8) 
		{
			route[i] = '4'; 
			x--; 
			continue;
		}
		if (tX < x && tY > y && s < cul + 8 && s > cul - 8)		
		{
			route[i] = '8'; 
			y++; 
			continue;
		}
		if (tX < x && tY < y && w < cul + 8 && w > cul - 8)
		{
			route[i] = '4';
			x--; 
			continue;
		}
		if (tX < x && tY < y && n < cul + 8 && n > cul - 8) 
		{
			route[i] = '2';
			y--;
			continue;
		}
		if (tX == x + 1 || tY == y + 1 || tX == x - 1 || tY == y - 1) 
		{
			route[i] = 0; 
			break;
		}

		//0040CAC6
		if (tX == x && tY > y && se < cul + 8 && se > cul - 8) 
		{
			route[i] = '9';
			x++; 
			y++;
			continue;
		}
		if (tX == x && tY > y && sw <cul + 8 && sw > cul - 8) 
		{ 
			route[i] = '7'; 
			x--; 
			y++;
			continue; 
		}
		if (tX == x && tY < y && ne < cul + 8 && ne > cul - 8) 
		{ 
			route[i] = '3';
			x++;
			y--;
			continue; 
		}
		if (tX == x && tY < y && nw < cul + 8 && nw > cul - 8)
		{ 
			route[i] = '1';
			x--;
			y--;
			continue; 
		}
		if (tX < x && tY == y && sw < cul + 8 && sw > cul - 8) 
		{ 
			route[i] = '7'; 
			x--;
			y++;
			continue; 
		}
		if (tX < x && tY == y && nw  < cul + 8 && nw > cul - 8)
		{ 
			route[i] = '1';
			x--;
			y--;
			continue; 
		}
		if (tX > x && tY == y && se < cul + 8 && se > cul - 8)
		{ 
			route[i] = '9';
			x++; 
			y++;
			continue; 
		}
		if (tX > x && tY == y && ne < cul + 8 && ne > cul - 8)
		{ 
			route[i] = '3';
			x++; 
			y--; 
			continue; 
		}
		
		break;
	}

	route[i] = 0;

	if (lastX == x && lastY == y)
		return 0;

	*targetX = x;
	*targetY = y;

	if (lastX == x && lastY == y)
		return 0;

	return 1;
}

int GetDamage(int dam, int ac, int combat) 
{
	int calcDmg = dam - (ac >> 1);
	combat >>= 1;
    if(combat > 7)
        combat = 7;
 
    int calcRand = 12 - combat;
    int incDmg = (Rand() % calcRand) + combat + 99;
 
    calcDmg = (calcDmg * incDmg) / 100;
    if(calcDmg < -50)
		calcDmg = 0;
    else if(calcDmg >= -50 && calcDmg < 0)
		calcDmg = (calcDmg + 50) / 7;
    else if(calcDmg >= 0 && calcDmg <= 50)
		calcDmg = ((calcDmg * 5) >> 2) + 7;
 
    if(calcDmg <= 0)
		calcDmg = 1;
 
    return calcDmg;
}

INT32 GetParryRate(int clientId, int mobId, int type)
{
	constexpr std::array pEvadeEsfera = 
	{
		0, 10, 0, 0, 10, 0, 0, 10, 0, 16, 10, 0, 0, 60, 60, 50, 60, 0, 0, 0
	};
	
	INT32 dex = Mob[clientId].Mobs.Player.Status.DEX / 5; // arg3?
	if(Mob[clientId].Mobs.Player.AffectInfo.Evasion)
		dex += 150;

	if(type == -2)
		dex += 300;
	
	if(Mob[clientId].Mobs.Player.AffectInfo.SpeedMov)
		dex += 50;
	
	INT32 targetDex = Mob[mobId].Mobs.Player.Status.DEX; // local1
	if(targetDex > 1000)
		targetDex = 1000;

	INT32 targetDexCalc1 = Mob[mobId].Mobs.Player.Status.DEX - 1000; // local2
	if(targetDexCalc1 < 0)
		targetDexCalc1 = 0;

	if(targetDexCalc1 > 2000)
		targetDexCalc1 = 2000;

	INT32 targetDexCalc2 = Mob[mobId].Mobs.Player.Status.DEX - 3000;
	if(targetDexCalc2 < 0)
		targetDexCalc2 = 0;

	// ECX = LOCAL_1 >> 1
	// ESI = LOCAL_2 >> 2 + arg2
	// EAX = LOCAL_3 >> 3
	// LOCAL_4 = ESI + ECX + EAX
	INT32 calcParry = (targetDex >> 1) + (targetDexCalc1 >> 2) + (targetDexCalc2 >> 3);//(LOCAL_3 >> 3) + ((LOCAL_2 + (LOCAL_1 >> 1)) >> 2 + player[clientId].Parry);
	calcParry -= dex;

	if (Mob[mobId].Mobs.Player.Equip[14].Index >= 3980 && Mob[mobId].Mobs.Player.Equip[14].Index <= 3999)
	{
		int itemId = Mob[mobId].Mobs.Player.Equip[14].Index - 3980;
		calcParry += pEvadeEsfera[itemId];
	}

	if (Mob[clientId].Mobs.Player.Equip[14].Index >= 3980 && Mob[clientId].Mobs.Player.Equip[14].Index <= 3999)
	{
		int itemId = Mob[clientId].Mobs.Player.Equip[14].Index - 3980;
		calcParry += pEvadeEsfera[itemId];
	}

	if (calcParry > 750)
		calcParry = 750;

	if (Mob[clientId].Mobs.Player.ClassInfo == 1 && (Mob[clientId].Mobs.Player.Learn[0] & (1 << (47 % 24))) && type == -2)
		calcParry -= 50;

	if (Mob[clientId].Mobs.Player.ClassInfo == 0 && ((Mob[clientId].Mobs.Player.Learn[0] & (1 << 7)) || (Mob[clientId].Mobs.Player.Learn[0] & (1 << 23))) && type == -1)
		calcParry -= 75;

	calcParry += Mob[mobId].Parry;

	int jewelIndex = Mob[clientId].Jewel;
	if (jewelIndex >= 0 && jewelIndex < 32 && (Mob[clientId].Mobs.Affects[jewelIndex].Value & 64) != 0)
		calcParry -= 50;

	if ((Mob[clientId].Mobs.Player.Learn[0] & (1 << 28)))
		calcParry -= 75;

	calcParry -= Mob[clientId].HitRate;

	if(calcParry < 1)
		calcParry = 1;

	return calcParry;
}

int GetPKPoint(int Index)
{
	return Mob[Index].Mobs.Player.Inventory[63].Effect[0].Index;
}

INT32 GetSkillDamage_PvP(INT32 skillId, CMob *player, INT32 weather, INT32 weaponDamage)
{ //7DCDD8
	INT32 value[5];
	INT32 damage[2];

	INT32 type = SkillData[skillId].InstanceType;  // local1

	auto mob = &player->Mobs.Player;
	INT32 mod = skillId % 0x18;
	value[0] = (mod >> 3) + 1; // local2  - 3 

	value[1] = mob->Status.Level; // local3  - 199
	if (mob->Equip[0].EFV2 >= CELESTIAL)
		value[1] += 400;

	if (value[1] < 0)
		value[1] = 0;

	if (value[1] > 600)
		value[1] = 600;

	value[2] = mob->Status.Mastery[value[0]]; //local4

	damage[0] = SkillData[skillId].InstanceValue; //local5
	damage[1] = SkillData[skillId].AffectValue; //local6 - TickValue old

	INT32 retn = 0; //local7

	if (type == 0)
	{
		switch (skillId)
		{

		case 11:
			retn = value[2] / 10 + damage[1];
			break;
		case 13:
			retn = ((value[2] * 3) >> 2) + damage[1];
			break;
		case 41:
			retn = value[2] / 25 + 2;
			break;
		case 43:
			retn = value[2] / 3 + damage[1];
			break;
		case 44:
			retn = ((value[2] * 3) / 20 + damage[1]) << 1;
			break;
		case 45:
			retn = (value[2] / 10 + damage[1]);
			break;
			//	default:
			//		return retn;
		}
	}
	else if (type >= 1 && type <= 5)
	{
		int local8 = skillId >> 3;

		if (skillId == 97)
			retn = value[1] * 15 + damage[0];
		else if (mob->ClassInfo == 0 && local8 == 1) // TK
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.DEX * 2) + (weaponDamage * 3);
		else if (mob->ClassInfo == 0 && local8 != 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + (mob->Status.INT / 2);

			if (skillId == 7 || skillId >= 17 && skillId <= 23 && (mob->Learn[0] & 0x800000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->Status.INT / 2) * 110 / 100);

			if (skillId == 7)
				retn += (retn * 10 / 100);
		}
		else if (mob->ClassInfo == 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.INT / 2) + weaponDamage;

			if (skillId == 28) // CHOQUE DIVINO
				retn += (mob->Status.curHP * 10 / 100);
		}
		else if (mob->ClassInfo == 2)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.INT / 2);

			// Com oitava habilidade
			if (skillId == 48 || skillId == 50 || skillId == 52 || skillId == 55 && (mob->Learn[0] & 0x80))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->Status.INT / 2) * 108 / 100);

			retn += (retn * 5 / 100);
		}
		else if (mob->ClassInfo == 3)
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.STR * 3) + (weaponDamage * 3);

		if (weather == 1)
		{
			if (skillId == 39 || skillId == 48)
				retn = retn * 90 / 100;
			else if (skillId == 17)
				retn = retn * 90 / 100;
			else if (skillId == 33)
				retn = retn * 150 / 100;
		}
		else if (weather == 2 && type == 3)
			retn = retn * 120 / 100;

		if (skillId == 97)
		{
			retn = retn;
			return retn;
		}
		else if (mob->ClassInfo || local8 != 1)
		{
			if (mob->ClassInfo == 3)
				retn = (retn * 5 / 4);
			else
			{
				retn = ((player->MagicIncrement * 2) + 100) * retn / 100;
				retn += (retn * 10 / 100);
			}
		}
		else
			return (retn * 5 / 4);
	}
	else if (type == 6)
		retn = (value[2] * 3 + damage[0]);
	else if (type == 11)
	{
		retn = damage[0];
		return retn;
	}
	else
		retn = player->MagicIncrement;

	if (skillId == 79)
		retn = mob->Status.Attack * 85 / 100;

	return retn;
}

INT32 GetSkillDamage_PvM(INT32 skillId, CMob *player, INT32 weather, INT32 weaponDamage)
{ //7DCDD8
	INT32 value[5];
	INT32 damage[2];

	INT32 type = SkillData[skillId].InstanceType;  // local1

	auto mob = &player->Mobs.Player;

	INT32 mod = skillId % 0x18;
	value[0] = (mod >> 3) + 1; // local2  - 3 
	value[1] = mob->Status.Level; // local3  - 199

	if (mob->Equip[0].EFV2 >= CELESTIAL)
		value[1] += 400;

	if (value[1] < 0)
		value[1] = 0;

	if (value[1] > 600)
		value[1] = 600;

	value[2] = mob->Status.Mastery[value[0]]; //local4

	damage[0] = SkillData[skillId].InstanceValue; //local5
	damage[1] = SkillData[skillId].AffectValue; //local6 - TickValue old

	INT32 retn = 0; //local7

	if (type == 0)
	{
		switch (skillId)
		{

		case 11:
			retn = value[2] / 10 + damage[1];
			break;
		case 13:
			retn = ((value[2] * 3) >> 2) + damage[1];
			break;
		case 41:
			retn = value[2] / 25 + 2;
			break;
		case 43:
			retn = value[2] / 3 + damage[1];
			break;
		case 44:
			retn = ((value[2] * 3) / 20 + damage[1]) << 1;
			break;
		case 45:
			retn = (value[2] / 10 + damage[1]);
			break;
			//	default:
			//		return retn;
		}
	}
	else if (type >= 1 && type <= 5)
	{
		int local8 = skillId >> 3;

		if (skillId == 97)
			retn = value[1] * 15 + damage[0];
		else if (mob->ClassInfo == 0 && local8 == 1) // TK
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.DEX) + (weaponDamage * 3);
		else if (mob->ClassInfo == 0 && local8 != 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + (mob->Status.INT / 2);

			if (skillId == 7 || skillId >= 17 && skillId <= 23 && (mob->Learn[0] & 0x800000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->Status.INT / 2) * 108 / 100);

			if (skillId == 7)
				retn += (retn * 10 / 100);
		}
		else if (mob->ClassInfo == 1)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.INT / 2) + weaponDamage;

			if (skillId >= 32 && skillId <= 39 && (mob->Learn[0] & 0x8000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->Status.INT / 2) * 115 / 100);

			if (skillId == 40 && (mob->Learn[0] & 0x800000))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->Status.INT / 2) * 115 / 100);

			if (skillId == 28) // CHOQUE DIVINO
				retn += (mob->Status.curHP * 10 / 100);
		}
		else if (mob->ClassInfo == 2)
		{
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.INT / 2);

			// Com oitava habilidade
			if (skillId == 48 || skillId == 50 || skillId == 52 || skillId == 55 && (mob->Learn[0] & 0x80))
				retn = (damage[0] + value[2]) + (value[1] >> 1) + weaponDamage + ((mob->Status.INT / 2) * 115 / 100);

			retn += (retn * 8 / 100);
		}
		else if (mob->ClassInfo == 3)
			retn = (damage[0] + value[2]) + (value[1] >> 1) + (mob->Status.STR * 3) + (weaponDamage * 3);

		if (weather == 1)
		{
			if (skillId == 39 || skillId == 48)
				retn = retn * 90 / 100;
			else if (skillId == 17)
				retn = retn * 90 / 100;
			else if (skillId == 33)
				retn = retn * 150 / 100;
		}
		else if (weather == 2 && type == 3)
			retn = retn * 120 / 100;

		if (skillId == 97)
		{
			retn = retn;
			return retn;
		}
		else if (mob->ClassInfo || local8 != 1)
		{
			if (mob->ClassInfo == 3)
				retn = (retn * 5 / 4);
			else
			{
				retn = ((player->MagicIncrement * 2) + 100) * retn / 100;
				retn = retn * 5 / 4;
			}
		}
		else
			return (retn * 5 / 4);
	}
	else if (type == 6)
		retn = (value[2] * 3 + damage[0]);
	else if (type == 11)
	{
		retn = damage[0];
		return retn;
	}
	else
		retn = player->MagicIncrement;

	if (skillId == 79)
		retn = mob->Status.Attack;

	return retn;
}

INT32 GetSkillDamage_2(INT32 damage, INT32 defenserAC, INT32 master)
{
	int calcDmg = damage - (defenserAC >> 1); // local_1

	if(master > 15)
		master = 15;

	int delta =		21 - master;
	int rnd = (Rand() % delta) +  master + 90; // local3

	calcDmg = calcDmg * rnd / 100;

	if(calcDmg < -50)
		calcDmg = 0;
	else if(calcDmg >= -50 && calcDmg <= 0)
		calcDmg = (calcDmg + 50) / 10;
	else if(calcDmg >= 0 && calcDmg <= 45)
		calcDmg = ((calcDmg * 5) >> 2) + 5;
	else if(calcDmg <= 0)
		calcDmg = 1;

	return calcDmg;
}

int GetVillage(unsigned int x, unsigned int y)
{
	for(int i = 0; i < 5;i++)
	{
		if(x >= g_pCityZone[i].city_min_x && x <= g_pCityZone[i].city_max_x &&
			y >= g_pCityZone[i].city_min_y && y <= g_pCityZone[i].city_max_y)
			return i;
	}
	return 5;
}

int GetArena(unsigned int x, unsigned int y)
{
	for(int i = 0; i < 4; i++)
	{
		if(x >= g_pArenaArea[i][0] && x <= g_pArenaArea[i][1] && y >= g_pArenaArea[i][2] && y <= g_pArenaArea[i][3])
			return i;
	}

	return 5;
}

int GetEmptyAffect(int mobId, int buffId)
{
	for(int i = 0; i < 32; i++)
		if(Mob[mobId].Mobs.Affects[i].Index == buffId)
			return i;

	for(int i = 0; i < 32; i++)
		if(!Mob[mobId].Mobs.Affects[i].Index)
			return i;

	return -1;
}

int GetCurKill(int Index)
{
	return Mob[Index].Mobs.Player.Inventory[63].EFV1;
}

int GetTotKill(int Index)
{
	unsigned char f_tFrag =  Mob[Index].Mobs.Player.Inventory[63].EFV2;
	unsigned char s_tFrag =  Mob[Index].Mobs.Player.Inventory[63].EFV3;
	return f_tFrag + (s_tFrag << 8);
}

int GetGuilty(int Index)
{// 0x004012CB
	int guilty = Mob[Index].Mobs.Player.Inventory[63].EF2; 
	int value = guilty & 0xFF;

	if(value > 50)
	{
        Mob[Index].Mobs.Player.Inventory[63].EF2 = 0; // 0041E775  |. C680 906C5C01 >MOV BYTE PTR DS:[EAX+15C6C90],0
		value = 0;
	}

	return value;
}

void GetHitPosition(int arg1,int arg2,int *arg3,int *arg4)
{
	if(arg1 == *arg3 && arg2 == *arg4)
		return;

	if(arg1 == 0 || arg2 == 0 || *arg3 == 0 || *arg4 == 0)
		return;

	INT32 LOCAL_1, LOCAL_2, LOCAL_3, LOCAL_4, LOCAL_5, LOCAL_6;
	//0040D0E5
	if(arg1 > *arg3)
		LOCAL_1 = arg1 - *arg3;
	else
		LOCAL_1 = *arg3 - arg1;

	if(arg2 > *arg4)
		LOCAL_2 = arg2 - *arg4;
	else
		LOCAL_2 = *arg4 - arg2;

	INT32 LOCAL_7 = GetDistance(arg1, arg2, *arg3, *arg4);

	if(LOCAL_7 <= 1)
		return;
	
	INT32 LOCAL_8;
	if(LOCAL_1 > LOCAL_2)
	{
		if(!(*arg3 - arg1))
			return;

		// 0040D170
		LOCAL_5 = ((*arg4 - arg2) * 1000) / (*arg3 - arg1);
		LOCAL_6 = (arg2 * 1000) - (LOCAL_5 * arg1);

		if(arg1 < *arg3)
			LOCAL_3 = 1;
		else
			LOCAL_3 = -1;

		arg1 = arg1 + LOCAL_3;
		if(LOCAL_1 == LOCAL_2)
		{
			if(arg2 < *arg4)
				arg2++;
			else
				arg2--;
		}

		INT32 LOCAL_9 = LOCAL_5 * arg1 + LOCAL_6;
		INT32 LOCAL_10 = LOCAL_9 / 1000;

		LOCAL_8 = g_pHeightGrid[LOCAL_10 - g_HeightPosY][arg1 - g_HeightPosX]; // look

		if(LOCAL_8 == 127)
		{
			*arg3 = 0;
			*arg4 = 0;

			return;
		}

		INT32 LOCAL_11 = LOCAL_1;
		for(INT32 LOCAL_12 = arg1; (LOCAL_12 != *arg3); LOCAL_12 += LOCAL_3)
		{
			if(LOCAL_12 == arg1)
			{
				LOCAL_11 = LOCAL_11 - 1;
				if(LOCAL_11 < 1)
					break;

				continue;
			}

			LOCAL_9 = LOCAL_5 * LOCAL_12 + LOCAL_6;
			LOCAL_10 = LOCAL_9 / 1000;

			LOCAL_4 = LOCAL_8;
			
			LOCAL_8 = g_pHeightGrid[LOCAL_10 - g_HeightPosY][LOCAL_12 - g_HeightPosX];
			if(LOCAL_8 == 127)
			{
				*arg3 = 0;
				*arg4 = 0;

				return;
			}

			if(LOCAL_8 > (LOCAL_4 + 8) || LOCAL_8 < LOCAL_4 - 8)
			{
				*arg3 = LOCAL_12;
				*arg4 = LOCAL_10;

				return;
			}
			LOCAL_11 = LOCAL_11 - 1;

			if(LOCAL_11 < 1)
				break;
		}
	}

	else
	{
		if(!(*arg4 - arg2))
			return;

		LOCAL_5 = (*arg3 - arg1) * 1000 / (*arg4 - arg2);
		LOCAL_6 = (arg1 * 1000) - (LOCAL_5 * arg2);

		if(arg2 < *arg4)
			LOCAL_3 = 1;
		else
			LOCAL_3 = -1;

		arg2 = arg2 + LOCAL_3;
		if(LOCAL_1 == LOCAL_2)
		{
			if(arg1 < *arg3)
				arg1++;
			else
				arg1--;
		}

		INT32 LOCAL_13 = LOCAL_5 * arg2 + LOCAL_6;
		INT32 LOCAL_14 = LOCAL_13 / 1000;
		
		LOCAL_8 = g_pHeightGrid[arg2 - g_HeightPosY][LOCAL_14 - g_HeightPosX];

		if(LOCAL_8 == 127)
		{
			*arg3 = 0;
			*arg4 = 0;

			return;
		}

		INT32 LOCAL_15 = LOCAL_2;
		for(INT32 LOCAL_16 = arg2; LOCAL_16 != *arg4; LOCAL_16 += LOCAL_3)
		{
			if(LOCAL_16 == arg2)
			{
				LOCAL_15 --;

				if (LOCAL_15 < 1)
					break;

				continue;
			}

			LOCAL_13 = LOCAL_5 * LOCAL_16 + LOCAL_6;
			LOCAL_14 = LOCAL_13 / 1000;
			
			LOCAL_4 = LOCAL_8;
			LOCAL_8 = g_pHeightGrid[LOCAL_16 - g_HeightPosY][LOCAL_14 - g_HeightPosX];
			if(LOCAL_8 == 127)
			{
				*arg3 = 0;
				*arg4 = 0;

				return;
			}

			if(LOCAL_8 > (LOCAL_4 + 8) || LOCAL_8 < LOCAL_4 - 8)
			{
				*arg3 = LOCAL_14;
				*arg4 = LOCAL_16;

				return;
			}

			LOCAL_15 --;
			if(LOCAL_15 < 1)
				break;
		}
	}
}

void GetMultiAttack(int attackerId, int *target, p367 *p)
{
	INT32 first = 0;
	for(INT32 i = 0; i < 13 ;i++)
	{
		if(target[i] > 0 && target[i] < MAX_SPAWN_MOB)
		{
			first = target[i];
			break;
		}
	}

	if(first == 0)
		return;

	p->Header.ClientId = 0x7530;
	p->attackerId = attackerId;
	p->Header.TimeStamp = clock();

	p->attackerPos.X = Mob[attackerId].Target.X;
	p->attackerPos.Y = Mob[attackerId].Target.Y;
	
	p->targetPos.X = Mob[first].Target.X;
	p->targetPos.Y = Mob[first].Target.Y;

	p->Header.Size = sizeof p367;
	p->Header.PacketId = 0x367;

	p->doubleCritical = 0;
	p->currentMp = 0;
	p->currentExp = -1;

	p->skillParm = 0;
	p->skillId = -1;
	p->reqMP = 0;
	p->Motion = -1;

	if(attackerId >= MAX_PLAYER)
	{
		INT32 _rand = Rand() % 100;
		if(Mob[attackerId].Mobs.Player.SkillBar1[0] != -1 && _rand >= 25 && _rand <= 40)
			p->skillId = Mob[attackerId].Mobs.Player.SkillBar1[0];
		else if(Mob[attackerId].Mobs.Player.SkillBar1[1] != -1 && _rand >= 0 && _rand <= 49)
			p->skillId = Mob[attackerId].Mobs.Player.SkillBar1[1];
		else if(Mob[attackerId].Mobs.Player.SkillBar1[2] != -1 && _rand >= 50 && _rand <= 84)
			p->skillId = Mob[attackerId].Mobs.Player.SkillBar1[2];
		else if(Mob[attackerId].Mobs.Player.SkillBar1[3] != -1 && _rand >= 85 && _rand <= 99)
			p->skillId = Mob[attackerId].Mobs.Player.SkillBar1[3];

		if(p->skillId != -1)
		{
			//Skill recuperar
			if(p->skillId == 29) 
			{
				INT32 leader = Mob[attackerId].Leader;
				if(leader <= 0)
					leader = attackerId;

				// Pega 5% do HP do leader
				INT32 heal = Mob[leader].Mobs.Player.Status.maxHP  * 5 / 100;
				
				p->Target[0].Index = leader;
				p->Target[0].Damage = -heal;
				return;
			}
		}
	}

	INT32 total = 0;
	for(INT32 i = 0 ; i < 13; i++)
	{
		if(p->skillId == -1 && total == 1)
			break;
		
		INT32 mobId = target[i];
		if(mobId <= 0 || mobId >= MAX_PLAYER)
			continue;

		INT32 damage = 0;
		if(p->skillId != -1)
		{
			damage = GetSkillDamage_PvM(p->skillId, &Mob[attackerId], sServer.Weather, Mob[attackerId].WeaponDamage);

			damage = GetSkillDamage_2(damage, Mob[attackerId].Mobs.Player.Status.Defense, 15);
		}
		else
			damage = GetDamage(Mob[attackerId].Mobs.Player.Status.Attack, Mob[mobId].Mobs.Player.Status.Defense, 0);

		if(damage > 0 && mobId < MAX_PLAYER && Mob[mobId].ReflectDamage > 0)
		{
			damage -= Mob[mobId].ReflectDamage;

			if(damage < 0)
				damage = 0;
		}

		p->Target[total].Index = mobId;
		p->Target[total].Damage = damage;

		total ++;
	}
}

void GetAttack(int clientId, int mobId, p39D* p)
{
	memset(p, 0, sizeof p39D);

	p->Header.ClientId = 0x7531;
	p->attackerId = clientId;
	p->Header.TimeStamp = clock();

	p->attackerPos.X = Mob[clientId].Target.X;
	p->attackerPos.Y = Mob[clientId].Target.Y;
	
	p->targetPos.X = Mob[mobId].Target.X;
	p->targetPos.Y = Mob[mobId].Target.Y;

	//0041D27A
	p->Header.Size = sizeof p39D;
	p->Header.PacketId = 0x39D;

	p->doubleCritical = 0;
	p->currentMp = -1;
	p->currentExp = -1;

	p->Target.Damage = 0;

	//p->unknow_2 = -2;
	p->Target.Index = mobId;

	INT32 LOCAL_1 = -1;
	p->skillParm = 0;
	p->skillId = -1;

	p->Motion = Rand() % 3 + 4;

	if(clientId >= MAX_PLAYER)
	{
		INT32 LOCAL_2 = GetDistance(Mob[clientId].Target.X, Mob[clientId].Target.Y, Mob[mobId].Target.X, Mob[mobId].Target.Y);

		INT16 LOCAL_3;
		INT32 LOCAL_4;
		if(LOCAL_2 < 3)
		{
			LOCAL_3 = Mob[clientId].Mobs.Player.Status.Mastery[0];
			LOCAL_4 = Mob[clientId].Mobs.Player.Status.Mastery[1];
		}
		else
		{
			LOCAL_3 = Mob[clientId].Mobs.Player.Status.Mastery[2];
			LOCAL_4 = Mob[clientId].Mobs.Player.Status.Mastery[3];
		}

		p->Motion= 0;

		if((LOCAL_3 & 0xFF) == 0xFF)
			LOCAL_3 = 0xFF;

		// 0041D3B1
		//LOCAL_3 &= 0xFF;

		if(LOCAL_3 == 0x79)
		{
			p->skillId = 103;
			p->skillParm = 5;
		}
		else if(LOCAL_3 == 0x7A)
		{
			p->skillId = 0x69;
			p->skillParm = 1;
		}
		else if(LOCAL_3 == 0x7B)
		{
			p->skillId = 0x65;
			p->skillParm = 1;
		}
		else if(LOCAL_3 == 0x7C)
		{
			p->skillId = 0x65;
			p->skillParm = 2;
		}
		else if(LOCAL_3 == 0x7D)
		{
			p->skillId = 0x28;
			p->skillParm = 1;
		}
		else if(LOCAL_3 == 0x7E)
		{
			p->skillId = 0x28;
			p->skillParm = 2;
		}
		
		else if(LOCAL_3 == 0x7F)
		{
			p->skillId = 0x28;
			p->skillParm = 3;
		}
		else if(LOCAL_3 == 0x80)
		{
			p->skillId = 0x21;
			p->skillParm = 0xFC;
			p->Motion = 0xFC;
		}
		else
			p->skillId = LOCAL_3;

		if(LOCAL_4 != 0)
		{
			p->Motion = 4;

			INT32 LOCAL_5 = Rand() & 0x80000003;
			if(LOCAL_4 == 3)
			{
				if(LOCAL_5 <= 1)
					p->Motion++;
			}
			else if(LOCAL_4 == 6)
			{
				if(LOCAL_5 <= 1)
					p->Motion++;
				else
					p->Motion += 2;
			}
			else if(LOCAL_4 == 7)
			{
				if(LOCAL_5 == 1)
					p->Motion++;
				else
					p->Motion += 2;
			}
			else if(LOCAL_4 == 15)
			{
				if(LOCAL_5 == 1)
					p->Motion++;
				else if(LOCAL_5 == 2)
					p->Motion += 2;
				else if(LOCAL_5 == 3)
					p->Motion += 3;
			}
			else if(LOCAL_4 == 0x18)
			{
				if(LOCAL_5 <= 1)
					p->Motion += 3;
				else 
					p->Motion += 4;
			}
			else if(LOCAL_4 == 0x1B)
			{
				if(LOCAL_5 == 1)
					p->Motion ++;
				else if(LOCAL_5 == 2)
					p->Motion += 3;
				else if(LOCAL_5 == 3)
					p->Motion += 4;
			}
			else if(LOCAL_4 == 0x17)
			{
				if(LOCAL_5 == 1)
					p->Motion ++;
				else if(LOCAL_5 == 2)
					p->Motion += 2;
				else if(LOCAL_5 == 3)
					p->Motion += 4;
			}
		}

		INT32 LOCAL_6 = Rand() % 100;
		if(Mob[clientId].Mobs.Player.SkillBar1[3] != -1 && LOCAL_6 >= 0x19 && LOCAL_6 <= 40)
		{// skillData base = 7DCDD8
			INT32 LOCAL_7 = Mob[clientId].Mobs.Player.SkillBar1[3];
			INT32 LOCAL_8 = SkillData[LOCAL_7].InstanceType;
			INT32 LOCAL_9 = Mob[clientId].Leader;

			if(LOCAL_9 <= 0)
				LOCAL_9 = clientId;

			INT32 LOCAL_10 = Mob[clientId].Mobs.Player.Status.curHP;
			LOCAL_10 = LOCAL_10 * 10;
			LOCAL_10 = LOCAL_10 / Mob[clientId].Mobs.Player.Status.maxHP;
			INT32 LOCAL_11 = Mob[LOCAL_9].Mobs.Player.Status.curHP;
			LOCAL_11 = LOCAL_11 * 10;
			LOCAL_11 = LOCAL_11 / Mob[clientId].Mobs.Player.Status.maxHP;

			if(LOCAL_8 == 6 && LOCAL_10 <= 8 && LOCAL_11 <= 8)
			{
				p->skillId = LOCAL_7;

				INT32 LOCAL_12 = clientId;
				if(LOCAL_10 > LOCAL_11)
					LOCAL_12 = LOCAL_9;

				INT32 LOCAL_13 = Mob[LOCAL_12].Mobs.Player.Status.curHP / 10;

				p->Target.Index = LOCAL_12;
				p->Target.Damage = -LOCAL_13;

				return;
			}
		}
		
		if(Mob[clientId].Mobs.Player.SkillBar1[0] != -1 && LOCAL_6 >= 0 && LOCAL_6 <= 25)
		{
			INT32 LOCAL_14 = Mob[clientId].Mobs.Player.SkillBar1[0];
			INT32 LOCAL_15 = SkillData[LOCAL_14].InstanceType;

			LOCAL_1 = LOCAL_15 - 2;

			p->skillId = LOCAL_14;
		}
		else if(Mob[clientId].Mobs.Player.SkillBar1[1] != -1 && LOCAL_6 >= 26 && LOCAL_6 <= 50)
		{
			INT32 LOCAL_16 = Mob[clientId].Mobs.Player.SkillBar1[1];
			INT32 LOCAL_17 = SkillData[LOCAL_16].InstanceType;

			LOCAL_1 = LOCAL_17 - 2;

			p->skillId = LOCAL_16;
		}
		else if(Mob[clientId].Mobs.Player.SkillBar1[2] != -1 && LOCAL_6 >= 51 && LOCAL_6 <= 75)
		{
			INT32 LOCAL_16 = Mob[clientId].Mobs.Player.SkillBar1[2];
			INT32 LOCAL_17 = SkillData[LOCAL_16].InstanceType;

			LOCAL_1 = LOCAL_17 - 2;

			p->skillId = LOCAL_16;
		}
		else if(Mob[clientId].Mobs.Player.SkillBar1[3] != -1 && LOCAL_6 >= 51 && LOCAL_6 <= 75)
		{
			INT32 LOCAL_16 = Mob[clientId].Mobs.Player.SkillBar1[3];
			INT32 LOCAL_17 = SkillData[LOCAL_16].InstanceType;

			LOCAL_1 = LOCAL_17 - 2;

			p->skillId = LOCAL_16;
		}
	}

	INT32 LOCAL_20 = Mob[clientId].Mobs.Player.Status.Attack;

	LOCAL_20 = GetDamage(LOCAL_20, Mob[mobId].Mobs.Player.Status.Defense, 0);

	if(LOCAL_1 >= 0 && LOCAL_1 <= 3)
		LOCAL_20 = LOCAL_20 * (100 - *(BYTE*)(&Mob[mobId].Mobs.Player.Resist + LOCAL_1)) / 100;

	if(LOCAL_20 > 0 && mobId < MAX_PLAYER && Mob[mobId].ReflectDamage > 0)
	{
		LOCAL_20 -= Mob[mobId].ReflectDamage;

		if(LOCAL_20 < 0)
			LOCAL_20 = 0;
	}

	if(LOCAL_20 > 0)
	{
		if(clientId < MAX_PLAYER || Mob[clientId].Mobs.Player.CapeInfo == 4)
		{
			if(mobId < MAX_PLAYER || Mob[mobId].Mobs.Player.CapeInfo == 4)
			{
				LOCAL_20 = LOCAL_20 * 3 / 10;
				
				MapAttribute LOCAL_21 = GetAttribute(Mob[mobId].Target.X, Mob[mobId].Target.Y);
				MapAttribute LOCAL_22 = GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y);

				if(LOCAL_21.Village || !LOCAL_21.PvP)
					LOCAL_20 = 0;

				if(LOCAL_22.Village || !LOCAL_22.PvP)
					LOCAL_20 = 0;
			}
		}
		
		INT32 LOCAL_23 = Mob[clientId].Leader;
		if(LOCAL_23 == 0)
			LOCAL_23 = clientId;

		INT32 LOCAL_24= Mob[mobId].Leader;
		if(LOCAL_24 == 0)
			LOCAL_24 = mobId;

		INT32 LOCAL_25 = Mob[clientId].Mobs.Player.GuildIndex; 
		INT32 LOCAL_26 = Mob[mobId].Mobs.Player.GuildIndex;    // quem está sendo atacado
		INT32 ally = g_pGuildAlly[LOCAL_25];
		if(ally == 0)
			ally = -1;

		if(LOCAL_25 == 0 && LOCAL_26 == 0)
			LOCAL_25 = -1;

		if(mobId < MAX_PLAYER && Mob[mobId].GuildDisable)
			LOCAL_26 = -2;

		if (Mob[LOCAL_23].Target.X >= 1041 && Mob[LOCAL_23].Target.X <= 1248 &&
			Mob[LOCAL_23].Target.Y >= 1950 && Mob[LOCAL_23].Target.Y <= 2158 && sServer.RvR.Status == 1)
		{ // se estiver dentro da área
			if(Mob[LOCAL_23].Mobs.Player.CapeInfo == Mob[LOCAL_24].Mobs.Player.CapeInfo)
			{
				LOCAL_20 = 0;
				Mob[clientId].CurrentTarget = 0;
			}
		}

		if(LOCAL_23 == LOCAL_24 || LOCAL_25 == LOCAL_26 || ally == LOCAL_26)
		{
			LOCAL_20 = 0;
			Mob[clientId].CurrentTarget = 0;
		}
	}

	p->Target.Damage = LOCAL_20;
}

st_Item *GetItemPointer(int clientIndex, int invType, int invSlot)
{
	if(clientIndex <= 0 || clientIndex >= MAX_SPAWN_MOB)
		return NULL;

	switch(invType)
	{
		case 0:
			return &Mob[clientIndex].Mobs.Player.Equip[invSlot];
		case 1:
			return &Mob[clientIndex].Mobs.Player.Inventory[invSlot];
		case 2:
		{
			if (clientIndex < MAX_PLAYER)
				return &Users[clientIndex].User.Storage.Item[invSlot];

			return NULL;
		}
	}

	return NULL;
}

bool GetEmptyItemGrid(int *posX, int *posY)
{
    if(g_pItemGrid[*posY][*posX] == 0)
        if(g_pHeightGrid[*posY][*posX] != 127)
            return true;

    for(int nY = (*posY - 1); nY <= (*posY + 1); nY++)
    {
        for(int nX = (*posX - 1); nX <= (*posX + 1); nX++)
        {
            if(nX < 0 || nY < 0 || nX >= 4096 || nY >= 4096)
                continue;

            if(g_pItemGrid[nY][nX] == 0)
            {
                if(g_pHeightGrid[nY][nX] != 127)
                {
                    *posX = nX;
                    *posY = nY;
                    return true;
                }
            }
        }
    }

    return false;
}

void GetCreateItem(int Index, p26E *p)
{
	const stInitItem *init = &pInitItem[Index];

	p->Header.Size = sizeof p26E;
	p->Header.PacketId = 0x26E;
	p->Header.ClientId = 0x7530;

	p->Init.X = init->PosX;
	p->Init.Y = init->PosY;

	p->Index = (Index + 10000);

	memcpy(&p->Item, &init->Item, 8);

	p->Rotation = init->Rotation;
	p->Status = init->Status;
	p->Unknow = 0;
	p->HeightGrid = init->HeightGrid;
}
 
INT32 GetUserByName(char *name)
{
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(Users[i].Status != USER_PLAY)
			continue;

		if(!strcmp(Mob[i].Mobs.Player.Name, name))
			return i;
	}

	return 0;
}

INT32 GetEmptyItem()
{
	for(int i = 1; i < 4096; i++)
	{
		if(pInitItem[i].Open == 0)
			return i;
	}

	return 0;
}

//long long GetExpApplyMerida(long long exp, int ownerId, int meridaLevel, int targetId)
//{
//	meridaLevel += 300;
//	long long	multiexp = targetId * 100 / meridaLevel;
//	if (multiexp < 80 && meridaLevel >= 50)
//		multiexp = ((multiexp - 80) * 2) + 100;
//	else if (multiexp > 200)
//		multiexp = 200;
//	else if (multiexp < 0)
//		multiexp = 0;
//
//	exp = exp * multiexp / 100;
//	if (exp <= 0)
//		return 0;
//
//	if (meridaLevel >= 419)
//		exp /= 2;
//	if (meridaLevel >= 429)
//		exp /= 2;
//
//	if (Mob[ownerId].ExpBonus != 0)
//		exp = exp + (exp * static_cast<long long>(Mob[ownerId].ExpBonus) / 100);
//
//	if (sServer.KefraDead)
//		exp += (exp / 100ULL) * 50ULL;
//
//	return exp;
//}

long long GetExpApply(long long exp, int attackerId, int targetId)
{
	CMob *spwAttacker = &Mob[attackerId];
	CMob *spwTarget   = &Mob[ targetId ];
	
	long long attacker = spwAttacker->Mobs.Player.Status.Level;
	long long target = spwTarget->Mobs.Player.Status.Level;
	long long ev = spwAttacker->Mobs.Player.Equip[0].EFV2;
	if((ev <= ARCH && attacker >= MAX_LEVEL) || (ev >= CELESTIAL && attacker >= MAX_LEVEL_CELESTIAL))
		return 0;

	if(attacker >= 400 || target >= 400 || attacker < 0 || target < 0)
		return exp;

	attacker++;
	target++;

	if (ev >= CELESTIAL)
		attacker += (400 + (ev - ARCH) * 50);

	long long	multiexp = target * 100 / attacker;
	if(multiexp < 80 && attacker >= 50)
		multiexp = ((multiexp  - 80) * 2) + 100;
	else if(multiexp > 200)			
		multiexp = 200;
	else if(multiexp < 0)		
		multiexp = 0;

	exp = exp * multiexp / 100;
	if(exp <= 0)
		return 0;

	INT32 posX = spwAttacker->Target.X,
		  posY = spwAttacker->Target.Y;
	
	if(posX >= 1152 && posX <= 1282 && posY >= 130 && posY <= 217)
	{
		INT32 count = 0;
		for(INT32 x = 0; x < 8; x++)
		{
			if(sServer.Nightmare[2].Alive[x])
				continue;
			
			count ++;
		}		
		
		if(ev == MORTAL)
			exp += exp * 20ULL / 100ULL;
		else if(ev == ARCH)
			exp += exp * 15ULL / 100ULL;

		count = 8 - count;

		if(count != 0)
			exp -= static_cast<long long>((exp * (((count * 7) / 0.70f) / 100.0f)));
	}
	else if(posX >= 1049 && posX <= 1130 && posY >= 272 && posY <= 334)
	{
		INT32 count = 0;
		for(INT32 x = 0; x < 8; x++)
		{
			if(sServer.Nightmare[1].Alive[x])
				continue;
			
			count ++;
		}		
		
		if(ev == MORTAL)
			exp += exp * 20ULL / 100ULL;
		else if(ev == ARCH)
			exp += exp * 15ULL / 100ULL;

		count = 8 - count;

		if(count != 0)
			exp -= static_cast<long long>((exp * (((count * 7) / 0.70) / 100)));
	}

	if(spwAttacker->ExpBonus != 0)
		exp = exp + (exp * static_cast<long long>(spwAttacker->ExpBonus) / 100);

	if (sServer.KefraDead)
		exp += (exp / 100ULL) * 50ULL;

	return exp ;
}

long long GetExpApply_2(long long exp, int receiver, int attackerId, bool useBoxExp)
{
	if(exp == 0)
		return 0;
	
	long long level = Mob[receiver].Mobs.Player.Status.Level;
	TOD_Class classMaster = Mob[receiver].Mobs.Player.GetEvolution();

	auto checkIsBlocked = [](int receiver) -> bool {
		long long level = Mob[receiver].Mobs.Player.Status.Level;
		TOD_Class classMaster = Mob[receiver].Mobs.Player.GetEvolution();

		bool isBlocked = false;
		bool isBlockedAndNeedToCompleteExperience = false;
		if (Mob[receiver].Mobs.Info.LvBlocked)
			return (classMaster >= TOD_Class::Celestial && (level == 39 || level == 89 || level == 199 || level == 209)) || (classMaster == TOD_Class::Arch && (level == 354 || level == 369));

		return false;
	};


	if (checkIsBlocked(attackerId))
		return 0;

	bool isBlocked = checkIsBlocked(receiver);
	if (classMaster >= TOD_Class::Celestial && level >= MAX_LEVEL_CELESTIAL)
		return 0;

	// Somente batedores celestiais ou superior podem dar experiência para celestiais ou superior 200+
	auto attackerClassMaster = Mob[attackerId].Mobs.Player.GetEvolution();
	if (classMaster >= CELESTIAL && level >= 199 && attackerClassMaster < TOD_Class::Celestial)
		return 0;

	if (classMaster >= CELESTIAL && Mob[receiver].Mobs.GetTotalResets() < 3 && level == 199)
		return 0;

	if (isBlocked)
		return 0;

	if (classMaster == TOD_Class::Celestial)
		exp -= ((exp * (level + 100LL) / 10LL) / 100LL);
	else if (classMaster == TOD_Class::SubCelestial)
		exp -= ((exp * (level + 200LL) / 10LL) / 100LL);

	if (classMaster == TOD_Class::Mortal)
		exp += (exp * 100LL / 100LL);
	else if (classMaster == TOD_Class::Arch)
	{
		if (level <= 320)
			exp = exp;
		else if (level <= 354)
			exp /= 2LL;
		else
			exp /= 4LL;
	}
	else if (classMaster == TOD_Class::Celestial)
	{
		if (level < 39)
			exp = exp / 10LL;
		else if (level < 89)
			exp = exp / 20LL;
		else if (level < 170)
			exp = exp / 50LL;
		else if (level < 181)
			exp = exp / 200LL;
		else if(level < 199)
			exp = exp / 250LL;
		else if (level < 206)
			exp = exp / 450LL;
		else
			exp = exp / 900LL;

	}
	else if (classMaster == TOD_Class::SubCelestial)
	{
		if (level < 39)
			exp = exp / 25LL;
		else if (level < 89)
			exp = exp / 35LL;
		else if (level < 120)
			exp = exp / 50LL;
		else if (level < 170)
			exp = exp / 70LL;
		else if (level < 194)
			exp = exp / 200LL;
		else if (level < 199)
			exp = exp / 350LL;
		else if (level < 206)
			exp = exp / 450LL;
		else
			exp = exp / 900LL;
	}

	auto posX = Mob[receiver].Target.X;
	auto posY = Mob[receiver].Target.Y;
	if ((posX >= 1049 && posX <= 1130 && posY >= 272 && posY <= 334) ||
		(posX >= 1152 && posX <= 1282 && posY >= 130 && posY <= 217))
	{
		if (classMaster >= TOD_Class::Celestial && level >= 120)
			exp = exp * 60LL / 100LL;
	}

	if (useBoxExp)
	{
		for (INT32 i = 0; i < 32; i++)
		{
			if (Mob[attackerId].Mobs.Affects[i].Index == 39) // baú de experiência
			{
				exp *= 2LL;
				break;
			}
		}
	}

	if ((classMaster == TOD_Class::Celestial && Mob[receiver].Mobs.Player.bStatus.Level >= 64) || classMaster >= TOD_Class::SubCelestial)
	{
		for (int t = 0; t < 9; t++)
		{
			if (Mob[receiver].Target.X >= waterMaxMin[1][t][0] && Mob[receiver].Target.X <= waterMaxMin[1][t][2] && Mob[receiver].Target.Y >= waterMaxMin[1][t][1] && Mob[receiver].Target.Y <= waterMaxMin[1][t][3])
			{
				exp /= 4LL;

				break;
			}
		}
	}

	if (classMaster >= TOD_Class::Celestial && Mob[receiver].Mobs.Player.bStatus.Level >= 199)
	{
		for (int t = 0; t < 9; t++)
		{
			if (Mob[receiver].Target.X >= waterMaxMin[2][t][0] && Mob[receiver].Target.X <= waterMaxMin[2][t][2] && Mob[receiver].Target.Y >= waterMaxMin[2][t][1] && Mob[receiver].Target.Y <= waterMaxMin[2][t][3])
			{
				exp /= 4LL;

				break;
			}
		}
	}

	bool onNightmareArea{ false };
	for (int i = 0; i < 3; ++i)
	{
		if (Mob[attackerId].Target.X >= g_pPesaArea[i][0] && Mob[attackerId].Target.X <= g_pPesaArea[i][2] && Mob[attackerId].Target.Y >= g_pPesaArea[i][1] && Mob[attackerId].Target.Y <= g_pPesaArea[i][3])
		{
			onNightmareArea = true;

			break;
		}
	}

	if (!onNightmareArea && sServer.BonusEXP != 0)
		exp += (exp * sServer.BonusEXP / 100LL);

	st_Position userPosition{ static_cast<unsigned short>(Mob[attackerId].Target.X), static_cast<unsigned short>(Mob[attackerId].Target.Y) };
	for (const auto& bonusArea : sServer.BonusExpArea)
	{
		if (userPosition >= bonusArea.MinPosition && userPosition <= bonusArea.MaxPosition)
		{
			exp += (static_cast<unsigned long long>(bonusArea.Value) * exp / 100LL);

			break;
		}
	}

	exp += GetFairyExpBonus(exp, Mob[attackerId].Mobs.Player.Equip[13].Index);

	if (Mob[receiver].IndividualExpBonus > 0 && Mob[receiver].IndividualExpBonus < 100)
		exp += (exp * static_cast<long long>(Mob[receiver].IndividualExpBonus) / 100LL);

	return exp;
}

unsigned long long GetFairyExpBonus(unsigned long long exp, INT32 fadaId)
{
	if (fadaId == 3900 || fadaId == 3906 || fadaId == 3914)
		exp = (exp * 16 / 100);
	else if (fadaId == 3915)
		exp = exp * 18 / 100;
	else if(fadaId == 3902 || fadaId == 3908)
		exp = (exp * 32 / 100);
	else
		exp = 0;

	return exp;
}

INT32 GetManaSpent(INT32 skillId, INT32 saveMana, INT32 mastery)
{
	INT32 manaSpent = SkillData[skillId].Mana; // -4

	manaSpent = (((mastery >> 1) + 100) * manaSpent) / 100;
	manaSpent = ((100 - saveMana) * manaSpent) / 100;

	return manaSpent;
}

INT32 GetDoubleCritical(CMob *arg1, short *arg2, short *arg3, unsigned char *arg4)
{
	*arg4 = 0;

	if (arg3 == 0)
		return 0;

	if (*arg3 >= 1024)
		*arg3 = *arg3 & 0x800003FF;

	if (arg2 != 0 && *arg2 >= 1024)
		*arg2 = *arg2 & 0x800003FF;

	INT32 LOCAL_1 = 1;
	INT32 LOCAL_3[2] = { (((arg1->AttackSpeed >> 4) - 5) * 100), arg1->Mobs.Player.Critical };
	INT32 LOCAL_4;
	if (arg2 != 0 && arg3 != 0 && *arg3 != *arg2)
	{
		*arg2 = *arg3;
		LOCAL_4 = *arg3;

		if (LOCAL_4 < *arg2)
			LOCAL_4 += 256;

		if (LOCAL_4 < *arg2 && LOCAL_4 < *arg2 + 5)
			LOCAL_1 = 1;
	}

	INT32 LOCAL_5 = g_pHitRate[*arg3];
	for (INT32 LOCAL_6 = 0; LOCAL_6 < 2; LOCAL_6++)
	{
		INT32 LOCAL_7 = 0;
		if (LOCAL_6 == 0 && LOCAL_5 < LOCAL_3[LOCAL_6])
			LOCAL_7 = 1;

		if (LOCAL_6 == 1)
		{
			if ((Rand() % 250) < LOCAL_3[LOCAL_6])
				LOCAL_7 = 1;
		}

		*arg4 = *arg4 | (LOCAL_7 << LOCAL_6);
	}

	if (arg2 != 0)
		*arg2 = *arg2 + 1;

	*arg3 = *arg3 + 1;

	return LOCAL_1;
}
INT32 GetItemAmount(st_Item *item)
{
	INT32 amount = 1;
	
	for(INT8 i = 0; i < 3; i++)
	{
		if(item->Effect[i].Index == EF_AMOUNT)
		{
			amount = item->Effect[i].Value;
			if(amount == 0)
				amount = 1;

			break;
		}
	}

	return amount;
}

int GetUserInArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char* first)
{ //401348
	INT32 LOCAL_1 = 0,
		  LOCAL_2 = 1;

	for(; LOCAL_2 < MAX_PLAYER; LOCAL_2 ++)
	{
		if(Users[LOCAL_2].Status != USER_PLAY)
			continue;

		if(Mob[LOCAL_2].Mode == 0)
			continue;

		if(Mob[LOCAL_2].Target.X < x1 || Mob[LOCAL_2].Target.X > x2 || Mob[LOCAL_2].Target.Y < y1 || Mob[LOCAL_2].Target.Y > y2)
			continue;

		if(LOCAL_1 == 0)
			strncpy_s(first, 16, Mob[LOCAL_2].Mobs.Player.Name, 12);

		LOCAL_1++;
	}

	return LOCAL_1;
}

INT32 GetInventoryAmount(int clientId, int itemId)
{
	st_Item *item = Mob[clientId].Mobs.Player.Inventory;
	
	INT32 total = 0;
	for(INT32 i = 0; i < 30; i++)
	{
		if(item[i].Index == itemId)
			total += GetItemAmount(&item[i]);
	}
	
	double bolsa1 = TimeRemaining(item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
	double bolsa2 = TimeRemaining(item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);

	if(bolsa1 > 0.0f)
	{
		for(INT32 i = 30; i < 45; i++)
		{
			if(item[i].Index == itemId)
				total += GetItemAmount(&item[i]);
		}
	}

	if(bolsa2 > 0.0f)
	{
		for(INT32 i = 45; i < 60; i++)
		{
			if(item[i].Index == itemId)
				total += GetItemAmount(&item[i]);
		}
	}

	return total;
}

void GetGuild(int clientId)
{
	st_Item *LOCAL_1 = &Mob[clientId].Mobs.Player.Equip[12];

/*	if(Mob[clientId].Mobs.MedalId == 509)
		Mob[clientId].Mobs.Player.GuildMemberType = 9;
	else if(Mob[clientId].Mobs.MedalId >= 526 && Mob[clientId].Mobs.MedalId <= 531)
		Mob[clientId].Mobs.Player.GuildMemberType = (3 + Mob[clientId].Mobs.MedalId - 526);
	else if(Mob[clientId].Mobs.MedalId == 508)
		Mob[clientId].Mobs.Player.GuildMemberType = 1;
	else 
		Mob[clientId].Mobs.Player.GuildMemberType = 0;*/
}

unsigned int GetWeekNumber()
{
	time_t now;
	time(&now);

	unsigned int week = 86400;

	__int64 ret = static_cast<unsigned int>(now) / week - 3;

	return static_cast<unsigned int>(ret);
}

UINT8 DayOfWeek()
{
	// 0 = segunda .... 6 = domingo
	static const int szTable[12] = {-1, 2, 2, 5, 0, 3, 5, 1, 4, -1, 2, 4};
    time_t rawnow = time(NULL);
    struct tm now; localtime_s(&now, &rawnow);
	int cDay = now.tm_mday;
	int cMon = now.tm_mon;
	int cYear = now.tm_year;

	return (cYear + ((cYear / 4) + szTable[cMon] + (cDay - 1))) % 7;
}

INT32 GetInHalf(INT32 clientId, INT32 mobId)
{ // 401159
	if (Mob[mobId].Target.X - HALFGRIDX > Mob[clientId].Target.X)
		return false;

	if (Mob[mobId].Target.X + HALFGRIDX < Mob[clientId].Target.X)
		return false;

	if (Mob[mobId].Target.Y - HALFGRIDY > Mob[clientId].Target.Y)
		return false;

	if (Mob[mobId].Target.Y + HALFGRIDY < Mob[clientId].Target.Y)
		return false;

	return true;
}

INT32 GetBonusSet(st_Mob *player, INT32 defense)
{
	INT32 start = -1;
	st_Item *item;
	if(player->Equip[0].EFV2 >= 2) 
	{
		start = 3;
		item = (&player->Equip[2]);
	}
	else
	{
		start = 2;
		item = &player->Equip[1];
	}

	// O primeiro item já não existe, logo, não há porque aplicar o bônus
	if(item->Index <= 0 || item->Index >= MAX_ITEMLIST)
		return 0;

	INT32 mobType = GetEffectValueByIndex(item->Index, EF_MOBTYPE),
		  _class  = GetEffectValueByIndex(item->Index, EF_CLASS),
		  letter  = GetEffectValueByIndex(item->Index, EF_UNKNOW1);

	INT32 i;
	for(i = start; i < 6; i++)
	{
		st_Item *tmpItem = &player->Equip[i];
		if(tmpItem->Index <= 0 || tmpItem->Index >= 6500)
			break;

		if(mobType != GetEffectValueByIndex(tmpItem->Index, EF_MOBTYPE) ||
			_class != GetEffectValueByIndex(tmpItem->Index, EF_CLASS) ||
			letter != GetEffectValueByIndex(tmpItem->Index, EF_UNKNOW1))
			break;
	}

	if(i == 6)
		return (defense * 5 / 100);

	return 0;
}

int GetStaticItemAbility(st_Item *item, unsigned char Type)
{
	int value = 0;
	int idx = item->Index;

	if(idx <= 0 || idx > MAX_ITEMLIST)
		return value;

	int nPos = ItemList[idx].Pos;

	if(Type == EF_LEVEL && idx >= 2330 && idx < 2360)
		value = item->Effect[1].Index - 1;
	else if(Type == EF_LEVEL)
		value += ItemList[idx].Level;

	if(Type == EF_REQ_STR)
		value += ItemList[idx].Str;

	if(Type == EF_REQ_INT)
		value += ItemList[idx].Int;

	if(Type == EF_REQ_DEX)
		value += ItemList[idx].Dex;

	if(Type == EF_REQ_CON)
		value += ItemList[idx].Con;

	if(Type == EF_POS)
		value += ItemList[idx].Pos;

	if(Type != EF_INCUBATE)
	{
		for(int i = 0; i < 12; i++)
		{
			if(ItemList[idx].Effect[i].Index != Type)
                		continue;

			int tvalue = ItemList[idx].Effect[i].Value;

            if(Type == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
		}
	}

	if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
	{
		for(int i = 0; i < 12; i++)
		{
			if(ItemList[idx].Effect[i].Index == EF_RESISTALL)
				value += ItemList[idx].Effect[i].Value;
		}

		for(int i = 0; i < 3; i++)
		{
			if(item->Effect[i].Index == EF_RESISTALL)
				value += item->Effect[i].Value;
		}
	}

	if(idx >= 2330 && idx < 2390)
	{
		if(Type == EF_MOUNTHP)
			return item->Effect[0].Value;

		else if(Type == EF_MOUNTSANC)
			return item->Effect[1].Index;

		else if(Type == EF_MOUNTLIFE)
			return item->Effect[1].Value;

		else if(Type == EF_MOUNTFEED)
			return item->Effect[2].Index;

		else if(Type == EF_MOUNTKILL)
			return item->Effect[2].Value;

		if(idx < 2362 || idx >= 2390 || item->Effect[0].Value <= 0)
			return value;

		int lv = item->Effect[1].Index;
		int cd = item->Index - 2360;

		if(Type == EF_DAMAGE)
			return (lv + 20) * mMont[cd].atkFisico / 100; //Retorna o DN da montaria no level atual

		else if(Type == EF_MAGIC)
			return (lv + 15) * mMont[cd].atkMagico / 100;

		else if(Type == EF_PARRY)
			return mMont[cd].Evasion;

		else if(Type == EF_RESIST1 || Type == EF_RESIST2 || Type == EF_RESIST3 || Type == EF_RESIST4)
			return mMont[cd].Resist;
		else
			return value;
	}

	int sanc = GetItemSanc(item);

	if(sanc == 9 && (nPos & 0xF00) != 0)
		sanc = 10;

	if(Type != EF_GRID && Type != EF_CLASS && Type != EF_POS && Type != EF_WTYPE && Type != EF_RANGE && Type != EF_LEVEL && 
		Type != EF_REQ_STR && Type != EF_REQ_INT && Type != EF_REQ_DEX && Type != EF_REQ_CON && Type != EF_INCUBATE && Type != EF_INCUDELAY)
	{
		value *= sanc + 10;
		value /= 10;
	}

	if(Type == EF_RUNSPEED)
	{
		if(value >= 3)
			value = 2;

		if(value > 0 && sanc == 9)
			value++;
	}

	if(Type == EF_HWORDGUILD || Type == EF_LWORDGUILD)
	{
		unsigned char v = value;

		value = v;
	}

	return value;
}

INT32 GetCompounderDeal(INT32 clientId)
{
	INT32 slotId = GetFirstSlot(clientId, 4529);
	if(slotId == -1)
		return 0;

	AmountMinus(&Mob[clientId].Mobs.Player.Inventory[slotId]);
	SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);

	INT32 value = ((Rand() % 5) + 15);
	Log(clientId, LOG_INGAME, "Utilizado o item Escritura de Composição - %d", value);
	return value;
}

INT32 getWeek(INT32 day, INT32 month, INT32 year) 
{
	time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);
	
	char buffer[16];
	strftime (buffer,4,"%W", &now);   // '%W' = week number of the year, eg 1/1/09 == 1
	
	return (atoi(buffer));
}

std::tuple<int, int> GetPriceAndImpost(sItemData* item, int city)
{
	int itemPrice = item->Price;
	int perc_impost = g_pCityZone[city].perc_impost;
	int impost = 0;
	if (perc_impost != 0)
		impost = (itemPrice * perc_impost / 100);

	return std::make_tuple(itemPrice, impost);
}

int GetOverPower(const CMob& mob) 
{
	const stCharInfo& charInfo = mob.Mobs;
	const st_Mob& player = charInfo.Player;
	int points = 0;
	int evolution = player.Equip[0].Effect[1].Value;

	int level = player.Status.Level;
	if (evolution >= CELESTIAL)
		level += 400;

	points = level * 2;

	if (evolution >= ARCH)
	{
		if (charInfo.Info.Elime)
			points += 100;

		if (charInfo.Info.Thelion)
			points += 100;

		if (charInfo.Info.Noas)
			points += 100;

		if (charInfo.Info.Sylphed)
			points += 100;
	}

	for (int i = 1; i < 18; i++)//alterado
	{
		const st_Item& item = player.Equip[i];
		if (item.Index <= 0 || item.Index >= MAX_ITEMLIST)
			continue;

		int itemType = GetItemAbility(const_cast<st_Item*>(&item), EF_MOBTYPE);

		// verifica se é um item arch
		if (itemType == 1)
			points += 30;

		if (i != 14)
		{
			int sanc = GetItemSanc(const_cast<st_Item*>(&item));
			points += (sanc * 5);
		}
		else
		{
			int mountLevel = item.Effect[1].Index;
			if (mountLevel == 120)
				points += 100;
				
			if (mountLevel > 120)
				points += ((mountLevel - 120) * 15);

			points += (mountLevel * 2);
		}
	}

	return points;
}

int GetDamageByJewel(int clientId, int damage)
{
	if (clientId < MAX_PLAYER)
		return damage;

	INT32 itemId = Mob[clientId].Mobs.Player.Equip[13].Index;
	if (itemId == 786 || itemId == 1936 || itemId == 1937)
	{
		INT32 sanc = GetItemSanc(&Mob[clientId].Mobs.Player.Equip[13]); // local209
		if (sanc < 2)
			sanc = 2;

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

		multHP *= sanc;
		damage /= multHP;
	}

	return damage;
}

int ReturnChance(st_Item *item)
{
	constexpr std::array refs{ 2, 3, 4, 5, 6, 7 , 8, 10, 12, 15 };
	if (item->EF1 == 43)
		return refs[item->EFV1];
	else if (item->EF2 == 43)
		return refs[item->EFV2];
	else if (item->EF3 == 43)
		return refs[item->EFV3];

	return 2;
}

int GetMaxAmountItem(const st_Item* item)
{
	switch (item->Index)
	{
	case 412:
	case 413:
	case 3200:
	case 3201:
	case 3202:
	case 3203:
	case 3204:
	case 3205:
	case 3206:
	case 3207:
	case 3208:
	case 3214:
	case 3209:
	case 2390:
	case 2391:
	case 2392:
	case 2393:
	case 2394:
	case 2395:
	case 2396:
	case 2397:
	case 2398:
	case 2399:
	case 2400:
	case 2401:
	case 2402:
	case 2403:
	case 2404:
	case 2405:
	case 2406:
	case 2407:
	case 2408:
	case 2409:
	case 2410:
	case 2411:
	case 2412:
	case 2413:
	case 2414:
	case 2415:
	case 2416:
	case 2417:
	case 2418:
	case 2419:
	case 415:
	case 419:
	case 420:
	case 4599:
	case 4601:
	case 4603:
	case 4547:
	case 4528:
	case 1739:
	case 4641:
	case 4719:
		return 120;

	case 4140:
	case 4548:
	case 4549:
	case 3314:
	case 4016:
	case 4017:
	case 4018:
	case 4019:
	case 4020:
	case 777:
	case 3182:
	case 3252:
	case 3253:
		return 120;

	case 4850:
		return 150;
	}

	return 0;
}

bool IsWarTime()
{
	return (sServer.TowerWar.Status || sServer.WeekMode == 3 || sServer.ForceWeekDay == 3 || sServer.CastleState != 0);

}

int GetSpiritHPMPBonus(int sanc)
{
	if (sanc == 0)
		sanc = 1;

	if (sanc > 9)
		sanc = 9;

	return 10 * sanc;
}

int GetSpiritRessBonus(int sanc)
{
	if (sanc == 0)
		sanc = 1;

	if (sanc > 9)
		sanc = 9;

	int total = 10 * sanc;
	if (total > 60)
		total = 60;

	return total;
}

bool IsCostume(const st_Item* item)
{
	return (item->Index >= 4151 && item->Index <= 4189) || (item->Index >= 4210 && item->Index <= 4229) || (item->Index >= 4230 && item->Index <= 4247);
}

std::vector<CUser*> GetSameMACUsers(const CUser& thisUser, std::function<bool(CUser& user)> function)
{
	std::vector<CUser*> users;
	for (auto& user : Users)
	{
		if (&thisUser == &user || user.Status != USER_PLAY || memcmp(user.MacAddress, thisUser.MacAddress, 8) != 0 || (function != nullptr && !function(user)))
			continue;

		users.push_back(&user);
	}

	return users;
}

TOD_Valley GetValleyWithMinimum()
{
	std::array<int, 2> total{ 0, 0 };

	for (const auto& user : Users)
	{
		if (user.Status != USER_PLAY)
			continue;

		auto& mob = Mob[user.clientId];
		if (mob.Target.X >= 2176 && mob.Target.X <= 2304 && mob.Target.Y >= 3585 && mob.Target.Y <= 3711)
			total[0]++;
		else if (mob.Target.X >= 2176 && mob.Target.X <= 2304 && mob.Target.Y >= 3712 && mob.Target.Y <= 3839)
			total[1]++;
	}

	return total[0] > total[1] ? TOD_Valley::First : TOD_Valley::Second;
}

std::string GetEvolutionName(TOD_Class evolution)
{
    constexpr std::array evolutionName = { "Unknown", "Mortal", "Arch", "Celestial", "Subcelestial" };

    if (evolution > TOD_Class::SubCelestial || evolution < TOD_Class::Mortal)
        return evolutionName[0];

    return evolutionName[evolution];
}