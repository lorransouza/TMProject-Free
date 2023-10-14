#ifndef UOD_EventH
#define UOD_EventH

#include <chrono>
#include <array>

using time_point = std::chrono::time_point<std::chrono::steady_clock>;
using namespace std::chrono_literals;

class CUser;
struct st_Item;

class TOD_Event
{
protected:
	std::chrono::milliseconds _interval;
	time_point _lastExecution;
	
	virtual void Work() = 0;

	TOD_Event(std::chrono::milliseconds interval);
public:
	void Fire();

	std::chrono::milliseconds GetInterval() const;
	void SetInterval(std::chrono::milliseconds interval);
	time_point GetLastExecution() const;
	void SetLastExecution(time_point time);
};

class TOD_EventItem : public TOD_Event
{
public:
	TOD_EventItem(std::chrono::milliseconds interval)
		: TOD_Event(interval)
	{

	}
	virtual bool CanRegister(CUser& user) = 0;
	virtual void Unregister(CUser& user) = 0;
	virtual bool Register(CUser& user, st_Item* item) = 0;
};

#endif