#include "UOD_ResetAccount.h"
#include "Basedef.h"
#include <vector>

enum class Evolution
{
	Mortal = 1,
	Arch,
	Celestial,
	Subcelestial
};

TOD_ResetAccount::TOD_ResetAccount(stAccount* account)
	: _account(*account)
{
	LoadDonatePrice();
}

stAccount TOD_ResetAccount::ResetAccount()
{
	std::vector<st_Item> items;
	int cash = _account.Cash;

	Evolution evolution = Evolution::Mortal;
	auto checkItem = [&](st_Item* item) {
		int itemPrice = GetItemPrice(item);
		if (itemPrice != 0 && !isExpired(item))
		{
			cash += itemPrice;
			Log_Text("Recebeu %d cash pelo item %s", itemPrice, item->toString().c_str());
		}

		if (isInitialPackage(item))
		{
			items.push_back(*item);

			Log_Text("O item %s é eterno e será devolvido", item->toString().c_str());
		}

		auto[isMount, petItem] = isMountWithCostume(item);

		if (isMount)
		{
			Log_Text("O item %s possui traje e foi devolvido o item %s", item->toString().c_str(), petItem.toString().c_str());

			items.push_back(petItem);
		}
	};

	for (int i = 0; i < 4; ++i)
	{
		if (!_account.Mob[i].Player.Name[0])
			continue;

		auto mob = &_account.Mob[i];
		for (int iSlot = 0; iSlot < 64; ++iSlot)
		{
			auto item = &mob->Player.Inventory[iSlot];
			if (item->Index <= 0 || item->Index >= 6500)
				continue;

			checkItem(item);
 		}

		for (int iSlot = 0; iSlot < 16; ++iSlot)
		{
			auto item = &mob->Player.Equip[iSlot];
			if (item->Index <= 0 || item->Index >= 6500)
				continue;

			checkItem(item);
		}

		if (mob->Player.Equip[0].EFV2 > (int)evolution)
			evolution = (Evolution)mob->Player.Equip[0].EFV2;

		if (mob->Player.Equip[0].EFV2 == 3 && mob->Sub.Status == 1)
			evolution = Evolution::Subcelestial;
	}

	for (int i = 0; i < 120; ++i)
	{
		auto item = &_account.Storage.Item[i];
		if (item->Index <= 0 || item->Index >= 6500)
			continue;

		checkItem(item);
	}

	if (evolution != Evolution::Mortal)
	{
		if (evolution == Evolution::Arch)
			items.push_back(st_Item{ 4643 });
		else if (evolution == Evolution::Celestial)
			items.push_back(st_Item{ 4644 });
		else if (evolution == Evolution::Subcelestial)
			items.push_back(st_Item{ 4645 });
	}

	Log_Text("Total de cash: %d. Total de itens: %u", cash, items.size());

	stAccount acc{};
	for (size_t i = 0; i < items.size(); ++i)
	{
		acc.Storage.Item[i] = items[i];

		Log_Text("Adicionado o item %s no slot %u", items[i].toString().c_str(), i);
	}

	strncpy_s(acc.Password, _account.Password, 36);
	strncpy_s(acc.Username, _account.Username, 16);
	acc.Cash = cash;
	acc.AccessLevel = _account.AccessLevel;

	acc.Ban = _account.Ban;
	acc.BanType = _account.BanType;
	return acc;
}

void TOD_ResetAccount::LoadDonatePrice()
{
	donateStore[3996] = 800;

	for (int i = 4150; i < 4189; ++i)
	{
		if (i == 4152 || i == 4153 || i == 4155 || i == 4156)
			continue;

		donateStore[i] = 2000;
	}

	for (int iStore = 0; iStore < 10; ++iStore)
	{
		for (int i = 0; i < 27; ++i)
		{
			auto item = g_pStore[iStore][i].item;
			if (item.Index == 0)
				continue;

			if (item.Index >= 2420 && item.Index <= 2433)
				continue;

			if (item.Index == 3172 || item.Index == 4146 || item.Index == 4131)
				continue;

			donateStore[item.Index] = g_pStore[iStore][i].Price;
		}
	}
}

int TOD_ResetAccount::GetItemPrice(const st_Item* item)
{
	if (donateStore.find(item->Index) != std::end(donateStore))
	{
		int price = donateStore[item->Index];
		if (item->Index == 4140)
			price *= GetItemAmount(item);
		if (item->Index == 3314)
		{
			for (int i = 0; i < 3; ++i)
			{
				if (item->Effect[i].Index == 200)
					return 0;
			}

			price *= GetItemAmount(item);
		}

		return price;
	}
	return 0;
}

void TOD_ResetAccount::Log_Text(const char* msg, ...)
{
	/* Arglist */
	char buffer[512];
	va_list arglist;
	va_start(arglist, msg);
	vsprintf_s(buffer, msg, arglist);
	va_end(arglist);
	/* Fim arlist */

	report << buffer << std::endl;
}

bool TOD_ResetAccount::isInitialPackage(const st_Item* item) const
{
	constexpr std::array eternalItems = { 4152, 4153, 4155, 4156, 4220, 4221, 4194, 4195, 4191, 4192, 4199, 4193, 4595 };

	if (std::find(std::begin(eternalItems), std::end(eternalItems), item->Index) != std::end(eternalItems))
		return true;
	
	return false;
}


float TOD_ResetAccount::TimeRemaining(int dia, int mes, int ano) const
{
	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	int month = now.tm_mon; //0 Janeiro, 1 Fev
	int day = now.tm_mday;
	int year = now.tm_year;

	struct std::tm a = { 0,0,0, day, month, year };
	struct std::tm b = { 0,0,0, dia, mes - 1, ano - 1900 };

	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	if (x != (std::time_t)(-1) && y != (std::time_t)(-1))
	{
		double difference = (std::difftime(y, x) / (60 * 60 * 24));
		return static_cast<float>(difference);
	}

	return 0;
}

bool TOD_ResetAccount::isExpired(const st_Item* item) const
{
	if ((item->Index >= 3980 && item->Index <= 3999) || (item->Index >= 4151 && item->Index <= 4189) || (item->Index >= 3995 && item->Index <= 3995) || (item->Index >= 4210 && item->Index <= 4229) || (item->Index >= 4235 && item->Index <= 4241))
	{
		if (item->EF1 == 106 && item->EF2 == 110 && item->EF3 == 109)
			return TimeRemaining(item->EFV1, item->EFV2, (item->EFV3 + 2000)) <= 0.0f;
	}

	return false;
}

std::tuple<bool, st_Item> TOD_ResetAccount::isMountWithCostume(const st_Item* item)
{
	if (item->Index >= 2360 && item->Index < 2390 && item->Effect[2].Value > 11)
	{
		st_Item petItem{};
		int trajeId = 4190 + (item->Effect[2].Value - 11);
		switch (item->Effect[2].Value)
		{
		case 22:
			trajeId = 4200;
			break;
		case 24:
			trajeId = 4201;
			break;
		case 25:
			trajeId = 4202;
			break;
		case 26:
			trajeId = 4204;
			break;
		}

		petItem.Index = trajeId;

		return std::make_tuple(true, petItem);
	}

	return std::make_tuple(false, st_Item{});
}

int TOD_ResetAccount::GetItemAmount(const st_Item* item) const
{
	INT32 amount = 1;

	for (INT8 i = 0; i < 3; i++)
	{
		if (item->Effect[i].Index == 61)
		{
			amount = item->Effect[i].Value;
			if (amount == 0)
				amount = 1;

			break;
		}
	}

	return amount;
}