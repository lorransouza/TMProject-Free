#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include <ctime>
#include <algorithm>
#include <sstream>

bool CUser::RequestCommand(PacketHeader *Header)
{
	p334 *p = (p334*)(Header);
	p->eCommand[15] = '\0';
	p->eValue[99] = '\0';

	char szTMP[128];

	Log(clientId, LOG_INGAME, "Comando \"/%s %s\"", p->eCommand, p->eValue);

	if (Status != USER_PLAY && clientId != SCHEDULE_ID)
	{
		Log(clientId, LOG_INGAME, "Enviado comando estando fora da sessão de jogo... Sessão atual: %d", Status);

		return true;
	}

	CMob *mob = &Mob[clientId];
	if (!strcmp(p->eCommand, "day"))
		SendClientMessage(clientId, "!#11 2");
	else if (!strncmp(p->eCommand, "pontos", 6))
		SendClientMessage(clientId, "Você possui %d pontos de contribuição", mob->Mobs.Contribuition.Points);
	else if (!strncmp(p->eCommand, "novato", 6))
	{
		if (User.Unique.Novato)
		{
			SendClientMessage(clientId, "Você já recebeu o seu presente!");

			return true;
		}

		std::array items =
		{
			st_Item { 3995, EF_NOTRADE, 1 },
			st_Item { 3314, EF_AMOUNT, 3, EF_NOTRADE, 1 },
			st_Item { 4639, EF_NOTRADE, 1 }
		};

		if (static_cast<int>(GetInventoryAmount(clientId, 0)) < static_cast<int>(items.size()))
		{
			SendClientMessage(clientId, "Não possui espaço no inventário");

			return true;
		}

		for (auto& item : items)
		{
			if (PutItem(clientId, &item))
				SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item.Index].Name);
			else
				SendClientMessage(clientId, "!Falha ao receber %s", ItemList[item.Index].Name);
		}

		Log(clientId, LOG_INGAME, "Digitou o comando /novato. Novato.");
		User.Unique.Novato = 1;
	}

	if (!strcmp(p->eCommand, "xLojaDonateX"))
	{
		int Type = 0;
		int Page = 0;
		int Slot = 0;
		if (sscanf_s(p->eValue, "%d %d %d", &Type, &Page, &Slot))
		{
			if (Type != -1 && Page != -1 && Slot != -1)
				BuyItem(clientId, Type, Page, Slot);
		}
		return true;
	}

	else if (!strncmp(p->eCommand, "tgp", 3))
	{
		INT32 mobId = GetUserByName(p->eValue);
		if (mobId <= 0)
		{
			SendClientMessage(clientId, "O usuário não está conectado.");

			return true;
		}

		if (Mob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			return true;
		}

		INT32 i = 0;
		for (; i < 12; i++)
		{
			if (Mob[clientId].PartyList[i] == mobId)
				break;
		}

		if (i == 12)
		{
			SendClientMessage(clientId, "O membro deve ser do seu grupo");

			return true;
		}

		GroupTransfer(clientId, mobId);
	}
	else if (!strcmp(p->eCommand, "srv"))
	{
		INT32 channel = atoi(p->eValue);

		if (sServer.Channel == channel)
		{
			SendClientMessage(clientId, "Você já está neste canal");

			return true;
		}

		MSG_DBServerChange packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0x814;
		packet.Header.Size = sizeof MSG_DBServerChange;
		packet.Header.ClientId = clientId;

		packet.NewServerID = channel;
		packet.Slot = inGame.CharSlot;

		AddMessageDB((BYTE*)&packet, sizeof MSG_DBServerChange);

		Log(clientId, LOG_INGAME, "Solicitou trocar para o servidor %d", channel);
		return true;
	}
	else if (!strcmp(p->eCommand, "handover"))
	{
		// Retorna o tempo para o pesadelo
		time_t nowraw;
		struct tm now;

		nowraw = time(NULL);
		localtime_s(&now, &nowraw);

		int guildId = Mob[clientId].Mobs.Player.GuildIndex;
		if (!guildId)
		{
			SendClientMessage(clientId, "Necessário possuir uma guild.");
			return true;
		}

		int mobId = GetUserByName(p->eValue);
		if (mobId == 0)
		{
			SendClientMessage(clientId, "O usuário não está conectado.");
			return true;
		}

		if (Mob[mobId].Mobs.Player.GuildIndex != guildId)
		{
			SendClientMessage(clientId, "Necessário ser da mesma guilda.");
			return true;
		}

		if (now.tm_wday == DOMINGO)
		{
			SendClientMessage(clientId, "Não é possível transferir no domingo");

			return true;
		}

		if (Mob[clientId].Mobs.Player.GuildMemberType != 9)
		{
			SendClientMessage(clientId, "Transferência habilitada apenas para líderes de guild.");

			return true;
		}

		SetGuildFame(guildId, 0);

		auto memberType = Mob[mobId].Mobs.Player.GuildMemberType;
		std::swap(Mob[clientId].Mobs.Player.GuildMemberType, Mob[mobId].Mobs.Player.GuildMemberType);

		SendClientMessage(clientId, "Medalha transferida");
		SendClientMessage(mobId, "Medalha transferida");

		Log(clientId, LOG_INGAME, "Medalha transferida para %s GuildID : %d. Fame perdida.", Mob[mobId].Mobs.Player.Name, guildId);
		Log(mobId, LOG_INGAME, "Medalha recebida de %s - GuildID: %d. Fame perdida", Mob[clientId].Mobs.Player.Name, guildId);

		MulticastGetCreateMob(clientId);
		MulticastGetCreateMob(mobId);
	}
	else if (!strcmp(p->eCommand, "fimirma"))
	{
		INT32 guildId = Mob[clientId].Mobs.Player.GuildIndex;
		if (guildId <= 0 || guildId >= MAX_GUILD || Mob[clientId].Mobs.Player.GuildMemberType != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_can]);

			return true;
		}

		INT32 guildAlly = g_pGuildAlly[guildId];
		if (guildAlly <= 0 || guildAlly >= MAX_GUILD)
		{
			SendClientMessage(clientId, "Você não possui aliança");

			return true;
		}

		if (IsWarTime())
		{
			SendClientMessage(clientId, "Não é possível realizar em horário de guerra");

			return true;
		}

		pE12 packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0xE12;
		packet.Header.Size = sizeof pE12;

		packet.GuildIndex1 = guildId;
		packet.GuildIndex2 = 0;

		AddMessageDB((BYTE*)&packet, sizeof pE12);
	}
	else if (!strncmp(p->eCommand, "war", 3))
	{/*
		INT32 conn		= sServer.Channel - 1;
		INT32 otherConn = ((conn % 2) ? (conn - 1) : (conn + 1));

		if(sServer.TowerWar[conn].WarState == 1)
			SendClientMessage(clientId, "O canal %d está em guerra com o canal %d", otherConn + 1, conn + 1);
		else if(sServer.TowerWar[conn].WarState == 2)
			SendClientMessage(clientId, "O canal %d precisa reconquistar o próprio canal", conn);

		if(sServer.TowerWar[otherConn].WarState == 1)
			SendClientMessage(clientId, "O canal %d está em guerra com o canal %d", conn + 1, otherConn + 1);
		else if(sServer.TowerWar[otherConn].WarState == 2)
			SendClientMessage(clientId, "O canal %d precisa reconquistar o próprio canal", otherConn);*/
	}
	else if (!strcmp(p->eCommand, "rvr"))
	{
		SendClientMessage(clientId, "Você possui %d pontos", Mob[clientId].Mobs.RvRPoints);
	}
	else if (!strcmp(p->eCommand, "info"))
	{
		if (Mob[clientId].Mobs.Player.Equip[0].EFV2 <= 2)
			return true;

		sprintf_s(szTMP, "Você está no %s e possui %d resets.", (Mob[clientId].Mobs.Player.Equip[0].EFV2 == 4) ? "Subcelestial" : "Celestial", Mob[clientId].Mobs.GetTotalResets());
		SendClientMessage(clientId, szTMP);
	}
	else if (!strncmp(p->eCommand, "lock", 4))
	{
		if (strcmp(p->eValue, User.Block.Pass))
		{
			SendClientMessage(clientId, "Senha incorreta");

			return true;
		}

		User.Block.Blocked = !User.Block.Blocked;
		SendClientMessage(clientId, (User.Block.Blocked) ? "Seus itens foram bloqueados" : "Seus itens foram desbloqueados");

		Log(clientId, LOG_INGAME, (User.Block.Blocked) ? "Seus itens foram bloqueados" : "Seus itens foram desbloqueados");
	}
	else if (!strncmp(p->eCommand, "entrar", 5))
	{
		char nickname[16] = { 0 };
		char password[16] = { 0 };

		int ret = sscanf_s(p->eValue, "%s %s", nickname, 16, password, 16);
		if (ret != 2)
		{
			SendClientMessage(clientId, "Digite o nome do personagem e a senha");

			return true;
		}

		MSG_AUTOPARTY_ENTERPARTY packet{};
		packet.Header.PacketId = AutoPartyEnterPartyPacket;
		
		strncpy_s(packet.Nickname, nickname, 12);
		strncpy_s(packet.Password, password, 12);

		PacketControl(reinterpret_cast<BYTE*>(&packet), sizeof packet);
		return true;
	}	
	else if (!strncmp(p->eCommand, "criar", 6) || !strncmp(p->eCommand, "create", 6))
	{
		INT32 lastIndex = -1;

		for (INT32 i = 1; i < MAX_GUILD; i++)
		{
			if (g_pGuild[i].Name.empty())
			{
				lastIndex = i;
				break;
			}
		}

		if (lastIndex <= 0 || lastIndex >= MAX_GUILD)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Limit]);

			return true;
		}

		st_Mob *player = (st_Mob*)&mob->Mobs.Player;

		int guildTicket = GetFirstSlot(clientId, 4614);
		if (guildTicket == -1 && player->Gold < 100000000)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Need100mGold]);

			return true;
		}

		if (mob->Mobs.Player.GuildIndex != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_YouHaveAGuild]);

			return true;
		}

		if (player->CapeInfo != 7 && player->CapeInfo != 8)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_NeedKingdom]);

			return true;
		}

		std::time_t rawnow = std::time(nullptr);
		struct tm now; localtime_s(&now, &rawnow);

		auto now_time_t = std::mktime(&now);
		auto diffTime = std::difftime(now_time_t, Mob[clientId].Mobs.LastGuildKickOut.GetTMStruct());

		if (diffTime < KickOutPenalty && diffTime != 0.0)
		{
			SendClientMessage(clientId, "Você não pode criar uma guild ainda");

			return true;
		}

		INT32  i = 0;
		for (; i < MAX_GUILD; i++)
		{
			if (_stricmp(g_pGuild[i].Name.c_str(), p->eValue) == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Guild_GuildName]);

				return true;
			}
		}

		if (Trade.ClientId != 0)
		{
			RemoveTrade(clientId);

			AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
			return true;
		}

		MSG_CREATEGUILD packet;
		memset(&packet, 0, sizeof packet);
		packet.Header.ClientId = clientId;
		packet.Header.PacketId = MSG_CREATEGUILD_OPCODE;
		packet.Header.Size = sizeof MSG_CREATEGUILD;

		packet.kingDom = player->CapeInfo;
		packet.citizen = sServer.Channel;
		packet.guildId = lastIndex;

		strncpy_s(packet.GuildName, p->eValue, 16);

		AddMessageDB((BYTE*)&packet, sizeof MSG_CREATEGUILD);

		// Atribui o guildIndex
		player->GuildIndex = lastIndex;

		// Retira o gold
		if(guildTicket == -1)
			player->Gold -= 100000000;
		else
		{
			AmountMinus(&Mob[clientId].Mobs.Player.Inventory[guildTicket]);

			SendItem(clientId, SlotType::Inv, guildTicket, &Mob[clientId].Mobs.Player.Inventory[guildTicket]);
		}

		player->GuildMemberType = 9; // líder

		// Atualiza o gold
		SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

		p364 packetMob;
		GetCreateMob(clientId, (BYTE*)&packetMob);

		GridMulticast_2(mob->Target.X, mob->Target.Y, (BYTE*)&packetMob, 0);

		// Atribu os valores da guild a estrutura
		g_pGuild[lastIndex].Name = std::string(p->eValue);

		g_pGuild[lastIndex].Citizen = sServer.Channel;
		g_pGuild[lastIndex].Kingdom = player->CapeInfo;

		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_SuccessCreate]);
		Log(clientId, LOG_INGAME, "Criou a guilda %s no canal %d. Reino %d. GuildIndex: %d", p->eValue, sServer.Channel, player->CapeInfo, lastIndex);
	}
	else if (!strcmp(p->eCommand, "Reino"))
	{
		int slotId = GetFirstSlot(clientId, 699);
		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 776);

		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 3430);

		if (slotId == -1)
		{
			SendClientMessage(clientId, "Necessário ter um pergaminho do portal no inventário.");
			return true;
		}

		AmountMinus(&Mob[clientId].Mobs.Player.Inventory[slotId]);
		SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);

		INT32 reino = Mob[clientId].Mobs.Player.CapeInfo;
		if (reino != CAPE_BLUE && reino != CAPE_RED)
			Teleportar(clientId, 1760, 1726);
		else if (reino == CAPE_BLUE)
			Teleportar(clientId, 1690, 1617);
		else if (reino == CAPE_RED)
			Teleportar(clientId, 1690, 1835);

		Log(clientId, LOG_INGAME, "Usado o comando /Reino");
	}
	else if (!strcmp(p->eCommand, "kingdom"))
	{
		INT32 capeInfo = Mob[clientId].Mobs.Player.CapeInfo;

		if (capeInfo == CAPE_BLUE)
			Teleportar(clientId, 1690, 1617);
		else if (capeInfo == CAPE_RED)
			Teleportar(clientId, 1690, 1835);

		Log(clientId, LOG_INGAME, "Usado comando /kingdom");
	}
	else if (!strcmp(p->eCommand, "summonguild"))
	{
		INT32 guildId = Mob[clientId].Mobs.Player.GuildIndex,  // LOCAL_1673
			type = Mob[clientId].Mobs.Player.GuildMemberType, // LOCAL_1674
			posX = Mob[clientId].Target.X,  // LOCAL_1675
			posY = Mob[clientId].Target.Y, // LOCAL_1676
			village = GetVillage(posX, posY); // LOCAL_1677

		MapAttribute map = GetAttribute(posX, posY);
		if (type != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_Can]);

			return true;
		}

		if (map.CantSummon || map.PvP || village == 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

			return true;
		}

		SummonGuild(guildId, posX, posY, MAX_PLAYER);
	}
	else if (!strcmp(p->eCommand, "gnotice"))
	{
		INT32 guildId = Mob[clientId].Mobs.Player.GuildIndex,
			type = Mob[clientId].Mobs.Player.GuildMemberType;

		if (!guildId)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		if (type != 9 && (type < 3 || type > 5))
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_Can]);

			return true;
		}

		if (p->eValue[0])
			strncpy_s(g_pGuildNotice[guildId], p->eValue, 99);
		else
		{
			g_pGuildNotice[guildId][0] = 0;

			SendClientMessage(clientId, "Aviso foi removido");
			return true;
		}

		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY || Mob[i].Mobs.Player.GuildIndex != guildId)
				continue;

			SendChatGuild(i, guildId, "--Aviso: %s", g_pGuildNotice[guildId]);
		}
	}
	else if (!strcmp(p->eCommand, "gmsg"))
	{
		INT32 guildId = Mob[clientId].Mobs.Player.GuildIndex,
			type = Mob[clientId].Mobs.Player.GuildMemberType;

		if (!guildId)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		if (type != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Master_Can]);

			return true;
		}

		INT32 msgLen = strlen(p->eValue);
		if (msgLen <= 0)
		{
			SendClientMessage(clientId, "Escreva uma mensagem");

			return true;
		}

		SendGuildNotice(guildId, "[%s]> %s", Mob[clientId].Mobs.Player.Name, p->eValue);
		Log(clientId, LOG_GUILD, "Enviado GuildNotice. Fame: %d - %s", g_pGuild[guildId].Fame, p->eValue);
		return true;
	}
	else if (!strcmp(p->eCommand, "king"))
	{
		INT32 capeInfo = Mob[clientId].Mobs.Player.CapeInfo;

		if (capeInfo == 7)
			Teleportar(clientId, 1748, 1574);
		else if (capeInfo == 8)
			Teleportar(clientId, 1748, 1880);

		Log(clientId, LOG_INGAME, "Usado comando /king");
	}
	else if (!strcmp(p->eCommand, "teste1234"))
	{
		Mob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);
	}


	else if (!strcmp(p->eCommand, "subcreate"))
	{
		char nick[100];
		char subname[100];

		int ret = sscanf_s(p->eValue, "%s %s", nick, 16, subname, 16);

		if (ret != 2)
		{
			SendClientMessage(clientId, "Comando inválido");

			return true;
		}

		if (Mob[clientId].Mobs.Player.GuildIndex == 0 || Mob[clientId].Mobs.Player.GuildMemberType != 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_With_Guild_Master]);

			return true;
		}

		int client = GetUserByName(nick);
		if (client == 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);
			return true;
		}

		INT32 guildIndex = Mob[clientId].Mobs.Player.GuildIndex;
		if (Mob[client].Mobs.Player.GuildIndex != guildIndex)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		if (Mob[clientId].Mobs.Player.Gold < 50000000)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NotEnoughGold_Teleport]);

			return true;
		}

		int i = -1;
		for (i = 0; i < 3; i++)
		{
			if (!g_pGuild[guildIndex].SubGuild[i][0])
				break;
		}

		if (i == 3)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_MaxSub]);

			return true;
		}

		Mob[client].Mobs.Player.GuildMemberType = 3 + i;

		g_pGuild[guildIndex].SubGuild[i] = subname;

		p364 packetMob;
		GetCreateMob(client, (BYTE*)&packetMob);

		GridMulticast_2(Mob[client].Target.X, Mob[client].Target.X, (BYTE*)&packetMob, 0);

		MSG_ADDSUB packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = MSG_ADDSUB_OPCODE;
		packet.Header.ClientId = clientId;
		packet.Header.Size = sizeof MSG_ADDSUB;

		packet.SubIndex = i;
		packet.GuildIndex = guildIndex;

		strncpy_s(packet.Name, subname, 16);

		packet.Status = 0;

		AddMessageDB((BYTE*)&packet, sizeof MSG_ADDSUB);

		SendClientMessage(clientId, g_pLanguageString[_NN_Guild_SubCreated]);
		SendClientMessage(client, g_pLanguageString[_NN_Guild_Sub_Called]);

		Mob[clientId].Mobs.Player.Gold -= 50000000;
		SendSignalParm(clientId, clientId, 0x3AF, Mob[clientId].Mobs.Player.Gold);
	}
	else if (!strcmp(p->eCommand, "expulsar"))
	{
		if (p->eValue[0])
		{
			SendClientMessage(clientId, "Para expulsar um jogador, utilize o menu!");

			return true;
		}

		std::time_t rawnow = std::time(nullptr);
		struct tm now; localtime_s(&now, &rawnow);

		INT32 guildIndex = Mob[clientId].Mobs.Player.GuildIndex;
		if (guildIndex <= 0 || guildIndex >= MAX_GUILD)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Need_Guild_Medal]);

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(now_time_t, Mob[clientId].Mobs.LastGuildKickOut.GetTMStruct());

			int totalSeconds = (KickOutPenalty - (int)diffTime);
			int hours = (totalSeconds / 3600) % 24;
			int days = (totalSeconds / 3600) / 24;
			int mins = (totalSeconds % 3600) / 60;
			int seconds = totalSeconds % 60;

			if (diffTime < KickOutPenalty && diffTime != 0.0)
			{
				SendClientMessage(clientId, "Você ainda tem %02d dias e %02d horas %02d minutos %02d segundos de penalidade", days, hours, mins, seconds);
				Log(clientId, LOG_INGAME, "Você ainda tem %02d dias e %02d horas %02d minutos %02d segundos de penalidade");
			}

			return true;
		}

		INT32 medalId = Mob[clientId].Mobs.Player.GuildMemberType;
		if (medalId >= 3 && medalId <= 5)
		{
			// Remove a medalha da estrutura dos subs
			g_pGuild[guildIndex].SubGuild[medalId - 3][0] = 0;

			// Retira da DBSrv e do restante dos canais
			MSG_ADDSUB packet;
			packet.Header.PacketId = MSG_ADDSUB_OPCODE;
			packet.Header.Size = sizeof MSG_ADDSUB;

			packet.GuildIndex = guildIndex;
			packet.Status = 1;
			packet.SubIndex = medalId - 3;

			AddMessageDB((BYTE*)&packet, sizeof MSG_ADDSUB);

			Mob[clientId].Mobs.Player.GuildMemberType = 1;
			SendClientMessage(clientId, "Medallha de sublíder destruída. Agora você é um membro comum");
			Log(clientId, LOG_INGAME, "Destruiu medalha de sublíder da guilda %s. Id: %d.. Id da medalha de sub", g_pGuild[guildIndex].Name.c_str(), guildIndex, medalId);
		}
		else
		{
			Mob[clientId].Mobs.Player.GuildMemberType = 0;
			Mob[clientId].Mobs.Player.GuildIndex = 0;

			SendClientMessage(clientId, g_pLanguageString[_NN_Guild_Kicked]);

			auto& lastKick = Mob[clientId].Mobs.LastGuildKickOut;
			lastKick.Ano = 1900 + now.tm_year;
			lastKick.Mes = now.tm_mon + 1;
			lastKick.Dia = now.tm_mday;
			lastKick.Hora = now.tm_hour;
			lastKick.Minuto = now.tm_min;
			lastKick.Segundo = now.tm_sec;

			Log(clientId, LOG_INGAME, "Saiu da guilda %s. Id: %d.", g_pGuild[guildIndex].Name.c_str(), guildIndex);
		}

		p364 packet;
		GetCreateMob(clientId, (BYTE*)&packet);

		GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);
	}
	else if (!strcmp(p->eCommand, "spk") || !strcmp(p->eCommand, "gritar"))
	{
		INT32 slotId = GetFirstSlot(clientId, 3330);
		if (slotId == -1)
			return true;

		AmountMinus(&Mob[clientId].Mobs.Player.Inventory[slotId]);
		SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);

		pD1D packet;
		memset(&packet, 0, sizeof packet);

		packet.Header.PacketId = 0xD1D;
		packet.Header.ClientId = 0x7530;
		packet.Header.Size = sizeof pD1D;

		p->eValue[80] = 0;
		sprintf_s(packet.eMsg, "[%s] : %s", Mob[clientId].Mobs.Player.Name, p->eValue);

		packet.Header.ClientId = clientId;
		AddMessageDB((BYTE*)&packet, sizeof pD1D);

		Log(clientId, LOG_INGAME, "Trombeta: %s", p->eValue);
	}
	else if (!strcmp(p->eCommand, "evasao"))
	{
		int userId = GetUserByName(p->eValue);
		if (userId == 0)
		{
			SendClientMessage(clientId, "O usuário não está conectado");

			return true;
		}

		int typeOnUser = -2;
		if (Mob[clientId].Mobs.Player.Status.INT > Mob[clientId].Mobs.Player.Status.STR + Mob[clientId].Mobs.Player.Status.DEX)
			typeOnUser = -1;

		int typeOnMe = -2;
		if (Mob[userId].Mobs.Player.Status.INT > Mob[userId].Mobs.Player.Status.STR + Mob[userId].Mobs.Player.Status.DEX)
			typeOnMe = -1;

		float evasionOnUser = static_cast<float>(GetParryRate(clientId, userId, typeOnUser)) / 10.0f;
		float evasionOnMe = static_cast<float>(GetParryRate(userId, clientId, typeOnMe)) / 10.0f;

		SendClientMessage(clientId, "Você tem %.2f%% de errar o hit no usuário. Ele tem %.2f%% de errar em você", evasionOnUser, evasionOnMe);
		return true;
	}
	else if (strcmp(p->eCommand, "evento") == 0)
	{
		if (!sServer.DropArea.Status)
		{
			SendClientMessage(clientId, "Não existe evento de drop ativo no momento");

			return true;
		}

		if (!p->eValue[0])
		{
			SendClientMessage(clientId, "O evento nesta conta está %s", DropEvent.IsValid ? "ativado" : "desativado");

			return true;
		}

		std::vector<CUser*> users;
		for (auto& user : Users)
		{
			if (user.Status < USER_SELCHAR || user.clientId == clientId || memcmp(&user.MacAddress, MacAddress, sizeof MacAddress) != 0)
				continue;

			users.push_back(&user);
		}

		bool status = false;
		if (strcmp(p->eValue, "ativar") == 0)
			status = true;

		if (status)
		{
			if (DropEvent.IsValid)
			{
				SendClientMessage(clientId, "O evento já está ativo para esta conta");

				return true;
			}

			for (auto& user : users)
			{
				if (user->DropEvent.IsValid)
				{
					SendClientMessage(user->clientId, "!O evento foi desativado nesta conta");
					Log(user->clientId, LOG_INGAME, "O evento foi desativado pois a conta %s ativou", User.Username);
				}
/*
				// Caso queira ativar das duas uma, lojinha ou evento global
				if (user->EventAutoTrade.IsValid)
				{
					SendClientMessage(user->clientId, "!O evento foi desativado nesta conta");
					Log(user->clientId, LOG_INGAME, "O evento (de loja) foi desativado pois a conta %s ativou", User.Username);
				}

				user->EventAutoTrade.IsValid = false;
*/
				user->DropEvent.IsValid = false;
			}
		}

		DropEvent.IsValid = status;

		Log(clientId, LOG_INGAME, "O evento foi %s", status ? "ativado" : "desativado");
		SendClientMessage(clientId, "O evento foi %s", status ? "ativado" : "desativado");
	}
	else if (!strcmp(p->eCommand, "Davi")) // Trocar para o cmd de liberação
	{
		// Envia a mensagem de Personagem desconectado da mesma forma para "enganar"... Loucura, né?
		SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

		// Caso haja um AccessLevel setado, libera o admin
		if (Users[clientId].AccessLevel != 0)
			Users[clientId].IsAdmin = !Users[clientId].IsAdmin;

		return true;
	}
	else if (!strcmp(p->eCommand, "admin"))
	{
		if (!p->eValue[0])
		{
			SendClientMessage(clientId, "Utilize /admin +command");

			return true;
		}

		if (clientId == SCHEDULE_ID)
		{
			Users[clientId].AccessLevel = 100;
			Users[clientId].IsAdmin = true;
		}

		if (Users[clientId].AccessLevel == 0 || !Users[clientId].IsAdmin)
			return true;

		return HandleAdminCommand(p);
	}
	else if (p->eValue[0] == '@' && p->eCommand[0] == 0)
	{
		INT32 myKingdom = Mob[clientId].Mobs.Player.CapeInfo;
		INT32 myCitizen = Mob[clientId].Mobs.Citizen;

		strncpy_s(p->eCommand, Mob[clientId].Mobs.Player.Name, 12);

		auto last = std::chrono::high_resolution_clock::now() - citizenChatTime;
		if (last <= 5s && AccessLevel == 0)
		{
			SendClientMessage(clientId, "Tempo mínimo para enviar outra mensagem é de 5 segundos.");

			return true;
		}

		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY || i == clientId)
				continue;

			if (p->eValue[1] != '@')
			{
				if (Users[i].AllStatus.Kingdom)
					continue;

				if (Mob[i].Mobs.Player.CapeInfo == 0 || Mob[i].Mobs.Player.CapeInfo == myKingdom || myKingdom == 0 || Users[i].AccessLevel != 0)
					Users[i].AddMessage((BYTE*)p, sizeof p334);
			}
			else
			{
				if (Users[i].AllStatus.Citizen)
					continue;

				if (Mob[i].Mobs.Citizen == myCitizen || Users[i].AccessLevel != 0)
					Users[i].AddMessage((BYTE*)p, sizeof p334);
			}
		}

		Log(clientId, LOG_INGAME, "Mensagem enviada: %s", p->eValue);
		citizenChatTime = std::chrono::high_resolution_clock::now();
	}
	else if (p->eValue[0] == '=' && p->eCommand[0] == 0)
	{
		INT32 leader = Mob[clientId].Leader;
		if (leader <= 0)
			leader = clientId;

		strncpy_s(p->eCommand, Mob[clientId].Mobs.Player.Name, 12);
		for (INT32 i = 0; i < 12; i++)
		{
			INT32 memberId = Mob[leader].PartyList[i];
			if (memberId <= 0 || memberId >= MAX_PLAYER || Users[memberId].Status != USER_PLAY || Users[memberId].AllStatus.Chat || clientId == memberId)
				continue;

			Users[memberId].AddMessage((BYTE*)p, sizeof p334);
		}

		if (leader != clientId)
			Users[leader].AddMessage((BYTE*)p, sizeof p334);

		Log(clientId, LOG_INGAME, "Mensagem enviada: %s", p->eValue);
	}
	else if ((p->eValue[0] == '-' || (p->eValue[0] == '-' && p->eValue[1] == '-')) && p->eCommand[0] == 0)
	{
		INT32 myGuild = Mob[clientId].Mobs.Player.GuildIndex;
		if (!myGuild)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Guild_Member_Can]);

			return true;
		}

		strncpy_s(p->eCommand, Mob[clientId].Mobs.Player.Name, 12);
		for (INT32 i = 1; i < MAX_PLAYER; i++)
		{
			if (Users[i].Status != USER_PLAY || i == clientId || Users[i].AllStatus.Guild)
				continue;

			INT32 ally = g_pGuildAlly[myGuild];
			if (ally == 0)
				ally = -1;

			if (Mob[i].Mobs.Player.GuildIndex == myGuild || Users[i].IsAdmin)
				Users[i].AddMessage((BYTE*)p, sizeof p334);
			else if (Mob[i].Mobs.Player.GuildIndex == ally && p->eValue[1] == '-')
				Users[i].AddMessage((BYTE*)p, sizeof p334);
		}

		Log(clientId, LOG_INGAME, "Mensagem enviada: %s", p->eValue);
	}
	else if (!strcmp(p->eCommand, "not"))
	{
		if (AccessLevel == 0 || !IsAdmin)
			return true;

		SendNotice(p->eValue);
	}
	else if (!strcmp(p->eCommand, "nott"))
	{
		if (!AccessLevel || !IsAdmin)
			return true;

		if (strlen(p->eValue) > 0)
			SendServerNotice(p->eValue);
	}
	else if (!strcmp(p->eCommand, "tab"))
	{
		if (Mob[clientId].Mobs.Player.Status.Level < 69 && Mob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL)
		{
			SendClientMessage(clientId, "Somente acima do level 70");
			return true;
		}

		if (!p->eValue[0])
		{
			Mob[clientId].Tab[0] = 0;

			Log(clientId, LOG_INGAME, "Tab removido");
		}
		else
		{
			strncpy_s(Mob[clientId].Tab, p->eValue, 25);

			Log(clientId, LOG_INGAME, "Tab setado para: %s", p->eValue);
		}
		p364 packet;
		GetCreateMob(clientId, (BYTE*)&packet);

		GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);
	}
	else if (!strcmp(p->eCommand, "nig"))
	{
		// Retorna o tempo para o pesadelo
		time_t nowraw;
		struct tm now;
		int hour, min, sec;

		nowraw = time(NULL);
		localtime_s(&now, &nowraw);

		hour = now.tm_hour;
		min = now.tm_min;
		sec = now.tm_sec;

		SendClientMessage(clientId, "!!%02d%02d%02d", hour, min, sec);
	}
	else if (!strcmp(p->eCommand, "snd"))
	{
		if (p->eValue[0])
		{
			strncpy_s(Users[clientId].SNDMessage, p->eValue, 96);

			SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], p->eValue);

			Log(clientId, LOG_INGAME, "SND setado para: %s", p->eValue);
		}
		else
			Users[clientId].SNDMessage[0] = 0;
	}
	else if (!strcmp(p->eCommand, "cp"))
		SendClientMessage(clientId, g_pLanguageString[_NN_CPPoint], GetPKPoint(clientId) - 75);
	else if (!strcmp(p->eCommand, "nt"))
	{
		int value = Mob[clientId].Mobs.PesaEnter;
		if (value <= 0)
			SendClientMessage(clientId, g_pLanguageString[_NN_NT_Zero]);
		else
			SendClientMessage(clientId, g_pLanguageString[_NN_NT_Amount], value);
	}
	else if (!strcmp(p->eCommand, "wt"))
	{
		SendClientMessage(clientId, "Você usou %d entradas das %d disponíveis", User.Water.Total, sServer.MaxWaterEntrance);
	}
	else if (!strcmp(p->eCommand, "time"))
	{
		char tmp3[108];

		time_t rawtime;
		struct tm timeinfo;
		rawtime = time(0);
		localtime_s(&timeinfo, &rawtime);

		strftime(tmp3, 80, "%H:%M:%S %d-%m-%Y", &timeinfo);
		SendClientMessage(clientId, "[%s] - %s %dx %dy - Canal %d", Mob[clientId].Mobs.Player.Name, tmp3, Mob[clientId].Target.X, Mob[clientId].Target.Y, sServer.Channel);
	}
	else if (!strcmp(p->eCommand, "guildfame"))
	{
		INT32 gId = Mob[clientId].Mobs.Player.GuildIndex;
		if (gId <= 0)
			return true;

		SendClientMessage(clientId, "Você possui um total de %d pontos de fame guild", g_pGuild[gId].Fame);
	}
	else if (!strcmp(p->eCommand, "relo"))
	{
		INT32 userId = GetUserByName(p->eValue);
		if (userId <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		// se é foema
		bool accepted = false;
		if (Mob[clientId].Mobs.Player.ClassInfo != 1)
		{
			if (Users[userId].SummonedUser == 0)
			{
				SendClientMessage(clientId, "Só pode usar quando convocado pelo grupo ou pelo Líder de Guild");

				return true;
			}

			accepted = true;
		}

		if (!Mob[clientId].Mobs.Player.Status.curHP)
		{
			SendClientMessage(clientId, "Você não pode ir até o usuário, pois está morto.");
			if (accepted)
				Users[userId].SummonedUser = 0;

			return true;
		}

		INT32 leader = Mob[clientId].Leader;
		if (leader == 0)
			leader = clientId;

		bool canMove = false;
		if (leader == userId)
			canMove = true;
		else
		{
			for (INT32 i = 0; i < 12; i++)
			{
				INT32 mobId = Mob[leader].PartyList[i];
				if (mobId <= 0 || mobId >= MAX_PLAYER)
					continue;

				if (mobId == userId)
				{
					canMove = true;

					break;
				}
			}
		}

		INT32 guildIndex = Mob[clientId].Mobs.Player.GuildIndex;
		INT32 userGuild = Mob[userId].Mobs.Player.GuildIndex;

		if (guildIndex != 0 && userGuild != 0 && guildIndex == userGuild)
			canMove = true;

		MapAttribute map = GetAttribute(Mob[userId].Target.X, Mob[userId].Target.Y);
		if (map.CantSummon)
			canMove = false;

		INT32 slotId = GetFirstSlot(clientId, 699);
		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 776);

		if (slotId == -1)
			slotId = GetFirstSlot(clientId, 3430);

		if (slotId == -1)
			SendClientMessage(clientId, "Necessário Pergaminho Portal para dar relo");
		else
		{
			if (canMove)
			{
				if (slotId != -1)
				{
					AmountMinus(&Mob[clientId].Mobs.Player.Inventory[slotId]);

					SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);
				}

				Teleportar(clientId, Mob[userId].Target.X, Mob[userId].Target.Y);
				Log(clientId, LOG_INGAME, "Relo no personagem %s %dx %dy", Mob[userId].Mobs.Player.Name, Mob[userId].Target.X, Mob[userId].Target.Y);
			}
			else
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);
		}
		if (accepted)
			Users[userId].SummonedUser = 0;
	}
	else if (!strcmp(p->eCommand, "summon"))
	{
		INT32 userId = GetUserByName(p->eValue);
		if (userId <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		if (Mob[clientId].Mobs.Player.ClassInfo != 1 || !(Mob[clientId].Mobs.Player.Learn[0] & 0x40000))
		{
			SendClientMessage(clientId, "Só pode usar quando convocado pelo grupo ou pelo Líder de Guild");

			return true;
		}

		p3B2 packet{};

		packet.Header.PacketId = 0x3B2;
		packet.Header.ClientId = 0x7530;

		int len = strlen(Mob[clientId].Mobs.Player.Name);
		if (len >= 12)
			len = 11;
		for (int i = 0; i < len; i++)
			packet.Nickname[i] = Mob[clientId].Mobs.Player.Name[i];

		Users[userId].AddMessage(reinterpret_cast<BYTE*>(&packet), sizeof p3B2);

		SummonedUser = userId;

		Log(clientId, LOG_INGAME, "Solicitado summon %s", p->eValue);
		Log(userId, LOG_INGAME, "Recebido solicitação de summon de %s", p->eValue);
	}
	else if (p->eCommand[0] == 'r' && p->eCommand[1] == 0)
	{
		INT32 userId = Users[clientId].LastWhisper;
		if (userId <= 0 || userId >= MAX_PLAYER || Users[userId].Status != USER_PLAY)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		if (Users[userId].AllStatus.Whisper)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Whisper_MP]);

			return true;
		}

		if (!p->eValue[0])
		{
			if (Mob[userId].Mobs.Player.GuildIndex != 0)
				sprintf_s(szTMP, "%s Fame: %d Guild: %s", Mob[userId].Mobs.Player.Name, Mob[userId].Mobs.Fame, g_pGuild[Mob[userId].Mobs.Player.GuildIndex].Name.c_str());
			else
				sprintf_s(szTMP, "%s Fame: %d", Mob[userId].Mobs.Player.Name, Mob[userId].Mobs.Fame);

			SendClientMessage(clientId, szTMP);

			if (Users[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], Users[userId].SNDMessage);
		}
		else
		{
			Users[clientId].LastWhisper = userId;
			Users[userId].LastWhisper = clientId;

			p334 packet;
			memset(&packet, 0, sizeof p334);

			packet.Header.ClientId = userId;
			packet.Header.Size = sizeof p334;
			packet.Header.PacketId = 0x334;

			strncpy_s(packet.eCommand, Mob[clientId].Mobs.Player.Name, 16);
			strncpy_s(packet.eValue, p->eValue, 100);

			Users[userId].AddMessage((BYTE*)&packet, sizeof p334);

			Log(clientId, LOG_INGAME, "Mensagem Privada para: %s: %s", Mob[userId].Mobs.Player.Name, p->eValue);
			Log(userId, LOG_INGAME, "Mensagem Privada recebida: %s: %s", Mob[clientId].Mobs.Player.Name, p->eValue);

			LogPlayer(clientId, "Mensagem Privada para: %s: %s", Mob[userId].Mobs.Player.Name, p->eValue);
			LogPlayer(userId, "Mensagem Privada recebida: %s: %s", Mob[clientId].Mobs.Player.Name, p->eValue);

			if (Users[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], Users[userId].SNDMessage);
		}
	}
	else
	{
		INT32 userId = GetUserByName(p->eCommand);

		if (userId <= 0 || userId >= MAX_PLAYER || Users[userId].Status != USER_PLAY)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected]);

			return true;
		}

		if (Users[userId].AllStatus.Whisper)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Whisper_MP]);

			return true;
		}

		if (!p->eValue[0])
		{
			if (Mob[userId].Mobs.Player.GuildIndex != 0)
			{
				sprintf_s(szTMP, "%s Fame: %d Guild: %s", Mob[userId].Mobs.Player.Name, Mob[userId].Mobs.Fame, g_pGuild[Mob[userId].Mobs.Player.GuildIndex].Name.c_str());
			}
			else
				sprintf_s(szTMP, "%s Fame: %d", Mob[userId].Mobs.Player.Name, Mob[userId].Mobs.Fame);

			SendClientMessage(clientId, szTMP);

			if (Users[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], Users[userId].SNDMessage);
		}
		else
		{
			Users[clientId].LastWhisper = userId;
			Users[userId].LastWhisper = clientId;

			p334 packet;
			memset(&packet, 0, sizeof p334);

			packet.Header.ClientId = userId;
			packet.Header.Size = sizeof p334;
			packet.Header.PacketId = 0x334;

			strncpy_s(packet.eCommand, Mob[clientId].Mobs.Player.Name, 16);
			strncpy_s(packet.eValue, p->eValue, 100);

			Users[userId].AddMessage((BYTE*)&packet, sizeof p334);

			Log(clientId, LOG_INGAME, "Mensagem Privada para: %s: %s", Mob[userId].Mobs.Player.Name, p->eValue);
			Log(userId, LOG_INGAME, "Mensagem Privada recebida: %s: %s", Mob[clientId].Mobs.Player.Name, p->eValue);

			LogPlayer(clientId, "Mensagem privada para %s: %s", Mob[userId].Mobs.Player.Name, p->eValue);
			LogPlayer(userId, "Mensagem Privada recebida: %s: %s", Mob[clientId].Mobs.Player.Name, p->eValue);

			if (Users[userId].SNDMessage[0])
				SendClientMessage(clientId, g_pLanguageString[_NN_Message_SND], Users[userId].SNDMessage);
		}
	}

	return true;
}