#ifndef AntiBotH
#define AntiBotH

#include "AntiBotPacketTest.h"

#include <thread>
#include <memory>
#include <array>

using namespace std::chrono_literals;
using timepoint = std::chrono::time_point<std::chrono::steady_clock>;

struct TUOD_AB_UserInfo
{
	timepoint lastUpdate;

	std::unique_ptr<IOD_AB_PacketGenerator> packet;
	bool isWaiting;
};

class TUOD_AntiBot
{
private:
	std::thread threadAntibot;

	TUOD_AntiBot();
	~TUOD_AntiBot();

	// sem operadores dee movimento
	TUOD_AntiBot(const TUOD_AntiBot&) = delete;
	TUOD_AntiBot(TUOD_AntiBot&&) = delete;
	TUOD_AntiBot& operator=(TUOD_AntiBot&&) = delete;
	TUOD_AntiBot& operator=(const TUOD_AntiBot&) = delete;

	volatile bool _disposing{ false };
public:
	void GetGenerator(IOD_AB_PacketGenerator*) const;
	void AntiBot();
};
#endif