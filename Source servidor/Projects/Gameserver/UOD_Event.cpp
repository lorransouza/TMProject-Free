#include "Basedef.h"
#include "UOD_Event.h"
#include "UOD_DuelLetter.h"
#include "UOD_Quiz.h"
#include <memory>

TOD_Event::TOD_Event(std::chrono::milliseconds interval)
	: _interval(interval)
{

}

void TOD_Event::Fire()
{
	try
	{
		Work();
	}
	catch (std::exception& e)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Exceção ocorreu em \"Fire\". Mensagem: %s", e.what());
	}
}

void TOD_Event::SetLastExecution(time_point time)
{
	_lastExecution = time;
}

void TOD_Event::SetInterval(std::chrono::milliseconds time)
{
	_interval = time;
}

std::chrono::milliseconds TOD_Event::GetInterval() const
{
	return _interval;
}

time_point TOD_Event::GetLastExecution() const
{
	return _lastExecution;
}