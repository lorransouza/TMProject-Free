#ifndef AntiBotPacketTestH
#define AntiBotPacketTestH

#include "AntiBotPacketGenerator.h"

struct TOD_AB_PacketTest_Structure : public IOD_AB_PacketBase
{
	int timer;
};

struct TOD_AB_PacketTest_Response_Structure : public IOD_AB_PacketBase
{
	int timer;
};

class TOD_AB_PacketTest : public IOD_AB_PacketGenerator
{
public:
	TOD_AB_PacketTest(unsigned short packetId)
		: IOD_AB_PacketGenerator(packetId)
	{
	}

	virtual void Generate() ;
	virtual bool Check(const CUser& user, const PacketHeader* packet);
	virtual PacketHeader* GetPacket() const;
};

#endif