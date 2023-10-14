#ifndef __CPSOCK_H__
#define __CPSOCK_H__

#define MAX_PENDING_CONNECTS 8

#define WSA_READ            (WM_USER+100) 
#define WSA_ACCEPT          (WM_USER+3) 
#define WSA_ACCEPTADMIN     (WM_USER + 5) 
#define WSA_READADMIN		(WM_USER + 6) 
#define WSA_READADMINCLIENT	(WM_USER + 7) 
class CPSock
{
public:
	UINT32 Sock; // 0 - 3

	char *recvBuffer; // 4 - 7
	char *sendBuffer; // 8 - 11
	
	int		nSendPosition; // 12 - 15
	int		nRecvPosition; // 16 - 19
	int		nProcPosition; // 20 - 23
	int		nSentPosition; // 24 - 27
	int     Init;	 // 28 - 31

	INT32 IP;
	UINT32 Port;

	void RefreshSendBuffer(); 
	void RefreshRecvBuffer();
	
	bool WSAInitialize();
	bool ListenServer(HWND hWnd, int ip,int port, int WSA );
	BOOL Receive();
	BOOL AddMessage(char *packet, INT32 size);
	BOOL SendOneMessage(char *packet, INT32 size);
	BOOL SendMessage(void);
	BOOL CloseSocket();

	char * ReadMessage(int *ErrorCode, int* ErrorType);
};

#endif