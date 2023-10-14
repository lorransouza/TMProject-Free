#pragma once

#include <vector>
#include <map>
#include "Struct.h"
#include "UOD_Event.h"

using milliseconds = std::chrono::duration<unsigned int, std::milli>;

class TOD_Quiz : public TOD_EventItem
{
	enum class TOD_Quiz_Quest
	{
		LanHouseN,
		LanHouseM,
		LanHouseA
	};

	struct TOD_Quiz_UserInfo
	{
		CUser* User;

		time_point lastInteraction;
		std::chrono::milliseconds nextIteraction;

		TOD_Quiz_Quest quest;

		bool isWaitingResponse{ false };
		bool firstInteraction{ false };
		int correctIndex;
	};

public:
	TOD_Quiz(std::chrono::milliseconds interval)
		: TOD_EventItem(interval)
	{
	}

	virtual bool Register(CUser&, st_Item*);
	virtual void Unregister(CUser&)
	{
	}

	virtual bool CanRegister(CUser&);

	bool GetStatus() const noexcept;
	bool HandlePacket(CUser& user, MSG_QUIZ_ANSWER* packet);

protected:
	virtual void Work();

private:
	bool _status{ true };

	std::chrono::milliseconds GetNextQuiz() const;
	std::vector<TOD_Quiz_UserInfo> _users;

	constexpr std::chrono::milliseconds GetWaitResponseTime() const;
};