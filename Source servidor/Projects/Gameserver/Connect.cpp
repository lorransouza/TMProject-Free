#include "cServer.h"
#include "Basedef.h"

bool ConnectDB()
{
	bool Sucessful = true;
	
	SOCKADDR_IN local_sin; // Bind local IP

	// Em processo de conexão
	sData.Status = IN_PROCCESS_CONNECT;

	sData.Service.sin_family = AF_INET;
	sData.Service.sin_addr.s_addr =  inet_addr(sServer.Data.IP);
	sData.Service.sin_port = htons(sServer.Data.Porta);

	if ((sData.Socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		MessageBoxA(0, "socket error", "Server::socket", 4096);
		Sucessful = false;
	}
	
	SOCKADDR_IN localinfo{};
	localinfo.sin_family = AF_INET;
	localinfo.sin_addr.s_addr = inet_addr(sServer.Config.IP);
	localinfo.sin_port = htons(0);

	if (bind(sData.Socket, (struct sockaddr FAR*) & localinfo, sizeof(localinfo)) == SOCKET_ERROR)
	{
		MessageBox(GUI.hGUI, "Binding fail", "ERROR", MB_OK);
		closesocket(sData.Socket);

		return FALSE;
	}

	if (connect(sData.Socket, (SOCKADDR*)&sData.Service, sizeof sData.Service) == SOCKET_ERROR)
	{
		MessageBoxA(0, "connect error", "Server::connect", 4096);
		Sucessful = false;
	}
	else if (WSAAsyncSelect(sData.Socket, GUI.hGUI, WSA_READDB, FD_READ | FD_CLOSE) == SOCKET_ERROR)
	{
		MessageBoxA(0, "WSAAsyncSelect error", "Server::WSAAsyncSelect", 4096);
		Sucessful = false;
	}

	if (!Sucessful)
	{
		WSACleanup();
		return Sucessful;
	}

	BYTE Hello[4];
	*(int*)&Hello = 0x1F11F311;

	// Caso envie o pacote corretamente, o mesmo será setado
	// Como ESPERANDO_HELLO_PACKET
	if ((send(sData.Socket, (char*)&Hello, 4, 0)) == 4)
	{
		sData.Status = WAITING_HELLO;

		sData.recvBuffer = new BYTE[MAX_BUFFER];
		sData.sendBuffer = new BYTE[MAX_BUFFER];

		printf("Conectado com sucesso na DBSRV\n");
	}

	return Sucessful;
}
