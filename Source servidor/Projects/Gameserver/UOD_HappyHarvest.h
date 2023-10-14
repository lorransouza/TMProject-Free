#pragma once

#include "UOD_Event.h"

enum class PlantationStage
{
	Seed,
	SeedGrowing,
	Corn,
	Poor
};

class TOD_HappyHarvest : public TOD_EventItem
{
	struct SeedInfo
	{
		PlantationStage Stage;
		uint32_t TimeToPick;
		uint16_t InitIndex;

		std::chrono::time_point<std::chrono::steady_clock> LastUpdate;
	};

	struct HHUserInfo
	{
		CUser* user;
		std::vector<SeedInfo> SeedsOwned;

		bool IsUsingFertilizer;
	};

public:
	TOD_HappyHarvest(std::chrono::milliseconds interval);
	~TOD_HappyHarvest() = default;

	virtual void Work();
	virtual bool CanRegister(CUser& user);
	virtual void Unregister(CUser& user);
	virtual bool Register(CUser& user, st_Item* item);

	bool PickItem(CUser& user, int initItemIndex);
	void SetStatus(bool status);
private:
	bool _status{ false };

	std::chrono::time_point<std::chrono::steady_clock> _lastNPCReborn;
	int _teleportNpcId{ -1 };
	std::vector<HHUserInfo> _users;
	void GridMulticastInitInfo(int index) const;

	void RemoveSeed(int initItemIndex);

	void RemoveNPC();
	void GenerateNPC();
};

