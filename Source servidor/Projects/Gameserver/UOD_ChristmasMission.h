#ifndef UOD_ChristmasMissionH
#define UOD_ChristmasMissionH

#include "UOD_Event.h"

class TOD_ChristmasMission : public TOD_Event
{
public:
	TOD_ChristmasMission(std::chrono::milliseconds interval);
	~TOD_ChristmasMission() = default;

	virtual void Work();

	bool HandleReply(CUser& user, pMsgSignal* packet);
	void SetNextRound(CUser& user);
	void RefreshChristmasMission(int clientId, int mobId);

	void SetStatus(bool status)
	{
		_status = true;
	}
private:

	bool _status{ false };
};

#endif