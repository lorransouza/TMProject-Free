#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "UOD_CRC32.h"
#include "BufferReaderWriter.h"

bool CUser::HandleModulesInfo(PacketHeader* header)
{
	if (header->Size <= 12 || header->Size >= 65535)
	{
		Log(clientId, LOG_INGAME, "Enviado pacote de modules com size %d",header->Size);
		return true;
	}

	BufferReader reader(reinterpret_cast<unsigned char*>(header), header->Size);
	reader += 12u;

	auto crc32 = reader.Get<unsigned int>();
	std::string modules = reader.Get<std::string>();

	for (auto& c : modules)
		c ^= 0x5B;

	if (UOD_EncodeCRC32_String(modules.c_str()) != crc32)
		Log(clientId, LOG_HACK, "O CRC32 do usuário referente aos módulos está incorreto");

	Log(clientId, LOG_INGAME, modules.c_str());
	return true;
}