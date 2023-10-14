#ifndef __STRUCT_H__
#define __STRUCT_H__

#include <Windows.h>
#include <chrono>
#include "stBase.h"
#include "UOD_GUID.h"
#include <vector>

class UOD_GUID;

template<typename T>
struct InfoCache
{
	T Info;

	std::chrono::time_point<std::chrono::steady_clock> Last;
};

typedef struct
{
	UINT32 Mode;
	INT32 Leader;
	UINT32 Time;
}stWater;

typedef struct
{
	INT32 ItemId;

	st_Item Item[8];
	INT32 Amount[8];
} stPacItem;

typedef struct
{
	// Indice da guilda que o cliente
    // que dominou a cidade possui
    int owner_index;

    // Indice da guilda que o cliente
    // que fez a maior aposta para
    // lutar com a guilda dominante
    int chall_index;

	// Indice da guilda que o cliente
	// que fez a maior aposta para lutar
	// com a outra guild desafiante (caso de cidades sem dono)
	int chall_index_2;

	// Posição em que as torres das guilds
	// irão nascer após a inicialização do servidor
	unsigned int tower_x, tower_y;

    // Posição que os membros da guilda
    // que é dona da cidade vao nascer
	unsigned int area_guild_x, area_guild_y;

    // Posição que os clientes que foram na
    // cidade por ultimo e depois em nenhuma
    // outra cidade iram nascer
	unsigned int city_x, city_y;

    // Posição minima e maxima da cidade, usado para
    // a verificação se o cliente entrou no
    // limite da cidade, assim podendo mudar
    // o indice da ultima cidade acessada
    unsigned int city_min_x, city_min_y, city_max_x, city_max_y;

    // Posicao minima e maxima da area da guilda, usado
    // para verificar se algum outro cliente que nao
    // é da guilda dominante da cidade entre na area
	unsigned int area_guild_min_x, area_guild_min_y, area_guild_max_x, area_guild_max_y;

    // Posição minima e maxima da guerra, usado para
    // depois que acabar a guerra de cidade o servidor
    // mandar todos os clientes que estão neste limite
    // para a cidade onde devem nascer, também é usado
    // para a verificacao se existe membro da guilda
    // desafiante vivo na guerra
	unsigned int war_min_x, war_min_y, war_max_x, war_max_y;

    // Posicao onde os membros da guilda que é dona
    // da cidade que esta acontecendo a guerra iram nascer
	unsigned int guilda_war_x, guilda_war_y;

    // Posicao onde os membros da guilda que desafiou
    // a guilda dona da cidade que esta acontecendo a
    // guerra iram nascer
	unsigned int guildb_war_x, guildb_war_y;

    // Imposto cobrado pelos produtos comprados
    // e vendidos pelos npcs e das lojas pessoais
    int perc_impost;

	// Quantas vezes a guild citada foi campeã
	// da respectiva cidade
	int win_count;

	// Imposto total arrecadado por cada cidade
	INT64 impost;
} sGuildZone;

enum class SlotType : int
{
	Equip,
	Inv,
	Storage,
	Rune,
	MeridaInv,
	MeridaEquip
};
typedef struct 
{
	st_Item Item;	// 0 - 7
	// 1 =Aberto 0 = fechado
	int Open; // 8 - 11
	unsigned int PosX; // 12 - 15
	unsigned int PosY; // 16 - 19
	
	// 3 = Não passa
	int CanRun; // 20 - 23
	int Status; // 24 - 27
	int Rotation; // 28 - 31

	DWORD IsOpen; // 32 - 35
	DWORD Unknow_36; // 36 - 39
	int HeightGrid; // 40 - 43
	DWORD Unknow_44[6];
}stInitItem;

typedef struct 
{
	short PosX;
	short PosY;
	short Index;
	short Rotate;
} BASE_InitItem;

typedef struct
{
	DWORD atkFisico;
	DWORD atkMagico;
	DWORD Evasion;
	DWORD Resist;
	DWORD speedMove;
} st_MountData;

union MapAttribute
{
	struct
	{
		UINT8 Village  : 1; // 1
		UINT8 CantGo  : 1; // 2
		UINT8 CantSummon : 1; // 4
		UINT8 House   : 1; // 8
		UINT8 Teleport  : 1; // 16
		UINT8 Guild   : 1; // 32
		UINT8 PvP   : 1; // 64
		UINT8 Newbie  : 1; // 128
	};
	UINT8 Value;
};

// DB -> GS

// GS -> DB

// -------
// Pacote 0x800
// -------
#define _ACCEPT_DAILY_QUEST 0x705

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
} stSaveChar;

struct MSG_DBServerChange
{
	PacketHeader Header;

	int NewServerID;
	int Slot;
};


typedef struct
{
	PacketHeader Header;

	INT32 Value;
} pMsgSignal;

typedef struct
{
	PacketHeader Header;
	
	INT32 Value;
	INT32 Value2;
} pMsgSignal2;

struct		 MSG_GuildReport
{
	PacketHeader Header;
	int ChargedGuildList[10][5];
};

typedef struct
{
	PacketHeader Header;
	
	INT32 Value;
	INT32 Value2;
	INT32 Value3;
} pMsgSignal3;

typedef struct
{
	PacketHeader Header;
	char Msg[128];
} p101;

typedef struct
{
	PacketHeader Header;
	BYTE Keys[16];

	INT32 Unkw;

	struct
	{
		INT16 PositionX[4];
		INT16 PositionY[4];

		char Name[4][16];

		st_Status Status[4];
		st_Item Equip[4][18];

		UINT16 GuildIndex[4];

		int Gold[4];
		INT64 Exp[4];
	} CharList;
	
	st_Item Storage[128];
	UINT32 GoldStorage;

	char UserName[16];
	BYTE Unknow[12];
} p10A;

typedef struct
{
	PacketHeader Header;

	struct
	{
		INT16 PositionX[4];
		INT16 PositionY[4];

		char Name[4][16];

		st_Status Status[4];
		st_Item Equip[4][18];

		UINT16 GuildIndex[4];

		int Gold[4];
		INT64 Exp[4];
	} CharList;
} p112;



// Spawn Info
typedef struct
{
	PacketHeader Header; // 0 - 11

	// Spawn Info
	struct
	{
		short X, Y;
	} Current; // 12 - 15

	 short Index; // 16 - 17

	char Name[12]; // 18 - 29

	// Killer Points
	unsigned char ChaosPoints; // 30 
	unsigned char CurrentKill; // 31 
	unsigned short TotalKill; // 32 - 33

	// Item Info
	WORD Item_Refine[18]; // 34 - 65


	// Skill Info
	struct
	{
		BYTE Time;
		BYTE Index;
	} Affect[32]; // 66 - 129

	// Guild Info
	WORD GuildIndex; // 130 - 131

	// Unknow byte 757 
	char GuildMemberType; // 132 - 133

	WORD Unknow;

	// Score Info 
	st_Status Status; // 134 - 181

	// Spawn Type
	struct
	{
		WORD Type;
	} Spawn; // 182 - 185

	char pAnctCode[18];
	char pTab[26];

	BYTE Unknow_02[4];

	int Life;
} p364;

// Spawn Info
typedef struct
{
	PacketHeader Header; // 0 - 11

	// Spawn Info
	struct
	{
		short X, Y;
	} Current; // 12 - 15

	short Index; // 16 - 17

	char Name[16]; // 18 - 33

	// Item Info
	WORD Item_Refine[18]; // 34 - 65


	// Skill Info
	struct
	{
		BYTE Time;
		BYTE Index;
	} Affect[32]; // 66 - 129

	// Guild Info
	WORD GuildIndex; // 130 - 131

	// Unknow byte 757 
	char GuildMemberType; // 132 - 133

	WORD Unknow;

	// Score Info 
	st_Status Status; // 134 - 181

	// Spawn Type
	struct
	{
		WORD Type;
	} Spawn; // 182 - 185

	char pAnctCode[18];
	char pTab[26];

	BYTE Unknow_02[4];

	int Life;
} p364_Mob;

// Spawn Info
typedef struct 
{
	PacketHeader Header;

    // Spawn Info
    struct
    {
        short X, Y;
    } Current;

    short Index;

    char Name[12];

	// Killer Points
	unsigned char ChaosPoints;
	unsigned char CurrentKill;
	unsigned short TotalKill;

    // Item Info
	WORD Item_Refine[18];

    // Skill Info
    struct
    {
		BYTE Time;
		BYTE Index;
    } Affect[32];

    // Guild Info
	WORD GuildIndex;

	// Unknow byte 757
	char Unknow;

    // Score Info
    st_Status Status;

    // Spawn Type
    struct
    {
        unsigned short Type : 8;
        unsigned short MemberType : 8;
    } Spawn;

    char pAnctCode[18];
    char pTab[26];

	char StoreName[27];
} p363;

typedef struct
{
	PacketHeader Header; // 0 - 11
	st_Position LastPos;  // 12 - 15

	UINT32 MoveType; // 16 - 19
	UINT32 MoveSpeed;  // 20 - 23

	char Command[24];  // 23 - 47

	st_Position Destiny; // 48 - 50
} p36C;

typedef struct 
{
    PacketHeader Header;
	int Type;
    char eChat[128];    
}p333;


const short _MSG_GuildZoneReport = (13 | 2048);
struct		 MSG_GuildZoneReport
{
	PacketHeader Header;
	int	Guild[5];
};

typedef struct 
{
    PacketHeader Header;
    char eCommand[16];
    char eValue[100];
}p334;

typedef struct
{
	PacketHeader Header;

	st_Affect Affect[32];
} p3B9;

struct  p376
{
    PacketHeader Header;
    BYTE DstType;
	BYTE DstSlot;
	BYTE SrcType;
	BYTE SrcSlot;
	WORD PosX;
	WORD PosY;
} ;


typedef struct
{
	PacketHeader Header;
    short invType;
    short invSlot;
    st_Item itemData;
} p182;

typedef struct 
{
    PacketHeader Header;
    
	short ItemEff[18];
	char pAnctCode[18];
} p36B; 

struct p3B2
{
	PacketHeader Header;
	int unknown;
	char Nickname[16];
};

typedef struct
{
	PacketHeader Header; // 0 - 11

	st_Status Status; // 12 - 59

	BYTE Critical; // 60
	BYTE SaveMana; // 61

	struct
	{
		BYTE Time;
		BYTE Index;
	} Affect[32]; //62 - 125

	WORD GuildIndex; // 126 - 127
	WORD GuildLevel;
	BYTE RegenHP, // 128
		RegenMP; // 129

	struct
	{
		BYTE Resist1; // 130
		BYTE Resist2; // 131
		BYTE Resist3; // 132
		BYTE Resist4; // 133
	} Resist; // 804 - 807


	WORD Unknow; // 134 - 135
	WORD CurrHP; // 136 - 137
	DWORD CurrMP; // 138 - 142
	BYTE Unknow_2; // 143
	int MagicIncrement; // 144 - 147
	DWORD Unknow_148; // 148 - 151
	int Life;
} p336;

typedef struct
{
    PacketHeader Header; // 0 - 11

    char Name[24]; // 12 - 35
    st_Item Item[12]; // 36 - 131

    BYTE Slot[12]; // 132 - 143

    int Gold[12]; // 144 - 191
    short Unknown; // 192 - 193
    short Index; // 194 - 195
} p397;

typedef struct
{
	PacketHeader Header;
	int slot;
	short mobid;
	int price;
	int unk;
	st_Item Item;	
} p398;

// Request Open Trade
typedef struct 
{
	PacketHeader Header;
    int Index;
} p39A;

struct p39B
{
	PacketHeader Header;
	INT32 MobID;
	INT32 SlotID;
};

typedef struct
{
	PacketHeader Header;

	UINT32 Gold;
} p339;

typedef struct
{ 
	PacketHeader Header;

	UINT32 Index;
} p384;

typedef struct 
{
   PacketHeader Header; // 0 - 11
   short Mode; // 12 - 13
   short Info; // 14 - 15
   int unk; // 16 - 17
}p277;

typedef struct 
{
	PacketHeader Header;

	DWORD Hold;
	UINT64 Exp;
	UINT32 Learn;
	UINT32 SecLearn;

	WORD pStatus;
	WORD pMaster;
	WORD pSkills;
	BYTE Magic; // Unknow
	BYTE Unk;

	DWORD Gold;
} p337;

typedef struct
{
	PacketHeader Header;
    short npcID;
    short Warp;
} p27B;

typedef struct 
{
	PacketHeader Header;
	int Unknow;
	st_Item Item[27];
	int Taxes;
}p17C;

typedef struct
{
	PacketHeader Header;

    short mobID;
    short sellSlot;
    short invSlot;
    short Unknown1;
    int Unknown2;
} p379;

typedef struct 
{
	INT32 Index;
	INT32 Damage;
}st_Target;



typedef struct 
{
	PacketHeader Header; // 0 - 11
	INT32 Hold; // 12 - 15
	INT32 reqMP;  // 16 - 19
	//INT32 Unk; // 20 - 23
	INT64 currentExp;  // 24 - 27
	short unknow;  // 28 - 31
	st_Position attackerPos;  // 32 - 35
	st_Position targetPos;  // 36 - 39
	short attackerId;  // 40 - 41
	short attackCount;  // 42 - 43
	unsigned char Motion; // 44
	unsigned char skillParm;  // 45
	unsigned char doubleCritical; // 46
	unsigned char FlagLocal;
	//char flagLocal;  // 47
	short Rsv;

	int currentMp;
	short skillId;  // 48 - 51
	short reqMp;
	st_Target Target[13];  // 56 - 
	//int padding;
} p367;

typedef struct 
{
	PacketHeader Header; // 0 - 11
	INT32 Hold; // 12 - 15
	INT32 reqMP;  // 16 - 19
	INT32 Unk; // 20 - 23
	INT64 currentExp;  // 24 - 27
	short unknow;  // 28 - 31
	st_Position attackerPos;  // 32 - 35
	st_Position targetPos;  // 36 - 39
	short attackerId;  // 40 - 41
	short attackCount;  // 42 - 43
	unsigned char Motion; // 44
	unsigned char skillParm;  // 45
	unsigned char doubleCritical; // 46
	unsigned char FlagLocal;
	//char flagLocal;  // 47
	short Rsv;

	int currentMp;
	short skillId;  // 48 - 51
	short reqMp;
	st_Target Target; //44 - 47
	//int   Padding;
} p39D;

typedef struct 
{       
	PacketHeader Header;
	int Hold;
    short killed;
	short killer;
	INT32 Unknow;
	INT64  Exp;
}p338;

typedef struct 
{
	PacketHeader Header;

	INT32 CurHP;
	INT16 Incress;
}p18A;

typedef struct 
{
	PacketHeader Header;

	INT16 Motion;
	INT16 Parm;
	INT32 NotUsed;
} p36A;

typedef struct 
{
	PacketHeader Header;

	INT32 curHP;
	INT32 curMP;
	INT32 maxHP;
	INT32 maxMP;
} p181;

typedef struct  {
	PacketHeader Header; 
	DWORD SrcType; 
	DWORD SrcSlot; 
	DWORD DstType; 
	DWORD DstSlot; 
	WORD PosX; 
	WORD PosY; 
	int warp; 
}p373;

typedef struct
{
	PacketHeader Header; // 0 -11 
	st_Position Init; // 12 - 15
	short Index; // 16 - 17
	st_Item Item; // 18 - 25
	BYTE Rotation; // 26
	BYTE Status; // 27
	BYTE HeightGrid; // 28
	BYTE Unknow; // 29;
} p26E;

struct		  MSG_DecayItem
{
	PacketHeader Header;

	short ItemID;
	short unk;
};

typedef struct 
{
	PacketHeader Header;
    int invType;
    int InvSlot;
    int Unknown1;
    short posX;
    short posY;
    int Unknown2;
}p272;

typedef struct
{
	PacketHeader Header;

	int invType;
	int invSlot;
	
    int Unknown1;
	short posX;
	short posY;
} p175;

typedef struct
{
	PacketHeader Header;
    WORD partyId;
    WORD Level;
    WORD maxHP;
    WORD curHP;
    WORD leaderId;
    char nickName[16];
    BYTE unk;
    WORD targetId;
} p37F;

typedef struct
{
	PacketHeader Header;
    WORD liderID;
    char nickName[16];
} p3AB;

 typedef struct
{
	PacketHeader Header;
    WORD LiderID;
    WORD Level;
    WORD maxHP;
    WORD curHP;
    WORD PartyID;
    char nickName[16];
    WORD ID;
} p37D;

 typedef struct
{
	PacketHeader Header;
	INT16 mobId;
} p37E;

typedef struct 
{
	PacketHeader Header;
	int gateId; // 12 - 15
	short status; // 16 - 17
	short unknow; // 18 - 19
}p374;

typedef struct{
	PacketHeader Header;

	INT32 CurHP;
	INT16 Status;
}  p292 ;

typedef struct
{
	PacketHeader Header;
	unsigned char SkillBar1[4];
    unsigned char SkillBar2[16];
} p378;

typedef struct { 
	PacketHeader Header; 
	short npcId; 
	short type; 
	int sellSlot; 
} p37A; 

typedef struct
{
	PacketHeader Header;
	short SlotID;
	short Unk;
	short ItemID;
	short Unk2;
} p2E4;

typedef struct
{
	PacketHeader Header;
    int invType;
    int InvSlot;
    short initID;
    short posX;
    short posY;
    short Unknown1;
} p270;

// Request Remove Item
typedef struct 
{
	PacketHeader Header;
    short initID;
    short NotUsed1;
}p16F;

typedef struct
{
	PacketHeader Header;
	int amount;
	struct
	{
		char name[16];
		short X;
		short Y;
		int exp;
		int gold;
		st_Item carry[64];
	}Drop[10];
} p2568;

struct    NewStore
{
	PacketHeader Header;
	int Parm1;
};

struct LojaDonate
{
	int type;

	int page;

	st_Item item;

	int price;

	int stuck;

	int slot;

	LojaDonate(int type, int page, st_Item item, int price, int stuck, int slot)
	{
		this->type = type;
		this->page = page;
		memcpy_s(&this->item, sizeof(st_Item), &item, sizeof(st_Item));
		this->price = price;
		this->stuck = stuck;
		this->slot = slot;
	}
};

struct LojaDonateInfor
{
	int Type;

	int Page;

	int Slot;
};


typedef struct
{        
	PacketHeader Header;
	int DestType;
	int	DestPos;
	st_Item Item;
} p171;

typedef struct
{
	PacketHeader Header;
	st_Item Item[15];
	char Slot[15];
	BYTE Unknow;
	int Gold;
	bool Confirm;
	WORD ClientId;
} p383;

typedef struct
{
	PacketHeader Header;

	st_Item Item[64];
	INT32 Gold;
} p185;

typedef struct
{
	PacketHeader Header;

	INT32 mobId;
	INT32 Type;
} p39F;

typedef struct
{
	PacketHeader Header;
	INT16 MinX;
	INT16 MinY;
	INT16 MaxX;
	INT16 MaxY;
	INT16 Type1;
	INT16 Type2;
} p3A2;

typedef struct //NPC CLICK
{
	PacketHeader Header;
	DWORD npcId;
	DWORD click;
} p28B;

typedef struct
{
	PacketHeader Header;

	stCharInfo Mob;
	INT32 Cash;
	char Pass[16];
	INT32 Blocked;
} p802;

typedef struct
{
	PacketHeader Header;

	stCharInfo Mob;
	UINT32 Value;
} p804;

typedef struct {
	PacketHeader Header;
	INT32 SlotID;
	INT32 ItemID;
	INT32 Amount;
} p2E5;

typedef struct
{
	PacketHeader Header;
    st_Item items[8];
    char slot[8];
} pCompor;

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

typedef struct
{
	PacketHeader Header; // 0 -11

	char Username[16]; // 12 - 27
	char Token[52];
} p52A;

typedef struct 
{
	PacketHeader Header;

	INT32 Warp;
	INT32 Type;
} pAD9;

typedef struct {
	PacketHeader Header;
	char eMsg[96];
} pD1D;

typedef struct {
	PacketHeader Header;
	DWORD GuildIndex1;
	DWORD GuildIndex2;
} pE12;

typedef struct
{
	PacketHeader Header;
	INT16 User;
	INT16 Unknow;
} p3AD;

typedef struct
{
	PacketHeader Header;
	DWORD WeatherId;
} p18B;


struct		  MSG_NPNotice
{
	PacketHeader Header;
	int  Parm1;
	int  Parm2;
	char AccountName[16];
	char String[96];
};



typedef struct{
	PacketHeader Header;
	short Value;
	short Total;
} p3BB;

typedef struct  
{
	PacketHeader Header;
	WORD unknow; // 12 - 13
	WORD confirm; // 14 - 15
	WORD slotId; // 16 - 17
	WORD unkw_2; // 18 - 19
}p2D4;

#define MAX_NPCEVENTO 10
#define MAX_NPCQUEST_CONDITION 10
#define MAX_NPCQUEST_CONDITION_ITEM 4

#define MAX_NPCQUEST_REWARD 10
#define MAX_NPCQUEST_REWARD_ITEM 4

#define MAX_NPCQUEST 50

typedef struct
{
	INT32 minLevel;
	INT32 maxLevel;

	struct
	{
		INT32 Item;
		INT32 Amount;
	} Item;
	
	struct
	{
		INT32 Slot;
		INT32 ItemID;
	} Equip;

	INT32 Exp;
	INT32 Gold;

	INT32 Evolution;
	INT32 Class;

	char Speech[96];
} stNPCQuest_Condition;

typedef struct
{
	struct
	{
		st_Item Item;

		INT32 Amount;
	}Item[MAX_NPCQUEST_REWARD_ITEM];
	INT32 Exp;
	INT32 Level;
	INT32 Gold;
	
	struct
	{
		INT32 Slot;
		st_Item Item;
	} Equip;

	st_Position Teleport;

	char Speech[96];
} stNPCQuest_Reward;

typedef struct
{
	struct
	{
		INT32 Item;
		INT32 Amount;
	} Item[MAX_NPCQUEST_REWARD_ITEM];

	INT32 Exp;
	INT32 Gold;
	
	struct
	{
		INT32 Slot;
		INT32 Item;
	} Equip; 
} stNPCQuest_Remove;

typedef struct
{
	char Name[16];
	char Tab[28];

	st_Position Pos;

	stNPCQuest_Condition Condition[MAX_NPCQUEST_CONDITION];	
	stNPCQuest_Reward    Reward   [MAX_NPCQUEST_REWARD];
	stNPCQuest_Remove    Remove;
} stNPCQuest;

typedef struct
{
	// ------
	// Item requerido
	// ------
	st_Item itemRequired [10];
	
	// ------
	// Quantidade de itens requeridos 
	// Na mesma ordem que os itens pedidos
	// ------
	UINT8 amountRequired[10];
	
	// ------
	// Gold pedido no NPC
	// ------
	INT32 goldRequired;

	// ------
	// Premiações
	// ------
	st_Item itemEarned [10][10];

	// ------
	// Rates
	// ------
	UINT16 Rates[10];

	// ------
	// Posição que será teleportado
	// ------
	st_Position Pos[10];
	char msg[10][108];
	char npcId[108];
} stNPCEvent; 

typedef struct
{
	st_Position Max;
	st_Position Min;
} stPositionCP;

typedef struct
{
	PacketHeader Header;

	UINT32 server;
} pED7;

typedef struct 
{
	PacketHeader Header;

	INT32 Type;
} p27C;

#define FLAG_DB2GAME     0x0400
#define _MSG_STARTTOWERWAR          ( 15 | FLAG_DB2GAME)

// Estrutura da guerra de torres
typedef struct
{
	INT16 TowerState; // Indica qual canal está avançando neste momento
	INT16 WarState;   // Indica se a guerra está declarada ou recusada
	INT16 TMP;
} stTowerWar;

struct MSG_STARTTOWERWAR
{
	PacketHeader Header;

	BYTE isStarting;

	stTowerWar war[10];
};

#define MSG_UPDATETOWERINFO 0x999
struct _MSG_UPDATETOWERINFO
{
	PacketHeader Header;

	int  KillerId; // GuildIndex que matou a torre
};

#define MSG_FRIENDLIST_UPDATESTATUS_OPCODE 0x907
typedef struct 
{
	PacketHeader Header;

	char Name[16];
	INT32 Status; // 1 = online - 0 = offline
	INT32 Server;
} _MSG_FRIENDLIST_UPDATESTATUS;

#define MSG_FRIENDLIST_OPCODE 0x905
typedef struct
{
	PacketHeader Header;

	INT32 ClientID;

	char Name[30][16];
	INT8 Status[30];
	INT8 Server[30];
} _MSG_FRIENDLIST; 

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

#define MSG_FRIENDLIST_REQUESTUPDATE_OPCODE 0x906
typedef struct
{
	PacketHeader Header;

	INT32 Type;
	INT32 Server;
	char Name[16];
} _MSG_FRIENDLIST_UPDATE;

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

#define MSG_FIRST_KEFRA_NOTIFY 0x90A
#define MSG_REBORN_KEFRA 0x90B
typedef struct
{
	PacketHeader Header;

	int Channel;
} _MSG_FIRST_KEFRA_NOTIFY;

typedef struct
{
	PacketHeader Header;

	st_Item Item[27];
	INT32 Price[27];
	INT32 LK;
} _MSG_STOREDONATE;

#define MSG_BLOCKPASS_OPCODE 0x903
typedef struct
{
	PacketHeader Header;

	char Password[16];
} _MSG_BLOCKPASS;

constexpr int MSG_BUYSTORE_OPCODE = 0x380;
typedef struct
{
	PacketHeader Header;

	INT32 Price;
	st_Item Item;
} _MSG_BUYSTORE;

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

#define MSG_PERFIL_OPCODE 0x881
typedef struct
{
	PacketHeader Header;

	char nickname[16];
	INT32 Level;
	struct
	{
		INT32 Kill;
		INT32 Death;
	} PvP;

	struct
	{
		INT32 Kill;
		INT32 Death;
	} PvM;

	INT32 Fame;
	INT32 Citizen;
	INT32 CapeId;
	char guildName[16];
} _MSG_PERFIL;

struct p3E8
{
	PacketHeader Header;

	struct
	{
		int index;
		st_Item item;
		int sellPrice;
	} Item[10];
};

enum class TOD_Store_Type
{
	PremierStore,
	ArenaStore
};

typedef struct
{
	PacketHeader Header;
	TOD_Store_Type Type;
	st_Item Item[40];
	INT32 Price[40];
} _MSG_NEWSTORE;

typedef struct
{
	PacketHeader Header;

	INT32 Requested;
	BYTE Mac[6];
} p655;

typedef struct
{
	PacketHeader Header;

	INT32 Requested;
	char nick[16];
} p656;

constexpr int Msvfw32IntegrityPacket = 0x635;

struct _MSG_MSVFW32_INTEGRITY
{
	PacketHeader Header;

	char Path[MAX_PATH];
};

struct _MSG_MACADDRESS_INTEGRITY
{
	PacketHeader Header;

	unsigned char mac[8];
	unsigned long crc32;
};

constexpr int MacAddressIntegrityPacket = 0x636;

constexpr int ModulesInfoPacket = 0x637;

struct _MSG_MODULESINFO
{
	PacketHeader Header;

	unsigned int size;
	unsigned int crc32;
	char* modules;
};

struct _MSG_REALBATTLE
{
	PacketHeader Header;

	int Kingdom;
	int Status;
};

constexpr int RealBattlePacket = 0x637;

#pragma region PAINEL DE GUILD
#define MSG_PANELGUILD_GETLIST 0x766
typedef struct 
{
	PacketHeader Header;

	stGuild Guild[40];
} _MSG_PANELGUILD_GETLIST;
#pragma endregion

constexpr int RefreshGoldPacket = 0x3B1;

#pragma region DAILY REWARD

constexpr int DailyRewardInfoPacket = 0x671;
constexpr int DailyRewardCollectPacket = 0x672;

#pragma pack(push, 1)
struct MSG_DAILYREWARDINFO
{
	PacketHeader Header;

	// Todas as premiações
	st_Item Item[7][4];

	// Quais dias ele recebeu
	bool Received[7];

	// Dia que está
	int Day;
};
#pragma pack(pop)
#pragma endregion

struct MSG_SENDSCORE_CUSTOM
{
	PacketHeader Header;

	int Lifes;
};

struct MSG_QUIZ 
{
	PacketHeader Header;
	char Title[128];
	char Ans[4][32];
};

struct MSG_QUIZ_ANSWER
{
	PacketHeader Header;
	short Asw;
	short Unk;
};

constexpr int ChristmasMissionPacket = 0x673;
constexpr int ChristmasMissionReplyPacket = 0x674;
constexpr int ChristmasMissionAcceptedPacket = 0x675;

struct MSG_CHRISTMASMISSION
{
	PacketHeader Header;

	char Title[32];

	struct
	{
		char MobName[3][16];
		int MobCount[3];
	} Info;

	struct
	{
		int MobCount[3];
	} Completed;

	int Status;

	st_Item Rewards[4];
};

constexpr int ArenaScoreboardRefreshPacket = 0x690;
constexpr int ArenaScoreboardSetStatusPacket = 0x691;
struct MSG_ARENASCOREBOARD
{
	PacketHeader Header;

	int Points[4];
};

constexpr int AutoPartyInfoPacket = 0x700;
constexpr int AutoPartySetPasswordPacket = AutoPartyInfoPacket + 1;
constexpr int AutoPartyAddRemoveNamePacket = AutoPartyInfoPacket + 2;
constexpr int AutoPartyDisableEnableAllPacket = AutoPartyInfoPacket + 3;
constexpr int AutoPartyEnterPartyPacket = AutoPartyInfoPacket + 4;

struct MSG_AUTOPARTY
{
	PacketHeader Header;

	int EnableAll;
	char Nickname[15][16];
};

struct MSG_AUTOPARTY_ADDREMOVE_NAME
{
	PacketHeader Header;

	char Nickname[16];
	int Mode;
};

struct MSG_AUTOPARTY_SETPASSWORD
{
	PacketHeader Header;

	char Password[16];
};

struct MSG_AUTOPARTY_ENTERPARTY
{
	PacketHeader Header;

	char Password[16];
	char Nickname[16];
};

constexpr int MissionInfoPacket = 0x347;
constexpr int MissionCollectPacket = MissionInfoPacket + 1;
constexpr int MissionAcceptOrRejectPacket = MissionInfoPacket + 2;

struct MissionInfo_Reward
{
	st_Item Item[6];
};

struct MSG_MISSIONINFO
{
	PacketHeader Header;

	bool IsAccepted;
	char Name[32];
	
	struct
	{
		char Name[16];
		int Total;

		int Killed;
	} Mob[5];

	struct
	{
		int ItemId;
		int Total;

		int Dropped;
	} Item[5];

	st_Item FreeReward[6];
	st_Item BattlePassReward[6];

	int Gold;
	unsigned long long Exp;

	stDate BattlePassValidation;
	stDate LastUpdate;
};

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
struct pDC3
{
	PacketHeader Header;
	SealInfo Info;
};

struct TOD_RvRStore_Item
{
	st_Item Item;
	int Available;
	int Price;
};

constexpr int MSG_RVRSTORE_BUY_OPCODE = 0x17F;
struct p17E
{
	PacketHeader Header;

	st_Item Item[27];
	int Price[27];
};

constexpr int ChatMessagePacket = 0xD2D;

struct MSG_CHATMESSAGE
{
	PacketHeader Header;

	char Message[96];
	int Color;
};

struct MobDropInfo
{
	char Name[16];
	st_Position Position;

	INT32 Level;
	INT32 MaxHp;
	INT64 Exp;

	//
	st_Item item[30];
};

constexpr int UseEssencePacket = 0x473;

struct MSG_ESSENCEPACKET
{
	PacketHeader Header;

	int Slot;
	int Amount;
};

constexpr int WelcomePacket = 0x985;
constexpr int PromotionPacket = 0x986;
constexpr int ResetSoulDelayPacket = 0x987;

constexpr int StoreInfoPacket = 0x586;
constexpr int BuyStorePacket = 0x587;
constexpr int AskStoreInfoPacket = 0x588;

struct TOD_OverStore_ItemInfo
{
	st_Item Item;
	short Price;
	short Available{ -1 };
};

struct TOD_OverStore_Category
{
	std::string Name;
	std::vector<TOD_OverStore_ItemInfo> Items;
};

struct MSG_BUYOVERSTORE
{
	PacketHeader Header;

	st_Item Item;
	int Quantity;
	char Nickname[16];
};

constexpr int RecruitRequestPacket = 0x680;
constexpr int RecruitAcceptPacket = 0x681;

struct MSG_RECRUITREQUEST
{
	PacketHeader Header;
	char Nickname[16];
	char GuildName[16];
	int GuildId;
	int ClientId;
};

constexpr int NightmareCanEnterWarnPacket = 0x630;
constexpr int NightmareAcceptPacket = 0x631;


constexpr int RedeemGriffinPacket = 0x632;
constexpr int RedeemGriffinClosePacket = 0x633;

constexpr int WPEOnWarnPacket = 0x634;

#endif