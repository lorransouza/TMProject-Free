#ifndef AntiBotPacketGeneratorH
#define AntiBotPacketGeneratorH

#include <type_traits>
class CUser;
struct PacketHeader;

struct IOD_AB_PacketBase : public PacketHeader
{
};

template<typename T>
void GenerateHeader(T& packet, unsigned short packetId)
{
	packet.PacketId = packetId;
	packet.Size = sizeof(T);
}

class IOD_AB_PacketGenerator
{
protected:
	unsigned short _packetId;
	IOD_AB_PacketBase* _packet;

	IOD_AB_PacketGenerator(unsigned short packetId)
		: _packetId(packetId)
	{
	}

	
public:
	virtual void Generate() = 0;
	virtual bool Check(const CUser& user, const PacketHeader* packet) = 0;
	virtual PacketHeader* GetPacket() const = 0;
};

#endif