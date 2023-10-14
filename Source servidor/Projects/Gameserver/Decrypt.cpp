#include "cServer.h"
#include "Basedef.h"

bool PacketDecrypt(BYTE* PacketBuffer)
{
	PacketHeader* Header = (PacketHeader*)PacketBuffer;

	UINT32 KeyResult = NULL;
	UINT32 KeyIncrement = KeyTable[(Header->Key & 255) * 2];

	for (UINT32 i = 4; i < Header->Size; i++, KeyIncrement++)
	{
		KeyResult = KeyTable[((KeyIncrement & 0x800000FF) * 2) + 1];
		switch (i & 3)
		{
		case 00:
			*(UINT8*)((UINT32)PacketBuffer + i) -= KeyResult << 1;
			break;
		case 01:
			*(UINT8*)((UINT32)PacketBuffer + i) += KeyResult >> 3;
			break;
		case 02:
			*(UINT8*)((UINT32)PacketBuffer + i) -= KeyResult << 2;
			break;
		case 03:
			*(UINT8*)((UINT32)PacketBuffer + i) += KeyResult >> 5;
			break;
		}
	}

	// TODO :Falta fazer a checagem do checksum
	return true;
}