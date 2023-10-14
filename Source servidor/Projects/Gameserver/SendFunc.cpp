#include "cServer.h"
#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "BufferReaderWriter.h"

void SendSignal(INT32 toClientId, INT32 clientId, INT16 packetId)
{
	if (toClientId <= 0 || toClientId >= MAX_PLAYER)
		return;

	PacketHeader header;
	memset(&header, 0, sizeof header);

	header.PacketId = packetId;
	header.ClientId = clientId;
	header.Size = sizeof header;

	Users[toClientId].AddMessage((BYTE*)&header, sizeof header);
}

void SendSignalParm(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value)
{
	if (toClientId <= 0 || toClientId >= MAX_PLAYER)
		return;

	pMsgSignal packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = packetId;
	packet.Header.ClientId = clientId;
	packet.Header.Size = sizeof packet;

	packet.Value = value;

	Users[toClientId].AddMessage((BYTE*)&packet, sizeof packet);
}

void SendSignalParm2(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value, INT32 value2)
{
	if (toClientId <= 0 || toClientId >= MAX_PLAYER)
		return;

	pMsgSignal2 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = packetId;
	packet.Header.ClientId = clientId;
	packet.Header.Size = sizeof packet;

	packet.Value = value;
	packet.Value2 = value2;

	Users[toClientId].AddMessage((BYTE*)&packet, sizeof packet);
}

void SendSignalParm3(INT32 toClientId, INT32 clientId, INT16 packetId, INT32 value, INT32 value2, INT32 value3)
{
	if (toClientId <= 0 || toClientId >= MAX_PLAYER)
		return;

	pMsgSignal3 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = packetId;
	packet.Header.ClientId = clientId;
	packet.Header.Size = sizeof packet;

	packet.Value = value;
	packet.Value2 = value2;
	packet.Value3 = value3;

	Users[toClientId].AddMessage((BYTE*)&packet, sizeof packet);
}

void SendKingdomBattleInfo(int clientId, int kingdom, bool status)
{
	_MSG_REALBATTLE packet{};
	packet.Header.PacketId = RealBattlePacket;
	packet.Header.ClientId = clientId;

	packet.Kingdom = kingdom;
	packet.Status = status;

	if (clientId <= 0 || clientId >= MAX_PLAYER)
	{
		for (auto& user : Users)
		{
			if (user.Status == USER_PLAY)
				user.AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof _MSG_REALBATTLE);
		}

		return;
	}

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof _MSG_REALBATTLE);
}

void SendGridMob(int Index)
{
	CMob *mob = (CMob*)&Mob[Index];

	int VisX = VIEWGRIDX, VisY = VIEWGRIDY,
		minPosX = (mob->Target.X - HALFGRIDX),
		minPosY = (mob->Target.Y - HALFGRIDY);

	if ((minPosX + VisX) >= 4096)
		VisX = (VisX - (VisX + minPosX - 4096));

	if ((minPosY + VisY) >= 4096)
		VisY = (VisY - (VisY + minPosY - 4096));

	if (minPosX < 0)
	{
		minPosX = 0;
		VisX = (VisX + minPosX);
	}

	if (minPosY < 0)
	{
		minPosY = 0;
		VisY = (VisY + minPosY);
	}

	int maxPosX = (minPosX + VisX),
		maxPosY = (minPosY + VisY);

	for (int nY = minPosY; nY < maxPosY; nY++)
	{
		for (int nX = minPosX; nX < maxPosX; nX++)
		{
			short mobID = g_pMobGrid[nY][nX];
			short initID = g_pItemGrid[nY][nX];

			if (mobID != 0 && Index != mobID)
			{
				if (mobID < MAX_PLAYER)
					SendCreateMob(mobID, Index);

				if (Index < MAX_PLAYER)
					SendCreateMob(Index, mobID);
			}

			if (initID > 0 && mobID < MAX_PLAYER)
				SendCreateItem(mobID, initID, 0);

			if (initID > 0 && Index < MAX_PLAYER)
				SendCreateItem(Index, initID, 0);
		}
	}
}

void SendCreateMob(int sendClientID, int createClientID, INT32 send)
{
	if (sendClientID >= MAX_PLAYER || sendClientID <= 0)
		return;

	if (createClientID < MAX_PLAYER && Users[createClientID].IsAutoTrading)
	{ // Envia a venda do player
		p363 pak;
		GetCreateMobTrade(createClientID, (BYTE*)&pak);

		Users[sendClientID].AddMessage((BYTE*)&pak, sizeof pak);

		if (send != 0)
			Users[sendClientID].SendMessageA();
	}
	else
	{ // Envia o spawn normal
		if (createClientID > 0 && createClientID < MAX_PLAYER && Users[createClientID].IsBanned == 1 && !Users[sendClientID].IsAdmin)
			return;

		p364 packet{};
		GetCreateMob(createClientID, (BYTE*)&packet);

		Users[sendClientID].AddMessage((BYTE*)&packet, sizeof packet);

		if (send != 0)
			Users[sendClientID].SendMessageA();
	}
}

void SendCounterMob(int clientId, short value, short total)
{
	p3BB packet;
	memset(&packet, 0, sizeof p3BB);

	packet.Header.Size = sizeof p3BB;
	packet.Header.PacketId = 0x3BB;
	packet.Header.ClientId = clientId;

	packet.Value = value;
	packet.Total = total;

	Users[clientId].AddMessage((BYTE*)&packet, sizeof p3BB);
}

void SendCounterMobArea(int value1, int value2, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		if (Users[i].Status != USER_PLAY)
			continue;

		if (Mob[i].Target.X >= x1 && Mob[i].Target.Y >= y1 &&
			Mob[i].Target.X <= x2 && Mob[i].Target.Y <= y2)
		{
			SendCounterMob(i, value1, value2);
		}
	}
}

void SendAffect(int clientId)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	p3B9 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x3B9;
	packet.Header.ClientId = clientId;
	packet.Header.Size = sizeof p3B9;

	st_Affect *affect = Mob[clientId].Mobs.Affects;
	// Passa a divina pra frente
	for (int i = 1; i < 32; i++)
	{
		if (affect[i].Index == 34) // DIVINA
		{
			// Buffer temporário para salvar o buff atual
			st_Affect tmpAffect;

			memcpy(&tmpAffect, &affect[0], sizeof st_Affect);
			memcpy(&affect[0], &affect[i], sizeof st_Affect);
			memcpy(&affect[i], &tmpAffect, sizeof st_Affect);
			break;
		}
	}

	// Caso tenha ainda divina restante, atribui a estrutura
	float timeDiv = TimeRemaining(Users[clientId].User.Divina);
	if (timeDiv > 0.0f)
	{
		// Checa se já há divina na estrutura
		int i;
		for (i = 0; i < 32; i++)
		{
			if (affect[i].Index == 34)
				break;
		}

		// Caso retorne 32, quer dizer que ele possui divina ativada porém não está 
		// buffado, então buffará sozinho
		if (i == 32)
		{
			for (i = 0; i < 32; i++)
			{
				if (affect[i].Index == 0)
				{
					affect[i].Index = 34;
					affect[i].Master = 1;
					affect[i].Time = 999;

					break;
				}
			}
		}
	}

	float timeSeph = TimeRemaining(Users[clientId].User.Sephira);
	if (timeSeph > 0.0f)
	{
		// Checa se já há sephira na estrutura
		int i;
		for (i = 0; i < 32; i++)
		{
			if (affect[i].Index == 4)
				break;
		}

		// Caso retorne 32, quer dizer que ele possui sephira ativada porém não está 
		// buffado, então buffará sozinho
		if (i == 32)
		{
			for (i = 0; i < 32; i++)
			{
				if (affect[i].Index == 0)
				{
					affect[i].Index = 4;
					affect[i].Master = 1;
					affect[i].Value = 4;
					affect[i].Time = 999;
					break;
				}
			}
		}
	}

	float timeSaude = TimeRemaining(Mob[clientId].Mobs.Saude);
	if (timeSaude > 0.0f)
	{
		// Checa se já há saúde na estrutura
		int i;
		for (i = 0; i < 32; i++)
		{
			if (affect[i].Index == 35)
				break;
		}

		// Caso retorne 32, quer dizer que ele possui saúde ativada porém não está 
		// buffado, então buffará sozinho
		if (i == 32)
		{
			for (i = 0; i < 32; i++)
			{
				if (affect[i].Index == 0)
				{
					affect[i].Index = 35;
					affect[i].Master = 1;
					affect[i].Value = 1;
					affect[i].Time = 999;
					break;
				}
			}
		}
	}

	float timeRevi = TimeRemaining(Mob[clientId].Mobs.Revigorante);
	if (timeRevi > 0.0f)
	{
		// Checa se já há saúde na estrutura
		int i;
		for (i = 0; i < 32; i++)
		{
			if (affect[i].Index == 51)
				break;
		}

		// Caso retorne 32, quer dizer que ele possui saúde ativada porém não está 
		// buffado, então buffará sozinho
		if (i == 32)
		{
			for (i = 0; i < 32; i++)
			{
				if (affect[i].Index == 0)
				{
					affect[i].Index = 51;
					affect[i].Time = 999;

					break;
				}
			}
		}
	}

	for (INT32 i = 0; i < 32; i++)
	{
		if (affect[i].Index == 0 || affect[i].Time <= 0)
			continue;

		packet.Affect[i].Index = affect[i].Index;
		packet.Affect[i].Value = affect[i].Value;
		packet.Affect[i].Master = affect[i].Master;
		packet.Affect[i].Time = affect[i].Time;

		// Divina
		if (affect[i].Index == 34)
		{
			if (timeDiv <= 0)
			{
				memset(&affect[i], 0, sizeof st_Affect);

				continue;
			}

			affect[i].Time = 999;

			packet.Affect[i].Time = static_cast<int>(timeDiv);
		}
		else if (affect[i].Index == 4 && (affect[i].Value != 6 && affect[i].Value != 7 && affect[i].Value != 8))
		{
			if (timeSeph <= 0)
			{
				memset(&affect[i], 0, sizeof st_Affect);

				continue;
			}

			affect[i].Time = 999;

			packet.Affect[i].Time = static_cast<int>(timeSeph);
		}
		else if (affect[i].Index == 35 && affect[i].Value == 1)
		{
			if (timeSaude <= 0)
			{
				memset(&affect[i], 0, sizeof st_Affect);

				continue;
			}

			affect[i].Time = 999;

			packet.Affect[i].Time = static_cast<int>(timeSaude);
		}
		else if (affect[i].Index == 51)
		{
			if (timeRevi <= 0)
			{
				memset(&affect[i], 0, sizeof st_Affect);

				continue;
			}

			affect[i].Time = 999;

			packet.Affect[i].Time = static_cast<int>(timeRevi);
		}
	}

	Users[clientId].AddMessage((BYTE*)&packet, sizeof p3B9);
}

void SendItem(int clientId, SlotType invType, int slotId, st_Item *item)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	p182 p;

	p.Header.ClientId = clientId;
	p.Header.PacketId = 0x182;
	p.Header.Size = sizeof p182;

	p.invType = (short)invType;
	p.invSlot = slotId;

	memcpy(&p.itemData, item, sizeof st_Item);

	Users[clientId].AddMessage((BYTE*)&p, sizeof p182);

	//	SendEquip(clientId);
}

void SendEquip(int clientId)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER || Users[clientId].Status != USER_PLAY)
		return;

	p36B p;
	memset(&p, 0, sizeof p36B);
	p.Header.ClientId = clientId;
	p.Header.PacketId = 0x36B;
	p.Header.Size = sizeof p36B;

	st_Mob *mob = (st_Mob*)(&Mob[clientId].Mobs.Player);
	bool isUsingCostume = mob->Equip[12].Index != 0;
	for (int i = 0; i < 18; i++)
	{
		short effValue = 0;
		st_Item item = mob->Equip[i];

		if (i == 14)
		{
			if (item.Index >= 2360 && item.Index <= 2389)
			{
				if (*(short*)&item.EF1 <= 0)
				{
					p.ItemEff[i] = 0;
					p.pAnctCode[i] = 0;

					continue;
				}
			}
		}

		if (i == 0)
		{
			// se tiver um traje equipado
			if (mob->Equip[12].Index != 0 && mob->ClassInfo == 2)
				item.Index = item.EF2;
		}
		p.pAnctCode[i] = GetAnctCode(&item, isUsingCostume);
		p.ItemEff[i] = GetItemIDAndEffect(&item, i, isUsingCostume);

	}

	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&p, 0);

}

struct p3366
{
	PacketHeader Header; // 0 - 11
	st_Status   Status;

	unsigned char  Critical;
	unsigned char  SaveMana;

	struct
	{
		BYTE Time;
		BYTE Index;
	} Affect[32]; //62 - 125

	unsigned short GuildIndex;
	unsigned short GuildLevel;

	BYTE Resist1; // 130 
	BYTE Resist2; // 131
	BYTE Resist3; // 132
	BYTE Resist4; // 133

	unsigned char RegenHP;
	unsigned char RegenMP;

	int CurrHp;
	int CurrMp;

	int OldMagic;
	unsigned char Special[4];

	int Life;
	int Magic;
};

void SendScore(int clientIndex)
{
	st_Mob *mob = &Mob[clientIndex].Mobs.Player;

	p3366 p;
	memset(&p, 0, sizeof p);
	p.Header.ClientId = clientIndex;
	p.Header.PacketId = 0x336;
	p.Header.Size = sizeof  p3366;

	if (clientIndex < MAX_PLAYER)
	{
		p.Life = Mob[clientIndex].Lifes;

		p.CurrHp = mob->Status.curHP;
		p.CurrMp = mob->Status.curMP;
	}

	p.GuildIndex = (mob->GuildIndex);
	p.GuildLevel = mob->GuildMemberType;

	if (Mob[clientIndex].GuildDisable != 0)
		p.GuildIndex = 0;

	p.OldMagic = Mob[clientIndex].MagicIncrement;

	p.RegenHP = mob->RegenHP;
	p.RegenHP = mob->RegenMP;

	p.Critical = mob->Critical;
	p.Resist1 = mob->Resist.Fogo;
	p.Resist2 = mob->Resist.Gelo;
	p.Resist3 = mob->Resist.Sagrado;
	p.Resist4 = mob->Resist.Trovao;


	p.SaveMana = mob->SaveMana;

	memcpy(&p.Status, &mob->Status, sizeof st_Status);

	st_Affect *affect = Mob[clientIndex].Mobs.Affects;
	for (int i = 0; i < 32; i++)
	{
		p.Affect[i].Index = affect[i].Index & 0xFF;
		p.Affect[i].Time = affect[i].Time & 0xFF;
	}

	if (Mob[clientIndex].Mobs.Player.Info.Merchant & 1)
		p.Status.Merchant.Merchant = 1;

	if (clientIndex < MAX_PLAYER && Users[clientIndex].Arena.GroupIndex != -1 && Mob[clientIndex].Target.X >= 143 && Mob[clientIndex].Target.Y >= 546 && Mob[clientIndex].Target.X <= 195 && Mob[clientIndex].Target.Y <= 625)
	{
		p.GuildIndex = 0;
		p.GuildLevel = 0;
	}

	GridMulticast_2(Mob[clientIndex].Target.X, Mob[clientIndex].Target.Y, (BYTE*)&p, 0);
	SendAffect(clientIndex);
}

void SendAutoTrade(int sendClientId, int tradeClientId)
{
	if (!Users[tradeClientId].IsAutoTrading)
		return;

	p397 pTrade;
	memset(&pTrade, 0, sizeof pTrade);

	pTrade.Header.Size = sizeof(p397);
	pTrade.Header.PacketId = 0x397;
	pTrade.Header.ClientId = tradeClientId;

	pTrade.Index = tradeClientId;

	memcpy(pTrade.Gold, Users[tradeClientId].AutoTrade.Price, sizeof(int) * 12);
	memcpy(pTrade.Item, Users[tradeClientId].AutoTrade.Item, 12 * sizeof(st_Item));
	memcpy(pTrade.Slot, Users[tradeClientId].AutoTrade.Slots, 8);
	strncpy_s(pTrade.Name, Users[tradeClientId].AutoTrade.Name, 24);
	pTrade.Unknown = Users[tradeClientId].AutoTrade.Unknown_1784;

	Users[sendClientId].AddMessage((BYTE*)&pTrade, sizeof p397);
}

void SendCargoCoin(int clientId)
{
	if (clientId <= MOB_EMPTY || clientId >= MAX_PLAYER || Users[clientId].Status != USER_PLAY)
		return;

	p339 p;
	memset(&p, 0, sizeof p);

	p.Header.ClientId = clientId;
	p.Header.PacketId = 0x339;

	p.Gold = Users[clientId].User.Storage.Coin;

	Users[clientId].AddMessage((BYTE*)&p, sizeof p);
}

void SendEtc(int clientId)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER || Users[clientId].Status != USER_PLAY)
		return;

	st_Mob *mob = &Mob[clientId].Mobs.Player;
	p337 p;
	memset(&p, 0, sizeof p);

	p.Header.ClientId = clientId;
	p.Header.PacketId = 0x337;

	p.Gold = mob->Gold;
	p.Exp = mob->Exp;

	p.Hold = static_cast<unsigned int>(Mob[clientId].Mobs.Hold);

	p.Learn = mob->Learn[0];
	p.SecLearn = mob->Learn[1];
	p.Magic = mob->MagicIncrement;

	p.pMaster = mob->MasterPoint;
	p.pSkills = mob->SkillPoint;
	p.pStatus = mob->StatusPoint;

	Users[clientId].AddMessage((BYTE*)&p, sizeof p337);
}

void DeleteMob(int clientId, int reason)
{
	pMsgSignal packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x165;
	packet.Header.ClientId = clientId;
	packet.Header.Size = 16;

	packet.Value = reason;

	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);

	if (reason != 0)
	{
		if (clientId >= MAX_PLAYER)
		{
			INT32 LOCAL_5 = Mob[clientId].GenerateID;

			if (LOCAL_5 >= 0 && LOCAL_5 < MAX_NPCGENERATOR)
			{
				mGener.pList[LOCAL_5].MobCount -= 1;

				if (mGener.pList[LOCAL_5].MobCount < 0)
					mGener.pList[LOCAL_5].MobCount = 0;
			}
		}

		Mob[clientId].Mode = 0;

		if (Mob[clientId].Target.Y >= 0 && Mob[clientId].Target.Y < 4096 && Mob[clientId].Target.X >= 0 && Mob[clientId].Target.X < 4096)
			g_pMobGrid[Mob[clientId].Target.Y][Mob[clientId].Target.X] = 0;

		RemoveParty(clientId);
	}
}

void SendSay(int clientId, const char* msg, ...)
{
	/* Arglist */
	char buffer[108];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */

	SendMobSay(clientId, 0, 0, buffer);
}

void SendMobSay(int clientId, int receiverId, int Type, const char* msg, ...)
{
	/* Arglist */
	char buffer[108];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */

	p333 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x333;
	packet.Header.Size = sizeof p333;
	packet.Header.ClientId = clientId;
	packet.Type = Type;
	strncpy_s(packet.eChat, buffer, 96);

	if (!receiverId)
		GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);
	else
		Users[receiverId].AddMessage((BYTE*)&packet, sizeof packet);
}

void SendRemoveMob(UINT16 killerId, UINT16 killedId, UINT32 mType, INT32 send)
{
	pMsgSignal p;
	memset(&p, 0, sizeof pMsgSignal);

	p.Header.ClientId = killedId;
	p.Header.PacketId = 0x165;
	p.Header.Size = sizeof pMsgSignal;

	p.Value = mType;

	Users[killerId].AddMessage((BYTE*)&p, sizeof p);

	if (send != 0)
		Users[killerId].SendMessageA();
}

void SendEmotion(int clientId, int val1, int val2)
{
	p36A packet;
	memset(&packet, 0, sizeof p36A);

	packet.Header.Size = sizeof p36A;
	packet.Header.PacketId = 0x36A;
	packet.Header.ClientId = clientId;

	packet.NotUsed = 0;
	packet.Motion = val1;
	packet.Parm = val2;

	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)(&packet), 0);
}

void SendNoticeArea(const char *Message, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{//0x00401177;
	int LOCAL_1 = 1;
	for (; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if (Users[LOCAL_1].Status != USER_PLAY)
			continue;

		if (Mob[LOCAL_1].Target.X >= x1 && Mob[LOCAL_1].Target.X <= x2
			&& Mob[LOCAL_1].Target.Y >= y1 && Mob[LOCAL_1].Target.Y <= y2)
			SendClientMessage(LOCAL_1, Message);
	}
}

void SendSetHpMp(int clientId)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER || Users[clientId].Status != USER_PLAY)
		return;

	if (Users[clientId].Socket.Socket <= 0)
		return;

	p181 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x181;
	packet.Header.Size = sizeof packet;
	packet.Header.ClientId = clientId;

	SetReqHp(clientId);
	SetReqMp(clientId);
	packet.curHP = Mob[clientId].Mobs.Player.Status.curHP;
	packet.curMP = Mob[clientId].Mobs.Player.Status.curMP;
	packet.maxHP = Users[clientId].Potion.CountHp;
	packet.maxMP = Users[clientId].Potion.CountMp;

	Users[clientId].AddMessage((BYTE*)&packet, sizeof packet);
}

void SendCreateItem(int toClientId, int initId, int unk)
{
	if (toClientId <= 0 || toClientId >= MAX_PLAYER || Users[toClientId].Status != USER_PLAY)
		return;

	p26E pak;
	GetCreateItem(initId, &pak);

	Users[toClientId].AddMessage((BYTE*)&pak, sizeof p26E);
}

void SendRemoveItem(int dest, int itemid, int bSend)
{
	MSG_DecayItem sm_deci;
	memset(&sm_deci, 0, sizeof(MSG_DecayItem));

	sm_deci.Header.PacketId = 0x16F;
	sm_deci.Header.Size = sizeof(MSG_DecayItem);
	sm_deci.Header.ClientId = 30000;
	sm_deci.ItemID = 10000 + itemid;
	sm_deci.unk = 0;

	Users[dest].AddMessage((BYTE*)&sm_deci, sizeof(MSG_DecayItem));

	if (bSend)
		Users[dest].SendMessageA();
}

void SendAddParty(int target, int whom, int leader)
{
	if (target <= 0 || target >= MAX_PLAYER || Users[target].Status != USER_PLAY)
		return;

	p37D packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x37D;
	packet.Header.Size = sizeof packet;
	packet.Header.ClientId = 30000;

	packet.PartyID = static_cast<unsigned short>(whom);
	packet.Level = static_cast<unsigned short>(Mob[whom].Mobs.Player.bStatus.Level);
	strcpy_s(packet.nickName, Mob[whom].Mobs.Player.Name);

	if (leader)
		packet.LiderID = 0;
	else
		packet.LiderID = 30000;

	// O HP nos pacotes vão são do tamanho de 2 bytes, sendo assim, se for unsigned até 65535 e se for signed
	// até 32767. Na atualização de versão do WYD para a versão 759+, os HPs foram alterados para 4 bytes, um
	// limite bem maior que o anterior. Neste caso, acho que os kr esqueceram de atualizar este pacote... Neste
	// caso, aqui so teremos a porcentagem na barar de grupo do personagem, então faremos um cálculo de quantos %
	// do HP do usuarío está para mostrarmos na tela (não mostrará o valor real)
	int hpPercent = static_cast<int>((static_cast<float>(Mob[whom].Mobs.Player.Status.curHP) / static_cast<float>(Mob[whom].Mobs.Player.Status.maxHP) * 100.0f));

	packet.maxHP = 100;
	packet.curHP = hpPercent;

	packet.ID = 52428;

	Users[target].AddMessage((BYTE*)&packet, sizeof packet);
}

void SendRemoveParty(int target, int whom)
{
	if (target <= 0 || target >= MAX_PLAYER || Users[target].Status != USER_PLAY || Users[target].Socket.Socket <= 0)
		return;

	p37E packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x37E;
	packet.Header.Size = sizeof packet;
	packet.Header.ClientId = 30000;

	packet.mobId = whom;

	Users[target].AddMessage((BYTE*)&packet, sizeof packet);
}
void RemoveSummonerParty(INT32 clientId)
{
	if (clientId <= 0 || clientId > 1000)
		return;

	if (Mob[clientId].Mode == 0)
		return;

	for (int i = 0; i < 12; i++)
	{
		int summonerId = Mob[clientId].SummonerParty[i];

		if (summonerId == 0)
			continue;

	    Mob[clientId].SummonerParty[i] = 0;
		DeleteMob(summonerId, 5);
	}
}
void SendHpMode(int clientId)
{ // 004428E0
	if (clientId <= 0 || clientId >= MAX_PLAYER || Users[clientId].Status != USER_PLAY)
		return;

	p292 p;
	p.Header.PacketId = 0x292;
	p.Header.Size = 16;
	p.Header.ClientId = clientId;

	p.CurHP = Mob[clientId].Mobs.Player.Status.curHP;
	p.Status = Users[clientId].Status;

	Users[clientId].AddMessage((BYTE*)&p, sizeof p292);
}

void SendCarry(int clientId)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER || Users[clientId].Status != USER_PLAY)
		return;

	p185 packet;
	packet.Header.ClientId = clientId;
	packet.Header.PacketId = 0x185;
	packet.Header.Size = sizeof p185;

	memcpy(packet.Item, Mob[clientId].Mobs.Player.Inventory, sizeof st_Item * 64);

	packet.Gold = Mob[clientId].Mobs.Player.Gold;
	Users[clientId].AddMessage((BYTE*)&packet, sizeof packet);
}

void SendDamage(unsigned int min_x, unsigned int min_y, unsigned int max_x, unsigned int max_y)
{//04012A8
	for (INT32 LOCAL_1 = 1; LOCAL_1 < MAX_PLAYER; LOCAL_1++)
	{
		if (Users[LOCAL_1].Status != USER_PLAY)
			continue;

		if (Mob[LOCAL_1].Target.X >= min_x && Mob[LOCAL_1].Target.X <= max_x && Mob[LOCAL_1].Target.Y >= min_y &&
			Mob[LOCAL_1].Target.Y <= max_y)
		{
			float LOCAL_2 = 0.25;
			INT32 LOCAL_3 = 2000;
			INT32 LOCAL_4 = Mob[LOCAL_1].Mobs.Player.Status.curHP;
			INT32 LOCAL_5 = LOCAL_4 - LOCAL_3;
			LOCAL_4 = LOCAL_4 * (float)LOCAL_2;

			if (LOCAL_4 < LOCAL_5)
				LOCAL_4 = LOCAL_5;

			if (LOCAL_4 < 1)
				LOCAL_4 = 1;

			INT32 LOCAL_6 = LOCAL_4 - Mob[LOCAL_1].Mobs.Player.Status.curHP;

			Mob[LOCAL_1].Mobs.Player.Status.curHP = LOCAL_4;
			Users[LOCAL_1].Potion.CountHp += LOCAL_6;

			SetReqHp(LOCAL_1);
			SetReqMp(LOCAL_1);

			p18A packet{};
			packet.Header.PacketId = 0x18A;
			packet.Header.Size = sizeof p18A;
			packet.Header.ClientId = LOCAL_1;

			packet.CurHP = LOCAL_4;
			packet.Incress = LOCAL_6;

			INT32 LOCAL_11 = Mob[LOCAL_1].Target.X;
			INT32 LOCAL_12 = Mob[LOCAL_1].Target.Y;

			GridMulticast_2(LOCAL_11, LOCAL_12, (BYTE*)&packet, 0);
		}
	}
}

// EBP + 8, EBP + 0C, EBP + 10, EBP + 14, EBP + 18, EBP + 1C
void SendEnvEffect(int min_x, int min_y, int max_x, int max_y, int type1, int type2)
{//04012A8
	p3A2 packet;

	packet.Header.PacketId = 0x3A2;
	packet.Header.Size = sizeof p3A2;
	packet.Header.ClientId = 0x7530;

	packet.MinX = min_x;
	packet.MaxX = max_x;
	packet.MinY = min_y;
	packet.MaxY = max_y;

	packet.Type1 = type1;
	packet.Type2 = type2;

	GridMulticast_2(min_x + ((max_x - min_x) >> 1), min_y + ((max_y - min_y) >> 1), (BYTE*)&packet, 0);
}

void SendNotice(const char* msg, ...)
{
	/* Arglist */
	char buffer[256];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */

	buffer[107] = '\0';

	p101 packet;
	memset(&packet, 0, sizeof p101);

	packet.Header.PacketId = 0x101;
	packet.Header.ClientId = 0;
	packet.Header.Size = sizeof p101;

	strncpy_s(packet.Msg, buffer, 128);

	bool needLog = buffer[0] != '.';
	for (INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if (Users[i].Status != USER_PLAY)
			continue;

		Users[i].AddMessage((BYTE*)&packet, sizeof p101);

		if(needLog)
			Log(i, LOG_INGAME, "[Mensagem do servidor]: %s", buffer);
	}
}

void SendGuildNotice(INT32 guildId, const char *msg, ...)
{
	/* Arglist */
	char buffer[108];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */

	p101 packet;
	memset(&packet, 0, sizeof p101);

	packet.Header.PacketId = 0x101;
	packet.Header.ClientId = 0;
	packet.Header.Size = sizeof p101;

	strncpy_s(packet.Msg, buffer, 96);

	for (INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if (Users[i].Status != USER_PLAY)
			continue;

		if (Mob[i].Mobs.Player.GuildIndex != guildId)
			continue;

		Users[i].AddMessage((BYTE*)&packet, sizeof p101);
	}
}

// Notifica a DBSRV para enviar a mensagem para todos os canais
void SendServerNotice(const char *msg, ...)
{
	/* Arglist */
	char buffer[108];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);

	_MSG_SEND_SERVER_NOTICE p;
	memset(&p, 0, sizeof _MSG_SEND_SERVER_NOTICE);

	p.Header.ClientId = 0;
	p.Header.PacketId = MSG_SEND_SERVER_NOTICE;
	p.Header.Size = sizeof _MSG_SEND_SERVER_NOTICE;

	strncpy_s(p.Notice, buffer, 96);

	AddMessageDB((BYTE*)&p, sizeof _MSG_SEND_SERVER_NOTICE);
}

void SendWarInfo(INT32 clientId, INT32 capeWin)
{
	INT32 LOCAL_1 = 0;
	INT32 LOCAL_2 = MAX_GUILD;
	INT32 LOCAL_3 = Mob[clientId].Mobs.Player.GuildIndex;

	if (LOCAL_3 <= 0 || LOCAL_3 >= LOCAL_2)
		LOCAL_3 = 0;

	INT32 LOCAL_4 = g_pGuildWar[LOCAL_3];
	if (LOCAL_4 <= 0 || LOCAL_4 >= LOCAL_2)
		LOCAL_4 = 0;

	INT32 LOCAL_5 = g_pGuildAlly[LOCAL_3];
	if (LOCAL_5 <= 0 || LOCAL_5 >= LOCAL_2)
		LOCAL_5 = 0;

	if (LOCAL_3 != 0 && LOCAL_4 != 0 && g_pGuildWar[LOCAL_4] != LOCAL_3)
		LOCAL_1 = LOCAL_4;
	else
		LOCAL_1 = 0;

	if (false)
	{
		//SendSignalParm2(arg1, 0x7530 ,0x3A8, LOCAL_1, arg2);
	}
	else
		SendSignalParm3(clientId, 0x7530, 0x3A8, LOCAL_1, capeWin, LOCAL_5);
}

void SendGuildList(INT32 clientId)
{
	INT32 LOCAL_1 = Mob[clientId].Mobs.Player.GuildIndex;

	if (LOCAL_1 <= 0)
		return;

	char LOCAL_65[108] = {};

	INT32 LOCAL_66 = 0;
	INT32 LOCAL_67 = 70;
	INT32 LOCAL_68 = 1;

	for (; LOCAL_68 < MAX_PLAYER; LOCAL_68++)
	{
		if (Users[LOCAL_68].Status == USER_PLAY && Mob[LOCAL_68].Mobs.Player.GuildIndex == LOCAL_1)
		{
			LOCAL_66++;

			INT32 LOCAL_69 = strlen(LOCAL_65);

			if (LOCAL_69 >= LOCAL_67)
			{
				SendClientMessage(clientId, LOCAL_65);

				LOCAL_65[0] = 0;
			}

			Mob[LOCAL_68].Mobs.Player.Name[15] = 0;
			Mob[LOCAL_68].Mobs.Player.Name[14] = 0;

			strcat_s(LOCAL_65, Mob[LOCAL_68].Mobs.Player.Name);
			strcat_s(LOCAL_65, " ");
		}
	}

	if (LOCAL_65[0])
		SendClientMessage(clientId, LOCAL_65);

	if (LOCAL_66 == 0)
		SendClientMessage(clientId, g_pLanguageString[_NN_No_Guild_Members]);

	INT32 LOCAL_70 = MAX_GUILD;

	if (LOCAL_1 > 0 && LOCAL_1 < LOCAL_70)
	{
		INT32 LOCAL_71 = g_pGuildWar[LOCAL_1],
			LOCAL_80 = -1;

		if (LOCAL_71 > 0 && LOCAL_71 < LOCAL_70)
		{
			INT32 LOCAL_81 = g_pGuildWar[LOCAL_71];

			if (LOCAL_1 == LOCAL_81)
			{
				SendClientMessage(clientId, g_pLanguageString[_SN_Your_are_at_war], g_pGuild[LOCAL_71].Name.c_str());

				LOCAL_80 = LOCAL_71;
			}
			else
				SendClientMessage(clientId, g_pLanguageString[_SN_War_to_S], g_pGuild[LOCAL_71].Name.c_str());
		}

		INT32 LOCAL_82 = 1;
		for (; LOCAL_82 < LOCAL_70; LOCAL_82++)
		{
			if (g_pGuildWar[LOCAL_82] == LOCAL_1 && LOCAL_82 != LOCAL_80)
				SendClientMessage(clientId, g_pLanguageString[_SN_War_from_S], g_pGuild[LOCAL_82].Name.c_str());
		}

		LOCAL_71 = g_pGuildAlly[LOCAL_1];
		if (LOCAL_71 > 0 && LOCAL_71 < LOCAL_70)
			SendClientMessage(clientId, g_pLanguageString[_SN_Ally_to_S], g_pGuild[LOCAL_71].Name.c_str());

		LOCAL_82 = 1;
		for (; LOCAL_82 < LOCAL_70; LOCAL_82++)
		{
			if (g_pGuildAlly[LOCAL_82] == LOCAL_1)
				SendClientMessage(clientId, g_pLanguageString[_SN_Ally_from_S], g_pGuild[LOCAL_82].Name.c_str());
		}
	}
}

void SendWeather()
{
	p18B packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x18B;
	packet.Header.Size = sizeof packet;
	packet.Header.ClientId = 0x7530;

	packet.WeatherId = sServer.Weather;

	for (INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if (Users[i].Status != USER_PLAY)
			continue;

		INT32 LOCAL_7;
		if (Mob[i].Target.X >> 7 < 12 && Mob[i].Target.Y >> 7 > 25)
			LOCAL_7 = 1;
		else
			LOCAL_7 = 0;

		if (LOCAL_7 != 0)
			continue;

		Users[i].AddMessage((BYTE*)&packet, sizeof packet);
	}
}

void SendChatGuild(INT32 clientId, INT32 guildId, const char *msg, ...)
{
	if (clientId <= 0 || clientId >= MAX_PLAYER)
		return;

	/* Arglist */
	char buffer[128];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);

	p334 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x334;
	packet.Header.Size = sizeof p334;
	packet.Header.ClientId = SERVER_SIDE;

	sprintf_s(packet.eValue, buffer);
	strncpy_s(packet.eCommand, g_pGuild[guildId].Name.c_str(), 16);

	char *p = (char*)&packet;
	*(INT16*)&p[124] = 3;

	Users[clientId].AddMessage((BYTE*)&packet, sizeof p334);
}

void SendRepurchase(int clientId)
{
	p3E8 packet{};

	INT32 cityZone = GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y);
	if (cityZone == 5)
		cityZone = 4;

	for (int i = 0; i < 10; i++)
	{
		packet.Item[i].index = i + 1;

		st_Item* item = &Users[clientId].Repurchase.Items[i];
		if (item->Index <= 0 || item->Index >= MAX_ITEMLIST)
			continue;

		int price, impost;
		std::tie(price, impost) = GetPriceAndImpost(&ItemList[item->Index], cityZone);

		packet.Item[i].item = *item;
		packet.Item[i].sellPrice = price;
	}

	packet.Header.PacketId = 0x3E8;
	packet.Header.Size = sizeof p3E8;

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof p3E8);
}

void SendQuiz(int clientId)
{
	MSG_QUIZ packet{};
	packet.Header.PacketId = 0x1C6;
	packet.Header.Size = sizeof MSG_QUIZ;
	packet.Header.ClientId = clientId;

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof MSG_QUIZ);
}

void SendQuiz(int clientId, const char* question, std::array<std::string, 4> answers)
{
	MSG_QUIZ packet{};
	packet.Header.PacketId = 0x1C6;
	packet.Header.Size = sizeof MSG_QUIZ;
	packet.Header.ClientId = clientId;

	strncpy_s(packet.Title, question, 128);

	for (int i = 0; i < 4; i++)
		strncpy_s(packet.Ans[i], answers[i].c_str(), 32);

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof MSG_QUIZ);
}

void SendChristmasMission(int clientId, int status)
{
	if (Users[clientId].Christmas.Mission.Status == TOD_ChristmasMission_Status::WaitingNextRound)
		return;

	MSG_CHRISTMASMISSION packet{};
	packet.Header.PacketId = ChristmasMissionPacket;
	packet.Header.Size = sizeof MSG_CHRISTMASMISSION;
	packet.Header.ClientId = clientId;

	int missionId = Users[clientId].Christmas.Mission.MissionId;
	packet.Status = status;

	if (missionId != -1)
	{
		auto missionIt = std::find_if(std::begin(sServer.Christmas.Missions), std::end(sServer.Christmas.Missions), [&](const TOD_ChristmasMissionInfo& info) {
			return missionId == info.Id;
		});

		if (missionIt == std::end(sServer.Christmas.Missions))
		{
			Log(clientId, LOG_INGAME, "Missão não encontrada. MissãoId: %d", missionId);

			Users[clientId].Christmas.Mission.MissionId = -1;
			return ;
		}

		auto& mission = *missionIt;
		memcpy(&packet.Info.MobCount, mission.Count.data(), sizeof packet.Info.MobCount);

		for (int i = 0; i < 3; i++)
			strncpy_s(packet.Info.MobName[i], mission.MobName[i].c_str(), 16);

		strncpy_s(packet.Title, mission.Title.c_str(), 31);

		memcpy(packet.Rewards, mission.Reward.data(), sizeof packet.Rewards);
		memcpy(packet.Completed.MobCount, Users[clientId].Christmas.Mission.Count.data(), sizeof(int) * 3);
	}

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof MSG_CHRISTMASMISSION);
}

void SendArenaScoreboard(int clientId, const std::array<int, 4>& points)
{
	MSG_ARENASCOREBOARD packet{};
	packet.Header.PacketId = ArenaScoreboardRefreshPacket;
	packet.Header.Size = sizeof MSG_ARENASCOREBOARD;

	memcpy(&packet.Points, points.data(), sizeof packet.Points);

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof MSG_ARENASCOREBOARD);
}

void SendAutoPartyInfo(int clientId)
{
	MSG_AUTOPARTY packet{};
	packet.Header.PacketId = AutoPartyInfoPacket;
	packet.Header.ClientId = clientId;
	packet.Header.Size = sizeof packet;

	int i = 0;
	for (const auto& name : Users[clientId].AutoParty.Nicknames)
	{
		if (name.empty())
			continue;

		strncpy_s(packet.Nickname[i++],  name.c_str(), 16);
	}

	packet.EnableAll = Users[clientId].AutoParty.EnableAll;

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
}

void SendMissionInfo(int clientId)
{
	MSG_MISSIONINFO packet{};
	packet.Header.PacketId = MissionInfoPacket;
	packet.Header.Size = sizeof packet;

	const auto& daily = Mob[clientId].Mobs.DailyQuest;
	if (daily.QuestId == -1)
	{
		Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);

		return;
	}

	const auto& quest = std::find_if(std::begin(sServer.Missions), std::end(sServer.Missions), [&](const TOD_MissionInfo& info) {
		return info.QuestId == daily.QuestId;
	});

	if (quest == std::end(sServer.Missions))
		return;

	packet.IsAccepted = daily.IsAccepted;
	strncpy_s(packet.Name, quest->QuestName.c_str(), sizeof packet.Name);

	unsigned long long exp = quest->Exp;
	if (sServer.ExpDailyBonus != 0)
		exp = sServer.ExpDailyBonus * exp / 100;

	int evolution = Mob[clientId].Mobs.Player.Equip[0].Effect[1].Value;
	int level = Mob[clientId].Mobs.Player.Status.Level;

	if ((evolution <= ARCH && level == MAX_LEVEL) || (evolution >= CELESTIAL && level >= MAX_LEVEL_CELESTIAL))
		exp = 0;

	if (Mob[clientId].Mobs.Player.Equip[0].Effect[1].Value != MORTAL)
		exp = GetExpApply_2(exp, clientId, clientId, false);

	packet.Exp = exp;

	if (sServer.GoldDailyBonus != 0)
		packet.Gold = quest->Gold * sServer.GoldDailyBonus / 100;
	else
		packet.Gold = quest->Gold;

	for (int i = 0; i < 6; i++)
	{
		packet.FreeReward[i] = quest->FreeReward[i].Item;
		packet.BattlePassReward[i] = quest->BattlePassReward[i].Item;
	}

	for (int i = 0; i < 5; i++)
	{
		strncpy_s(packet.Mob[i].Name, quest->Mob.mobName[i].c_str(), 16);
		packet.Mob[i].Total = quest->Mob.Amount[i];

		packet.Item[i].ItemId = quest->Drop.Item[i];
		packet.Item[i].Total = quest->Drop.Amount[i];

		packet.Item[i].Dropped = daily.ItemCount[i];
		packet.Mob[i].Killed = daily.MobCount[i];
	}

	packet.LastUpdate = daily.LastUpdate;
	packet.BattlePassValidation = daily.BattlePass;

	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
}

void MulticastGetCreateMob(int clientId)
{
	p364 packet{};

	GetCreateMob(clientId, (BYTE*)&packet);
	GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);
}

void SendChatMessage(int clientId, int color, const char* message, ...)
{
	/* Arglist */
	char buffer[108];
	va_list arglist;
	va_start(arglist, message);
	vsprintf_s(buffer, message, arglist);
	va_end(arglist);
	/* Fim arlist */

	MSG_CHATMESSAGE packet{};
	packet.Header.PacketId = ChatMessagePacket;
	packet.Header.Size = sizeof packet;

	strncpy_s(packet.Message, buffer, 96);

	packet.Color = color;
	Users[clientId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
}

void SendChatMessage(int color, const char* message, ...)
{
	/* Arglist */
	char buffer[108];
	va_list arglist;
	va_start(arglist, message);
	vsprintf_s(buffer, message, arglist);
	va_end(arglist);
	/* Fim arlist */

	MSG_CHATMESSAGE packet{};
	packet.Header.PacketId = ChatMessagePacket;
	packet.Header.Size = sizeof packet;

	strncpy_s(packet.Message, buffer, 96);

	packet.Color = color;

	for (auto& user : Users)
	{
		if (user.Status == USER_PLAY)
			user.AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof packet);
	}
}

void SendOverStoreInfo(int clientId)
{
	BufferWriter writer{};
	writer += 12;

	writer.Set<unsigned int>(Users[clientId].User.Cash);
	const auto& categories = sServer.DonateStore.Categories;
	int totalCategories = categories.size();
	writer.Set<unsigned short>(totalCategories);

	for (auto category : categories)
	{
		size_t totalItems = category.Items.size();
		writer.Set<const char*>(category.Name.c_str(), category.Name.size());

		writer.Set<size_t>(totalItems);
		for (auto item : category.Items)
		{
			writer.Set<st_Item>(item.Item);
			writer.Set<short>(item.Price);
			writer.Set<short>(item.Available);
		}
	}

	int size = writer.GetBuffer().size();
	writer.Set<unsigned short>(static_cast<unsigned short>(writer.GetBuffer().size()), 0);
	writer.Set<unsigned short>(StoreInfoPacket, 4);

	Users[clientId].AddMessage(writer.GetBuffer().data(), size);
}