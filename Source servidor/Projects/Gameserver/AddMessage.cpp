#include "cServer.h"
#include "Basedef.h"

bool CUser::AddMessage(BYTE *packet, UINT32 size)
{
	BYTE *LOCAL_66 = packet;
	
	if(size <= 0)
		return false;

	std::lock_guard<decltype(messageMutex)> mutex{ messageMutex };

	if(Socket.nSendPosition + size > MAX_BUFFER)
	{
		::Log(clientId, LOG_ERROR, "add buffer full -  Atual: %d. Tamanho: %d. PacketID: %X", Socket.nSendPosition, size, *(WORD*)(packet + 4));

		return false;
	}

	if(Socket.Socket <= 0)
	{
		::Log(clientId, LOG_ERROR, "add buffer invalid");

		return false;
	}

	INT32 LOCAL_67 = Rand() % 256; // ebp - 10Ch
	//INT32 LOCAL_68 = KeyTable[(LOCAL_67 & 0xFF) * 2]; // ebp -110
	//INT32 LOCAL_69 = 0; // ebp - 114

	if(!packet)
	{
		::Log(clientId, LOG_ERROR, "add buffer packet invalid");

		return false;
	}

	PacketHeader *header = (PacketHeader*)packet;
	header->Size = size;
	header->Key = LOCAL_67;
	header->CheckSum = 0;

	header->TimeStamp = CurrentTime;
	sServer.LastTime = CurrentTime;

	BYTE *LOCAL_70 = packet;
	//if(!CheckPacket(LOCAL_70))
	// blabla

	INT32 LOCAL_71 = 0; // 11C
	INT32 LOCAL_72 = 0; // 120;
	UINT8 LOCAL_73 = KeyTable[(LOCAL_67 & 255) * 2];

	for(UINT32 LOCAL_74 = 4; LOCAL_74 < size; LOCAL_74++, LOCAL_73++)
	{
		LOCAL_71 += packet[LOCAL_74];

		INT32 LOCAL_75 = LOCAL_73 % 256;
		UINT8 LOCAL_76 = KeyTable[(LOCAL_75 & 255) * 2 + 1]; // - 130
		INT32 LOCAL_77 = LOCAL_74 & 3;

		if(LOCAL_77 == 0)
			Socket.sendBuffer[Socket.nSendPosition + LOCAL_74] =  packet[LOCAL_74] + ((LOCAL_76 & 0xFF) << 1);
		else if(LOCAL_77 == 1)
			Socket.sendBuffer[Socket.nSendPosition + LOCAL_74] =  packet[LOCAL_74] - ((LOCAL_76 & 0xFF) >> 3);
		else if(LOCAL_77 == 2)
			Socket.sendBuffer[Socket.nSendPosition + LOCAL_74] =  packet[LOCAL_74] + ((LOCAL_76 & 0xFF) << 2);
		else if(LOCAL_77 == 3)
			Socket.sendBuffer[Socket.nSendPosition + LOCAL_74] =  packet[LOCAL_74] - ((LOCAL_76 & 0xFF) >> 5);

		LOCAL_72 += Socket.sendBuffer[Socket.nSendPosition + LOCAL_74];
	}

	header->CheckSum = ((LOCAL_72 & 0xFF) - (LOCAL_71 & 0xFF) & 255);

	memcpy(&Socket.sendBuffer[Socket.nSendPosition], packet, 4);

	Socket.nSendPosition += size;
	return true;
}
/*
bool CUser::AddMessage(BYTE* pBuffer, UINT32 packetSize)
{
	INT32 retn = TRUE;
	char error[150];

	PacketHeader* pHeader = (PacketHeader*)pBuffer;

	BYTE* data = Socket.sendBuffer;
	UINT32 size = Socket.SizeOfSend;

	if ((size + packetSize) >= MAX_BUFFER)
	{
		//sprintf_s(error, "err,add buffer full %d %d %d %d", size, packetSize, pHeader->PacketId, socket->Game.Socket);
		//AddError(error, "-system", NULL);

		retn = FALSE;
		goto RETN;
	}

	if (!Socket.Socket)
	{
		//sprintf_s(error, "err,add buffer invalid %d %d %d %d", size, packetSize, pHeader->PacketId, socket->Game.Socket);
		//AddError(error, "-system", NULL);

		retn = FALSE;
		goto RETN;
	}

	if (packetSize < 12 || pHeader->PacketId == 0)
	{
		//sprintf_s(error, "err,add buffer invalid packet %d %d %d %d", size, packetSize, pHeader->PacketId, socket->Game.Socket);
		//AddError(error, "-system", NULL);

		retn = FALSE;
		goto RETN;
	}

	PacketEncrypt(&data[size], pBuffer, packetSize, Keys);

	Socket.SizeOfSend += packetSize;
RETN:
	return retn;
}
*/