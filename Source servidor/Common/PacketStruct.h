#pragma once
#include <Windows.h>
#include <vector>
#include "stBase.h"

typedef struct
{
	WORD Size;
	BYTE Key;
	BYTE CheckSum;
	WORD PacketId;
	WORD ClientId;
	DWORD TimeStamp;
} PacketHeader;

struct MSG_MessagePanel
{
	PacketHeader Header;
	int nID;
	char String[92];
};

struct  MSG_STANDARDPARM3
{
	PacketHeader Header;
	int Parm1;
	int Parm2;
	int Parm3;
};

struct  MSG_STANDARDPARM1
{
	PacketHeader Header;
	int Parm1;
};

struct  MSG_STANDARDPARM2
{
	PacketHeader Header;
	int Parm1;
	int Parm2;
};

#define _MSG_DBSavingQuit         ( 10 | FLAG_DB2GAME)
struct  MSG_DBSavingQuit
{
	PacketHeader Header;
	char AccountName[16]; 
	int  Mode; 
};

struct st_CharList
{
	INT16 PositionX[4];
	INT16 PositionY[4];

	char Name[4][16];

	st_Status Status[4];
	st_Item Equip[4][18];

	UINT16 GuildId[4];

	int Gold[4];
	INT64 Exp[4];
}; // 744

struct p213 {
	PacketHeader Header;
	int CharIndex;
	char Zero[18];
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
};

struct pFDE
{
	PacketHeader Header;
	char num[16];

	INT32 RequestChange;
};

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

	INT32 PosID;
	char Name[16];
	INT32 ClassID;
	INT32 ClassInfo;
	INT32 MortalSlot;
} MSG_DBNewArch;

typedef struct {
	PacketHeader Header;
	char Password[12];
	char Login[16];

	const char Unknow[52];
	UINT32 CliVer;
	UINT32 Unknow_84;

	char Keys[16];
} p20D;

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

typedef struct
{
	PacketHeader Header;
	int SlotIndex;
	char Name[16];
	char Pwd[12];
} p211;

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

constexpr int LogPacket = 0x8580;
class BufferWriter
{
	size_t index{ 0 };
	std::vector<unsigned char> data;
public:
	BufferWriter(int size)
	{
		data.resize(size);
	}

	BufferWriter() = default;

	void advance(unsigned int size)
	{
		index += size;
	}

	BufferWriter& operator+=(unsigned int value)
	{
		index += value;

		return *this;
	}

	template<typename T>
	void Set(T value, int position = -1)
	{
		bool moveIndex{ false };
		if (position == -1)
		{
			position = index;
			moveIndex = true;
		}

		if (position + sizeof (T) > data.size())
			data.resize(position + sizeof (T));

		if constexpr (std::is_class<T>::value)
			memcpy(&data[position], (void*)&value, sizeof (T));
		else
			*reinterpret_cast<T*>(&data[position]) = value;

		if (moveIndex)
			index += sizeof(T);
	}

	template<typename T = std::string>
	void Set(const char* value, size_t size)
	{
		Set<unsigned int>(size);

		if (index + size > data.size())
			data.resize(index + size);

		memcpy_s(&data[index], data.size() - index, value, size);

		index += size;
	}

	std::vector<unsigned char> GetBuffer()
	{
		return data;
	}

	template<typename T>
	T* GetAs()
	{
		return reinterpret_cast<T*>(data.data());
	}
};

#define FLAG_GAME2CLIENT				0x0100
#define FLAG_CLIENT2GAME				0x0200
#define FLAG_DB2GAME					0x0400
#define FLAG_GAME2DB					0x0800

#define _MSG_MessagePanel               ( 1 | FLAG_GAME2CLIENT)
#define _MSG_DBNewCharacter				( 2 | FLAG_GAME2DB)
#define _MSG_DBAccountLogin				( 3 | FLAG_GAME2DB) 
#define _MSG_DBCharacterLogin			( 4 | FLAG_GAME2DB)
#define _MSG_DBCNFCharacterLogin		0x417
#define _MSG_DBCNFCharacterLoginExtra	0x820
#define _MSG_DBNoNeedSave				( 5 | FLAG_GAME2DB) 
#define _MSG_DBUpdateSapphire			0x80E
#define _MSG_DBStaffMode				0x80F
#define _MSG_DBSaveMobQuit				( 6 | FLAG_GAME2DB) 
#define _MSG_DBSaveMob					( 7 | FLAG_GAME2DB)
#define	_MSG_DBDeleteCharacter			( 9 | FLAG_GAME2DB)
#define _MSG_STARTTOWERWAR				( 15 | FLAG_DB2GAME)
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
#define _MSG_DBNewArch					( 11 | FLAG_GAME2DB)