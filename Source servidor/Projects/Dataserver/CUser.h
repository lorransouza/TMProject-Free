#ifndef __CUSER_H__
#define __CUSER_H__

#include "CPSock.h"

#define RECV_BUFFER_SIZE 65535 * 40
#define SEND_BUFFER_SIZE 65535 * 40
#define MAX_MESSAGE_SIZE 20000
#define INITCODE		 0x1F11F311
#define USER_EMPTY		 0  
#define USER_ACCEPT		 1   
class CUser
{
public:
	INT32 IP; // 0 - 3
	INT32 Mode{ USER_EMPTY }; // 4 - 7

	CPSock Sock; // 8 -
	INT32 Count; // 1168d90
	int    Level;
	int    Encode1;
	int    Encode2;
	char   Name[16];
	int    DisableID;

	int Year;
	int YearDay;
	INT32 Staff{ 1 };

	BOOL AcceptUser(int ListenSocket,int wsa);
};

#endif