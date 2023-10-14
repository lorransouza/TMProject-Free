#include <Windows.h>

#ifndef __ENCDEC_H__
#define __ENCDEC_H__

void PacketEncrypt(BYTE* pBuffer, BYTE* data, INT32 packetSize, BYTE *hashs);
bool PacketDecrypt(BYTE* PacketBuffer);

#endif