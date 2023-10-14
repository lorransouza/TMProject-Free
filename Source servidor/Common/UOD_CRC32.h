/*
** Arquivo: CRC32.h
**
** Data da Criacao: 20-abr-98
** Data da Revisao: 20-abr-98
**
** Autor: Nestor C. Fernandes
**
*/

// ---------------------------------------------------------------------------
#ifndef USTEP_CRC32H
#define USTEP_CRC32H
// ---------------------------------------------------------------------------

extern unsigned long crcTable[256];

class TSTEP_CRC32
{
public:
	TSTEP_CRC32()
	{
		_crc = 0xFFFFFFFF;
	};

	void Reset()
	{
		_crc = 0xFFFFFFFF;
	};

	void Calc(unsigned char uc)
	{
		_crc = ((_crc >> 8) & 0x00FFFFFF) ^ crcTable[(_crc ^ uc) & 0xFF];
	};

	unsigned long GetCRC()
	{
		return (_crc ^ 0xFFFFFFFF);
	};

private:
	unsigned long _crc;
};

unsigned long UOD_EncodeCRC32_String(const char *text);

#endif
