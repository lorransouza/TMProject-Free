#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "AntiBotPacketTest.h"

void TOD_AB_PacketTest::Generate()
{
	TOD_AB_PacketTest_Structure packet{};
	GenerateHeader<TOD_AB_PacketTest_Structure>(packet, _packetId);

	packet.timer = GetTickCount();

	_packet = &packet;
}

bool TOD_AB_PacketTest::Check(const CUser& user, const PacketHeader* packet) 
{
	auto sendedPacket = static_cast<TOD_AB_PacketTest_Structure*>(_packet);
	auto receivedPacket = static_cast<TOD_AB_PacketTest_Response_Structure*>(const_cast<PacketHeader*>(packet));

	return true;
}

PacketHeader* TOD_AB_PacketTest::GetPacket() const
{
	return _packet;
}