#pragma once
#include "singleton.h"

class MacroMsg : public Singleton<MacroMsg>
{
protected:
	friend class Singleton<MacroMsg>;

	MacroMsg() {}
	virtual ~MacroMsg() {}
	MacroMsg(const MacroMsg&) = delete;
	MacroMsg& operator=(const MacroMsg&) = delete;

public:
	void onEvent(const int posX, const int posY, const int Param);

	void onPutItem(const int posX, const int posY, const int Level);

	void onItemUsed(const int WaterScrollId);

	int WaterLevel = -1;
};