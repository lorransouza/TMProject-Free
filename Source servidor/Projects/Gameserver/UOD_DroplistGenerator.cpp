#include "UOD_DroplistGenerator.h"
#include "Basedef.h"
#include "CNPCGener.h"

bool TOD_DroplistGenerator::Generate()
{
	FileWrapper fileWrapper{ "droplist.bin" };

	std::vector<MobDropInfo> droplist;
	for (const auto& gener : mGener.pList)
	{
		if (gener.MinuteGenerate == -1 || !gener.Leader.Name[0])
			continue;

		if (gener.Leader.bStatus.Merchant.Merchant != 0)
			continue;

		std::string name{ gener.Leader.Name };

		for (auto& c : name)
			if (c == '\'')
				c = ' ';

		for (auto u = name.size(); u > 0; u--)
		{
			if (isalnum(name[u]) || name[u] == '-' || name[u] == '[' || name[u] == ']')
				break;

			if (name[u] == '_')
				name = name.substr(0, u);
		}

		if (std::find_if(std::begin(droplist), std::end(droplist), [name](const MobDropInfo& mobInfo) {
			auto str = std::string(mobInfo.Name);
			return str == name;
		}) != std::end(droplist))
			continue;

		droplist.emplace_back();
		auto& mobInfo = droplist.back();

		const st_Mob* mob = &gener.Leader;

		int slot = 0;
		for (int i = 0; i < 64; i++)
		{
			if (mob->Inventory[i].Index <= 0)
				continue;
			{
				bool exists = false;
				for (int j = 0; j < slot; j++)
					if (mobInfo.item[j].Index == mob->Inventory[i].Index)
						exists = true;

				if (exists)
					continue;
			}

			{
				bool exists = false;
				for (int p = 0; p < MAX_BLOCKITEM; ++p)
				{
					if (g_pBlockedItem[p] == mob->Inventory[i].Index)
					{
						exists = true;

						break;
					}
				}

				if (exists)
					continue;
			}

			mobInfo.item[slot++] = mob->Inventory[i];
		}

		strncpy_s(mobInfo.Name, name.c_str(), 15);
		mobInfo.Position.X = gener.Segment_X[0];
		mobInfo.Position.Y = gener.Segment_Y[0];
	}

	std::sort(std::begin(droplist), std::end(droplist), [](const MobDropInfo& lhs, const MobDropInfo& rhs) {
		return lhs.Name[0] < rhs.Name[0];
	});

	auto i = droplist.size();
	fwrite(&i, sizeof size_t, 1, fileWrapper);
	fwrite(droplist.data(), sizeof MobDropInfo, droplist.size(), fileWrapper);
	return true;
}