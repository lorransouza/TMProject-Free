#include "pch.h"
#include "MacroLevel.h"
#include "MacroFactory.h"

MacroFactory::MacroFactory() : _map()
{
}

Macro::MacroLevel* MacroFactory::getMacroLevel(const uint32_t level)
{
	auto it = _map.find(level);

	if (it == _map.end())
	{
		Macro::MacroLevel* currentLevel = nullptr;

		if (level == 1)
			currentLevel = new Macro::Water_N();

		else if (level == 2)
			currentLevel = new Macro::Water_M();

		else if (level == 3)
			currentLevel = new Macro::Water_A();

		else
			return nullptr;

		if (currentLevel == nullptr)
			return nullptr;

		_map.insert(std::make_pair(level, std::unique_ptr<Macro::MacroLevel>(currentLevel)));
	}

	return _map[level].get();
}

const std::vector<Macro::MacroLevel*> MacroFactory::getAllMacroLevel() const
{
	auto tmp = std::vector<Macro::MacroLevel*>();

	for (auto& i : _map)
		tmp.push_back(i.second.get());

	return tmp;
}

void MacroFactory::onEvent()
{
}
