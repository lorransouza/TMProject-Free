#include <Windows.h>
#include <winsock.h>
#include <ctime>

// Windows librarys
#include <fstream>
#include <string>
#include <mutex>

// Include the project files
#include "stBase.h"
#include "KeyTable.h"
#include "Struct.h"
#include "EncDec.h"
#include "CUser.h"
#include "Socket.h"
#include "CMob.h"
#include "ItemEffect.h"
#include "CNPCGener.h"
#include "MessageSender.h"
#include "AntiBot.h"

#ifndef __CSERVER_H__
#define __CSERVER_H__

#include <array>
#include <memory>
#include <vector>

// Adiciona a library da winsock
#pragma comment (lib, "WS2_32.lib")

// Define argumentos padrões para o sistema
// de sockets
#define ACCEPT_USER WM_USER + 1
#define RECV_USER   WM_USER + 2
#define WM_SOCKET   WM_USER 
#define WM_DATASERVER	WM_USER + 3

#define WSA_READ            (WM_USER + 100)
#define WSA_READDB          (WM_USER + 2) 
#define WSA_ACCEPT          (WM_USER + 3) 

// Define os valores máximos 
#define MAX_BUFFER 65535 * 8
#define MAX_PLAYER 1000
#define SCHEDULE_ID MAX_PLAYER - 1
#define CLIVER 6999


#define MAX_MESSAGE_SIZE 8096
#define INITCODE 0x1F11F311

// Define os valores padrões
#define NOT_CONNECT 0
#define IN_PROCCESS_CONNECT 1
#define WAITING_HELLO 2
#define CONNECTED 4

#define	TIMER_SEC    0
#define TIMER_MIN    1
#define	TIMER_HOUR   2
#define	TIMER_SEND   3

#define MH 8

#define VIEWGRIDX 33
#define VIEWGRIDY 33
#define HALFGRIDX 16
#define HALFGRIDY 16

#define MAX_QUESTDIARIA 256
#define MAX_LEVELITEM 256
#define MAX_PREMIERSTORE 40

constexpr int MaxMessage = 128;

// Struct to GUI user interface
struct stGUI
{
	HWND hGUI,
	hButton[3],
	hLabel[3];

	HFONT Font;
	ATOM Class;
};

struct BossQuest
{
	bool isAlive;
	int  gennerId;
	st_Item Gifts[10];
	INT8 Chances[10];
	int CountToBorn;
};

struct Quests
{
	INT16 KillCount;
	BossQuest Boss;
};

struct TOD_QuizQuestions
{
	std::string Question;
	std::array<std::string, 4> Answers;
};

struct TOD_ChristmasMissionInfo
{
	int Id;

	std::string Title;
	std::array<std::string, 3> MobName;
	std::array<int, 3> Count;
	std::array<st_Item, 4> Reward;
};

struct TOD_DailyQuestInfo_RewardItem
{
	st_Item Item;
	unsigned int Amount;
};

enum class TOD_DailyQuestInfo_Type
{
	Normal = 0,
	Special
};

enum class TOD_Valley
{
	First,
	Second
};

struct TOD_MissionInfo
{
	int QuestId;
	TOD_DailyQuestInfo_Type Type; // 0 = mob, 1 = drop, 2 = mob + drop
	std::vector<int> ClassMaster;

	struct
	{
		// Pedido de até 04 mobs por quest
		// O mob é checado através do nome
		std::array<std::string, 5> mobName;

		// Quantos mobs devem ser mortos
		std::array<unsigned int, 5> Amount;
	} Mob;

	struct
	{
		// Pedido te até 08 itens
		std::array<unsigned int, 5> Item;

		// Quantos itens vão ser pedidos
		std::array<unsigned int, 5> Amount;
	} Drop;

	// Aqui a premiação entregue pela quest diária
	// Capaz de entregar gold, exp e até 05 itens

	std::array<TOD_DailyQuestInfo_RewardItem, 6> FreeReward;
	std::array<TOD_DailyQuestInfo_RewardItem, 6> BattlePassReward;

	int Gold;
	int Exp;

	int MinLevel;
	int MaxLevel;

	std::string QuestName;
} ;


#define _REFRESH_DAILYQUEST_OPCODE 0x701
struct stRefreshDailyQuest
{
	PacketHeader Header;
	UINT32 MobCount[5];
	UINT16 MobCountKill[5];

	UINT32 ItemCount[5];
	UINT16 ItemCountDrop[5];
};

#define _DAILY_INFO_OPCODE 0x702
#define _FINISH_DAILY_OPCODE 0x703
struct stDailyInfo
{
	PacketHeader Header;
	struct
	{
		// Pedido de até 04 mobs por quest
		// O mob é checado através do nome
		char mobName[5][32];

		// Quantos mobs devem ser mortos
		UINT32 Amount[5];
	} Mob;

	struct
	{
		// Pedido te até 08 itens
		UINT32 Item[5];

		// Quantos itens vão ser pedidos
		UINT32 Amount[5];
	} Drop;

	struct
	{
		UINT32 Gold;
		UINT32 Exp;
		UINT32 ScaleExp;

		st_Item Item[5];
		UINT32 Amount[5];
	} Reward;

	char History[5][85];
	char QuestName[32];

	short Accepted;
	UINT16 AmountKilled[5];
	UINT16 AmountDroped[5];

};

struct stLevelItem
{
	INT32 Evolution;
	INT32 Type;
	INT32 Level;
	INT32 Classe;

	st_Item item;
};

struct TOD_AreaToDrop
{
	int index;
	st_Position Min;
	st_Position Max;

	st_Item Item;
	mutable int Dropped;

	int Rate;
	int Limit;
};

struct TOD_KingdomBattle
{
	bool Status;

	int TowerId;
	int KingId;

	bool isTowerAlive;
	bool isKingAlive;
};
	 
enum class TOD_Colosseum_Type
{
	Normal,
	Mystic,
	Arcane
};

struct InternalArena
{
	unsigned long long Experience;

	int MaximumLevel;
};

struct ChallengeInfo
{
	int GuildId;
	int Value;
};

struct Scheduled
{
	std::string Command;

	int Month;
	int Day;

	int Hour;
	int Min;

	bool Executed;
	std::chrono::time_point<std::chrono::steady_clock> ExecuteTime;
};

struct TOD_MobBoss
{
	std::vector<int> Geners;
	int Index;
	int TimeToReborn;
	int MaxTimeIngame;
	int Fame;
	int GenerGenerated;

	std::chrono::time_point<std::chrono::steady_clock> LastUpdate;
};

struct ArenaReward
{
	std::vector<st_Item> Items;
	std::array<std::vector<InternalArena>, 4> Experience;
	int Gold;
};

struct TOD_BonusExpArea
{
	int Index;

	st_Position MinPosition;
	st_Position MaxPosition;

	int Value;

	bool operator==(const int& rhs)
	{
		return Index == rhs;
	}
};

struct TOD_Nightmare
{
	// Membros no grupo, permitidos até 40
	std::array<unsigned short, 40> Members;
	std::array<std::string, 40>  MembersName;

	// Restante de NPCs
	UINT16 NPCsLeft;

	// Tempo restante no Pesadelo
	UINT16 TimeLeft;

	// True quando o NPC já morreu
	// False quando ainda não morreu
	UINT8 Alive[8];

	UINT16 Status;
};
constexpr int MaxBoss = 64;

// Estrutura do banco de dados
struct stServer
{
	struct
	{
		UINT32 Porta;
		char IP[32];
	} Config;

	struct
	{
		UINT32 Porta;
		char IP[32];
	} Data;

	bool Status;

	SOCKADDR_IN Service;
	UINT32 Socket;

	UINT32 UsersON;

	// Level na área de treinamento
	UINT32 NewbieZone;

	// Contadores
	UINT32 MinCounter;
	UINT32 HourCounter;
	UINT32 SecCounter;

	// DeadPoint
	// Perda de experiência em caso de morte?
	UINT32 DeadPoint;

	UINT32 SaveCount; // 4CBBD8

	// Contagem ed Init
	UINT32 InitCount;
	UINT32 ItemCount;

	// Bônus experiência
	UINT32 BonusEXP;

	UINT32 ExpDailyBonus;
	UINT32 GoldDailyBonus;

	std::vector<TOD_BonusExpArea> BonusExpArea;

	UINT32 Sapphire;
	UINT32 StatSapphire;

	// Rates para refinar refinação abençoada
	UINT8 RateRef[5];

	struct
	{
		INT32 RankingProcess; // 30
		INT32 Timer; // 44

		INT16 Challanger1; // 34
		INT16 Challanger2; // 38

		char RankingName[4][32]; // 0 = 4CBC9C 32 em 32
		INT32 RankingLevel[2];
	} Challanger;

	UINT32 Channel;
	UINT32 TotalServer;
	UINT32 ServerGroup; //4C7BC0


	UINT32 NoviceChannel;
	UINT32 WarChannel;

	UINT32 FirstKefra;
	UINT32 KefraKiller;
	UINT8  KefraDead;

	UINT32 Kingdom1Clear;
	UINT32 Kingdom2Clear;
	
	stWater pWater[3][9];
	
	TOD_Nightmare Nightmare[3];
	UINT32 GoldBonus;

	// Última guild registrada
	UINT32 LastGuild;

	// Taxa do dia
	// Se estiver 1 quer dizer que a taxa naquele dia
	// já foi alterada pelo líder de guild da cidade
	UINT32 TaxesDay[5];

	// Capa vencedora da guerra de noatun
	INT32 CapeWin; // 4C7D4C
	
	struct
	{
		char msg[108];

		short Hour;
		short Min;

		short Interval;

		short Repeat;

		clock_t Time;

		bool InThisHour;
	} Messages[MaxMessage];
	
	std::vector<Scheduled> Scheduled;

	struct
	{
		st_Item Target[5];
		INT16 Rate[5];
		INT32 Source;
	} Treasure[8];

	INT32 CastleState;
	INT32 CastleHour{ 21 };

	INT32 AltarId;
	INT32 LiveTower[3];
	INT32 NewbieEventServer; // 8BF17C0
	INT32 BRState;// 8BF185C
	INT32 BRItem; // 8BF1828
	INT32 BRMode; // 8BF1864
	INT32 BRGrid; // 8BF1868
	INT32 BRHour; // 4CBC40
	
	INT32 GuildDay; // 4CBC34
	INT32 GuildHour; // 4CBC38
	INT32 NewbieHour; // 4CBC3C

	INT32 ColoState; // 4CBC2C
	INT32 Colo150Limit; // 8BF180C

	INT32 PotionReady; // 8BF1810
	
	INT32 ForceWeekDay; // 4CBC30
	INT32 WeekMode; // 4CBC28
	INT32 WeekHour{ 20 };

	std::array<std::vector<ChallengeInfo>, 5> ChallengerMoney;

	INT32 Weather; //8BF181C
	INT32 ForceWeather; //004CBBE0

	INT32 Staff;

	INT32 ServerTime;
	INT32 LastTime;

	INT32 Encode; // 00BAF180 -> 7556

	struct
	{
		INT32 Hour{ 21 };

		INT32 Points[2];
		INT32 Status; // 1 = online
		INT32 Bonus; // 1 = blue, 2 = red
		INT32 Annoucement_Point;
		INT32 Annoucement;

		std::array<TOD_RvRStore_Item, 27> Items;
	} RvR;

	//stTowerWar TowerWar[10];

	Quests QuestsBosses[5];
	
	st_Position AlcateiaPos[128];
	st_Position AloneWolfPos[128];

	int TotalAlcateiaPos;
	int TotalAloneWolfPos;

	int StatusDaily;

	struct
	{
		TOD_Colosseum_Type Type;

		// Level em que se encontra o Coliseu
		WORD level;
		// Tempo restante para o nível 
		WORD time;
		// NPCID do 'Vitor Gay'
		WORD npc;

		struct 
		{
			// Portões já foram fechados
			bool closedGate;
			// Muralhas desceram
			bool closedWalls;

			// Mobs do level indicado já nasceram ou não
			bool wasBorn;

			// Se o usuário está no boss do nível
			bool boss;
		} inSec;

		std::vector<unsigned int> clients;
	} Colosseum;

	std::vector<TOD_MissionInfo> Missions;

	struct 
	{
		INT32 Status;

		// Membros registrados
		INT32 Registered[MAX_PLAYER];

		// Contador dentro da quest ^^ 
		INT32 Counter; 
	} Zombie;

	struct
	{
		INT32 Status;
		INT32 Guild;
		INT32 Hour{ 22 };
	} TowerWar;

	struct
	{
		st_Item item;

		INT32 Count;

		// Status do evento
		INT32 Status;

		// Taxa de drop
		INT32 Rate;

		// Bonus por possuir XX buff
		INT32 Bonus;
	} AutoTradeEvent;

	struct
	{
		st_Item item;
		INT32 Count;
		INT32 Status;
		INT32 Rate;
		INT32 Bonus;
	} BossEvent;
	
	struct
	{
		st_Item Item;

		INT32 Gold;
		INT32 Last;
		INT32 Second;
		INT32 Interval;
	} PremiumTime;

	struct
	{
		st_Item item[MAX_PREMIERSTORE];
		INT32 Price[MAX_PREMIERSTORE];
	} PremierStore;

	struct
	{
		st_Item item[MAX_PREMIERSTORE];
		INT32 Price[MAX_PREMIERSTORE];
	} ArenaStore;

	std::vector<stLevelItem> levelItem;

	struct
	{
		st_Item Item[7][4];
	} Daily;

	struct
	{
		std::vector<TOD_AreaToDrop> areas;

		bool Message;
		bool Status;
	} DropArea;

	struct
	{
		std::array<TOD_KingdomBattle, 2> Info;
		int Winner;
	} KingdomBattle;

	bool NewRandomMode{ true };

	std::vector<TOD_QuizQuestions> QuizQuestions;

	struct
	{
		int Count;
		int MobId;

		int TotalToKill{ 500 };
	} LanHouseN;

	struct
	{
		int Count;
		int MobId;

		int TotalToKill{ 500 };
	} LanHouseM;

	std::recursive_mutex logMutex;

	struct
	{
		std::vector<int> itemsOnGround;
		std::vector<TOD_ChristmasMissionInfo> Missions;
	} Christmas;

	struct
	{
		std::vector<int> HoursAllowed;
		int MaxKills;
		
		ArenaReward WinnerRewards;
		ArenaReward TopKillRewards;
		ArenaReward ParticipantRewards;
	} Arena;


	std::vector<InfoCache<SealInfo>> SealCache;

	int AnnubisBonus;
	struct
	{
		std::chrono::time_point<std::chrono::steady_clock> StartTime;
		UINT32 Clear;
		bool IsOperating;

		std::vector<unsigned short> Users;

		void Unregister(unsigned short clientId)
		{
			auto it = std::find(std::begin(Users), std::end(Users), clientId);
			if (it != std::end(Users))
				Users.erase(it);
		}
	}Zakum;

	int RunesPerSanc{ 1 };
	unsigned int MaximumPesaLevel{ 120 };

	bool PromotionStatus{ false };
	int MaxWaterEntrance{ 1080 };
	int CliVer{ 6999 };

	struct 
	{
		std::vector<TOD_OverStore_Category> Categories;
	} DonateStore;

	std::array<TOD_MobBoss, MaxBoss> Boss;
} ;

 struct stGameServer
{
	UINT8 Status;
	UINT32 Socket;

	BYTE *sendBuffer;
	BYTE *recvBuffer;

	UINT32 SizeOfData;

	SOCKADDR_IN Service;
	WSADATA wsa;
	
	UINT32 nRecvPosition;
	UINT32 nProcPosition;
    UINT32 Init;
} ;

extern char FailAccount[18][18];

// Responsável pelas variáveis dentro da própria aplicação
extern stServer sServer;
// Responsável pela estrutura que vai armazenar as variáveis
// responsáveis pela GUI do usuário
extern stGUI GUI;
// Responsável pela conexão com o banco de dados
extern stGameServer sData;

extern unsigned int CurrentTime;

// Functions
// ----
//  Responsável pela criação da GUI do usuário
// ----
void CreateGUI();

void DrawConfig();
// ----
//  Responsável pelo processo de análise dos sockets
//  e mensagens da janela
// ----
LONG APIENTRY WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

// ----
// Thread principal
// ----
bool MainThread();

// ----
// ENvio de pacotes para a DBsrv
// ----
bool AddMessageDB(BYTE *pBuffer, UINT32 packetSize);

// ----
// Recebimento de pacoste da DBsrv
// ----
bool PacketControl(BYTE* pBuffer, INT32 size);
char* ReadMessageDB(int*, int*);
BOOL Receive(void);


// Processamento do tempo
void ProcessSendTimer();
void ProcessSecTimer();
void ProcessMinTimer();
void ProcessHourTimer();


void AddFailAcount(char *account);
INT32 CheckFailAccount(char *account);
#endif