#include "Server.h"
#include "CUser.h"

BOOL CUser::AcceptUser(int ListenSocket,int wsa)
{
     SOCKADDR_IN acc_sin;  
	 int Size = sizeof( acc_sin );

	 int tSock = accept( ListenSocket, (struct sockaddr FAR *)&acc_sin, (int FAR *)&(Size) );
     if (tSock<0)  
	 {  return FALSE;
	 }

 	 if (WSAAsyncSelect( tSock, GUI.hGUI, wsa , FD_READ | FD_CLOSE ) > 0) 
   	 {  closesocket(tSock);
	    return FALSE;
	 }
 
	 Sock.Sock          = tSock;
	 Sock.nRecvPosition = 0;
	 Sock.nProcPosition = 0;
	 Sock.nSendPosition = 0;

	 char * tmp = inet_ntoa(acc_sin.sin_addr);
	 IP		= Sock.IP	= acc_sin.sin_addr.S_un.S_addr;
	 Mode	= USER_ACCEPT;

	 return TRUE;
}
