#include <Windows.h>
#include "Basedef.h"
#include "Server.h"

bool CPSock::WSAInitialize()
{
	WSAData WSADATA;
	if (WSAStartup(MAKEWORD(2, 2), &WSADATA) != 0)
		return FALSE;

	return TRUE;
}

bool CPSock::ListenServer(HWND hWnd, int ip, int port, int WSA)
{
	SOCKADDR_IN	local_sin;
	char		Temp[256];
	SOCKET		tSock = socket(AF_INET, SOCK_STREAM, 0);

	if (tSock == INVALID_SOCKET)
	{
		MessageBox(hWnd, "Initialize socket fail", "ERROR", MB_OK);
		return FALSE;
	}

	gethostname(Temp, 256);
	local_sin.sin_family = AF_INET;
	local_sin.sin_addr.s_addr = ip;		//INADDR_ANY;
	local_sin.sin_port = htons((unsigned short int)port);


	if (bind(tSock, (struct sockaddr FAR *)&local_sin, sizeof(local_sin)) == SOCKET_ERROR)
	{
		MessageBox(hWnd, "Binding fail", "ERROR", MB_OK);
		closesocket(tSock);
		return FALSE;
	}
	if (listen(tSock, MAX_PENDING_CONNECTS) < 0)
	{
		MessageBox(hWnd, "Listen fail", "ERROR", MB_OK);
		closesocket(tSock);
		return FALSE;
	}
	if (WSAAsyncSelect(tSock, hWnd, WSA, FD_ACCEPT)>0)
	{
		MessageBox(hWnd, "WSAAsyncSelect fail", "ERROR", MB_OK);
		closesocket(tSock);
		return FALSE;
	}
	Sock = tSock;
	Port = port;
	IP = ip;
	return TRUE;
}

BOOL CPSock::Receive()
{
	int Rest = RECV_BUFFER_SIZE - nRecvPosition;
	int tReceiveSize = recv(Sock, (char*)(recvBuffer + nRecvPosition), Rest, 0);

	if (tReceiveSize == SOCKET_ERROR)
	{
		Log("Falha recv packet. Errorcode: %d", WSAGetLastError());
		return FALSE;
	}

	nRecvPosition += tReceiveSize;
	if (tReceiveSize == Rest)
	{
		Log("Sem espaço suficiente para o pacote completo. Movendo bufer e fazendo um novo recv");
		RefreshRecvBuffer();

		Rest = RECV_BUFFER_SIZE - nRecvPosition;
		tReceiveSize = recv(Sock, (char*)(recvBuffer + nRecvPosition), Rest, 0);
		nRecvPosition = nRecvPosition + tReceiveSize;

		Log("Total lido após o recv: %d", tReceiveSize);
	}

	return TRUE;
}

BOOL CPSock::AddMessage(char *packet, INT32 size)
{
	char *packetPtr = packet;

	if (nSendPosition + size > SEND_BUFFER_SIZE)
	{
		Log("Maximum buffer size. Position: %d. PacketSize: %d", nSendPosition, size);
		return false;
	}

	if (Sock <= 0)
	{
		// Log(add buffer invalid);
		Log("add buffer invalid", nSendPosition, size);
		return false;
	}

	INT32 rnd = Rand() & 0x800000FF; // ebp - 10Ch

	PacketHeader *header = (PacketHeader*)packet;
	header->Size = size;
	header->Key = rnd;
	header->CheckSum = 0;

	header->TimeStamp = sServer.CurrentTime;
	sServer.LastSendTime = sServer.CurrentTime;

	char *packetBuffer = packet;
	//if(!CheckPacket(LOCAL_70))
	// blabla

	INT32 sum1 = 0; // 11C
	INT32 sum2 = 0; // 120;
	INT32 pos = KeyTable[(rnd & 255) * 2];

	for (INT32 i = 4; i < size; i++, pos++)
	{
		sum1 += packet[i];

		INT32 rst = pos & 0x800000FF;
		INT32 trans = KeyTable[(rst & 255) * 2 + 1]; // - 130
		INT32 mod = i & 3;

		if (mod == 0)
			sendBuffer[nSendPosition + i] = packet[i] + ((trans & 0xFF) << 1);
		else if (mod == 1)
			sendBuffer[nSendPosition + i] = packet[i] - ((trans & 0xFF) >> 3);
		else if (mod == 2)
			sendBuffer[nSendPosition + i] = packet[i] + ((trans & 0xFF) << 2);
		else if (mod == 3)
			sendBuffer[nSendPosition + i] = packet[i] - ((trans & 0xFF) >> 5);

		sum2 += sendBuffer[nSendPosition + i];
	}

	header->CheckSum = ((sum2 & 0xFF) - (sum1 & 0xFF) & 255);

	memcpy(&sendBuffer[nSendPosition], packet, 4);

	nSendPosition += size;
	return true;
}

void CPSock::RefreshSendBuffer()
{
	INT32 left = nSendPosition - nSentPosition;
	if (left <= 0)
		return;

	memcpy(sendBuffer, sendBuffer + nSentPosition, left);
	nSentPosition = 0;
	nSendPosition -= left;
}

void CPSock::RefreshRecvBuffer()
{
	int left = nRecvPosition - nProcPosition;
	if (left <= 0)
		return;

	Log("Limpando buffer. Posição atual: %d. Restante: %d", nProcPosition, left);
	memcpy(recvBuffer, recvBuffer + nProcPosition, left);

	nProcPosition = 0;
	nRecvPosition = left;
}

BOOL CPSock::SendOneMessage(char *packet, INT32 size)
{
	AddMessage(packet, size);
	return SendMessage();
}

BOOL CPSock::SendMessage(void)
{
	if (Sock <= 0)
	{
		nSendPosition = 0;
		nSentPosition = 0;

		return false;
	}

	if (nSentPosition > 0)
		RefreshSendBuffer();

	if (nSendPosition > SEND_BUFFER_SIZE || nSendPosition < 0 || Sock < 0)
	{
		nSendPosition = 0;
		nSentPosition = 0;

		Log("Invalid send position. nSendPosition: %d", nSendPosition);
		return false;
	}

	if (nSentPosition > nSendPosition || nSentPosition > SEND_BUFFER_SIZE || nSentPosition < 0)
	{
		Log("Buffer inválido ao tentar enviar pacote. nSendPosition: %d. nSentPosition: %d", nSendPosition, nSentPosition);
		nSendPosition = 0;
		nSentPosition = 0;
	}

	INT32 err = 0;
	for (INT32 i = 0; i < 1; i++)
	{
		INT32 left = nSendPosition - nSentPosition;

		INT32 tSend = send(Sock, sendBuffer + nSentPosition, left, 0);
		if (tSend != -1)
			nSentPosition += tSend;
		else
		{
			err = WSAGetLastError();

			Log("Error on send packet, error: %X", err);
		}
	}

	if (nSentPosition >= nSendPosition || err == -1)
	{
		nSendPosition = 0;
		nSentPosition = 0;

		return 1;
	}

	if (nSendPosition >= SEND_BUFFER_SIZE)
		return false;

	return true;
}

BOOL CPSock::CloseSocket()
{
	nSendPosition = 0;
	nSentPosition = 0;
	nRecvPosition = 0;
	nProcPosition = 0;
	Init = 0;

	if (Sock != 0)
		closesocket(Sock);

	IP = 0;
	Sock = 0;
	return TRUE;
}

bool PacketDecrypt(BYTE* PacketBuffer)
{
	PacketHeader* Header = (PacketHeader*)PacketBuffer;

	INT32 KeyResult = NULL;
	INT32 KeyIncrement = KeyTable[Header->Key * 2];

	for (INT32 i = 4; i < Header->Size; i++, KeyIncrement++)
	{
		KeyResult = KeyTable[((KeyIncrement & 0x800000FF) * 2) + 1];

		switch (i & 3)
		{
		case 00:
			*(INT8*)((UINT32)PacketBuffer + i) -= ((KeyResult & 255) << 1);
			break;
		case 01:
			*(INT8*)((UINT32)PacketBuffer + i) += ((KeyResult & 255) >> 3);
			break;
		case 02:
			*(INT8*)((UINT32)PacketBuffer + i) -= ((KeyResult & 255) << 2);
			break;
		case 03:
			*(INT8*)((UINT32)PacketBuffer + i) += ((KeyResult & 255) >> 5);
			break;
		}

	}


	// TODO :Falta fazer a checagem do checksum
	return true;
}

char* CPSock::ReadMessage(int *ErrorCode, int* ErrorType)
{
	*ErrorCode = 0;

	if (nProcPosition >= nRecvPosition)
	{
		nRecvPosition = 0;
		nProcPosition = 0;

		return NULL;
	}

	if (Init == 0)
	{
		if ((nRecvPosition - nProcPosition) < 4)
			return NULL;

		int HelloPacket = *(int*)(recvBuffer + nProcPosition);
		if (HelloPacket != INITCODE)
		{
			*ErrorCode = 2;
			*ErrorType = HelloPacket;

			return NULL;
		}

		Init = TRUE;
		nProcPosition = nProcPosition + 4;
	}

	if ((nRecvPosition - nProcPosition) < 12)
		return NULL;

	unsigned short Size = *((unsigned short*)(recvBuffer + nProcPosition));
	unsigned short CheckSum = *((unsigned char*)(recvBuffer + nProcPosition + 2));

	if (Size > MAX_MESSAGE_SIZE || Size < sizeof(PacketHeader))
	{
		nRecvPosition = 0;
		nProcPosition = 0;

		*ErrorCode = 2;
		*ErrorType = Size;
		Log("Pacote inválido recebido. %hd > %d ou %hd < %u. Possível id do pacote: 0x%03X", Size, MAX_MESSAGE_SIZE, Size, sizeof PacketHeader, *((unsigned short*)(recvBuffer + nProcPosition + 4)));
		return NULL;
	}

	auto Rest = nRecvPosition - nProcPosition;
	if (Size > Rest)
	{
		Log("Tamanho de pacote esperado: %hu. Tamanho atual: %hu", Size, Rest);

		return NULL;
	}

	char*pMsg = (char*)&(recvBuffer[nProcPosition]);
	nProcPosition = nProcPosition + Size;
	if (nRecvPosition <= nProcPosition)
	{
		nRecvPosition = 0;
		nProcPosition = 0;
	}

	PacketDecrypt((BYTE*)pMsg);
	return pMsg;
}