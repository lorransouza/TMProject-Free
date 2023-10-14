#include "cServer.h"
#include "Basedef.h"

bool AddMessageDB(BYTE *pBuffer, UINT32 packetSize)
{
	INT32 retn = TRUE;
	if (pBuffer == nullptr)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Argumento veio nulo - AddMessageDB");
		return false;
	}

	PacketHeader* pHeader = (PacketHeader*)pBuffer;
	if (pHeader == nullptr)
	{
		Log(SERVER_SIDE, LOG_INGAME, "pHeader nulo ao converter pBuffer");
		return false;
	}
	
	BYTE* data = sData.sendBuffer;
	if (data == nullptr)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Buffer de destino inválido");
		return false;
	}

	UINT32 size = sData.SizeOfData;
	if ((size + packetSize) >= MAX_BUFFER)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Buffer da DB está full. Atual: %d. Tamanho do pacote %d", size, packetSize);
		return false;
	}

	if (!sData.Socket)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Socket da DB está inválido inválido", size, packetSize);
		return false;
	}

	if (packetSize < 12 || pHeader->PacketId == 0)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Pacote inválido, packetSize: %d. PacketId %03X", packetSize, pHeader->PacketId);
		return false;
	}
	
	PacketEncrypt(&data[size], pBuffer, packetSize, NULL);
	sData.SizeOfData += packetSize;

	int ret = send(sData.Socket, (char*)sData.sendBuffer, sData.SizeOfData, 0);

	// Enviado com sucesso
	if (ret == sData.SizeOfData)
		sData.SizeOfData = 0;
	else
		Log(SERVER_SIDE, LOG_INGAME, "Falha ao tentar enviar o pacote para a DBSRV. Erro: %d", WSAGetLastError());

	return true;
}