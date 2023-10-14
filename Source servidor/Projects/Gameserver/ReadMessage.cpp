#include "cServer.h"
#include "Basedef.h"
#include "UOD_Log.h"
BOOL CUser::Receive(void)
{
    int Rest = MAX_BUFFER - Socket.nRecvPosition;
	int tReceiveSize = recv(Socket.Socket, (char*)(Socket.recvBuffer + Socket.nRecvPosition), Rest, 0);
    if(tReceiveSize == SOCKET_ERROR)
	{
		Socket.nRecvPosition = 0;
		Socket.nProcPosition = 0;

		if (User.Username[0])
			Log(clientId, LOG_INGAME, "Falha ao tentar receber o pacote. Recv fail. ErrorCode: %d", WSAGetLastError());

		return FALSE;
	}
 
    if(tReceiveSize == Rest)
		return FALSE;

    Socket.nRecvPosition = Socket.nRecvPosition + tReceiveSize;
    return TRUE;
}

BOOL CUser::AcceptUser(int ListenSocket)
{
	SOCKADDR_IN acc_sin;
	int Size = sizeof(acc_sin);

	int tSock = accept(ListenSocket, (struct sockaddr FAR *)&acc_sin, (int FAR *)&(Size));

	if (tSock < 0)
	{
		Status = USER_EMPTY;
		return FALSE;
	}
	if (WSAAsyncSelect(tSock, GUI.hGUI, WSA_READ, FD_READ | FD_CLOSE) > 0)
	{
		closesocket(tSock);

		return FALSE;
	}

	Socket.Socket = tSock;
	Socket.nRecvPosition = 0;
	Socket.nProcPosition = 0;
	Socket.nSendPosition = 0;
	Status = USER_ACCEPT;

	Socket.recvBuffer = new BYTE[MAX_BUFFER];
	Socket.sendBuffer = new BYTE[MAX_BUFFER];
	strncpy_s(IP, inet_ntoa(acc_sin.sin_addr), 16);

	return TRUE;
}

char* CUser::ReadMessage(int *ErrorCode, int* ErrorType)
{
    *ErrorCode = 0;
 
    if(Socket.nProcPosition >= Socket.nRecvPosition)
    {
        Socket.nRecvPosition = 0;
        Socket.nProcPosition = 0;

		return NULL;
    }
 
	if(Socket.Init == 0)
    {
        if((Socket.nRecvPosition - Socket.nProcPosition) < 4)
			return NULL;
 
		int HelloPacket = *(int*)(Socket.recvBuffer + Socket.nProcPosition);
        if(HelloPacket != INITCODE)
        {
			*ErrorCode = 2;
			*ErrorType = HelloPacket;

			CloseUser(clientId);
			return NULL;
        }
 
        Socket.Init = TRUE;
        Socket.nProcPosition = Socket.nProcPosition + 4;
    }
 
	if ((Socket.nRecvPosition - Socket.nProcPosition) < 12)
	{
		if (User.Username[0])
			Log(clientId, LOG_INGAME, "Posição nRecvPosition - nProcPosition é inválida. Valor %d", (Socket.nRecvPosition - Socket.nProcPosition));

		return NULL;
	}
 
	unsigned short Size = *((unsigned short*)(Socket.recvBuffer + Socket.nProcPosition));
	unsigned short CheckSum = *((unsigned char*)(Socket.recvBuffer + Socket.nProcPosition + 2));
 
    if(Size > MAX_MESSAGE_SIZE || Size < sizeof(PacketHeader))
    {
        Socket.nRecvPosition = 0;
        Socket.nProcPosition = 0;
       
		*ErrorCode   =  2;
        *ErrorType   =  Size;

		if (User.Username[0])
			Log(clientId, LOG_INGAME, "Enviado pacote maior que o máximo ou menor que o Header. Size: %d", Size);

        return NULL;
    }
 
    auto Rest = Socket.nRecvPosition - Socket.nProcPosition;
	if (Size > Rest)
	{
		if (User.Username[0])
			Log(clientId, LOG_INGAME, "O pacote recebido esperava tamanho %d e recebemos %d", Size, Rest);

		return NULL;
	}
 
	char*pMsg = (char*)&(Socket.recvBuffer[Socket.nProcPosition]);
    Socket.nProcPosition = Socket.nProcPosition + Size;
    if(Socket.nRecvPosition <= Socket.nProcPosition)
    {
        Socket.nRecvPosition = 0;
        Socket.nProcPosition = 0;
    }
    unsigned int i;
    int pos, Key;
    int sum1 = 0, sum2 = 0;
    pos = KeyTable[CheckSum * 2];
    for(i = 4; i < Size; i++, pos++)
    {
        sum1 += pMsg[i];
        Key = KeyTable[((pos & 0xFF) * 2) + 1];
        switch(i & 3)
        {
            case 0:
                    Key <<= 1;
                    Key &= 255;
                    pMsg[i] -= Key;
                    break;
            case 1:
                    Key >>= 3;
                    Key &= 255;
                    pMsg[i] += Key;
                    break;
            case 2:
                    Key <<= 2;
                    Key &= 255;
                    pMsg[i] -= Key;
                    break;
            case 3: default:
                    Key >>= 5;
                    Key &= 255;
                    pMsg[i] += Key;
                    break;
        }
        sum2 += pMsg[i];
    }
 
    sum2 &= 255;
    sum1 &= 255;

	if (sum2 != sum1)
	{
		*ErrorCode = 1;
		*ErrorType = Size;
	}
    return pMsg;
}
/*
bool CUser::ReadMessage()
{
	// Pega o pacote
	int len = recv(Socket.Socket, (char*)Socket.recvBuffer, MAX_BUFFER, 0);

	// Caso seja algo não válido, simplesmente retorna
	if (len <= 0)
		return false;

	// Caso seja o Hello World, tratamento especial
	if (len == 4 || len == sizeof(p20D) + 4)
	{
		if (len == sizeof(p20D)+4)
		{
			memcpy(&Socket.recvBuffer[0], &Socket.recvBuffer[4], sizeof p20D);
			len = 116;

			Socket.Hello = HELLO;
		}
		// Caso esteja esperando o hello
		else if (Socket.Hello == WAITING_HELLO)
			Socket.Hello = HELLO;
		else
		{
			// Caso não esteja e tenha recebido o hello, então
			// fecha a conexão
			//CloseGame();

			return false;
		}
	}

	// Checa se está realmente conectado
	if (Socket.Hello != HELLO)
		return false;

	BYTE* ptr = Socket.recvBuffer;
	INT32 lastSize = 0;
	PacketHeader *Header = (PacketHeader*)ptr;

	for (; len > 0; len -= lastSize)
	{
		if (len < 12)
		{
			// TODO : Log de erro
			break;
		}

		if (len > MAX_BUFFER)
		{
			// TODO : Log de erro
			break;
		}

		if (len < Header->Size)
		{
			// TODO : Log de erro
			break;
		}

		PacketDecrypt((BYTE*)Header);

		BYTE hash = GetHashKey();
		if (hash == Header->Key || hash == 0)
		{
			if(CheckPacket(Header))
			{
				bool success = PacketControl((BYTE*)Header, len);
				if (!success)
				{
				}
			}
			else
			{
				AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

				Log(clientId, LOG_HACK, "CheckPacket Fail: %X - %d", Header->PacketId, Header->Size);
			}
		}
		else
		{
			Log(clientId, SERVER_SIDE, "HashKey inválida - %X : %X", hash, Header->Key);

			CloseUser(clientId);
		}

		lastSize = Header->Size;
		ptr += lastSize;

		Header = (PacketHeader*)ptr;
	}

	return true;
}*/