#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestEssenceUse(PacketHeader* header)
{
	MSG_ESSENCEPACKET* p = reinterpret_cast<MSG_ESSENCEPACKET*>(header);

	static const int LevelMax[7] = {
		25, 35, 45, 55, 65, 75, 100
	};
	static const int Rate[2][12] = {
		{ 100, 100, 95, 85, 75, 65, 55, 45, 35, 25, 10, 3 },
		{ 95, 85, 75, 65, 55, 45, 35, 25, 15, 7, 5, 3 },
	};

	if (p->Slot < 0 || p->Slot >= 60)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

		return true;
	}

	st_Item* srcItem = &Mob[clientId].Mobs.Player.Inventory[p->Slot];
	st_Item* dstItem = GetItemPointer(clientId, (int)SlotType::Equip, 14);
	if (dstItem->Index < 2330 || dstItem->Index > 2389)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

		SendItem(clientId, SlotType::Inv, p->Slot, srcItem);
		return true;
	}

	st_Mob* player = &Mob[clientId].Mobs.Player;

	int mount = player->Equip[14].Index;
	int mountIndex = 0;
	if (mount >= 2360 && mount <= 2389)
		mountIndex = mount - 2360;
	else if (mount >= 2330 && mount <= 2358)
		mountIndex = mount - 2330;

	int amagoId = 2390 + mountIndex;
	if (mount == 2387 || mount == 2388)
		amagoId = 2417 + (mount - 2387);

	if (amagoId != srcItem->Index)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

		SendItem(clientId, SlotType::Inv, p->Slot, srcItem);
		return true;
	}

	int amount = GetItemAmount(srcItem);
	if (amount < p->Amount || p->Amount < 0 || p->Amount > 120)
	{
		SendClientMessage(clientId, "Valor incorreto");

		SendItem(clientId, SlotType::Inv, p->Slot, srcItem);
		Log(SERVER_SIDE, LOG_INGAME, "Valor para uso de âmagos incorreto. Total de amount: %d. Valor enviado: %d", amount, p->Amount);
		return true;
	}

	if (player->Equip[14].EFV2 <= 5)
	{
		SendClientMessage(clientId, "Vitalidade mínima para aumentar o nível da montaria é 6.");

		SendItem(clientId, SlotType::Inv, p->Slot, srcItem);
		return true;
	}

	Log(clientId, LOG_INGAME, "Solicitado o uso de %d âmagos", p->Amount);

	int success = 0;
	int fail = 0;
	int lastLevel = player->Equip[14].EF2;

	for (int i = 0; i < p->Amount; ++i)
	{
		if (mount < 2360)
		{
			player->Equip[14].EF2++;
			if (mountIndex > 6)
				mountIndex = 6;

			if (player->Equip[14].EF2 >= LevelMax[mountIndex])
			{
				mountIndex = mount - 2330;

				player->Equip[14].Index = 2360 + mountIndex;

				*(short*)& player->Equip[14].EF1 = 5000;

				player->Equip[14].EF2 = 1;
				player->Equip[14].EFV2 += Rand() % 16 + 5;

				player->Equip[14].EF3 = 100;
				player->Equip[14].EFV3 = 0;

				SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);

				Log(clientId, LOG_INGAME, "Montaria cresceu: %s %s", ItemList[player->Equip[14].Index].Name, player->Equip[14].toString().c_str());

				MountProcess(clientId, 0);
				success++;
				break;
			}
			else
			{
				Log(clientId, LOG_INGAME, "Âmago obteve sucesso de %s %s.",
					ItemList[srcItem->Index].Name,
					player->Equip[14].toString().c_str());

				success ++;
			}
		}
		else
		{
			if (player->Equip[14].EF2 >= 120)
			{
				SendClientMessage(clientId, "Level máximo.");

				break;
			}

			int ref = player->Equip[14].EF2 / 10;

			if (ref > 11)
				ref = 11;

			int _rate;
			if (mountIndex < 17)
				_rate = Rate[0][ref];
			else
				_rate = Rate[1][ref];

			int _bonus = 0;
			for (int i = 0; i < 3; i++)
			{
				if (srcItem->Effect[i].Index == 210)
				{
					_bonus = srcItem->Effect[i].Value;
					break;
				}
			}

			_rate += _bonus;

			int _rand = Rand() % 100;
			player->Equip[14].EF3 = 100;

			if (_rand > _rate)
			{
				if ((Rand() % 3) != 0 || (player->Equip[14].EF2 % 2) != 0)
				{
					Log(clientId, LOG_INGAME, "Falhou %s e nada aconteceu. %s. Level atual: %hhu. Bônus: %d", 
						ItemList[srcItem->Index].Name,
						player->Equip[14].toString().c_str(), 
						player->Equip[14].EF2,
						_bonus);
				}
				else if (player->Equip[14].EF2)
				{
					Log(clientId, LOG_INGAME, "%s falhou e level baixou %s. Level atual: %hhu, Level novo: %hhu BÔnus: %d", 
						ItemList[srcItem->Index].Name,
						player->Equip[14].toString().c_str(), 
						player->Equip[14].EF2, 
						player->Equip[14].EF2 - 1, 
						_bonus);

					player->Equip[14].EF2 = (player->Equip[14].EF2 - 1);
				}

				fail++;
			}
			else
			{
				player->Equip[14].EF2++;
				Log(clientId, LOG_INGAME, "%s obteve sucesso de %s. Level atual: %hhu, Level novo: %hhu. Bônus %d", 
					ItemList[srcItem->Index].Name,
					player->Equip[14].toString().c_str(), 
					player->Equip[14].EF2 - 1,
					player->Equip[14].EF2);

				success++;
			}
		}

		AmountMinus(srcItem);
	}

	SendItem(clientId, SlotType::Equip, 14, &Mob[clientId].Mobs.Player.Equip[14]);
	SendItem(clientId, SlotType::Inv, p->Slot, srcItem);

	SendClientMessage(clientId, "Âmagos usados: %d. Com sucesso: %d. Com falha: %d Leveis ganhos: %d", success + fail, success, fail, player->Equip[14].EF2 - lastLevel);
	SendEmotion(clientId, 14, 3);
	return true;
}