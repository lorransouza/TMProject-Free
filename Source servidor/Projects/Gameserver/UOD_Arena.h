#pragma once

#include <array>
#include <vector>
#include "UOD_Event.h"
#include "Basedef.h"

constexpr int MaxGroups = 4;
constexpr int MinimumParticipants = 8;

constexpr std::array<const char*, 4> GroupsName = {
	"Alfa",
	"Beta",
	"Ômega",
	"Gama"
};

class CUser;

enum class TOD_Arena_Status
{
	Disabled,
	WaitingToStart,
	Enabled,
	WaitingToDisable
};

class TOD_Arena : public TOD_EventItem
{
	struct InternalArena
	{
		CUser* User;
		bool IsParticipant;
	};
public:
	TOD_Arena(std::chrono::milliseconds interval)
		: TOD_EventItem(interval)
	{
	}

	virtual void Work();
	virtual bool CanRegister(CUser& user);
	virtual bool Register(CUser& user, st_Item* item);
	virtual void Unregister(CUser& user);

	size_t GetTotalRegistered() const
	{
		return _registeredUsers.size();
	}

	bool MobKilled(int killed, int killer);
private:
	int _pointsToWin{ 0 };

	TOD_Arena_Status _status{ TOD_Arena_Status::Disabled };
	std::chrono::time_point<std::chrono::steady_clock> _startTime;
	std::chrono::time_point<std::chrono::steady_clock> _lastMessage;
	std::array<std::vector<InternalArena>, MaxGroups> _groups;
	std::vector<CUser*> _registeredUsers;

	bool StartArena();
	bool IsOnHour() const;

	std::array<int, MaxGroups> GetPoints() const;
	
	bool IsUserValid(const InternalArena& user) const;

	void GiveReward(int groupIndex);
	void GiveReward(CUser* user, const ArenaReward& reward);

	void RefreshScoreboard() const;
	void CheckForWinner();
	void DecideWinner();
	void Reset();

	bool ReadHoursAllowed();
};