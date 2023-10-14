#pragma once
#include "singleton.h"

class MacroFactory : public Singleton<MacroFactory>
{
protected:
	friend class Singleton<MacroFactory>;

	MacroFactory();
	virtual ~MacroFactory() {}
	MacroFactory(const MacroFactory&) = delete;
	MacroFactory& operator=(const MacroFactory&) = delete;

public:
	Macro::MacroLevel* getMacroLevel(const uint32_t level);

	const std::vector<Macro::MacroLevel*> getAllMacroLevel() const;

	static void onEvent();

private:
	std::map<uint32_t, std::unique_ptr<Macro::MacroLevel>> _map;
};