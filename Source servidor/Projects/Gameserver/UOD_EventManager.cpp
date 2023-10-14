#include "UOD_EventManager.h"
#include "UOD_Event.h"
#include "UOD_DuelLetter.h"
#include "UOD_Quiz.h"
#include "UOD_ChristmasTree.h"
#include "UOD_ChristmasMission.h"
#include "UOD_BossEvent.h"
#include "UOD_Arena.h"
#include "UOD_Mission.h"
#include "UOD_PreWarBoss.h"
#include "UOD_HappyHarvest.h"
#include "Basedef.h"

TOD_EventManager::TOD_EventManager()
{
	_events[TOD_EventType::DuelLetter] = std::make_unique<TOD_DuelLetter>(800ms);
	_events[TOD_EventType::Quiz] = std::make_unique<TOD_Quiz>(800ms);
	_events[TOD_EventType::Arena] = std::make_unique<TOD_Arena>(800ms);
	_events[TOD_EventType::Mission] = std::make_unique<TOD_Mission>(800ms);
	_events[TOD_EventType::PreWarBoss] = std::make_unique<TOD_PreWarBoss>(800ms);
	_events[TOD_EventType::ChristmasTree] = std::make_unique<TOD_ChristmasTree>(1min);
	_events[TOD_EventType::ChristmasMission] = std::make_unique<TOD_ChristmasMission>(800ms);
	_events[TOD_EventType::BossEvent] = std::make_unique<TOD_BossEvent>(800ms);
	_events[TOD_EventType::HappyHarvest] = std::make_unique<TOD_HappyHarvest>(800ms);
}

std::map<TOD_EventType, std::unique_ptr<TOD_Event>>& TOD_EventManager::GetEvents() 
{
	return _events;
}

TOD_Event* TOD_EventManager::GetEvent(TOD_EventType type) 
{
	auto eventIt = _events.find(type);
	if (eventIt == std::end(_events))
		return nullptr;

	return &*eventIt->second;
}

TOD_EventItem* TOD_EventManager::GetEventItem(TOD_EventType type)
{
	auto eventIt = _events.find(type);
	if (eventIt == std::end(_events))
		return nullptr;

	return static_cast<TOD_EventItem*>(&*eventIt->second);
}