#pragma once

#include "stBase.h"
#include <map>
#include <tuple>
#include <sstream>

struct st_Item;

class TOD_ResetAccount
{
public:
	TOD_ResetAccount(stAccount*);

	stAccount ResetAccount();
	
	std::string GetReport() const
	{
		return report.str();
	}
private:
	stAccount _account;

	void LoadDonatePrice();

	std::map<short, int> donateStore;

	int GetItemPrice(const st_Item* item);
	void Log_Text(const char* msg, ...);

	bool isInitialPackage(const st_Item* item) const;
	bool isExpired(const st_Item* item) const;
	int GetItemAmount(const st_Item* item) const;
	float TimeRemaining(int dia, int mes, int ano) const; 
	
	std::tuple<bool, st_Item>  isMountWithCostume(const st_Item* item);
	std::stringstream report;
};

