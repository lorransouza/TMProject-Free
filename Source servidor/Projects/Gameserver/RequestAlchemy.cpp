#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <vector>

bool CUser::RequestAlchemy(PacketHeader* Header)
{
	struct Item
	{
		int Index{ 0 };
		short Sanc{ 0 };
		short Amount{ 1 };
	};

	struct AlchemyItem
	{
		std::array<Item, 7> Required;

		int Gold;
		int Earned;
	};

	static const std::vector<AlchemyItem> compositionItems
	{
		{
			{
				{
					{413, 0},
					{2441, 0},
					{2442, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3200
		},
		{
			{
				{
					{413, 0},
					{2443, 0},
					{2442, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3201
		},
		{
			{
				{
					{4127, 0},
					{4127, 0},
					{4127, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3202
		},
		{
			{
				{
					{4127, 0},
					{4127, 0},
					{697, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3203
		},
		{
			{
				{
					{412, 0},
					{2441, 0},
					{2444, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3204
		},
		{
			{
				{
					{412, 0},
					{2444, 0},
					{2443, 0},
					{0, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3205
		},
		{
			{
				{
					{4127, 0},
					{4127, 0},
					{4127, 0},
					{413, 0, 10},
					{3221, 0},
					{0, 0},
					{0, 0},
				},
			},
			20000000,
			3214
		},
		// Joias Palito
		// Joia do Poder
		{
			{
				{
					{413, 0, 60},
					{4127, 0 },
					{4127, 0 },
					{4127, 0 },
					{614, 9},
					{3221, 0},
					{0, 0},
				},
			},
			100000000,
			3206
		},
		/*{
			{
				{
					{612, 0},
					{613, 0},
					{614, 0},
					{615, 0},
					{0, 0},
					{0, 0},
					{0, 0},
				},
			},
			0,
			3207
		},*/
		// Precisão
		{
			{
				{
					{413, 0, 60},
					{4127, 0 },
					{4127, 0 },
					{4127, 0 },
					{612, 9},
					{3221, 0},
					{0, 0},
				},
			},
			100000000,
			3208
		},	
		// Magia
		{
			{
				{
					{413, 0, 60},
					{4127, 0 },
					{4127, 0 },
					{4127, 0 },
					{613, 9},
					{3221, 0},
					{0, 0},
				},
			},
			100000000,
			3209
		}
	};

	st_Mob* player = &Mob[clientId].Mobs.Player;
	pCompor* p = (pCompor*)Header;

	for (int i = 0; i < 7; i++)
	{
		if (p->slot[i] == -1)
		{
			memset(&p->items[i], 0, sizeof st_Item);
			continue;
		}

		if (p->slot[i] < 0 || p->slot[i] >= 60)
		{
			SendClientMessage(clientId, "Inválido");

			return true;
		}

		for (int y = 0; y < 7; y++)
		{
			if (y == i)
				continue;

			if (p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Alchemy - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Alchemy - %d", player->Name, p->items[i].Index);

				SendCarry(clientId);
				return true;
			}
		}

		if (!memcmp(&p->items[i], &player->Inventory[p->slot[i]], sizeof st_Item))
			continue;

		SendClientMessage(clientId, "Inválido");
		return true;
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	for (int i = 0; i < 7; i++)
	{
		if (p->items[i].Index == 0)
			continue;

		Log(clientId, LOG_COMP, "Alchemy  - %d - %s %s - %d", i, ItemList[p->items[i].Index].Name, p->items[i].toString().c_str(), p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	const AlchemyItem* alchemy = nullptr;
	for (const auto& comp : compositionItems)
	{
		bool itsThatComp = true;
		for (int i = 0; i < 7; i++) 
		{
			if (comp.Required[i].Index == 0)
				continue;

			if (comp.Required[i].Index != p->items[i].Index || GetItemSanc(&p->items[i]) != comp.Required[i].Sanc || GetItemAmount(&p->items[i]) != comp.Required[i].Amount)
			{
				itsThatComp = false;

				break;
			}
		}

		if (itsThatComp)
		{
			alchemy = &comp;
			break;
		}
	}

	if (alchemy == nullptr)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

		return true;
	}

	if (alchemy->Gold != 0 && player->Gold < alchemy->Gold)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

		return true;
	}

	INT32 chance = 3 + (Mob[clientId].Mobs.Player.Status.Mastery[2] * 2 / 10);
	if ((Mob[clientId].Mobs.Player.Learn[0] & 0x8000))
		chance += 20;

	int rand = Rand() % 100;
	for (int i = 0; i < 7; i++)
	{
		char slotId = p->slot[i];
		if (slotId == -1)
			continue;

		Log(clientId, LOG_INGAME, "Removido o item %s %s. Slot: %d", ItemList[p->items[i].Index].Name, p->items[i].toString().c_str(), (int)p->slot[i]);
		memset(&player->Inventory[slotId], 0, sizeof st_Item);
		SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}

	if (alchemy->Gold != 0)
	{
		player->Gold -= alchemy->Gold;
	
		Log(clientId, LOG_INGAME, "Removido %d gold do personagem. Gold restante:", alchemy->Gold, player->Gold);
		SendSignalParm(clientId, clientId, 0x3AF, player->Gold);
	}

	// Fecha o inventário
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	if (rand > chance)
	{
		Log(clientId, LOG_COMP, "Falha na composição na skill de Alquimia");

		SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);
	}
	else
	{
		// Seta a Pedra Secreta
		player->Inventory[p->slot[0]].Index = alchemy->Earned;

		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		// Envia a mensagem de sucesso
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_COMP, "Sucesso na composição do item %s [%d]", ItemList[alchemy->Earned].Name, alchemy->Earned);
	}

	SaveUser(clientId, 0);
	return true;
}