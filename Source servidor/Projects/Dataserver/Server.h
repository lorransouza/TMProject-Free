#include <Windows.h>
#include "CPSock.h"
#include "Basedef.h"

#ifndef __SERVER_H__
#define __SERVER_H__

#include "UOD_Log.h"

BOOL ReadServerIP();
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

void ImportCash();
void ImportItem();
void ImportUser();
void ImportPass();
void ImportBan();
void UpdateConnection();
void ProcessSecTimer();
void ReadAdmin();
void ProcessClientMessage(INT32 conn, char *msg);
int GetAdminFromSocket(int Sock);
void SendNotice(int srv, const char *msg, ...);

void Log(const char* msg, ...);
int ProcessAdminMessage(int conn, char *msg);

void DisableAccount(int conn, char *account, int Year, int YearDay);
int SendAdminSignal(int svr, unsigned short id, unsigned short signal);
int SendAdminMessage(int svr, unsigned short id, const char *message);
int SendAdminParm(int svr, unsigned short id, unsigned short signal, int parm);
int SendAdminState(int svr, unsigned short id, unsigned short signal, const char *pass);

extern CPSock Server[MAX_SERVERNUMBER];
extern INT32 UserConnection[MAX_SERVERNUMBER];
extern CUser pUser[MAX_SERVERNUMBER];
extern CUser TempUser;
extern CUser pAdmin[MAX_ADMIN];
extern CPSock AdminClient;
extern std::unique_ptr<TOD_Log> serverLog;
extern CPSock AdminSocket;
extern unsigned int pAdminIP[MAX_ADMIN];
extern unsigned char	LocalIP[4];
#endif