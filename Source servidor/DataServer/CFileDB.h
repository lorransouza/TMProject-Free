#pragma once
#include <stBase.h>
#include <PacketStruct.h>

struct stAccountDB
{
	int Login{};
	int Slot{};

	stAccount Account{};

	stCharInfo Mob[4]{};
	
	int SecurePass{};

	bool IsBanned{};

	UINT8 Skillbar[4][8]{ {} };  // Possivel segunda barra de skill
};

class CFileDB
{
public:
	CFileDB();

public:
	stAccountDB Account[MAX_DBACCOUNT];

	INT32 GetIndex(char* accountName);
	INT32 GetIndex(INT32 server, INT32 id);

	INT32 SendDBMessage(INT32 server, INT32 id, const char* msg);
	INT32 SendDBSignal(INT32 server, INT32 clientId, INT32 signal);
	INT32 SendDBSignalParm2(INT32 server, INT32 clientId, INT32 packetId, INT32 parm1, INT32 parm2);
	INT32 SendDBSignalParm3(INT32 server, INT32 clientId, INT32 packetId, INT32 parm1, INT32 parm2, INT32 parm3);
	INT32 SendDBSignalParm1(INT32 server, INT32 clientId, INT32 packetId, INT32 parm1);
	void SendDBSavingQuit(INT32 id, INT32 mode);

	INT32 DBWriteAccount(stAccountDB* acc);
	INT32 DBReadAccount(stAccountDB* acc);
	INT32 GetEncPassword(int idx, int* Enc);

	INT32 CreateCharacter(const char* ac, char* ch);
	INT32 DeleteCharacter(const char* ch, char* ac);

	void AddAccountList(INT32 index);
	void RemoveAccountList(INT32 index);

	void GetAccountByChar(char* acc, char* cha);
	void DBGetSelChar(st_CharList* p, stAccountDB* file);

	void SendLog(int server, int clientId, const char* message);
	void SendAdditionalAccountInfo(INT32 conn, INT32 clientId);
	INT32 ProcessMessage(char* msg, INT32 conn);
};

extern CFileDB cFileDB;