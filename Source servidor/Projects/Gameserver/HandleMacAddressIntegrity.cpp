#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "UOD_CRC32.h"

bool CUser::HandleMacAddressIntegrity(PacketHeader *header)
{
	_MSG_MACADDRESS_INTEGRITY* p = reinterpret_cast<_MSG_MACADDRESS_INTEGRITY*>(header);

	bool anyError = false;
	char macaddress[32] = { 0 };
	sprintf_s(macaddress, "%02X:%02X:%02X:%02X:%02X:%02X", p->mac[0], p->mac[1], p->mac[2], p->mac[3], p->mac[4], p->mac[5]);

	auto crc32 = UOD_EncodeCRC32_String(std::string(macaddress).c_str());
	if (p->crc32 != crc32)
	{
		Log(clientId, LOG_HACK, "O usuário adulterou o macaddress enviado. CRC32 esperado: %d. CRC32 enviado: %d", crc32, p->crc32);
		Log(SERVER_SIDE, LOG_HACK, "O usuário %s adulterou o macaddress enviado. CRC32 esperado: %d. CRC32 enviado: %d", User.Username, crc32, p->crc32);

		anyError = true;
	}

	if (memcmp(p->mac, MacAddress, 8) != 0)
	{
		Log(clientId, LOG_HACK, "MacAddress diferente do login. Novo macaddress: %02X:%02X:% 02X:%02X:%02X:%02X", p->mac[0], p->mac[1], p->mac[2], p->mac[3], p->mac[4], p->mac[5]);
		Log(SERVER_SIDE, LOG_HACK, "MacAddress %s diferente do login. Novo macaddress: %02X:%02X:% 02X:%02X:%02X:%02X",User.Username, p->mac[0], p->mac[1], p->mac[2], p->mac[3], p->mac[4], p->mac[5]);

		anyError = true;
	}

	if (!anyError)
		Log(clientId, LOG_INGAME, "MacAddress correct");

	MacIntegrity.IsChecked = true;
	return true;
}
