#include "Basedef.h"
#include "Server.h"
#include "CFileDB.h"
#include "UOD_SealSoul.h"
#include "UOD_ResetAccount.h"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <filesystem>
#include "pugixml.hpp"
#include "BufferReaderWriter.h"
#include "UOD_Log.h"

using namespace std::string_literals;
CFileDB cFileDB;

CFileDB::CFileDB()
{
	for (INT i = 0; i < MAX_DBACCOUNT; i++)
	{
		Account[i].Login = 0;
		Account[i].Slot = -1;

		memset(&Account[i].Account, 0, sizeof stAccount);
	}
}

INT32 CFileDB::GetIndex(INT32 server, INT32 id)
{
	return server * MAX_PLAYER + id;
}

void CFileDB::GetAccountByChar(char *acc, char *cha)
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

INT32 CFileDB::GetIndex(char *accountName)
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

INT32 CFileDB::SendDBMessage(INT32 server, INT32 id, const char *msg)
{
	MSG_MessagePanel packet;
	packet.Header.PacketId = _MSG_MessagePanel;
	packet.Header.ClientId = id;
	packet.Header.Size = sizeof MSG_MessagePanel;

	strncpy_s(packet.String, msg, 92);

	pUser[server].Sock.SendOneMessage((char*)&packet, sizeof MSG_MessagePanel);
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

INT32 CFileDB::SendDBSignalParm1(INT32 server, INT32 clientId, INT32 packetId, INT32 parm1)
{
	MSG_STANDARDPARM1 packet;
	packet.Header.PacketId = packetId;
	packet.Header.ClientId = clientId;

	packet.Parm1 = parm1;

	pUser[server].Sock.SendOneMessage((char*)&packet, sizeof MSG_STANDARDPARM3);
	return true;
}

INT32 CFileDB::DBReadAccount(stAccount *file)
{
	file->Username[15] = 0;
	file->Username[14] = 0;
	file->Password[11] = 0;
	file->Password[10] = 0;

	char check[16]; //LOCAL_33
	strncpy_s(check, file->Username, 15);
	_strupr_s(check);

	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;

	char temp[256];//LOCAL_65
	char firstKey[10];//LOCAL_97
	GetFirstKey(file->Username, firstKey);

	sprintf_s(temp, "./account/%s/%s.xml", firstKey, file->Username);

	try 
	{
		std::error_code code;
		if (!std::filesystem::exists(temp, code))
			return false;

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(temp, pugi::parse_full);
		if (!result)
			return 0;

		auto accNode = doc.child("account");

		XMLToStructure(accNode, file);
	}
	catch(std::exception& e)
	{
		return 0;
	}
	return 1;
}

INT32 CFileDB::DBWriteAccount(stAccount *acc)
{
	char accName[16];//LOCAL_33
	strncpy_s(accName, acc->Username, 16);
	_strupr_s(accName);

	if (accName[0] == 'C' && accName[1] == 'O' && accName[2] == 'M' && accName[3] >= 0x30 && accName[3] <= 0x39 && accName[4] == 0)
		return false;
	if (accName[0] == 'L' && accName[1] == 'P' && accName[2] == 'T' && accName[3] >= 0x30 && accName[3] <= 0x39 && accName[4] == 0)
		return false;

	char temp[256];//LOCAL_66
	char firstKey[10];//LOCAL_98
	GetFirstKey(acc->Username, firstKey);

	sprintf_s(temp, "./account/%s/%s.xml", firstKey, acc->Username);

	pugi::xml_document doc;
	auto account = doc.append_child("account");

	AppendStructure(account, acc);

	doc.save_file(temp);
	return true;
}

INT32 CFileDB::DBExportAccount(stAccount *file)
{
	stAccount *accName = file;//LOCAL_2

	char check[16];//LOCAL_34
	strncpy_s(check, accName->Username, 16);

	_strupr_s(check);
	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;

	char temp[256];//LOCAL_66
	sprintf_s(temp, "S:\\export\\account%d\\%s", sServer.ServerIndex, accName->Username);

	INT32 handle;
	_sopen_s(&handle, temp, _O_CREAT | _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);//LOCAL_67
	if (handle == -1)
		return false;

	INT32 ret = _lseek(handle, 0, 0);//LOCAL_68
	if (ret == -1)
	{
		_close(handle);
		return false;
	}

	ret = _write(handle, file, sizeof stAccountDB);
	if (ret == -1)
	{
		_close(handle);
		return false;
	}

	_close(handle);
	return true;
}

INT32 CFileDB::DeleteCharacter(const char *ch, char *ac)
{
	char check[16] = { 0 };//LOCAL_33
	strncpy_s(check, ch, 15);
	_strupr_s(check);

	if (check[0] == 'C' && check[1] == 'O' && check[2] == 'M' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;
	if (check[0] == 'L' && check[1] == 'P' && check[2] == 'T' && check[3] >= 0x30 && check[3] <= 0x39 && check[4] == 0)
		return false;

	char temp[256];//LOCAL_65
	char firstKey[10];//LOCAL_97
	GetFirstKey(ch, firstKey);

	sprintf_s(temp, "./char/%s/%s", firstKey, ch);

	return DeleteFileA(temp);
}

INT32 CFileDB::CreateCharacter(const char *ac, char *ch)
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

	char temp[256];//LOCAL_66
	char firstKey[10];//LOCAL_98
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

void CFileDB::DBGetSelChar(st_CharList *p, stAccount *file)
{
	for (INT32 i = 0; i < 4; i++)
	{
		memcpy(p->Name[i], file->Mob[i].Player.Name, 16);
		memcpy(p->Equip[i], file->Mob[i].Player.Equip, sizeof st_Item * 16);

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

void CFileDB::AddAccountList(INT32 index)
{
	if (Account[index].Login == 1){
		// err addAccountList - already added

		return;
	}

	INT32 conn = index / MAX_PLAYER;//LOCAL_2
	pUser[conn].Count++;

	Account[index].Login = 1;
	Account[index].Slot = -1;
}

void CFileDB::RemoveAccountList(INT32 index)
{
	if (Account[index].Login == 0)
		return;

	INT32 conn = index / MAX_PLAYER;//LOCAL_2

	pUser[conn].Count--;
	Account[index].Login = 0;
	Account[index].Slot = -1;
	Account[index].IsBanned = false;

	memset(&Account[index].Account, 0, sizeof stAccount);
}

double TimeRemaining(int dia, int mes, int ano, int hora, int min)
{
    time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);

	int month  = now.tm_mon; //0 Janeiro, 1 Fev
	int day    = now.tm_mday;
	int year   = now.tm_year;

	struct std::tm a = {0, now.tm_min, now.tm_hour, day, month, year};
	struct std::tm b = {0, min, hora, dia, mes - 1, ano-1900};

	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	if ( x != (std::time_t)(-1) && y != (std::time_t)(-1) )
	{
		double difference = (std::difftime(y, x) / (60.0 * 60.0 * 24.0));
		return difference;
	}

	return 0;
}

INT32 CFileDB::ProcessMessage(char *msg, INT32 conn)
{
	PacketHeader *Header = (PacketHeader*)msg; // LOCAL-2;
	INT32 packetId = Header->PacketId; // LOCAL_3857

	switch (packetId)
	{

	case _MSG_DBRequestCreateSubCele:
	{
		p830 *p = (p830*)Header;

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

		memset(Account[index].Account.Mob[charPos].Sub.Equip, 0, sizeof st_Item * 2);

		Account[index].Account.Mob[charPos].Sub.Equip[0].Index = (baseFace + p->ClassInfo);
		Account[index].Account.Mob[charPos].Sub.Equip[0].EF2 = (baseFace + p->ClassInfo);
		Account[index].Account.Mob[charPos].Sub.Equip[0].EFV2 = 4;

		Account[index].Account.Mob[charPos].Sub.Status = 1;

		Account[index].Account.Mob[charPos].Sub.Exp = 0;
		Account[index].Account.Mob[charPos].Sub.Info.Value = 0;
		Account[index].Account.Mob[charPos].Sub.Learn = learn;
		Account[index].Account.Mob[charPos].Sub.Soul = 0;

		memcpy(&Account[index].Account.Mob[charPos].Sub.SubStatus, &pBaseSet[p->ClassInfo].bStatus, sizeof st_Status);
		memset(Account[index].Account.Mob[charPos].Sub.SkillBar, -1, 20);
	}
	break;
	case MSG_FRIENDLIST_REQUESTUPDATE_OPCODE:
	{
		INT32 idx = GetIndex(conn, Header->ClientId);//LOCAL_3103

		_MSG_FRIENDLIST fList{};
		memset(&fList, 0, sizeof _MSG_FRIENDLIST);

		fList.Header.PacketId = MSG_FRIENDLIST_OPCODE;
		fList.Header.ClientId = Header->ClientId;
		fList.Header.Size = sizeof _MSG_FRIENDLIST;

		for(INT32 i = 0; i < 30; i ++)
		{
			if(!Account[idx].Account.Friends[i][0])
				continue;

			strncpy_s(fList.Name[i], Account[idx].Account.Friends[i], 16);

			INT32 memberId = -1;
			for (INT32 t = 0; t < MAX_DBACCOUNT; t++)
			{
				if (Account[t].Login == 0)
					continue;
				
				INT32 slot = Account[t].Slot;
				if(slot < 0 || slot >= 4)
					continue;

				if(Account[t].Account.AccessLevel != 0)
					continue;

				if (!strncmp(Account[t].Account.Mob[slot].Player.Name, Account[idx].Account.Friends[i], 16))
				{
					memberId = t;
					break;
				}
			}

			if(memberId == -1)
			{
				fList.Status [i] = 2;
				continue;
			}

			fList.Status [i] = 1;
			fList.Server [i] = (idx / 1000) + 1;
		}

		pUser[conn].Sock.SendOneMessage((char*)&fList, sizeof _MSG_FRIENDLIST);
	}
	break;

	case _MSG_DBCharacterLogin:
	{
		p213 *p = (p213*)(Header);

		INT32 slot = p->CharIndex,//LOCAL_3102
			idx = GetIndex(conn, Header->ClientId);//LOCAL_3103

		SendLog(conn, Header->ClientId, "Recebido pedido para entrar no jogo");
		if (slot < 0 || slot >= 4)
		{
			SendLog(conn, Header->ClientId, ("Invalid charslot CharacterLogin"s + std::to_string(slot)).c_str());
			break;
		}

		if (!Account[idx].Account.Mob[slot].Player.Name[0])
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
		packet.Mob = Account[idx].Account.Mob[slot].Player;
		memcpy(&packet.SkillBar2, &Account[idx].Account.Mob[slot].SkillBar[0], 16);

		p820 packetExtra{};
		memset(&packetExtra, 0, sizeof p820);
		packetExtra.Header.ClientId = p->Header.ClientId;
		packetExtra.Header.PacketId = _MSG_DBCNFCharacterLoginExtra;
		packetExtra.Header.Size = sizeof p820;
		
		stCharInfo *mob = &Account[idx].Account.Mob[slot];
		packetExtra.Mob = *mob;

		strncpy_s(packetExtra.Pass, Account[idx].Account.Block.Pass, 16);
		packetExtra.Blocked = Account[idx].Account.Block.Blocked;
			
		memcpy(packet.Affect, Account[idx].Account.Mob[slot].Affects, sizeof st_Affect * 32);

		pUser[conn].Sock.SendOneMessage((char*)&packetExtra, sizeof p820);
		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p114);

		SendLog(conn, Header->ClientId, "Enviado informações do personagem");
	
		if(Account[idx].Account.AccessLevel == 0)
		{
			_MSG_FRIENDLIST_UPDATESTATUS f;
			memset(&f, 0, sizeof _MSG_FRIENDLIST_UPDATESTATUS);

			f.Header.ClientId = Header->ClientId;
			f.Header.PacketId = MSG_FRIENDLIST_UPDATESTATUS_OPCODE;
			f.Header.Size = sizeof _MSG_FRIENDLIST_UPDATESTATUS;

			strncpy_s(f.Name, packet.Mob.Name, 16);
			f.Status = 1;
			f.Server = conn + 1;

			for(INT32 h = 0; h < MAX_SERVERGROUP;h ++)
			{
				if (pUser[h].Mode == 0 || pUser[h].Sock.Sock == 0)
					continue;

				pUser[h].Sock.SendOneMessage((char*)&f, sizeof _MSG_FRIENDLIST_UPDATESTATUS);
			}
		}
	}
	break;
	case _MSG_DBNoNeedSave:
	{
		INT32 index = GetIndex(conn, Header->ClientId);

		char acc[16];//LOCAL_3335
		strncpy_s(acc, Account[index].Account.Username, 16);

		RemoveAccountList(index);

		// ADMIN - 00412B76
		//for(INT32 i = 0; i < 
		for (int i = 0; i < MAX_ADMIN; i++)
		{
			if (pAdmin[i].Level <= 2)
				continue;

			if (pAdmin[i].DisableID == 0)
				continue;

			if (pAdmin[i].DisableID != index)
				continue;

			DisableAccount(i, acc, pAdmin[i].Year, pAdmin[i].YearDay);
		}
	}
	break;
	case _MSG_DBUpdateSapphire:
	{
		MSG_STANDARDPARM1 *p = (MSG_STANDARDPARM1*)(Header);

		if (p->Parm1 == 1)
			sServer.Sapphire <<= 1;
		else
			sServer.Sapphire >>= 1;

		if (sServer.Sapphire < 1)
			sServer.Sapphire = 1;

		if (sServer.Sapphire > 35)
			sServer.Sapphire = 35;

		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			SendDBSignalParm1(i, 0, _MSG_DBUpdateSapphire, p->Parm1);
		}

		WriteConfig();
	}
	break;
	case _MSG_DBStaffMode:
	{
		MSG_STANDARDPARM2 *p = (MSG_STANDARDPARM2 *)(Header);

		if (p->Parm1 == 1)
			pUser[conn].Staff = p->Parm2;
	}
	break;

	case _MSG_DBSaveMobQuit:
	{//00412EE7
		p807 *p = reinterpret_cast<p807*>(Header);
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

		if(p->Arg2 && Account[idx].Account.AccessLevel == 0)
		{
			_MSG_FRIENDLIST_UPDATESTATUS f;
			memset(&f, 0, sizeof _MSG_FRIENDLIST_UPDATESTATUS);

			f.Header.ClientId = Header->ClientId;
			f.Header.PacketId = MSG_FRIENDLIST_UPDATESTATUS_OPCODE;
			f.Header.Size = sizeof _MSG_FRIENDLIST_UPDATESTATUS;
					
			strncpy_s(f.Name, p->Mob.Player.Name, 16);

			f.Status = 2;
			f.Server = 0;

			for(INT32 h = 0; h < MAX_SERVERGROUP;h ++)
			{
				if (pUser[h].Mode == 0 || pUser[h].Sock.Sock == 0)
					continue;

				pUser[h].Sock.SendOneMessage((char*)&f, sizeof _MSG_FRIENDLIST_UPDATESTATUS);
			}
		}

		Account[idx].Account.Mob[slot] = p->Mob;

		memcpy(Account[idx].Account.Storage.Item, p->Storage, sizeof st_Item * 128);
		memcpy(Account[idx].Skillbar, p->SkillBar, 8);

		memcpy(&Account[idx].Account.Mob[slot].Player.SkillBar1[0], p->SkillBar, 4);
		memcpy(&Account[idx].Account.Mob[slot].SkillBar[0], &p->SkillBar[4], 16);

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

		Account[idx].Account.Daily.WeekYear  = p->Daily.WeekYear;
		memcpy(&Account[idx].Account.Daily.Received[0], p->Daily.Received, sizeof(p->Daily.Received));
		
		Account[idx].Account.Water.Day       = p->Water.Day;
		Account[idx].Account.Water.Total     = p->Water.Total;
		Account[idx].Account.SingleGift = p->SingleGift;
		
		DBWriteAccount(&Account[idx].Account);

		RemoveAccountList(idx);
		SendDBSignal(conn, Header->ClientId, 0x40B);

		for (int i = 0; i < MAX_ADMIN; i++)
		{
			if (pAdmin[i].Level <= 2 || pAdmin[i].DisableID == 0)
				continue;

			if (pAdmin[i].DisableID == idx)
			{
				DisableAccount(i, acc, pAdmin[i].Year, pAdmin[i].YearDay);

				break;
			}
		}
	}
	break;
	case _MSG_DBSaveMob:
	{
		p807 *p = (p807*)(Header);
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
		
		if(p->Arg2)
		{
			_MSG_FRIENDLIST_UPDATESTATUS f;
			memset(&f, 0, sizeof _MSG_FRIENDLIST_UPDATESTATUS);

			f.Header.ClientId = Header->ClientId;
			f.Header.PacketId = MSG_FRIENDLIST_UPDATESTATUS_OPCODE;
			f.Header.Size = sizeof _MSG_FRIENDLIST_UPDATESTATUS;
		
			strncpy_s(f.Name, p->Mob.Player.Name, 16);

			f.Status = 2;
			f.Server = 0;

			for(INT32 h = 0; h < MAX_SERVERGROUP;h ++)
			{
				if (pUser[h].Mode == 0 || pUser[h].Sock.Sock == 0)
					continue;

				pUser[h].Sock.SendOneMessage((char*)&f, sizeof _MSG_FRIENDLIST_UPDATESTATUS);
			}
		}

		Account[index].Account.Mob[slot] = p->Mob;

		memcpy(&Account[index].Account.Mob[slot].Player.SkillBar1[0], p->SkillBar, 4);
		memcpy(&Account[index].Account.Mob[slot].SkillBar[0], &p->SkillBar[4], 16);

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

		Account[index].Account.Daily.WeekYear  = p->Daily.WeekYear;

		Account[index].Account.Water.Day       = p->Water.Day;
		Account[index].Account.Water.Total     = p->Water.Total;
		Account[index].Account.SingleGift = p->SingleGift;

		memcpy(&Account[index].Account.Daily.Received[0], p->Daily.Received, sizeof(p->Daily.Received));

		DBWriteAccount(&Account[index].Account);

		if (p->Arg2 != 0)
			DBExportAccount(&Account[index].Account);
	}
	break;
	case _MSG_DBNewArch:
	{
		MSG_DBNewArch *p = (MSG_DBNewArch*)(Header);

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

		st_Mob *mob = &Account[index].Account.Mob[slotId].Player;//LOCAL_2844
		stCharInfo *extra = &Account[index].Account.Mob[slotId];

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

		for (int i = 1; i < 15; i++)
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

		INT32 ret = DBWriteAccount(&Account[index].Account);//LOCAL_2847
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

		DBGetSelChar(&packet.CharList, &Account[index].Account);

		packet.Header.ClientId = Header->ClientId;

		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
	}
	break;
	case _MSG_DBNewCharacter:
	{
		p20F *p = (p20F*)(Header);

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

		st_Mob *mob = &Account[idx].Account.Mob[slot].Player;//LOCAL_2844
		stCharInfo *extra = &Account[idx].Account.Mob[slot];

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
		mob->Equip[0].EF2 =  mob->Equip[0].Index & 127;

		memcpy(mob->Name, p->Nick, 16);
		
		ret = DBWriteAccount(&Account[idx].Account);
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

		DBGetSelChar(&packet.CharList, &Account[idx].Account);

		packet.Header.ClientId = Header->ClientId;

		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
	}
	break;
	case _MSG_DBAccountLogin:
	{
		p20D *p = (p20D*)Header; // 1050
		_strupr_s(p->Login);

		const char *login = p->Login;
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

		stAccount account{};//LOCAL_2126
		memcpy(account.Username, p->Login, 16);

		INT32 ret = DBReadAccount(&account);//LOCAL_2127
		if (ret == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x421);

			return true;
		}

		if (account.Storage.Coin < 0)
			account.Storage.Coin = 0;

		if (pUser[conn].Staff == 1 && account.AccessLevel == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x426);

			return true;
		}

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
		if(account.TempKey[0] != 0 && p->Unknow[0] != 0)
		{
			if (strncmp(account.TempKey, p->Unknow, strlen(account.TempKey)) == 0)
			{
				memset(account.TempKey, 0, 52);
				changeServer = 1;
			}
			else
			{
				memset(account.TempKey, 0, 52);
				DBWriteAccount(&account);

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
			if (account.Mob[q].Player.Equip[13].Index == 774)
				left = q;
			if (account.Mob[q].Player.Equip[13].Index == 775)
				right = q;
		}

		if (left != -1 && right != -1)
		{
			char temp[16];//LOCAL_2134l
			memcpy(temp, account.Mob[left].Player.Name, 16);
			memcpy(account.Mob[left].Player.Name, account.Mob[right].Player.Name, 16);
			memcpy(account.Mob[right].Player.Name, temp, 16);

			memset(&account.Mob[left].Player.Equip[13], 0, sizeof st_Item);
			memset(&account.Mob[right].Player.Equip[13], 0, sizeof st_Item);
			DBWriteAccount(&account);
		}

		stAccount *pAccount = &Account[idx].Account;//LOCAL_2199
		memcpy(pAccount, &account, sizeof stAccount);

		AddAccountList(idx);

		Account[idx].IsBanned = master == 0;

		p416 selChar{};//LOCAL_2385
		memset(&selChar, 0, sizeof p416);

		DBGetSelChar(&selChar.CharList, &account);

		selChar.Header.PacketId = _MSG_DBCNFAccountLogin;
		selChar.Header.ClientId = Header->ClientId;

		strncpy_s(selChar.UserName, account.Username, 16);
		memcpy(&selChar.Storage[0], &account.Storage.Item[0], sizeof st_Item * 128);
		selChar.GoldStorage = pAccount->Storage.Coin;

		pUser[conn].Sock.SendOneMessage((char*)&selChar, sizeof selChar);

		SendAdditionalAccountInfo(conn, Header->ClientId);

		if(changeServer)
		{
			DBWriteAccount(&account);

			INT32 slot	  = 0;
			INT32 server  = 0;
			INT32 enc[8] = { 0 };
			
			sscanf_s(p->Unknow, "%d %d %d %d %d %d %d %d %d %d", &server, &enc[0], &enc[1], &enc[2], &enc[3], &enc[4], &enc[5], &enc[6], &enc[7], &slot);

			Account[idx].Slot = slot;

			p114 packet{};
			packet.Header.PacketId = _MSG_DBCNFCharacterLogin;
			packet.Header.ClientId = Header->ClientId;
			packet.Header.Size = sizeof p114;

			packet.SlotIndex = slot;
			packet.Mob = Account[idx].Account.Mob[slot].Player;

			memcpy(&packet.SkillBar2, &Account[idx].Account.Mob[slot].SkillBar[0], 16);

			if (!Account[idx].Account.Mob[slot].Player.Name[0])
			{
				//Log( err, charlogin mobname empty
				return true;
			}

			p820 packetExtra;
			memset(&packetExtra, 0, sizeof p820);
			packetExtra.Header.ClientId = p->Header.ClientId;
			packetExtra.Header.PacketId = _MSG_DBCNFCharacterLoginExtra;
			packetExtra.Header.Size = sizeof p820;
		
			stCharInfo *mob = &Account[idx].Account.Mob[slot];
			memcpy(&packetExtra.Mob, mob, sizeof stCharInfo);
			packetExtra.Cash = Account[idx].Account.Cash;

			strncpy_s(packetExtra.Pass, Account[idx].Account.Block.Pass, 16);
			packetExtra.Blocked = Account[idx].Account.Block.Blocked;

			memcpy(packet.Affect, Account[idx].Account.Mob[slot].Affects, sizeof st_Affect * 32);

			pUser[conn].Sock.SendOneMessage((char*)&packetExtra, sizeof p820);
			pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p114);

			if(account.AccessLevel == 0)
			{
				_MSG_FRIENDLIST_UPDATESTATUS f;
				memset(&f, 0, sizeof _MSG_FRIENDLIST_UPDATESTATUS);

				f.Header.ClientId = Header->ClientId;
				f.Header.PacketId = MSG_FRIENDLIST_UPDATESTATUS_OPCODE;
				f.Header.Size = sizeof _MSG_FRIENDLIST_UPDATESTATUS;

				strncpy_s(f.Name, packet.Mob.Name, 16);
				f.Status = 1;
				f.Server = conn + 1;

				for(INT32 h = 0; h < MAX_SERVERGROUP;h ++)
				{
					if (pUser[h].Mode == 0 || pUser[h].Sock.Sock == 0)
						continue;

					pUser[h].Sock.SendOneMessage((char*)&f, sizeof _MSG_FRIENDLIST_UPDATESTATUS);
				}
			}
		}
	}
	break;
	case _MSG_DBDeleteCharacter:
	{
		p211 *p = (p211*)(Header);

		INT32 index = GetIndex(conn, p->Header.ClientId),
			slot = p->SlotIndex;//LOCAL_3602

		if (slot < 0 || slot >= 4)
		{
			SendDBSignal(conn, Header->ClientId, 0x41E);
			// Log(err, deletechar slot
			return true;
		}
		st_Mob *mob = &Account[index].Account.Mob[slot].Player;//LOCAL_3667
		if (Account[index].Account.Mob[slot].Player.Equip[0].EFV2 >= 3)
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

		memset(&Account[index].Account.Mob[slot], 0, sizeof stCharInfo);

		// log( delete char %s
		ClearMob(mob);

		DBWriteAccount(&Account[index].Account);

		p418 packet;
		packet.Header.PacketId = _MSG_DBCNFDeleteCharacter;

		DBGetSelChar(&packet.CharList, &Account[index].Account);

		packet.Header.ClientId = Header->ClientId;

		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
	}
	break;
	case 0x814: // change server
	{
		MSG_DBServerChange *m = (MSG_DBServerChange*)msg;

		INT32 clientId	= m->Header.ClientId; 
		INT32 newServer = m->NewServerID;
		INT32 slot		= m->Slot;

		if(clientId <= 0 || clientId >= MAX_PLAYER)
			break;

		if(newServer <= 0 || newServer >= MAX_SERVER + 1)
			break;
		
		INT32 idx	= GetIndex(conn, clientId);
		INT32 enc[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		INT32 encRes = GetEncPassword(idx, enc);

		if(encRes)
		{ // fodasse, sempre retorna false q
		}
		else
		{
			MSG_DBCNFServerChange packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0x52A;
			packet.Header.Size	   = sizeof MSG_DBCNFServerChange;
			packet.Header.ClientId = clientId;

			strncpy_s(packet.AccountName, Account[idx].Account.Username, 16);
			
			sprintf_s(packet.Enc, "*%d %d %d %d %d %d %d %d %d %d", m->NewServerID,
				enc[0], enc[1], enc[2], enc[3], enc[4], enc[5], enc[6], enc[7], m->Slot);

			memset(Account[idx].Account.TempKey, 0, 52);
			memcpy(Account[idx].Account.TempKey, packet.Enc, strlen(packet.Enc));

			pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof MSG_DBCNFServerChange);
		}
	}
	break;
	case _MSG_GuildZoneReport:
	{
		MSG_GuildZoneReport *p = (MSG_GuildZoneReport*)msg;

		INT32 srv = p->Header.ClientId;//LOCAL_379
		if (srv < 0 || srv >= MAX_SERVERGROUP)
			return true;

		for (INT32 i = 0; i < 5; i++)
			ChargedGuildList[srv][i] = p->Guild[i];

		MSG_ChargedGuildList packet;
		packet.Header.PacketId = _MSG_DBCNFChargedList;
		packet.Header.Size = sizeof MSG_ChargedGuildList;
		packet.Header.ClientId = 0;

		memcpy(packet.ChargedGuildList, ChargedGuildList, sizeof ChargedGuildList);

		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			pUser[i].Sock.SendOneMessage((char*)&packet, 0x708);
		}
	}
	break;
	case _MSG_DBGuildAlly:
	{
		pE12 *p = (pE12*)(Header);

		INT32 gId1 = p->GuildIndex1,//LOCAL_522
			gId2 = p->GuildIndex2;//LOCAL_523

		if (gId1 <= 0 || gId1 >= 65536)
			break;

		g_pGuildAlly[gId1] = gId2;

		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			SendDBSignalParm2(i, 0, 0xE12, p->GuildIndex1, p->GuildIndex2);
		}
	}
	break;
	case MSG_ADDGUILD_OPCODE:
	{
		MSG_ADDGUILD *p = (MSG_ADDGUILD*)(Header);

		if(p->guildIndex <= 0 || p->guildIndex >= MAX_GUILD || p->Type < 0 || p->Type > 2)
			return true;

		switch(p->Type)
		{
			case 0: // set fame guild
				g_pGuild[p->guildIndex].Fame = p->Value;
				break;
			case 1:
				g_pGuild[p->guildIndex].Citizen = p->Value;
				break;
			case 2:
				g_pGuild[p->guildIndex].Wins = p->Value;
				break;
		}
		
		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;
			
			pUser[i].Sock.SendOneMessage((char*)Header, sizeof MSG_ADDGUILD);
		}
	}
	break;
	case MSG_ADDSUB_OPCODE:
	{
		MSG_ADDSUB *p = (MSG_ADDSUB*)(Header);
	
		if(p->GuildIndex > MAX_GUILD)
			return true;

		if(p->SubIndex < 0 || p->SubIndex >= 3)
			return true;

		if(p->Status == 1)
			g_pGuild[p->GuildIndex].SubGuild[p->SubIndex].clear();
		else
			g_pGuild[p->GuildIndex].SubGuild[p->SubIndex] = std::string(p->Name);
		
		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;
			
			pUser[i].Sock.SendOneMessage((char*)Header, sizeof MSG_ADDSUB);
		}
		return true;
	}
	case MSG_CREATEGUILD_OPCODE:
	{
		MSG_CREATEGUILD *p = (MSG_CREATEGUILD*)(Header);

		INT32 guildId = p->guildId;
		if(guildId <= 0 || guildId >= MAX_GUILD)
			break;

		if(g_pGuild[guildId].Name[0])
		{
			MessageBoxA(NULL, "error : guild slot is not empty", "error", MB_OK | MB_OKCANCEL);

			break;
		}

		g_pGuild[guildId].Name = std::string(p->GuildName);

		g_pGuild[guildId].Citizen = p->citizen;
		g_pGuild[guildId].Fame = 0;
		g_pGuild[guildId].Kingdom = p->kingDom;

		for (INT32 i = 0; i < 3; i++)
			g_pGuild[guildId].SubGuild[i].clear();

		WriteGuilds();
		
		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;
			
			pUser[i].Sock.SendOneMessage((char*)Header, sizeof MSG_CREATEGUILD);
		}
	}
	break;
	case _MSG_DBGuildWar:
	{
		pE12 *p = (pE12*)(Header);

		time_t nowraw = time(NULL);
		struct tm when;
		localtime_s(&when, &nowraw);//LOCAL_521

		INT32 gId1 = p->GuildIndex1, //LOCAL_522
			gId2 = p->GuildIndex2;//LOCAL_523

		if (gId1 <= 0 || gId1 >= 65536)
			break;

		g_pGuildWar[gId1] = gId2;

		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			SendDBSignalParm2(i, 0, 0xE0E, p->GuildIndex1, p->GuildIndex2);
		}
	}
	break;
	case _MSG_DBReloadDonateList:
	{
		ReadNPCDonate();
		
		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			pUser[i].Sock.SendOneMessage((char*)Header, Header->Size);
		}
	}
	break;

	case _MSG_DBBroadcastSetAvaible:
	{
		pMsgSignal3 *p = (pMsgSignal3*)(Header);

		g_pStore[p->Value][p->Value2].Avaible = p->Value3;
		
		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			pUser[i].Sock.SendOneMessage((char*)Header, Header->Size);
		}

		WriteNPCDonate();
	}
	break;

	case _MSG_DBBuyStoreSaveList:
	{
		pMsgSignal2 *p = (pMsgSignal2*)(Header);

		if(g_pStore[p->Value][p->Value2].Avaible != -1)
			g_pStore[p->Value][p->Value2].Avaible --;
		
		p->Header.PacketId = _MSG_DBBroadcastStoreBuy;

		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			pUser[i].Sock.SendOneMessage((char*)Header, Header->Size);
		}

		WriteNPCDonate();
	}
	break;
	case _MSG_DBBroadcastChannelChat:
	{
		for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
		{
			if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
				continue;

			pUser[i].Sock.SendOneMessage((char*)Header, Header->Size);
		}
	}
	break;
	case 0xFAA:
	{
	}
	break;
	case _MSG_DBRequestNumericPassword:
	{
		pFDE *p = (pFDE*)Header;

		pFDE fde;
		memset(&fde, 0, sizeof pFDE);

		fde.Header.ClientId = p->Header.ClientId;
		fde.Header.PacketId = _MSG_DBCNFRequestNumericPass;
		fde.Header.Size = 32;

		INT32 index = GetIndex(conn, p->Header.ClientId);

		if (Account[index].IsBanned)
		{
			pUser[conn].Sock.SendOneMessage((char*)& fde, sizeof pFDE);
			return true;
		}

		if (!Account[index].Account.SecondPass[0])
		{
			// Seta a senha numérica
			strncpy_s(Account[index].Account.SecondPass, p->num, 16);
			pUser[conn].Sock.SendOneMessage((char*)&fde, sizeof pFDE);
			DBWriteAccount(&Account[index].Account);
		}
		else
		{
			if (p->RequestChange == 1)
			{
				// Copia a nova senha numérica para o usuário
				strncpy_s(Account[index].Account.SecondPass, p->num, 16);

				// Envia o pacote para o usuário, informando que está correta
				pUser[conn].Sock.SendOneMessage((char*)&fde, sizeof pFDE);
				DBWriteAccount(&Account[index].Account);
			}
			else if (!strncmp(p->num, Account[index].Account.SecondPass, 6))
				pUser[conn].Sock.SendOneMessage((char*)&fde, sizeof pFDE);
			else
				SendDBSignal(conn, p->Header.ClientId, 0xFDF);
		}
	}
	break;
	/*
	case MSG_UPDATEWARDECLARATION://0x998
	{
		// Update de estado da guerra.
        // Algum líder declarou ou recusou guerra
		_MSG_UPDATEWARDECLARATION *p = (_MSG_UPDATEWARDECLARATION*)Header;

		BYTE newInfo = p->newInfo;

		// Quando declara guerra: newInfo = 1
		// Quando recusa guerra:  newInfo = 0
		// Pega o canal par
		BYTE otherConn = ((conn % 2) ? (conn - 1) : (conn + 1));

		if (g_pTowerWarState[otherConn].WarState != newInfo && g_pTowerWarState[otherConn].WarState != 2) 
		{
			// O if irá entrar ao declarar pois normalmente a guerra não está declarada
			// Ou seja, WarState = 0 e newInfo = 1
			_MSG_UPDATEWARANSWER answer;
			memset(&answer, 0, sizeof _MSG_UPDATEWARANSWER);

			answer.Header.PacketId = MSG_UPDATEWARDECLARATION;
			answer.Header.Size = sizeof _MSG_UPDATEWARANSWER;

			// Atribui quando está declarando ou recusando uma guerra
			g_pTowerWarState[otherConn].WarState = newInfo;

			// Declarando ou recusando
			answer.action = newInfo;

			// O canal que está a declarar a guerra
			answer.declarant = conn;

			// O canal que é o par, ou seja, quem recebe a declaração
			answer.receiver  = otherConn;

			for (int i = 0; i < MAX_SERVERGROUP; i++)
			{
				if (pUser[i].Mode == USER_EMPTY)
					continue;

				if (pUser[i].Sock.Sock == INVALID_SOCKET)
					continue;

				// Envia o pacote para todos os canais disponíveis
				answer.Header.ClientId = i;
				pUser[i].Sock.SendOneMessage((char*)&answer, sizeof _MSG_UPDATEWARANSWER);
			}
		}
	}
	break;
	case MSG_REWARDWARTOWER_OPCODE: //0x996
	{
		_MSG_REWARDWARTOWER *p = (_MSG_REWARDWARTOWER*)(Header);	
		if(p->Server < 0 || p->Server >= MAX_SERVERGROUP)
			break;

		if(pUser[p->Server].Mode == USER_EMPTY || pUser[p->Server].Sock.Sock == INVALID_SOCKET)
			break;

		pUser[p->Server].Sock.SendOneMessage((char*)(p), sizeof _MSG_REWARDWARTOWER);
	}
	break;
	case MSG_UPDATETOWERINFO://0x999
	{
		// Este pacote informa que a torre de um canal caiu
		// Avisar aos canais que a guerra mudou
		_MSG_UPDATETOWERINFO *p = (_MSG_UPDATETOWERINFO*)Header;

		if (p->KillerId < 0 || p->KillerId > MAX_GUILD)
			p->KillerId = 0;

		int channel = conn;

		// A guild que matou a torre ganha 50 de fame
		if (p->KillerId)
			SetFame(p->KillerId, g_pGuild[p->KillerId].Fame + 50);

		// Para cada torre que cai em seu canal que não tenha sido ela a matar
		// A coroa paga 50 de fame
		// Desde que não seja o seu avanço
		INT32 leader = ChargedGuildList[conn][4];
		if (leader != p->KillerId && g_pTowerWarState[conn].TowerState != conn)
		{
			if (g_pGuild[leader].Fame >= 100)
				SetFame(leader, g_pGuild[leader].Fame - 125);
			else
				SetFame(leader, 0);
		}

		// Pega o canal correspondente
		// A guerra sempre acontece em pares
		// Canal 0->1
		// Canal 2->3
		// Canal 4->5
		// Canal 6->7
		// Canal 8->9

		// Exemplo
		// Canal 1 derrota canal 1 irá para o else
		int otherConn = ((conn % 2) ? (conn - 1) : (conn + 1)); 
		if (g_pTowerWarState[conn].TowerState != conn)
		{
			// Um canal conseguiu derrubar a torre em outro canal
			// O outro canal deve primeiro derrubar a torre em seu canal para
			// Depois voltar ao ataque 
			//g_pTowerWarState[otherConn].TowerState = MAX_SERVERGROUP;6
			g_pTowerWarState[conn].TMP = g_pTowerWarState[conn].WarState;
			g_pTowerWarState[otherConn].WarState   = 2;

			// Informa que quem deve atacar a torre é o pessoal do canal
			g_pTowerWarState[conn].TowerState = conn;
			
			// Envia a mensagem para todos os canais se alguém avançou
			for (int i = 0; i < MAX_SERVERGROUP; i++)
			{
				if (pUser[i].Mode == USER_EMPTY)
					continue;

				if (pUser[i].Sock.Sock == INVALID_SOCKET)
					continue;

				SendNotice(i, "O canal %d declarou guerra ao canal %d.", conn + 1, 0);
			}
		}
		else
		{
			// Matou a torre em seu próprio canal
			// Caso queira, pode declarar a guerra e avançar no outro canal
			// Desde que sua torre não morra antes disso

			// Informa que quem está avançando neste canal é o outro
			g_pTowerWarState[conn].TowerState = otherConn;

			// Libera para possível avanço no outro canal
			g_pTowerWarState[otherConn].TowerState = conn;

			// otherConn = 1
			// conn      = 0
			
			// Seta como é possível declarar guerra novamente ao canal contrário
			g_pTowerWarState[conn].WarState        = g_pTowerWarState[conn].TMP;
			g_pTowerWarState[otherConn].WarState   = 0;

			// Envia a mensagem para todos os canais se alguém avançou
			for (int i = 0; i < MAX_SERVERGROUP; i++)
			{
				if (pUser[i].Mode == USER_EMPTY)
					continue;

				if (pUser[i].Sock.Sock == INVALID_SOCKET)
					continue;

				SendNotice(i, "O canal %d avança em seu território.", conn + 1);
			}

			// Para avançar no outro canal, basta declarar a guerra
		}

		// Enviar aos canais envolvidos suas novas informações
		// Enviar a notícia de quem avançou onde
		SendTowerWarInfo(2);
		break;
	}
	*/
	case MSG_SEND_SERVER_NOTICE:
	{
		_MSG_SEND_SERVER_NOTICE *p = (_MSG_SEND_SERVER_NOTICE*)Header;

		if (strlen(p->Notice) > 0)
		{
			for (int i = 0; i < MAX_SERVERGROUP; i++)
			{
				if (pUser[i].Mode == USER_EMPTY)
					continue;

				if (pUser[i].Sock.Sock == INVALID_SOCKET)
					continue;

				_MSG_SEND_SERVER_NOTICE answer;
				memset(&answer, 0, sizeof _MSG_SEND_SERVER_NOTICE);

				answer.Header.PacketId = MSG_SEND_SERVER_NOTICE;
				answer.Header.Size = sizeof _MSG_SEND_SERVER_NOTICE;

				strncpy_s(answer.Notice, p->Notice, 96);
				answer.Header.ClientId = i;

				pUser[i].Sock.SendOneMessage((char*)&answer, answer.Header.Size);
			}
		}

		break;
	}

	case MSG_NOTIFY_KEFRA_DEATH:
	{
		_MSG_NOTIFY_KEFRA_DEATH *p = (_MSG_NOTIFY_KEFRA_DEATH*)Header;
		
		if (sServer.FirstKefra == MAX_SERVERGROUP)
		{
			sServer.FirstKefra = conn;

			if (strlen(p->Name) > 0)
				strncpy_s(sServer.KefraKiller, p->Name, 16);
			
			_MSG_FIRST_KEFRA_NOTIFY packet;
			memset(&packet, 0, sizeof _MSG_FIRST_KEFRA_NOTIFY);

			packet.Header.PacketId = MSG_FIRST_KEFRA_NOTIFY;
			packet.Header.Size = sizeof _MSG_FIRST_KEFRA_NOTIFY;

			for (int i = 0; i < MAX_SERVERGROUP; i++)
			{
				// Notifica os outros canais de que o Kefra morreu primeiro em outro canal

				if (pUser[i].Mode == USER_EMPTY)
					continue;

				if (pUser[i].Sock.Sock == INVALID_SOCKET)
					continue;

				packet.Channel = conn;

				pUser[i].Sock.SendOneMessage((char*)&packet, packet.Header.Size);
			}
		}

		break;
	}

	case 0x899:
		{
			for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
			{
				if (pUser[i].Mode == 0 || pUser[i].Sock.Sock == 0)
					continue;

				pUser[i].Sock.SendOneMessage((char*)Header, Header->Size);
			}
		}
		break;

	case MSG_PANELGUILD_GETLIST:
		{
			_MSG_PANELGUILD_GETLIST packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = MSG_PANELGUILD_GETLIST;
			packet.Header.Size     = sizeof _MSG_PANELGUILD_GETLIST;
			packet.Header.ClientId = Header->ClientId;

			memcpy(&packet.Guild, &g_pGuild, sizeof stGuild * 40);

			pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof packet);
		}
		break;
	case SealInfoPacket:
	{
		pMsgSignal* p = reinterpret_cast<pMsgSignal*>(Header);

		INT32 index = GetIndex(conn, p->Header.ClientId);

		MSG_SEALINFO packet{};
		packet.Header.PacketId = SealInfoPacket;
		packet.Header.Size = sizeof packet;
		packet.Header.ClientId = p->Header.ClientId;

		SealFileInfo fileInfo;
		if (!ReadSealInfo(p->Value, fileInfo))
			packet.Info.Status = -1;
		else
		{
			const auto mob = fileInfo.Mob.Info;

			if (mob.Noas)
				packet.Info.QuestInfo = 1111;
			else if (mob.Thelion)
				packet.Info.QuestInfo = 111;
			else if (mob.Sylphed)
				packet.Info.QuestInfo = 11;
			else if (mob.Noas)
				packet.Info.QuestInfo = 1;
			
			packet.Info = fileInfo.Seal;
		}
		pUser[conn].Sock.SendOneMessage(reinterpret_cast<char*>(&packet), sizeof packet);
	}
	break;
	case OutSealPacket:
	{
		int index = GetIndex(conn, Header->ClientId);
		int slot = Account[index].Slot;

		if (slot < 0 || slot >= 4)
			return false;

		int newSlot = -1;
		for (int i = 0; i < 4; i++)
		{
			if (!Account[index].Account.Mob[i].Player.Name[0])
			{
				newSlot = i;
				break;
			}
		}

		if (newSlot == -1)
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			return true;
		}

		MSG_PUTOUTSEAL* p = reinterpret_cast<MSG_PUTOUTSEAL*>(Header);
		INT32 len = strlen(p->MobName);//LOCAL_2845
		for (INT32 i = 0; i < len; i++)
		{
			if (p->MobName[i] == 0x2D)
				continue;

			if (!isalnum(p->MobName[i]))
			{
				SendDBSignal(conn, Header->ClientId, 0x41D);

				return true;
			}
		}

		st_Item& item = Account[index].Account.Mob[slot].Player.Inventory[p->SrcSlot];
		int sealIndex = item.Effect[0].Value * 256 + item.Effect[1].Value;

		SealFileInfo fileInfo{};
		if(!ReadSealInfo(sealIndex, fileInfo))
			return false;

		// 00412710
		INT32 ret = CreateCharacter(Account[index].Account.Username, p->MobName);//LOCAL_2847
		if (ret == 0)
		{
			SendDBSignal(conn, Header->ClientId, 0x41D);

			return true;
		}

		{
			auto mob = &Account[index].Account.Mob[newSlot];
			ClearMob(&mob->Player);
			*mob = fileInfo.Mob;

			mob->Player.Gold = 0;
			mob->Player.GuildIndex = 0;
			mob->Player.GuildMemberType = 0;
			mob->LastGuildKickOut = stDate{};
			mob->Revigorante = stDate{};
			mob->Divina = stDate{};
			mob->Sephira = stDate{};
			mob->RvRPoints = 0;
			memset(&item, 0, sizeof st_Item);

			strncpy_s(mob->Player.Name, p->MobName, 16);
		}

		ret = DBWriteAccount(&Account[index].Account);
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

		DBGetSelChar(&packet.CharList, &Account[index].Account);

		packet.Header.ClientId = Header->ClientId;

		pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
	}
	break;
	case PutInSealPacket:

		int index = GetIndex(conn, Header->ClientId);
		int slot = Account[index].Slot;

		sServer.LastSeal++;

		const st_Mob *mob = &Account[index].Account.Mob[slot].Player;//LOCAL_2844
		const stCharInfo *extra = &Account[index].Account.Mob[slot];

		SealFileInfo info{};
		info.Seal.CON = mob->bStatus.CON;
		info.Seal.STR = mob->bStatus.STR;
		info.Seal.INT = mob->bStatus.INT;
		info.Seal.DEX = mob->bStatus.DEX;

		info.Seal.Evolution = mob->Equip[0].EFV2;
		info.Seal.Face = mob->Equip[0].EF2;
		info.Seal.Level = static_cast<short>(mob->bStatus.Level);

		int baseSkill = mob->ClassInfo * 24;
		int octaveSkill = -1;
		for (int i = 1; i < 4; i++)
		{
			bool have = mob->Learn[0] & (1 << ((i * 8) - 1));

			if (have)
				octaveSkill = baseSkill + ((i * 8) - 1);
		}

		info.Seal.Skills[0] = octaveSkill;

		for (int i = 1; i < 9; i++)
		{
			int skillId = 95 + i;
			
			if ((mob->Learn[0] & (1 << (23 + i))))
				info.Seal.Skills[i] = skillId;
			else
				info.Seal.Skills[i] = -1;
		}

		info.Mob = *extra;

		if(mob->Equip[0].EFV2 >= 3)
			info.Mob.MortalSlot = -1;

		info.Mob.Escritura = stDate{};
		info.Mob.PesaEnter = 0;
		info.Mob.DailyQuest.BattlePass = stDate{};

		// Tira as bolsas
		info.Mob.Player.Inventory[60] = st_Item{};
		info.Mob.Player.Inventory[61] = st_Item{};
		info.Mob.Player.GuildIndex = 0;
		info.Mob.Player.GuildMemberType = 0;
		info.Mob.Contribuition.Points = 0;
		info.Mob.Fame = 0;
		info.Mob.HallEnter = 0;
		info.Mob.PesaEnter = 0;
		info.Mob.Revigorante = stDate{};
		info.Mob.Saude = stDate{};

		WriteSealInfo(sServer.LastSeal, info);

		bool freeNickname = true;
		for (int i = 0; i < 4; i++)
		{
			if (i == slot)
				continue;

			if (strcmp(Account[index].Account.Mob[i].Player.Name, mob->Name) == 0)
				freeNickname = false;
		}

		if (freeNickname)
			DeleteCharacter(mob->Name, Account[index].Account.Username);

		memset(&Account[index].Account.Mob[slot], 0, sizeof stCharInfo);

		ClearMob(&Account[index].Account.Mob[slot].Player);

		DBWriteAccount(&Account[index].Account);
		{
			p418 packet;
			packet.Header.PacketId = _MSG_DBCNFDeleteCharacter;

			DBGetSelChar(&packet.CharList, &Account[index].Account);

			packet.Header.ClientId = Header->ClientId;

			pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof p418);
		}

		{
			pMsgSignal packet{};
			packet.Header.PacketId = PutInSealSuccess;
			packet.Header.ClientId = Header->ClientId;

			packet.Value = slot;

			pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof pMsgSignal);
		}

		{
			pCOF packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xC0F;
			packet.Header.Size = sizeof pCOF;
			packet.Header.ClientId = index;

			strncpy_s(packet.Username, Account[index].Account.Username, 16);

			packet.item.Index = 3443;
			packet.item.Effect[0].Index = 59;
			packet.item.Effect[0].Value = sServer.LastSeal / 256;;
			packet.item.Effect[1].Index = 59;
			packet.item.Effect[1].Value = sServer.LastSeal % 256;

			pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof pCOF);
		}
		WriteConfig();
		break;
	}
	return true;
}

void CFileDB::SendAdditionalAccountInfo(INT32 conn, INT32 clientId)
{
	INT32 idx = GetIndex(conn, clientId); //LOCAL_1052

	p415 packet;
	memset(&packet, 0, sizeof packet);

	packet.Header.PacketId = 0x415;
	packet.Header.Size     = sizeof packet;
	packet.Header.ClientId = clientId;
	
	packet.Access = Account[idx].Account.AccessLevel;
	packet.Unique = Account[idx].Account.Unique.Value;

	packet.Water.Total		= Account[idx].Account.Water.Total;
	packet.Water.Day		= Account[idx].Account.Water.Day;

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

INT32 CFileDB::SendDBSignal(INT32 server, INT32 clientId, INT32 signal)
{
	PacketHeader header;
	header.PacketId = signal;
	header.Size = sizeof header;
	header.ClientId = clientId;

	pUser[server].Sock.SendOneMessage((char*)&header, sizeof PacketHeader);
	return true;
}

void CFileDB::SendDBSavingQuit(INT32 id, INT32 mode)
{
	MSG_DBSavingQuit packet;
	INT32 conn = id / MAX_PLAYER;//LOCAL_2
	INT32 idx = id % MAX_PLAYER;//LOCAL_3

	packet.Header.PacketId = _MSG_DBSavingQuit;
	packet.Header.ClientId = idx;
	packet.Header.Size = sizeof MSG_DBSavingQuit;

	packet.Mode = mode;

	memcpy(&packet.AccountName, &Account[id].Account.Username, 16);

	pUser[conn].Sock.SendOneMessage((char*)&packet, sizeof MSG_DBSavingQuit);
}

INT32 CFileDB::GetEncPassword(int idx, int *Enc)
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