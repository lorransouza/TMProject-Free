#pragma once
#include <stBase.h>

class stServer
{
public:
	int SecCounter;
	int MinCounter;
	int HourCounter;

	int CurrentTime;
	int LastSendTime;

	int ServerIndex;

	uint32_t Sapphire;

	int LastSeal{ 1 };
	int LastMerida{ 1 };
};

struct STRUCT_GUI_INFO
{
	HWND hGUI,
		hButton[1],
		hLabel[1];

	HFONT Font;
	ATOM Class;
};
void		GetFirstKey(const char* source, char* dest);
int			Rand();

int			WriteAccount(char* file, stAccount Pointer);
int			WriteChar(char* file, stCharInfo Pointer);
int			ReadAccount(char* file, stAccount* Pointer);
int			ReadChar(char* file, stCharInfo* Pointer);
void		ClearItem(st_Item* item);
void		ClearMob(st_Mob* player);

double		TimeRemaining(int dia, int mes, int ano, int hora, int min);

INT32		ReadConfig();
INT32		WriteConfig();
INT32		ReadCharBase(int index, const char* file);
INT32		InitializeBaseDef();
INT32		InitializeServerList();

INT32		GetUserFromSocket(INT32 soc);
void		ProcessClientMessage(INT32 conn, char* msg);

extern STRUCT_GUI_INFO GUI;

extern		stServer			cServer;
extern		CUser				pUser[MAX_SERVERNUMBER];
extern		CPSock				Server[MAX_SERVERNUMBER];

extern		unsigned char		LocalIP[4];

extern		char				g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];
extern		INT32				ChargedGuildList[MAX_SERVERGROUP][5];


extern		st_Mob				pBaseSet[4];


