#pragma once

#include "UOD_Event.h"
#include <chrono>

class TOD_PreWarBoss : public TOD_Event
{
public:
	virtual void Work();

	void MobKilled(int killer, int killed);
	TOD_PreWarBoss(std::chrono::milliseconds interval)
		: TOD_Event(interval)
	{

	}

	bool GetStatus() const
	{
		return _status;
	}

private:
	std::chrono::time_point<std::chrono::steady_clock> _startTime;

	bool _status{ false };

	void Start();
	void Finalize();

};

