#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestEhre(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	static BYTE valueAdd[3] = { 70, 69, 42 };
	static BYTE initialAdd[3] = { 2, 2, 10 };

	bool sucess = false;

	st_Mob *player = &Mob[clientId].Mobs.Player;

	for (int i = 0; i < 7; i++)
	{
		if (p->slot[i] == -1)
		{
			memset(&p->items[i], 0, sizeof st_Item);
			continue;
		}

		if ((p->slot[i] < 0 || p->slot[i] >= 60))
		{
			Log(clientId, LOG_HACK, "Banido por enviar índice inválido - NPC Ehre - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar índice inválido - NPC Ehre - %d", player->Name, p->slot[i]);

			SendCarry(clientId);
			return true;
		}

		if (memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Ehre - %d", p->items[i].Index);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Ehre - %d", player->Name, p->items[i].Index);

			SendCarry(clientId);
			return true;
		}

		for (int y = 0; y < 7; y++)
		{
			if (y == i)
				continue;

			if (p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Ehre - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Ehre - %d", player->Name, p->items[i].Index);

				CloseUser(clientId);
				return false;
			}
		}
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	for (int i = 0; i < 7; i++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Ehre - %d - Sem item", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Ehre - %d - %s %s - %hhd", i, ItemList[p->items[i].Index].Name, p->items[i].toString().c_str(), p->slot[i]);
	}

	st_Item *items = p->items;
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

	if (items[2].Index == 633 || items[2].Index == 632 || items[2].Index == 631)
	{
		int sanc = GetItemSanc(&player->Inventory[p->slot[2]]);

		if (sanc < 9)
		{
			SendClientMessage(clientId, "Pedra [F] precisa estar +9");

			return true;
		}

		int hp = GetItemAbilityNoSanc(&p->items[2], EF_HPADD2);
		int mp = GetItemAbilityNoSanc(&p->items[2], EF_MPADD2);
		int critico = GetItemAbilityNoSanc(&p->items[2], EF_CRITICAL);

		for (int i = 0; i < 2; ++i)
		{
			if (items[i].Index == 661)
				mp += 2;
			if (items[i].Index == 662)
				hp += 2;
			if (items[i].Index == 663)
				critico += 10;
		}

		if (hp > 20 || mp > 20 || critico > 100 || (hp > 10 && mp > 10) || (hp > 10 && critico >= 50) || (hp > 10 && mp > 10) || (mp > 10 && critico > 50) || (hp > 10 && critico != 0) || (hp > 10 && (mp != 0 || critico != 0)) || (mp > 10 && (critico != 0 || hp != 0)) || (critico > 50 && (mp != 0 || hp != 0)) || (mp != 0 && hp != 0 && critico != 0))
		{
			SendClientMessage(clientId, "Composição incorreta");

			return true;
		}

		int _rand = Rand() % 101;
		for (int i = 0; i < 3; i++)
		{
			if (items[0].Index == (661 + i))
			{
				int add = (items[0].Index - 661);
				int value = initialAdd[add];
				for (int x = 0; x < 3; x++)
					if (items[2].Effect[x].Index == valueAdd[add])
						value += items[2].Effect[x].Value;

				if (value == initialAdd[add])
				{
					int x;
					for (x = 0; x < 3; x++)
					{
						if (items[2].Effect[x].Index == 0)
						{
							value = initialAdd[add];
							break;
						}
					}

					if (x == 3)
					{
						SendClientMessage(clientId, "Item impossível de refinar...");

						return true;
					}
				}

				// rate
				if (_rand <= 50)
				{
					sucess = true;

					if (value == initialAdd[add])
					{
						for (int x = 0; x < 3; x++)
						{
							if (items[2].Effect[x].Index == 0)
							{
								items[2].Effect[x].Index = valueAdd[add];
								items[2].Effect[x].Value = initialAdd[add];

								break;
							}
						}
					}
					else
					{
						for (int x = 0; x < 3; x++)
						{
							if (items[2].Effect[x].Index == valueAdd[add])
							{
								items[2].Effect[x].Index = valueAdd[add];
								items[2].Effect[x].Value += initialAdd[add];

								break;
							}
						}
					}
				}
			}

			if (items[1].Index == (661 + i))
			{
				int add = (items[1].Index - 661);
				int value = initialAdd[add];
				for (int x = 0; x < 3; x++)
					if (items[2].Effect[x].Index == valueAdd[add])
						value += items[2].Effect[x].Value;

				if (value == initialAdd[add])
				{
					int x;
					for (x = 0; x < 3; x++)
					{
						if (items[2].Effect[x].Index == 0)
						{
							value = initialAdd[add];
							break;
						}
					}

					if (x == 3)
					{
						SendClientMessage(clientId, "Item impossível de refinar...");

						return true;
					}
				}

				if ((valueAdd[add] != 42 && value > 20) || (valueAdd[add] == 42 && value > 100))
				{
					SendClientMessage(clientId, "Item com seu adicional máximo...");

					return true;
				}

				// rate
				if (_rand <= 50) // 35
				{
					sucess = true;

					if (value == initialAdd[add])
					{
						for (int x = 0; x < 3; x++)
						{
							if (items[2].Effect[x].Index == 0)
							{
								items[2].Effect[x].Index = valueAdd[add];
								items[2].Effect[x].Value = initialAdd[add];

								break;
							}
						}
					}
					else
					{
						for (int x = 0; x < 3; x++)
						{
							if (items[2].Effect[x].Index == valueAdd[add])
							{
								items[2].Effect[x].Index = valueAdd[add];
								items[2].Effect[x].Value += initialAdd[add];

								break;
							}
						}
					}
				}
			}
		}

		SendClientMessage(clientId, (!sucess) ? "Houve uma falha na composição %d/50" : "Composição concluída com sucesso %d/50", _rand);

		memset(&player->Inventory[p->slot[0]], 0, sizeof st_Item);
		memset(&player->Inventory[p->slot[1]], 0, sizeof st_Item);

		Log(clientId, LOG_COMP, "%s da Pedra Espiritual [F] - %s - %d/50", (!sucess) ? "Falha na composição" : "Sucesso na composição", items[2].toString().c_str(), _rand);
		LogPlayer(clientId, "%s da transferência de adicional para Pedra Espiritual [F]", (!sucess) ? "Falha na composição" : "Sucesso na composição");

		SendNotice(".%s obteve %s na transferência de adicional para Pedra Espiritual [F]", Mob[clientId].Mobs.Player.Name, (!sucess) ? "falha" : "sucesso");
		Log(SERVER_SIDE, LOG_INGAME, "[%s] %s da Pedra Espiritual [F] - %s - %d/50", User.Username, (!sucess) ? "Falha na composição" : "Sucesso na composição", items[2].toString().c_str(), _rand);

		if (sucess)
			memcpy(&player->Inventory[p->slot[2]], &items[2], sizeof st_Item);
		else
			memset(&player->Inventory[p->slot[2]], 0, sizeof st_Item);

		for (int i = 0; i < 3; i++)
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}
	else if (((items[0].Index == 697 && items[1].Index == 697) || (items[0].Index == 4717 && items[1].Index == 4717)) && (items[2].Index == 3338 || items[2].Index == 4864))
	{// Refinação de Refinação Abençoada
		if (Mob[clientId].Mobs.Player.Equip[0].EFV2 <= ARCH)
		{
			SendClientMessage(clientId, "Disponivel apenas para celestiais ou superior");

			return true;
		}

		if (player->bStatus.Level < 39)
		{
			SendClientMessage(clientId, "O level mínimo é o 40");

			return true;
		}

		int sanc = GetItemSanc(&player->Inventory[p->slot[2]]);
		if (sanc >= 9)
		{
			SendClientMessage(clientId, "O item já está em sua refinação máxima");

			return true;
		}

		int chance_total = 0;
		if (player->bStatus.Level >= 39 && player->bStatus.Level <= 149)
			chance_total = sServer.RateRef[0];
		else if (player->bStatus.Level > 149 && player->bStatus.Level <= 169)
			chance_total = sServer.RateRef[1];
		else if (player->bStatus.Level >= 170 && player->bStatus.Level <= 179)
			chance_total = sServer.RateRef[2];
		else if (player->bStatus.Level >= 180 && player->bStatus.Level <= 190)
			chance_total = sServer.RateRef[3];
		else if (player->bStatus.Level > 190)
			chance_total = sServer.RateRef[4];
		else
			return true;

		if (player->Exp < 10000000)
		{
			SendClientMessage(clientId, "Erro - Contate a administração #3000");

			return true;
		}

		int _rand = Rand() % 101;
		if (_rand <= chance_total)
		{
			SetItemSanc(&player->Inventory[p->slot[2]], sanc + 1, 0);

			SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
		}
		else
			SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);

		Log(clientId, LOG_COMP, "%s na composição %s - %d/%d - %lld", (_rand < chance_total) ? "Sucesso" : "Falha", items[2].toString().c_str(), _rand, chance_total, player->Exp);
		LogPlayer(clientId, "%s na refinação da Refinação Abençoada para +%d", (_rand < chance_total) ? "Sucesso" : "Falha", sanc + 10);

		if (items[0].Index == 697)
		{
			Log(clientId, LOG_COMP, "Consumindo experiência");
			player->Exp = (player->Exp - 10000000);

			while (player->bStatus.Level > 0 && player->Exp < g_pNextLevel[3][player->bStatus.Level - 1])
			{
				player->bStatus.Level -= 1;
				player->Status.Level -= 1;
			}
		}
		else
			Log(clientId, LOG_COMP, "Não consumiu a experiência");

		Mob[clientId].GetCurrentScore(clientId);

		SendScore(clientId);
		SendEtc(clientId);

		memset(&player->Inventory[p->slot[0]], 0, sizeof st_Item);
		memset(&player->Inventory[p->slot[1]], 0, sizeof st_Item);

		for (int i = 0; i < 3; i++)
			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
	}
	else if (p->items[0].Index == 3252 && GetItemAmount(&p->items[0]) == 10 && p->items[1].Index == 3253 && GetItemAmount(&p->items[1]) == 5)
	{
		if (player->Gold < 250000000)
		{
			SendClientMessage(clientId, "São necessários 250 milhões de gold para a composição");

			return true;
		}

		// Remove os itens, independente se deu certo ou não
		for (int i = 0; i < 2; i++)
		{
			player->Inventory[p->slot[i]] = st_Item{};

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		player->Gold -= 250000000;
		SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

		// Seta o item no inventário
		player->Inventory[p->slot[0]].Index = 3250;

		// Atualiza o inventário
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_INGAME, "Sucesso na composição da Benção de Âmago");
		LogPlayer(clientId, "Sucesso na composição da Benção de Âmago");
	}
	else if ((items[0].Index >= 5110 && items[0].Index <= 5133) && (items[1].Index >= 5110 && items[1].Index <= 5133) &&
		items[2].Index == 413)
	{
		int _amount = GetItemAmount(&items[2]);
		if (_amount != 10)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		for (int i = 0; i < 3; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		player->Inventory[p->slot[2]].Index = 4148;

		player->Inventory[p->slot[2]].EF1 = EF_AMOUNT;
		player->Inventory[p->slot[2]].EFV1 = 10;

		SendItem(clientId, SlotType::Inv, p->slot[2], &player->Inventory[p->slot[2]]);

		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_INGAME, "Composição realizada para Pedra Misteriosa");
		LogPlayer(clientId, "Composição realizada para Pedra Misteriosa");
	}
	else if ((items[0].Index >= 2360 && items[0].Index <= 2389) && (((items[1].Index >= 4190 && items[1].Index <= 4219) || (items[1].Index >= 5706 && items[1].Index <= 5725)) || items[1].Index == 4899))
	{
		INT32 traje = items[0].Effect[2].Value;
		if ((traje >= 11 && items[1].Index != 4899) || (traje < 11 && items[1].Index == 4899))
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		if (Mob[clientId].Mobs.Player.Gold < 1000000)
		{
			SendClientMessage(clientId, "Gold insuficiente");

			return true;
		}

		if (items[1].Index == 4899)
			traje = 0;
		else
			traje = (11 + (items[1].Index - 4190));

		switch (items[1].Index)
		{
		case 4200:
			traje = 22;
			break;
		case 4201:
			traje = 24;
			break;
		case 4202:
			traje = 25;
			break;
		case 4204:
			traje = 26;
			break;
		case 4205:
			traje = 27;
			break;
		case 4206:
			traje = 28;
			break;
		case 4207:
			traje = 29;
			break;
		case 4208:
			traje = 30;
			break;
		case 4209:
			traje = 31;
			break;
		case 4210:
			traje = 32;
			break;
		case 4211:
			traje = 33;
			break;
		case 4212:
			traje = 34;
			break;
		case 4213:
			traje = 35;
			break;
		case 4214:
			traje = 36;
			break;
		case 4215:
			traje = 37;
			break;
		case 4216:
			traje = 38;
			break;
		case 4217:
			traje = 39;
			break;
		case 4218:	
			traje = 40;
			break;
		case 4219:
			traje = 41;
			break;
		case 5706:
			traje = 42;
			break;
		case 5707:
			traje = 43;
			break;
		case 5708:
			traje = 44;
			break;
		case 5709:
			traje = 45;
			break;
		// Grifo de Lava
		case 5710:
			traje = 46;
			break;
		// Grifo de Gelo
		case 5711:
			traje = 47;
			break;
		// Grifo de Esmeralda
		case 5712:
			traje = 48;
			break;
		// Grifo de dOURADO
		case 5713:
			traje = 49;
			break;
		// Gargula de Pedra
		case 5714:
			traje = 50;
			break;
		// Gargula Dourado
		case 5715:
			traje = 51;
			break;
		case 5716:
			traje = 52;
			break;
		case 5717:
			traje = 53;
			break;
		case 5718:
			traje = 54;
			break;
		case 5719:
			traje = 55;
			break;
		// Nidhogg (Terra)
		case 5720:
			traje = 56;
			break;
		// Nidhogg (Infernal)
		case 5721:
			traje = 57;
			break;
		// Nidhogg (Gelo)
		case 5722:
			traje = 58;
			break;
		case 5723:
			traje = 59;
			break;
		case 5724:
			traje = 60;
			break;
		case 5725:
			traje = 61;
			break;
		}

		int mountId = player->Inventory[p->slot[0]].Index;
		if ((traje >= 29 && traje <= 34) || (traje >= 46 && traje <= 49) || (traje >= 50 && traje <= 51))
		{
			if (mountId != 2384 && mountId != 2385 && mountId != 2386)
			{
				SendClientMessage(clientId, "Não disponível para esta montaria");

				return true;
			}
		}
		else if (traje >= 35 && traje <= 37)
		{
			if (mountId != 2381 && mountId != 2382 && mountId != 2383)
			{
				SendClientMessage(clientId, "Não disponível para esta montaria");

				return true;
			}
		}
		else if (traje == 40 || traje == 41)
		{
			if (mountId != 2376 && mountId != 2378)
			{
				SendClientMessage(clientId, "Não disponível para esta montaria");

				return true;
			}
		}
		else if (traje == 38 || traje == 39 || (traje >= 56 && traje <= 58))
		{
			if (mountId != 2377)
			{
				SendClientMessage(clientId, "Não disponível para esta montaria");

				return true;
			}
		}
		else if (traje >= 42 && traje <= 45)
		{
			if (mountId != 2366 && mountId != 2367 && mountId != 2368 && mountId != 2369 && mountId != 2371 && mountId != 2372 && mountId != 2373 && mountId != 2374 && mountId != 2370 && mountId != 2375)
			{
				SendClientMessage(clientId, "Não disponível para esta montaria");

				return true;
			}
		}

		player->Inventory[p->slot[0]].Effect[2].Value = traje;

		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		memset(&player->Inventory[p->slot[1]], 0, sizeof st_Item);
		SendItem(clientId, SlotType::Inv, p->slot[1], &player->Inventory[p->slot[1]]);

		Mob[clientId].Mobs.Player.Gold -= 1000000;
		SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
	}
	else if (items[0].Index == 4862)
	{
		for (int i = 1; i < 5; ++i)
		{
			if (items[i].Index < 2441 || items[i].Index > 2444)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

				return true;
			}
		}

		if (Mob[clientId].Mobs.Player.Gold < 100000000)
		{
			SendClientMessage(clientId, "Gold insuficiente");

			return true;
		}

		for (int i = 0; i < 5; i++)
		{
			Log(clientId, LOG_COMP, "Consumido o item %s %s", ItemList[player->Inventory[p->slot[i]].Index].Name, Mob[clientId].Mobs.Player.Inventory[p->slot[i]].toString().c_str());

			Mob[clientId].Mobs.Player.Inventory[p->slot[i]] = st_Item{};
			SendItem(clientId, SlotType::Inv, p->slot[i], &Mob[clientId].Mobs.Player.Inventory[p->slot[i]]);
		}

		int rand = Rand() % 101;
		int rate = 75;
		if (rand <= rate)
		{
			Mob[clientId].Mobs.Player.Inventory[p->slot[0]].Index = 4860;
			SendItem(clientId, SlotType::Inv, p->slot[0], &Mob[clientId].Mobs.Player.Inventory[p->slot[0]]);
			SendClientMessage(clientId, "Sucesso na composição %d/%d", rand, rate);

			Log(clientId, LOG_INGAME, "Composto com sucesso %s %d/75", ItemList[4860].Name, rand);
		}
		else
		{
			Log(clientId, LOG_INGAME, "Falhou na composição de %s %d/75", ItemList[4860].Name, rand);
			SendClientMessage(clientId, "Falha na composição %d/%d", rand, rate);
		}
		Mob[clientId].Mobs.Player.Gold -= 100000000;
		SendSignalParm(clientId, clientId, 0x3AF, player->Gold);
	}
	else if (items[0].Index >= 591 && items[0].Index <= 595)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (items[i].Index < 591 && items[i].Index > 595)
			{
				SendClientMessage(clientId, "Composição incorreta");

				return true;
			}
		}

		for (int i = 0; i < 3; i++)
		{
			Log(clientId, LOG_COMP, "Consumido o item %s %s", ItemList[player->Inventory[p->slot[i]].Index].Name, Mob[clientId].Mobs.Player.Inventory[p->slot[i]].toString().c_str());

			Mob[clientId].Mobs.Player.Inventory[p->slot[i]] = st_Item{};
			SendItem(clientId, SlotType::Inv, p->slot[i], &Mob[clientId].Mobs.Player.Inventory[p->slot[i]]);
		}

		int rate = 75;
		int rand = Rand() % 101;
		if (rand <= rate)
		{
			int totalPowder = 0;
			for (int i = 0; i < 3; i++)
				totalPowder += (Rand() % 3) + 1;

			Mob[clientId].Mobs.Player.Inventory[p->slot[0]].Index = 4719;
			Mob[clientId].Mobs.Player.Inventory[p->slot[0]].EF1 = EF_AMOUNT;
			Mob[clientId].Mobs.Player.Inventory[p->slot[0]].EFV1 = totalPowder;

			Log(clientId, LOG_INGAME, "Recebeu %d Pó de Brinco", totalPowder);

			SendItem(clientId, SlotType::Inv, p->slot[0], &Mob[clientId].Mobs.Player.Inventory[p->slot[0]]);
			SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
		}
		else
		{
			SendClientMessage(clientId, "Falha na composição %d/%d", rand, rate);

			Log(clientId, LOG_INGAME, "Falha na criação do Pó de Brinco. %d/%d", rand, rate);
		}
	}
	else if (items[0].Index == 4684)
	{
		if (items[1].Index != 5121 || items[2].Index != 5119 || items[3].Index != 5131 || items[4].Index != 5120 || items[5].Index != 5133 || items[6].Index != 5128)
		{
			SendClientMessage(clientId, "Composição incorreta");

			return true;
		}

		for (int i = 0; i < 7; i++)
		{
			Log(clientId, LOG_COMP, "Consumido o item %s %s", ItemList[player->Inventory[p->slot[i]].Index].Name, Mob[clientId].Mobs.Player.Inventory[p->slot[i]].toString().c_str());

			Mob[clientId].Mobs.Player.Inventory[p->slot[i]] = st_Item{};
			SendItem(clientId, SlotType::Inv, p->slot[i], &Mob[clientId].Mobs.Player.Inventory[p->slot[i]]);
		}

		Mob[clientId].Mobs.Player.Inventory[p->slot[0]].Index = 4683;
		SendItem(clientId, SlotType::Inv, p->slot[0], &Mob[clientId].Mobs.Player.Inventory[p->slot[0]]);
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
	}
	else
	{
		static WORD dwSoul[11][3] = {
			{2441, 2441, 2441}, {2442, 2442, 2442}, {2443, 2443, 2443}, {2444, 2444, 2444},
			{2441, 2442, 2443}, {2441, 2443, 2444}, {2442, 2443, 2444}, {2442, 2441, 2443},
			{2443, 2442, 2444}, {2444, 2441, 2443}, {2441, 2443, 2442}
		};
		int i = 0;
		for (i = 0; i < 11; i++)
		{
			if (items[0].Index == dwSoul[i][0] && items[1].Index == dwSoul[i][1] && items[2].Index == dwSoul[i][2])
			{
				Mob[clientId].Mobs.Soul = i;

				SendClientMessage(clientId, "Limite da Alma configurada com sucesso.");

				for (int x = 0; x < 3; x++)
				{
					memset(&player->Inventory[p->slot[x]], 0, sizeof st_Item);

					SendItem(clientId, SlotType::Inv, p->slot[x], &player->Inventory[p->slot[x]]);
				}

				break;
			}
		}

		if (i == 10)
		{
			SendClientMessage(clientId, "Composição incorreta");

			return true;
		}
	}

	SaveUser(clientId, 0);
	return true;
}