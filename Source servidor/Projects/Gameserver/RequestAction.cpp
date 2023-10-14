#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <sstream>

bool CUser::RequestAction(PacketHeader *Header)
{ // 0042351A
	p36C *p = (p36C*)(Header);
	
	if(Users[clientId].Status != USER_PLAY)
	{
		SendHpMode(clientId);

		return true;
	}

	if(Mob[clientId].Mobs.Player.Status.curHP == 0)
	{
		SendHpMode(clientId);

		Log(clientId, LOG_INGAME, "Tentativa de mover enquanto morto. Posição: %ux %uy", Mob[clientId].Target.X, Mob[clientId].Target.Y);
		return true;
	}

	if (Users[clientId].Trade.ClientId != 0)
	{
		RemoveTrade(Users[clientId].Trade.ClientId);
		RemoveTrade(clientId);
	}

	UINT32 moveTime = p->Header.TimeStamp, // local38
		  checkTime; // local39

	if(CurrentTime >= 120000)
		checkTime = CurrentTime - 120000;
	else
		checkTime = 0;

	Mob[clientId].Motion = 0;
	
	if(p->Header.PacketId == 0x368)
	{
		if(Mob[clientId].Mobs.Player.ClassInfo != 3 || !(Mob[clientId].Mobs.Player.Learn[0] & 2))
		{
			if(Mob[clientId].Mobs.Player.bStatus.Level < 400)
				if(AddCrackError(clientId, 10, 28))
					return true;
		}

		INT32 mp = SkillData[73].Mana; // local40

		if(Mob[clientId].Mobs.Player.Status.curMP < mp)
		{
			SendSetHpMp(clientId);

			return true;
		}

		Mob[clientId].Mobs.Player.Status.curMP -= mp;
		Users[clientId].Potion.CountMp -= mp;
		/*
		if(Users[clientId].IllusionTime != 0xE0A1ACA && moveTime < Users[clientId].IllusionTime + 900)
		{
			Log(clientId, LOG_ERROR, "err,illusion %d %d 900 ms limit", moveTime, Users[clientId].IllusionTime);

			AddCrackError(clientId, 1, 105);
			return true;
		}
		*/
		if(moveTime > CurrentTime + 15000 || moveTime < checkTime)
		{
			Log(clientId, LOG_ERROR, "err,illusion %d %d 15000 ms limit", moveTime, CurrentTime);

			AddCrackError(clientId, 1, 104);
			return true;
		}

		Users[clientId].IllusionTime = moveTime;
	}

	else if(p->Header.PacketId == 0x36C)
	{
		if(Users[clientId].Movement.TimeStamp != 0xE0A1ACA && moveTime < Users[clientId].Movement.TimeStamp + 900)
		{
			Log(clientId, LOG_ERROR, "err,action %d %d 900 ms limit", moveTime, Users[clientId].Movement.TimeStamp);

			AddCrackError(clientId, 1, 103);
			return true;
		}
		
		if(moveTime > CurrentTime + 15000 || moveTime < checkTime)
		{
			Log(clientId, LOG_ERROR, "err,action %d %d 15000 ms limit", moveTime, CurrentTime);

			AddCrackError(clientId, 1, 102);
			return true;
		}

		Users[clientId].Movement.TimeStamp = moveTime;
	}
	else if(p->Header.PacketId == 0x366)
	{
		if (Users[clientId].Movement.PacketId == 0x366)
		{
			Log(clientId, LOG_ERROR, "err, can't send MSG_Stop continuosly");

			AddCrackError(clientId, 1, 101);
			return true;
		}
	}

	if (p->MoveSpeed > static_cast<unsigned int>(Mob[clientId].Mobs.Player.Status.Move.Speed))
	{
		// Retira o CrackError já que o MoveSpeed correto é setado novamente abaixo.
		//AddCrackError(clientId, 5, 4);

		std::stringstream str;
		str << "Informações do pacote:\n";
		str << "MoveType: " << p->MoveType << "\n";
		str << "MoveSpeed: " << p->MoveSpeed << "\n";
		str << "DestinyX: " << p->Destiny.X << "\n";
		str << "DestinyY: " << p->Destiny.Y << "\n";
		str << "DestinyX: " << p->LastPos.X << "\n";
		str << "DestinyY: " << p->LastPos.Y << "\n";

		str << "etc,diffrent movement. Speed que era para ser: " << (Mob[clientId].Mobs.Player.Status.Move.Value & 15);

		Log(clientId, LOG_ERROR, str.str().c_str());
		p->MoveSpeed = Mob[clientId].Mobs.Player.Status.Move.Speed; 
	}

	//00423AB3
	INT32 tgtX = Mob[clientId].Target.X; // local41
	INT32 tgtY = Mob[clientId].Target.Y; // local42

	// Aqui há uma checagem do tipo de movimento
	// Quando é 1 ou 2, naturalmente a TMsrv não faz a checagem de distância. Ridículo, não? :)
	if (p->Destiny.X < tgtX - VIEWGRIDX || p->Destiny.X > tgtX + VIEWGRIDX || p->Destiny.Y < tgtY - VIEWGRIDY || p->Destiny.Y > tgtY + VIEWGRIDY)
	{
		if (p->Destiny.X < tgtX - (VIEWGRIDX * 2) || p->Destiny.X > tgtX + VIEWGRIDX * 2 || p->Destiny.Y < tgtY - VIEWGRIDY * 2 || p->Destiny.Y > tgtY + VIEWGRIDY * 2)
		{
			p36C sm; // local55
			memset(&sm, 0, sizeof p36C);
			
			GetAction(clientId, tgtX, tgtY, &sm);
			sm.Header.PacketId = 0x368;
			sm.MoveSpeed = 6;

			Users[clientId].SendOneMessage((BYTE*)&sm, sizeof p36C);
		}

		AddCrackError(clientId, 1, 5);
		return true;
	}

	if(tgtX >= 3329 && tgtX <= 3452 && tgtY >= 1408 && tgtY <= 1462 && !IsAdmin)
	{
		stPista *pista = &pPista[2];
		if(!pista->Status)
		{
			DoRecall(clientId);

			return true;
		}

		if(pista->inSec.Born)
		{
			INT32 dist = GetDistance(tgtX, tgtY, Mob[pista->inSec.BossID].Target.X, Mob[pista->inSec.BossID].Target.Y);
			if(dist <= 14)
			{
				INT32 x;
				for(x = 0; x < 3; x++)
				{
					if(pista->Clients[x][12] == clientId)
						break;
				}

				if(x == 3)
				{
					Teleportar(clientId, 3418, 1451);
				
					return true;
				}
			}
		}
	}

	if (tgtX >= 2447 && tgtX <= 2545 && tgtY >= 1857 && tgtY <= 1919)
	{
		time_t rawnow = time(NULL);
		struct tm now; localtime_s(&now, &rawnow);
		/*
		if (now.tm_hour == 23 && now.tm_wday != 0 && now.tm_wday != 6)
		{
			// Caso a guerra esteja recusada somente os membros do canal podem subir 
			// Caso a guerra esteja no outro canal somente os membros do canal podem subir
			if (Mob[clientId].Mobs.Citizen != sServer.Channel && (!sServer.TowerWar[sServer.Channel - 1].WarState || sServer.TowerWar[sServer.Channel - 1].WarState == 2) && !IsAdmin)
			{
				DoRecall(clientId);

				return true;
			}
		}*/
	}

	int cape = Mob[clientId].Mobs.Player.CapeInfo;
	/*
	// RvR
	if (p->Destiny.X >= 1710 && p->Destiny.X <= 1715 && p->Destiny.Y >= 1969 && tgtY <= p->Destiny.Y && sServer.RvR.Status == 1)
	{
		if (cape != CAPE_RED)
		{
			p36C sm; // local55
			memset(&sm, 0, sizeof p36C);
			
			GetAction(clientId, tgtX, tgtY, &sm);
			sm.Header.PacketId = 0x368;
			sm.MoveSpeed = p->MoveSpeed;

			Users[clientId].SendOneMessage((BYTE*)&sm, sizeof p36C);
			return true;
		}
	}
	else if (p->Destiny.X >= 1748 && p->Destiny.X <= 1754 && p->Destiny.Y >= 1969 && p->Destiny.Y <= 1994 && sServer.RvR.Status == 1)
	{
		if (cape != CAPE_BLUE)
		{
			p36C sm; // local55
			memset(&sm, 0, sizeof p36C);
			
			GetAction(clientId, tgtX, tgtY, &sm);
			sm.Header.PacketId = 0x368;
			sm.MoveSpeed = p->MoveSpeed;

			Users[clientId].SendOneMessage((BYTE*)&sm, sizeof p36C);
			return true;
		}
	}*/

	if(p->Destiny.X <= 0 || p->Destiny.X >= 4096 || p->Destiny.Y <= 0 || p->Destiny.Y >= 4096)
	{
		Log(clientId, LOG_ERROR, "err,action - viewgrid %s", Mob[clientId].Mobs.Player.Name);

		return true;
	}

	if (p->Destiny.X != Mob[clientId].Target.X || p->Destiny.Y != Mob[clientId].Target.Y)
	{
		// 00423CEC
		MapAttribute attribute = GetAttribute(p->Destiny.X, p->Destiny.Y); // local56
		if ((attribute.Newbie && static_cast<int>(Mob[clientId].Mobs.Player.Status.Level) > static_cast<int>(sServer.NewbieZone) && Mob[clientId].Mobs.Player.Status.Level < 1000) && !Users[clientId].IsAdmin)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Newbie_zone]);

			DoRecall(clientId);
			return true;
		}

		if (attribute.Newbie && Mob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL && !Users[clientId].IsAdmin)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Newbie_zone]);

			DoRecall(clientId);
			return true;
		}

		if (attribute.Guild && Mob[clientId].Mobs.Player.Status.Level < 400 && !Users[clientId].IsAdmin)
		{
			INT32 village = GetVillage(p->Destiny.X, p->Destiny.Y); // local57

			if (village >= 0 && village < 5)
			{
				if (Mob[clientId].Mobs.Player.GuildIndex != ChargedGuildList[sServer.Channel - 1][village])
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Members]);

					DoRecall(clientId);
					return true;
				}
			}
		}

		Users[clientId].Movement.PacketId = p->Header.PacketId;
		INT32 LOCAL_58 = 0, // ?
			posX = p->Destiny.X,
			posY = p->Destiny.Y;

		char pCmd[256];
		strncpy_s(pCmd, p->Command, 24);

		if (g_pMobGrid[posY][posX] != 0 && g_pMobGrid[posY][posX] != clientId)
		{
			unsigned int tmpPosX = p->Destiny.X;
			unsigned int tmpPosY = p->Destiny.Y;
			size_t strsize = strlen(p->Command);

			int j;
			
			for (j = strsize; j >= 0; j--)
			{
				GetRoute(p->LastPos.X, p->LastPos.Y, &tmpPosX, &tmpPosY, p->Command, j); // (char*)g_pHeightGrid);
				if (g_pMobGrid[p->Destiny.Y][p->Destiny.X] != 0 && g_pMobGrid[p->Destiny.Y][p->Destiny.X] != clientId)
					continue;

				break;
			}

			if (j == -1)
			{
				p->Destiny.X = Mob[clientId].Target.X;
				p->Destiny.Y = Mob[clientId].Target.Y;

				p->Command[0] = 0;

				AddMessage((BYTE*)p, sizeof p);
				return true;
			}

			p->Destiny.X = tmpPosX;
			p->Destiny.Y = tmpPosY;

			AddMessage((BYTE*)p, sizeof p);
		}

		memcpy(Mob[clientId].Route, p->Command, 24);

		GridMulticast(clientId, posX, posY, (BYTE*)p);

		if (p->Header.PacketId == 0x368)
		{
			AddMessage((BYTE*)p, sizeof p36C);
			SendSetHpMp(clientId);
		}
		// 004241BB
		INT32 routeSize = strlen((char*)Mob[clientId].Route); // local129
		BYTE LOCAL_130 = 0; // EBP - 208h ?? - direction? 

		if (routeSize > 0 && routeSize < 24)
		{
			LOCAL_130 = Mob[clientId].Route[routeSize - 1];
			LOCAL_130 = LOCAL_130 - 0x30;

			if (LOCAL_130 >= 1 && LOCAL_130 <= 9)
				LOCAL_130 = LOCAL_130 << 4;
			else
				LOCAL_130 = 0;
		}

		Mob[clientId].Mobs.Player.Status.Merchant.Value = Mob[clientId].Mobs.Player.Status.Merchant.Value | LOCAL_130 & 0xF0;
		Mob[clientId].Mobs.Player.bStatus.Merchant.Value = Mob[clientId].Mobs.Player.Status.Merchant.Value;

		//if (sServer.BRState && sServer.BRItem > 0)
		//{
		//	INT32 tX = Mob[clientId].Target.X;
		//	INT32 tY = Mob[clientId].Target.Y;
		//	INT32 level = Mob[clientId].Mobs.Player.Status.Level;

		//	if (tX >= 2604 && tY >= 1708 && tX <= 2648 && tY <= 1744)
		//	{
		//		if (!sServer.BRGrid && level >= 100 && level < 1000)
		//			DoRecall(clientId);

		//		if (sServer.BRGrid == 1 && level >= 200 && level < 1000)
		//			DoRecall(clientId);
		//	}
		//}

		if (sServer.Colo150Limit)
		{
			INT32 level = Mob[clientId].Mobs.Player.Status.Level;
			if (level >= 150)
			{
				INT32 tX = Mob[clientId].Target.X;
				INT32 tY = Mob[clientId].Target.Y;

				if (tX >= 2604 && tY >= 1708 && tX <= 2648 && tY <= 1744)
					DoRecall(clientId);
			}
		}
	}

//#if defined(_DEBUG)
//	printf("Pacote recebido. Posicao: %hux %huy\n", p->Destiny.X, p->Destiny.Y);
//#endif

	return true;
}


// CMOB -> 1FDECA0
// CUSER-> 752BAD8
//	int32_t PosX; // 1FDF1F4
// int32_t PosY; // 01FDF1F8
//local198 = buffer
// TMSRV 756
bool RequestAction2(PacketHeader *Header)
{
	int conn = Header->ClientId;

	p36C *p = (p36C*)Header;

	int LOCAL199 = Mob[conn].Last.X; // local199
	int posY = Mob[conn].Last.Y; // local200

	int destX = p->Destiny.X; // local201
	int destY = p->Destiny.Y; // local202

	int lastX = p->LastPos.X; // local203
	int lastY = p->LastPos.Y; // local204

	if (destX <= 0 || destX >= 4096 || destY <= 0 || destY >= 4096)
	{
		// LOG ERROR

		return true;
	}

	if (lastX <= 0 || lastX >= 4096 || lastY <= 0 || lastY >= 4096)
	{
		// LOG ERROR

		return true;
	}

	if (LOCAL199 <= 0 || LOCAL199 >= 4096 || posY <= 0 || posY >= 4096)
	{
		// LOG ERROR

		return true;
	}

	// local205
	int heightPos1 = (g_pHeightGrid[posY][LOCAL199]) - (g_pHeightGrid[destY][destX]);
	
	if (heightPos1 < 0)
	{
		heightPos1 = -heightPos1; // NEG ECX
	}

	// local206
	int heightPos2 = (g_pHeightGrid[posY][LOCAL199]) - (g_pHeightGrid[lastY][lastX]);

	if (heightPos2 < 0)
	{
		heightPos2 = -heightPos2;
	}

	if (g_pHeightGrid[destY][destX] == 127 || g_pHeightGrid[lastY][lastX] == 127 || heightPos1 > 30 || heightPos2 > 30)
	{
		// CREATE MOB
		// LOG ERROR
		p364 msg;
		GetCreateMob(conn, (BYTE*)&msg);

		GridMulticast_2(Mob[conn].Target.X, Mob[conn].Target.Y, (BYTE*)&msg, 0);

		// LOG
		return true;
	}

	if (Users[conn].Status != 22)
	{
		SendHpMode(conn);

		return true;
	}

	int LOCAL251 = Mob[conn].Mobs.Player.Status.curHP;
	if (!LOCAL251)
	{		
		SendHpMode(conn);
		return true;
	}

	if (p->Header.TimeStamp != 0x0E0A1ACA)
	{
		int LOCAL252 = 0;
		// 0x752C474 = Logout
		// 0x752C478 = Recall
		// 0x752C47C = Restart
		if (Users[conn].Movement7556.Logout != 0)
		{
			if (Users[conn].Movement7556.Logout >= CurrentTime - 5000)
				LOCAL252 = 1;
		}

		if (Users[conn].Movement7556.Recall != 0)
		{
			if (Users[conn].Movement7556.Recall >= CurrentTime - 5000)
				LOCAL252 = 1;
		}

		if (Users[conn].Movement7556.Restart != 0)
		{
			if (Users[conn].Movement7556.Restart >= CurrentTime - 5000)
				LOCAL252 = 1;
		}
		if (LOCAL252 != 0)
		{
			// LOG Action While delay_mode

			Users[conn].Movement7556.Logout = 0;
			Users[conn].Movement7556.Recall = 0;
			Users[conn].Movement7556.Restart = 0;
		}
	}

	unsigned int LOCAL253 = p->Header.TimeStamp;
	unsigned int LOCAL254 = 0;
	if (CurrentTime > 20000)
		LOCAL254 = CurrentTime - 20000;
	else
		LOCAL254 = 0;

	if (p->Header.PacketId == 0x368)
	{
		if (Mob[conn].Mobs.Player.ClassInfo != 3 || !(Mob[conn].Mobs.Player.Learn[0] & 2))
		{
			if (Mob[conn].Mobs.Player.Status.Level < 400)
			{
				// LOG Request Illusion
				return true;
			}
		}
		// fmaster - savemana
		int LOCAL_255 = SkillData[73].Mana;

		if (Mob[conn].Mobs.Player.Status.curMP < LOCAL_255)
		{
			SendSetHpMp(conn);
			return false;
		}

		Mob[conn].Mobs.Player.Status.curMP -= LOCAL_255;
		Users[conn].Potion.CountMp -= LOCAL_255; // 752C438 
		// 0x752C0C0
		if (Users[conn].IllusionTime != 0x0E0A1ACA)
		{
			if (LOCAL253 < Users[conn].IllusionTime + 900)
			{
				// LOG illusion 900ms
				return true;
			}
		}

		if (LOCAL253 > CurrentTime + 3000 || LOCAL253 > LOCAL254)
		{
			// LOCAL illusion 3000ms
			CurrentTime = GetTickCount();

			return true;
		}

		Users[conn].IllusionTime = LOCAL253;
	}
	else if (p->Header.PacketId == 0x36C)
	{
		if (Users[conn].Movement.TimeStamp != 0x0E0A1ACA)
		{
			if (LOCAL253 < Users[conn].Movement.TimeStamp + 900)
			{
				// LOCAL action 900ms

				return true;
			}
		}
		
		if (LOCAL253 > CurrentTime + 3000 || LOCAL253 > LOCAL254)
		{
			if (LOCAL253 > CurrentTime + 3000)
			{
				// LOG action 3000ms
			}
			else
			{
				// Log action 2000ms
			}

			// log action faster 3000ms - verificar os JNB
			if (LOCAL253 > CurrentTime + 6000 || LOCAL253 > LOCAL254)
			{
				CloseUser(conn);

				// action faster 6000ms - closeuser

				return true;
			}

			CurrentTime = GetTickCount();
			return true;
		}

		Users[conn].Movement.TimeStamp = LOCAL253;
	}
	else if (p->Header.PacketId == 0x366)
	{
		if (Users[conn].Movement.PacketId == 0x366)
		{
			// LOG cant stop MSG_STOP continuously
			return true;
		}
	}
	else
		return true;

	Mob[conn].GetCurrentScore(conn);

	unsigned int LOCAL_256 = GetSpeed(&Mob[conn].Mobs.Player.Status);

	if (p->MoveSpeed > LOCAL_256)
	{
		// log different moviemtn

		p->MoveSpeed = Mob[conn].Mobs.Player.Status.Move.Value & 15;
	}

	/* if (p->MoveType != 1 && p->MoveType != 2)
	{
		if (p->Destiny.X < LOCAL199 - VIEWGRIDX || p->Destiny.X > LOCAL199 + VIEWGRIDX || p->Destiny.Y < posY - VIEWGRIDY || p->Destiny.Y > posY + VIEWGRIDY)
		{
			if (p->Destiny.X < LOCAL199 - (VIEWGRIDX * 2) || p->Destiny.X > LOCAL199 + VIEWGRIDX * 2 || p->Destiny.Y < posY - VIEWGRIDY * 2 || p->Destiny.Y > posY + VIEWGRIDY * 2)
			{
				p36C sm; // local55
				memset(&sm, 0, sizeof p36C);

				GetAction(conn, LOCAL199, posY, &sm);
				sm.Header.PacketId = 0x368;
				sm.MoveSpeed = 6;

				Users[conn].SendOneMessage((BYTE*)&sm, sizeof p36C);
				return true;
			}
		}
	}
	*/

	//004615B8  |. 83C4 0C        ADD ESP,0C


	return true;
}
