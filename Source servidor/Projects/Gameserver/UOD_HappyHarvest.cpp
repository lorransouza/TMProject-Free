#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "UOD_HappyHarvest.h"

constexpr std::array<st_Position, 4> playerPosition =
{
	{
		{ 1130, 1512 },
		{ 1130, 1499 },
		{ 1131, 1451 },
		{ 1131, 1437 }
	}
};

TOD_HappyHarvest::TOD_HappyHarvest(std::chrono::milliseconds interval)
	: TOD_EventItem(interval)
{
}

void TOD_HappyHarvest::Work()
{
	for (auto userIt = std::begin(_users); userIt != std::end(_users);)
	{
		bool remove = false;
		auto user = *userIt;
		if (user.user->Status != USER_PLAY)
			remove = true;

		CMob& mob = Mob[user.user->clientId];
		remove = remove || !(mob.Target.X >= 1030 && mob.Target.X <= 1150 && mob.Target.Y >= 1409 && mob.Target.Y <= 1535);

		if (remove)
		{
			for (auto& plantation : user.SeedsOwned)
			{
				auto& initItem = pInitItem[plantation.InitIndex];
				if (initItem.Item.Index < 4704 || initItem.Item.Index > 4706)
					continue;

				RemoveSeed(plantation.InitIndex);
			}

			userIt = _users.erase(userIt);
		}
		else	
			userIt++;
	}

	if (!_status)
		return;

	auto now = std::chrono::steady_clock::now();
	if (now - _lastNPCReborn >= 10min)
		GenerateNPC();

	for (auto& user : _users)
	{
		for (auto& plantation : user.SeedsOwned)
		{
			auto& initItem = pInitItem[plantation.InitIndex];
			if (initItem.Item.Index < 4704 || initItem.Item.Index > 4706)
				continue;

			if (plantation.Stage == PlantationStage::Seed)
			{
				if (now - plantation.LastUpdate >= std::chrono::milliseconds(plantation.TimeToPick / 2))
				{
					initItem.Item.Index = 4705;
					plantation.Stage = PlantationStage::SeedGrowing;

					GridMulticastInitInfo(plantation.InitIndex);
				}
			}
			else if (plantation.Stage == PlantationStage::SeedGrowing)
			{
				if (now - plantation.LastUpdate >= std::chrono::milliseconds(plantation.TimeToPick))
				{
					Log(user.user->clientId, LOG_INGAME, "Milho %d cresceu e está pronto para ser colhido", plantation.InitIndex);
					SendClientMessage(user.user->clientId, "Um Milho cresceu e está pronto para ser colhido");

					initItem.Item.Index = 4706;
					plantation.Stage = PlantationStage::Corn;
					plantation.LastUpdate = now;

					GridMulticastInitInfo(plantation.InitIndex);
				}
			}
			else if (plantation.Stage == PlantationStage::Corn)
			{
				if (now - plantation.LastUpdate >= std::chrono::minutes(5))
				{
					Log(user.user->clientId, LOG_INGAME, "Removeu o Milho pronto %d por passar mais de dois minutos sem ser colhido", plantation.InitIndex);

					SendClientMessage(user.user->clientId, "Um dos seus milhos apodreceu :(");
					RemoveSeed(plantation.InitIndex);
					plantation.Stage = PlantationStage::Poor;
				}
			}
		}

		size_t lastSeedsCount = user.SeedsOwned.size();
		user.SeedsOwned.erase(std::remove_if(std::begin(user.SeedsOwned), std::end(user.SeedsOwned), [](const SeedInfo& info) {
			return info.Stage == PlantationStage::Poor;
		}), std::end(user.SeedsOwned));

		if (lastSeedsCount != 0 && user.SeedsOwned.size() == 0)
		{
			DoRecall(user.user->clientId);
			SendClientMessage(user.user->clientId, "Todos os seus milhos acabaram!");

			Log(user.user->clientId, LOG_INGAME, "Voltou para a cidade pois colheu o último Milho");
		}
	}
}

bool TOD_HappyHarvest::CanRegister(CUser& user)
{
	return _status;
}

void TOD_HappyHarvest::Unregister(CUser& user)
{
	auto harvestInfoIt = std::find_if(std::begin(_users), std::end(_users), [&user](const HHUserInfo& userInfo) {
		return userInfo.user == &user;
	});

	if (harvestInfoIt == std::end(_users))
		return;

	for (auto& plantation : harvestInfoIt->SeedsOwned)
		RemoveSeed(plantation.InitIndex);

	_users.erase(harvestInfoIt);
}

bool TOD_HappyHarvest::Register(CUser& user, st_Item* item)
{
	if (!_status)
		return false;

	if (item == nullptr)
	{
#if !defined(_DEBUG)
		for (const auto& userRegistered : _users)
		{
			if (&user == userRegistered.user)
			{
				Log(user.clientId, LOG_INGAME, "Usuário já registrado na Colheita Feliz");
				return false;
			}

			if (memcmp(&userRegistered.user->MacAddress, &user.MacAddress, 8) == 0)
			{
				Log(user.clientId, LOG_INGAME, "Usuário já registrado na Colheita Feliz com a conta %s", userRegistered.user->User.Username);

				SendClientMessage(user.clientId, "Somente uma conta por computador");
				return false;
			}
		}
#endif
		auto& userInfo = _users.emplace_back();
		int fertilizerSlotId = GetFirstSlot(user.clientId, 4712);
		if (fertilizerSlotId != -1)
		{
			auto& fertilizerItem = Mob[user.clientId].Mobs.Player.Inventory[fertilizerSlotId];

			Log(user.clientId, LOG_INGAME, "Consumido 1 unidade de Adubo. Quantidades atuais: %d", GetItemAmount(&fertilizerItem));

			AmountMinus(&fertilizerItem);
			SendItem(user.clientId, SlotType::Inv, fertilizerSlotId, &fertilizerItem);

			userInfo.IsUsingFertilizer = true;
		}

		userInfo.user = &user;

		st_Position position = *select_randomly(std::begin(playerPosition), std::end(playerPosition));
		Teleportar(user.clientId, position.X, position.Y);
		RemoveParty(user.clientId);

		int max = 5;
		int amount = GetInventoryAmount(user.clientId, 4704);
		if (amount < max)
		{
			int left = max - amount;
			for (int i = 0; i < left; ++i)
			{
				int slotId = GetFirstSlot(user.clientId, 0);
				if (slotId == -1)
				{
					SendClientMessage(user.clientId, "Sem espaço disponível para receber a Semente de Milho");

					break;
				}

				auto& item = Mob[user.clientId].Mobs.Player.Inventory[slotId];
				item = st_Item{};
				item.Index = 4704;

				SendItem(user.clientId, SlotType::Inv, slotId, &item);
			}
		}

		Log(user.clientId, LOG_INGAME, "Registrado na área da Colheita Feliz");
		return true;
	}

	auto targetPosition = st_Position{ (BYTE)Mob[user.clientId].Target.X, (BYTE)Mob[user.clientId].Target.Y };
	if (targetPosition.X < 1045 || targetPosition.X > 1104 || targetPosition.Y < 1423 || targetPosition.Y > 1518)
	{
		SendClientMessage(user.clientId, "Só é possível plantar na área de cultivo");

		return false;
	}

	auto harvestInfoIt = std::find_if(std::begin(_users), std::end(_users), [&user](const HHUserInfo& userInfo) {
		return userInfo.user == &user;
	});

	if (harvestInfoIt == std::end(_users))
	{
		DoRecall(user.clientId);

		return false;
	}

	if (harvestInfoIt->SeedsOwned.size() >= 5)
	{
		SendClientMessage(user.clientId, "Máximo de plantios no momento");

		return false;
	}

	int createdIndex = CreateItem(targetPosition.X, targetPosition.Y, item, 0, 0);
	if (createdIndex == 0)
	{
		SendClientMessage(user.clientId, "Não foi possível dropar o item no chão");

		return false;
	}

	SeedInfo info{};
	info.InitIndex = createdIndex;
	info.LastUpdate = std::chrono::steady_clock::now();

	if(harvestInfoIt->IsUsingFertilizer)
		info.TimeToPick = (120 + (Rand() % 60)) * 1000;
	else 
		info.TimeToPick = (180 + (Rand() % 60)) * 1000;

	info.Stage = PlantationStage::Seed;
	Log(user.clientId, LOG_INGAME, "Plantado Semente de Milho. Tempo: %d.", info.TimeToPick);

	harvestInfoIt->SeedsOwned.push_back(info);
	return true;
}

void TOD_HappyHarvest::RemoveNPC()
{
	if (_teleportNpcId <= 0 || _teleportNpcId >= 30000)
		return;

	std::string npcName{ Mob[_teleportNpcId].Mobs.Player.Name };
	if (npcName == "Fazendeira")
		DeleteMob(_teleportNpcId, 1);
}

void TOD_HappyHarvest::GenerateNPC()
{
	if (_teleportNpcId > 0 && _teleportNpcId < 30000)
		RemoveNPC();

	int npcId = CreateMob("Fazendeira", 2130, 2108, "npc");
	if (npcId == 0)
	{
		Log(SERVER_SIDE, LOG_INGAME, "Falha ao gerar NPC Fazendeira");

		return;
	}

	_teleportNpcId = npcId;
	_lastNPCReborn = std::chrono::steady_clock::now();
}

void TOD_HappyHarvest::SetStatus(bool status)
{
	if (_status != status)
	{
		if (status)
			GenerateNPC();
		else
			RemoveNPC();
	}
	_status = status;
}

void TOD_HappyHarvest::GridMulticastInitInfo(int index) const
{
	if (index < 0 || index >= 4096)
		return;

	const auto& initItem = pInitItem[index];
	p26E packet{};
	GetCreateItem(index, &packet);

	GridMulticast_2(initItem.PosX, initItem.PosY, reinterpret_cast<BYTE*>(&packet), 0);
}

void TOD_HappyHarvest::RemoveSeed(int initItemIndex)
{
	if (initItemIndex < 0 || initItemIndex >= 4096)
		return;

	auto& initItem = pInitItem[initItemIndex];
	p16F sm{};
	sm.Header.PacketId = 0x16F;
	sm.Header.Size = sizeof p16F;
	sm.Header.ClientId = 0x7530;
	sm.initID = initItemIndex + 10000;

	GridMulticast_2(initItem.PosX, initItem.PosY, (BYTE*)&sm, 0);

	g_pItemGrid[initItem.PosY][initItem.PosX] = 0;
	pInitItem[initItemIndex].Open = 0;
}

bool TOD_HappyHarvest::PickItem(CUser& user, int initItemIndex) 
{
	auto harvestInfoIt = std::find_if(std::begin(_users), std::end(_users), [&user](const HHUserInfo& userInfo) {
		return userInfo.user == &user;
	});

	if (harvestInfoIt == std::end(_users))
	{
		DoRecall(user.clientId);

		return false;
	}

	auto seedIt = std::find_if(std::begin(harvestInfoIt->SeedsOwned), std::end(harvestInfoIt->SeedsOwned), [initItemIndex](const SeedInfo& seedInfo) {
		return seedInfo.InitIndex == initItemIndex;
	});

	if (seedIt == std::end(harvestInfoIt->SeedsOwned))
	{
		SendClientMessage(user.clientId, "Você só pode colher milhos plantados por você");

		return false;
	}
	int slotId = GetFirstSlot(user.clientId, 0);
	if (slotId == -1)
	{
		SendClientMessage(user.clientId, "Não há espaço no inventário para colher o Milho");

		return false;
	}
	
	int rate = 80;
	if (harvestInfoIt->IsUsingFertilizer)
		rate = 100;

	if (sServer.Weather == 1)
		rate += 10;

	int rand = Rand() % 100;
	if (rand <= rate)
	{
		auto& item = Mob[user.clientId].Mobs.Player.Inventory[slotId];
		item = st_Item{};
		item.Index = 4706;

		SendItem(user.clientId, SlotType::Inv, slotId, &item);
		Log(user.clientId, LOG_INGAME, "Colheu com sucesso. Rate: %d/%d", rand, rate);
	}
	else
	{
		Log(user.clientId, LOG_INGAME, "Falha ao colher. Rate: %d/%d", rand, rate);

		SendClientMessage(user.clientId, "Sua colheita não deu certo. Mais sorte da próxima vez");
	}

	harvestInfoIt->SeedsOwned.erase(seedIt);
	if (harvestInfoIt->SeedsOwned.size() == 0)
	{
		DoRecall(user.clientId);

		Log(user.clientId, LOG_INGAME, "Voltou para a cidade pois colheu o último Milho");
	}

	return true;
}