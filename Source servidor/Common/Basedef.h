
#ifndef __BASEDEF_H__
#define __BASEDEF_H__

#include <Windows.h>
#include <fstream>
#include "CUser.h"
#include "stBase.h"
#include "Keytable.h"
#include "pugixml.hpp"

// Adiciona a library da winsock
#pragma comment (lib, "WS2_32.lib")

#define MAX_SERVER			2
#define MAX_SERVERGROUP		10
#define MAX_GUILD 65535
#define MAX_SERVERNUMBER	(MAX_SERVER + 3)
#define	MAX_ADMIN				10 
#define MAX_PLAYER			1000
#define MAX_DBACCOUNT		(MAX_PLAYER * MAX_SERVER)
#define		ACCOUNTNAME_LENGTH		16		// Max length of the account login
#define		ACCOUNTPASS_LENGTH		12      // Max length of the account password
#define TIMER_SEC 0

struct stGUI
{
	HWND hGUI,
	hButton[1],
	hLabel[1];

	HFONT Font;
	ATOM Class;
};

// Estrutura do banco de dados
struct stServer
{
	struct
	{
		UINT32 Porta;
		char IP[32];
	} Config;

	INT32 SecCounter;
	INT32 MinCounter;
	INT32 HourCounter;

	INT32 CurrentTime;
	INT32 LastSendTime;

	UINT32 hServer;
	SOCKADDR_IN Service;

	INT32 ServerIndex;

	bool Status;

	// Safiras do servidor
	UINT32 Sapphire;

	// Primeiro canal que matou Kefra
	UINT32 FirstKefra;
	char   KefraKiller[16];

	UINT32 Staff; //
	UINT32 LastGuild;

	INT32 LastSeal{ 1 };
	INT32 LastMerida{ 1 };
} ;

// Estrutura da guerra de torres
typedef struct
{
	INT16 TowerState; // Indica qual canal está avançando neste momento
	INT16 WarState;   // Indica se a guerra está declarada ou recusada
	INT16 TMP;        // Guarda o valor antigo
} stTowerWar;

static int dias_mes[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
extern INT32 ChargedGuildList[MAX_SERVERGROUP][5];
extern stServer sServer;
extern stGUI GUI;
extern st_Mob pBaseSet[4];
extern INT16 g_pGuildWar[MAX_GUILD];
extern stGuild g_pGuild[MAX_GUILD];
extern INT32 g_pGuildPoint [MAX_GUILD];
extern INT16 g_pGuildAlly[MAX_GUILD];
extern char	g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];
extern stTowerWar g_pTowerWarState[MAX_SERVERGROUP];
extern stDonateStore g_pStore[10][27];

INT32 InitializeBaseDef();

INT32 ReadNPCDonate();
INT32 WriteNPCDonate();

void Log(char *username, const char *msg, ...);

INT32 WriteConfig();
INT32 WriteGuilds();
INT32 ReadGuilds();
INT32 ReadConfig();

INT32 DecideWinnerTowerWar();

void SetFame(INT32 guildId, INT32 value);

BOOL SendTowerWarInfo(BYTE Info);
BOOL ReadCharBase(INT32 index, const char *fileName);
BOOL InitializeServerList();
int Rand();

void ClearItem(st_Item *item);
void ClearMob(st_Mob *player);
INT32 GetUserFromSocket(INT32 soc);

void GetFirstKey(const char *source, char *dest);


void AppendStructure(pugi::xml_node mob, stCharInfo* charInfo);
void AppendStructure(pugi::xml_node mob, st_Mob* mobInfo);
void AppendStructure(pugi::xml_node mob, st_Status* status);
void AppendStructure(pugi::xml_node node, const st_Affect* affect);
void AppendStructure(pugi::xml_node node, const stDate* date);
void AppendStructure(pugi::xml_node node, const st_Position* position);
void AppendStructure(pugi::xml_node node, const st_Item* item);
void AppendStructure(pugi::xml_node account, stAccount* acc);

void XMLToStructure(pugi::xml_node node, stCharInfo& charInfo);
void XMLToStructure(pugi::xml_node node, stSub& sub);
void XMLToStructure(pugi::xml_node node, stDate& date);
void XMLToStructure(pugi::xml_node node, st_Position& position);
void XMLToStructure(pugi::xml_node node, st_Status& status);
void XMLToStructure(pugi::xml_node node, st_Item& item);
void XMLToStructure(pugi::xml_node node, st_Affect& affect);
void XMLToStructure(pugi::xml_node accNode, stAccount* file);

#define FLAG_GAME2CLIENT 0x0100
#define FLAG_CLIENT2GAME 0x0200
#define FLAG_DB2GAME     0x0400
#define FLAG_GAME2DB	 0x0800

// ------------------------------
// Structs
// ------------------------------
typedef struct
{
	WORD Size;
	BYTE Key;
	BYTE CheckSum;
	WORD PacketId;
	WORD ClientId;
	DWORD TimeStamp;
} PacketHeader;

#define _MSG_MessagePanel                 ( 1 | FLAG_GAME2CLIENT)
#define _MSG_DBNewCharacter         ( 2 | FLAG_GAME2DB)
#define _MSG_DBAccountLogin			( 3 | FLAG_GAME2DB) // login
#define _MSG_DBCharacterLogin       ( 4 | FLAG_GAME2DB)
#define _MSG_DBCNFCharacterLogin	0x417
#define _MSG_DBCNFCharacterLoginExtra 0x820
#define _MSG_DBNoNeedSave           ( 5 | FLAG_GAME2DB) 
#define _MSG_DBUpdateSapphire		0x80E
#define _MSG_DBStaffMode			0x80F
#define _MSG_DBSaveMobQuit           ( 6 | FLAG_GAME2DB) 
#define _MSG_DBSaveMob              ( 7 | FLAG_GAME2DB)
#define	_MSG_DBDeleteCharacter		( 9 | FLAG_GAME2DB)
#define _MSG_STARTTOWERWAR          ( 15 | FLAG_DB2GAME)
#define _MSG_DBCNFNewCharacter			0x418
#define _MSG_DBCNFAccountLogin			0x416
#define _MSG_DBCNFDeleteCharacter		0x419
#define _MSG_DBReloadDonateList			0x80C
#define _MSG_DBBuyStoreSaveList			0x380
#define _MSG_DBBroadcastStoreBuy		0x381
#define _MSG_DBBroadcastSetAvaible		0x382
#define _MSG_DBBroadcastChannelChat		0xD1D
#define _MSG_DBRequestNumericPassword	0xFDE
#define _MSG_DBCNFRequestNumericPass    0xFDE
#define _MSG_DBRequestCreateSubCele     0x830
struct MSG_MessagePanel
{        
	PacketHeader Header;
	int nID;
	char String[92];  
};

#define _MSG_DBSavingQuit         ( 10 | FLAG_DB2GAME)
struct  MSG_DBSavingQuit 
{       
	PacketHeader Header; // 0 -11
	char AccountName[16]; // 12 - 27
	int  Mode; // 28 - 31
};

struct		 MSG_DBCNFServerChange
{
	PacketHeader Header;

	char AccountName[ACCOUNTNAME_LENGTH];
	char Enc[52];
};

struct		 MSG_DBServerChange
{
	PacketHeader Header;

	int NewServerID;
	int Slot;
};

#define _MSG_DBNewArch         ( 11 | FLAG_GAME2DB)
typedef struct
{
	PacketHeader Header;

	INT32 PosID;
	char Name[16];
	INT32 ClassID;
	INT32 ClassInfo;
	INT32 MortalSlot;
} MSG_DBNewArch;

#define _MSG_GuildZoneReport		( 13 | FLAG_GAME2DB)
#define _MSG_DBCNFChargedList       0x428
struct	MSG_GuildZoneReport
{
	PacketHeader Header;
	INT32 Guild[5];
};

struct MSG_ChargedGuildList
{
	PacketHeader Header;

	INT32 ChargedGuildList[MAX_SERVERGROUP][5];
};

struct  MSG_STANDARDPARM1
{  	    PacketHeader Header;
        int Parm1;
};

struct  MSG_STANDARDPARM2
{  	    PacketHeader Header;
        int Parm1;
		int Parm2;
};

struct  MSG_STANDARDPARM3
{  	    PacketHeader Header;
        int Parm1;
		int Parm2;
		int Parm3;
};


typedef struct {
	PacketHeader Header;
	char Password[12];
	char Login[16];

	const char Unknow[52];
	UINT32 CliVer;
	UINT32 Unknow_84;

	char Keys[16];
} p20D;

struct st_CharList
{
	INT16 PositionX[4];
	INT16 PositionY[4];

	char Name[4][16];

	st_Status Status[4];
	st_Item Equip[4][16];

	UINT16 GuildId[4];

	int Gold[4];
	INT64 Exp[4];
}; // 744

struct p213 {
	PacketHeader Header;
	int CharIndex;
	char Zero[18];
};

struct p416 
{
	PacketHeader Header; // 0 - 11 

	BYTE Keys[16];

	INT32 Unkw;

	st_CharList CharList; // 12 - 755
	
	st_Item Storage[128]; // 756 - 1779 
	UINT32 GoldStorage; // 1880 - 1883

	char UserName[16]; // 1884 - 1900 
	BYTE Unknow[12];
};

struct p418
{
	PacketHeader Header; // 0 - 11 
	st_CharList CharList; // 12 - 755
};

struct p20F
{
	PacketHeader Header;
	UINT32 SlotID;
	char Nick[16];
	UINT32 ClassID;
};

struct p114
{
	PacketHeader Header; // 0 - 11
	st_Position WorldPos; // 12 - 15
	st_Mob Mob; // 16 - 823

	char dummy[212];
	unsigned short CurrentKill;
	unsigned short TotalKill;

	short SlotIndex; // 1040 - 1041
	short ClientIndex; // 1042 - 1043
	short Weather; // 1044 - 1045

	char SkillBar2[16]; // 1046 - 1061

	int Unknow_1062[8];
	st_Affect Affect[32];

	char Unknown_1350[360];
} ; 

typedef struct
{
    PacketHeader Header;
    int SlotIndex;
    char Name[16];
    char Pwd[12];
} p211; 

typedef struct
{
	PacketHeader Header; // 0 - 11

	UINT32 CharSlot; // 12 - 15
	stCharInfo Mob; // 16 - 771
	st_Item Storage[128]; // 772 - 1795
	INT32 Coin; // 1796 - 1799
	char SkillBar[20]; // 1800 - 1807
	char User[16]; // 1808 - 1823
	UINT32 Arg2; // 1824 - 1827
	char Friends[30][16];
	char Pass[16];
	INT32 Blocked;
	INT32 Cash;
	INT64 Insignia;
	UINT32 BanType;
	stDate Ban;

	st_Position Position;
	INT16 Slot;
	INT64 Unique;
	
	struct
	{
		// O que ele jï¿½ recebeu referente ao dia
		bool Received[7];

		INT32 WeekYear;
	} Daily; 
	
	struct
	{
		INT32 Day;
		INT32 Total;
	} Water;

	stDate Divina;
	stDate Sephira;

	UINT64 SingleGift;
} p807;

typedef struct 
{
	PacketHeader Header;

	INT32 Val;
	char Username[16];
	st_Item item;
} pCOF;

typedef struct
{
	PacketHeader Header;

	INT32 Val;
	char Username[16];
	INT32 Cash;
} pCOE;

typedef struct
{
	PacketHeader Header;

	char Username[16];
	INT32 BanType;
	stDate Ban;
} pC10;

typedef struct {
	PacketHeader Header;

	INT32 Value;
} pMsgSignal;

typedef struct {
	PacketHeader Header;

	INT32 Value;
	INT32 Value2;
} pMsgSignal2;

typedef struct {
	PacketHeader Header;

	INT32 Value;
	INT32 Value2;
	INT32 Value3;
} pMsgSignal3;

#define _MSG_DBGuildAlly 0xE12
#define _MSG_DBGuildWar 0xE0E
typedef struct {
	PacketHeader Header;
	DWORD GuildIndex1;
	DWORD GuildIndex2;
} pE12;

typedef struct 
{
	PacketHeader Header;
	char num[16];

	int RequestChange;
} pFDE; 

typedef struct
{
	PacketHeader Header;

	stCharInfo Mob;
	INT32 Cash;
	char Pass[16];
	INT32 Blocked;
} p820;


typedef struct
{ 
	PacketHeader Header;

	INT32 Access;
	INT64 Unique;

	struct 
	{
		// O que ele já recebeu referente ao dia
		bool Received[7];

		INT32 WeekYear;
	} Daily;

	struct
	{
		INT32 Day;
		INT32 Total;
	} Water;

	stDate Divina;
	stDate Sephira;

	UINT64 SingleGift;

	stDate Ban;
	UINT32 BanType;

	INT32 IsBanned;
	INT32 Cash;
} p415;

typedef struct
{ 
	PacketHeader Header;

	UINT32 Level;
	UINT32 ClassInfo;
	UINT32 Learn;
	UINT32 Mantle;
	UINT32 Face;
	UINT32 CharPos;

	char Name[16];
} p830;

typedef struct
{
	PacketHeader Header;
	
	UINT32 ClassInfo;
	UINT32 Learn;
	UINT64 Exp;

	stDate Escritura;
	stQuestInfo Info;

	st_Item Item[2];
	st_Status Status;
}p432;

struct MSG_STARTTOWERWAR
{
	PacketHeader Header;

	BYTE isStarting;

	stTowerWar war[10];
};

typedef struct
{
	PacketHeader Header;
	char eMsg[96];
} p101;
// NPKO

const int FLAG_DB2NP = 0x1000;
const int FLAG_NP2DB = 0x2000;

const short  _MSG_NPReqIDPASS = (1 | FLAG_DB2NP);
const short  _MSG_NPIDPASS = (2 | FLAG_NP2DB);
struct		  MSG_NPIDPASS
{
	PacketHeader Header;
	char Account[16];
	int  Encode1;
	char Pass[12];
	int  Encode2;
};

const short  _MSG_NPReqAccount = (3 | FLAG_NP2DB);
struct		  MSG_NPReqAccount
{
	PacketHeader Header;
	char Account[16];
	char Char[16];
};

const short  _MSG_NPNotFound = (4 | FLAG_DB2NP); //   Signal

const short  _MSG_NPAccountInfo = (5 | FLAG_DB2NP);
struct		  MSG_NPAccountInfo
{
	PacketHeader Header;
	stAccount account;
	short Session;
	short State;  // 0:Normal  1:Blocked= (@);  2:Deleted= (_);  3:Disabled= (#);
};

const short  _MSG_NPReqSaveAccount = (6 | FLAG_NP2DB); //   MSG_NPAccountInfo

const short  _MSG_NPDisable = (7 | FLAG_NP2DB | FLAG_DB2NP);

const short  _MSG_NPEnable = (8 | FLAG_NP2DB | FLAG_DB2NP); //   NPEnableParm 
struct		  MSG_NPEnable
{
	PacketHeader Header;
	char AccountName[16];
	int Year;
	int YearDay;
};

const short  _MSG_NPNotice = (9 | FLAG_NP2DB | FLAG_DB2NP | FLAG_DB2GAME); //   Parm 
struct		  MSG_NPNotice
{
	PacketHeader Header;
	int  Parm1;
	int  Parm2;
	char AccountName[16];
	char String[96];
};

const short  _MSG_NPState = (10 | FLAG_NP2DB | FLAG_DB2NP); //   Parm 

const short _MSG_NPCreateCharacter = (11 | FLAG_NP2DB | FLAG_DB2NP);
struct		 MSG_NPCreateCharacter
{
	PacketHeader Header;
	int  Slot;
	char Account[16];
	st_Mob Mob;
};

const short _MSG_NPCreateCharacter_Reply = (12 | FLAG_DB2NP | FLAG_NP2DB);
struct		 MSG_NPCreateCharacter_Reply
{
	PacketHeader Header;
	int  Slot;
	char Account[16];
	int  Result;
	char Name[16];
};

const short  _MSG_NPDonate = (13 | FLAG_NP2DB | FLAG_DB2NP);
struct		  MSG_NPDonate
{
	PacketHeader Header;
	char AccountName[16];
	int Donate;
};

const short _MSG_NPAppeal = (16 | FLAG_DB2GAME | FLAG_GAME2DB | FLAG_DB2NP);

#define MSG_CREATEGUILD_OPCODE 0x310
struct MSG_CREATEGUILD
{ 
	PacketHeader Header;
	char GuildName[16];

	int kingDom;
	int citizen;
	int guildId;
};

#define MSG_ADDGUILD_OPCODE 0x313
struct MSG_ADDGUILD
{ 
	PacketHeader Header;
	int Type;
	int Value;
	int guildIndex;
};

#define MSG_ADDSUB_OPCODE 0x315
struct MSG_ADDSUB
{ 
	PacketHeader Header;

	INT16 GuildIndex;
	INT16 SubIndex;
	char Name[16];
	INT16 Status;
};

#define MSG_FRIENDLIST_REQUESTUPDATE_OPCODE 0x906

#define MSG_FRIENDLIST_UPDATESTATUS_OPCODE 0x907
typedef struct 
{
	PacketHeader Header;

	char Name[16];
	INT32 Status; // 1 = online - 0 = offline
	INT32 Server;
} _MSG_FRIENDLIST_UPDATESTATUS;

#define MSG_UPDATETOWERINFO 0x999
struct _MSG_UPDATETOWERINFO
{
	PacketHeader Header;

	int  KillerId; // GuildIndex que matou a torre
};

#define MSG_UPDATEWARDECLARATION 0x998
struct _MSG_UPDATEWARDECLARATION
{
	PacketHeader Header;

	BYTE newInfo;
};

struct _MSG_UPDATEWARANSWER
{
	PacketHeader Header;

	BYTE action,
		 declarant,
		 receiver;
};

#define MSG_RESULTWARTOWER_OPCODE 0x997
typedef struct
{
	PacketHeader Header;

	INT32 Winner;
} _MSG_RESULTWARTOWER;

#define MSG_REWARDWARTOWER_OPCODE 0x996
typedef struct
{
	PacketHeader Header;

	INT32 Server;
	INT64 Gold;
	INT32 Taxe;
} _MSG_REWARDWARTOWER;

#define MSG_FRIENDLIST_OPCODE 0x905
typedef struct
{
	PacketHeader Header;

	INT32 ClientID;

	char Name[30][16];
	INT8 Status[30];
	INT8 Server[30];
} _MSG_FRIENDLIST; 

#define MSG_SEND_SERVER_NOTICE 0x908
typedef struct
{
	PacketHeader Header;

	char Notice[96];
} _MSG_SEND_SERVER_NOTICE;

#define MSG_NOTIFY_KEFRA_DEATH 0x909
typedef struct
{
	PacketHeader Header;

	char Name[16];
} _MSG_NOTIFY_KEFRA_DEATH;

#define MSG_FIRST_KEFRA_NOTIFY 0x90A
#define MSG_REBORN_KEFRA 0x90B
typedef struct
{
	PacketHeader Header;

	int Channel;
} _MSG_FIRST_KEFRA_NOTIFY;

#define MSG_PANELGUILD_GETLIST 0x766
typedef struct 
{
	PacketHeader Header;

	stGuild Guild[40];
} _MSG_PANELGUILD_GETLIST;

constexpr auto OutSealPacket = 3132;
constexpr auto PutInSealPacket = 3133;
constexpr auto SealInfoPacket = 3134;
constexpr auto PutInSealSuccess = 3135;

struct MSG_PUTOUTSEAL
{
	PacketHeader Header;
	int      SrcType;
	int      SrcSlot;
	int      DstType;
	int      DstSlot;
	unsigned short GridX, GridY;
	unsigned short WarpID;

	char MobName[16];
};

// PAcket 0x2CD  - Size 16 vem o pedido de colocar o mouse no item
struct MSG_SEALINFO
{
	PacketHeader Header;
	SealInfo Info;
};

class TOD_Log;

constexpr int LogPacket = 0x8580;
#endif