#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "UOD_EventManager.h"
#include "UOD_Quiz.h"
#include "UOD_ChristmasMission.h"
#include "BufferReaderWriter.h"

bool PacketControl(BYTE* pBuffer, INT32 size)
{
	PacketHeader *header = (PacketHeader*)pBuffer;

	int clientId = header->ClientId;

	switch (header->PacketId)
	{
		case LogPacket:
		{
			try {
				BufferReader buffer(reinterpret_cast<unsigned char*>(pBuffer), size);
				buffer += 12;

				Log(clientId, LOG_INGAME, buffer.Get<std::string>().c_str());
			}
			catch (std::exception&)
			{
			}
		}
		break;
	case MSG_PANELGUILD_GETLIST:
		Users[clientId].AddMessage(pBuffer, size);
		break;

	case MSG_FRIENDLIST_UPDATESTATUS_OPCODE:
	{
		_MSG_FRIENDLIST_UPDATESTATUS *packet = (_MSG_FRIENDLIST_UPDATESTATUS*)(header);

		for (INT32 i = 0; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			for (INT32 t = 0; t < 30; t++)
			{
				if (!_strnicmp(Users[i].User.Friends[t], packet->Name, 16))
				{
					Users[i].AddMessage((BYTE*)packet, sizeof _MSG_FRIENDLIST_UPDATESTATUS);
					break;
				}
			}
		}
	}
	break;
	case MSG_FRIENDLIST_OPCODE:
	{
		_MSG_FRIENDLIST *p = (_MSG_FRIENDLIST*)(header);

		for (INT32 i = 0; i < 30; i++)
			strncpy_s(Users[clientId].User.Friends[i], p->Name[i], 16);

		Users[clientId].AddMessage((BYTE*)p, sizeof _MSG_FRIENDLIST);
	}
	break;

	case 0x381:
	{
		pMsgSignal2 *p = (pMsgSignal2*)(header);

		if (p->Value < 0 || p->Value >= MAX_STORE || p->Value2 < 0 || p->Value2 >= MAX_DONATEITEM)
			return true;

		stDonateStore *store = &g_pStore[p->Value][p->Value2];

		if (store->Avaible != -1)
			store->Avaible--;
	}
	break;

	case 0x382:
	{
		pMsgSignal3 *p = (pMsgSignal3*)(header);

		if (p->Value < 0 || p->Value >= MAX_STORE || p->Value2 < 0 || p->Value2 >= MAX_DONATEITEM)
			return true;

		stDonateStore *store = &g_pStore[p->Value][p->Value2];
		store->Avaible = p->Value3;
	}
	break;

	case 0x80C:
	{
		ReadNPCDonate();
		Log(SERVER_SIDE, LOG_ADMIN, "Reload NPCDonate");
	}
	break;

	case 0xD1D:
		header->ClientId = 0x7530;

		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY)
				continue;

			Users[i].AddMessage((BYTE*)header, header->Size);
		}
		break;

	case 0xC0E:
	{
		pCOE *p = (pCOE*)(header);

		if (Users[clientId].Status < USER_SELCHAR)
		{
			Log(clientId, LOG_ERROR, "Falha ao entregar cash ImportCash - Status < USER_SELCHAR - %d",
				p->Cash);

			break;
		}

		if (strncmp(p->Username, Users[clientId].User.Username, 16))
		{
			Log(clientId, LOG_ERROR, "Falha ao entregar cash ImportCash - %s != %s - %d",
				p->Username, Users[clientId].User.Username, p->Cash);

			break;
		}

		Users[clientId].User.Cash += p->Cash;

		SendClientPacket(clientId);

		if (Users[clientId].Status == USER_SELCHAR)
			SaveUser(clientId, 0);

		Log(clientId, LOG_INGAME, "Recebeu %d de Cash - Total %d - ImportCash", p->Cash, Users[clientId].User.Cash);

		if (Users[clientId].Status == USER_PLAY)
		{
			SendClientMessage(clientId, "!Chegou [ %d ] Cash", p->Cash);
			SendSignalParm(clientId, clientId, RefreshGoldPacket, Users[clientId].User.Cash);
		}
	}
	break;

	case 0xC10:
	{
		pC10 *p = (pC10*)(header);

		if (strncmp(p->Username, Users[clientId].User.Username, 16))
		{
			Log(clientId, LOG_ERROR, "Falha ao banir o usuário %s != %s - %d %d/%d/%d",
				p->Username, Users[clientId].User.Username, p->BanType, p->Ban.Dia, p->Ban.Mes, p->Ban.Ano);

			break;
		}

		Users[clientId].User.BanType = p->BanType;
		memcpy(&Users[clientId].User.Ban, &p->Ban, sizeof(p->Ban));

		SendClientMessage(clientId, "Conta banida. Contate o suporte para maiores informações.");

		SaveUser(clientId, 0);
		CloseUser(clientId);
	}
	break;

	case 0xC0F:
	{
		pCOF *p = (pCOF*)(header);

		if (Users[clientId].Status < USER_SELCHAR)
		{
			Log(clientId, LOG_ERROR, "Falha ao receber item do ImportItem - Status < USER_SELCHAR - %d %d %d %d %d %d %d",
				p->item.Index, p->item.Effect[0].Index, p->item.Effect[0].Value, p->item.Effect[1].Index, p->item.Effect[1].Value, p->item.Effect[2].Index, p->item.Effect[2].Value);

			break;
		}

		if (strncmp(p->Username, Users[clientId].User.Username, 16))
		{
			Log(clientId, LOG_ERROR, "Falha ao receber item do ImportItem - %s != %s - %d %d %d %d %d %d %d",
				p->Username, Users[clientId].User.Username, p->item.Index, p->item.Effect[0].Index, p->item.Effect[0].Value, p->item.Effect[1].Index, p->item.Effect[1].Value, p->item.Effect[2].Index, p->item.Effect[2].Value);

			break;
		}

		for (INT32 i = 0; i < 120; i++)
		{
			if (Users[clientId].User.Storage.Item[i].Index != 0)
				continue;

			Users[clientId].User.Storage.Item[i] = p->item;
			SendItem(clientId, SlotType::Storage, i, &p->item);

			SendClientMessage(clientId, "!Chegou um item [ %s ]", ItemList[p->item.Index].Name);

			Log(clientId, LOG_INGAME, "Recebeu o item %s [%d] [%d %d %d %d %d %d]",
				ItemList[p->item.Index].Name, p->item.Index, p->item.Effect[0].Index, p->item.Effect[0].Value, p->item.Effect[1].Index, p->item.Effect[1].Value, p->item.Effect[2].Index, p->item.Effect[2].Value);

			if (Users[clientId].Status == USER_SELCHAR)
				SaveUser(clientId, 0);

			return true;
		}

		for (INT32 i = 119; i > 0; i--)
		{
			if (Users[clientId].User.Storage.Item[i].Index != 0)
				continue;

			Users[clientId].User.Storage.Item[i] = p->item;
			SendItem(clientId, SlotType::Storage, i, &p->item);

			SendClientMessage(clientId, "!Chegou um item [ %s ]", ItemList[p->item.Index].Name);

			Log(clientId, LOG_INGAME, "Recebeu o item %s [%d] [%d %d %d %d %d %d]",
				ItemList[p->item.Index].Name, p->item.Index, p->item.Effect[0].Index, p->item.Effect[0].Value, p->item.Effect[1].Index, p->item.Effect[1].Value, p->item.Effect[2].Index, p->item.Effect[2].Value);

			if (Users[clientId].Status == USER_SELCHAR)
				SaveUser(clientId, 0);

			return true;
		}

		SaveUser(clientId, 1);

		Log(clientId, LOG_ERROR, "Não recebeu o item %s [%d] [%d %d %d %d %d %d] por falta de espaço",
			ItemList[p->item.Index].Name, p->item.Index, p->item.Effect[0].Index, p->item.Effect[0].Value, p->item.Effect[1].Index, p->item.Effect[1].Value, p->item.Effect[2].Index, p->item.Effect[2].Value);

		SendClientMessage(clientId, "!Falta espaço para receber o item! Libere seu baú!");
	}
	break;

	case 0x52A:
		Users[clientId].ChangeServer(header);
		break;

	case MSG_CREATEGUILD_OPCODE:
		return GetNewGuild(header);

	case MSG_ADDSUB_OPCODE:
		return AddSubLider(header);

	case MSG_ADDGUILD_OPCODE:
		return GetGuildInfo(header);

	case 0x899:
	{
		pMsgSignal *p = (pMsgSignal*)(header);

		sServer.RvR.Bonus = p->Value;
		Log(SERVER_SIDE, LOG_INGAME, "Reino campeão da RvR: %d", p->Value);
	}
	break;
	case 0x3409:
	{
		MSG_NPNotice *m = (MSG_NPNotice*)(header);

		if (m->Parm1 == 1)
			SendNotice(m->String);

	} break;

	case 0x101:
	{
		p101 *p = (p101*)(header);

		SendNotice(p->Msg);
	}
	break;
	case MSG_RESULTWARTOWER_OPCODE:
	{
		_MSG_RESULTWARTOWER *p = (_MSG_RESULTWARTOWER*)(header);

		ResultTowerWar(p->Winner);
	}
	break;
	case 0x428:
	{
		MSG_GuildReport *m = (MSG_GuildReport*)header;

		memcpy(ChargedGuildList, m->ChargedGuildList, sizeof(ChargedGuildList));
	} break;

	case 0x415:
	{
		p415 *p = (p415*)(header);

		Users[clientId].User.SingleGift = p->SingleGift;
		Users[clientId].AccessLevel = p->Access;
		Users[clientId].User.Unique.Value = p->Unique;

		Users[clientId].User.Daily.WeekYear = p->Daily.WeekYear;

		Users[clientId].User.Water.Day = p->Water.Day;
		Users[clientId].User.Water.Total = p->Water.Total;

		Users[clientId].User.Divina = p->Divina;
		Users[clientId].User.Sephira = p->Sephira;

		memcpy(&Users[clientId].User.Ban, &p->Ban, sizeof stDate);
		Users[clientId].User.BanType = p->BanType;

		Users[clientId].IsBanned = p->IsBanned == 1;
		Users[clientId].User.Cash = p->Cash;
		memcpy(&Users[clientId].User.Daily.Received[0], &p->Daily.Received[0], sizeof p->Daily.Received);

		// Somente para liberação da missão após logar
		auto christmasMission = static_cast<TOD_ChristmasMission*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::ChristmasMission));
		if (christmasMission != nullptr)
			christmasMission->SetNextRound(Users[clientId]);

		if (TOD_EventManager::GetInstance().GetEvent(TOD_EventType::ChristmasMission) != nullptr)
			Users[clientId].Christmas.Tree.LastBox = std::chrono::steady_clock::now() - 5min;
	}
	break;
	case 0x41D:
	{
		SendSignal(clientId, 0, 0x11A);

		Users[clientId].Status = USER_SELCHAR;
	}
	break;
	case 0x41E:
	{
		SendSignal(clientId, 0, 0x11B);

		Users[clientId].Status = USER_SELCHAR;
	}
	break;
	case 0x40A:
	{// 0044E5DD
		MSG_DBSavingQuit *p = (MSG_DBSavingQuit *)(header);
		if (Users[clientId].Status != USER_PLAY && Users[clientId].Status != USER_SAVING4QUIT)
		{
			pMsgSignal packet;
			packet.Header.PacketId = 0x805;
			packet.Header.ClientId = clientId;

			AddMessageDB((BYTE*)&packet, 12);
		}

		if (Users[clientId].Status != USER_PLAY || Users[clientId].Status == USER_SELCHAR)
		{
			if (p->Mode == 0)
				SendClientMessage(clientId, g_pLanguageString[_NN_Your_Account_From_Others]);
			else if (p->Mode == 1)
				SendClientMessage(clientId, g_pLanguageString[_NN_Disabled_Account]);

			Users[clientId].SendMessageA();
		}

		CloseUser(clientId);
	}
	break;
	case 0x40B:
	{
		Mob[clientId].Mode = 0;
		Users[clientId].Status = USER_ACCEPT;
		CloseUser(clientId);
	}
	break;
	case 0x420:
	{
		SendSignal(clientId, 0x7532, 0x11D);

		Users[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x421:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_No_Account_With_That_Name]);

		Users[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;

	case 0x422:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Password]);

		AddFailAcount(Users[clientId].User.Username);

		LogPlayer(clientId, "Senha incorreta enviada");

		Users[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x424:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Blocked_Account]);

		Users[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;

	case 0x419:
	{
		p112 *p = (p112*)(header);

		header->PacketId = 0x112;
		header->ClientId = 0x7531;

		Users[clientId].AddMessage((BYTE*)header, header->Size);

		// Copia a estrutura da charlist para o User
		memcpy(&Users[clientId].CharList, &p->CharList, sizeof p->CharList);


		Users[clientId].Status = USER_SELCHAR;
	}
	break;
	case 0x426:
	{
		SendClientMessage(clientId, "Servidor em manutenção. Verifique o site para maiores infos.");

		Users[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x425:
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Disabled_Account]);

		Users[clientId].SendMessageA();
		CloseUser(clientId);
	}
	break;
	case 0x41F:
	{
		//	SendSignal(clientId, 0x7532, 0x11C);
		SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Password]);

		Users[clientId].Status = USER_SELCHAR;
		//	Users[clientId].SendMessageA();
		//	CloseUser(clientId);
	}
	break;
	case 0xFDF:
	{
		CUser *user = &Users[clientId];

		user->inGame.incorrectNumeric++;
		if (user->inGame.incorrectNumeric >= 4)
		{
			CloseUser(clientId);

			return false;
		}

		header->ClientId = 0;

		Log(clientId, LOG_INGAME, "Errou senha numérica: %d.", user->inGame.incorrectNumeric);
		Users[clientId].AddMessage(pBuffer, size);
	}
	break;

	case 0x820:
		return Users[clientId].GetCharInfo(header);

	case 0x417:
		Users[clientId].SendCharToWorld(header);
		break;

	case 0x80E:
	{
		pMsgSignal *p = (pMsgSignal*)(header);

		if (p->Value == 1)
			sServer.Sapphire <<= 1;
		else
			sServer.Sapphire >>= 1;

		if (sServer.Sapphire < 1)
			sServer.Sapphire = 1;

		if (sServer.Sapphire > 35)
			sServer.Sapphire = 35;
	}
	break;
	// SignalParam charlist
	case 0x423:
	{
		pMsgSignal2 *p = (pMsgSignal2*)(pBuffer);

		if (p->Value == 1)
			sServer.Sapphire = p->Value2;
		else if (p->Value == 2)
			sServer.FirstKefra = p->Value2;
	}
	break;
	case 0x427:
	{
		SendClientMessage(header->ClientId, g_pLanguageString[_NN_Wrong_Password]);

		return true;
	}
	break;

	case 0x416:
	{
		Users[clientId].SendCharList(header);
		break;
	}
	case 0x418:
		Users[clientId].ResendCharList(header);
		break;

	
	case SealInfoPacket:
	{
		pDC3 *packet = reinterpret_cast<pDC3*>(header);

		if (packet->Info.Status == -1)
		{
			CloseUser(clientId);

			return true;
		}

		auto sealIt = std::find_if(std::begin(sServer.SealCache), std::end(sServer.SealCache), [&](const InfoCache<SealInfo> seal) {
			return seal.Info.Status == packet->Info.Status;
		});

		if (sealIt == std::end(sServer.SealCache))
		{
			InfoCache<SealInfo> cache{};
			cache.Info = packet->Info;
			cache.Last = std::chrono::steady_clock::now();

			sServer.SealCache.push_back(cache);
		}
		else
		{
			sealIt->Last = std::chrono::steady_clock::now();
			sealIt->Info = packet->Info;
		}

		packet->Header.PacketId = 0xDC3;
		packet->Header.ClientId = SERVER_SIDE;

		Users[clientId].AddMessage(reinterpret_cast<BYTE*>(packet), packet->Header.Size);
		return true;
	}
	break;
	case PutInSealSuccess:
		Log(clientId, LOG_INGAME, "Foi criado o Selo da Alma do personagem. Removendo todas as suas informações");

		Mob[clientId].Mobs = stCharInfo{};
		break;
	case 0xE12:
	{
		pE12 *p = (pE12*)(header);
		if (IsWarTime())
		{
			SendClientMessage(clientId, "Não é possível realizar em horário de guerra");

			return true;
		}

		DoAlly(p->GuildIndex1, p->GuildIndex2);
		break;
	}
	/*
case MSG_REWARDWARTOWER_OPCODE:
{
	_MSG_REWARDWARTOWER *p = (_MSG_REWARDWARTOWER*)(header);

	g_pCityZone[4].impost += p->Gold;
	sServer.CitizenEXP.Bonus += p->Taxe;

	if(sServer.CitizenEXP.Bonus > MAX_CITIZENXP)
		sServer.CitizenEXP.Bonus = MAX_CITIZENXP;
}
break;

case _MSG_STARTTOWERWAR:
	{
		MSG_STARTTOWERWAR *p = (MSG_STARTTOWERWAR*)(header);

		// Pacote de início da guerra
		// Informa a situação e se está começando
		if (!p->isStarting)
			FinalizeTowerWar(p->war);
		else if (p->isStarting == 1)
			InitializeTowerWar(p->war);
		else if(p->isStarting == 2)
			UpdateTowerWar(p->war);

		break;
	}

case MSG_UPDATEWARDECLARATION:
{
	_MSG_UPDATEWARANSWER *p = (_MSG_UPDATEWARANSWER*)(header);

	// Pega o canal atual
	// Deve ser decrementado um pois está trabalhando de acordo com o 'conn' da DBsrv
	// que trabalha com índices de 0~9
	int thisChannel = sServer.Channel - 1;

	// Caso o canal que deve receber a informação seja o canal que recebeu este pacote
	// e a atual situação seja diferente, seta o novo valor
	// WarState = 1 : guerra declarada
	// WarState = 0 : guerra não declarada
	if (p->receiver == thisChannel && sServer.TowerWar[sServer.Channel - 1].WarState != p->action)
		sServer.TowerWar[sServer.Channel - 1].WarState = p->action;

	char szMsg[120];

	// Soma somente para fins de mensagem
	BYTE declarant = p->declarant + 1,
		 receiver  = p->receiver  + 1;

	if (p->action)
		sprintf_s(szMsg, "O canal %d declara guerra ao canal %d!", declarant, receiver);
	else
		sprintf_s(szMsg, "O canal %d recua!", declarant);

	SendNotice(szMsg);
	break;
}*/

	case MSG_SEND_SERVER_NOTICE:
	{
		_MSG_SEND_SERVER_NOTICE *p = (_MSG_SEND_SERVER_NOTICE*)(header);

		if (strlen(p->Notice) > 0)
			SendNotice(p->Notice);

		break;
	}

	case MSG_FIRST_KEFRA_NOTIFY:
	{
		int thisChannel = sServer.Channel - 1;

		_MSG_FIRST_KEFRA_NOTIFY *p = (_MSG_FIRST_KEFRA_NOTIFY*)(header);
		if (thisChannel != p->Channel)
			sServer.FirstKefra = FALSE;
		else
			sServer.FirstKefra = TRUE;

		break;
	}

	case MSG_REBORN_KEFRA:
		RebornKefra();
		break;

	case 0xFDE:
		Users[clientId].SendNumericToken(header);
		break;

	}
	return true;
}
/*
	if(Header->PacketId == 0x3c9)
	{
	char tmp [ 1024 ];
	sprintf_s(tmp, "packet//pacote_%0X.txt", Header->PacketId);

	FILE *pFile = NULL;
	fopen_s(&pFile, tmp, "a+");

	if(pFile)
	{
		fprintf(pFile, "Send -> Packet: 0x%03X Size : %d ClientId: %d\n",Header->PacketId,Header->Size,Header->ClientId);

		fprintf(pFile, "   %05d   ", 0);
		for(int i=0; i < Header->Size;i++)
		{
			fprintf(pFile, "%02X ", pBuffer[i]);
			if(i != 0 && (i + 1) % 10 == 0)
			{
				fprintf(pFile, "  ");
				for(int x = (i - 10); x < i; x++)
				{
					if(pBuffer[x]  == 0)
						fprintf(pFile, ".");
					else
						fprintf(pFile, "%c", pBuffer[x]);
				}

				fprintf(pFile, "\n");
				fprintf(pFile, "   %05d   ", (i + 1));
			}
		}

		fprintf(pFile, "\n\n");
		fclose(pFile);
	}
	}
	*/

bool CUser::PacketControl(BYTE *pBuffer, INT32 size)
{
	PacketHeader *Header = (PacketHeader*)pBuffer;
	BYTE hash = GetHashKey();
	//0042074C  |. 0F84 43010000  JE TMSRV.00420895
	if (Keys[0] != 0)
	{
		INT32 ebp_24 = 0;

		if (hashIncrement > 15)
		{
			//	if(sServer.Encode
		}
	}
	/*
	if (((KeyTable[hash * 2]) & 255) != Header->Key && hash != 0 && Header->Key != hash)
	{
		Log(clientId, LOG_HACK, "Checksum Fail: %02X- %d", Header->PacketId, Header->Size);
		Log(SERVER_SIDE, LOG_HACK, "Checksum Fail: %X - %02X - %s", Header->PacketId, Header->Size, User.Username);

		printf("Error hashkey HashTable: %02X Hash: %02X - Hash packet: %02X - 0x%X\n", ((KeyTable[hash * 2]) & 255), hash, Header->Key, Header->PacketId);

		return false;
	}
	*/
	if (!CheckPacket(Header) && Header->PacketId != 0x334)
	{
		AddCrackError(clientId, 4, CRACK_USER_PKTHACK);

		Log(clientId, LOG_HACK, "CheckPacket Fail: %X - %d", Header->PacketId, Header->Size);
		return true;
	}

	if(TimeStamp.TimeStamp != 0x0E0A1ACA && Header->TimeStamp != 0x0E0A1ACA)
		Header->ClientId = clientId;

	TimeStamp.LastReceiveTime = sServer.SecCounter;

	if (Header->PacketId == 0x3AE)
		return true;

	switch (Header->PacketId)
	{
	case 0x655:
	case 0x656:
		return RequestClientInfo(Header);

	case DailyRewardCollectPacket:
		return RequestCollectDaily(Header);

	case 0x3E8:
		return RequestRepurchase(Header);



	case 0x20D:
		return RequestLogin(Header);
	case 0x20F:
		return RequestCreateChar(Header);

	case 0xFDE:
		return Msg_Numeric(Header);

	case 0x36C:
	case 0x366:
	case 0x368:
		return RequestAction(Header);
	case 0x37A:
		return RequestSellShop(Header);
	case 0x39E:
	case 0x39D:
	case 0x367:
		return RequestAttack(Header);

	case AskStoreInfoPacket:
		return RequestDonateShop(Header);

	case BuyStorePacket:
		return RequestBuyDonateShop(Header);

	case 0x2CB:
		return true;

	case 0x215:
		Log(clientId, LOG_INGAME, "Personagem deu personagem");

		CharLogOut(clientId);
		break;

	case 0x906:
		return RequestFriendListUpdate(Header);
	case 0x903:
		return RequestBlockItems(Header);
	case 0x333:
		return RequestChat(Header);
	case 0x334:
		return RequestCommand(Header);
	case 0x290:
		return RequestTeleport(Header);
	case 0x376:
		return RequestMoveItem(Header);
	case 0x397:
		return RequestCreateAutoTrade(Header);
	case 0x398:
		return RequestBuyAutoTrade(Header);
	case 0x39A:
		return RequestOpenAutoTrade(Header);
	case 0x384:
		return RequestCloseAutoTrade(Header);
	case 0x291:
		return RequestChangeCity(Header);
	case 0x277:
		return RequestAddPoint(Header);
	case 0x27B:
		return RequestOpenShop(Header);
	case 0x379:
		return RequestBuyShop(Header);
	case 0x374:
		return RequestOpenGate(Header);
	case 0x289:
		return RequestRessuctPlayer(Header);
	case 0x16B:
		return RequestPremierStore(Header);
	case 0x377:
		return RequestPremierStoreBuy(Header);

	case 0x369:
	{
		pMsgSignal *p = (pMsgSignal*)(Header);
		INT32 mobId = p->Value;
		if (mobId <= 0 || mobId >= MAX_SPAWN_MOB)
			return true;

		if (Mob[mobId].Mode == 0)
		{
			SendRemoveMob(clientId, mobId, 0, 0);

			return true;
		}

		if (mobId < MAX_PLAYER && Users[mobId].Status != USER_PLAY)
		{
			SendRemoveMob(clientId, mobId, 0, 0);

			return true;
		}

		if (!GetInHalf(mobId, clientId))
		{
			SendRemoveMob(clientId, mobId, 0, 0);

			return true;
		}

		SendCreateMob(clientId, mobId, 1);
	}
	break;
	case 0x36A:
		return RequestMotion(Header);

	case 0x3A6:
		return RequestCompounder(Header);

	case 0x2C3:
		return RequestLindy(Header);

	case 0x2D2:
		return RequestOdin(Header);

	case 0x2D3:
		return RequestEhre(Header);

	case 0x2C4:
		return RequestShany(Header);

	case 0x3BA:
		return RequestAgatha(Header);

	case 0x3B5:
		return RequestAylin(Header);

	case 0x3C0:
		return RequestTiny(Header);

	case 0x3D5:
		return RequestRecruitMember(Header);

	case 0x28C:
		return RequestKickMember(Header);

	case 0x213:
	{
#if !defined(_DEBUG)
		if (TokenOk)
		{
			if (Status != USER_SELCHAR)
				return true;

			Header->ClientId = clientId;
			Header->PacketId = 0x804;
			Log(clientId, LOG_INGAME, "Solicitou ir ao mundo...");

			return AddMessageDB((BYTE*)Header, sizeof p213);
		}
		else
		{
			SendSignal(clientId, 0x7530, 0xFDF);
			SendClientMessage(clientId, "Digite a senha numérica");
		}
		return true;
#else
		if (Status != USER_SELCHAR)
			return true;

		Header->PacketId = 0x804;
		Log(clientId, LOG_INGAME, "Solicitou ir ao mundo...");

		return AddMessageDB((BYTE*)Header, Header->Size);
#endif
	}

	case 0x2D4:
		return RequestExtraction(Header);
	case 0x211:
		return RequestDeleteChar(Header);

	case 0x378:
		return RequestChangeSkillbar(Header);

	case 0x37F:
		return RequestAddParty(Header);

	case 0x3AB:
		return RequestAcceptParty(Header);

	case 0x37E:
		return RequestExitParty(Header);

	case 0x373:
		return RequestUseItem(Header);

	case 0x272:
		return RequestDropItem(Header);

	case 0x2E4:
		return RequestDeleteItem(Header);

	case 2568:
	{
		auto Reg = (pMsgSignal*)(Header);

		if (Users[clientId].Status == USER_PLAY)
		{
			if (Users[clientId].LastTimeRequestDrop + 7000 > CurrentTime)
				return 0;
			
			SendDropList(clientId);
	    }

	}break;

	case 0x270:
		return RequestPickItem(Header);

	case 0x399:
	{
		pMsgSignal *p = (pMsgSignal*)(Header);

		AllStatus.PK = p->Value;

		if (Trade.ClientId)
		{
			if (Users[Trade.ClientId].Status == USER_PLAY && Users[Trade.ClientId].Trade.ClientId == clientId)
			{
				SendClientMessage(clientId, "Não é possível trocar com o modo PvP ativo");
				SendClientMessage(Trade.ClientId, "O outro jogador está com o modo PvP ativo");

				RemoveTrade(Trade.ClientId);
				RemoveTrade(clientId);
			}
		}

	}
	break;

	case 0x383:
		return RequestTrade(Header);

	case 0x387:
		return RequestTransferGoldToInv(Header);

	case 0x388:
		return RequestTransferGoldToBank(Header);

	case 0x39F:
		return RequestDuel(Header);

	case 0x28B:
		return RequestMerchantNPC(Header);

	case 0x2E5:
		return RequestUngroupItem(Header);

	case 0xAD9:
		return RequestGriffinMaster(Header);

	case 0xE12:
		return RequestAlly(Header);

	case 0x3A0:
		break;

	case 0x28E:
		return RequestReqChallange(Header);

		// Provavelmente desativado
		//case 0x18D:
			//return RequestChallange(Header);

	case MSG_PANELGUILD_GETLIST:
		AddMessageDB((BYTE*)Header, size);
		break;
	case 0xED7:
		return RequestDeclareWar(Header);

	case 0x2E1:
		return RequestAlchemy(Header);

	case Msvfw32IntegrityPacket:
		return RequestMsvfw32Info(Header);
	case ModulesInfoPacket:
		return HandleModulesInfo(Header);

	case MacAddressIntegrityPacket:
		return HandleMacAddressIntegrity(Header);

	case 0x2C7:
		return static_cast<TOD_Quiz*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::Quiz))->HandlePacket(*this, reinterpret_cast<MSG_QUIZ_ANSWER*>(Header));

	case ChristmasMissionReplyPacket:
	{
		auto christmasMission = static_cast<TOD_ChristmasMission*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::ChristmasMission));
		if (christmasMission != nullptr)
			return christmasMission->HandleReply(*this, reinterpret_cast<pMsgSignal*>(Header));

		return true;
	}

	case AutoPartyAddRemoveNamePacket:
		return RequestAutoPartyAddRemoveName(Header);

	case AutoPartySetPasswordPacket:
		return RequestAutoPartySetPassword(Header);

	case AutoPartyDisableEnableAllPacket:
		return RequestAutoPartyEnableAll(Header);

	case AutoPartyEnterPartyPacket:
		return RequestAutoPartyEnterParty(Header);

	case MissionAcceptOrRejectPacket:
		return RequestAcceptRejectMission(Header);

	case MissionCollectPacket:
		return RequestCollectMission(Header);

	case 0x2CD:
		return RequestSealInfo(Header);

	case 0x3CC:
		return RequestPutOutSeal(Header);

	case MSG_RVRSTORE_BUY_OPCODE:
		return RequestBuyRvRShop(Header);

	case UseEssencePacket:
		return RequestEssenceUse(Header);

	case RecruitAcceptPacket:
		return RequestAcceptRecruit(Header);

	case 0x3776:
	{
		p376* p = reinterpret_cast<p376*>(Header);
		AddMessage(reinterpret_cast<BYTE*>(Header), Header->Size);
		break;
	}
	case NightmareAcceptPacket:
		return RequestNightmareAccept(Header);
	case RedeemGriffinPacket:
		return RequestRedeemGriffin(Header);
	case WPEOnWarnPacket:
		CloseUser(clientId);
		break;

	default:
		printf("Pacote desconhecido recebido - 0x%X - Size: %d (%s)\n", Header->PacketId, Header->Size, User.Username);
		break;
}

	return true;
}