#include <Windows.h>

#ifndef __CUSER_H__
#define __CUSER_H__

#include "stBase.h"
#include "cServer.h"
#include "AntiBot.h"
#include "UOD_GUID.h"
#include "UOD_Log.h"

#include <chrono>
#include <array>
#include <mutex>
#include <memory>
#include <vector>
#include <tuple>
#include "../../DataServer/CPSock.h"

using time_point_t = std::chrono::time_point<std::chrono::steady_clock>;

#define USER_EMPTY       0   
#define USER_ACCEPT      1   
#define USER_LOGIN       2   


#define USER_SELCHAR     11  
#define USER_CHARWAIT    12
#define USER_CREWAIT     13   
#define USER_DELWAIT     14   

#define USER_PLAY        22 
#define USER_SAVING4QUIT 24   

#define USER_CHANGESUB 25

#define HELLO 2

using timepoint = std::chrono::time_point<std::chrono::steady_clock>;

struct DroppedItem
{
	st_Item Item;
	int SlotId;
	timepoint Time;
};

enum class TOD_ChristmasMission_Status
{
	Accepted,
	WaitingAnswer,
	WaitingNextRound
};

struct TOD_ChristmasUserInfo
{
	struct
	{
		// Último box que pegou, podendo pegar somente dentro de um tempo específico
		timepoint LastBox;
	} Tree;

	struct
	{
		TOD_ChristmasMission_Status Status{ TOD_ChristmasMission_Status::WaitingNextRound };
		int MissionId{ -1 } ;

		std::array<int, 3> Count;
		timepoint LastUpdate;

		std::chrono::milliseconds NextMission;
	} Mission;

	int HitsOnBoss;
};

class CUser
{
public:
	CUser();
	~CUser();
	
	stAccount User;

	struct
	{
		INT32 Socket;

		BYTE *sendBuffer;
		BYTE *recvBuffer;


		INT32 SizeOfSend;

		INT32 nRecvPosition;
		INT32 nProcPosition;
		INT32 nSendPosition; // 12 - 15
		INT32 nSentPosition; // 24 - 27

		UINT32 Init;

		INT32 Error;
	} Socket;
	
	struct
	{
		// Última vez solicitado
		INT32 Last;

		// Próxima vez que irá ser solicitado
		INT32 Next; 

		// Quantidade de erros
		INT32 Error;

		// O que foi solicitado ao servidor
		INT32 Question;

		// Se precis ou não ser respondido
		INT32 Response;
	} aHack;

	INT32 clientId;
	INT32 Status;
	INT32 LastWhisper;
	INT32 hashIncrement; 
	INT32 CharLoginTime; // regen mob
	INT32 Time; // time real online
	INT32 AttackCount;
	bool IsBanned;

	UINT32 IllusionTime;

	struct 
	{
		INT32 TorreErion; 
	} Damage;

	BYTE Keys[16];

	BOOL IsAutoTrading;
	BOOL IsAdmin;

	char AutoTradeName[24];
	char IP[32];
	char SNDMessage[96];
	time_t LastReceive;

	struct
	{
		INT8 CharSlot; // Char em que o usuário está logado
		INT8 incorrectNumeric; //
		BOOL canEnter;  // Usuário já acertou a senha numérica?
		INT32 MagicIncrement;
	} inGame;

	struct
	{
		int Dropped;
		bool IsValid;
	} DropEvent;

	p383 Trade;

	struct
	{

		INT32 Whisper; // 2636
		INT32 Guild; // 2640
		INT32 Citizen; // 2644
		INT32 Chat; // 2648
		INT32 Kingdom;

		INT8 AutoTradeName[24]; // 2652

		INT32 PK; // 2676
	} AllStatus; 

	struct
	{
		PacketHeader Header; // 1592
		char Name[24]; // 1604
		st_Item Item[12]; // 1628
		INT8 Slots[12]; // 1724
		INT32 Price[12]; // 1736
		INT16 Unknown_1784; // 1784
		INT16 Index; // 1786
	} AutoTrade; 

	struct
	{
		time_point_t LastTime;
		std::chrono::milliseconds TimeToWin;
		bool IsValid;

	} EventAutoTrade;
	
	struct
	{
		int Kills;
		size_t GroupIndex;
	} Arena;
	 INT32 nTargetX; // 2628
	 INT32 nTargetY; // 2632

	struct
	{
		UINT32 PacketId;
		UINT32 TimeStamp;
	} Movement;

	struct
	{
		unsigned int Logout;
		unsigned int Recall;
		unsigned int Restart;
	} Movement7556;

	struct
	{
		INT16 PositionX[4];
		INT16 PositionY[4];

		char Name[4][16];

		st_Status Status[4];
		st_Item Equip[4][18];

		UINT32 GuildIndex[4];

		int Gold[4];
		INT64 Exp[4];
	} CharList;
	
	struct
	{
		INT32 CountHp; // 2680
		INT32 CountMp; // 2684

		INT32 bQuaff; // 2688;
	} Potion;

	struct
	{

		INT32 Mode; // 2708
		INT32 Index; // 2712

		INT32 Type; // 2716 - confirmar.
	} Challenger; 

	struct
	{
		bool IsChecked;
		mutable bool WasWarned;

		time_point_t loginTime;
	} MacIntegrity;

	time_point_t LastDuel;
	time_point_t LastTrade;

	bool TokenOk;
	INT32 Range;

	INT32 AccountID;
	INT32 AccessLevel;

	INT32 CrackCount;

	INT32 TimerCount; // 7B32380

	INT32 Gold;
	INT32 GoldCount;

	struct
	{
		INT32 LastReceiveTime;
		INT32 LastAttack;
		INT32 TimeStamp;

		std::array<time_point_t, 255> Skills;
	} TimeStamp;

	unsigned char MacAddress[8];

	struct 
	{
		// Se está valendo o fucking premio
		INT32 Status;

		// O tempo que já rolou o bagulhete!!
		INT32 Time;

		// Tempo de espera para reabrir a loja
		INT32 Wait;

		INT32 Count;
	} PremierStore;

	struct
	{
		std::array<st_Item, 10> Items;
		unsigned int LastIndex;
	} Repurchase;

	struct
	{
		std::array<DroppedItem, 10> Items;

		unsigned int LastIndex;
	} Dropped;

	struct
	{
		time_point_t LastDeletedItem;
		time_point_t LastUsedItem;

		time_point_t LastLanHouseA;
		time_point_t LastLanHouseM;
		time_point_t LastLanHouseN;
	} Times;

	struct
	{
		std::string Password;

		bool EnableAll{ true };
		std::array<std::string, 15> Nicknames;
	} AutoParty;

	TOD_ChristmasUserInfo Christmas;
	timepoint citizenChatTime;
	
	std::recursive_mutex messageMutex;
	std::recursive_mutex logMutex;

	mutable timepoint lastLogEquipAndInventory;

	void LogEquipsAndInventory(bool force) const;
	std::string LogSameAccounts() const;
	void GenerateNewAutoTradeTime();

	TUOD_AB_UserInfo ABInfo{};

	std::vector<std::string> invitedUsers;

	std::tuple<bool, std::chrono::milliseconds> CheckIfIsTooFast(const st_Item* item, int slotId);
	std::unique_ptr<TOD_Log> NormalLog;
	std::unique_ptr<TOD_Log> HackLog;
	int SummonedUser;
	unsigned int LastTimeRequestDrop;
	// Quest registrada no momento
	INT32 QuestAccess;
	// Quando equipar o ovo, seta como 60
	INT32 WolfEggEquipedTime;
	INT32 WolfEquipedTime;
	INT32 WolfTotalTime;
	INT32 AlphaPotionRewardCounter;

	BOOL Receive(void);
	void RefreshSendBuffer();
	char* ReadMessage(int *ErrorCode, int* ErrorType);
	bool AddMessage(BYTE* pBuffer, UINT32 packetSize);
	bool PacketControl(BYTE*, INT32 size);
	BOOL SendOneMessage(BYTE *packet, INT32 size);
	BOOL AcceptUser(int ListenSocket);
	BOOL SendMessageA(void);
	byte GetHashKey();

	// Functions
	//bool CloseUser(clientId);
	bool CloseUser_OL1();
	bool CloseSocket();

	// Recv Functions
	bool RequestLogin(PacketHeader *Header);
	bool RequestCreateChar(PacketHeader *Header);
	bool Msg_Numeric(PacketHeader* Header);
	bool RequestAction(PacketHeader *Header);
	bool RequestChat(PacketHeader *Header);
	bool RequestCommand(PacketHeader *Header);
	bool RequestTeleport(PacketHeader *Header);
	bool RequestMoveItem(PacketHeader *header);
	bool RequestCreateAutoTrade(PacketHeader *Header);
	bool RequestBuyAutoTrade(PacketHeader *Header);
	bool RequestOpenAutoTrade(PacketHeader *Header);
	bool RequestCloseAutoTrade(PacketHeader *Header);
	bool RequestChangeCity(PacketHeader *Header);
	bool RequestAddPoint(PacketHeader *Header);
	bool RequestDeleteChar(PacketHeader* header);
	bool RequestOpenShop(PacketHeader *Header);
	bool RequestBuyShop(PacketHeader *Header);
	bool RequestAttack(PacketHeader *Header);
	bool RequestDropItem(PacketHeader *Header);
	bool RequestAddParty(PacketHeader *Header);
	bool RequestAcceptParty(PacketHeader *Header);
	bool RequestExitParty(PacketHeader *Header);
	bool RequestOpenGate(PacketHeader *Header);
	bool RequestRessuctPlayer(PacketHeader *Header);
	bool RequestChangeSkillbar(PacketHeader *Header);
	bool RequestSellShop(PacketHeader *Header);
	bool RequestDeleteItem(PacketHeader *Header);
	bool RequestPickItem(PacketHeader *Header);
	bool RequestMotion(PacketHeader *Header);
	bool RequestTrade(PacketHeader *Header);
	bool RequestTransferGoldToBank(PacketHeader *Header);
	bool RequestTransferGoldToInv(PacketHeader *Header);
	bool RequestDuel(PacketHeader *Header);
	bool RequestUseItem(PacketHeader *Header);
	bool RequestMerchantNPC(PacketHeader *Header);
	bool RequestUngroupItem(PacketHeader *Header);
	bool RequestCompounder(PacketHeader *Header);
	bool RequestEhre(PacketHeader *Header);
	bool RequestShany(PacketHeader *Header);
	bool RequestAgatha(PacketHeader *Header);
	bool RequestAylin(PacketHeader *Header);
	bool RequestTiny(PacketHeader *Header);
	bool RequestLindy(PacketHeader *Header);
	bool RequestOdin(PacketHeader *Header);
	bool RequestRecruitMember(PacketHeader *Header);
	bool RequestDeclareWar(PacketHeader *Header);
	bool RequestKickMember(PacketHeader *Header);
	bool RequestAlly(PacketHeader *Header);
	bool RequestGriffinMaster(PacketHeader *Header);
	bool RequestFriendListUpdate(PacketHeader *Header);
	bool RequestReqChallange(PacketHeader *Header);
	bool RequestBuyRvRShop(PacketHeader *Header); 
	bool RequestBuyDonateShop(PacketHeader *Header);
	bool RequestChallange(PacketHeader *Header);
	bool RequestBlockItems(PacketHeader *Header);
	bool RequestDonateShop(PacketHeader *Header);
	bool GetCharInfo(PacketHeader *Header);
	bool ChangeServer(PacketHeader *Header);
	bool RequestExtraction(PacketHeader *Header);
	bool RequestPremierStore(PacketHeader *Header);
	bool RequestPremierStoreBuy(PacketHeader *Header);
	bool RequestClientInfo(PacketHeader *Header);
	bool RequestCollectDaily(PacketHeader *Header);
	bool RequestAlchemy(PacketHeader* Header);
	bool RequestRepurchase(PacketHeader* header);
	bool RequestMsvfw32Info(PacketHeader *header);
	bool HandleModulesInfo(PacketHeader* header);
	bool RequestAutoPartySetPassword(PacketHeader* header);
	bool RequestAutoPartyAddRemoveName(PacketHeader* header);
	bool RequestAutoPartyEnableAll(PacketHeader* header);
	bool RequestAutoPartyEnterParty(PacketHeader* header);
	bool RequestAcceptRejectMission(PacketHeader* header);
	bool RequestCollectMission(PacketHeader* header);
	bool RequestPutOutSeal(PacketHeader* header);

	bool RequestSealInfo(PacketHeader* header);
	bool HandleMacAddressIntegrity(PacketHeader* header);
	bool RequestEssenceUse(PacketHeader* header);

	bool RequestAcceptRecruit(PacketHeader* header);
	bool RequestNightmareAccept(PacketHeader* header);
	bool RequestRedeemGriffin(PacketHeader* header);
	bool HandleAdminCommand(p334* packet);
	// Send
	bool SendCharList(PacketHeader *Header);	
	bool ResendCharList(PacketHeader *Header);
	bool SendNumericToken(PacketHeader *Header);
	bool SendCharToWorld(PacketHeader *Header);

	bool canConnectionContinue(int error) const;
};

extern std::array<CUser, MAX_PLAYER> Users;

#endif