#include "cServer.h"

stGUI GUI;
stServer sServer;
stGameServer sData;
char FailAccount[18][18];
unsigned int CurrentTime;


void AddFailAcount(char *account)
{
	for(INT32 LOCAL_1 = 0; LOCAL_1 < 18; LOCAL_1 ++)
	{
		if(!FailAccount[LOCAL_1][0])
		{
			strncpy_s(FailAccount[LOCAL_1], account, 18);

			break;
		}
	}
}

INT32 CheckFailAccount(char *account)
{
	INT32 LOCAL_1 = 0,
		  LOCAL_2 = 0;

	for(; LOCAL_2 < 18; LOCAL_2 ++)
	{
		if(FailAccount[LOCAL_2][0] && !strncmp(account, FailAccount[LOCAL_2], 18))
			LOCAL_1++;
	}

	return LOCAL_1;
}