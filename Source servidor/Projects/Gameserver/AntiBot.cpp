#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "AntiBot.h"
#include <sstream>
#include <memory>

using namespace std::chrono_literals;

void TUOD_AntiBot::GetGenerator(IOD_AB_PacketGenerator* retn) const
{
	int value = Rand() % 1;
	
	if (value == 0)
	{
		//retn = std::move(&TOD_AB_PacketTest{0x300});
		return;
	}
}
TUOD_AntiBot::TUOD_AntiBot()
{
	threadAntibot = std::thread(&TUOD_AntiBot::AntiBot, this);
}

TUOD_AntiBot::~TUOD_AntiBot()
{
	_disposing = true;

	threadAntibot.join();
}

void TUOD_AntiBot::AntiBot()
{
	while (!_disposing)
	{
		auto now = std::chrono::high_resolution_clock::now();
		for (auto& user : Users)
		{
			auto last = now - user.ABInfo.lastUpdate;
			if (user.ABInfo.isWaiting)
			{
				if(last <= 10s)
					continue;

				std::stringstream str;
				str << "O usuário não respondeu a tempo a checagem de antibot.\n";
				::Log(user.clientId, LOG_HACK, "O usuário não respondeu a tempo o pacote de checagem de antibot.\nPacote");
			}

			if (last < 20s)
				continue;

			/*IOD_AB_PacketGenerator* generator;
			GetGenerator(generator);

			generator->Generate();

			PacketHeader* packet = generator->GetPacket();
			user.AddMessage(reinterpret_cast<BYTE*>(packet), sizeof *packet);*/
		}
	}
}