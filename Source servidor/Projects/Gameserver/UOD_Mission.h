#pragma once

#include "UOD_Event.h"

class TOD_Mission : public TOD_Event
{
public:
	TOD_Mission(std::chrono::milliseconds interval)
		: TOD_Event(interval)
	{
	}

	~TOD_Mission() = default;

	virtual void Work();
	void SetLastUpdateMission(CUser& user) const;
	void MobKilled(int killer, int killed) const;
	void Reset(CUser& user) const;
	
	void SetStatus(bool status)
	{
		_status = status;
	}

private:
	void SetNextDailyQuest(CUser& user);
	void UncompletedMission(CUser& user);
	bool _status{ true };
};

