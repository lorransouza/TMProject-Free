#include "cServer.h"
#include "Basedef.h"

bool CreateServer()
{
	bool Sucessful = true;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		MessageBoxA(0, "WSAStartup error", "Server::WSAStartup", 4096);
		Sucessful = false;
	}

	char Temp[256];
	SOCKET tSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (tSock == INVALID_SOCKET)
	{
		MessageBox(GUI.hGUI, "Initialize socket fail", "ERROR", MB_OK);
		return false;
	}

	gethostname(Temp, 256);

	sServer.Service.sin_family = AF_INET;
	sServer.Service.sin_addr.s_addr = inet_addr(sServer.Config.IP);
	sServer.Service.sin_port = htons(sServer.Config.Porta);

	if (bind(tSock, (struct sockaddr FAR *)&sServer.Service, sizeof(sServer.Service)) == SOCKET_ERROR)
	{
		MessageBox(GUI.hGUI, "Binding fail", "ERROR", MB_OK);
		closesocket(tSock);

		return FALSE;
	}

	if (listen(tSock, 8) < 0)
	{
		MessageBox(GUI.hGUI, "Listen fail", "ERROR", MB_OK);
		closesocket(tSock);

		return FALSE;
	}

	if (WSAAsyncSelect(tSock, GUI.hGUI, WSA_ACCEPT, FD_ACCEPT) > 0)
	{
		MessageBox(GUI.hGUI, "WSAAsyncSelect fail", "ERROR", MB_OK);
		closesocket(tSock);

		return FALSE;
	}

	sServer.Socket = tSock;

	return true;
}