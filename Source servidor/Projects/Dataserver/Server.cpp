#include "Basedef.h"
#include "CUser.h"
#include "CPSock.h"
#include "Server.h"
#include "CFileDB.h"
#include <fcntl.h>
#include <io.h>
#include <fstream>
#include "UOD_ResetAccount.h"
CPSock Server[MAX_SERVERNUMBER];
CUser pUser[MAX_SERVERNUMBER];
CUser pAdmin[MAX_ADMIN];
CPSock AdminClient;
CPSock AdminSocket;
CUser TempUser;
stServer sServer;
unsigned char	LocalIP[4] = { 0, };
unsigned int pAdminIP[MAX_ADMIN] = { 0, };
INT32 UserConnection[MAX_SERVERNUMBER];

std::unique_ptr<TOD_Log> serverLog;

void DisableAccount(int conn, char *account, int Year, int YearDay)
{
	account[16 - 1] = 0;
	account[16 - 2] = 0;

	_strupr_s(account, 16);

	stAccount file;
	memcpy(file.Username, account, 16);

	int iret = cFileDB.DBReadAccount(&file);

	if (iret == FALSE)
	{
		if (conn >= 0)
			SendAdminSignal(conn, 0, _MSG_NPNotFound);

		if (conn >= 0)
			SendAdminMessage(conn, 0, "There's no account with that account name");

		return;
	}

	if (file.YearDay != 0 && file.Year != 0)
	{
		if (conn >= 0)
			SendAdminState(conn, 0, _MSG_NPState, "1");

		return;
	}

	file.Password[12 - 1] = 0;
	file.Password[12 - 2] = 0;

	file.Year = Year;
	file.YearDay = YearDay;

	cFileDB.DBWriteAccount(&file);

	if (conn >= 0)
	{
		int idx = pAdmin[conn].DisableID;

		MSG_NPReqAccount sm;
		memset(&sm, 0, sizeof(MSG_NPReqAccount));

		sm.Header.PacketId = _MSG_NPReqAccount;
		sm.Header.Size = sizeof(MSG_NPReqAccount);
		sm.Header.ClientId = idx;

		strncpy_s(sm.Account, account, 16);

		sm.Char[0] = 0;

		ProcessAdminMessage(conn, (char*)&sm);

		pAdmin[conn].DisableID = 0;
	}
}

void EnableAccount(int conn, char *account)
{
	account[16 - 1] = 0;
	account[16 - 2] = 0;

	_strupr_s(account, 16);

	stAccount file;
	memcpy(file.Username, account, 16);

	int iret = cFileDB.DBReadAccount(&file);

	if (iret == FALSE)
	{
		if (conn >= 0)
			SendAdminSignal(conn, 0, _MSG_NPNotFound);

		if (conn >= 0)
			SendAdminMessage(conn, 0, "There's no account with that account name");

		return;
	}

	if (file.Year == 0 && file.YearDay == 0)
	{
		if (conn >= 0)
			SendAdminState(conn, 0, _MSG_NPState, file.Password);

		return;
	}

	file.Password[12 - 1] = 0;
	file.Password[12 - 2] = 0;
	file.Year = 0;
	file.YearDay = 0;

	cFileDB.DBWriteAccount(&file);

	if (conn >= 0)
		SendAdminState(conn, 0, _MSG_NPState, file.Password);

	return;
}

int ProcessAdminMessage(int conn, char *msg)
{
	PacketHeader *std = (PacketHeader *)msg;

	if (!(std->PacketId & FLAG_NP2DB) || (std->ClientId < 0) || (std->ClientId >= MAX_DBACCOUNT))
	{
		PacketHeader *m = (PacketHeader *)msg;

		return FALSE;
	}

	switch (std->PacketId)
	{
	case _MSG_NPCreateCharacter:
	{
		MSG_NPCreateCharacter *m = (MSG_NPCreateCharacter*)msg;

		MSG_NPCreateCharacter_Reply	sm;
		memset(&sm, 0, sizeof(MSG_NPCreateCharacter_Reply));
		sm.Header.PacketId = _MSG_NPCreateCharacter_Reply;
		sm.Header.Size = sizeof(MSG_NPCreateCharacter_Reply);
		sm.Slot = m->Slot;
		sm.Header.ClientId = m->Header.ClientId;
		sm.Result = 0;

		strncpy_s(sm.Account, m->Account, ACCOUNTNAME_LENGTH);
		strncpy_s(sm.Name, m->Mob.Name, 16);

		char *account = m->Account;
		int slot = m->Slot;
		int IdxName = cFileDB.GetIndex(m->Account);

		if (IdxName != 0)
		{
			sm.Result = 3;

			pAdmin[conn].Sock.SendOneMessage((char*)&sm, sm.Header.Size);

			return TRUE;
		}

		stAccount file;

		memcpy(file.Username, m->Account, ACCOUNTNAME_LENGTH);

		int ret = cFileDB.DBReadAccount(&file);

		if (ret == FALSE)
		{
			sm.Result = 4;

			pAdmin[conn].Sock.SendOneMessage((char*)&sm, sm.Header.Size);

			return TRUE;
		}

		int empty;

		for (empty = 0; empty < 4; empty++)
		{
			if (file.Mob[empty].Player.Name[0] == 0)
				break;
		}

		if (empty == 4)
		{
			sm.Result = 2;

			pAdmin[conn].Sock.SendOneMessage((char*)&sm, sm.Header.Size);

			return TRUE;
		}

		ret = cFileDB.CreateCharacter(m->Account, m->Mob.Name);

		if (ret == FALSE)
		{
			sm.Result = 1;

			pAdmin[conn].Sock.SendOneMessage((char*)&sm, sm.Header.Size);

			return TRUE;
		}

		for (int i = 0; i < 16 + 64 - 1; i++)
		{
			st_Item *sour;

			if (i < 16)
				sour = &m->Mob.Equip[i];
			else
				sour = &m->Mob.Inventory[i];

			int sidx = sour->Index;

			//if (sidx == 446 || (sidx >= 508 && sidx <= 509) || (sidx >= 526 && sidx <= 537) || sidx == 522)
			//	ConvertGuildNumber(conn, sour);
		}

		memcpy(&file.Mob[empty], &(m->Mob), sizeof(st_Mob));

		_strupr_s(file.Username);

		ret = cFileDB.DBWriteAccount(&file);

		if (ret == FALSE)
		{

			sm.Result = 4;

			pAdmin[conn].Sock.SendOneMessage((char*)&sm, sm.Header.Size);

			return TRUE;
		}

		sm.Result = 0;

		pAdmin[conn].Sock.SendOneMessage((char*)&sm, sm.Header.Size);

	} break;

	case _MSG_NPNotice:
	{
		MSG_NPNotice *m = (MSG_NPNotice*)msg;

		m->AccountName[ACCOUNTNAME_LENGTH - 1] = 0;
		m->AccountName[ACCOUNTNAME_LENGTH - 2] = 0;
		m->String[96 - 1] = 0;
		m->String[96 - 2] = 0;

		if (m->AccountName[0] == 0 && m->Parm1 == 1)
		{
			if (pAdmin[conn].Level < 2)
				return TRUE;

			for (int i = 0; i < MAX_SERVER; i++)
			{
				if (pUser[i].Mode == USER_EMPTY)
					continue;

				if (pUser[i].Sock.Sock == NULL)
					continue;

				MSG_NPNotice sm;
				memset(&sm, 0, sizeof(MSG_NPNotice));

				sm.Header.PacketId = _MSG_NPNotice;
				//				sm.Header.PacketId = 0;
				sm.Header.Size = sizeof(MSG_NPNotice);

				sm.Parm1 = 1;
				sm.Parm2 = 1;

				strncpy_s(sm.String, m->String, 96);

				pUser[i].Sock.SendOneMessage((char*)&sm, sizeof(sm));

			}

			return TRUE;
		}

		if (pAdmin[conn].Level <= 0)
			return TRUE;

		int IdxName = cFileDB.GetIndex(m->AccountName);

		if (IdxName == 0)
		{
			SendAdminMessage(conn, 0, "Specified user not connected. can't send notice.");

			return TRUE;
		}

		int svr = IdxName / MAX_PLAYER;
		int id = IdxName % MAX_PLAYER;

		if (svr < 0 || svr >= MAX_SERVER || id <= 0 || id >= MAX_PLAYER)
		{
			SendAdminMessage(conn, 0, "Wrong SVR and ID");

			return TRUE;
		}

		MSG_NPNotice sm;
		memset(&sm, 0, sizeof(MSG_NPNotice));

		sm.Header.PacketId = _MSG_NPNotice;
		sm.Header.ClientId = id;
		sm.Header.Size = sizeof(MSG_NPNotice);

		sm.Parm1 = 0;
		sm.Parm2 = 0;

		strncpy_s(sm.String, m->String, 96);

		pUser[svr].Sock.SendOneMessage((char*)&sm, sizeof(sm));

		return TRUE;

	} break;

	case _MSG_NPIDPASS:
	{
		MSG_NPIDPASS *m = (MSG_NPIDPASS *)msg;

		_strupr_s(m->Account);

		if (pAdmin[conn].Encode1 != m->Encode1 || pAdmin[conn].Encode2 != m->Encode2)
			return FALSE;

		if (pAdmin[conn].Level != -1)
			return TRUE;

		stAccount file;

		memcpy(file.Username, m->Account, ACCOUNTNAME_LENGTH);

		int iret = cFileDB.DBReadAccount(&file);

		if (iret == FALSE)
			return TRUE;

		if (file.Password[0] == '_')
			return FALSE;
		if (file.Password[0] == '@')
			return FALSE;

		char *p = (char*)file.Password;

		iret = strncmp(m->Pass, p, ACCOUNTPASS_LENGTH);

		if (iret)
			return FALSE;

		DWORD maxlevel = 0;

		for (int i = 0; i < 4; i++)
		{
			if (file.Mob[i].Player.Name[0] != 0 && file.Mob[i].Player.bStatus.Level > maxlevel)
				maxlevel = file.Mob[i].Player.bStatus.Level;
		}

		if (maxlevel < 1000)
			return FALSE;

		int admin = maxlevel - 1000;

		pAdmin[conn].Level = admin;
		pAdmin[conn].DisableID = 0;

		strncpy_s(pAdmin[conn].Name, m->Account, ACCOUNTNAME_LENGTH);

		char temp[256];
		sprintf_s(temp, "sys,Admin Login Success - Level: %d", admin);
		//Log(temp, pAdmin[conn].Name, pAdmin[conn].IP);
		SendAdminMessage(conn, 0, temp);

	} break;

	case _MSG_NPReqAccount:
	{
		MSG_NPReqAccount *m = (MSG_NPReqAccount*)msg;

		if (pAdmin[conn].Level <= 0)
			return TRUE;

		_strupr_s(m->Account);
		_strupr_s(m->Char);

		if (m->Char[0] != 0)
			cFileDB.GetAccountByChar(m->Account, m->Char);


		if (m->Account[0] == 0)
		{
			SendAdminSignal(conn, 0, _MSG_NPNotFound);
			SendAdminMessage(conn, 0, "There's no account with that character name");

			return TRUE;
		}

		stAccount file;

		memcpy(file.Username, m->Account, ACCOUNTNAME_LENGTH);

		int iret = cFileDB.DBReadAccount(&file);

		if (iret == FALSE)
		{
			SendAdminSignal(conn, 0, _MSG_NPNotFound);
			SendAdminMessage(conn, 0, "There's no account with that account name");

			return TRUE;
		}

		MSG_NPAccountInfo sm;
		memset(&sm, 0, sizeof(MSG_NPAccountInfo));

		sm.Header.PacketId = _MSG_NPAccountInfo;
		sm.Header.Size = sizeof(MSG_NPAccountInfo);
		sm.Header.ClientId = 0;
		sm.account = file;

		int IdxName = cFileDB.GetIndex(m->Account);

		sm.Session = IdxName;

		file.Mob[0].Player.Name[16 - 1] = 0;
		file.Mob[0].Player.Name[16 - 2] = 0;
		file.Mob[1].Player.Name[16 - 1] = 0;
		file.Mob[1].Player.Name[16 - 2] = 0;
		file.Mob[2].Player.Name[16 - 1] = 0;
		file.Mob[2].Player.Name[16 - 2] = 0;
		file.Mob[3].Player.Name[16 - 1] = 0;
		file.Mob[3].Player.Name[16 - 2] = 0;
		file.Username[16 - 1] = 0;
		file.Username[16 - 2] = 0;

		if (pAdmin[conn].Level <= 0)
			return TRUE;

		DWORD maxlevel = 0;

		for (int i = 0; i < 4; i++)
		{
			if (file.Mob[i].Player.Name[0] != 0 && file.Mob[i].Player.bStatus.Level > maxlevel)
				maxlevel = file.Mob[i].Player.bStatus.Level;

		}

		int admin = maxlevel - 1000;

		sm.State = 0;

		char accountstate[256];
		memset(accountstate, 0, 255);

		if (sm.account.Password[0] == '@' || sm.account.Year != 0)
		{
			sprintf_s(accountstate, "Conta bloqueada.");
			sm.State = 1;
		}
		if (sm.account.Password[0] == '_')
		{
			sprintf_s(accountstate, "Conta Defeituosa.");
			sm.State = 2;
		}
		if (sm.account.Password[0] == '#')
		{
			sprintf_s(accountstate, "Conta Desativada.");
			sm.State = 3;
		}

		char temp[256];
		sprintf_s(temp, "%s < %s | %s | %s | %s> - %s", file.Username, file.Mob[0].Player.Name, file.Mob[1].Player.Name, file.Mob[2].Player.Name, file.Mob[3].Player.Name, accountstate);

		temp[128 - 1] = 0;
		temp[128 - 2] = 0;

		SendAdminMessage(conn, 0, temp);
		/*
		sm.account.Info.AccountPass[0] = rand() % 24 + 'A';
		sm.account.Info.AccountPass[1] = rand() % 24 + 'A';
		sm.account.Info.AccountPass[2] = rand() % 24 + 'A';
		sm.account.Info.AccountPass[3] = rand() % 24 + 'A';
		sm.account.Info.AccountPass[4] = 0;
		*/
		pAdmin[conn].Sock.SendOneMessage((char*)&sm, sizeof(MSG_NPAccountInfo));

	} break;

	case _MSG_NPReqSaveAccount:
	{
		if (pAdmin[conn].Level <= 2)
		{
			SendAdminMessage(conn, 0, "Not allowed");

			return TRUE;
		}

		MSG_NPAccountInfo *m = (MSG_NPAccountInfo*)msg;

		int IdxName = cFileDB.GetIndex(m->account.Username);

		if (IdxName != 0)
		{
			SendAdminMessage(conn, 0, "For saving, account should be disabled.");

			return TRUE;
		}

		DWORD maxlevel = 0;
		for (int i = 0; i < 4; i++)
		{

			if (m->account.Mob[i].Player.Name[0] != 0 && m->account.Mob[i].Player.bStatus.Level > maxlevel)
				maxlevel = m->account.Mob[i].Player.bStatus.Level;

		}

		int admin = maxlevel - 1000;

		if (maxlevel >= 2000)
		{
			if (admin > pAdmin[conn].Level)
			{
				SendAdminMessage(conn, 0, "Set admin level error.");

				return TRUE;
			}

			if (pAdmin[conn].Level == admin && strcmp(pAdmin[conn].Name, m->account.Username))
			{
				SendAdminMessage(conn, 0, "Set admin level error.");

				return TRUE;
			}

		}

		stAccount tmpact;

		memcpy(tmpact.Username, m->account.Username, ACCOUNTNAME_LENGTH);

		int iret = cFileDB.DBReadAccount(&tmpact);

		if (!iret)
		{
			SendAdminMessage(conn, 0, "There's no account with that account name");

			return TRUE;
		}

		memcpy(m->account.Password, tmpact.Password, ACCOUNTPASS_LENGTH);

		char temp[128];
		sprintf_s(temp, "Saving account [%s] success", m->account.Username);
		cFileDB.DBWriteAccount(&m->account);

		SendAdminMessage(conn, 0, temp);

	} break;

	case _MSG_NPDisable:
	{
		if (pAdmin[conn].Level <= 2)
			return TRUE;

		MSG_NPEnable *m = (MSG_NPEnable*)msg;

		m->AccountName[ACCOUNTNAME_LENGTH - 1] = 0;
		m->AccountName[ACCOUNTNAME_LENGTH - 2] = 0;

		_strupr_s(m->AccountName);

		int IdxName = cFileDB.GetIndex(m->AccountName);

		if (IdxName == 0)
		{
			DisableAccount(conn, m->AccountName, m->Year, m->YearDay);

			return TRUE;
		}
		else
		{
			cFileDB.SendDBSavingQuit(IdxName, 1);
			pAdmin[conn].DisableID = IdxName;
			pAdmin[conn].Year = m->Year;
			pAdmin[conn].YearDay = m->YearDay;
		}

	} break;

	case _MSG_NPEnable:
	{
		if (pAdmin[conn].Level <= 2)
			return TRUE;

		MSG_NPEnable *m = (MSG_NPEnable*)msg;

		m->AccountName[ACCOUNTNAME_LENGTH - 1] = 0;
		m->AccountName[ACCOUNTNAME_LENGTH - 2] = 0;

		_strupr_s(m->AccountName);

		int IdxName = cFileDB.GetIndex(m->AccountName);

		if (IdxName != 0)
		{
			SendAdminMessage(conn, 0, "Check session. already playing.");

			return TRUE;
		}

		EnableAccount(conn, m->AccountName);

	} break;

	}

	return TRUE;

}

int SendAdminSignal(int svr, unsigned short id, unsigned short signal)
{
	PacketHeader sm;

	sm.PacketId = signal;
	sm.ClientId = id;
	sm.Size = sizeof(PacketHeader);

	pAdmin[svr].Sock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}
void ReadAdmin()
{
	char temp[256];

	sprintf_s(temp, "Admin.txt");

	FILE *fp;
	fopen_s(&fp, temp, "r");

	if (fp == NULL)
		return;

	while (1)
	{
		int a, b, c, d;
		a = b = c = d = 0;

		char * ret = fgets((char*)temp, 127, fp);

		if (ret == NULL)
			break;

		int idx = -1;

		for (int i = 0; i < 255; i++)
			if (temp[i] == '.')
				temp[i] = ' ';

		sscanf_s(temp, "%d %d %d %d %d", &idx, &a, &b, &c, &d);

		unsigned int ip = (d << 24) + (c << 16) + (b << 8) + a;

		if (idx < 0 || idx >= MAX_ADMIN)
			continue;

		pAdminIP[idx] = ip;
	};

	fclose(fp);
}

void SendNotice(int srv, char *msg, ...)
{
	/* Arglist */
	char buffer[150];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */
	p101 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x101;
	packet.Header.ClientId = 0x7530;
	packet.Header.Size = sizeof packet;

	strncpy_s(packet.eMsg, buffer, 96);

	pUser[srv].Sock.AddMessage((char*)&packet, sizeof p101);
}

int GetAdminFromSocket(int Sock)
{
	for (int i = 0; i < MAX_ADMIN; i++)
	{
		if (pAdmin[i].Sock.Sock == (unsigned)Sock)
			return i;
	}

	return -1;
}

int SendAdminMessage(int svr, unsigned short id, const char *message)
{
	MSG_NPNotice sm;

	sm.Header.PacketId = _MSG_NPNotice;
	sm.Header.ClientId = id;
	sm.Header.Size = sizeof(MSG_NPNotice);

	strncpy_s(sm.String, message, 96);

	pAdmin[svr].Sock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int SendAdminParm(int svr, unsigned short id, unsigned short signal, int parm)
{
	MSG_STANDARDPARM1 sm;

	sm.Header.PacketId = signal;
	sm.Header.ClientId = id;
	sm.Header.Size = sizeof(MSG_STANDARDPARM1);
	sm.Parm1 = parm;

	pAdmin[svr].Sock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}

int SendAdminState(int svr, unsigned short id, unsigned short signal, const  char *pass)
{
	MSG_STANDARDPARM1 sm;

	sm.Header.PacketId = signal;
	sm.Header.ClientId = id;
	sm.Header.Size = sizeof(MSG_STANDARDPARM1);

	if (pass[0] == '@') sm.Parm1 = 1;
	if (pass[0] == '_') sm.Parm1 = 2;
	if (pass[0] == '#') sm.Parm1 = 3;

	pAdmin[svr].Sock.SendOneMessage((char*)&sm, sizeof(sm));

	return TRUE;
}


void ProcessSecTimer()
{
	sServer.SecCounter++;

	ImportUser();
	ImportPass();
	ImportBan();

	if (!(sServer.SecCounter % 10))
	{
		WriteGuilds();
		WriteConfig();
	}

	if (!(sServer.SecCounter % 10))
		ImportItem();

	if (!(sServer.SecCounter % 2))
		ImportCash();

	if (!(sServer.SecCounter % 30))
		UpdateConnection();

	time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);

	if (!now.tm_sec && now.tm_wday >= 0 && now.tm_wday < 6 && !now.tm_min)
	{/*
		// 21 Horas de segunda~sexta
		// Enviar pacote para as tmsrv avisando do início da guerra
		if (now.tm_hour == 23 && now.tm_wday != 0)
		{
			// Informa os canais das condições iniciais da guerra e libera o funcionamento
			for (int i = 0; i < MAX_SERVERGROUP; i++)
			{
				if (pUser[i].Mode == USER_EMPTY)
					continue;

				if (pUser[i].Sock.Sock == INVALID_SOCKET)
					continue;

				// Seta o TowerState em todos os canais
				// Como quem está avançadno é o canal contrário
				// Quando o sistema inicia:
				// O canal 1 está avançando sobre o 2
				// O canal 2 está avançando sobre o 1
				int otherConn = ((i % 2) ? (i - 1) : (i + 1));
				g_pTowerWarState[i].TowerState = otherConn;
			}

			// Envia a informação para a TMsrv da atual situação da guerra
			// O parametro 1 indica que a guerra está sendo iniciada
			SendTowerWarInfo(1);
		}

		// 22 Horas
		// Enviar pacote para as tmsrv avisando do fim da guerra
		if (now.tm_hour == 0)
		{
			SendTowerWarInfo(0);
			DecideWinnerTowerWar();
		}*/
	}

	if (!now.tm_sec && now.tm_wday == 3 && !now.tm_min && now.tm_hour == 19)
	{
		// Renasce o Kefra em todos os canais ^^ 
		sServer.FirstKefra = MAX_SERVERGROUP;
		sServer.KefraKiller[0] = 0;

		_MSG_FIRST_KEFRA_NOTIFY packet;
		memset(&packet, 0, sizeof _MSG_FIRST_KEFRA_NOTIFY);

		packet.Header.PacketId = MSG_FIRST_KEFRA_NOTIFY;
		packet.Header.Size = sizeof _MSG_FIRST_KEFRA_NOTIFY;

		PacketHeader header;
		header.PacketId = MSG_REBORN_KEFRA;
		header.Size = sizeof header;

		for (int i = 0; i < MAX_SERVERGROUP; i++)
		{
			// Notifica os outros canais de que o Kefra morreu primeiro em outro canal
			// E chama a função de renascimento de Kefra em cada canal
			if (pUser[i].Mode == USER_EMPTY)
				continue;

			if (pUser[i].Sock.Sock == INVALID_SOCKET)
				continue;

			packet.Channel = MAX_SERVERGROUP;

			pUser[i].Sock.SendOneMessage((char*)&packet, packet.Header.Size);
			pUser[i].Sock.SendOneMessage((char*)&header, sizeof PacketHeader);
		}
	}
}

void ImportPass()
{
	char line[256] = { 0 };
	char path[256] = { 0 };
	char arq[256] = { 0 };
	strncpy_s(path, "./ImportPass/*.txt", 256);

	struct _finddata_t file;
	INT32 findFile = _findfirst(path, &file);

	if (findFile == -1)
		return;

	do
	{
		if (file.name[0] == '.')
		{
			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;

			continue;
		}

		sprintf_s(arq, "./ImportPass/%s", file.name);
		FILE* pFile;
		fopen_s(&pFile, arq, "rt");

		if (pFile != NULL)
		{
			char *result = fgets(line, 127, pFile);
			if (result == 0)
			{
				fclose(pFile);

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			fclose(pFile);

			char user[16] = { 0 };
			char pass[16] = { 0 };
			char num[16] = { 0 };

			INT32 ret = sscanf_s(line, "%s %s %s", user, 16, pass, 16, num, 6);
			if (ret != 2 && ret != 3)
			{
				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			INT32 idxName = cFileDB.GetIndex(user);
			if (idxName > 0 && idxName < MAX_DBACCOUNT)
			{
				INT32 srv = idxName / MAX_PLAYER;
				INT32 id = idxName % MAX_PLAYER;

				cFileDB.SendLog(srv, id, "A conta estava online. Não é possível trocar a senha.");

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			stAccount account{};
			_strupr_s(user);
			memcpy(account.Username, user, 16);

			INT32 readedAcc = cFileDB.DBReadAccount(&account);
			if (readedAcc == 0)
			{
				INT32 tryFindFile = _findnext(findFile, &file);
				if (tryFindFile != 0)
					break;

				continue;
			}

			std::stringstream str;
			str << "A senha da conta foi trocada para " << pass;

			strncpy_s(account.Password, pass, 16);

			if (num[0])
			{
				strncpy_s(account.SecondPass, num, 6);

				str << std::endl << "A senha numérica foi trocada para " << num;
			}

			INT32 suc = cFileDB.DBWriteAccount(&account);
			if (suc)
				DeleteFileA(arq);

			Log(account.Username, str.str().c_str());

			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;
		}
	} while (true);

	if (findFile != -1)
		_findclose(findFile);
}

void ImportUser()
{
	char line[256];
	char path[256];
	char arq[256];
	strncpy_s(path, "./ImportUser/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(path, &file);

	if (findFile == -1)
		return;

	do
	{
		if (file.name[0] == '.')
		{
			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;

			continue;
		}

		sprintf_s(arq, "./ImportUser/%s", file.name);
		FILE* pFile; 
		fopen_s(&pFile, arq, "rt");

		if (pFile != NULL)
		{
			char *result = fgets(line, 127, pFile);
			if (result == 0)
			{
				fclose(pFile);

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			char user[16];
			char pass[16];

			INT32 access = 0;
			INT32 ret = sscanf_s(line, "%s %s %d", user, 16, pass, 16, &access);
			if (ret != 2 && ret != 3)
			{
				fclose(pFile);

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			fclose(pFile);

			_strupr_s(user);

			stAccount account;
			memset(&account, 0, sizeof account);

			strncpy_s(account.Username, user, 16);
			strncpy_s(account.Password, pass, 12);

			if (access != 0)
				account.AccessLevel = access;

			INT32 suc = cFileDB.DBWriteAccount(&account);
			if (suc)
				DeleteFileA(arq);

			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;
		}
	} while (true);

	if (findFile != -1)
		_findclose(findFile);
}

void UpdateConnection()
{
	char file[256];
	sprintf_s(file, "C:\\xampp\\htdocs\\over\\servers\\serv%2.2d.htm", sServer.ServerIndex);

	FILE *pFile;
	fopen_s(&pFile, file, "wt");

	if (pFile == nullptr)
		return;

	for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
	{
		if (pUser[i].Mode == 0)
		{
			fprintf(pFile, "-1 ");
			continue;
		}

		if (UserConnection[i] < pUser[i].Count)
			UserConnection[i] = pUser[i].Count;

		fprintf(pFile, "%d ", ((pUser[i].Count << 2) / 3) + Rand() % 3);
	}

	fclose(pFile);
}


void ImportItem()
{
	char pathSucess[256];
	char temp[256];
	char pathError[256];
	strncpy_s(temp, "./ImportItem/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(temp, &file);
	if (findFile == -1)
		return;

	INT32 serverId = 0;
	while (true)
	{
		serverId++;
		if (serverId > MAX_SERVERGROUP)
			break;

		if (file.name[0] == '.')
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sprintf_s(pathSucess, "./ImportItem/%s", file.name);
		sprintf_s(pathError, "./ImportItem/err/%s", file.name);

		FILE *pFile;
		fopen_s(&pFile, pathSucess, "rt");
		if (pFile == nullptr)
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		char accountName[256];
		accountName[0] = 0; // EBP - 52C

		INT32 itemId = 0,
			ef1 = 0,
			ef2 = 0,
			ef3 = 0,
			efv1 = 0,
			efv2 = 0,
			efv3 = 0;

		char*line = fgets(temp, 256, pFile);
		if (line == 0)
		{
			fclose(pFile);

			MoveFileA(pathSucess, pathError);
			//Log(-import no contents
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sscanf_s(temp, "%s %d %d %d %d %d %d %d", accountName, 16, &itemId, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);
		fclose(pFile);

		//Log import starting
		_strupr_s(accountName);
		if (itemId < 0) //|| LOCAL_332[0] >= 6500 || LOCAL_332[1] < 0 || LOCAL_332[3] < 0 || LOCAL_332[5] < 0 || LOCAL_332[1] > 255 || LOCAL_332[3] > 255 || LOCAL_332[5] > 255)
		{
			MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		bool runTimeSent = false;
		bool success = false;
		INT32 idxName = cFileDB.GetIndex(accountName);
		if (idxName > 0 && idxName < MAX_DBACCOUNT && cFileDB.Account[idxName].Slot >= 0 && cFileDB.Account[idxName].Slot <= 3)
		{
			INT32 srv = idxName / MAX_PLAYER;
			INT32 id = idxName % MAX_PLAYER;

			pCOF packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xC0F;
			packet.Header.Size = sizeof pCOF;
			packet.Header.ClientId = id;

			strncpy_s(packet.Username, accountName, 16);

			packet.item.Index = itemId;
			packet.item.Effect[0].Index = ef1;
			packet.item.Effect[0].Value = efv1;
			packet.item.Effect[1].Index = ef2;
			packet.item.Effect[1].Value = efv2;
			packet.item.Effect[2].Index = ef3;
			packet.item.Effect[2].Value = efv3;

			pUser[srv].Sock.SendOneMessage((char*)&packet, sizeof pCOF);
			runTimeSent = 1;

			std::stringstream str;
			str << "Recebido o item através da DBsrv " << packet.item.toString().c_str();

			cFileDB.SendLog(srv, id, str.str().c_str());
		}

		if (!runTimeSent)
		{
			stAccount account{};
			_strupr_s(accountName);
			memcpy(account.Username, accountName, 16);

			INT32 readedAccount = cFileDB.DBReadAccount(&account);
			if (readedAccount == 0)
			{
				// Log err- import item success - account save fail
				MoveFileA(pathSucess, pathError);

				INT32 tryFindNext = _findnext(findFile, &file);
				if (tryFindNext != 0)
					break;

				continue;
			}


			st_Item item{};
			item.Index = itemId;
			item.Effect[0].Index = ef1;
			item.Effect[0].Value = efv1;
			item.Effect[1].Index = ef2;
			item.Effect[1].Value = efv2;
			item.Effect[2].Index = ef3;
			item.Effect[2].Value = efv3;

			INT32 pos = -1;
			for (INT32 i = 0; i < 120; i++)
			{
				if (account.Storage.Item[i].Index != 0)
					continue;

				pos = i;
				break;
			}

			if (pos == -1)
			{
				INT32 moveRet = MoveFileA(pathSucess, pathError);
				Log(accountName, "Sem espaço para receber o item %s.", item.toString().c_str());

				if (moveRet == 0)
				{
					Log("no space, move failed. Arquivo: %s. Item: %s GetLastError: %d.\n", pathSucess, item.toString().c_str(), GetLastError());
					Log(accountName, "no space, move failed. Arquivo %s. Item: %s. GetLastError %s.", pathSucess, item.toString().c_str(), GetLastError());
				}
				else Log("no space, move failed. Arquivo: %s. Item: %s GetLastError: %d.\n", pathSucess, item.toString().c_str(), GetLastError());

				INT32 tryFindNext = _findnext(findFile, &file);
				if (tryFindNext != 0)
					break;

				continue;
			}

			account.Storage.Item[pos] = item;
			Log(accountName, "Item %s entregue para o personagem", item.toString().c_str());

			INT32 tryReadAccount = cFileDB.DBWriteAccount(&account);
			if (tryReadAccount == 0)
			{
				// Log err- import item success - account save fail
				if (runTimeSent == 0)
					MoveFileA(pathSucess, pathError);

				tryReadAccount = _findnext(findFile, &file);
				if (tryReadAccount != 0)
					break;
			}
		}

		auto tryReadAccount = DeleteFileA(pathSucess);
		if (tryReadAccount == 0)
		{
			if (!MoveFileA(pathSucess, pathError))
			{
				printf("Import item sucess - file not deleted\n");
			}
			else
			{
				printf("Import item sucess - file moved WARNING\n");
			}
		}

		tryReadAccount = _findnext(findFile, &file);
		if (tryReadAccount != 0)
			break;
	}

	if (findFile != -1)
		_findclose(findFile);
}
void ImportBan()
{
	char pathSucess[256];
	char temp[256];
	char pathError[256];
	strncpy_s(temp, "./ImportBan/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(temp, &file);

	if (findFile == -1)
		return;

	INT32 serverId = 0;
	while (true)
	{
		serverId++;
		if (serverId > MAX_SERVERGROUP)
			break;

		if (file.name[0] == '.')
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sprintf_s(pathSucess, "./ImportBan/%s", file.name);
		sprintf_s(pathError, "./ImportBan/err/%s", file.name);

		FILE *pFile;
		fopen_s(&pFile, pathSucess, "rt");
		if (pFile == 0)
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		char accountName[256] = { 0 };

		INT32 type = 0, day = 0, mounth = 0, year = 0;
		char*line = fgets(temp, 256, pFile);
		if (line == 0)
		{
			fclose(pFile);

			MoveFileA(pathSucess, pathError);
			//Log(-import no contents
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sscanf_s(temp, "%s %d %d/%d/%d", accountName, 16, &type, &day, &mounth, &year);
		fclose(pFile);

		//Log import starting
		_strupr_s(accountName);
		if (type < 0)
		{
			MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		time_t rawnow = time(NULL);
		struct tm now;
		localtime_s(&now, &rawnow);

		INT32 runTimeSent = 0;
		INT32 idxName = cFileDB.GetIndex(accountName);
		// Dá DC no hotário
		if (idxName > 0 && idxName < MAX_DBACCOUNT)
		{
			INT32 srv = idxName / MAX_PLAYER;
			INT32 id = idxName % MAX_PLAYER;

			pC10 packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xC10;
			packet.Header.Size = sizeof pC10;
			packet.Header.ClientId = id;

			packet.BanType = type;
			packet.Ban.Dia = day;
			packet.Ban.Mes = mounth;
			packet.Ban.Ano = year;
			packet.Ban.Minuto = now.tm_min;
			packet.Ban.Hora = now.tm_hour;
			packet.Ban.Segundo = now.tm_sec;
			strncpy_s(packet.Username, accountName, 16);

			pUser[srv].Sock.SendOneMessage((char*)&packet, sizeof pC10);
			runTimeSent = 1;
		}

		stAccount account;

		memset(&account, 0, sizeof stAccount);
		_strupr_s(accountName);
		memcpy(account.Username, accountName, 16);

		INT32 readedAccount = cFileDB.DBReadAccount(&account);
		if (readedAccount == 0)
		{
			if (runTimeSent == 0)
				MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		account.BanType = type;
		account.Ban.Dia = day;
		account.Ban.Mes = mounth;
		account.Ban.Ano = year;
		account.Ban.Minuto = now.tm_min;
		account.Ban.Hora = now.tm_hour;
		account.Ban.Segundo = now.tm_sec;


		INT32 tryReadAccount = cFileDB.DBWriteAccount(&account);
		if (tryReadAccount == 0)
		{
			// Log err- import item success - account save fail
			if (runTimeSent == 0)
				MoveFileA(pathSucess, pathError);

			tryReadAccount = _findnext(findFile, &file);
			if (tryReadAccount != 0)
				break;
		}

		tryReadAccount = DeleteFileA(pathSucess);
		if (tryReadAccount == 0)
		{
			if (!MoveFileA(pathSucess, pathError))
			{
				printf("Import Ban sucess - file not deleted\n");
			}
			else
			{
				printf("Import Ban sucess - file moved WARNING\n");
			}
		}

		tryReadAccount = _findnext(findFile, &file);
		if (tryReadAccount != 0)
			break;
	}

	if (findFile != -1)
		_findclose(findFile);
}

void ImportCash()
{
	char pathSucess[256];
	char temp[256];
	char pathError[256];
	strncpy_s(temp, "./ImportCash/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(temp, &file);

	if (findFile == -1)
		return;

	INT32 serverId = 0;
	while (true)
	{
		serverId++;
		if (serverId > MAX_SERVERGROUP)
			break;

		if (file.name[0] == '.')
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sprintf_s(pathSucess, "./ImportCash/%s", file.name);
		sprintf_s(pathError, "./ImportCash/err/%s", file.name);

		FILE *pFile;
		fopen_s(&pFile, pathSucess, "rt");
		if (pFile == 0)
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		char accountName[256];
		accountName[0] = 0; // EBP - 52C

		INT32 cash = 0;
		char*line = fgets(temp, 256, pFile);
		if (line == 0)
		{
			fclose(pFile);

			MoveFileA(pathSucess, pathError);
			//Log(-import no contents
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sscanf_s(temp, "%s %d", accountName, 16, &cash);

		fclose(pFile);

		//Log import starting
		_strupr_s(accountName);
		if (cash < 0) //|| LOCAL_332[0] >= 6500 || LOCAL_332[1] < 0 || LOCAL_332[3] < 0 || LOCAL_332[5] < 0 || LOCAL_332[1] > 255 || LOCAL_332[3] > 255 || LOCAL_332[5] > 255)
		{
			MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		INT32 runTimeSent = 0;
		INT32 idxName = cFileDB.GetIndex(accountName);
		if (idxName > 0 && idxName < MAX_DBACCOUNT)
		{
			INT32 srv = idxName / MAX_PLAYER;
			INT32 id = idxName % MAX_PLAYER;

			pCOE packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xC0e;
			packet.Header.Size = sizeof pCOE;
			packet.Header.ClientId = id;

			packet.Cash = cash;
			strncpy_s(packet.Username, accountName, 16);

			pUser[srv].Sock.SendOneMessage((char*)&packet, sizeof pCOE);
			runTimeSent = 1;

			std::stringstream str;
			str << "Enviado pela DBsrv " << cash;
			cFileDB.SendLog(srv, id, str.str().c_str());
		}
		else
		{
			stAccount account{};
			_strupr_s(accountName);
			memcpy(account.Username, accountName, 16);

			INT32 readedAccount = cFileDB.DBReadAccount(&account);
			if (readedAccount == 0)
			{
				if (runTimeSent == 0)
					MoveFileA(pathSucess, pathError);

				INT32 tryFindNext = _findnext(findFile, &file);
				if (tryFindNext != 0)
					break;

				continue;
			}

			Log(accountName, "Foi entregue um total de %d cash para o usuário. Total: %d", cash, account.Cash + cash);

			account.Cash += cash;

			INT32 tryReadAccount = cFileDB.DBWriteAccount(&account);
			if (tryReadAccount == 0)
			{
				// Log err- import item success - account save fail
				if (runTimeSent == 0)
					MoveFileA(pathSucess, pathError);

				tryReadAccount = _findnext(findFile, &file);
				if (tryReadAccount != 0)
					break;
			}
		}

		bool wasDeleted = DeleteFileA(pathSucess) != 0;
		if (!wasDeleted)
		{
			auto le = GetLastError();
			if (!MoveFileA(pathSucess, pathError))
				Log("Import item sucess - file not deleted. Arquivo: %s. Error on delete: %d. Error on move: %d\n", pathSucess, le, GetLastError());
			else
				Log("Import item sucess - file moved WARNING Arquivo: %s\n. Error: %d", pathSucess, le);
		}

		auto nextAccount = _findnext(findFile, &file);
		if (nextAccount != 0)
			break;
	}

	if (findFile != -1)
		_findclose(findFile);
}

void CreateConsole()
{
	AllocConsole();

	// Output fix 
	freopen_s((FILE * *)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE * *)stdin, "CONIN$", "r", stdin);
}

BOOL ReadServerIP()
{
	FILE *pFile = NULL;

	fopen_s(&pFile, "serverip.txt", "rt");
	if (!pFile)
		return false;

	char szTMP[256];
	while (fgets(szTMP, 256, pFile))
	{
		if (*szTMP == '#' || *szTMP == '\n')
			continue;

		char cmd[32], val[32];
		int ret = sscanf_s(szTMP, "%[^=]=%[^\n]", &cmd, 32, &val, 32);
		if (ret != 2)
			continue;

		if (!strcmp(cmd, "Porta"))
			sServer.Config.Porta = atoi(val);
		else if (!strcmp(cmd, "IP"))
			strncpy_s(sServer.Config.IP, 16, val, 16);
	}

	fclose(pFile);
	return true;
}

void ProcessClientMessage(INT32 conn, char *msg)
{
	cFileDB.ProcessMessage(msg, conn);
}

void ResetAllAccounts()
{
	FILE *pFile{ nullptr };
	// Abre o arquivo de log 
	pFile = _fsopen("reportreset.txt", "w", _SH_DENYNO);
	// Busca por arquivos no diretorio dir
	HANDLE handle;
	WIN32_FIND_DATA win32_find_data;
	char tmp[1024];
	handle = FindFirstFile("account/*", &win32_find_data);

	if (handle == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "Handle invalido.", "Error : FindFirstFile", MB_OK);
		return;
	}
	while (FindNextFile(handle, &win32_find_data))
	{
		char *fileName = win32_find_data.cFileName;

		sprintf_s(tmp, "account/%s", fileName);

		if (!strstr(fileName, "."))
		{
			char szFolder[512];
			sprintf_s(szFolder, "account/%s/*", fileName);

			WIN32_FIND_DATA win32_find_data2;
			HANDLE handle2 = FindFirstFile(szFolder, &win32_find_data2);

			while (FindNextFile(handle2, &win32_find_data2))
			{
				char *fileName2 = win32_find_data2.cFileName;
				if (fileName[2] == '.')
					continue;

				sprintf_s(tmp, "Players/%s/%s", fileName, fileName2);

				std::string fileName{ fileName2 };
				std::string accountName = fileName.substr(0, fileName.find_first_of('.'));

				stAccount account{};
				memcpy(account.Username, accountName.c_str(), 16);
				
				if (cFileDB.DBReadAccount(&account))
				{
					TOD_ResetAccount reset{ &account };
					auto acc = reset.ResetAccount();

					cFileDB.DBWriteAccount(&acc);

					// Insere a hora no arquivo
					fprintf(pFile, "[CONTA %12s]\n%s\n", acc.Username, reset.GetReport().c_str());
				}
				else
					fprintf(pFile, "Falha ao abrir arquivo %s\n", tmp);
			}
		}
	}

	fclose(pFile);
	FindClose(handle);
}

void Log(const char* msg, ...)
{
	if (!(bool)serverLog)
		return;

	// Inicia a lista de argumentos
	va_list arglist;
	va_start(arglist, msg);

	serverLog->Log(msg, arglist);
	va_end(arglist);
}
bool CreateGUI()
{
	WNDCLASSEX wcex;
	const char* szWindowClass = "W2 OverDestiny - DataServer";
	const char* szWindowTitle = "W2 OverDestiny - DataServer v759";

	HINSTANCE hInstance = GetModuleHandle(NULL);
	bool Sucessful = true;

	wcex.cbSize = sizeof WNDCLASSEX;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(105));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(105));

	if (!(GUI.Class = RegisterClassEx(&wcex))) {
		MessageBoxA(0, "RegisterClassEx error", "Server::RegisterClassEx", 4096);
		return false;
	}
	else if (!(GUI.hGUI = CreateWindow(szWindowClass, szWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, NULL, NULL, hInstance, NULL))) {
		MessageBoxA(0, "CreateWindow error", "Server::CreateWindow", 4096);
		return false;
	}
	else if (!(GUI.Font = CreateFont(8, NULL, NULL, NULL, FW_DONTCARE, false, false, false, DEFAULT_CHARSET, NULL, NULL, DEFAULT_QUALITY, FF_DONTCARE, "MS Sans Serif"))) {
		MessageBoxA(0, "CreateFont error", "Server::CreateFont", 4096);
		return false;
	}

	if (!Sucessful)
		PostQuitMessage(1);

	SendMessage(GUI.hButton[0], WM_SETFONT, (WPARAM)GUI.Font, NULL);
	SendMessage(GUI.hLabel[0], WM_SETFONT, (WPARAM)GUI.Font, NULL);
	SendMessage(GUI.hLabel[1], WM_SETFONT, (WPARAM)GUI.Font, NULL);
	SendMessage(GUI.hLabel[2], WM_SETFONT, (WPARAM)GUI.Font, NULL);

	//ShowWindow(GUI.hGUI, SW_SHOW);
	UpdateWindow(GUI.hGUI);

	SetConsoleTitle(szWindowTitle);

	bool success = true;
	//////////////////////
	InitializeBaseDef();
	if (!ReadCharBase(0, "npc\\TK"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo TRANSKNIGHT", "Error : read", MB_OK);

		return false;
	}
	else if (!ReadCharBase(1, "npc\\FM"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo TRANSKNIGHT", "Error : read", MB_OK);

		return false;
	}
	else if (!ReadCharBase(2, "npc\\BM"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo TRANSKNIGHT", "Error : read", MB_OK);

		return false;
	}
	else if (!ReadCharBase(3, "npc\\HT"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo TRANSKNIGHT", "Error : read", MB_OK);

		return false;
	}
	else if (!ReadGuilds())
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo guilds.txt", "Error : read", MB_OK);

		return false;
	}

	memset(g_pGuildAlly, 0, sizeof g_pGuildAlly);
	memset(g_pGuildWar, 0, sizeof g_pGuildWar);
	memset(g_pTowerWarState, 0, sizeof g_pTowerWarState);

	sServer.FirstKefra = MAX_SERVERGROUP;

	INT32 wsaInitialize = Server[0].WSAInitialize();
	if (wsaInitialize == 0)
	{
		MessageBoxA(NULL, "err, wsainitialize fail", "-system", MB_OK);

		return false;
	}

	char name[255];
	if (!gethostname(name, 255))
	{
		hostent* hretn = gethostbyname(name);
		if (hretn)
		{
			char *cIp = inet_ntoa(*(struct in_addr *)*hretn->h_addr_list);

			sscanf_s(cIp, "%hhd %hhd %hhd %hhd", &LocalIP[0], &LocalIP[1], &LocalIP[2], &LocalIP[3]);
			strncpy_s(name, cIp, 255);
		}
	}

	if (!LocalIP[0])
	{
		MessageBox(NULL, "Can't get local address", "Reboot error", MB_OK);

		return false;
	}

	for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
	{
		if (!strcmp(g_pServerList[i][0], name))
		{
			sServer.ServerIndex = i;

			break;
		}
	}

	if (sServer.ServerIndex == -1)
	{
		MessageBoxA(NULL, "Can't get Server Group Index LOCALIP:", "", MB_OK | MB_SYSTEMMODAL);

		return true;
	}

	ReadConfig();
	ReadNPCDonate();
	Server[0].ListenServer(GUI.hGUI, 0, 0x1D5A, WSA_ACCEPT);

	//ListenSocket.StartListen(hWndMain, 0, DB_PORT, WSA_ACCEPT);
	AdminSocket.ListenServer(GUI.hGUI, 0, 8895, WSA_ACCEPTADMIN);

	serverLog = std::make_unique<TOD_Log>("..\\Logs\\DBSRV");
	Log("Dbsrv Iniciado com sucesso");

	SetTimer(GUI.hGUI, TIMER_SEC, 1000, NULL);
	return true;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	CreateGUI();
	CreateConsole();

	printf("Servidor ligado...\n");
	MSG Message;
	try
	{
		while (GetMessage(&Message, NULL, NULL, NULL) != 0)
		{
			TranslateMessage(&Message);
			DispatchMessageA(&Message);
		}
	}
	catch (std::exception& e)
	{
		printf("Excecao WINMAIN %s", e.what());
	}
}