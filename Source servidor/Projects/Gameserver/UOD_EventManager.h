#ifndef UOD_EventManagerH
#define UOD_EventManagerH

#include <map>
#include <memory>
#include "UOD_Event.h"
#include "UOD_Singleton.h"

enum class TOD_EventType
{
	DuelLetter = 0,
	Quiz = 1,
	ChristmasTree,
	ChristmasMission,
	BossEvent,
	Arena,
	Mission,
	PreWarBoss,
	HappyHarvest
};

class TOD_EventManager
{
public:
	TOD_EventManager();

	TOD_EventManager(const TOD_EventManager&) = delete;
	TOD_EventManager& operator=(const TOD_EventManager&) = delete;
	TOD_EventManager(TOD_EventManager &&) = delete;
	TOD_EventManager& operator=(TOD_EventManager&&) = delete;

	TOD_Event* GetEvent(TOD_EventType);
	TOD_EventItem* GetEventItem(TOD_EventType);
	std::map<TOD_EventType, std::unique_ptr<TOD_Event>>& GetEvents();

	static TOD_EventManager& GetInstance()
	{
		static TOD_EventManager event;

		return event;
	}

private:
	std::map<TOD_EventType, std::unique_ptr<TOD_Event>> _events;
};

#endif