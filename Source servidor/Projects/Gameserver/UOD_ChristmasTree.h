#pragma once

#include "UOD_Event.h"

class CUser;

class TOD_ChristmasTree : public TOD_Event
{
public:
	TOD_ChristmasTree(std::chrono::milliseconds interval);
	~TOD_ChristmasTree() = default;

	virtual void Work();
	void SetStatus(bool status);
private:
	bool _status{ false };
};

