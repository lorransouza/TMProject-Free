#include "Basedef.h"
#include "cServer.h"

BOOL Receive(void)
{
    int Rest = MAX_BUFFER - sData.nRecvPosition;
	int tReceiveSize = recv(sData.Socket, (char*)(sData.recvBuffer + sData.nRecvPosition), Rest, 0);
    if(tReceiveSize == SOCKET_ERROR)
            return FALSE;
 
    if(tReceiveSize == Rest)
    {
            //Não entendi o codigo TM 1.2 -> 0041703E
            return FALSE;
    }
 
    sData.nRecvPosition = sData.nRecvPosition + tReceiveSize;
    return TRUE;
}

char* ReadMessageDB(int *ErrorCode, int* ErrorType)
{
    if(sData.nProcPosition >= sData.nRecvPosition)
    {
        sData.nRecvPosition = 0;
        sData.nProcPosition = 0;
        
		return NULL;
    }
 
    if((sData.nRecvPosition - sData.nProcPosition) < 12)
            return NULL;
 
	unsigned short Size = *((unsigned short*)(sData.recvBuffer + sData.nProcPosition));
	unsigned short CheckSum = *((unsigned char*)(sData.recvBuffer + sData.nProcPosition + 2));
 
    if(Size > MAX_MESSAGE_SIZE || Size < sizeof(PacketHeader))
    {
        sData.nRecvPosition = 0;
        sData.nProcPosition = 0;
       
		*ErrorCode   =  2;
        *ErrorType   =  Size;
		Log(SERVER_SIDE, LOG_INGAME, "Size > MAX_MESSAGE_SIZE || Size < 12. Size: %hu", Size);
        return NULL;
    }
 
    unsigned short Rest = sData.nRecvPosition - sData.nProcPosition;
    if(Size > Rest)
		return NULL;
 
	char*pMsg = (char*)&(sData.recvBuffer[sData.nProcPosition]);
    sData.nProcPosition = sData.nProcPosition + Size;
    if(sData.nRecvPosition <= sData.nProcPosition)
    {
        sData.nRecvPosition = 0;
        sData.nProcPosition = 0;
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

    *ErrorCode = 1;
    *ErrorType = Size;
 
    return pMsg;
}