#include "cServer.h"
#include "Basedef.h"

void PacketEncrypt(BYTE* pBuffer, BYTE* data, INT32 packetSize, BYTE *hashs)
{
	PacketHeader* pHeader = (PacketHeader*)pBuffer;

	INT32 checkSum[2] = { 0, 0 };

	UINT8 hashKey = 0;
	if(hashs == NULL)
		hashKey = KeyTable[Rand() % 512];
	else
		hashKey = hashs[Rand() % 16];

	pHeader->Size = (INT16)packetSize;
	pHeader->Key = hashKey;
	pHeader->TimeStamp = GetTickCount();

	INT32 keyIncrement = KeyTable[(hashKey & 255) * 2];

	for (INT32 i = 4; i < packetSize; i++, keyIncrement++)
	{
		INT32 keyResult = KeyTable[(((keyIncrement & 255) & 0x800000FF) * 2) + 1];

		switch (i & 3)
		{
		case 00:
		{
				   pBuffer[i] = data[i] + ((keyResult & 255) << 1);
				   break;
		}
		case 01:
		{
				   pBuffer[i] = data[i] - ((keyResult & 255) >> 3);
				   break;
		}
		case 02:
		{
				   pBuffer[i] = data[i] + ((keyResult & 255) << 2);
				   break;
		}
		case 03:
		{
				   pBuffer[i] = data[i] - ((keyResult & 255) >> 5);
				   break;
		}
		}

		checkSum[0] += data[i];
		checkSum[1] += pBuffer[i];
	}

	pHeader->CheckSum = ((checkSum[1] & 255) - (checkSum[0] & 255)) & 255;
}