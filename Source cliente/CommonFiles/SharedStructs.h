 #pragma once
/*
Este header, ira compartilhar as structs entre servidor e client a medida que o mesmo for sendo atualizado
outros headers tamb�m poder�ser criado como de constantes por exemplo.
*/

#ifndef _SHAREDSTRUCTS_ // Last updated 27/01/2013

#define _SHAREDSTRUCTS_

#define _MSG      	short		  Size;			\
					char		  KeyWord;		\
					char		  CheckSum;		\
					short		  Type;			\
					short		  ID;			\
					unsigned int  ClientTick;

struct MSG_HSTANDARD
{
	short Size;
	char KeyWord;
	char CheckSum;
	short Type;
	short ID;
	int Tick;
};

#define _PCKHEADER MSG_HSTANDARD Header

#ifdef _WYDCLIENT
typedef struct		
{
	WORD Size;
	BYTE KeyWord;
	BYTE CheckSum;
	WORD Type;
	WORD ID;
	DWORD Tick;
} MSG_STANDARD;
#endif // _WYDCLIENT
struct		  MSG_AccountLogin
{
	MSG_STANDARD Header;

	char AccountPassword[12];
	char AccountName[16];
	char Zero[52];
	int  ClientVersion;
	int  DBNeedSave;
	int AdapterName[4];

};
#pragma pack(push, 1)

#pragma pack(pop)

#endif