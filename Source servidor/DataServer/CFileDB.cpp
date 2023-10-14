#include <io.h>
#include <fcntl.h>

#include "framework.h"
#include "DataServer.h"
#include "CPSock.h"
#include "CUser.h"
#include "CFileDB.h"
#include "Base.h"

using namespace std::string_literals;

CFileDB cFileDB = CFileDB();

CFileDB::CFileDB()
{
	for (INT i = 0; i < MAX_DBACCOUNT; i++)
	{
		Account[i].Login = 0;
		Account[i].SecurePass = 0;
		Account[i].Slot = -1;


		memset(&Account[i].Account, 0, sizeof stAccount);
	}
}

INT32 CFileDB::GetIndex(char* accountName)
{
	for (INT32 i = 0; i < MAX_DBACCOUNT; i++)
	{
		if (Account[i].Login == 0)
			continue;

		if (!strncmp(Account[i].Account.Username, accountName, 16))
			return i;
	}

	return 0;
}

INT32 CFileDB::GetIndex(INT32 server, INT32 id)
{
	return server * MAX_PLAYER + id;
}

INT32 CFileDB::SendDBMessage(INT32 server, INT32 id, const char* msg)
{
	MSG_MessagePanel packet;
	packet.Header.PacketId = _MSG_MessagePanel;
	packet.Header.ClientId = id;
	packet.Header.Size = sizeof MSG_MessagePanel;

	strncpy_s(packet.String, msg, 92);

	pUser[server].Sock.SendOneMessage((char*)&packet, sizeof MSG_MessagePanel);
	return true;
}

INT32 CFileDB::SendDBSignal(INT32 server, INT32 clientId, INT32 signal)
{
	PacketHeader header;
	header.PacketId = signal;
	header.Size = sizeof header;
	header.ClientId = clientId;

	pUser[server].Sock.SendOneMessage((char*)&header, sizeof PacketHeader);
	return true;
}

INT32 CFileDB::SendDBSignalParm2(INT32 server, INT32 clientId, INT32 packetId, INT32 parm1, INT32 parm2)
{
	MSG_STANDARDPARM3 packet;
	packet.Header.PacketId = packetId;
	packet.Header.ClientId = clientId;

	packet.Parm1 = parm1;
	packet.Parm2 = parm2;

	pUser[server].Sock.SendOneMessage((char*)&packet, sizeof MSG_STANDARDPARM3);
	return true;
}

INT32 CFileDB::SendDBSignalParm3(INT32 server, INT32 clientId, INT32 packetId, INT32 parm1, INT32 parm2, INT32 parm3)
{
	MSG_STANDARDPARM3 packet;
	packet.Header.PacketId = packetId;
	packet.Header.ClientId = clientId;

	packet.Parm1 = parm1;
	packet.Parm2 = parm2;
	packet.Parm3 = parm3;

	pUser[server].Sock.SendOneMessage((char*)&packet, sizeof MSG_STANDARDPARM3);
	return true;
}

INT32 CFileDB::SendDBSignalParm1(INT32 server, INT32 clientId, INT32 packetId, INT32 parm1)
{
	MSG_STANDARDPARM1 packet;
	packet.Header.PacketId = packetId;
	packet.Header.ClientId = clientId;

	packet.Parm1 = parm1;

	pUser[server].Sock.SendOneMessage((char*)&packet, sizeof MSG_STANDARDPARM3);
	return true;
}

INT32 CFileDB::DBWriteAccount(stAccountDB* acc)
{
	char accName[16];
	strncpy_s(accName, acc->Account.Username, 16);
	_strupr_s(accName);

	if (accName[0] == 'C' && accName[1] == 'O' && accName[2] == 'M' && accName[3] >= 0x30 && accName[3] <= 0x39 && accName[4] == 0)
		return false;
	if (accName[0] == 'L' && accName[1] == 'P' && accName[2] == 'T' && accName[3] >= 0x30 && accName[3] <= 0x39 && accName[4] == 0)
		return false;

	char temp[256];
	char firstKey[10];

	GetFirstKey(acc->Account.Username, firstKey);

	sprintf_s(temp, "./Account/%s/%s/Account.bin", firstKey, acc->Account.Username);
	int retn = WriteAccount(temp, acc->Account);

	if (!retn)
		return FALSE;

	for (int i = 0; i < 4; i++)
	{
		sprintf_s(temp, "./Account/%s/%s/Chars/%d.bin", firstKey, acc->Account.Username, i);
		retn = WriteChar(temp, acc->Mob[i]);

		if (!retn)
			return FALSE;
	}

	return TRUE;
}


INT32 CFileDB::DBReadAccount(stAccountDB* acc)
{
	acc->Account.Username[15] = 0;
	acc->Account.Username[14] = 0;
	acc->Account.Password[11] = 0;
	acc->Account.Password[10] = 0;

	char check[16] = { 0, };
	
	strncpy_s(check, acc->Account.Username, 15);
	_strupr_s(check);

	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;

	char temp[256] = { 0, };
	char firstKey[10] = { 0, };

	GetFirstKey(acc->Account.Username, firstKey);

	sprintf_s(temp, "./Account/%s/%s/Account.bin", firstKey, acc->Account.Username);

	int retn = ReadAccount(temp, &acc->Account);
	if (!retn)
		return FALSE;

	for (int i = 0; i < 4; i++)
	{
		sprintf_s(temp, "./Account/%s/%s/Chars/%d.bin", firstKey, acc->Account.Username, i);
		retn = ReadChar(temp, &acc->Mob[i]);

		if (!retn)
			return FALSE;
	}


	return TRUE;
}

INT32 CFileDB::GetEncPassword(int idx, int* Enc)
{
	Enc[0] = Rand() % 900 + 100;
	Enc[1] = Rand() % 900 + 100;
	Enc[2] = Rand() % 900 + 100;
	Enc[3] = Rand() % 900 + 100;
	Enc[4] = Rand() % 900 + 100;
	Enc[5] = Rand() % 900 + 100;
	Enc[6] = Rand() % 900 + 100;
	Enc[7] = Rand() % 900 + 100;

	return FALSE;
}

INT32 CFileDB::CreateCharacter(const char* ac, char* ch)
{
	if (!ac[0] || !ch[0])
		return false;

	char check[16] = { 0 };//LOCAL_33
	strncpy_s(check, ac, 15);
	_strupr_s(check);

	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;

	strncpy_s(check, ch, 15);
	_strupr_s(check);

	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;

	char temp[256] = { 0, };
	char firstKey[10] = { 0, };

	memset(firstKey, 0, sizeof firstKey);
	GetFirstKey(ch, firstKey);

	sprintf_s(temp, "./char/%s/%s", firstKey, ch);

	INT32 handle;
	_sopen_s(&handle, temp, O_RDONLY | O_BINARY, _SH_DENYNO, 0);//LOCAL_99

	if (handle != -1)
	{
		_close(handle);

		return false;
	}

	_sopen_s(&handle, temp, O_RDWR | O_CREAT | O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	if (handle == -1)
	{
		if (errno == 17)
		{
			printf("err, writeaccount EEXIST '%s' '%s'\n", ac, ch);
			return false;
		}
		if (errno == 13)
		{
			printf("err, writeaccount EACCES '%s' '%s'\n", ac, ch);
			return false;
		}
		if (errno == 22)
		{
			printf("err, writeaccount EINVAL '%s' '%s'\n", ac, ch);
			return false;
		}
		if (errno == 24)
		{
			printf("err, writeaccount EMFILE '%s' '%s'\n", ac, ch);
			return false;
		}
		if (errno == 2)
		{
			printf("err, writeaccount ENOENT '%s' '%s'", ac, ch);
			return false;
		}
	}

	_write(handle, ac, 16);

	_close(handle);
	return true;
}

INT32 CFileDB::DeleteCharacter(const char* ch, char* ac)
{
	char check[16] = { 0 };//LOCAL_33
	strncpy_s(check, ch, 15);
	_strupr_s(check);

	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;

	char temp[256] = { 0, };
	char firstKey[10] = { 0, };
	GetFirstKey(ch, firstKey);

	sprintf_s(temp, "./char/%s/%s", firstKey, ch);

	return DeleteFileA(temp);
}

void CFileDB::AddAccountList(INT32 index)
{
	if (Account[index].Login == 1) {
		// err addAccountList - already added

		return;
	}

	INT32 conn = index / MAX_PLAYER;
	pUser[conn].Count++;

	Account[index].Login = 1;
	Account[index].Slot = -1;
	Account[index].SecurePass = 0;
}

void CFileDB::RemoveAccountList(INT32 index)
{
	if (Account[index].Login == 0)
		return;

	INT32 conn = index / MAX_PLAYER;//LOCAL_2

	pUser[conn].Count--;
	Account[index].Login = 0;
	Account[index].Slot = -1;
	Account[index].SecurePass = 0;
	
	memset(&Account[index].Account, 0, sizeof stAccount);
	memset(&Account[index].Mob, 0, sizeof (Account[index].Mob));
}

void CFileDB::GetAccountByChar(char* acc, char* cha)
{
	char check[16];

	strncpy_s(check, cha, 15);
	check[15] = 0;

	_strupr_s(check);

	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= '0' && check[3] <= '9' && check[4] == 0)
		return;

	char first[16];
	memset(first, 0, 16);

	GetFirstKey(check, first);

	char temp[128];

	sprintf_s(temp, "./char/%s/%s", first, check);

	int Handle;
	_sopen_s(&Handle, temp, O_RDONLY | O_BINARY, _SH_DENYNO, _S_IREAD);

	if (Handle == -1)
	{
		return;
	}

	_read(Handle, acc, ACCOUNTNAME_LENGTH);
	_close(Handle);
}

void CFileDB::DBGetSelChar(st_CharList* p, stAccountDB* file)
{
	for (INT32 i = 0; i < 4; i++)
	{
		memcpy(p->Name[i], file->Mob[i].Player.Name, 16);
		memcpy(p->Equip[i], file->Mob[i].Player.Equip, sizeof st_Item * 18);

		if ((p->Equip[i][0].Index >= 22 && p->Equip[i][0].Index <= 25) || p->Equip[i][0].Index == 32)
			p->Equip[i][0].Index = p->Equip[i][0].EF2;

		p->GuildId[i] = file->Mob[i].Player.GuildIndex;
		p->PositionX[i] = file->Mob[i].Player.Last.X;
		p->PositionY[i] = file->Mob[i].Player.Last.Y;

		memcpy(&p->Status[i], &file->Mob[i].Player.Status, sizeof st_Status);

		p->Gold[i] = file->Mob[i].Player.Gold;
		p->Exp[i] = file->Mob[i].Player.Exp;
	}
}

void CFileDB::SendDBSavingQuit(INT32 id, INT32 mode)
{
	MSG_DBSavingQuit packet{};
	INT32 conn = id / MAX_PLAYER;
	INT32 idx = id % MAX_PLAYER;

	packet.Header.PacketId = _MSG_DBSavingQuit;
	packet.Header.ClientId = idx;
	packet.Header.Size = sizeof MSG_DBSavingQuit;

	packet.Mode = mode;

	memcpy(&packet.AccountName, &Account[id].Account.Username, 16);
}

void CFileDB::SendLog(int server, int clientId, const char* msg)
{
	std::string message{ msg };
	BufferWriter writer{};

	writer.Set<unsigned short>((short)message.size() + (short)12u);
	writer += 2;

	writer.Set<unsigned short>(LogPacket);
	writer.Set<unsigned short>(clientId);
	writer += 4;

	writer.Set<const char*>(message.c_str(), message.size());

	pUser[server].Sock.SendOneMessage((char*)writer.GetBuffer().data(), writer.GetBuffer().size());
}

void CFileDB::SendAdditionalAccountInfo(INT32 conn, INT32 clientId)
{
	INT32 idx = GetIndex(conn, clientId); //LOCAL_1052

	p415 packet{};
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x415;
	packet.Header.Size = sizeof packet;
	packet.Header.ClientId = clientId;

	packet.Access = Account[idx].Account.AccessLevel;
	packet.Unique = Account[idx].Account.Unique.Value;

	packet.Water.Total = Account[idx].Account.Water.Total;
	packet.Water.Day = Account[idx].Account.Water.Day;

	packet.Divina = Account[idx].Account.Divina;
	packet.Sephira = Account[idx].Account.Sephira;
	packet.SingleGift = Account[idx].Account.SingleGift;

	memcpy(&packet.Ban, &Account[idx].Account.Ban, sizeof stDate);
	packet.BanType = Account[idx].Account.BanType;

	packet.Daily.WeekYear = Account[idx].Account.Daily.WeekYear;
	memcpy(&packet.Daily.Received[0], &Account[idx].Account.Daily.Received[0], sizeof(bool) * 7);

	packet.IsBanned = Account[idx].IsBanned;
	packet.Cash = Account[idx].Account.Cash;
	pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof packet);
}

INT32 CFileDB::ProcessMessage(char* msg, INT32 conn)
{
	auto Header = (PacketHeader*)msg; 

	switch (Header->PacketId)
	{
	case _MSG_DBRequestCreateSubCele:
	{
		p830* p = (p830*)Header;

		INT32 index = GetIndex(conn, Header->ClientId),
			charPos = p->CharPos;

		if (charPos < 0 || charPos >= 4)
			break;

		INT32 baseFace = 0;
		if (p->Face < 10)
			baseFace = 6;
		else if (p->Face < 20)
			baseFace = 16;
		else if (p->Face < 30)
			baseFace = 26;
		else if (p->Face < 40)
			baseFace = 36;

		unsigned int learn = 0;
		for (int i = 0; i < 8; i++)
		{
			if (24 + i == 30)
				continue;

			int has = (p->Learn & (1 << (24 + i)));
			if (has)
				learn |= (1 << (24 + i));
		}

		learn |= (1 << 30);

		memset(Account[index].Mob[charPos].Sub.Equip, 0, sizeof st_Item * 2);

		Account[index].Mob[charPos].Sub.Equip[0].Index = (baseFace + p->ClassInfo);
		Account[index].Mob[charPos].Sub.Equip[0].EF2 = (baseFace + p->ClassInfo);
		Account[index].Mob[charPos].Sub.Equip[0].EFV2 = 4;

		Account[index].Mob[charPos].Sub.Status = 1;

		Account[index].Mob[charPos].Sub.Exp = 0;
		Account[index].Mob[charPos].Sub.Info.Value = 0;
		Account[index].Mob[charPos].Sub.Learn = learn;
		Account[index].Mob[charPos].Sub.Soul = 0;

		memcpy(&Account[index].Mob[charPos].Sub.SubStatus, &pBaseSet[p->ClassInfo].bStatus, sizeof st_Status);
		memset(Account[index].Mob[charPos].Sub.SkillBar, -1, 20);
	}
	break;

	case _MSG_DBCharacterLogin:
	{
		p213* p = (p213*)(Header);

		INT32 slot = p->CharIndex,//LOCAL_3102
			idx = GetIndex(conn, Header->ClientId);//LOCAL_3103

		SendLog(conn, Header->ClientId, "Recebido pedido para entrar no jogo");
		if (slot < 0 || slot >= 4)
		{
			SendLog(conn, Header->ClientId, ("Invalid charslot CharacterLogin"s + std::to_string(slot)).c_str());
			break;
		}

		if (!Account[idx].Mob[slot].Player.Name[0])
		{
			SendLog(conn, Header->ClientId, ("Invalid charname is empty CharacterLogin"s + std::to_string(slot)).c_str());
			return true;
		}

		Account[idx].Slot = slot;

		p114 packet{};
		memset(&packet, 0, sizeof packet);
		packet.Header.PacketId = _MSG_DBCNFCharacterLogin;
		packet.Header.ClientId = Header->ClientId;
		packet.Header.Size = sizeof p114;

		packet.SlotIndex = slot;
		packet.Mob = Account[idx].Mob[slot].Player;
		memcpy(&packet.SkillBar2, &Account[idx].Mob[slot].SkillBar[0], 16);

		p820 packetExtra{};
		memset(&packetExtra, 0, sizeof p820);
		packetExtra.Header.ClientId = p->Header.ClientId;
		packetExtra.Header.PacketId = _MSG_DBCNFCharacterLoginExtra;
		packetExtra.Header.Size = sizeof p820;

		stCharInfo* mob = &Account[idx].Mob[slot];
		packetExtra.Mob = *mob;

		strncpy_s(packetExtra.Pass, Account[idx].Account.Block.Pass, 16);
		packetExtra.Blocked = Account[idx].Account.Block.Blocked;

		memcpy(packet.Affect, Account[idx].Mob[slot].Affects, sizeof st_Affect * 32);

		pUser[conn].Sock.SendOneMessage((char*)&packetExtra, sizeof p820);
		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p114);

		SendLog(conn, Header->ClientId, "Enviado informações do personagem");
	}
	break;

	case _MSG_DBRequestNumericPassword:
	{
		pFDE* p = (pFDE*)Header;

		pFDE fde;
		memset(&fde, 0, sizeof pFDE);

		fde.Header.ClientId = p->Header.ClientId;
		fde.Header.PacketId = _MSG_DBCNFRequestNumericPass;
		fde.Header.Size = sizeof(pFDE);

		INT32 index = GetIndex(conn, p->Header.ClientId);

		if (Account[index].IsBanned)
		{
			pUser[conn].Sock.SendOneMessage((char*)&fde, sizeof pFDE);
			return true;
		}

		if (!Account[index].Account.SecondPass[0])
		{
			// Seta a senha numérica
			strncpy_s(Account[index].Account.SecondPass, p->num, 16);
			pUser[conn].Sock.SendOneMessage((char*)&fde, sizeof pFDE);
			DBWriteAccount(&Account[index]);

			Account[index].SecurePass = true;
		}
		else
		{
			if (p->RequestChange == 1)
			{
				// Copia a nova senha numérica para o usuário
				strncpy_s(Account[index].Account.SecondPass, p->num, 16);

				// Envia o pacote para o usuário, informando que está correta
				pUser[conn].Sock.SendOneMessage((char*)&fde, sizeof pFDE);
				DBWriteAccount(&Account[index]);
			}
			else if (!strncmp(p->num, Account[index].Account.SecondPass, 6))
			{
				pUser[conn].Sock.SendOneMessage((char*)&fde, sizeof pFDE);
				Account[index].SecurePass = true;

			}
			else
				SendDBSignal(conn, p->Header.ClientId, 0xFDF);
		}
	}
	break;

	case _MSG_DBNoNeedSave:
	{
		INT32 index = GetIndex(conn, Header->ClientId);

		char acc[16];//LOCAL_3335
		strncpy_s(acc, Account[index].Account.Username, 16);

		RemoveAccountList(index);
	}
	break;

	case _MSG_DBUpdateSapphire:
	{
		MSG_STANDARDPARM1* p = (MSG_STANDARDPARM1*)(Header);

		if (p->Parm1 == 1)
			cServer.Sapphire <<= 1;
		else
			cServer.Sapphire >>= 1;

		if (cServer.Sapphire < 1)
			cServer.Sapphire = 1;

		if (cServer.Sapphire > 35)
			cServer.Sapphire = 35;

		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			SendDBSignalParm1(i, 0, _MSG_DBUpdateSapphire, p->Parm1);
		}

		WriteConfig();
	}
	break;

	case _MSG_DBSaveMobQuit:
	{
		p807* p = reinterpret_cast<p807*>(Header);
		INT32 idx = GetIndex(conn, Header->ClientId),//LOCAL_3469
			slot = Account[idx].Slot;//LOCAL_3470

		Account[idx].Account.Username[15] = 0;
		Account[idx].Account.Username[14] = 0;

		char acc[16];//LOCAL_3502
		strncpy_s(acc, Account[idx].Account.Username, 16);

		p->User[15] = 0;
		p->User[14] = 0;

		if (strncmp(p->User, acc, 16) != 0)
		{
			printf("err, savenquit1\n");

			RemoveAccountList(idx);
			return true;
		}

		if (slot < 0 || slot >= 4 || p->CharSlot != slot)
		{
			printf("err, savenquit2 %s\n", Account[idx].Account.Username);

			RemoveAccountList(idx);
			return true;
		}

		if (Account[idx].Login == 0)
		{
			printf("%s - err,savenquit3\n", Account[idx].Account.Username);

			RemoveAccountList(idx);
			return true;
		}

		Account[idx].Mob[slot] = p->Mob;

		memcpy(Account[idx].Account.Storage.Item, p->Storage, sizeof st_Item * 128);
		memcpy(Account[idx].Skillbar, p->SkillBar, 8);

		memcpy(&Account[idx].Mob[slot].Player.SkillBar1[0], p->SkillBar, 4);
		memcpy(&Account[idx].Mob[slot].SkillBar[0], &p->SkillBar[4], 16);

		Account[idx].Account.Storage.Coin = p->Coin;
		Account[idx].Account.Insignias.Value = p->Insignia;

		strncpy_s(Account[idx].Account.Block.Pass, p->Pass, 16);
		Account[idx].Account.Block.Blocked = p->Blocked;

		Account[idx].Account.BanType = p->BanType;
		memcpy(&Account[idx].Account.Ban, &p->Ban, sizeof stDate);

		memcpy(&Account[idx].Account.Friends, &p->Friends, 30 * 16);

		Account[idx].Account.Cash = p->Cash;
		Account[idx].Account.Position.X = p->Position.X;
		Account[idx].Account.Position.Y = p->Position.Y;
		Account[idx].Account.CharSlot = -1;
		Account[idx].Account.Unique.Value = p->Unique;

		Account[idx].Account.Divina = p->Divina;
		Account[idx].Account.Sephira = p->Sephira;

		Account[idx].Account.Daily.WeekYear = p->Daily.WeekYear;
		memcpy(&Account[idx].Account.Daily.Received[0], p->Daily.Received, sizeof(p->Daily.Received));

		Account[idx].Account.Water.Day = p->Water.Day;
		Account[idx].Account.Water.Total = p->Water.Total;
		Account[idx].Account.SingleGift = p->SingleGift;

		DBWriteAccount(&Account[idx]);

		RemoveAccountList(idx);
		SendDBSignal(conn, Header->ClientId, 0x40B);
	}
	break;

	case _MSG_DBSaveMob:
	{
		p807* p = (p807*)(Header);
		INT32 index = GetIndex(conn, Header->ClientId);

		Account[index].Account.Username[15] = 0;
		Account[index].Account.Username[14] = 0;

		char acc[16];//LOCAL_3370
		strncpy_s(acc, Account[index].Account.Username, 16);

		p->User[15] = 0;
		p->User[14] = 0;

		INT32 slot = Account[index].Slot;//LOCAL_3371
		if (slot < 0 || slot >= 4)
		{
			// Log(err, savemob1 "
			printf("%s - err, savemob1. Slot %d\n", Account[index].Account.Username, slot);
			return 1;
		}

		// 00412D0E
		if (slot != p->CharSlot)
		{
			// Log(err, savemob2 "
			printf("%s - err, savemob2\n", Account[index].Account.Username);
			return true;
		}

		if (Account[index].Login == 0)
		{
			// Log(err, savemob3 
			printf("%s - err, savemob3\n", Account[index].Account.Username);

			return true;
		}

		Account[index].Mob[slot] = p->Mob;

		memcpy(&Account[index].Mob[slot].Player.SkillBar1[0], p->SkillBar, 4);
		memcpy(&Account[index].Mob[slot].SkillBar[0], &p->SkillBar[4], 16);

		memcpy(Account[index].Account.Storage.Item, p->Storage, sizeof st_Item * 128);
		//memcpy(Account[index].Skillbar, p->SkillBar, 8);

		Account[index].Account.BanType = p->BanType;
		memcpy(&Account[index].Account.Ban, &p->Ban, sizeof stDate);

		memcpy(&Account[index].Account.Friends, &p->Friends, 30 * 16);

		Account[index].Account.Insignias.Value = p->Insignia;

		strncpy_s(Account[index].Account.Block.Pass, p->Pass, 16);
		Account[index].Account.Block.Blocked = p->Blocked;
		Account[index].Account.Cash = p->Cash;
		Account[index].Account.Storage.Coin = p->Coin;
		Account[index].Account.Position = p->Position;
		Account[index].Account.CharSlot = p->Slot;
		Account[index].Account.Unique.Value = p->Unique;

		Account[index].Account.Divina = p->Divina;
		Account[index].Account.Sephira = p->Sephira;

		Account[index].Account.Daily.WeekYear = p->Daily.WeekYear;

		Account[index].Account.Water.Day = p->Water.Day;
		Account[index].Account.Water.Total = p->Water.Total;
		Account[index].Account.SingleGift = p->SingleGift;

		memcpy(&Account[index].Account.Daily.Received[0], p->Daily.Received, sizeof(p->Daily.Received));

		DBWriteAccount(&Account[index]);
	}
	break;

	case _MSG_DBNewArch:
	{
		MSG_DBNewArch* p = (MSG_DBNewArch*)(Header);

		INT32 slotId = p->PosID,
			classId = p->ClassID,
			index = GetIndex(conn, Header->ClientId);

		if (slotId < 0 || slotId >= 4)
		{
			//Log( err, new char slot out of range
			SendDBSignal(conn, Header->ClientId, 0x41D);
			return true;
		}

		char check[16];//LOCAL_2843
		strncpy_s(check, p->Name, 16);

		_strupr_s(check);

		if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[4] <= 0x39)
		{
			//Log(err, newchar - com
			SendDBSignal(conn, Header->ClientId, 0x41D);
			return true;
		}
		if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[4] <= 0x39)
		{
			//Log(err, newchar - com
			SendDBSignal(conn, Header->ClientId, 0x41D);
			return true;
		}

		st_Mob* mob = &Account[index].Mob[slotId].Player;//LOCAL_2844
		stCharInfo* extra = &Account[index].Mob[slotId];

		memset(extra, 0, sizeof stCharInfo);

		mob->Name[15] = 0;
		mob->Name[14] = 0;

		p->Name[15] = 0;
		p->Name[14] = 0;

		if (mob->Name[0])
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);
			// Log(err, newchar already charged
			return true;
		}

		p->Name[15] = 0;
		p->Name[14] = 0;

		INT32 len = strlen(p->Name);//LOCAL_2845
		for (INT32 i = 0; i < len; i++)
		{
			if (p->Name[i] == -95 && p->Name[i + 1] == -95)
			{
				SendDBSignal(conn, Header->ClientId, 0x41D);
				return true;
			}
		}

		memset(extra, 0, sizeof stCharInfo);

		ClearMob(mob);

		INT32 body = 0;
		if (p->ClassID >= 6 && p->ClassID < 10)
			body = 0;
		else if (p->ClassID >= 16 && p->ClassID < 20)
			body = 1;
		else if (p->ClassID >= 26 && p->ClassID < 30)
			body = 2;
		else if (p->ClassID >= 36 && p->ClassID < 40)
			body = 3;
		else
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			//lOG(err, newchar fail - undefined class
			return true;
		}

		if (body == 0)
			memcpy(mob, &pBaseSet[0], sizeof st_Mob);
		else if (body == 1)
			memcpy(mob, &pBaseSet[1], sizeof st_Mob);
		else if (body == 2)
			memcpy(mob, &pBaseSet[2], sizeof st_Mob);
		else if (body == 3)
			memcpy(mob, &pBaseSet[3], sizeof st_Mob);
		else
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			//lOG(err, newchar fail - undefined class
			return true;
		}

		memcpy(mob->Name, p->Name, 16);

		memset(extra->SkillBar, -1, 16);
		memset(mob->SkillBar1, -1, 4);

		//memset(&mob->Equip[1], 0, sizeof st_Item);

		for (int i = 1; i < 17; i++)//alterado
		{
			memset(&mob->Equip[i], 0, sizeof st_Item);
		}

		memset(&mob->Inventory[0], 0, sizeof st_Item * 64);


		//mob->Equip[1].Index = 3500;

		mob->Equip[0].Index = p->ClassID;
		mob->Equip[0].EFV2 = 2;
		mob->Equip[0].EF2 = p->ClassID;

		extra->MortalSlot = p->MortalSlot;

		mob->ClassInfo = p->ClassInfo;

		INT32 ret = DBWriteAccount(&Account[index]);//LOCAL_2847
		if (ret == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			//lOG(err, newchar fail - create file
			return true;
		}

		//Log ( createchar %s

		p418 packet;
		memset(&packet, 0, sizeof p418);

		packet.Header.PacketId = _MSG_DBCNFNewCharacter;
		packet.Header.Size = sizeof p418;

		DBGetSelChar(&packet.CharList, &Account[index]);

		packet.Header.ClientId = Header->ClientId;

		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
	}
	break;

	case _MSG_DBNewCharacter:
	{
		p20F* p = (p20F*)(Header);

		INT32 slot = p->SlotID,//LOCAL_2837
			cls = p->ClassID,//LOCAL_2838
			idx = GetIndex(conn, Header->ClientId);//LOCAL_2839

		if (slot < 0 || slot >= 4)
		{
			//Log( err, new char slot out of range
			SendDBSignal(conn, Header->ClientId, 0x41D);
			return true;
		}

		if (cls != 0 && cls != 1 && cls != 2 && cls != 3)
		{
			//Log( err, new char class out of range
			SendDBSignal(conn, Header->ClientId, 0x41D);
			return true;
		}

		char check[16];//LOCAL_2843
		strncpy_s(check, p->Nick, 16);

		_strupr_s(check);

		if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[4] <= 0x39)
		{
			//Log(err, newchar - com
			SendDBSignal(conn, Header->ClientId, 0x41D);
			return true;
		}
		if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[4] <= 0x39)
		{
			//Log(err, newchar - com
			SendDBSignal(conn, Header->ClientId, 0x41D);
			return true;
		}

		st_Mob* mob = &Account[idx].Mob[slot].Player;//LOCAL_2844
		stCharInfo* extra = &Account[idx].Mob[slot];

		memset(extra, 0, sizeof stCharInfo);

		mob->Name[15] = 0;
		mob->Name[14] = 0;

		p->Nick[15] = 0;
		p->Nick[14] = 0;

		if (mob->Name[0])
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);
			// Log(err, newchar already charged
			return true;
		}

		p->Nick[15] = 0;
		p->Nick[14] = 0;

		INT32 len = strlen(p->Nick);//LOCAL_2845
		for (INT32 i = 0; i < len; i++)
		{
			if (p->Nick[i] == 0x2D)
				continue;

			if (!isalnum(p->Nick[i]))
			{
				SendDBSignal(conn, Header->ClientId, 0x41D);

				return true;
			}
		}

		// 00412710
		INT32 ret = CreateCharacter(Account[idx].Account.Username, p->Nick);//LOCAL_2847
		if (ret == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			return true;
		}

		ClearMob(mob);

		if (cls == 0)
			memcpy(mob, &pBaseSet[0], sizeof st_Mob);
		else if (cls == 1)
			memcpy(mob, &pBaseSet[1], sizeof st_Mob);
		else if (cls == 2)
			memcpy(mob, &pBaseSet[2], sizeof st_Mob);
		else if (cls == 3)
			memcpy(mob, &pBaseSet[3], sizeof st_Mob);
		else
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			//lOG(err, newchar fail - undefined class
			return true;
		}

		memset(extra->SkillBar, -1, 16);
		memset(mob->SkillBar1, -1, 4);

		mob->Equip[0].EFV2 = 1;
		mob->Equip[0].EF2 = mob->Equip[0].Index & 127;

		memcpy(mob->Name, p->Nick, 16);

		ret = DBWriteAccount(&Account[idx]);
		if (ret == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			//lOG(err, newchar fail - create file
			return true;
		}

		//Log ( createchar %s

		p418 packet;
		memset(&packet, 0, sizeof p418);

		packet.Header.PacketId = _MSG_DBCNFNewCharacter;
		packet.Header.Size = sizeof p418;

		DBGetSelChar(&packet.CharList, &Account[idx]);

		packet.Header.ClientId = Header->ClientId;

		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
	}
	break;

	case _MSG_DBAccountLogin:
	{
		p20D* p = (p20D*)Header; // 1050
		_strupr_s(p->Login);

		const char* login = p->Login;
		if (login[0] == 'C' && login[1] == 'O' && login[2] == 'M' && login[3] >= 0x30 && login[4] <= 0x39 && login[5] == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x421);

			return false;
		}

		if (login[0] == 'L' && login[1] == 'P' && login[2] == 'T' && login[3] >= 0x30 && login[4] <= 0x39 && login[5] == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x421);

			return false;
		}

		INT32 idx = GetIndex(conn, Header->ClientId); //LOCAL_1052
		INT32 idxName = GetIndex(p->Login);//LOCAL_1053

		stAccountDB taccount{};//LOCAL_2126
		memcpy(taccount.Account.Username, p->Login, 16);

		INT32 ret = DBReadAccount(&taccount);//LOCAL_2127
		if (ret == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x421);

			return true;
		}

		stAccount account{};
		memcpy(&account, &taccount.Account, sizeof(stAccount));

		if (account.Storage.Coin < 0)
			account.Storage.Coin = 0;

		/*if (pUser[conn].Staff == 1 && account.AccessLevel == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x426);

			return true;
		}*/

		if (account.Year != 0 && account.YearDay != 0)
		{
			struct tm when;
			time_t now = time(&now);
			localtime_s(&when, &now);

			if (account.Year >= when.tm_year || account.Year >= when.tm_year && account.YearDay >= when.tm_yday)
			{
				SendDBSignal(conn, Header->ClientId, 0x424);
				return TRUE;
			}
		}

		INT32 changeServer = 0;
		if (account.TempKey[0] != 0 && p->Unknow[0] != 0)
		{
			if (strncmp(account.TempKey, p->Unknow, strlen(account.TempKey)) == 0)
			{
				memset(account.TempKey, 0, 52);
				changeServer = 1;
			}
			else
			{
				memset(account.TempKey, 0, 52);
				DBWriteAccount(&taccount);

				return true;
			}
		}

		INT32 master = strcmp(p->Password, "DEFINAASENHAMESTRAAQUI");
		INT32 banType = account.BanType;
		if (banType != 0 && account.Ban.Ano != 0 && account.Ban.Mes != 0 && account.Ban.Dia != 0 && master != 0)
		{
			double remaining = TimeRemaining(account.Ban.Dia, account.Ban.Mes, account.Ban.Ano, account.Ban.Hora, account.Ban.Minuto);
			if (remaining > 0.0)
			{
				if (banType == 1)
					SendDBSignal(conn, Header->ClientId, 0x424);
				else if (banType == 2)
					SendDBSignal(conn, Header->ClientId, 0x425);
				else if (banType == 3)
					SendDBSignal(conn, Header->ClientId, 0x424);

				return true;
			}
			else
			{
				account.BanType = 0;

				memset(&account.Ban, 0, sizeof stDate);
			}
		}

		if (strcmp(p->Password, account.Password) != 0 && !changeServer && master)
		{
			if (account.Password[0] == '_')
				SendDBSignal(conn, Header->ClientId, 0x421);

			if (account.Password[0] == '@')
				SendDBSignal(conn, Header->ClientId, 0x424);

			if (account.Password[0] == '#')
				SendDBSignal(conn, Header->ClientId, 0x425);
			else
				SendDBSignal(conn, Header->ClientId, 0x422);

			return true;
		}

		if (idxName == idx)
			return true;

		// 00411FAB
		if (idxName != 0)
		{
			// Log( err,disconect previous connection
			if (p->Unknow_84 == 0)
			{
				SendDBSignal(conn, Header->ClientId, 0x41F);

				return true;
			}

			SendDBSignal(conn, Header->ClientId, 0x420);
			SendDBSavingQuit(idxName, 0);
			return true;
		}

		_strupr_s(account.Username);

		INT32 left = -1,//LOCAL_2128
			right = -1,//LOCAL_2129
			q = 0;//LOCAL_2130

		for (; q < 4; q++)
		{
			if (taccount.Mob[q].Player.Equip[13].Index == 774)
				left = q;
			if (taccount.Mob[q].Player.Equip[13].Index == 775)
				right = q;
		}

		if (left != -1 && right != -1)
		{
			char temp[16];//LOCAL_2134l
			memcpy(temp, taccount.Mob[left].Player.Name, 16);
			memcpy(taccount.Mob[left].Player.Name, taccount.Mob[right].Player.Name, 16);
			memcpy(taccount.Mob[right].Player.Name, temp, 16);

			memset(&taccount.Mob[left].Player.Equip[13], 0, sizeof st_Item);
			memset(&taccount.Mob[right].Player.Equip[13], 0, sizeof st_Item);
			DBWriteAccount(&taccount);
		}

		stAccount* pAccount = &Account[idx].Account;//LOCAL_2199
		memcpy(pAccount, &account, sizeof stAccount);

		for (int i = 0; i < 4; i++)
		{
			stCharInfo* charInfo = &Account[idx].Mob[i];
			memcpy(charInfo, &taccount.Mob[i], sizeof stCharInfo);
		}

		AddAccountList(idx);

		Account[idx].IsBanned = master == 0;

		p416 selChar{};//LOCAL_2385
		memset(&selChar, 0, sizeof p416);

		DBGetSelChar(&selChar.CharList, &taccount);

		selChar.Header.PacketId = _MSG_DBCNFAccountLogin;
		selChar.Header.ClientId = Header->ClientId;

		strncpy_s(selChar.UserName, account.Username, 16);
		memcpy(&selChar.Storage[0], &account.Storage.Item[0], sizeof st_Item * 128);
		selChar.GoldStorage = pAccount->Storage.Coin;

		pUser[conn].Sock.SendOneMessage((char*)&selChar, sizeof selChar);

		SendAdditionalAccountInfo(conn, Header->ClientId);

		if (changeServer)
		{
			DBWriteAccount(&taccount);

			INT32 slot = 0;
			INT32 server = 0;
			INT32 enc[8] = { 0 };

			sscanf_s(p->Unknow, "%d %d %d %d %d %d %d %d %d %d", &server, &enc[0], &enc[1], &enc[2], &enc[3], &enc[4], &enc[5], &enc[6], &enc[7], &slot);

			Account[idx].Slot = slot;

			p114 packet{};
			packet.Header.PacketId = _MSG_DBCNFCharacterLogin;
			packet.Header.ClientId = Header->ClientId;
			packet.Header.Size = sizeof p114;

			packet.SlotIndex = slot;
			packet.Mob = Account[idx].Mob[slot].Player;

			memcpy(&packet.SkillBar2, &Account[idx].Mob[slot].SkillBar[0], 16);

			if (!Account[idx].Mob[slot].Player.Name[0])
			{
				//Log( err, charlogin mobname empty
				return true;
			}

			p820 packetExtra;
			memset(&packetExtra, 0, sizeof p820);
			packetExtra.Header.ClientId = p->Header.ClientId;
			packetExtra.Header.PacketId = _MSG_DBCNFCharacterLoginExtra;
			packetExtra.Header.Size = sizeof p820;

			stCharInfo* mob = &Account[idx].Mob[slot];
			memcpy(&packetExtra.Mob, mob, sizeof stCharInfo);
			packetExtra.Cash = Account[idx].Account.Cash;

			strncpy_s(packetExtra.Pass, Account[idx].Account.Block.Pass, 16);
			packetExtra.Blocked = Account[idx].Account.Block.Blocked;

			memcpy(packet.Affect, Account[idx].Mob[slot].Affects, sizeof st_Affect * 32);

			pUser[conn].Sock.SendOneMessage((char*)&packetExtra, sizeof p820);
			pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p114);
		}
	}
	break;

	case _MSG_DBDeleteCharacter:
	{
		p211* p = (p211*)(Header);

		INT32 index = GetIndex(conn, p->Header.ClientId),
			slot = p->SlotIndex;//LOCAL_3602

		if (slot < 0 || slot >= 4)
		{
			SendDBSignal(conn, Header->ClientId, 0x41E);
			// Log(err, deletechar slot
			return true;
		}
		st_Mob* mob = &Account[index].Mob[slot].Player;//LOCAL_3667
		if (Account[index].Mob[slot].Player.Equip[0].EFV2 >= 3)
		{
			SendDBSignal(conn, Header->ClientId, 0x41E);
			// Log(err,deletechar level 80);
			return true;
		}

		if (strncmp(p->Pwd, Account[index].Account.Password, 16))
		{
			SendDBSignal(conn, Header->ClientId, 0x41F);

			return true;
		}

		if (!mob->Name[0])
			return true;

		// 004133EC
		Account[index].Skillbar[slot][0] = 255;
		Account[index].Skillbar[slot][1] = 255;
		Account[index].Skillbar[slot][2] = 255;
		Account[index].Skillbar[slot][3] = 255;
		Account[index].Skillbar[slot][4] = 255;
		Account[index].Skillbar[slot][5] = 255;
		Account[index].Skillbar[slot][6] = 255;
		Account[index].Skillbar[slot][7] = 255;

		DeleteCharacter(mob->Name, Account[index].Account.Username);

		memset(&Account[index].Mob[slot], 0, sizeof stCharInfo);

		// log( delete char %s
		ClearMob(mob);

		DBWriteAccount(&Account[index]);

		p418 packet;
		packet.Header.PacketId = _MSG_DBCNFDeleteCharacter;

		DBGetSelChar(&packet.CharList, &Account[index]);

		packet.Header.ClientId = Header->ClientId;

		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
	}
	break;

	}

	return TRUE;
}
