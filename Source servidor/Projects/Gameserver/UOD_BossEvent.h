#pragma once

#include <vector>
#include "UOD_Event.h"

constexpr int TotalBoss = 2;

class TOD_BossEvent : public TOD_EventItem
{
private:
	std::array<int, TotalBoss> _bossId;
	int _lastUsersCount{ 0 }; 
	int _npcId{ -1 };
	std::vector<CUser*> _users;
	st_Item _item;
	std::string _bossName;
	std::string _npcName;
	int _medalId{ 0 };
	bool _eventStatus{ false };
	int _teleportId{ -1 };
	bool _status{ false };
	int _teleportInitIndex{ -1 };
	bool _doubleMode{ false };
protected:
	virtual void Work();

public:
	TOD_BossEvent(std::chrono::milliseconds interval)
		: TOD_EventItem(interval)
	{
	}

	virtual bool CanRegister(CUser& user);
	virtual bool Register(CUser& user, st_Item* item);
	virtual void Unregister(CUser& user)
	{
	}

	void Reset();
	void SetEventStatus(bool status)
	{
		_eventStatus = status;
	}

	void SetDoubleMode(bool status)
	{
		_doubleMode = status;
	}

	std::string GetBossName() const
	{
		return _bossName;
	}

	void SetMedalID(int medalId)
	{
		_medalId = medalId;
	}

	void SetItem(st_Item item)
	{
		_item = item;
	}

	void SetTeleportNPCName(std::string npcName)
	{
		_npcName = npcName;
	}

	void SetBossName(std::string bossName)
	{
		_bossName = bossName;
	}

	int GetTeleportIndex() const
	{
		return _teleportId;
	}
};


