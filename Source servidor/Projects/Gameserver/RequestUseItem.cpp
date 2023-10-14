#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "UOD_EventManager.h"
#include "UOD_DuelLetter.h"
#include "CNPCGener.h"
#include "ItemsAction.h"
#include <algorithm>
#include <random>

bool CUser::RequestUseItem(PacketHeader* Header)
{
	static const INT32 g_pItemRest[7][6] =
	{
		{2363, 2364, 2365, 0, 0, 0},
		{2366, 2367, 2368, 2372, 2373, 2371},
		{2369, 2370, 2374, 2375, 2387, 0},
		{2377, 2376, 2378, 0, 0, 0},
		{2381, 2382, 2383, 2388, 0, 0},
		{2384, 2385, 2386, 0, 0, 0},
		{2379, 2380, 0, 0, 0, 0}
	};

	p373* p = (p373*)(Header);
	if (p->SrcSlot < 0 || p->SrcSlot > 60 || p->DstSlot < 0 || p->DstSlot > 60 || p->DstType < 0 || p->DstType > 2 || p->SrcType < 0 || p->SrcType > 2)
		return false;

	st_Item* srcItem = GetItemPointer(clientId, p->SrcType, p->SrcSlot);
	if (srcItem == NULL)
		return false;

	if (Status != USER_PLAY)
	{
		Log(clientId, LOG_INGAME, "Enviado pacote 0x373 (RequestUseItem) não estando dentro do jogo. Status: %d", Status);
		return false;
	}
	if (Mob[clientId].Mobs.Player.Status.curHP <= 0)
	{
		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		return false;
	}

	if (p->PosX < 0 || p->PosX >= 4096 || p->PosY < 0 || p->PosY >= 4096)
	{
		Log(clientId, LOG_HACK, "Enviado pacote com posição %hux %huy", p->PosX, p->PosY);
		Log(SERVER_SIDE, LOG_HACK, "[%s] -  Enviado pacote com posição %hux %huy", User.Username, p->PosX, p->PosY);

		return false;
	}

	INT32 _volatile = GetItemAbility(srcItem, EF_VOLATILE); // LOCAL_929
	INT32 itemAmount = GetItemAmount(srcItem); // local930

	// Evita usar um item que está no trade
	if (Trade.ClientId != 0)
		RemoveTrade(clientId);

	if (_volatile != 2 && _volatile != 15)
	{
		if (User.Block.Blocked)
		{
			SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}
	}

	if (srcItem->Index <= 0 || srcItem->Index >= MAX_ITEMLIST)
	{
		SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

		return true;
	}

	if (_volatile != 1 && _volatile != 15 && _volatile != 191 && srcItem->Index != 0)
	{
		Log(clientId, LOG_INGAME, "Item %s [%d] [%d %d %d %d %d %d] utilizado em %hux %huy", ItemList[srcItem->Index].Name, srcItem->Index, srcItem->EF1, srcItem->EFV1, srcItem->EF2, srcItem->EFV2, srcItem->EF3, srcItem->EFV3,
			p->PosX, p->PosY);
		LogPlayer(clientId, "Item %s utilizado", ItemList[srcItem->Index].Name);

		auto now = std::chrono::steady_clock::now();
		auto timeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastUsedItem);
		if (timeInMs < 169ms)
		{
			Log(clientId, LOG_HACK, "O usuário usou itens muito rapidamente. Tempo para usar o item: %lld. Item: %s", timeInMs.count(), ItemList[srcItem->Index].Name);
			Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuário usou itens muito rapidamente. Tempo para usar o item: %lld. Item: %s", User.Username, timeInMs.count(), ItemList[srcItem->Index].Name);
		}

		Times.LastUsedItem = now;

		auto[isFast, time] = CheckIfIsTooFast(srcItem, p->SrcSlot);
		if (isFast)
		{
			Log(clientId, LOG_HACK, "O usuário usou o item %s em %dms.", srcItem->toString().c_str(), time);
			Log(SERVER_SIDE, LOG_HACK, "O usuário %s usou o item %s em %dms.", User.Username, srcItem->toString().c_str(), time);
		}
	}

	INT32 itemId = srcItem->Index;
#pragma region PAC ITENS
	if (_volatile == 210)
	{
		INT32 index = -1;
		for (INT32 i = 0; i < MAX_PACITEM; i++)
		{
			if (g_pPacItem[i].ItemId == srcItem->Index)
			{
				index = i;
				break;
			}
		}

		if (index < 0 || index >= MAX_PACITEM)
		{
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

			return true;
		}

		stPacItem* pac = &g_pPacItem[index];
		INT32 total = 0;

		for (INT32 i = 0; i < 8; i++)
			total += pac->Amount[i];

		if (GetInventoryAmount(clientId, 0) < total)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		// apaga
		AmountMinus(srcItem);

		for (INT32 i = 0; i < 8; i++)
		{
			if (pac->Item[i].Index <= 0 || pac->Item[i].Index >= MAX_ITEMLIST || pac->Amount[i] <= 0)
				continue;

			for (INT32 t = 0; t < pac->Amount[i]; t++)
			{
				INT32 slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					Log(clientId, LOG_INGAME, "Não foi possível encontrar espaço para o pacIndex %d. I: %d. Amount atual: %d", index, i, t);

					break;
				}

				memcpy(&Mob[clientId].Mobs.Player.Inventory[slotId], &pac->Item[i], sizeof st_Item);
			}
		}

		SendCarry(clientId);
	}
#pragma endregion
#pragma region POTS
	else if (_volatile == 1) //0042DE14
	{
		for (int i = 0; i < 32; i++)
		{
			if (Mob[clientId].Mobs.Affects[i].Index == 32)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}
		}

		INT32 countHeal; // LOCAL931
		INT32 hpHeal = GetItemAbility(srcItem, EF_HP); // LOCAL932

		int potionBonus = Mob[clientId].PotionBonus;
		if (potionBonus != 0)
			hpHeal += (hpHeal * potionBonus / 100);

		if (hpHeal != 0)
		{
			countHeal = Users[clientId].Potion.CountHp;
			countHeal += hpHeal;

			if (countHeal > Mob[clientId].Mobs.Player.Status.maxHP)
				countHeal = Mob[clientId].Mobs.Player.Status.maxHP;

			Users[clientId].Potion.CountHp = countHeal;
		}

		INT32 mpHeal = GetItemAbility(srcItem, EF_MP);
		if (potionBonus != 0)
			mpHeal += (mpHeal * potionBonus / 100);

		if (mpHeal != 0)
		{
			countHeal = Users[clientId].Potion.CountMp;
			countHeal += mpHeal;

			if (countHeal > Mob[clientId].Mobs.Player.Status.maxMP)
				countHeal = Mob[clientId].Mobs.Player.Status.maxMP;

			Users[clientId].Potion.CountMp = countHeal;
		}

		Users[clientId].Potion.bQuaff = 1;

		SendSetHpMp(clientId);
		AmountMinus(srcItem);

		return true;
	}
#pragma endregion
#pragma region POEIRAS
	else if ((_volatile == 4 || _volatile == 5) && srcItem->Index != 4141)
	{ // poeira - 0042DFB2
		static const int Rate[2][7][9] =
		{
			{
				{100, 100, 100, 100, 90, 60, 0, 0, 0 }, // A
				{100, 100, 100, 100, 90, 50, 0, 0, 0 }, // B
				{100, 100, 100, 80, 75, 50, 0, 0, 0 }, // C
				{100, 100, 95, 85, 80, 80, 0, 0, 0 }, // D
				{100, 95, 95, 85, 80, 80, 0, 0, 0 }, // E
				{100, 90, 80, 80, 80, 80, 0, 0, 0 }, // F
				{100, 90, 80, 80, 80, 80, 0, 0, 0 }, // Itens archs
			},
			{
				{100, 100, 100, 100, 100, 100, 75, 55, 40}, // A
				{100, 100, 100, 100, 100, 100, 75, 45, 35}, // B
				{100, 100, 100, 100, 100, 100, 75, 40, 30}, // C
				{100, 100, 100, 100, 100, 100, 75, 50, 45}, // D
				{100, 100, 100, 100, 100, 100, 65, 50, 45}, // E
				{100, 100, 100, 100, 100, 100, 55, 45, 45}, // F
				{100, 100, 100, 100, 100, 100, 55, 45, 45}, // Itens archs
			}
		};

		static const int Fail[9] = {
			0, 5, 5, 4, 4, 3, 3, 2, 1
		};

		static const int RateCele[15] = { 90, 85, 85, 80, 70, 60, 50, 45, 40, 35, 25, 15, 10, 7, 5 };
		static const short RateClosed[9] = { 90, 90, 80, 70, 65, 60, 55, 50, 30 };

		st_Mob* player = &Mob[clientId].Mobs.Player;

		st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
		if (p->DstSlot == 15 && p->DstType == 0)
		{
			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);
				return true;
			}
		}

		// Tentando refinar a face.
		if (p->DstSlot == 0 && p->DstType == 0)
			return true;

		if (dstItem->Index == 769)
		{
			for (int i = 0; i < 5; i++)
			{
				if (Mob[clientId].Target.X >= g_pCityZone[i].war_min_x && Mob[clientId].Target.X <= g_pCityZone[i].war_max_x && Mob[clientId].Target.Y >= g_pCityZone[i].war_min_y && Mob[clientId].Target.Y <= g_pCityZone[i].war_max_y)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine_War]);
					return true;
				}
			}
		}

		if (p->DstType == (unsigned int)SlotType::Equip && p->DstSlot == 12)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		switch (dstItem->Index)
		{
		case 3994:
		case 3993:
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
		break;
		}

		if (dstItem->Index == NUCLEO_LAC_ITEMID && _volatile == 4)
		{
			SendClientMessage(clientId, "Somente com poeiras de lac.");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			return true;
		}

		// Núcleo de lac
		if (dstItem->Index == NUCLEO_LAC_ITEMID && _volatile == 5)
		{
			// Inicializa o núcleo
			if (dstItem->Effect[0].Index != EF_AMOUNT)
			{
				dstItem->Effect[0].Index = EF_AMOUNT;
				dstItem->Effect[0].Value = 0;
			}

			// Núcleo já incializado.
			if (dstItem->Effect[0].Index == EF_AMOUNT)
			{
				if (dstItem->Effect[0].Value >= 100)
				{
					SendClientMessage(clientId, "O núcleo já está totalmente carregado!");

					SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				dstItem->Effect[0].Value += 2;
				SendClientMessage(clientId, "Núcleo refinado com sucesso!");

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				SendEmotion(clientId, 14, 3);
				AmountMinus(srcItem);
				return true;
			}
		}

		if (p->DstSlot == 13)
		{
			if (dstItem->Index >= 3397 && dstItem->Index <= 3406)
			{
				dstItem->Index += 10;

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				AmountMinus(srcItem);
				return true;
			}
		}

		if (p->DstSlot == 14 && p->DstType == 0)
		{ // Ovo 
			if (player->Equip[p->DstSlot].Index >= 2300 && player->Equip[p->DstSlot].Index < 2330)
			{
				int add = player->Equip[14].EFV3;

				if (add != 0)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					SendClientMessage(clientId, g_pLanguageString[_NN_Incu_Wait_More]);

					return true;
				}

				if (player->Equip[14].EFV1 <= 1)
				{
					player->Equip[p->DstSlot].Index += 30;

					player->Equip[p->DstSlot].EF2 = 1;
					player->Equip[p->DstSlot].EFV2 = (Rand() % 20 + 10);

					player->Equip[p->DstSlot].EF3 = 30;
					player->Equip[p->DstSlot].EFV1 = 100;
					player->Equip[p->DstSlot].EF1 = 100;
					player->Equip[p->DstSlot].EFV3 = 1;

					MountProcess(clientId, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);
				}
				else
				{
					if (player->Equip[14].EFV1 > 1)
						player->Equip[14].EFV1--;

					player->Equip[14].EF3 = 84;
					player->Equip[14].EFV3 = Rand() % 8 + 4;

				}

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendEmotion(clientId, 14, 3);

				AmountMinus(srcItem);
			}
			else
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			}

			return true;
		}

		if (p->DstType == 0 && p->DstSlot == 11)
		{
			if (dstItem->Index >= 1752 && dstItem->Index < 1760)
			{
				int _rand = Rand() % 101;
				if (_rand <= 80)
				{
					dstItem->Index -= 8;

					Log(clientId, LOG_INGAME, "Pedra %s foi aberta", ItemList[dstItem->Index + 8].Name);
					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);
				}
				else
				{
					Log(clientId, LOG_INGAME, "Pedra %s falhou para abrir", ItemList[dstItem->Index + 8].Name);

					SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);
				}

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendEmotion(clientId, 14, 3);

				AmountMinus(srcItem);

				return true;
			}
		}

		int sanc = GetItemSanc(dstItem);
		if (p->DstType == 0 && p->DstSlot == 8 && _volatile == 5)
		{
			if (dstItem->Index >= 591 && dstItem->Index <= 595)
			{
				if (sanc >= 9 && sanc <= 14)
				{
					int _rand = Rand() % 101;
					SendEmotion(clientId, 14, 3);

					int protectionSlot = GetFirstSlot(clientId, 4611);

					int rate = 90;
					if (sanc >= 11)
						rate = 95;

					if (_rand > rate)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

						Log(clientId, LOG_INGAME, "Sucesso - Item %s para %d %s %d/%d", ItemList[dstItem->Index].Name, sanc + 1, dstItem->toString().c_str(), _rand, rate);

						SetItemSanc(dstItem, sanc + 1, 0);
					}
					else
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

						if (protectionSlot == -1)
						{
							Log(clientId, LOG_INGAME, "Destruído - Item %s para %d %s %d/%d", ItemList[dstItem->Index].Name, sanc + 1, dstItem->toString().c_str(), _rand, rate);

							memset(dstItem, 0, sizeof st_Item);
						}
						else
							Log(clientId, LOG_INGAME, "Falhou - Item %s para %d %s %d/%d", ItemList[dstItem->Index].Name, sanc + 1, dstItem->toString().c_str(), _rand, rate);
					}

					if (protectionSlot != -1)
					{
						AmountMinus(&Mob[clientId].Mobs.Player.Inventory[protectionSlot]);

						SendItem(clientId, SlotType::Inv, protectionSlot, &Mob[clientId].Mobs.Player.Inventory[protectionSlot]);
						Log(clientId, LOG_INGAME, "Consumido Emblema da Proteção (Brinco), evitando a perda do item");
					}

					SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
					AmountMinus(srcItem);
					return true;
				}
			}
		}

		// Cythera Desencantada - Se falhar, volta pra +0
		if (dstItem->Index == 3508)
		{
			if (_volatile != 5)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 sanc = GetItemSanc(dstItem);
			if (sanc >= 9)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			static INT32 g_pCytheraRate[] = { 80, 70, 60, 50, 50, 50, 50, 40, 30, 30 };

			INT32 _rate = g_pCytheraRate[sanc],
				_rand = Rand() % 100;

			if (_rand <= _rate)
			{
				SetItemSanc(dstItem, sanc + 1, 0);

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

				if (sanc == 8)
					SendNotice(".%s refinou Cythera Desencantada para +9", Mob[clientId].Mobs.Player.Name);

				Log(clientId, LOG_INGAME, "Sucesso na refinação de %s para %d - [%d] [%d %d %d %d %d %d] ", ItemList[dstItem->Index].Name, sanc + 1, dstItem->Index, dstItem->Effect[0].Index, dstItem->Effect[0].Value,
					dstItem->Effect[1].Index, dstItem->Effect[1].Value, dstItem->Effect[2].Index, dstItem->Effect[2].Value);
			}
			else if (sanc != 6)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

				SetItemSanc(dstItem, 0, 0);
				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				Log(clientId, LOG_INGAME, "Falha na refinação de %s para %d - [%d] [%d %d %d %d %d %d] ", ItemList[dstItem->Index].Name, sanc + 1, dstItem->Index, dstItem->Effect[0].Index, dstItem->Effect[0].Value,
					dstItem->Effect[1].Index, dstItem->Effect[1].Value, dstItem->Effect[2].Index, dstItem->Effect[2].Value);
			}
			else
			{
				memset(dstItem, 0, sizeof st_Item);
				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				SendClientMessage(clientId, "Item destruído");

				Log(clientId, LOG_INGAME, "Falha na refinação de %s para %d - [%d] [%d %d %d %d %d %d] - ITEM DESTRUÍDO", ItemList[dstItem->Index].Name, sanc + 1, dstItem->Index, dstItem->Effect[0].Index, dstItem->Effect[0].Value,
					dstItem->Effect[1].Index, dstItem->Effect[1].Value, dstItem->Effect[2].Index, dstItem->Effect[2].Value);

				LogPlayer(clientId, "Destruiu o item %s tentando refinar para +7", ItemList[dstItem->Index].Name);
			}

			AmountMinus(srcItem);
			return true;
		}

		if (((dstItem->Index >= 762 && dstItem->Index <= 768)  || (dstItem->Index >= 788 && dstItem->Index <= 791)) && (sanc == 9 || sanc == 10) && _volatile == 5)
		{
			int rate = 30;
			if (sanc == 10)
				rate = 15;

			if (dstItem->Index >= 788 && dstItem->Index <= 791)
			{
				rate = 15;
				if (sanc == 10)
					rate = 6;
			}

			int _rand = Rand() % 100;
			if (_rand < rate)
			{
				SetItemSanc(dstItem, sanc + 1, 0);

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

				SendNotice(".%s refinou %s para +%d", Mob[clientId].Mobs.Player.Name, ItemList[dstItem->Index].Name, sanc + 1);

				Log(clientId, LOG_INGAME, "Sucesso na refinação de %s para %d - %s ", ItemList[dstItem->Index].Name, sanc + 1, dstItem->toString().c_str());
			}
			else
			{
				SetItemSanc(dstItem, 0, 0);

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

				SendNotice(".%s falhou %s para +%d", Mob[clientId].Mobs.Player.Name, ItemList[dstItem->Index].Name, sanc + 1);

				Log(clientId, LOG_INGAME, "Falha na refinação de %s para %d  %s", ItemList[dstItem->Index].Name, sanc + 1, dstItem->toString().c_str());
			}

			AmountMinus(srcItem);
			return true;
		}

		int _type = GetEffectValueByIndex(player->Equip[p->DstSlot].Index, EF_MOBTYPE);
		if (_volatile == 5 && _type == 3)
		{
			if (player->Equip[p->DstSlot].Index)
			{
				if (sanc == 15)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				int _rate = Rand() % 101;

				SendEmotion(clientId, 14, 3);

				if (_rate <= RateCele[sanc])
				{
					SetItemSanc(dstItem, sanc + 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Refinado %s (%d %d %d %d %d %d) para %d - %d", ItemList[dstItem->Index].Name, dstItem->EF1, dstItem->EFV1, dstItem->EF2,
						dstItem->EFV2, dstItem->EF3, dstItem->EFV3, sanc + 1, _rate);
				}
				else
				{
					if (sanc != 0)
						SetItemSanc(dstItem, sanc - 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);
				}

				AmountMinus(srcItem);
				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

				return true;
			}
		}

		if (p->DstSlot <= 7 && p->DstType == (unsigned int)SlotType::Equip && _volatile == 5 && sanc == 10)
		{
			INT32 _rate = Rand() % 101;

			AmountMinus(srcItem);
			if (_rate > 96)
			{
				INT32 i = 0;
				for (; i < 3; i++)
				{
					if (dstItem->Effect[i].Index == EF_SANC || (dstItem->Effect[i].Index >= 116 && dstItem->Effect[i].Index <= 125))
						break;
				}

				if (i != 3)
				{
					dstItem->Effect[i].Value += 4;

					SendEmotion(clientId, 14, 3);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Item %s refinado com sucesso para %d - [%d] [%d %d %d %d %d %d]", ItemList[dstItem->Index].Name, sanc + 1, dstItem->Index, dstItem->Effect[0].Index, dstItem->Effect[0].Value,
						dstItem->Effect[1].Index, dstItem->Effect[1].Value, dstItem->Effect[2].Index, dstItem->Effect[2].Value);
				}
				else
					Log(clientId, LOG_ERROR, "Não encontrado o ef 43 para refinar para +11");
			}
			else
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

				Log(clientId, LOG_INGAME, "Falha na refinação de %s para %d - [%d] [%d %d %d %d %d %d]", ItemList[dstItem->Index].Name, sanc + 1, dstItem->Index, dstItem->Effect[0].Index, dstItem->Effect[0].Value,
					dstItem->Effect[1].Index, dstItem->Effect[1].Value, dstItem->Effect[2].Index, dstItem->Effect[2].Value);
			}

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			return true;
		}

		if ((_volatile == 4 && sanc >= 6) || (_volatile == 5 && sanc >= 9))
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_More]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int i = 0;
		for (i = 0; i < 3; i++)
		{
			if (dstItem->Effect[i].Index == 0 || dstItem->Effect[i].Index == 43 || (dstItem->Effect[i].Index >= 116 && dstItem->Effect[i].Index <= 125))
				break;
		}

		if (i == 3)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int blocked = GetEffectValueByIndex(dstItem->Index, 125);
		if (blocked == 1)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}


		int _pos = ItemList[dstItem->Index].Pos;
		if (_pos == 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int fails = 0;
		for (i = 0; i < 3; i++)
		{
			if (dstItem->Effect[i].Index == 43 || (dstItem->Effect[i].Index >= 116 && dstItem->Effect[i].Index <= 125))
			{
				if (dstItem->Effect[i].Value != 0)
				{
					fails = dstItem->Effect[i].Value / 10;
					break;
				}
			}
		}

		int type = GetEffectValueByIndex(dstItem->Index, EF_UNKNOW1);
		if (type < 0 || type >= 6)
			type = 5;

		int _Fails = Fail[sanc] * fails;
		int rate = Rate[_volatile - 4][type][sanc] + _Fails;

		if (_type == 1)
			rate = Rate[_volatile - 4][6][sanc];

		int _rate = Rand() % 101;

		sItemData* item = (sItemData*)& ItemList[dstItem->Index];

		if (_rate > rate)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

			Log(clientId, LOG_INGAME, "Falha na refinação de %s para %d - %s %d/%d", ItemList[dstItem->Index].Name, sanc + 1, dstItem->toString().c_str(), _rate, rate);

			LogPlayer(clientId, "Falhou ao tentar refinar %s para %d", ItemList[dstItem->Index].Name, sanc + 1);
			SetItemSanc(dstItem, sanc, _Fails);

			SendEmotion(clientId, 15, 1);
		}
		else
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

			Log(clientId, LOG_INGAME, "Item %s refinado com sucesso para %d - %s %d/%d", ItemList[dstItem->Index].Name, sanc + 1, dstItem->toString().c_str(), _rate, rate);
			LogPlayer(clientId, "Sucesso na refinação de %s para %d", ItemList[dstItem->Index].Name, sanc + 1);

			SetItemSanc(dstItem, sanc + 1, _Fails);
		}

		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

		if (_rate > rate)
			SendEmotion(clientId, 15, 1);
		else
			SendEmotion(clientId, 14, 3);

		Mob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		AmountMinus(srcItem);

		SendEquip(clientId);
		return true;
	}
#pragma endregion
#pragma region PILULA DO PODER MÁGICO
	else if (_volatile == 6) // pilula do poder mágico
	{ // 0042E9BF
		// Retira no início do código 
		// Pois mesmo que não seja possível utilizar, o item é usado
		AmountMinus(srcItem);

		if (Mob[clientId].Mobs.Info.Pilula)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

			return true;
		}

		Mob[clientId].Mobs.Info.Pilula = true;
		Mob[clientId].GetCurrentScore(clientId);

		SendClientMessage(clientId, g_pLanguageString[_NN_Get_Skill_Point]);

		SendEtc(clientId);
		SendScore(clientId);
	}
#pragma endregion
#pragma region POEIRA DA FADA
	else if (_volatile == 7) // poeira da fada
	{ // 0042EA46
		Mob[clientId].Mobs.Player.Exp = 4000000000;

		SendClientMessage(clientId, "Recebeu 4.000.000.000 de experiência");
		Mob[clientId].GetCurrentScore(clientId);
		AmountMinus(srcItem);
		SendScore(clientId);
		SendEtc(clientId);
	}
#pragma endregion
#pragma region OLHO CRESCENTE
	else if (_volatile == 8) // olho crescente
	{ // 0042EBDD
		Mob[clientId].Mobs.Player.Exp += 3000;

		SendClientMessage(clientId, g_pLanguageString[_DN_get_D_exp], 3000);
		SendEmotion(clientId, 20, 0);

		AmountMinus(srcItem);

		Mob[clientId].CheckGetLevel();
	}
#pragma endregion
#pragma region PEDRA LE
	else if (_volatile == 9) // pedras LE
	{
		st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

		int itemIndex = srcItem->Index;
		itemIndex = itemIndex - 575;

		if (itemIndex < 0 || itemIndex > 4)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int itemPos = ItemList[dstItem->Index].Pos;
		if (itemPos < 2 || itemPos > 32)
		{
			SendClientMessage(clientId, "Utilize em equipamentos");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int itemAb = GetItemAbility(dstItem, EF_UNKNOW1);
		if (itemAb == 5)
		{
			SendClientMessage(clientId, "Não é possível utilizar nesse equipamento.");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if (GetItemSanc(dstItem) > 9)
		{
			SendClientMessage(clientId, "Não é possível utilizar em itens superiores a +9");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int UniqueType = -1;
		int nUnique = ItemList[dstItem->Index].Unique;

		if (nUnique == 5 || nUnique == 14 || nUnique == 24 || nUnique == 34)
			UniqueType = 0;

		if (nUnique == 6 || nUnique == 15 || nUnique == 25 || nUnique == 35)
			UniqueType = 1;

		if (nUnique == 7 || nUnique == 16 || nUnique == 26 || nUnique == 36)
			UniqueType = 2;

		if (nUnique == 8 || nUnique == 17 || nUnique == 27 || nUnique == 37)
			UniqueType = 3;

		if (nUnique == 10 || nUnique == 20 || nUnique == 30 || nUnique == 40)
			UniqueType = 3;

		if (UniqueType != itemIndex)
		{
			SendClientMessage(clientId, "Item inadequado");

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		int itemExtreme = ItemList[dstItem->Index].Extreme;
		if (itemExtreme <= 0 || itemExtreme >= MAX_ITEMLIST)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if ((Rand() % 100) < 50)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Fail_To_Refine]);

			Log(clientId, LOG_INGAME, "Item %s utilizado em %s - FALHOU %d/%d", ItemList[srcItem->Index].Name, dstItem->toString().c_str());

			AmountMinus(srcItem);
			return true;
		}

		int dstIndex = dstItem->Index;
		dstItem->Index = itemExtreme;

		Log(clientId, LOG_INGAME, "Item %s utilizado em %s %d [%d %d %d %d %d %d]", ItemList[srcItem->Index].Name, ItemList[dstIndex].Name, dstItem->Index, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

		SendEquip(clientId);
		AmountMinus(srcItem);

		Mob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		SendItem(clientId, SlotType::Equip, p->DstSlot, dstItem);
	}

#pragma endregion
#pragma region ANTIDOTO / POÇÃO KAPPA
	else if (_volatile == 10) // antidoto ou poção kappa
	{ // 0042F2B4
		st_Affect* affects = Mob[clientId].Mobs.Affects;

		int affectIndex = -1;
		for (int i = 0; i < 32; i++)
		{
			if (affects[i].Index == 4 && (affects[i].Value == 4 || affects[i].Value == 5))
				continue;

			if (affects[i].Index == 4)
			{
				affectIndex = i;
				break;
			}
		}

		if (affectIndex == -1)
			affectIndex = GetEmptyAffect(clientId, 0);

		if (affectIndex == -1)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		int value = 6;
		int time = 15;
		if (srcItem->Index == 1764)
			value = 7;
		else if (srcItem->Index == 600)
		{
			value = 8;
			time = 15 * 60 / 8;
		}

		if (affects[affectIndex].Index != 0 && affects[affectIndex].Value != value)
		{
			SendClientMessage(clientId, "Você já possui uma poção de outro tipo ativa");
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		affects[affectIndex].Index = 4;
		affects[affectIndex].Value = value;
		affects[affectIndex].Time = time;

		SendAffect(clientId);
		SendScore(clientId);

		p364 packet;
		GetCreateMob(clientId, (BYTE*)& packet);

		GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region PERGAMINHO DO RETORNO
	else if (_volatile == 11) // pergaminho do retorno
	{ //0042F32D
		MapAttribute map = GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y);

		int posX = Mob[clientId].Target.X;
		int posY = Mob[clientId].Target.Y;

		bool isHallKefra = posX >= 3199 && posX <= 3324 && posY >= 1663 && posY <= 1714;
		if (map.Village && !isHallKefra)
		{
			if (nTargetX == 0 && nTargetY == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_No_Return_Point]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			map = GetAttribute(nTargetX, nTargetY);
			if (map.CantSummon && Mob[clientId].Mobs.Player.bStatus.Level < 1000)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Teleportar(clientId, nTargetX, nTargetY);
		}
		else
		{
			nTargetX = Mob[clientId].Target.X;
			nTargetY = Mob[clientId].Target.Y;

			DoRecall(clientId);
		}

		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region GEMA ESTELAR
	else if (_volatile == 12)
	{ // 0042F533
		MapAttribute map = GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y);

		if (map.CantSummon && Mob[clientId].Mobs.Player.bStatus.Level < 1000)
		{
			for (INT32 i = 0; i < 3; i++)
			{
				if (Mob[clientId].Target.X >= g_pPesaArea[i][0] && Mob[clientId].Target.X <= g_pPesaArea[i][2] && Mob[clientId].Target.Y >= g_pPesaArea[i][1] && Mob[clientId].Target.Y <= g_pPesaArea[i][3])
				{
					INT32 face = Mob[clientId].Mobs.Player.Equip[0].Index / 10; // local960
					if (face == 0)
						SendEmotion(clientId, 23, 0);
					else if (face >= 1 && face <= 3)
						SendEmotion(clientId, 15, 0);

					Mob[clientId].Mobs.Nightmare[i].X = Mob[clientId].Target.X;
					Mob[clientId].Mobs.Nightmare[i].Y = Mob[clientId].Target.Y;

					Log(clientId, LOG_INGAME, "Armazenado Gema Estelar em %dx %dy", Mob[clientId].Target.X, Mob[clientId].Target.Y);

					AmountMinus(srcItem);

					SendClientMessage(clientId, g_pLanguageString[_NN_Set_Warp]);
					return true;
				}
			}

			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		INT32 arena = GetArena(Mob[clientId].Target.X, Mob[clientId].Target.Y); // local958
		INT32 village = GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y); // local959
		if (arena < 5 || village < 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Use_That_Here]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		INT32 face = Mob[clientId].Mobs.Player.Equip[0].Index / 10; // local960
		if (face == 0)
			SendEmotion(clientId, 23, 0);
		else if (face >= 1 && face <= 3)
			SendEmotion(clientId, 15, 0);

		Mob[clientId].Mobs.Player.Last.X = Mob[clientId].Target.X;
		Mob[clientId].Mobs.Player.Last.Y = Mob[clientId].Target.Y;

		Log(clientId, LOG_INGAME, "Armazenado Gema Estelar em %dx %dy", Mob[clientId].Target.X, Mob[clientId].Target.Y);

		AmountMinus(srcItem);

		SendClientMessage(clientId, g_pLanguageString[_NN_Set_Warp]);
	}
#pragma endregion
#pragma region PERGAMINHO DO TELEPORTE
	else if (_volatile == 13) // pergaminho do teleporte
	{ //0042F7A1
		Teleportar(clientId, Mob[clientId].Mobs.Player.Last.X, Mob[clientId].Mobs.Player.Last.Y);

		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region UNKNOWN
	else if (_volatile == 14) // evocação neses (N)
	{ //0042F824
	}
#pragma endregion 
#pragma region RAÇÕES
	else if (_volatile == 15) // rações
	{ //0042F919
		if (p->DstType != 0 || p->DstSlot != 14)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		st_Item* mont = &Mob[clientId].Mobs.Player.Equip[14];
		if (mont->Index < 2330 || mont->Index > 2390)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		INT32 baseId = 0;
		if (mont->Index >= 2330 && mont->Index < 2360)
			baseId = 2330;

		if (mont->Index >= 2360 && mont->Index < 2390)
			baseId = 2360;

		INT32 mountId = (mont->Index - baseId) % 30; // local1028
		INT32 feedId = (srcItem->Index - 2420);	// LOCAL1029

		if (mountId >= 6 && mountId <= 15 || mountId == 27)
			mountId = 6;

		if (mountId == 19)
			mountId = 7;

		if (mountId == 20)
			mountId = 8;

		if (mountId == 21 || mountId == 22 || mountId == 23 || mountId == 28)
			mountId = 9;

		if (mountId == 24 || mountId == 25 || mountId == 26)
			mountId = 10;

		if (mountId == 29)
			mountId = 19;

		if (mountId != feedId)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if (*(short*)& mont->Effect[0].Index <= 0)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		st_Item* item = &Mob[clientId].Mobs.Player.Equip[14];

		INT32 index = item->Index - baseId + 10;
		INT32 hp = *(short*)& item->Effect[0].Index + 5000;
		INT32 maxHp = NPCBase[index].bStatus.maxHP;

		if (hp > maxHp)
			hp = maxHp;

		*(short*)& item->Effect[0].Index = hp;

		INT32 LOCAL_1032 = item->Effect[2].Index + 2; // local1032
		if (LOCAL_1032 > 100)
			LOCAL_1032 = 100;

		item->Effect[2].Index = LOCAL_1032;

		if (mont->Index >= 2330 && mont->Index < 2360)
			MountProcess(clientId, 0);

		if (mont->Index >= 2360 && mont->Index < 2390)
			ProcessAdultMount(clientId, 0);

		AmountMinus(srcItem);

		SendSignalParm(clientId, 0x7530, 0x3A3, 0x10E);
		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, mont);
		return true;
	}
#pragma endregion
#pragma region PERGAMINHO ÁGUA + PERGAMINHO GUILDA
	else if (_volatile == 17) // pergaminho da guilda
	{ // 00430286
	}
	else if ((_volatile >= 21 && _volatile <= 30) || (_volatile >= 131 && _volatile <= 140) || (_volatile >= 161 && _volatile <= 170))
	{// 00430306

		st_Mob* player = &Mob[clientId].Mobs.Player;

		char tmp[1024];

		INT32 minus = 3182;
		INT32 type = 2;
		if (_volatile >= 21 && _volatile <= 30)
		{
			minus = 777;
			type = 1;
		}
		else if (_volatile >= 131 && _volatile <= 140)
		{
			minus = 3173;
			type = 0;
		}

		int Index = srcItem->Index - minus;

		if (sServer.pWater[type][Index].Time != -1)
		{
			sprintf_s(tmp, "Sala ocupada por %s e seu grupo. Tempo restante: %d seg.", Mob[sServer.pWater[type][Index].Leader].Mobs.Player.Name, sServer.pWater[type][Index].Time);

			SendClientMessage(clientId, tmp);
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (Mob[clientId].Leader != 0 && Mob[clientId].Leader != -1)
		{
			SendClientMessage(clientId, "Apenas o líder do grupo pode utilizar!");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return TRUE;
		}

		if (User.Water.Total >= sServer.MaxWaterEntrance)
		{
			SendClientMessage(clientId, "Você atingiu o limite de entradas diária!");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return TRUE;
		}

		INT32 t = 0;
		for (; t < 9; t++)
		{
			if (Mob[clientId].Target.X >= waterMaxMin[type][t][0] && Mob[clientId].Target.X <= waterMaxMin[type][t][2] && Mob[clientId].Target.Y >= waterMaxMin[type][t][1] && Mob[clientId].Target.Y <= waterMaxMin[type][t][3])
				break;
		}

		INT32 initial = PERGA_A;
		if (_volatile >= 21 && _volatile <= 30)
			initial = PERGA_M;
		else if (_volatile >= 131 && _volatile <= 140)
			initial = PERGA_N;

		if ((Mob[clientId].Target.X > 1961 && Mob[clientId].Target.X < 1970 && Mob[clientId].Target.Y > 1770 && Mob[clientId].Target.Y < 1778) || (t != 9 && sServer.pWater[type][t].Mode == 2))
		{
			if (t == Index)
			{
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (t != 9 && sServer.pWater[type][t].Mode == 2)
			{
				for (INT32 u = 1000; u < 30000; u++)
				{
					if (Mob[u].Target.X >= waterMaxMin[type][t][0] && Mob[u].Target.X <= waterMaxMin[type][t][2] && Mob[u].Target.Y >= waterMaxMin[type][t][1] && Mob[u].Target.Y <= waterMaxMin[type][t][3])
					{
						if (Mob[t].Summoner == 0)
							MobKilled(u, u, 0, 0);
					}
				}

				if (t == 8)
				{
					mGener.pList[initial + 8].MobCount = 0;
					mGener.pList[initial + 9].MobCount = 0;
					mGener.pList[initial + 10].MobCount = 0;
					mGener.pList[initial + 11].MobCount = 0;
				}
				else
					mGener.pList[initial + t].MobCount = 0;

				sServer.pWater[type][t].Time = -1;
				sServer.pWater[type][t].Mode = 0;
			}

			sServer.pWater[type][Index].Time = 120;

			int Generated = initial + Index;

			if (Index == 8)
			{
				int chance = Rand() % 100;
				if (chance >= 75)
					Generated = initial + 8;
				else if (chance >= 50)
					Generated = initial + 9;
				else if (chance >= 25)
					Generated = initial + 10;
				else
					Generated = initial + 11;
			}

			for (int i = 1000; i < 30000; i++)
			{
				if (Mob[i].GenerateID == Generated)
					MobKilled(i, i, 0, 0);
			}

			GenerateMob(Generated, 0, 0);

			Teleportar(clientId, waterPlace[type][Index][0], waterPlace[type][Index][1]);
			SendSignalParm(clientId, 0x7530, 0x3A1, 120);
			SendSignalParm(clientId, clientId, 0x3B0, Generated);

			Log(clientId, LOG_INGAME, "Água %d - Grupo teleportado para a área. Entradas restantes: %d", srcItem->Index, sServer.MaxWaterEntrance - User.Water.Total);

			for (INT32 i = 0; i < 12; i++)
			{
				INT32 mobId = Mob[clientId].PartyList[i];
				if (mobId <= 0 || mobId >= MAX_PLAYER)
					continue;

				if (Users[mobId].Status != USER_PLAY)
					continue;

				if (Users[mobId].User.Water.Total >= sServer.MaxWaterEntrance)
				{
					SendClientMessage(mobId, "Você atingiu o limite de entradas diária!");

					Log(mobId, LOG_INGAME, "Água %d - Não teleportado por falta de entradas água", Users[mobId].User.Water.Total);
					continue;
				}

				Teleportar(mobId, waterPlace[type][Index][0], waterPlace[type][Index][1]);

				Log(clientId, LOG_INGAME, "Água %d - Grupo: %d - Nome: %s", srcItem->Index, i, Mob[mobId].Mobs.Player.Name);
				Log(mobId, LOG_INGAME, "Água %d - Grupo de %s. Entradas restantes: %d", srcItem->Index, Mob[clientId].Mobs.Player.Name, sServer.MaxWaterEntrance - Users[mobId].User.Water.Total);

				LogPlayer(mobId, "%s usou a entrada da água. Entradas restantes: %d", Mob[clientId].Mobs.Player.Name, sServer.MaxWaterEntrance - Users[mobId].User.Water.Total);

				SendSignalParm(mobId, mobId, 0x3B0, Generated);
				SendSignalParm(mobId, 0x7530, 0x3A1, 120);

				Users[mobId].User.Water.Total++;
			}

			sServer.pWater[type][Index].Leader = clientId;
			sServer.pWater[type][Index].Mode = 1;
			sServer.pWater[type][Index].Time = 120;

			User.Water.Total++;

			AmountMinus(srcItem);

		}
		else
		{
			SendClientMessage(clientId, "Use na Zona Elemental da Água!");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}
	}
#pragma endregion
#pragma region PESADELOS
	else if (_volatile == 173) // Pesadelo N/Grupo
	{
		st_Mob* plaer = &Mob[clientId].Mobs.Player;
		int village = GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y);

		if (village != 2)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Erion]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (Mob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (sServer.Nightmare[0].Status != 1 || sServer.Nightmare[0].TimeLeft <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantEnter]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		std::vector<unsigned int> players;
		for (int i = 0; i < 13; i++)
		{
			int memberId = 0;
			if (i != 12)
				memberId = Mob[clientId].PartyList[i];
			else
				memberId = clientId;

			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (Mob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				if (Mob[memberId].Mobs.PesaEnter <= 0)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_NT_Zero]);

					continue;
				}
				else if (Mob[memberId].Mobs.Player.Status.Level >= sServer.MaximumPesaLevel)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_Level_NotAllowed], 1, sServer.MaximumPesaLevel);

					continue;
				}
			}

			players.push_back(memberId);
		}

		size_t remaining = 0;
		for (int i = 0; i < 40; i++)
		{
			if (sServer.Nightmare[0].Members[i] == 0)
				remaining++;
		}

		if (players.size() > remaining)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_AllGroup]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (players.size() == 0)
		{
			SendClientMessage(clientId, "Ninguém no grupo está habilitado a entrar no Pesadelo");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}
		
		for (size_t i = 0; i < players.size(); i++)
		{
			int memberId = players[i];
			int x = 0;
			for (; x < 40; x++)
			{
				if (sServer.Nightmare[0].Members[x] == 0)
					break;
			}

			if (Mob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				Mob[memberId].Mobs.PesaEnter--;
				Log(memberId, LOG_INGAME, "Consumiu NT. Restante: %d", (int)Mob[memberId].Mobs.PesaEnter);
			}

			sServer.Nightmare[0].Members[x] = memberId;
			Teleportar(memberId, 1210, 180);

			Log(clientId, LOG_INGAME, "Pesadelo N - Slot: %d - Name: %s", i, Mob[memberId].Mobs.Player.Name);
			Log(memberId, LOG_INGAME, "Pesadelo N - Grupo de %s", Mob[clientId].Mobs.Player.Name);

			int _rand = Rand() % 3;
			if (_rand >= 3)
				_rand = 2;

			int posX = CoordsPesaN[_rand][0];
			int posY = CoordsPesaN[_rand][1];
			if (Mob[memberId].Mobs.Nightmare[0].X != 0 && Mob[memberId].Mobs.Nightmare[0].Y != 0)
			{
				posX = Mob[memberId].Mobs.Nightmare[0].X;
				posY = Mob[memberId].Mobs.Nightmare[0].Y;
			}

			Teleportar(memberId, posX, posY);

			SendSignalParm(memberId, SERVER_SIDE, 0x3A7, 2);
			SendSignalParm(memberId, memberId, 0x3A1, sServer.Nightmare[0].TimeLeft);
		}

		AmountMinus(srcItem);
	}
	else if (_volatile == 174) // Pesadelo M/Grupo
	{
		st_Mob* player = &Mob[clientId].Mobs.Player;
		int village = GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y);

		if (village != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Armia]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (Mob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (sServer.Nightmare[1].Status != 1 || sServer.Nightmare[1].TimeLeft <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantEnter]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		std::vector<unsigned int> players;
		for (int i = 0; i < 13; i++)
		{
			int memberId = 0;
			if (i != 12)
				memberId = Mob[clientId].PartyList[i];
			else
				memberId = clientId;

			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (Mob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				if (Mob[memberId].Mobs.PesaEnter <= 0)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_NT_Zero]);

					continue;
				}
				else if (Mob[memberId].Mobs.Player.Status.Level >= sServer.MaximumPesaLevel)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_Level_NotAllowed], 1, sServer.MaximumPesaLevel);

					continue;
				}
			}

			players.push_back(memberId);
		}

		size_t remaining = 0;
		for (int i = 0; i < 40; i++)
		{
			if (sServer.Nightmare[1].Members[i] == 0)
				remaining++;
		}

		if (players.size() > remaining)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_AllGroup]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (players.size() == 0)
		{
			SendClientMessage(clientId, "Ninguém no grupo está habilitado a entrar no Pesadelo");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		for (size_t i = 0; i < players.size(); i++)
		{
			auto memberId = players[i];
			int x = 0;
			for (; x < 40; x++)
			{
				if (sServer.Nightmare[1].Members[x] == 0)
					break;
			}

			if (Mob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				Mob[memberId].Mobs.PesaEnter--;
				Log(memberId, LOG_INGAME, "Consumiu NT. Restante: %d", (int)Mob[memberId].Mobs.PesaEnter);
			}

			sServer.Nightmare[1].Members[x] = memberId;

			Teleportar(memberId, 1210, 180);

			Log(clientId, LOG_INGAME, "Pesadelo M - Slot: %d - Name: %s", i, Mob[memberId].Mobs.Player.Name);
			Log(memberId, LOG_INGAME, "Pesadelo M - Grupo de %s", Mob[clientId].Mobs.Player.Name);

			int _rand = Rand() % 6;
			if (_rand >= 6)
				_rand = 5;

			int posX = CoordsPesaM[_rand][0];
			int posY = CoordsPesaM[_rand][1];

			if (Mob[memberId].Mobs.Nightmare[1].X != 0 && Mob[memberId].Mobs.Nightmare[1].Y != 0)
			{
				posX = Mob[memberId].Mobs.Nightmare[1].X;
				posY = Mob[memberId].Mobs.Nightmare[1].Y;
			}

			Teleportar(memberId, posX, posY);

			SendSignalParm(memberId, SERVER_SIDE, 0x3A7, 2);
			SendSignalParm(memberId, memberId, 0x3A1, sServer.Nightmare[1].TimeLeft);
		}

		AmountMinus(srcItem);
	}
	else if (_volatile == 175) // Pesadelo A/Grupo
	{
		st_Mob* player = &Mob[clientId].Mobs.Player;
		int village = GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y);
		if (village != 1)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_Arzam]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (Mob[clientId].Leader != 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Party_Leader_Only]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (sServer.Nightmare[2].Status != 1 || sServer.Nightmare[2].TimeLeft <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantEnter]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		std::stringstream str;
		std::vector<unsigned int> players;
		for (int i = 0; i < 13; i++)
		{
			int memberId = 0;
			if (i != 12)
				memberId = Mob[clientId].PartyList[i];
			else
				memberId = clientId;

			if (memberId <= 0 || memberId >= MAX_PLAYER)
				continue;

			if (Mob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				if (Mob[memberId].Mobs.PesaEnter <= 0)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_NT_Zero]);

					str << "O jogador " << Mob[memberId].Mobs.Player.Name << "(" << Users[memberId].User.Username << ") não entrará por falta de NT" << std::endl;
					continue;
				}
				else if (Mob[memberId].Mobs.Player.Status.Level >= sServer.MaximumPesaLevel)
				{
					SendClientMessage(memberId, g_pLanguageString[_NN_Level_NotAllowed], 1, sServer.MaximumPesaLevel);

					str << "O jogador " << Mob[memberId].Mobs.Player.Name << "(" << Users[memberId].User.Username << ") não entrará por conta do limite de nível" << std::endl;
					continue;
				}
			}

			players.push_back(memberId);
			str << "O jogador " << Mob[memberId].Mobs.Player.Name << "(" << Users[memberId].User.Username << ") foi adicionado a lista" << std::endl;
		}

		size_t remaining = 0;
		for (int i = 0; i < 40; i++)
		{
			if (sServer.Nightmare[2].Members[i] == 0)
				remaining++;
		}

		if (players.size() > remaining)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_AllGroup]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (players.size() == 0)
		{
			SendClientMessage(clientId, "Ninguém no grupo está habilitado a entrar no Pesadelo");

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		str << "[GRUPO]" << std::endl;

		for (size_t i = 0; i < players.size(); i++)
		{
			int memberId = players[i];
			int x = 0;
			for (; x < 40; x++)
			{
				if (sServer.Nightmare[2].Members[x] == 0)
					break;
			}

			if (Mob[memberId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL)
			{
				Mob[memberId].Mobs.PesaEnter--;
				Log(memberId, LOG_INGAME, "Consumiu NT. Restante: %d", (int)Mob[memberId].Mobs.PesaEnter);
			}

			sServer.Nightmare[2].Members[x] = memberId;
			sServer.Nightmare[2].MembersName[x] = Mob[memberId].Mobs.Player.Name;

			INT32 posX = 1210,
				posY = 180;
			if (Mob[memberId].Mobs.Nightmare[2].X != 0 && Mob[memberId].Mobs.Nightmare[2].Y != 0)
			{
				posX = Mob[memberId].Mobs.Nightmare[2].X;
				posY = Mob[memberId].Mobs.Nightmare[2].Y;
			}

			Teleportar(memberId, posX, posY);

			str << "[Pesadelo A] - " << Users[memberId].User.Username << " - " << Mob[memberId].Mobs.Player.Name << std::endl;

			SendSignalParm(memberId, SERVER_SIDE, 0x3A7, 2);
			SendSignalParm(memberId, memberId, 0x3A1, sServer.Nightmare[2].TimeLeft);
		}

		AmountMinus(srcItem);

		for (auto player : players)
			Log(player, LOG_INGAME, str.str().c_str());
	}
#pragma endregion
#pragma region PEDRA IDEAL
	else if (_volatile == 211)
	{
		INT32 classMaster = Mob[clientId].Mobs.Player.Equip[0].EFV2;
		if (classMaster >= TOD_Class::Celestial && Mob[clientId].Mobs.GetTotalResets() == 3 && Mob[clientId].Mobs.Player.bStatus.Level >= 199 && Mob[clientId].Mobs.Info.LvBlocked)
		{
			if (Mob[clientId].Mobs.Player.bStatus.Level == 199 && Mob[clientId].Mobs.Sub.SubStatus.Level == 199)
			{
				int realSealSlot = GetFirstSlot(clientId, 4683);
				if (realSealSlot == -1)
				{
					SendClientMessage(clientId, "Necessário Selo Real para destravar o nível");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				if (Mob[clientId].Mobs.Fame < 1000)
				{
					SendClientMessage(clientId, "Necessário 1000 pontos de fama para destravar o nível");

					return true;
				}

				Mob[clientId].Mobs.Player.Inventory[realSealSlot] = st_Item{};
				Mob[clientId].Mobs.Fame -= 1000;

				SendItem(clientId, SlotType::Inv, realSealSlot, &Mob[clientId].Mobs.Player.Inventory[realSealSlot]);

				Mob[clientId].Mobs.Info.LvBlocked = 0;
				Mob[clientId].Mobs.Info.Unlock200 = 1;

                auto& sub = Mob[clientId].Mobs.Sub;
                sub.Info.LvBlocked = 0;
                sub.Info.Unlock200 = 1;

				Log(clientId, LOG_INGAME, "Nível desbloqueado com sucesso. Evolução atual: %d. Consumido Selo Real do slot %d", Mob[clientId].Mobs.Player.GetEvolution(), realSealSlot);
					
				SendClientMessage(clientId, "Nível desbloqueado");

				AmountMinus(srcItem);
				return true;
			}

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		if (classMaster == ARCH)
		{ // Transformar-se em celestial
			INT32 level = Mob[clientId].Mobs.Player.Status.Level;
			if (level < 354)
			{
				//TODO : HardCore
				SendClientMessage(clientId, "Função ainda não implementada!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			UINT16
				points = 0,
				cythera = 0;

			stQuestInfo quest = Mob[clientId].Mobs.Info;
			if (quest.Elime)
				points += 100;
			if (quest.Sylphed)
				points += 100;
			if (quest.Noas)
				points += 100;
			if (quest.Thelion)
				points += 100;

			if (level >= 355 && level <= 369)
				cythera = 3500;
			else if (level >= 370 && level <= 379)
				cythera = 3500;
			else if (level >= 380 && level <= 397)
				cythera = 3501;
			else if (level == 398)
				cythera = 3501;
			else if (level == 399)
				cythera = 3502;
			else
			{
				SendClientMessage(clientId, "Level inadequado!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int capeId = 0;
			switch (Mob[clientId].Mobs.Player.CapeInfo)
			{
			case CAPE_BLUE:
				capeId = 3197;
				break;
			case CAPE_RED:
				capeId = 3198;
				break;
			case CAPE_WHITE:
				capeId = 3199;
				break;
			}

			if (capeId == 0)
			{
				SendClientMessage(clientId, "Necessário possuir um Reino para criação do celestial");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 levelMortal = Users[clientId].CharList.Status[Mob[clientId].Mobs.MortalSlot].Level;
			if (levelMortal != 399)
			{
				SendClientMessage(clientId, "Necessário que o Mortal seja nível 400");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[1].Index != 0)
			{
				SendClientMessage(clientId, "Desequipe sua cythera para continuar");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int face = Mob[clientId].Mobs.Player.Equip[0].EF2;
			int points_tk[4] = { 8, 4, 7, 6 };
			int points_ht[4] = { 8, 9, 13, 6 };
			int points_bm[4] = { 6, 6, 9, 5 };
			int points_fm[4] = { 5, 8, 5, 5 };

			if (face > 31)
			{
				Mob[clientId].Mobs.Player.bStatus.STR = points_ht[0];
				Mob[clientId].Mobs.Player.bStatus.INT = points_ht[1];
				Mob[clientId].Mobs.Player.bStatus.DEX = points_ht[2];
				Mob[clientId].Mobs.Player.bStatus.CON = points_ht[3];
			}
			else if (face > 21)
			{
				Mob[clientId].Mobs.Player.bStatus.STR = points_bm[0];
				Mob[clientId].Mobs.Player.bStatus.INT = points_fm[1];
				Mob[clientId].Mobs.Player.bStatus.DEX = points_fm[2];
				Mob[clientId].Mobs.Player.bStatus.CON = points_fm[3];
			}
			else if (face > 11)
			{
				Mob[clientId].Mobs.Player.bStatus.STR = points_fm[0];
				Mob[clientId].Mobs.Player.bStatus.INT = points_fm[1];
				Mob[clientId].Mobs.Player.bStatus.DEX = points_fm[2];
				Mob[clientId].Mobs.Player.bStatus.CON = points_fm[3];
			}
			else if (face > 1)
			{
				Mob[clientId].Mobs.Player.bStatus.STR = points_tk[0];
				Mob[clientId].Mobs.Player.bStatus.INT = points_tk[1];
				Mob[clientId].Mobs.Player.bStatus.DEX = points_tk[2];
				Mob[clientId].Mobs.Player.bStatus.CON = points_tk[3];
			}

			if (level >= 355 && level <= 369)
				Mob[clientId].Mobs.Info.Level355 = 1;
			else if (level >= 370 && level <= 379)
				Mob[clientId].Mobs.Info.Level370 = 1;
			else if (level >= 380 && level <= 397)
				Mob[clientId].Mobs.Info.Level380 = 1;
			else if (level == 398)
				Mob[clientId].Mobs.Info.Level398 = 1;
			else if (level == 399)
				Mob[clientId].Mobs.Info.Level399 = 1;
			else
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Celestial criado. Level: %d - Cythera: %d. Points %d. Cape %d", level, cythera, points, capeId);

			SendServerNotice("Parabéns para %s pela criação do seu Celestial", Mob[clientId].Mobs.Player.Name);

			memset(&Mob[clientId].Mobs.Player.SkillBar1, -1, 4);
			memset(&Mob[clientId].Mobs.SkillBar, -1, 16);

			memset(&Mob[clientId].Mobs.Player.Equip[1], 0, sizeof st_Item);

			Mob[clientId].Mobs.Player.Learn[0] = (1 << 30);

			Mob[clientId].Mobs.Player.Equip[0].EFV2 = CELESTIAL;
			Mob[clientId].Mobs.Player.Equip[1] = st_Item{ cythera };

			Mob[clientId].Mobs.Player.bStatus.Level = 0;
			Mob[clientId].Mobs.Player.Status.Level = 0;
			Mob[clientId].Mobs.Player.Exp = 0;

			for (size_t i = 0; i < 4; i++)
				Mob[clientId].Mobs.Player.bStatus.Mastery[i] = 0;

			memset(&Mob[clientId].Mobs.Player.Equip[15], 0, sizeof st_Item);
			Mob[clientId].Mobs.Player.Equip[15].Index = capeId;

			SendItem(clientId, SlotType::Equip, 15, &Mob[clientId].Mobs.Player.Equip[15]);
			SendItem(clientId, SlotType::Equip, 1, &Mob[clientId].Mobs.Player.Equip[1]);
			SendScore(clientId);

			AmountMinus(srcItem);

			CharLogOut(clientId);
			SendSignalParm(clientId, clientId, 0x3B4, Users[clientId].inGame.CharSlot);

			SendClientMessage(clientId, "Que Deus abençoe seu caminho!");
		}
		else if (classMaster == CELESTIAL)
		{
			if (Mob[clientId].Mobs.Sub.Status == 1)
			{
				SendClientMessage(clientId, "Você já possui subcelestial");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.Status.Level < 119)
			{
				SendClientMessage(clientId, "Só é possível criar Subcelestial a partir do level 120");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[11].Index < 1760 || Mob[clientId].Mobs.Player.Equip[11].Index > 1763)
			{
				SendClientMessage(clientId, "É necessário uma Sephirot para a criação do Sub Celestial");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			p830 packet;
			memset(&packet, 0, sizeof p830);

			packet.Header.PacketId = 0x830;
			packet.Header.Size = sizeof packet;
			packet.Header.ClientId = clientId;

			packet.CharPos = Users[clientId].inGame.CharSlot;
			packet.ClassInfo = (Mob[clientId].Mobs.Player.Equip[11].Index - 1760);
			packet.Learn = Mob[clientId].Mobs.Player.Learn[0];
			packet.Face = Mob[clientId].Mobs.Player.Equip[0].EF2;

			int capeId = 0;
			switch (Mob[clientId].Mobs.Player.CapeInfo)
			{
			case CAPE_BLUE:
				capeId = 3197;
				break;
			case CAPE_RED:
				capeId = 3198;
				break;
			default:
				capeId = 3199;
				break;
			}

			packet.Mantle = capeId;

			strncpy_s(packet.Name, Mob[clientId].Mobs.Player.Name, 12);

			AmountMinus(srcItem);
			memset(&Mob[clientId].Mobs.Player.Equip[11], 0, sizeof st_Item);

			//-------------------------
			// Cria o subcele
			unsigned int learn = 0;
			for (int i = 0; i < 8; i++)
			{
				if (24 + i == 30)
					continue;

				int has = (Mob[clientId].Mobs.Player.Learn[0] & (1 << (24 + i)));
				if (has)
					learn |= (1 << (24 + i));
			}

			learn |= (1 << 30);

			INT32 baseFace = 0;
			if (packet.Face < 10)
				baseFace = 6;
			else if (packet.Face < 20)
				baseFace = 16;
			else if (packet.Face < 30)
				baseFace = 26;
			else if (packet.Face < 40)
				baseFace = 36;

			memset(&Mob[clientId].Mobs.Sub, 0, sizeof stSub);

			Mob[clientId].Mobs.Sub.Equip[0].Index = baseFace + packet.ClassInfo;
			Mob[clientId].Mobs.Sub.Equip[0].EF2 = baseFace + packet.ClassInfo;
			Mob[clientId].Mobs.Sub.Equip[0].EFV2 = SUBCELESTIAL;

			Mob[clientId].Mobs.Sub.Exp = 0;
			Mob[clientId].Mobs.Sub.Learn = learn;
			Mob[clientId].Mobs.Sub.Status = 1;

			SendServerNotice("Parabéns para %s pela criação do seu SubCelestial", Mob[clientId].Mobs.Player.Name);

			Log(clientId, LOG_INGAME, "Subcelestial criado. Classe %d - Level cele: %d - Reino: %d", packet.ClassInfo, Mob[clientId].Mobs.Player.Status.Level, capeId);

			memset(&Mob[clientId].Mobs.Sub.SkillBar[0], -1, sizeof(char) * 20);
			AmountMinus(srcItem);

			CharLogOut(clientId);
			AddMessageDB((BYTE*)& packet, sizeof p830);

			SendSignalParm(clientId, clientId, 0x3B4, Users[clientId].inGame.CharSlot);

			SendClientMessage(clientId, "Que Deus abençoe seu caminho!");
		}
		else
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
	}
#pragma endregion
#pragma region ERVA DE CURA
	else if (_volatile == 243)
	{
		bool any = false;
		for (int i = 0; i < 32; i++)
		{
			INT32 affectId = Mob[clientId].Mobs.Affects[i].Index;
			if (affectId == 1 || affectId == 3 || affectId == 5 || affectId == 7 || affectId == 10 || affectId == 12 || affectId == 20 || affectId == 56)
			{
				memset(&Mob[clientId].Mobs.Affects[i], 0, sizeof st_Affect);

				any = true;
			}
		}

		int heal = Users[clientId].Potion.CountHp + 25;
		if (heal > Mob[clientId].Mobs.Player.Status.maxHP)
			heal = Mob[clientId].Mobs.Player.Status.maxHP;

		Users[clientId].Potion.CountHp = heal;
		Users[clientId].Potion.bQuaff = 1;

		if (any)
		{
			Mob[clientId].GetCurrentScore(clientId);
			SendScore(clientId);
		}

		SendSetHpMp(clientId);
		AmountMinus(srcItem);
	}
#pragma endregion
#pragma region FEIJÕES + REMOVEDOR
	else if (_volatile == 186)
	{
		if (p->DstSlot < 1 || p->DstSlot > 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			return true;
		}

		st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
		if (dstItem == NULL || dstItem->Index == 0)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		bool any = false;
		if (srcItem->Index != 3417)
		{
			int efAdd = 116 + (srcItem->Index - 3407);
			for (int i = 0; i < 3; i++)
			{
				if (dstItem->Effect[i].Index == EF_SANC)
				{
					dstItem->Effect[i].Index = efAdd;

					any = true;
					break;
				}
			}

			if (!any)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_HasColor]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				if (dstItem->Effect[i].Index >= 116 && dstItem->Effect[i].Index <= 125)
				{
					dstItem->Effect[i].Index = EF_SANC;

					any = true;
					break;
				}
			}

			if (!any)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_NoColor]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}
		}

		if (any)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

			AmountMinus(srcItem);

			SendEquip(clientId);
			SendEmotion(clientId, 14, 3);
		}
	}
#pragma endregion
#pragma region CLASSES (REPLATIONS)
		else if (itemId >= 4016 && itemId <= 4020)
		{
			if (p->DstType != (unsigned int)SlotType::Inv)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (dstItem == nullptr || dstItem->Index == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			INT32 itemType = GetEffectValueByIndex(dstItem->Index, EF_UNKNOW1);
			INT32 mobType = GetEffectValueByIndex(dstItem->Index, EF_MOBTYPE);
			if (itemType != (srcItem->Index - 4015) || mobType == 1 || ItemList[dstItem->Index].Pos > 32)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Item_NotMatch]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (GetItemSanc(dstItem) > 6)
			{
				SendClientMessage(clientId, "Somente em itens menor ou igual a +6.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Replation usado em %s %s", ItemList[dstItem->Index].Name, dstItem->toString().c_str());

			if (IsImpossibleToRefine(dstItem))
			{
				for (int i = 0; i < 3; i++)
				{
					dstItem->Effect[i].Index = 0;
					dstItem->Effect[i].Value = 0;
				}
			}

			if (dstItem->Effect[0].Index != 43 && (dstItem->Effect[0].Index < 116 || dstItem->Effect[0].Index > 125))
			{
				int sanc = GetItemSanc(dstItem);
				if (sanc != 0)
				{
					if ((dstItem->Effect[1].Index == 43 || (dstItem->Effect[1].Index >= 116 && dstItem->Effect[1].Index <= 125)) ||
						(dstItem->Effect[2].Index == 43 || (dstItem->Effect[2].Index >= 116 && dstItem->Effect[2].Index <= 125)))
					{
						std::swap(dstItem->Effect[1], dstItem->Effect[2]);
					}
				}
			}

			for (int i = 1; i < 3; i++)
			{
				dstItem->Effect[i].Index = 0;
				dstItem->Effect[i].Value = 0;
			}

			SetItemBonus(dstItem);

			Log(clientId, LOG_INGAME, "Replation gerou o adicional %s", dstItem->toString().c_str());
			AmountMinus(srcItem);

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region BENÇÃO DE ÂMAGO
	else if (itemId == 3250)
	{
		auto dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
		if (dstItem == nullptr)
		{
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (dstItem->Index < 2400 || dstItem->Index > 2418)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		int essenceBlessed = 0;
		switch (dstItem->Index)
		{
		case 2400:
			essenceBlessed = 3230;
			break;
		case 2405:
			essenceBlessed = 3231;
			break;
		case 2406:
			essenceBlessed = 3232;
			break;
		case 2407:
			essenceBlessed = 3233;
			break;
		case 2409:
			essenceBlessed = 3234;
			break;
		case 2410:
			essenceBlessed = 3235;
			break;
		case 2411:
		case 2412:
		case 2413:
			essenceBlessed = 3236 + (dstItem->Index - 2411);
			break;
		case 2414:
		case 2415:
		case 2416:
			essenceBlessed = 3239 + (dstItem->Index - 2414);
			break;
		case 2417:
			essenceBlessed = 3242;
			break;
		case 2418:
			essenceBlessed = 3243;
			break;
		}

		if (essenceBlessed == 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		if (GetItemAmount(dstItem) != 120)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}

		*dstItem = st_Item{};
		dstItem->Index = essenceBlessed;

		AmountMinus(srcItem);

		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
		return true;
	}
#pragma endregion
#pragma region BARRA MYTRIL
	else if (_volatile >= 235 && _volatile <= 238)
	{
		static const INT16 g_pAddMyt[4][9] =
		{// MaxAddSet , MaxAddArma, MinAddSet, MinAddArma, SumSet, SumArma, EFFECT
			{42, 81, 0, 0, 6, 9, EF_DAMAGE, EF_DAMAGE2, 0},
			{14, 36,  0, 0, 2, 4, EF_MAGIC, 0, 0},
			{40,  0, 0,  0, 5, 0, EF_AC, EF_ACADD, EF_ACADD2},
			{90,  0, 0,  0, 10, 0, EF_CRITICAL, EF_CRITICAL2, 0}
		};

		st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

		INT32
			sanc = GetItemSanc(dstItem),
			mobType = GetEffectValueByIndex(dstItem->Index, EF_MOBTYPE),
			type = _volatile - 235;

		if (sanc < 11 || mobType != 1)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		INT32 pos = ItemList[dstItem->Index].Pos,
			totalAdd = 0;

		int addFound = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			if (dstItem->Effect[i].Index != 0 && dstItem->Effect[i].Index == g_pAddMyt[type][6] || dstItem->Effect[i].Index == g_pAddMyt[type][7] || dstItem->Effect[i].Index == g_pAddMyt[type][8])
			{
				totalAdd += dstItem->Effect[i].Value;
				addFound = dstItem->Effect[i].Index;
			}
		}

		if (g_pAddMyt[type][6] == EF_CRITICAL)
		{
			totalAdd += GetEffectValueByIndex(dstItem->Index, EF_CRITICAL);

			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->Effect[i].Index == EF_CRITICAL2)
				{
					totalAdd = dstItem->Effect[i].Value;

					break;
				}
			}
		}
		else if (g_pAddMyt[type][7] == EF_ACADD && addFound != EF_ACADD2)
			totalAdd += GetEffectValueByIndex(dstItem->Index, EF_ACADD);

		INT32 maxAdd = g_pAddMyt[type][0],
			minAdd = g_pAddMyt[type][2],
			sumAdd = g_pAddMyt[type][4];

		if (pos > 32)
		{
			maxAdd = g_pAddMyt[type][1];
			minAdd = g_pAddMyt[type][3];
			sumAdd = g_pAddMyt[type][5];
		}

		if (pos == 16 && type == 2)
			maxAdd = 70;

		if (totalAdd <= minAdd || totalAdd >= maxAdd)
		{
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}

		INT32 _rand = Rand() % 100;
		if (_rand >= 70)
		{
			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->Effect[i].Index == g_pAddMyt[type][6] || dstItem->Effect[i].Index == g_pAddMyt[type][7] || dstItem->Effect[i].Index == g_pAddMyt[type][8])
				{
					dstItem->Effect[i].Value += sumAdd;
					break;
				}
			}

			Log(clientId, LOG_INGAME, "Adicional de %s [%d] [%d %d %d %d %d %d] aumentou.", ItemList[dstItem->Index].Name, dstItem->Index, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);
		}
		else if (_rand <= 20)
		{
			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->Effect[i].Index == g_pAddMyt[type][6] || dstItem->Effect[i].Index == g_pAddMyt[type][7] || dstItem->Effect[i].Index == g_pAddMyt[type][8])
				{
					if (dstItem->Effect[i].Value - sumAdd < 0)
						dstItem->Effect[i].Value = 0;
					else
						dstItem->Effect[i].Value -= sumAdd;

					break;
				}
			}

			Log(clientId, LOG_INGAME, "Adicional de %s [%d] [%d %d %d %d %d %d] reduziu.", ItemList[dstItem->Index].Name, dstItem->Index, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);
		}
		else
			Log(clientId, LOG_INGAME, "Adicional de %s [%d] [%d %d %d %d %d %d] não aconteceu nada.", ItemList[dstItem->Index].Name, dstItem->Index, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

		SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
		AmountMinus(srcItem);
		return true;
	}
#pragma endregion
	else
	{
#pragma region _BOLSA DO ANDARILHO
		if (itemId == 3467)
		{ // Bolsa do Andarilho
			INT32 slot = -1;
			if (Mob[clientId].Mobs.Player.Inventory[60].Index == 0)
				slot = 60;
			else if (Mob[clientId].Mobs.Player.Inventory[61].Index == 0)
				slot = 61;
			else if (TimeRemaining(Mob[clientId].Mobs.Player.Inventory[60].EFV1, Mob[clientId].Mobs.Player.Inventory[60].EFV2, Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) <= 0)
				slot = 60;
			else if (TimeRemaining(Mob[clientId].Mobs.Player.Inventory[61].EFV1, Mob[clientId].Mobs.Player.Inventory[61].EFV2, Mob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900) <= 0)
				slot = 61;

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				SendClientMessage(clientId, g_pLanguageString[_NN_MaxBag_Actived]);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon; //0 Janeiro, 1 Fev
			int day = now.tm_mday;
			int year = now.tm_year;

			int mes = 0, dia = 0, ano = 0;
			int days = 30;

			mes = month + 1;
			dia = day + days;
			ano = year + 1900;

			if (dia > dias_mes[mes])
			{
				dia -= dias_mes[mes];
				mes += 1;
			}

			if (mes > 12)
			{
				mes -= 12;
				ano += 1;
			}

			Mob[clientId].Mobs.Player.Inventory[slot].Index = 3467;
			Mob[clientId].Mobs.Player.Inventory[slot].EF1 = 106;
			Mob[clientId].Mobs.Player.Inventory[slot].EFV1 = dia;
			Mob[clientId].Mobs.Player.Inventory[slot].EF3 = 109;
			Mob[clientId].Mobs.Player.Inventory[slot].EFV3 = (ano - 1900);
			Mob[clientId].Mobs.Player.Inventory[slot].EF2 = 110;
			Mob[clientId].Mobs.Player.Inventory[slot].EFV2 = mes;

			Log(clientId, LOG_INGAME, "Bolsa do Andarilho usado. Até dia %02d/%02d/%04d", dia, mes, ano);
			LogPlayer(clientId, "Bolsa do Andarilho utilizado durando até o dia %02d/%02d/%04d", dia, mes, ano);

			AmountMinus(srcItem);
			SendItem(clientId, SlotType::Inv, slot, &Mob[clientId].Mobs.Player.Inventory[slot]);
		}
#pragma endregion
#pragma region CAIXA DE JOIA
		else if (itemId == 4528)
		{
			int slotId = -1;
			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
			*item = st_Item{};
			item->Index = 2441 + Rand() % 4;
			SendItem(clientId, (SlotType)p->SrcType, slotId, item);

			SendClientMessage(clientId, "!Chegou um item [%s]", ItemList[item->Index].Name);
			Log(clientId, LOG_INGAME, "Abriu Caixa de Joia e recebeu: %s", ItemList[item->Index].Name);

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region PEDRA DA FURIA
		else if (itemId == 3020)
		{
			INT32 evId = Mob[clientId].Mobs.Player.Equip[0].EFV2;
			INT32 level = Mob[clientId].Mobs.Player.bStatus.Level;

			if (evId == SUBCELESTIAL || evId == CELESTIAL)
			{
				if (evId == SUBCELESTIAL && (Mob[clientId].Mobs.Info.Arcana || Mob[clientId].Mobs.Sub.Info.Arcana))
				{
					if (level >= 199)
					{
						if (Mob[clientId].Mobs.Player.Exp < 4000000000)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_CantTele_Nip]);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (Mob[clientId].Mobs.Fame < 1000)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 1000);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						int realSealSlot = GetFirstSlot(clientId, 4683);
						if (realSealSlot == -1)
						{
							SendClientMessage(clientId, "Você precisa de um Selo Real");

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (GetInventoryAmount(clientId, 4011) < 5)
						{
							SendClientMessage(clientId, "Você precisa de 5 bilhões de gold (em barras)");

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (Mob[clientId].Mobs.GetTotalResets() >= 3)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						if (!Mob[clientId].Mobs.Info.Reset_1)
						{
							Log(clientId, LOG_INGAME, "O personagem %s completou o primeiro reset do celestial.", Mob[clientId].Mobs.Player.Name);
							Mob[clientId].Mobs.Info.Reset_1 = 1;
							Mob[clientId].Mobs.Sub.Info.Reset_1 = 1;
						}
						else if (!Mob[clientId].Mobs.Info.Reset_2)
						{
							Log(clientId, LOG_INGAME, "O personagem %s completou o segundo reset do celestial.", Mob[clientId].Mobs.Player.Name);
							Mob[clientId].Mobs.Info.Reset_2 = 1;
							Mob[clientId].Mobs.Sub.Info.Reset_2 = 1;
						}
						else if (!Mob[clientId].Mobs.Info.Reset_3)
						{
							Log(clientId, LOG_INGAME, "O personagem %s completou o terceiro reset do celestial.", Mob[clientId].Mobs.Player.Name);
							Mob[clientId].Mobs.Info.Reset_3 = 1;
							Mob[clientId].Mobs.Sub.Info.Reset_3 = 1;
						}
						else
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

							SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
							return true;
						}

						RemoveAmount(clientId, 4011, 5);
						Mob[clientId].Mobs.Fame -= 1000;
						AmountMinus(srcItem);
						AmountMinus(&Mob[clientId].Mobs.Player.Inventory[realSealSlot]);
						SendItem(clientId, SlotType::Inv, realSealSlot, &Mob[clientId].Mobs.Player.Inventory[realSealSlot]);

						Mob[clientId].Mobs.Player.Exp -= 200000000LL;

						while (Mob[clientId].Mobs.Player.bStatus.Level > 0 && Mob[clientId].Mobs.Player.Exp < g_pNextLevel[Mob[clientId].Mobs.Player.Equip[0].EFV2][level - 1])
						{
							Mob[clientId].Mobs.Player.bStatus.Level--;
							level--;
						}

						Mob[clientId].GetCurrentScore(clientId);
						SendScore(clientId);
						SendEtc(clientId);

						SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
						return true;
					}
				}
				if (level == 199 && Mob[clientId].Mobs.Player.Equip[1].Index == 3502)
				{
					if (Mob[clientId].Mobs.Info.Arcana || Mob[clientId].Mobs.Sub.Info.Arcana)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

						SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
						return true;
					}

					if (Mob[clientId].Mobs.Fame < 500)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 500);

						SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
						return true;
					}

					if (Mob[clientId].Mobs.Player.Equip[1].Index == 3502)
					{
						int szStones[4] = { 0, 0, 0, 0 };
						for (int i = 0; i < 4; i++)
						{
							int Slot = GetFirstSlot(clientId, 5334 + i);
							if (Slot == -1)
							{
								SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

								return true;
							}

							szStones[i] = Slot;
						}

						for (int i = 0; i < 4; i++)
						{
							memset(&Mob[clientId].Mobs.Player.Inventory[szStones[i]], 0, sizeof st_Item);

							SendItem(clientId, SlotType::Inv, szStones[i], &Mob[clientId].Mobs.Player.Inventory[szStones[i]]);
						}

						// Retirar a fame
						Mob[clientId].Mobs.Fame -= 500;

						AmountMinus(srcItem);

						int _rand = Rand() % 100;
						int _add = Rand() % 50;
						if (_rand + _add > 100)
							_rand += _add - 50;

						if (_rand < 50)
						{
							Mob[clientId].Mobs.Player.Equip[1].Index = 3507;

							SendItem(clientId, SlotType::Equip, 1, &Mob[clientId].Mobs.Player.Equip[1]);
							SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

							Mob[clientId].Mobs.Info.Arcana = 1;
							Mob[clientId].Mobs.Sub.Info.Arcana = 1;

							Log(clientId, LOG_INGAME, "Sucesso ao criar Cythera Arcana");
							SendNotice(".O jogador %s concluiu com sucesso a criaçãoo da Cythera Arcana", Mob[clientId].Mobs.Player.Name);
						}
						else
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);

							Log(clientId, LOG_INGAME, "Falha ao criar Cythera Arcana");
							SendNotice(".O jogador %s falhou a criação da Cythera Arcana", Mob[clientId].Mobs.Player.Name);
						}
					}
					else if (level == 199 && evId == 4)
					{
						SendClientMessage(clientId, "Desativado...");

						SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
						return true;
					}
				}
			}

			if (evId != CELESTIAL || level != 89 || !Mob[clientId].Mobs.Info.LvBlocked || Mob[clientId].Mobs.Info.Unlock89)
			{
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

				return true;
			}

			if (Mob[clientId].Mobs.Fame < 500)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 500);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int _rand = Rand() % 101;
			Mob[clientId].Mobs.Fame -= 500;

			if (_rand <= 95)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

				SendEmotion(clientId, 14, 3);

				INT32 slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_NoSpace_Item], ItemList[3502].Name);

					Log(clientId, LOG_INGAME, "Desbloqueio 90: Sem espaço para receber cythera");
				}
				else
				{
					Log(clientId, LOG_INGAME, "Desbloqueio 90:  recebeu cythera mística");

					// Apaga o item
					memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

					// Atualiza o inventário
					Mob[clientId].Mobs.Player.Inventory[slotId].Index = 3502;

					SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);
				}

				Mob[clientId].Mobs.Info.LvBlocked = 0;
				Mob[clientId].Mobs.Info.Unlock89 = 1;
			}
			else
			{
				SendClientMessage(clientId, "Houve falha no destravamento do nível %d/95", _rand);

				Log(clientId, LOG_INGAME, "Desbloqueio 90: FALHA");
			}

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region PEDIDO DE CAÇA
		else if (itemId >= 3432 && itemId <= 3437)
		{
			INT32 warp = p->warp;
			if (warp > 10)
				return false;

			if (warp - 1 < 0)
				return false;

			warp--;

			int ID = itemId - 3432;
			int tX = HuntingScrolls[ID][warp][0];
			int tY = HuntingScrolls[ID][warp][1];

			AmountMinus(srcItem); // Removes 1 if the item has unities, deletes it if not.
			Teleportar(clientId, tX, tY);
		}
#pragma endregion
#pragma region PERGAMINHO DO PERDÃO
		else if (itemId == 3343)
		{
			if (GetPKPoint(clientId) - 75 > 60)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CantReset_ChaosPoint]);
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Log(clientId, LOG_INGAME, "Perdão utilizado. PK antigo: %d.", GetPKPoint(clientId) - 75);

			SetPKPoint(clientId, GetPKPoint(clientId) + 75);

			SendClientMessage(clientId, g_pLanguageString[_NN_CPPoint], GetPKPoint(clientId) - 75);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region FOGOS DE ARTIFICIO
		else if (itemId == 1728)
		{
			SendEmotion(clientId, 100, Rand() % 8);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region BAÚ DE EXPERIÊNCIA
		else if (itemId == 4140 || itemId == 4548 || itemId == 4549)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 39)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			INT32 time = 900;
			if (itemId == 4548)
				time = 225;
			else if (itemId == 4549)
				time = 450;

			Mob[clientId].Mobs.Affects[slot].Index = 39;

			if (Mob[clientId].Mobs.Affects[slot].Time > 0)
				Mob[clientId].Mobs.Affects[slot].Time += time;
			else
				Mob[clientId].Mobs.Affects[slot].Time = time;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region CHOCOLATE DO AMOR
		else if (itemId == 1739)
		{
			SetAffect(clientId, 41, 500, 125);
			SetAffect(clientId, 43, 500, 125);
			SetAffect(clientId, 44, 500, 125);
			SetAffect(clientId, 45, 500, 125);
			SendAffect(clientId);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region ESCRITURAS
		else if (itemId == 3445 || itemId == 3446)
		{
			INT32 searched = 412 + (itemId - 3445);
			INT32 totalAmount = GetInventoryAmount(clientId, searched);

			if (totalAmount > 10)
				totalAmount = 10;

			if (totalAmount == 0)
			{
				SendClientMessage(clientId, "Não há %s para juntar", ItemList[searched].Name);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = GetFirstSlot(clientId, 0);
			if (slotId == -1)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CarryFull]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = totalAmount;
			for (INT32 i = 0; i < 60; i++)
			{
				if (Mob[clientId].Mobs.Player.Inventory[i].Index == searched)
				{
					while (Mob[clientId].Mobs.Player.Inventory[i].Index == searched)
					{
						AmountMinus(&Mob[clientId].Mobs.Player.Inventory[i]);

						totalAmount--;
						if (totalAmount <= 0)
							break;
					}
				}

				if (totalAmount <= 0)
					break;
			}

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
			item->Index = 3447 + (itemId - 3445);

			item->EF1 = EF_AMOUNT;
			item->EFV1 = total;

			AmountMinus(srcItem);
			SendCarry(clientId);
		}

#pragma endregion
#pragma region FRANGO E POÇÕES DA CORAGEM
		else if (itemId == 3314 || itemId == 646 || itemId == 647)
		{
			int index = 0;
			int time = 0;
			if (itemId == 3314)
			{
				index = 1;
				time = 1800;
			}
			else if (itemId == 646 || itemId == 647)
			{
				index = 2 + (itemId - 646);

				time = 450;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 30)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Mob[clientId].Mobs.Affects[slot].Index = 30;
			Mob[clientId].Mobs.Affects[slot].Time = time;
			Mob[clientId].Mobs.Affects[slot].Value = index;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region BOOKS SEPHIRA
		else if (itemId >= 667 && itemId <= 671)
		{
			// SkillPoint necessário
			const WORD BooksSkill[5] = { 0, 36, 35, 40, 39 };

			int _index = srcItem->Index - 666;

			if (_index <= 0 || _index > 5)
				return true;

			int has = (Mob[clientId].Mobs.Player.Learn[0] & (1 << (24 + _index)));
			if (has)
			{
				SendClientMessage(clientId, "Você já aprendeu esta skill");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.SkillPoint < BooksSkill[_index - 1])
			{
				SendClientMessage(clientId, "Você não tem pontos de Skill suficiente");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			// Checagem da classe do personagem : <= arch , celestiais podem adquirir em qualquer level
			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL)
			{
				if (Mob[clientId].Mobs.Player.bStatus.Level < 255)
				{
					SendClientMessage(clientId, "Level inadequado");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			Log(clientId, LOG_INGAME, "Aprendido skill %s", ItemList[srcItem->Index].Name);
			LogPlayer(clientId, "Aprendeu a skill sephira %s", ItemList[srcItem->Index].Name);

			Mob[clientId].Mobs.Player.SkillPoint -= BooksSkill[_index - 1];
			Mob[clientId].Mobs.Player.Learn[0] |= (1 << (_index + 24));
			Mob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendEtc(clientId);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region POÇÃO REVIGORANTE XD
		else if (itemId >= 3210 && itemId <= 3213)
		{
			int days = 7;
			switch (itemId)
			{
			case 3210:
				days = 1;
				break;
			case 3211:
				days = 3;
				break;
			case 3212:
				days = 7;
				break;
			case 3213:
				days = 30;
				break;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 51)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			Mob[clientId].Mobs.Revigorante.Dia = day;
			Mob[clientId].Mobs.Revigorante.Mes = month;
			Mob[clientId].Mobs.Revigorante.Ano = year;
			Mob[clientId].Mobs.Revigorante.Hora = hour;
			Mob[clientId].Mobs.Revigorante.Minuto = min;
			Mob[clientId].Mobs.Revigorante.Segundo = sec;


			Mob[clientId].Mobs.Affects[slot].Index = 51;
			Mob[clientId].Mobs.Affects[slot].Time = 99;
			Mob[clientId].Mobs.Affects[slot].Master = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Poção Revigorante utilizada até dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year + 1900, hour, min, sec);
		}
#pragma endregion
#pragma region POÇÃO DIVINA
		else if (itemId >= 3379 && itemId <= 3381)
		{
			int days = 7;
			switch (itemId)
			{
			case 3379:
				days = 7;
				break;
			case 3380:
				days = 15;
				break;
			case 3381:
				days = 30;
				break;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 34)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			Users[clientId].User.Divina.Dia = day;
			Users[clientId].User.Divina.Mes = month;
			Users[clientId].User.Divina.Ano = year;
			Users[clientId].User.Divina.Hora = hour;
			Users[clientId].User.Divina.Minuto = min;
			Users[clientId].User.Divina.Segundo = sec;

			Mob[clientId].Mobs.Affects[slot].Index = 34;
			Mob[clientId].Mobs.Affects[slot].Time = 99;
			Mob[clientId].Mobs.Affects[slot].Master = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Poção Divina utilizada até dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year, hour, min, sec);
		}
#pragma endregion
#pragma region POÇÃO SEPHIRA
		else if (itemId >= 3361 && itemId <= 3363)
		{
			int days = 7;
			switch (itemId)
			{
			case 3361:
				days = 7;
				break;
			case 3362:
				days = 15;
				break;
			case 3363:
				days = 30;
				break;
			}

			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 4)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			Users[clientId].User.Sephira.Dia = day;
			Users[clientId].User.Sephira.Mes = month;
			Users[clientId].User.Sephira.Ano = year;
			Users[clientId].User.Sephira.Hora = hour;
			Users[clientId].User.Sephira.Minuto = min;
			Users[clientId].User.Sephira.Segundo = sec;

			Mob[clientId].Mobs.Affects[slot].Index = 4;
			Mob[clientId].Mobs.Affects[slot].Master = 1;
			Mob[clientId].Mobs.Affects[slot].Value = 4; // Poção Sephira = 4
			Mob[clientId].Mobs.Affects[slot].Time = 999;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);

			AmountMinus(srcItem);
			Log(clientId, LOG_INGAME, "Poção Sephira utilizada até dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year + 1900, hour, min, sec);
		}
#pragma endregion
#pragma region POÇÃO SAÚDE
		else if (itemId >= 3364 && itemId <= 3366)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 35)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			int days = 7;
			switch (itemId)
			{
			case 3364:
				days = 7;
				break;
			case 3365:
				days = 15;
				break;
			case 3366:
				days = 30;
				break;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			Mob[clientId].Mobs.Saude.Dia = day;
			Mob[clientId].Mobs.Saude.Mes = month;
			Mob[clientId].Mobs.Saude.Ano = year;
			Mob[clientId].Mobs.Saude.Hora = hour;
			Mob[clientId].Mobs.Saude.Minuto = min;
			Mob[clientId].Mobs.Saude.Segundo = sec;

			Mob[clientId].Mobs.Affects[slot].Index = 35;
			Mob[clientId].Mobs.Affects[slot].Master = 1;
			Mob[clientId].Mobs.Affects[slot].Value = 1; // Poção Saúde = 1 . Poção Vigor = 2
			Mob[clientId].Mobs.Affects[slot].Time = 999;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			SendAffect(clientId);

			AmountMinus(srcItem);
			Log(clientId, LOG_INGAME, "Poção Saúde utilizada até dia %02d/%02d/%02d %02d:%02d:%02d", day, month, year + 1900, hour, min, sec);
		}
#pragma endregion
#pragma region ESCRITURA DE PESADELO
		else if (itemId == 5137)
		{
			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Need_CelestialSub]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			int month = now.tm_mon; //0 Janeiro, 1 Fev
			int day = now.tm_mday;
			int year = now.tm_year;

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(now_time_t, Mob[clientId].Mobs.Escritura.GetTMStruct());
			if (Mob[clientId].Mobs.Escritura.Ano == 0)
				diffTime = ScrollWaitTime;

			if (diffTime < ScrollWaitTime)
			{
				int totalSeconds = (ScrollWaitTime - (int)diffTime);
				int hours = (totalSeconds / 3600) % 24;
				int mins = (totalSeconds % 3600) / 60;
				int seconds = totalSeconds % 60;

				SendClientMessage(clientId, "Você precisa aguardar %02d horas %02d minutos %02d segundos até o próximo uso", hours, mins, seconds);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 value = Mob[clientId].Mobs.PesaEnter;
			if (value >= 36)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Max36_Enter]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			day = now.tm_mday;
			year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			Mob[clientId].Mobs.Escritura.Ano = year;
			Mob[clientId].Mobs.Escritura.Mes = month;
			Mob[clientId].Mobs.Escritura.Dia = day;
			Mob[clientId].Mobs.Escritura.Hora = hour;
			Mob[clientId].Mobs.Escritura.Minuto = min;
			Mob[clientId].Mobs.Escritura.Segundo = sec;

			value += 12;
			if (value > 36)
				value = 36;

			Mob[clientId].Mobs.PesaEnter = value;

			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Escritura de Pesadelo utilizado e tempo atribuído");
		}
#pragma endregion
#pragma region BARRAS GOLD
		else if (itemId == 4010 || itemId == 4011 || (itemId >= 4026 && itemId <= 4029))
		{
			if (!GoldBar(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem))
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion
#pragma region GEMA ANCT
		else if (itemId >= 3386 && itemId <= 3389)
		{
			sItemData itemData = ItemList[Mob[clientId].Mobs.Player.Equip[p->DstSlot].Index];

			if (p->DstType != (unsigned int)SlotType::Equip)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (itemData.Pos == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (itemData.Pos >= 64 && itemData.Pos <= 2048)
			{
				if ((itemData.Grade < 5 || itemData.Grade > 8) && GetItemSanc(&Mob[clientId].Mobs.Player.Equip[p->DstSlot]) < 10)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true; // O item não é ancient
				}
			}

			INT32 actual = 0,
				stone = Mob[clientId].Mobs.Player.Inventory[p->SrcSlot].Index - 3386;

			if (itemData.Pos <= 32)
			{
				INT32 sanctmp = GetItemSanc(&Mob[clientId].Mobs.Player.Equip[p->DstSlot]);
				if (GetItemSanc(&Mob[clientId].Mobs.Player.Equip[p->DstSlot]) < 10)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true; // Set +9 ou menor ._.
				}

				INT32 value = -1;
				for (INT32 t = 0; t < 3; t++)
				{
					if (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[t].Index == 43 || (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[t].Index >= 116 && Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[t].Index <= 125))
					{
						value = Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[t].Value;
						break;
					}
				}

				value = (value - 230) % 4;
				if (value == stone)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true;
				}
			}
			else
			{
				actual = itemData.Grade - 5;

				// Checa se os dois não são os mesmos -> GARNET PARA GARNET
				if (stone == actual)
				{
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

					return true;
				}
			}

			st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (!dstItem)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			// Para armas
			if (itemData.Pos >= 64 && itemData.Pos <= 2048)
			{
				for (int i = 0; i < 3; i++)
				{
					if (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index == 43 || (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index >= 116 && Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index <= 125))
					{
						actual = Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Value;

						break;
					}
				}

				if (actual > 9)
				{
					int sanc = actual;
					// Pega o valor da refinação base 
					if (sanc < 234)
						sanc = 230;
					else if (sanc < 238)
						sanc = 234;
					else if (sanc < 242)
						sanc = 238;
					else if (sanc < 246)
						sanc = 242;
					else if (sanc < 250)
						sanc = 246;
					else
						sanc = 250;

					actual = actual - sanc;

					for (int i = 0; i < 3; i++)
					{
						if (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index == 43 || (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index >= 116 && Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index <= 125))
						{
							Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Value += (stone - actual);

							break;
						}
					}
				}

				if (itemData.Pos != 128 && (itemData.Grade >= 5 && itemData.Grade <= 8))
				{
					stone = Mob[clientId].Mobs.Player.Inventory[p->SrcSlot].Index - 3386;

					actual = itemData.Grade - 5;
					Mob[clientId].Mobs.Player.Equip[p->DstSlot].Index += (stone - actual);
				}
			}
			else if (itemData.Pos <= 32) {
				// Pega a refinação atual do item
				for (int i = 0; i < 3; i++)
				{
					if (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index == 43 || (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index >= 116 && Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index <= 125))
					{
						actual = Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Value;

						break;
					}
				}

				int sanc = actual;
				// Pega o valor da refinação base 
				if (sanc < 234)
					sanc = 230;
				else if (sanc < 238)
					sanc = 234;
				else if (sanc < 242)
					sanc = 238;
				else if (sanc < 246)
					sanc = 242;
				else if (sanc < 250)
					sanc = 246;
				else
					sanc = 250;

				actual = actual - sanc;
				for (int i = 0; i < 3; i++)
				{
					if (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index == 43 || (Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index >= 116 && Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Index <= 125))
					{
						Mob[clientId].Mobs.Player.Equip[p->DstSlot].Effect[i].Value += (stone - actual);

						break;
					}
				}
			}
			else
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, &Mob[clientId].Mobs.Player.Equip[p->DstSlot]);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Usado Gema no item %s [%d] [%d %d %d %d %d %d]", ItemList[dstItem->Index].Name, dstItem->Index,
				dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);
		}
#pragma endregion
#pragma region LANHOUSE (MASMORRA)
		else if (itemId == 4111)
		{
			if (GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y) == 5)
			{
				SendClientMessage(clientId, "Somente utilize em uma cidade!");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL)
			{
				SendClientMessage(clientId, "Disponível apenas para mortais");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = 0;
			for (INT32 i = 1; i < MAX_PLAYER; i++)
			{
				if (Users[i].Status != USER_PLAY)
					continue;

				if ((Mob[i].Target.X >= 3732 && Mob[i].Target.X <= 3816 && Mob[i].Target.Y >= 3476 && Mob[i].Target.Y <= 3562) ||
					(Mob[i].Target.X >= 3600 && Mob[i].Target.X <= 3700 && Mob[i].Target.Y >= 3600 && Mob[i].Target.Y <= 3700) ||
					(Mob[i].Target.X >= 3840 && Mob[i].Target.X <= 3967 && Mob[i].Target.Y >= 3584 && Mob[i].Target.Y <= 3712))
				{
					if (memcmp(Users[i].MacAddress, Users[clientId].MacAddress, 8) == 0)
						total++;
				}
			}

			if (total >= 1)
			{
				SendClientMessage(clientId, "Disponível apenas uma conta por computador na área");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (static_cast<TOD_EventItem*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::Quiz))->Register(*this, srcItem))
			{
				Teleportar(clientId, 3655, 3640);

				auto now = std::chrono::steady_clock::now();
				if (now - Times.LastLanHouseN <= 2s)
				{
					auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastLanHouseN);

					Log(clientId, LOG_HACK, "O usuário entrou na LanHouse (N) muito rapidamente. Tempo: %lld", time.count());
					Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuário entrou na LanHouse (N) muito rapidamente. Tempo: %lld", User.Username, time.count());
				}

				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region LANHOUSE (MASMORRA M)
		else if (itemId == 4112)
		{
			if (GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y) == 5)
			{
				SendClientMessage(clientId, "Somente utilize em uma cidade!");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 != ARCH)
			{
				SendClientMessage(clientId, "Disponível apenas para archs");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = 0;
			for (INT32 i = 1; i < MAX_PLAYER; i++)
			{
				if (Users[i].Status != USER_PLAY)
					continue;

				if ((Mob[i].Target.X >= 3732 && Mob[i].Target.X <= 3816 && Mob[i].Target.Y >= 3476 && Mob[i].Target.Y <= 3562) ||
					(Mob[i].Target.X >= 3600 && Mob[i].Target.X <= 3700 && Mob[i].Target.Y >= 3600 && Mob[i].Target.Y <= 3700) ||
					(Mob[i].Target.X >= 3840 && Mob[i].Target.X <= 3967 && Mob[i].Target.Y >= 3584 && Mob[i].Target.Y <= 3712))
				{
					if (memcmp(Users[i].MacAddress, Users[clientId].MacAddress, 8) == 0)
						total++;
				}
			}

			if (total >= 1)
			{
				SendClientMessage(clientId, "Disponível apenas uma conta por computador na área");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (static_cast<TOD_EventItem*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::Quiz))->Register(*this, srcItem))
			{
				Teleportar(clientId, 3768, 3513);

				auto now = std::chrono::steady_clock::now();
				if (now - Times.LastLanHouseM <= 2s)
				{
					auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastLanHouseM);

					Log(clientId, LOG_HACK, "O usuário entrou na LanHouse (M) muito rapidamente. Tempo: %lld", time.count());
					Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuário entrou na LanHouse (M) muito rapidamente. Tempo: %lld", User.Username, time.count());
				}

				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region LANHOUSE (MASMORRA M)
		else if (itemId == 4113)
		{
			if (GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y) == 5)
			{
				SendClientMessage(clientId, "Somente utilize em uma cidade!");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, "Disponível apenas para CELESTIAL");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 total = 0;
			for (INT32 i = 1; i < MAX_PLAYER; i++)
			{
				if (Users[i].Status != USER_PLAY)
					continue;

				if ((Mob[i].Target.X >= 3713 && Mob[i].Target.X <= 3838 && Mob[i].Target.Y >= 3459 && Mob[i].Target.Y <= 3582) ||
					(Mob[i].Target.X >= 3600 && Mob[i].Target.X <= 3700 && Mob[i].Target.Y >= 3600 && Mob[i].Target.Y <= 3700) ||
					(Mob[i].Target.X >= 3840 && Mob[i].Target.X <= 3967 && Mob[i].Target.Y >= 3584 && Mob[i].Target.Y <= 3712))
				{
					if (memcmp(Users[i].MacAddress, Users[clientId].MacAddress, 8) == 0)
						total++;
				}
			}

			if (total >= 1)
			{
				SendClientMessage(clientId, "Disponível apenas uma conta por computador na área");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (static_cast<TOD_EventItem*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::Quiz))->Register(*this, srcItem))
			{
				Teleportar(clientId, 3896, 3654);

				auto now = std::chrono::steady_clock::now();
				if (now - Times.LastLanHouseA <= 2s)
				{
					auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - Times.LastLanHouseA);

					Log(clientId, LOG_HACK, "O usuário entrou na LanHouse (A) muito rapidamente. Tempo: %lld", time.count());
					Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuário entrou na LanHouse (A) muito rapidamente. Tempo: %lld", User.Username, time.count());
				}

				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region LACTOLERIUM 100
		else if (itemId == 4141)
		{
			st_Mob* player = &Mob[clientId].Mobs.Player;
			if (p->DstType == 1)
			{
				int _type = GetEffectValueByIndex(player->Inventory[p->DstSlot].Index, EF_SELADO);
				int sanc = GetItemSanc(&player->Inventory[p->DstSlot]);

				if (sanc == 9)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				if (_type == 1)
				{
					st_Item* item = &player->Inventory[p->DstSlot];
					SetItemSanc(item, sanc + 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Poeira 100 usado em %s [%d] [%d %d %d %d %d %d]", ItemList[item->Index].Name, item->Index, item->EF1, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
				}
				else
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else
			{
				int _type = GetEffectValueByIndex(player->Equip[p->DstSlot].Index, EF_MOBTYPE);
				if (_type == 3)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine_ItemCele]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				int sanc = GetItemSanc(&player->Equip[p->DstSlot]);
				if (sanc <= 8 || sanc == 10)
				{
					if (p->DstSlot == 15)
					{
						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

						return true;
					}
					// Tentando refinar a face			
					if (p->DstSlot == 0)
						return true;

					st_Item* item = &player->Equip[p->DstSlot];

					SetItemSanc(item, sanc + 1, 0);

					SendClientMessage(clientId, g_pLanguageString[_NN_Refine_Success]);

					Log(clientId, LOG_INGAME, "Poeira 100 usado em %s [%d] [%d %d %d %d %d %d]", ItemList[item->Index].Name, item->Index, item->EF1, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
				}
				else
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				SendItem(clientId, (SlotType)p->DstType, p->DstSlot, &Mob[clientId].Mobs.Player.Equip[p->DstSlot]);
				AmountMinus(srcItem);

				SendEmotion(clientId, 14, 3);
			}
		}

#pragma endregion
#pragma region SELO DO GUERREIRO
		else if (itemId == 4146)
		{
			INT32 capeId = Mob[clientId].Mobs.Player.Equip[15].Index;

			// O que ser isso?
			if (Mob[clientId].Mobs.Player.Status.Level >= 354 && capeId != 3191 && capeId != 3192 && capeId != 3193)
			{
				INT32 newCape = -1;
				if (Mob[clientId].Mobs.Player.CapeInfo == 7)
					newCape = 3191;
				else if (Mob[clientId].Mobs.Player.CapeInfo == 8)
					newCape = 3192;
				else if (Mob[clientId].Mobs.Player.CapeInfo == CAPE_WHITE)
					newCape = 3193;

				if (newCape != -1)
				{
					memset(&Mob[clientId].Mobs.Player.Equip[15], 0, sizeof st_Item);

					Mob[clientId].Mobs.Player.Equip[15].Index = newCape;
					SendItem(clientId, SlotType::Equip, 15, &Mob[clientId].Mobs.Player.Equip[15]);
				}
			}

			Mob[clientId].Mobs.Fame += 10;

			AmountMinus(srcItem);
			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region ITENS QUEST
		else if (itemId >= 4117 && itemId <= 4121)
		{
			static DWORD expValue[5] = { 100000, 400000, 1000000, 2000000, 3000000 };
			static DWORD goldValue[5] = { 6000, 10000, 14000, 10000, 15000 };

			st_Mob* player = &Mob[clientId].Mobs.Player;
			int level = player->bStatus.Level;

			int questIndex = srcItem->Index - 4117;
			if (questIndex < 0 || questIndex > 4)
				return true;

			AmountMinus(srcItem);

			if (level < g_pQuestLevel[questIndex][0] || level > g_pQuestLevel[questIndex][1])
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Level_NotAllowed], g_pQuestLevel[questIndex][0] + 1, g_pQuestLevel[questIndex][1] + 1);

				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL && Mob[clientId].Mobs.Player.Equip[0].EFV2 != ARCH)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_OnlyMortal]);

				return true;
			}

			int gold = goldValue[questIndex];
			int exp = expValue[questIndex];

			if (player->Gold + gold > 2000000000)
				gold = (2000000000 - player->Gold);

			player->Gold += gold;

			player->Exp += exp;

			INT32 leader = Mob[clientId].Leader;
			if (leader == 0)
				leader = clientId;

			for (INT32 i = 0; i < 12; i++)
			{
				INT32 memberId = Mob[leader].PartyList[i];
				if (memberId <= 0 || memberId >= MAX_PLAYER || memberId == clientId)
					continue;

				if (Mob[memberId].Mobs.Player.Equip[0].EFV2 == MORTAL || Mob[memberId].Mobs.Player.Equip[0].EFV2 == ARCH)
				{
					if (Mob[memberId].Mobs.Player.Status.Level > g_pQuestLevel[questIndex][1])
						continue;

					Mob[memberId].Mobs.Player.Exp += (exp / 10);

					SendClientMessage(memberId, ".+++ %d de Experiência +++", exp / 10);

					SendEtc(memberId);
					SendEmotion(clientId, 14, 3);
				}
			}

			if (leader != 0 && clientId != leader && leader > 0 && leader <= MAX_PLAYER)
			{
				if (Mob[leader].Mobs.Player.Equip[0].EFV2 == MORTAL || Mob[leader].Mobs.Player.Equip[0].EFV2 == ARCH)
				{
					if (Mob[leader].Mobs.Player.Status.Level < g_pQuestLevel[questIndex][1])
					{
						Mob[leader].Mobs.Player.Exp += (exp / 10);

						SendClientMessage(leader, ".+++ %d de Experiência +++", exp / 10);

						SendEtc(leader);
						SendEmotion(clientId, 14, 3);
					}
				}
			}

			SendEmotion(clientId, 14, 3);

			Mob[clientId].GetCurrentScore(clientId);
			Mob[clientId].CheckGetLevel();

			SendScore(clientId);
			SendEtc(clientId);

			SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

			SendClientMessage(clientId, ".+++ %d de Experiência +++", exp);
		}
#pragma endregion
#pragma region PEDRA MISTERIOSA
		else if (itemId == 4148)
		{
            static clock_t delay[MAX_PLAYER] = { 0 };
			if (clock() - delay[clientId] < 5 * CLOCKS_PER_SEC)
			{
				SendClientMessage(clientId, "Aguarde alguns segundos para utilizar novamente...");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (!GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y).Village)
			{
				SendClientMessage(clientId, "Não é possível utilizar fora da cidade");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if ((Mob[clientId].Mobs.Player.Equip[0].EFV2 == CELESTIAL && Mob[clientId].Mobs.Sub.Status == 1) || (Mob[clientId].Mobs.Player.Equip[0].EFV2 == SUBCELESTIAL))
			{
                LogEquipsAndInventory(true);

				stSub* sub = &Mob[clientId].Mobs.Sub;
				st_Mob* player = &Mob[clientId].Mobs.Player;

				st_Status tmpStatus;
				memcpy(&tmpStatus, &player->bStatus, sizeof st_Status);
				memcpy(&player->bStatus, &sub->SubStatus, sizeof st_Status);
				memcpy(&sub->SubStatus, &tmpStatus, sizeof st_Status);

				st_Item item[2];
				memcpy(&item[0], &player->Equip[0], sizeof st_Item);
				memcpy(&item[1], &player->Equip[15], sizeof st_Item);

				memcpy(&player->Equip[0], &sub->Equip[0], sizeof st_Item);
				memcpy(&sub->Equip[0], &item[0], sizeof st_Item);

				INT64 exp = player->Exp;
				player->Exp = sub->Exp;
				sub->Exp = exp;

				stDate date;
				memcpy(&date, &Mob[clientId].Mobs.Escritura, sizeof stDate);

				INT32 baseFace = player->Equip[0].EF2;
				INT32 classInfo = 0;
				if (baseFace < 10)
					classInfo = (baseFace - 6);
				else if (baseFace < 20)
					classInfo = (baseFace - 16);
				else if (baseFace < 30)
					classInfo = (baseFace - 26);
				else if (baseFace < 40)
					classInfo = (baseFace - 36);

				player->ClassInfo = classInfo;

				Log(clientId, LOG_INGAME, "Classe %d gerada através da face %d", classInfo, baseFace);

				auto learn = player->Learn[0];
				auto secLearn = player->Learn[1];
				player->Learn[0] = sub->Learn;
				player->Learn[1] = sub->SecLearn;

				sub->Learn = learn;
				sub->SecLearn = secLearn;

				INT32 soul = Mob[clientId].Mobs.Soul;
				Mob[clientId].Mobs.Soul = sub->Soul;
				sub->Soul = soul;

				stQuestInfo tmpInfo;
				tmpInfo.Value = Mob[clientId].Mobs.Info.Value;
				Mob[clientId].Mobs.Info.Value = sub->Info.Value;
				sub->Info.Value = tmpInfo.Value;

				st_Affect affect[32];
				memcpy(affect, Mob[clientId].Mobs.Affects, sizeof st_Affect * 32);
				memcpy(Mob[clientId].Mobs.Affects, sub->Affect, sizeof st_Affect * 32);
				memcpy(sub->Affect, affect, sizeof st_Affect * 32);

				unsigned char skillBar[20];
				memset(&skillBar, 255, sizeof skillBar);

				memcpy(skillBar, player->SkillBar1, 4);
				memcpy(&skillBar[4], &Mob[clientId].Mobs.SkillBar[0], 16);

				memcpy(player->SkillBar1, sub->SkillBar, 4);
				memcpy(Mob[clientId].Mobs.SkillBar, &sub->SkillBar[4], 16);

				memcpy(sub->SkillBar, skillBar, 20);

				sub->Status = 1;

				if (Mob[clientId].Mobs.Player.Equip[0].EFV2 == 3)
					Mob[clientId].Mobs.Player.Equip[0].EFV2 = 3;
				else
					Mob[clientId].Mobs.Player.Equip[0].EFV2 = 4;

				Mob[clientId].GetCurrentScore(clientId);

				if (Mob[clientId].Mobs.Player.GetEvolution() >= Celestial && Mob[clientId].Mobs.Player.bStatus.Level == 199 && Mob[clientId].Mobs.Player.bStatus.Level < 399 && !Mob[clientId].Mobs.Info.Unlock200 && !Mob[clientId].Mobs.Info.LvBlocked && Mob[clientId].Mobs.GetTotalResets() == 3)
				{
					Log(clientId, LOG_INGAME, "Bloqueado o nível 200 do personagem devido a não estar bloqueado e não ter desbloqueado.");

					Mob[clientId].Mobs.Info.LvBlocked = 1;
					Mob[clientId].Mobs.Player.Exp = g_pNextLevel[Mob[clientId].Mobs.Player.Equip[0].EFV2][Mob[clientId].Mobs.Player.bStatus.Level - 1];
				}

				p364 packet;
				GetCreateMob(clientId, (BYTE*)& packet);

				GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

				SendItem(clientId, SlotType::Equip, 0, &Mob[clientId].Mobs.Player.Equip[0]);

				SendScore(clientId);
				SendEtc(clientId);

				p378 skill;
				skill.Header.PacketId = 0x378;
				skill.Header.Size = sizeof p378;
				skill.Header.ClientId = clientId;

				memcpy(skill.SkillBar1, player->SkillBar1, 4);
				memcpy(skill.SkillBar2, Mob[clientId].Mobs.SkillBar, 16);

				AddMessage((BYTE*)& skill, sizeof p378);

				SaveUser(clientId, 0);
				AmountMinus(srcItem);

				delay[clientId] = clock();
			}
			else
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion
#pragma region POÇÃO DO VIGOR
		else if (itemId == 3313)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 35)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Mob[clientId].Mobs.Affects[slot].Index = 35;
			Mob[clientId].Mobs.Affects[slot].Value = 2;
			Mob[clientId].Mobs.Affects[slot].Time = 900;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region JÓIAS
		else if (itemId >= 3200 && itemId <= 3209)
		{
			INT32 index = itemId - 3200;

			INT32 value = 0;
			switch (index)
			{
			case 0:
				value = 1;
				break;
			case 1:
				value = 2;
				break;
			case 2:
				value = 4;
				break;
			case 3: // Jóia da Recuperação
				break;
			case 4:
				value = 8;
				break;
			case 5:
				value = 16;
				break;
			case 6:
				value = 32;
				break;
			case 7: // Jóia armazenagem
				break;
			case 8:
				value = 64;
				break;
			case 9:
				value = 128;
				break;
			}

			if (value == 0)
			{
				if (index == 3)
				{
					INT32 i = 0;
					for (; i < 32; i++)
					{
						if (Mob[clientId].Mobs.Affects[i].Index == 32) // Cancel
						{
							memset(&Mob[clientId].Mobs.Affects[i], 0, sizeof st_Affect);

							SendAffect(clientId);

							p364 packet;
							GetCreateMob(clientId, (BYTE*)& packet);

							GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);
							break;
						}
					}

					if (i == 32)
					{
						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

						return true;
					}
				}

				AmountMinus(srcItem);
				return true;
			}

			INT32 slot = GetEmptyAffect(clientId, 8);

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Mob[clientId].Mobs.Affects[slot].Index = 8;
			Mob[clientId].Mobs.Affects[slot].Time = 450;

			if (!(Mob[clientId].Mobs.Affects[slot].Value & value))
				Mob[clientId].Mobs.Affects[slot].Value |= value;

			Mob[clientId].Jewel = slot;

			SendAffect(clientId);

			p364 packet;
			GetCreateMob(clientId, (BYTE*)& packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)& packet, 0);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region ABRIR PACLAC
		else if (itemId == 3447 || itemId == 3448)
		{
			INT32 amount = GetItemAmount(srcItem);

			INT32 i;
			for (i = 0; i < amount; i++)
			{
				INT32 slotId = GetFirstSlot(clientId, 0);
				if (slotId == -1)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

					break;
				}
				else
				{
					memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

					Mob[clientId].Mobs.Player.Inventory[slotId].Index = (412 + (srcItem->Index - 3447));
					SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);

					AmountMinus(srcItem);
				}
			}

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion
#pragma region SELO DO GUERREIRO
		else if (itemId == 4146)
		{
			Mob[clientId].Mobs.Fame += 10;

			if (Mob[clientId].Mobs.Player.Equip[15].Index != 3191 && Mob[clientId].Mobs.Player.Equip[15].Index != 3192 && Mob[clientId].Mobs.Player.bStatus.Level >= 354)
			{
				INT32 cape = 3193,
					capeInfo = Mob[clientId].Mobs.Player.CapeInfo;

				if (capeInfo == CAPE_BLUE)
					cape = 3191;
				else if (capeInfo == CAPE_RED)
					cape = 3192;
				else
					cape = 3193;

				st_Item* item = &Mob[clientId].Mobs.Player.Equip[15];
				memset(item, 0, sizeof st_Item);

				item->Index = cape;

				// Seta a refinação como +0
				SetItemSanc(item, 0, 0);

				SendItem(clientId, SlotType::Equip, 15, item);
				SendEquip(clientId);

				Log(clientId, LOG_INGAME, "Adquirido capa %d com Selo do Guerreiro", cape);
			}

			Log(clientId, LOG_INGAME, "Adquirido fame pelo Selo do Guerreiro - %d fame", Mob[clientId].Mobs.Fame);

			AmountMinus(srcItem);
			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region CRISTAIS ARCH
		else if (itemId >= 4106 && itemId <= 4109)
		{
			INT32 classMaster = Mob[clientId].Mobs.Player.Equip[0].EFV2,
				level = Mob[clientId].Mobs.Player.Status.Level;

			INT32 index = itemId - 4106;
			BOOL conc = FALSE,
				anterior = TRUE;

			if (index == 0)
				conc = Mob[clientId].Mobs.Info.Elime;
			else if (index == 1)
			{
				if (!Mob[clientId].Mobs.Info.Elime)
					anterior = FALSE;
				conc = Mob[clientId].Mobs.Info.Sylphed;
			}
			else if (index == 2)
			{
				if (!Mob[clientId].Mobs.Info.Elime)
					anterior = FALSE;
				else if (!Mob[clientId].Mobs.Info.Sylphed)
					anterior = FALSE;

				conc = Mob[clientId].Mobs.Info.Thelion;
			}
			else if (index == 3)
			{
				if (!Mob[clientId].Mobs.Info.Elime)
					anterior = FALSE;
				else if (!Mob[clientId].Mobs.Info.Sylphed)
					anterior = FALSE;
				else if (!Mob[clientId].Mobs.Info.Thelion)
					anterior = FALSE;

				conc = Mob[clientId].Mobs.Info.Noas;
			}

			if (Mob[clientId].Mobs.Player.Exp < 100000000)
			{
				SendClientMessage(clientId, "Experiência insuficiente.");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (conc)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (!anterior)
			{
				SendClientMessage(clientId, "Conclua a quest anterior para continuar");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}
			if (classMaster == ARCH)
			{
				if (level < 354)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantTele_Nip]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else if (classMaster == CELESTIAL)
			{
				if (Mob[clientId].Mobs.Fame < 100)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Need_XX_Fame], 100);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				if (level < 39)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_CantTele_Nip]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (index == 0)
				Mob[clientId].Mobs.Info.Elime = 1;
			else if (index == 1)
				Mob[clientId].Mobs.Info.Sylphed = 1;
			else if (index == 2)
				Mob[clientId].Mobs.Info.Thelion = 1;
			else if (index == 3)
				Mob[clientId].Mobs.Info.Noas = 1;

			Mob[clientId].Mobs.Player.Exp -= 100000000;

			switch (classMaster)
			{

			case ARCH:
				while (Mob[clientId].Mobs.Player.Exp < g_pNextLevel[classMaster][level - 1])
				{
					Mob[clientId].Mobs.Player.bStatus.Level--;
					level--;
				}
				break;

			case CELESTIAL:
				while (level > 0 && Mob[clientId].Mobs.Player.Exp < g_pNextLevel[classMaster][level - 1])
				{
					Mob[clientId].Mobs.Player.bStatus.Level--;
					level--;
				}
				break;
			}

			Mob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendEtc(clientId);

			AmountMinus(srcItem);

			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region  ACELERADOR DE NASCIMENTO
		else if (itemId == 3438)
		{
			st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

			if (dstItem->Index < 2300 || dstItem->Index >= 2330)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			int totalIncubate = GetItemAbility(dstItem, EF_INCUBATE) - 1;
			if (totalIncubate <= 0)
			{
				INT32 mountIndex = dstItem->Index - 2300;
				dstItem->Index = 2330 + mountIndex;

				*(short*)& dstItem->EF1 = 5000;

				dstItem->EF2 = 1;
				dstItem->EFV2 = (Rand() % 20 + 10);

				dstItem->EF3 = 30;
				dstItem->EFV3 = 1;

				SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);

				Log(clientId, LOG_INGAME, "Montaria cresceu com acelerador de nascimento: %s [%d] [%d %d %d %d %d %d]", ItemList[dstItem->Index].Name, dstItem->Index, dstItem->EF1,
					dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

				MountProcess(clientId, 0);
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					if (dstItem->Effect[i].Index == EF_INCUBATE)
					{
						dstItem->Effect[i].Value = totalIncubate;

						break;
					}
				}

				SendClientMessage(clientId, "A incubação foi reduzida");
				Log(clientId, LOG_INGAME, "Diminuído incubação do item %s %s", ItemList[dstItem->Index].Name, dstItem->toString().c_str());
			}

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, &Mob[clientId].Mobs.Player.Equip[14]);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region CATALISADORES
		else if (itemId >= 3344 && itemId <= 3350)
		{
			INT32 index = itemId - 3344;
			st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

			INT32 mountId = dstItem->Index;
			INT32 i = 0;
			for (; i < 6; i++)
			{
				if (dstItem->Index == (g_pItemRest[index][i] - 30))
					break;
			}

			if (i == 6)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);
				return true;
			}

			Mob[clientId].Mobs.Player.Equip[14].Index = mountId + 30;

			*(short*)& Mob[clientId].Mobs.Player.Equip[14].EF1 = 5000;

			Mob[clientId].Mobs.Player.Equip[14].EF2 = 1;
			Mob[clientId].Mobs.Player.Equip[14].EFV2 += Rand() % 16 + 5;

			Mob[clientId].Mobs.Player.Equip[14].EF3 = 100;
			Mob[clientId].Mobs.Player.Equip[14].EFV3 = 0;

			SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);

			Log(clientId, LOG_INGAME, "Montaria cresceu : %s [%d] [%d %d %d %d %d %d]", ItemList[Mob[clientId].Mobs.Player.Equip[14].Index].Name, Mob[clientId].Mobs.Player.Equip[14].Index, Mob[clientId].Mobs.Player.Equip[14].EF1,
				Mob[clientId].Mobs.Player.Equip[14].EFV1, Mob[clientId].Mobs.Player.Equip[14].EF2, Mob[clientId].Mobs.Player.Equip[14].EFV2, Mob[clientId].Mobs.Player.Equip[14].EFV2, Mob[clientId].Mobs.Player.Equip[14].EF3,
				Mob[clientId].Mobs.Player.Equip[14].EFV3);

			MountProcess(clientId, 0);

			SendClientMessage(clientId, g_pLanguageString[_NN_INCUBATED]);

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			SendEmotion(clientId, 14, 3);

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region RESTAURADORES
		else if (itemId >= 3351 && itemId <= 3357)
		{
			INT32 index = itemId - 3351;
			st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);

			if (dstItem->Index <= 2360 || dstItem->Index >= 2390 || index < 0 || index > 6)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);

				return true;
			}

			if (dstItem->EFV2 >= 60)
			{
				SendClientMessage(clientId, "Limite de 60 pontos de vitalidade");

				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);
				return true;
			}

			INT32 i = 0;
			for (; i < 6; i++)
			{
				if (dstItem->Index == g_pItemRest[index][i])
					break;
			}

			if (i == 6)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, (SlotType)p->SrcType, p->DstType, srcItem);
				return true;
			}

			int rand = (Rand() % 3) + 1;
			int vit = dstItem->EFV2 + rand;
			if (vit > 60)
				vit = 60;

			dstItem->EFV2 = vit;

			Log(clientId, LOG_INGAME, "Utilizado item %s [%d] em %s [%d] [%d %d %d %d %d %d]. Vitalidade aumentada em %d", ItemList[itemId].Name, itemId, ItemList[dstItem->Index].Name, dstItem->Index, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3,
				rand);

			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);
			AmountMinus(srcItem);

			SendEmotion(clientId, 14, 3);
		}
#pragma endregion
#pragma region EXTRAÇÕES
		else if (itemId >= 3021 && itemId <= 3026)
		{
			int destino = srcItem->Index - 3020;
			if (Mob[clientId].Mobs.Player.Equip[destino].Index == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* dstItem = GetItemPointer(clientId, (int)SlotType::Equip, destino);
			if (dstItem->Index <= 0 || dstItem->Index >= MAX_ITEMLIST)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 itemType = -1;
			for (INT32 i = 0; i < 3; i++)
			{
				if (srcItem->Effect[i].Index == 87)
				{
					itemType = srcItem->Effect[i].Value;

					break;
				}
			}

			if (itemType == -1)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Bad_Network_Packets]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 ability = GetItemAbility(dstItem, EF_UNKNOW1);
			if (ability != itemType && itemType <= 5)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Items_Same_Class]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 sanc = GetItemSanc(dstItem);

			INT32 mobType = GetEffectValueByIndex(dstItem->Index, EF_MOBTYPE);
			if (itemType == 5)
			{
				if (mobType == 1)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Items_JustMortal]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else if (itemType >= 0 && itemType <= 4)
			{
				if (sanc > 9)
				{
					SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}
			else if (itemType > 5)
			{
				if (itemType == 11 || itemType == 10)
				{// 11 = Arma arch +10+ Anct
				 // 10 = Arma arch +9- Anct
					INT32 grade = ItemList[dstItem->Index].Grade;
					if (grade < 5 || grade > 8)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_Anct]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					if (mobType != 1)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_Arch]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					INT32 tmpSanc = GetItemSanc(dstItem);
					if (itemType == 10)
					{
						if (tmpSanc > 9)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}
					}
					else
					{
						if (tmpSanc != 11)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Plus11]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}

						sanc = 11;
					}
				}

				else if (itemType == 9 || itemType == 8)
				{
					INT32 grade = ItemList[dstItem->Index].Grade;
					if (grade >= 5 && grade <= 8)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_NotAnct]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					if (mobType != 1)
					{
						SendClientMessage(clientId, g_pLanguageString[_NN_Items_Arch]);

						SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
						return true;
					}

					INT32 tmpSanc = GetItemSanc(dstItem);
					if (itemType == 9)
					{
						if (tmpSanc != 11)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}

						sanc = 11;
					}
					else
					{
						if (tmpSanc > 9)
						{
							SendClientMessage(clientId, g_pLanguageString[_NN_Items_Smaller9]);

							SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
							return true;
						}
					}
				}
			}

			INT32 ref = 0;
			for (INT32 i = 0; i < 3; i++)
			{
				if (dstItem->Effect[i].Index == EF_SANC || (dstItem->Effect[i].Index >= 116 && dstItem->Effect[i].Index <= 125))
				{
					ref = dstItem->Effect[i].Index;

					if (sanc == 0)
						sanc = dstItem->Effect[i].Value;

					break;
				}
			}

			for (INT32 i = 0; i < 3; i++)
			{
				if (srcItem->Effect[i].Index == 87)
				{
					srcItem->Effect[i].Index = ref;

					break;
				}
			}

			Log(clientId, LOG_INGAME, "Utilizou extração no item %s [%d] [%d %d %d %d %d %d]", ItemList[dstItem->Index].Name, dstItem->Index, dstItem->EF1, dstItem->EFV1, dstItem->EF2, dstItem->EFV2, dstItem->EF3, dstItem->EFV3);

			SetItemSanc(srcItem, sanc, 0);

			srcItem->Index = dstItem->Index;

			INT32 index = dstItem->Index;
			memcpy(dstItem, srcItem, sizeof st_Item);

			dstItem->Index = index;

			AmountMinus(srcItem);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			SendItem(clientId, SlotType::Equip, destino, dstItem);

		}
#pragma endregion
#pragma region COMPOSTO DO EQUILIBRIO
		else if (itemId == 4126)
		{
			if (Mob[clientId].Mobs.Player.bStatus.Level < 119 ||
				Mob[clientId].Mobs.Player.bStatus.Level > 124)
			{
				SendClientMessage(clientId, "Level inadequado.");

				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL)
			{
				SendClientMessage(clientId, "Somente para mortais.");

				return true;
			}

			if (Mob[clientId].Mobs.Info.BalanceQuest != 0)
			{
				SendClientMessage(clientId, "Quest já realizada.");

				return true;
			}

			INT32 totalMasterPoint = Mob[clientId].Mobs.Player.MasterPoint;
			for (INT32 i = 0; i < 4; i++)
			{
				totalMasterPoint += Mob[clientId].Mobs.Player.bStatus.Mastery[i];

				Mob[clientId].Mobs.Player.bStatus.Mastery[i] = 0;
			}

			Mob[clientId].Mobs.Player.MasterPoint = totalMasterPoint;

			for (int i = 0; i < 3; i++)
			{
				int initial = (i * 8);
				for (int a = initial; a < initial + 8; a++)
				{
					int has = (Mob[clientId].Mobs.Player.Learn[0] & (1 << a));
					if (has != 0)
						Mob[clientId].Mobs.Player.Learn[0] -= (1 << a);
				}
			}

			Mob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendAffect(clientId);

			AmountMinus(srcItem);

			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			Mob[clientId].Mobs.Info.BalanceQuest = 1;

			SendClientMessage(clientId, "Processo concluido.");

			return true;
		}
#pragma endregion
#pragma region MOLAR DE GÁRGULA
		else if (itemId == 4122)
		{
			if (Mob[clientId].Mobs.Player.bStatus.Level < 199 || Mob[clientId].Mobs.Player.bStatus.Level > 254)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Level_NotAllowed], 200, 255);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Info.MolarGargula)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Youve_Done_It_Already]);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Mob[clientId].Mobs.Info.MolarGargula = 1;

			for (INT32 i = 1; i < 6; i++)
			{
				st_Item* item = &Mob[clientId].Mobs.Player.Equip[i];
				if (item->Index <= 0 || item->Index >= MAX_ITEMLIST)
				{
					Log(clientId, LOG_INGAME, "Molar de Gárgula - Slot %d : não possuia item equipado", i);
					continue;
				}

				Log(clientId, LOG_INGAME, "Molar de Gárgula - Slot %d : %s [%d] [%d %d %d %d %d %d]", i, ItemList[item->Index].Name,
					item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3);

				INT32 sanc = GetItemSanc(item);
				if (sanc >= 6)
				{
					Log(clientId, LOG_INGAME, "Molar de Gárgula - Slot %d : %s item não refinado por ser +%d", i, ItemList[item->Index].Name, sanc);
					continue;
				}

				sanc += 4;
				if (sanc > 6)
					sanc = 6;

				SetItemSanc(item, sanc, 0);
				SendItem(clientId, SlotType::Equip, i, item);
			}

			AmountMinus(srcItem);
			Log(clientId, LOG_INGAME, "%s finalizou a quest do molár do gargula.", Mob[clientId].Mobs.Player.Name);
		}
#pragma endregion
#pragma region COMPOSTO DE CHANCE
		else if (itemId == 4124)
		{
			if (Mob[clientId].Mobs.Player.bStatus.Level < 69 ||
				Mob[clientId].Mobs.Player.bStatus.Level > 74)
			{
				SendClientMessage(clientId, "Level inadequado.");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 != MORTAL)
			{
				SendClientMessage(clientId, "Somente para mortais.");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Info.GodBless != 0)
			{
				SendClientMessage(clientId, "Quest já realizada.");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			INT32 totalMasterPoint = Mob[clientId].Mobs.Player.MasterPoint;
			for (INT32 i = 0; i < 4; i++)
			{
				if (Mob[clientId].Mobs.Player.bStatus.Mastery[i] >= 50)
				{
					totalMasterPoint += Mob[clientId].Mobs.Player.bStatus.Mastery[i];

					Mob[clientId].Mobs.Player.bStatus.Mastery[i] -= 50;
				}
			}

			Mob[clientId].Mobs.Player.MasterPoint = totalMasterPoint;

			for (int i = 0; i < 3; i++)
			{
				int initial = (i * 8);
				for (int a = initial; a < initial + 8; a++)
				{
					int has = (Mob[clientId].Mobs.Player.Learn[0] & (1 << a));
					if (has != 0)
						Mob[clientId].Mobs.Player.Learn[0] -= (1 << a);
				}
			}

			Mob[clientId].GetCurrentScore(clientId);

			SendScore(clientId);
			SendAffect(clientId);
			SendEtc(clientId);

			Mob[clientId].Mobs.Info.GodBless = 1;

			AmountMinus(srcItem);
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			return true;
		}
#pragma endregion
#pragma region RECUSA DE GUERRA
		else if (itemId == 4031)
		{
			if (Mob[clientId].Mobs.Player.GuildMemberType != 9 && !IsAdmin)
			{
				SendClientMessage(clientId, "Somente para líder de guild.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (ChargedGuildList[sServer.Channel - 1][4] != Mob[clientId].Mobs.Player.GuildIndex && !IsAdmin) // Somente caso possua a coroa
			{
				SendClientMessage(clientId, "Necessário possuir coroa.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			UpdateWarDeclaration(0);

			AmountMinus(srcItem);
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
		}
#pragma endregion
#pragma region RESET KD
		else if (itemId == 4532)
		{
			Mob[clientId].Mobs.Counters.PvP.Kill = 0;
			Mob[clientId].Mobs.Counters.PvP.Death = 0;

			SendClientMessage(clientId, "Resetado com sucesso");

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region MOEDA DE CASH
		else if (itemId == 4533 || itemId == 4534 || itemId == 4535)
		{
			INT32 cash = (itemId - 4533);

			if (cash == 0)
				cash = 200;
			else if (cash == 1)
				cash = 500;
			else if (cash == 2)
				cash = 1000;

			Users[clientId].User.Cash += cash;

			SendClientMessage(clientId, "Foram adicionados %d OverPoints em sua conta. Saldo total: %u", cash, Users[clientId].User.Cash);
			AmountMinus(srcItem);

			SendSignalParm(clientId, 0x7530, RefreshGoldPacket, User.Cash);
		}
#pragma endregion
#pragma region NUCLEO DE LAC
		else if (itemId == NUCLEO_LAC_ITEMID)
		{
			if (srcItem->Effect[0].Index != EF_AMOUNT)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				SendClientMessage(clientId, "Núcleo de Lac não inicializado.");
				return true;
			}

			int rand = Rand() % 100;
			if (rand > srcItem->Effect[0].Value && srcItem->Effect[0].Value != 100)
			{
				memset(srcItem, 0, sizeof(st_Item));

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				SendClientMessage(clientId, "Falha na composição.");
				return true;
			}

			// Deleta o núcleo.
			memset(srcItem, 0, sizeof(st_Item));
			SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

			// Entrega o lac 100%
			st_Item itemLac100;
			memset(&itemLac100, 0, sizeof(st_Item));

			itemLac100.Index = 4141;

			PutItem(clientId, &itemLac100);

			SendClientMessage(clientId, "Item composto com sucesso!");

			return true;
		}
#pragma endregion
#pragma region FLASH DO VALE
		else if (itemId == 4688)
		{
			int fadaId = Mob[clientId].Mobs.Player.Equip[13].Index;
			if (fadaId != 3916)
			{
				SendClientMessage(clientId, "Somente disponível para Fada do Vale");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Recebido %s por usar o %s em %s (%s)", ItemList[3917].Name, ItemList[itemId].Name, ItemList[fadaId].Name, Mob[clientId].Mobs.Player.Equip[13].toString().c_str());
			Mob[clientId].Mobs.Player.Equip[13].Index = 3917;

			SendItem(clientId, SlotType::Equip, 13, &Mob[clientId].Mobs.Player.Equip[13]);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region FLASH PRATEADO / DOURADO
		else if (itemId == 3451 || itemId == 3452)
		{
			int fadaId = Mob[clientId].Mobs.Player.Equip[13].Index;
			if (fadaId != 3902 && fadaId != 3905 && fadaId != 3908)
			{
				SendClientMessage(clientId, "Somente disponível para Fada Vermelha");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int destinyId = 3914;
			if (srcItem->Index == 3452)
				destinyId = 3915;

			Log(clientId, LOG_INGAME, "Recebido %s por usar o %s em %s (%s)", ItemList[destinyId].Name, ItemList[itemId].Name, ItemList[fadaId].Name, Mob[clientId].Mobs.Player.Equip[13].toString().c_str());
			Mob[clientId].Mobs.Player.Equip[13].Index = destinyId;

			SendItem(clientId, SlotType::Equip, 13, &Mob[clientId].Mobs.Player.Equip[13]);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region BAÚ MERCANTIL
		else if (itemId == 4545)
		{
			INT32 key = GetFirstSlot(clientId, 4546);
			if (key == -1)
			{
				if (Mob[clientId].Mobs.Player.Gold < 10000000)
				{
					SendClientMessage(clientId, "Necessário 10 milhões de gold ou Chave Mercantil!");

					SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
					return true;
				}

				Mob[clientId].Mobs.Player.Gold -= 10000000;
				SendSignalParm(clientId, clientId, 0x3AF, Mob[clientId].Mobs.Player.Gold);
			}
			else
			{
				AmountMinus(&Mob[clientId].Mobs.Player.Inventory[key]);

				SendItem(clientId, SlotType::Inv, key, &Mob[clientId].Mobs.Player.Inventory[key]);
			}

			INT32 _rand = Rand() % 100;

			st_Item item;
			memset(&item, 0, sizeof item);

			if (_rand <= 50)
			{
				_rand = Rand() % 2;

				if (_rand == 0)
					item.Index = 4547;
				else if (_rand == 1)
				{
					item.Index = 412;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 5;
				}
			}
			else if (_rand <= 90)
			{
				_rand = Rand() % 3;

				if (_rand == 0)
				{
					item.Index = 413;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 2;
				}
				else if (_rand == 1)
				{
					item.Index = 404;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 50;
				}
				else if (_rand == 2)
				{
					item.Index = 409;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 50;
				}
			}
			else
			{
				_rand = Rand() % 1;

				if (_rand == 0)
				{
					item.Index = 4547;

					item.EF1 = EF_AMOUNT;
					item.EFV1 = 3;
				}
			}

			AmountMinus(srcItem);

			if (PutItem(clientId, &item))
			{
				SendClientMessage(clientId, "!Chegou um item: [ %s ]", ItemList[item.Index].Name);

				Log(clientId, LOG_INGAME, "Recebeu o item %s [%d] da Caixa Mercantil!", ItemList[item.Index].Name, item.Index);
			}
			else
			{
				SendClientMessage(clientId, "!Não recebeu o item por falta de espaço!");

				Log(clientId, LOG_INGAME, "NÃO Recebeu o item %s [%d] da Caixa Mercantil! - FALTA DE ESPAÇO NO INVENTÁRIO", ItemList[item.Index].Name, item.Index);
			}
		}
#pragma endregion 
#pragma region TROCA DE CORPOS
		else if (itemId == 4649)
		{
			auto& player = Mob[clientId].Mobs.Player;
			for(int i = 1; i < 8; ++i)
			{
				if (player.Equip[i].Index != 0)
				{
					SendClientMessage(clientId, "Necessário desequipar todos os seus itens");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			if (player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, "Disponível apenas para celestiais ou superior");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int sephirotId = player.Equip[11].Index;
			if (sephirotId < 1760 || sephirotId > 1763)
			{
				SendClientMessage(clientId, "Necessário a sephirot da classe que deseja o novo corpo");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			sephirotId -= 1760;

			{
				int newFace = (sephirotId * 10) + (player.ClassInfo + 6);
				int oldFace = player.Equip[0].Index;

				if (newFace == oldFace)
				{
					SendClientMessage(clientId, "Não é possível trocar para a mesma classe");

					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}

				player.Equip[0].Index = newFace;
				player.Equip[0].EF2 = newFace;
				Log(clientId, LOG_INGAME, "trocou o corpo de %d para %d (%hhu) - Evolução: %hhu", oldFace, newFace, Mob[clientId].Mobs.Player.ClassInfo, Mob[clientId].Mobs.Player.Equip[0].EFV2);
			}

			bool hasSub = Mob[clientId].Mobs.Sub.Status == 1 || player.Equip[0].EFV2 == SUBCELESTIAL;
			if (hasSub)
			{
				auto& sub = Mob[clientId].Mobs.Sub;

				int baseFace = sub.Equip[0].EF2;
				int classInfo = 0;
				if (baseFace < 10)
					classInfo = (baseFace - 6);
				else if (baseFace < 20)
					classInfo = (baseFace - 16);
				else if (baseFace < 30)
					classInfo = (baseFace - 26);
				else if (baseFace < 40)
					classInfo = (baseFace - 36);

				int newFace = (sephirotId * 10) + (classInfo + 6);
				int oldFace = sub.Equip[0].Index;

				sub.Equip[0].Index = newFace;
				sub.Equip[0].EF2 = newFace;

				Log(clientId, LOG_INGAME, "trocou o corpo de %d para %d (%hhu) - Evolução: %hhu", oldFace, newFace, classInfo, sub.Equip[0].EFV2);
			}

			AmountMinus(srcItem);
			player.Equip[11] = st_Item{};

			SendClientMessage(clientId, "Que os Deuses continuem te abençoando");
			CharLogOut(clientId);

			SendSignalParm(clientId, clientId, 0x3B4, Users[clientId].inGame.CharSlot);
			return true;
		}
#pragma endregion
#pragma region TROCA DE CLASSE 
		else if (itemId == 3486)
		{
			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 < ARCH)
			{
				SendClientMessage(clientId, "Não disponível para mortais!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* seph = &Mob[clientId].Mobs.Player.Equip[11];
			if (seph->Index < 1760 || seph->Index > 1763)
			{
				SendClientMessage(clientId, "Necessário o sephirot da classe que deseja.");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			INT32 newClass = seph->Index - 1760;
			INT32 myFace = Mob[clientId].Mobs.Player.Equip[0].EF2 - Mob[clientId].Mobs.Player.ClassInfo;

			switch (myFace)
			{
			case 6:
			case 16:
			case 26:
			case 36:
				break;
			default:
				SendClientMessage(clientId, "Incorreto. Contate a administração #0001");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int newFace = myFace + newClass;
			int oldFace = Mob[clientId].Mobs.Player.Equip[0].Index;
			int oldClass = Mob[clientId].Mobs.Player.ClassInfo;

			if (newFace == oldFace)
			{
				SendClientMessage(clientId, "Não é possível trocar para a mesma classe");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Mob[clientId].Mobs.Player.ClassInfo = newClass;

			Mob[clientId].Mobs.Player.Equip[0].Index = newFace;
			Mob[clientId].Mobs.Player.Equip[0].EF2 = newFace;

			memset(seph, 0, sizeof st_Item);
			AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "%s trocou a classe de %d (%d) para %d (%hhu) - %hhu", Mob[clientId].Mobs.Player.Name, oldFace, oldClass, newFace, Mob[clientId].Mobs.Player.ClassInfo, Mob[clientId].Mobs.Player.Equip[0].EFV2);

			SendClientMessage(clientId, "Que os Deuses continuem te abençoando");
			CharLogOut(clientId);

			SendSignalParm(clientId, clientId, 0x3B4, Users[clientId].inGame.CharSlot);
			return true;
		}
#pragma endregion
#pragma region CARTA DE DUELO
		else if (itemId == 3172 || itemId == 1731 || itemId == 3171)
		{
			auto event = static_cast<TOD_EventItem*>(TOD_EventManager::GetInstance().GetEvent(TOD_EventType::DuelLetter));
			if (event == nullptr)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (Mob[clientId].Target.X < 1045 || Mob[clientId].Target.X > 1047 || Mob[clientId].Target.Y < 1689 || Mob[clientId].Target.Y > 1691)
			{
				SendClientMessage(clientId, "Utilize no Altar de Thor!");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (!event->CanRegister(*this))
			{
				SendClientMessage(clientId, "Quest já ocupada");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Leader != 0)
			{
				SendClientMessage(clientId, "Somente o líder do grupo pode utilizar");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (event->Register(*this, srcItem))
			{
				Log(clientId, LOG_INGAME, "Registrado na Carta de duelo");
				AmountMinus(srcItem);
			}
		}
#pragma endregion
#pragma region REMOVEDOR DE TRAJE DE MONTARIA
		else if (itemId == 4595)
		{
			st_Item& item = Mob[clientId].Mobs.Player.Equip[14];
			INT32 traje = item.Effect[2].Value;

			if (traje < 11 || item.Index < 2360 || item.Index >= 2390)
			{
				SendClientMessage(clientId, "Utilize com uma montaria utilizando traje");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (traje != 12 && traje != 13 && traje != 14 && traje != 15 && traje != 16 && traje != 20)
			{
				SendClientMessage(clientId, "Você não pode remover este traje");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = -1;
			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Removido traje da montaria. Index do traje que havia: %d. Item info: %s", traje, item.toString().c_str());

			item.Effect[2].Value = 0;

			memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);
			Mob[clientId].Mobs.Player.Inventory[slotId].Index = 4190 + (traje - 11);

			SendItem(clientId, SlotType::Equip, 14, &Mob[clientId].Mobs.Player.Equip[14]);
			SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);

			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region POÇÃO GELADA
		else if (itemId == 4624)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 54)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Mob[clientId].Mobs.Affects[slot].Index = 54;
			Mob[clientId].Mobs.Affects[slot].Time = 2700; // = 6hrs
			Mob[clientId].Mobs.Affects[slot].Value = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)&packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region POÇÃO ANNUBIS
		else if (itemId == 4668)
		{
			INT32 slot = -1;
			for (INT32 i = 0; i < 32; i++)
			{
				if (Mob[clientId].Mobs.Affects[i].Index == 59)
				{
					slot = i;

					break;
				}
			}

			if (slot == -1)
			{
				for (INT32 i = 0; i < 32; i++)
				{
					if (Mob[clientId].Mobs.Affects[i].Index == 0)
					{
						slot = i;

						break;
					}
				}
			}

			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Mob[clientId].Mobs.Affects[slot].Index = 59;
			Mob[clientId].Mobs.Affects[slot].Time = 2700; // = 6hrs
			Mob[clientId].Mobs.Affects[slot].Value = 1;

			p364 packet;
			GetCreateMob(clientId, (BYTE*)&packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region PANQUECA
		else if (itemId == 4639)
		{
			int slot = GetEmptyAffect(clientId, 55);
			if (slot == -1)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			Mob[clientId].Mobs.Affects[slot] = st_Affect{};
			Mob[clientId].Mobs.Affects[slot].Index = 55;
			Mob[clientId].Mobs.Affects[slot].Time = 900; // = 6hrs
			Mob[clientId].Mobs.Affects[slot].Value = 1;

			p364 packet{};
			GetCreateMob(clientId, (BYTE*)&packet);

			GridMulticast_2(Mob[clientId].Target.X, Mob[clientId].Target.Y, (BYTE*)&packet, 0);

			SendAffect(clientId);
			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region BAÚ CONGELADO
		else if (itemId == 4541)
		{
			struct FrozenBox
			{
				int ItemIndex;
				int Amount;
				bool Splitted{ false };
			};

			if (GetInventoryAmount(clientId, 0) < 3)
			{
				SendClientMessage(clientId, "No mínimo 3 slots vagos no inventário para usar este item");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento. Mínimo de 3 itens");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<FrozenBox> items
			{
				{
				{ 4542, 1 },
				{ 4140, 5 },
				{ 3314, 3 },
				{ 4538, 1 },
				{ 4522, 3, true },
				{ 4028, 1 }
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				if (!earnedItem.Splitted)
				{
					item->Effect[0].Index = EF_AMOUNT;
					item->Effect[0].Value = earnedItem.Amount;
				}

				item->Effect[1].Index = EF_NOTRADE;
				item->Effect[1].Value = 1;

				if (earnedItem.Splitted)
				{
					for (int i = 0; i < earnedItem.Amount - 1; i++)
					{
						int otherSlotId = GetFirstSlot(clientId, 0);
						if (otherSlotId == -1)
						{
							Log(clientId, LOG_INGAME, "Não encontrou slot para entrega do item");

							SendClientMessage(clientId, "Um problema aconteceu. Contate a administração");
							continue;
						}

						st_Item* otherItem = &Mob[clientId].Mobs.Player.Inventory[otherSlotId];

						*otherItem = st_Item{};
						otherItem->Index = earnedItem.ItemIndex;

						otherItem->Effect[0].Index = EF_NOTRADE;
						otherItem->Effect[0].Value = 1;

						Log(clientId, LOG_INGAME, "Recebido item %s %s no slot %d", ItemList[otherItem->Index].Name, otherItem->toString().c_str(), otherSlotId);
						SendItem(clientId, SlotType::Inv, otherSlotId, otherItem);
					}
				}
			}
			else
			{
				if (item->Index != 4542)
				{
					item->Effect[0].Index = EF_NOTRADE;
					item->Effect[0].Value = 1;
				}
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", ItemList[item->Index].Name, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region LIVRO AMALDIÇOADO CAVEIRA
		else if (itemId == 4623)
		{
			struct CursedBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<CursedBook> items =
			{
				{
					{ 4522, 3},
					{ 4140, 5},
					{ 3314, 3},
					{ 4627, 1},
					{ 4624, 1},
					{ 777, 5},
					{ 4028, 1},
					{ 4633, 1}
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->Effect[0].Index = EF_AMOUNT;
				item->Effect[0].Value = earnedItem.Amount;

				item->Effect[1].Index = EF_NOTRADE;
				item->Effect[1].Value = 1;
			}
			else
			{
				item->Effect[0].Index = EF_NOTRADE;
				item->Effect[0].Value = 1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", ItemList[item->Index].Name, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);

		}
#pragma endregion
#pragma region BAÚ DO SUKUR
		else if (itemId == 4647)
		{
			struct CursedBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<CursedBook> items =
			{
				{
					{ 2411, 20},
					{ 2412, 20},
					{ 2413, 20},
					{ 3182, 10},
					{ 3182, 10},
					{ 3326, 1},
					{ 4140, 10},
					{ 4140, 10},
					{ 671, 1},
					{ 4633, 1}
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->Effect[0].Index = EF_AMOUNT;
				item->Effect[0].Value = earnedItem.Amount;

				item->Effect[1].Index = EF_NOTRADE;
				item->Effect[1].Value = 1;
			}
			else
			{
				item->Effect[0].Index = EF_NOTRADE;
				item->Effect[0].Value = 1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", ItemList[item->Index].Name, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region LIVRO ANNUBIS
		else if (itemId == 4657)
		{
			struct AnnubisBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<AnnubisBook> items =
			{
				{
					{ 2411, 30 },
					{ 2412, 30 },
					{ 2413, 30 },
					{ 3326, 1 },
					{ 4633, 2 },
					{ 4140, 10 },
					{ 4140, 10 },
					{ 4854, 1 },
					{ 3210, 1 },
					{ 4665, 10 },
					{ 633, 1 }
				}
			};

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(items.begin(), items.end(), g);

			auto& earnedItem = items.front();
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->Effect[0].Index = EF_AMOUNT;
				item->Effect[0].Value = earnedItem.Amount;

				item->Effect[1].Index = EF_NOTRADE;
				item->Effect[1].Value = 1;
			}
			else
			{
				item->Effect[0].Index = EF_NOTRADE;
				item->Effect[0].Value = 1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", ItemList[item->Index].Name, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region LIVRO POSEIDON
		else if (itemId == 4667)
		{
			struct PoseidonBook
			{
				int ItemIndex;
				int Amount;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<PoseidonBook> commonItems =
			{
				{ 4633, 3 },
				{ 2411, 30 },
				{ 2412, 30 },
				{ 2413, 30 },
				{ 4672, 1 },
			};

			std::vector<PoseidonBook> unusualItems =
			{
				{2414, 15},
				{2415, 15},
				{2416, 15},
				{4665, 10},
			};

			std::vector<PoseidonBook> rareItems =
			{
				{4679, 1},
				{633, 1},
				{671, 1},
				{4673, 1},
			};

			std::vector<PoseidonBook> epicItems =
			{
				{4854, 1},
			};

			std::vector<PoseidonBook> sortedItems;
			int rand = Rand() % 100;
			if (rand <= 50)
				sortedItems = commonItems;
			else if (rand <= 80)
				sortedItems = unusualItems;
			else if (rand <= 95)
				sortedItems = rareItems;
			else
				sortedItems = epicItems;

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(sortedItems.begin(), sortedItems.end(), g);

			auto& earnedItem = sortedItems.front();
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->Effect[0].Index = EF_AMOUNT;
				item->Effect[0].Value = earnedItem.Amount;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", ItemList[item->Index].Name, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
	#pragma endregion
#pragma region LIVRO JUNINO
		else if (itemId == 4715)
		{
			struct JuninoBook
			{
				int ItemIndex;
				int Amount;
				int Ef1;
				int Efv1;
			};

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::vector<JuninoBook> commonItems =
			{
				{ 2414, 30 },
				{ 2415, 30 },
				{ 2416, 30 },
				{ 4708, 1 },
			};

			std::vector<JuninoBook> unusualItems =
			{
				{4679, 1 },
				{4851, 1 },
				{3338, 1 },
				{4665, 10 },
			};

			std::vector<JuninoBook> rareItems =
			{
				{633, 1, EF_HPADD2, 8},
				{633, 1, EF_MPADD2, 8},
				{633, 1, EF_CRITICAL, 40},
				{4709, 1},
			};

			std::vector<JuninoBook> epicItems =
			{
				{4854, 1},
			};

			std::vector<JuninoBook> sortedItems;
			int rand = Rand() % 100;
			if (rand <= 50)
				sortedItems = commonItems;
			else if (rand <= 80)
				sortedItems = unusualItems;
			else if (rand <= 95)
				sortedItems = rareItems;
			else
				sortedItems = epicItems;

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(sortedItems.begin(), sortedItems.end(), g);

			auto& earnedItem = sortedItems.front();
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = earnedItem.ItemIndex;

			if (earnedItem.Amount > 1)
			{
				item->Effect[0].Index = EF_AMOUNT;
				item->Effect[0].Value = earnedItem.Amount;
			}

			if (earnedItem.Ef1 != 0)
			{
				item->Effect[0].Index = earnedItem.Ef1;
				item->Effect[0].Value = earnedItem.Efv1;
			}

			SendItem(clientId, SlotType::Inv, slotId, item);
			Log(clientId, LOG_INGAME, "Recebido item %s %s", ItemList[item->Index].Name, item->toString().c_str());

			SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item->Index].Name);
			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region LIVRO DO BONE DEMON
        else if (itemId == 4724)
        {
            struct BoneBook
            {
                int ItemIndex;
                int Amount;
                int Ef1;
                int Efv1;
            };

            int slotId = -1;

            bool remove = true;
            if (GetItemAmount(srcItem) == 1)
            {
                slotId = p->SrcSlot;
                remove = false;
            }
            else
                slotId = GetFirstSlot(clientId, 0);

            if (slotId == -1)
            {
                SendClientMessage(clientId, "Sem espaço no inventário");

                Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
                SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
                return true;
            }

            if (GetInventoryAmount(clientId, 0) < 2)
            {
                SendClientMessage(clientId, "Você precisa de no mínimo dois slots vagos no inventário");

                Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
                SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
                return true;
            }

            std::vector<BoneBook> commonItems =
            {
                { 2414, 30 },
                { 2415, 30 },
                { 2416, 30 },
                { 1731, 1 },
                { 4727, 1 },
            };

            std::vector<BoneBook> unusualItems =
            {
                {3338, 1, 43, 1 },
                {4665, 10 },
                {4864, 1, 43, 1},
                {4679, 1 },
                {3182, 15}
            };

            std::vector<BoneBook> rareItems =
            {
                {633, 1, EF_HPADD2, 8},
                {633, 1, EF_MPADD2, 8},
                {633, 1, EF_CRITICAL, 40},
                {4728, 1},
            };

            std::vector<BoneBook> epicItems =
            {
                {3338, 1, 43, 9},
                {3338, 2, 43, 4},
            };

            std::vector<BoneBook> sortedItems;
            int rand = Rand() % 100;
            if (rand <= 50)
                sortedItems = commonItems;
            else if (rand <= 80)
                sortedItems = unusualItems;
            else if (rand <= 95)
                sortedItems = rareItems;
            else
                sortedItems = epicItems;

            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(sortedItems.begin(), sortedItems.end(), g);

            auto& earnedItem = sortedItems.front();
            st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

            *item = st_Item{};
            item->Index = earnedItem.ItemIndex;

            if (earnedItem.ItemIndex != 3338)
            {
                if (earnedItem.Amount > 1)
                {
                    item->Effect[0].Index = EF_AMOUNT;
                    item->Effect[0].Value = earnedItem.Amount;
                }

                if (earnedItem.Ef1 != 0)
                {
                    item->Effect[0].Index = earnedItem.Ef1;
                    item->Effect[0].Value = earnedItem.Efv1;
                }
            }
            else
            {
                for (int i = 0; i < earnedItem.Amount; ++i)
                {
                    if (i == 0)
                    {
                        item->Effect[0].Index = earnedItem.Ef1;
                        item->Effect[0].Value = earnedItem.Efv1;
                    }
                    else
                    {
                        int otherSlotId = GetFirstSlot(clientId, 0);
                        auto otherItem = &Mob[clientId].Mobs.Player.Inventory[otherSlotId];

                        *otherItem = st_Item{};
                        otherItem->Index = 3338;
                        otherItem->Effect[0].Index = earnedItem.Ef1;
                        otherItem->Effect[0].Value = earnedItem.Efv1;

                        SendItem(clientId, SlotType::Inv, otherSlotId, otherItem);
                    }
                }
            }
            SendItem(clientId, SlotType::Inv, slotId, item);
            Log(clientId, LOG_INGAME, "Recebido item %s %s", ItemList[item->Index].Name, item->toString().c_str());

            SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item->Index].Name);
            if (remove)
                AmountMinus(srcItem);
        }
#pragma endregion
#pragma region CAIXA MYTRIL
		else if (itemId == 4633)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = 3027 + (Rand() % 4);

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", ItemList[item->Index].Name, item->toString().c_str());
			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region CAIXA DE ARMAS D
		else if (itemId == 4631)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::array damageWeapons = { 824, 825, 839, 840, 870, 884, 885, 911, 809, 810 };
			std::array magicWeapons = { 854, 855, 902, 900 };

			int type = Rand() % 50;
			int addId = 0;
			int addValue = 0;
			int itemId = 0;

			if (type <= 25)
			{
				std::array damageValue = { 54, 54, 54, 54, 63, 63, 63, 72 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(damageValue.begin(), damageValue.end(), g);
				addId = EF_DAMAGE;
				addValue = damageValue.front();

				std::shuffle(damageWeapons.begin(), damageWeapons.end(), g);
				itemId = damageWeapons.front();
			}
			else
			{
				std::array magicValue = { 24, 24, 24, 24, 28, 28, 28, 32 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(magicValue.begin(), magicValue.end(), g);

				addId = EF_MAGIC;
				addValue = magicValue.front();

				std::shuffle(magicWeapons.begin(), magicWeapons.end(), g);
				itemId = magicWeapons.front();
			}

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = itemId;
			item->Effect[0].Index = EF_SANC;
			item->Effect[0].Value = 0;
			item->Effect[1].Index = addId;
			item->Effect[1].Value = addValue;

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", ItemList[item->Index].Name, item->toString().c_str());
			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region CAIXA DE ARMAS E
		else if (itemId == 4632)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			std::array damageWeapons = { 3551, 3556, 3561, 3571, 3576, 3591, 3596 };
			std::array magicWeapons = { 3566, 3581, 3582 };

			int type = Rand() % 50;
			int addId = 0;
			int addValue = 0;
			int itemId = 0;

			if (type <= 25)
			{
				std::array damageValue = { 54, 54, 54, 54, 63, 63, 63, 72, 72, 72 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(damageValue.begin(), damageValue.end(), g);
				addId = EF_DAMAGE;
				addValue = damageValue.front();

				std::shuffle(damageWeapons.begin(), damageWeapons.end(), g);
				itemId = damageWeapons.front();
			}
			else
			{
				std::array magicValue = { 24, 24, 24, 24, 28, 28, 28, 32, 32, 32 };

				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(magicValue.begin(), magicValue.end(), g);

				addId = EF_MAGIC;
				addValue = magicValue.front();

				std::shuffle(magicWeapons.begin(), magicWeapons.end(), g);
				itemId = magicWeapons.front();
			}

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];

			*item = st_Item{};
			item->Index = itemId;
			item->Effect[0].Index = EF_SANC;
			item->Effect[0].Value = 0;
			item->Effect[1].Index = addId;
			item->Effect[1].Value = addValue;

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", ItemList[item->Index].Name, item->toString().c_str());
			if (remove)
				AmountMinus(srcItem);

			return true;
		}
#pragma endregion
#pragma region CAIXA AMULETO ARCANO
		else if (itemId == 4634)
		{
			int arcaneId = 567 + (Rand() % 4);

			st_Item newItem{};
			newItem.Index = 567 + Rand() % 4;
			newItem.Effect[0].Index = EF_SANC;
			newItem.Effect[0].Value = 0;

			int type = Rand() % 3;
			if (type == 0)
			{
				newItem.Effect[1].Index = EF_HPADD2;
				newItem.Effect[1].Value = 10;
				newItem.Effect[2].Index = EF_MAGIC;
				newItem.Effect[2].Value = 16;
			}
			else if(type == 1)
			{
				newItem.Effect[1].Index = EF_MPADD2;
				newItem.Effect[1].Value = 10;
				newItem.Effect[2].Index = EF_MAGIC;
				newItem.Effect[2].Value = 16;
			}
			else 
			{
				newItem.Effect[1].Index = EF_HPADD2;
				newItem.Effect[1].Value = 10;
				newItem.Effect[2].Index = EF_DAMAGE;
				newItem.Effect[2].Value = 40;
			}

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
			*item = newItem;

			SendItem(clientId, SlotType::Inv, slotId, item);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", ItemList[item->Index].Name, item->toString().c_str());

			if (remove)
				AmountMinus(srcItem);
		}
#pragma endregion
#pragma region REPLATION (GRIUPAN IMP)
		else if (itemId == 4686 || itemId == 4687)
		{
			if (p->DstType != (int)SlotType::Inv)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (dstItem == nullptr || dstItem->Index == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if ((itemId == 4686 && dstItem->Index != 753) || (itemId == 4687 && dstItem->Index != 1726))
			{
				if(itemId == 4686)
					SendClientMessage(clientId, "Somente utilizado em auxiliares Imp");
				else
					SendClientMessage(clientId, "Somente utilizado em auxiliares Griupan");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Utilizado no item %s %s", ItemList[dstItem->Index].Name, dstItem->toString().c_str());

			for (int i = 0; i < 1; i++)
			{
				if (dstItem->Effect[i].Index == EF_SANC)
					continue;

				dstItem->Effect[i].Index = 0;
				dstItem->Effect[i].Value = 0;
			}

			int addEf = 0;
			int addEfv = 0;
			if (itemId == 4686)
			{
				int rand = Rand() % 100;
				if (rand < 50)
				{
					addEf = EF_DAMAGE;
					addEfv = 27 + ((Rand() % 7) * 3);
				}
				else
				{
					addEf = EF_MAGIC;
					addEfv = 5 + (Rand() % 6);
				}
			}
			else
			{
				int type = Rand() % 4;
				if (type == 0)
				{
					addEf = EF_AC;
					addEfv = 70 + ((Rand() % 4) * 10);
				}
				else if (type == 1)
				{
					addEf = EF_RESISTALL;
					if (Rand() % 11 <= 5)
						addEfv = 8;
					else
						addEfv = 10;
				}
				else if (type == 2 || type == 3)
				{
					if (type == 2)
						addEf = EF_HPADD2;
					else if (type == 3)
						addEf = EF_MPADD2;

					addEfv = 4 + ((Rand() % 3) * 2);
				}
			}

			int oldItemId = dstItem->Index;
			int sanc = GetItemSanc(dstItem);

			*dstItem = st_Item{};
			dstItem->Index = oldItemId;
			dstItem->Effect[0].Index = EF_SANC;
			dstItem->Effect[0].Value = sanc;
			dstItem->Effect[1].Index = addEf;
			dstItem->Effect[1].Value = addEfv;
			dstItem->Effect[2].Index = EF_NOTRADE;
			dstItem->Effect[2].Value = 1;

			Log(clientId, LOG_INGAME, "Gerado o adicional: %s", dstItem->toString().c_str());
			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region REPLATION (BRINCO)
		else if (itemId == 4610)
		{
			if (p->DstType != (int)SlotType::Inv)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Only_To_Equips]);

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* dstItem = GetItemPointer(clientId, p->DstType, p->DstSlot);
			if (dstItem == nullptr || dstItem->Index == 0)
			{
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);

				return true;
			}

			if (dstItem->Index < 590 || dstItem->Index > 595)
			{
				SendClientMessage(clientId, "Somente utilizado em Brincos");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (GetItemSanc(dstItem) > 9)
			{
				SendClientMessage(clientId, "Somente utilizado em Brincos igual ou inferior a +9");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Utilizado no item %s %s", ItemList[dstItem->Index].Name, dstItem->toString().c_str());

			for (int i = 0; i < 1; i++)
			{
				if (dstItem->Effect[i].Index == EF_SANC)
					continue;

				dstItem->Effect[i].Index = 0;
				dstItem->Effect[i].Value = 0;
			}

			int effectIndex = 0;
			int effectValue = 0;
			int rand = Rand() % 100;
			if (rand <= 25)
			{
				effectIndex = EF_HPADD2;
				effectValue = Rand() % 4 + 3;
			}
			else if (rand <= 50)
			{
				effectIndex = EF_MPADD2;
				effectValue = Rand() % 4 + 3;
			}
			else if (rand <= 75)
			{
				effectIndex = EF_DAMAGE;
				effectValue = Rand() % 26 + 15;
			}
			else
			{
				effectIndex = EF_MAGIC;
				effectValue = Rand() % 7 + 4;
			}

			for (int i = 1; i < 3; i++)
			{
				if (dstItem->Effect[i].Index == EF_SANC)
					continue;

				dstItem->Effect[i].Index = effectIndex;
				dstItem->Effect[i].Value = effectValue;
				break;
			}

			Log(clientId, LOG_INGAME, "Gerado o adicional: %s", dstItem->toString().c_str());
			SendItem(clientId, (SlotType)p->DstType, p->DstSlot, dstItem);

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region OVO DA SORTE
		else if (itemId == 4615 || itemId == 4642)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			st_Item item{};

			if (itemId == 4615)
				item.Index = 2360 + (Rand() % 6);
			else
			{
				constexpr std::array mountId = { 2366, 2367, 2368, 2371, 2372, 2373 };
				item.Index = *select_randomly(std::begin(mountId), std::end(mountId));
			}

			*(WORD*)&item.Effect[0].Index = 5000;
			item.Effect[1].Index = 120;
			item.Effect[1].Value = 10;
			item.Effect[2].Index = 100;

			Mob[clientId].Mobs.Player.Inventory[slotId] = item;
			SendItem(clientId, SlotType::Inv, slotId, &item);

			if (remove)
				AmountMinus(srcItem);

			Log(clientId, LOG_INGAME, "Recebido o item %s %s", ItemList[item.Index].Name, item.toString().c_str());
			SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item.Index].Name);
		}
#pragma endregion
#pragma region PASSE DE BATALHA
		else if (itemId == 4616 || itemId == 4617 || itemId == 4618 || itemId == 4621)
		{
			int index = itemId - 4616;
			int days = 3;
			switch (index)
			{
			case 1:
				days = 7;
				break;
			case 2:
				days = 15;
				break;
			case 5:
				days = 1;
				break;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(Mob[clientId].Mobs.DailyQuest.BattlePass.GetTMStruct(), now_time_t);

			if (diffTime > 0.0)
			{
				SendClientMessage(clientId, "Você já possui um Passe de Batalha ativo. Espere o mesmo acabar para ativar outro");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int month = now.tm_mon + 1; //0 Janeiro, 1 Fev
			int day = now.tm_mday + days;
			int year = now.tm_year + 1900;
			int hour = now.tm_hour;
			int min = now.tm_min;
			int sec = now.tm_sec;

			if (day > dias_mes[month])
			{
				day -= dias_mes[month];
				month += 1;
			}

			if (month > 12)
			{
				month -= 12;
				year += 1;
			}

			Mob[clientId].Mobs.DailyQuest.BattlePass.Dia = day;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Mes = month;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Ano = year;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Hora = hour;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Minuto = min;
			Mob[clientId].Mobs.DailyQuest.BattlePass.Segundo = sec;

			SendClientMessage(clientId, "Seu Passe de Batalha foi ativado até %02d/%02d/%02d às %02d:%02d:%02d", day, month, year, hour, min, sec);
			Log(clientId, LOG_INGAME, "Seu Passe de Batalha foi ativado até %02d/%02d/%02d às %02d:%02d:%02d", day, month, year, hour, min, sec);

			AmountMinus(srcItem);
			SendMissionInfo(clientId);
			return true;
		}
#pragma endregion
#pragma region SELO DAS ALMAS
		else if (itemId == 3443)
		{
			if (srcItem->Effect[0].Index != 0)
				return true;

			for (int i = 1; i < 15; i++)
			{
				if (Mob[clientId].Mobs.Player.Equip[i].Index > 0)
				{
					SendClientMessage(clientId, "Desequipe todos os itens do seu personagem para selá-lo");

					Log(clientId, LOG_INGAME, "Não é possível selar enquanto possuir itens equipados");
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			for (int i = 0; i < 60; i++)
			{
				if (i == p->SrcSlot)
					continue;

				if (Mob[clientId].Mobs.Player.Inventory[i].Index > 0)
				{
					SendClientMessage(clientId, "Deixe o inventário vazio para selar seu personagem");

					Log(clientId, LOG_INGAME, "Não é possível selar enquanto possuir itens no inventário");
					SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
					return true;
				}
			}

			bool haveStorageSlot = false;
			for (int i = 0; i < 120; i++)
			{
				if (User.Storage.Item[i].Index == 0)
					haveStorageSlot = true;
			}

			if (!haveStorageSlot)
			{
				SendClientMessage(clientId, "Necessário ter um slot vago no seu Guarda Carga");

				Log(clientId, LOG_INGAME, "Sem slot vago na seleção de personagem");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.GuildMemberType != 0 && Mob[clientId].Mobs.Player.GuildMemberType != 1)
			{
				SendClientMessage(clientId, "Não é possível selar personagens líderes/sublíderes de guild");

				Log(clientId, LOG_INGAME, "Não é possível selar personagens líderes/sublíderes de guild");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.Gold != 0)
			{
				SendClientMessage(clientId, "Não é possível selar personagem com gold no inventário");

				Log(clientId, LOG_INGAME, "Não é possível selar personagem com gold no inventário");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL)
			{
				SendClientMessage(clientId, "Não é possível selar personagem mortal");

				Log(clientId, LOG_INGAME, "Não é possível selar personagem mortal");
				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int capeIndex = Mob[clientId].Mobs.Player.CapeInfo - CAPE_BLUE;
			if (capeIndex == 0 || capeIndex == 1)
			{
				for (INT32 i = 0; i < 10; i++)
				{
					if (Mob[clientId].Mobs.Player.Equip[15].Index == g_pCapesID[capeIndex][i])
					{
						Log(clientId, LOG_INGAME, "Alterado a capa de %hu para %hu", Mob[clientId].Mobs.Player.Equip[15].Index, g_pCapesID[2][i]);
						Mob[clientId].Mobs.Player.Equip[15].Index = g_pCapesID[2][i];

						break;
					}
				}

				if ((Mob[clientId].Mobs.Player.Equip[0].EFV2 == CELESTIAL && Mob[clientId].Mobs.Sub.Status == 1) || (Mob[clientId].Mobs.Player.Equip[0].EFV2 == SUBCELESTIAL))
				{
					for (int i = 0; i < 10; i++)
					{
						if (Mob[clientId].Mobs.Sub.Equip[1].Index == g_pCapesID[capeIndex][i])
						{
							Log(clientId, LOG_INGAME, "Alterado a capa do Sub/cele %hu para %hu", Mob[clientId].Mobs.Sub.Equip[1].Index, g_pCapesID[2][i]);
							Mob[clientId].Mobs.Sub.Equip[1].Index = g_pCapesID[2][i];

							break;
						}
					}

					SetItemSanc(&Mob[clientId].Mobs.Sub.Equip[1], 0, 0);
				}
			}

			SetItemSanc(&Mob[clientId].Mobs.Player.Equip[15], 0, 0);
			AmountMinus(srcItem);

			CharLogOut(clientId);

			pMsgSignal packet{};
			packet.Header.PacketId = PutInSealPacket;
			packet.Header.Size = sizeof packet;
			packet.Header.ClientId = clientId;

			packet.Value = inGame.CharSlot;

			AddMessageDB(reinterpret_cast<BYTE*>(&packet), sizeof packet);

			return true;
		}
#pragma endregion
#pragma region COMPACTADOR DE GOLD
		else if (itemId == 4572)
		{
			auto gold = Mob[clientId].Mobs.Player.Gold;
			if (gold < 1000000000)
			{
				SendClientMessage(clientId, "O gold mínimo para compactar é de 1 bilhão");

				SendItem(clientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
				return true;
			}

			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Log(clientId, LOG_INGAME, "Gerado Barra de 1bi. Gold atual do inventário: %d", Mob[clientId].Mobs.Player.Gold);

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
			*item = st_Item{};
			item->Index = 4011;

			Mob[clientId].Mobs.Player.Gold -= 1000000000;
			SendSignalParm(clientId, clientId, 0x3AF, Mob[clientId].Mobs.Player.Gold);

			if (remove)
				AmountMinus(srcItem);

			SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[4011].Name);
			SendItem(clientId, SlotType::Inv, slotId, item);
		}
#pragma endregion
#pragma region BAÚ DE RUNAS
		else if (itemId == 4665)
		{
		int slotId = -1;

		bool remove = true;
		if (GetItemAmount(srcItem) == 1)
		{
			slotId = p->SrcSlot;
			remove = false;
		}
		else
			slotId = GetFirstSlot(clientId, 0);

		if (slotId == -1)
		{
			SendClientMessage(clientId, "Sem espaço no inventário");

			Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
			return true;
		}


		st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
		*item = st_Item{};
		item->Index = 5110 + (Rand() % 24);

		SendItem(clientId, SlotType::Inv, slotId, item);

		if (remove)
			AmountMinus(srcItem);

		SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item->Index].Name);
		Log(clientId, LOG_INGAME, "Recebeu o item [%s]", ItemList[item->Index].Name);
		}
#pragma endregion
#pragma region CAIXA DE SECRETAS
		else if (itemId == 4676)
		{
			int slotId = -1;

			bool remove = true;
			if (GetItemAmount(srcItem) == 1)
			{
				slotId = p->SrcSlot;
				remove = false;
			}
			else
				slotId = GetFirstSlot(clientId, 0);

			if (slotId == -1)
			{
				SendClientMessage(clientId, "Sem espaço no inventário");

				Log(clientId, LOG_INGAME, "Sem espaço no inventário para recebimento");
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[slotId];
			*item = st_Item{};
			item->Index = 5334 + (Rand() % 4);

			SendItem(clientId, SlotType::Inv, slotId, item);

			if (remove)
				AmountMinus(srcItem);

			SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[item->Index].Name);
			Log(clientId, LOG_INGAME, "Recebeu o item [%s]", ItemList[item->Index].Name);
		}
#pragma endregion
#pragma region JOIA DA ALMA
		else if (itemId == 3214)
		{
			if (!(Mob[clientId].Mobs.Player.Learn[0] & 0x40000000))
			{
				SendClientMessage(clientId, "Você não possui a habilidade Limite de Alma");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			SendSignal(clientId, SERVER_SIDE, ResetSoulDelayPacket);

			Log(clientId, LOG_INGAME, "Limite da Alma resetada com sucesso");
			TimeStamp.Skills[102] = std::chrono::steady_clock::time_point();

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region ÂMAGO ABENÇOADO
		else if (itemId >= 3230 && itemId <= 3244)
		{
			st_Item* dstItem = GetItemPointer(clientId, (int)SlotType::Equip, 14);
			if (dstItem->Index < 2330 || dstItem->Index > 2389)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int level = dstItem->Effect[1].Index;
			if (level < 120)
			{
				SendClientMessage(clientId, "O nível mínimo é 120");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (level >= 130)
			{
				SendClientMessage(clientId, "O nível máximo é 130");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int amagoId = -1;
			switch (dstItem->Index)
			{
			case 2370:
				amagoId = 3230;
				break;
			case 2375:
				amagoId = 3231;
				break;
			case 2376:
				amagoId = 3232;
				break;
			case 2377:
				amagoId = 3233;
				break;
			case 2379:
				amagoId = 3234;
				break;
			case 2380:
				amagoId = 3235;
				break;
			case 2381:
			case 2382:
			case 2383:
				amagoId = 3236 + (dstItem->Index - 2381);
				break;
			case 2384:
			case 2385:
			case 2386:
				amagoId = 3239 + (dstItem->Index - 2384);
				break;
			case 2388:
				amagoId = 3243;
				break;
			case 2387:
				amagoId = 3242;
				break;	
			}

			if (srcItem->Index != amagoId)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Mount_Not_Match]);

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int rand = Rand() % 100;
			int rate = 50;
			rate = rate - ((level - 120) * 2);

			if (rand <= rate)
			{
				dstItem->Effect[1].Index++;
				SendItem(clientId, SlotType::Equip, 14, dstItem);

				SendClientMessage(clientId, "Sucesso na refinação (%d/%d)", rand, rate);

				Log(clientId, LOG_INGAME, "Sucesso no uso do %s. Chance: %d/%d", ItemList[amagoId].Name, rand, rate);
			}
			else
			{
				SendClientMessage(clientId, "Falha na refinação (%d/%d)", rand, rate);

				Log(clientId, LOG_INGAME, "Falhou no uso do %s. Chance: %d/%d", ItemList[amagoId].Name, rand, rate);
			}

			AmountMinus(srcItem);
		}
#pragma endregion
#pragma region FUMAÇA CÓSMICA
		else if (itemId == 792)
		{
			auto dstItem = GetItemPointer(clientId, (int)SlotType::Equip, 11);
			if (dstItem == nullptr)
			{
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (dstItem->Index < 762 || dstItem->Index > 765)
			{
				SendClientMessage(clientId, "Equipe um planeta no slot adequado");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (GetItemSanc(dstItem) != 11)
			{
				SendClientMessage(clientId, "O planeta precisa estar +11 para composição");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			auto mob = &Mob[clientId].Mobs;
			if (mob->Fame < 500)
			{
				SendClientMessage(clientId, "Você precisa de 500 de fama para composição");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int totalSealed = GetInventoryAmount(clientId, 4127);
			if (totalSealed < 10)
			{
				SendClientMessage(clientId, "Você precisa de 10 Pergaminho Selado");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			for (int i = 0; i < 10; ++i)
			{
				int slotId = GetFirstSlot(clientId, 4127);
				if (slotId != -1)
				{
					Log(clientId, LOG_INGAME, "Removido item %s do slot %d", ItemList[mob->Player.Inventory[slotId].Index].Name, slotId);
					mob->Player.Inventory[slotId] = st_Item{};

					SendItem(clientId, SlotType::Inv, slotId, &mob->Player.Inventory[slotId]);
				}
			}

			mob->Fame -= 500;
			AmountMinus(srcItem);

			dstItem->Index += 26;

			SetItemSanc(dstItem, 0, 0);
			SendItem(clientId, SlotType::Equip, 11, dstItem);
		}
#pragma endregion
#pragma region ACELERADOR DE ESCRITURA
		else if (itemId == 4682)
		{
			if (Mob[clientId].Mobs.Player.Equip[0].EFV2 <= ARCH)
			{
				SendClientMessage(clientId, "Disponível apenas para celestiais ou superior");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			time_t rawnow = time(NULL);
			struct tm now; localtime_s(&now, &rawnow);

			auto now_time_t = std::mktime(&now);
			auto diffTime = std::difftime(now_time_t, Mob[clientId].Mobs.Escritura.GetTMStruct());
			if (Mob[clientId].Mobs.Escritura.Ano == 0)
				diffTime = ScrollWaitTime;

			if (diffTime >= ScrollWaitTime)
			{
				SendClientMessage(clientId, "Você não está com tempo de espera para usar um próximo Escritura de Pesadelo");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			int month = Mob[clientId].Mobs.Escritura.Mes;
			int day = Mob[clientId].Mobs.Escritura.Dia;
			int year = Mob[clientId].Mobs.Escritura.Ano;
			int hour = Mob[clientId].Mobs.Escritura.Hora - 10;
			int min = Mob[clientId].Mobs.Escritura.Minuto;
			int sec = Mob[clientId].Mobs.Escritura.Segundo;

			if (hour < 0)
			{
				hour += 24;
				day--;
			}

			if (day <= 0)
			{
				month--;
				day = dias_mes[month];
			}

			if (month <= 0)
			{
				month = 12;
				dias_mes[month];
				year--;
			}

			Mob[clientId].Mobs.Escritura.Ano = year;
			Mob[clientId].Mobs.Escritura.Mes = month;
			Mob[clientId].Mobs.Escritura.Dia = day;
			Mob[clientId].Mobs.Escritura.Hora = hour;
			Mob[clientId].Mobs.Escritura.Minuto = min;
			Mob[clientId].Mobs.Escritura.Segundo = sec;

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region MILHO
		else if (itemId == 4704)
		{
			auto harvest = TOD_EventManager::GetInstance().GetEventItem(TOD_EventType::HappyHarvest);
			if (!harvest->Register(*this, srcItem))
			{
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

				return true;
			}

			AmountMinus(srcItem);
			return true;
		}
#pragma endregion
#pragma region PÓ DE BRINCO
		else if (itemId == 4719)
		{
			int amount = GetItemAmount(srcItem);
			if (amount != 120)
			{
				SendClientMessage(clientId, "Necessário 120 unidades de Pó de Brinco");
			
				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			if (Mob[clientId].Mobs.Player.Gold < 100000000)
			{
				SendClientMessage(clientId, "Necessário 100 milhões de gold");

				SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);
				return true;
			}

			Mob[clientId].Mobs.Player.Gold -= 100000000;
			SendSignalParm(clientId, clientId, 0x3AF, Mob[clientId].Mobs.Player.Gold);

			*srcItem = st_Item{};
			srcItem->Index = 4611;

			SendItem(clientId, SlotType::Inv, p->SrcSlot, srcItem);

			SendClientMessage(clientId, "!Chegou um item: [%s]", ItemList[4611].Name);
			Log(clientId, LOG_INGAME, "Recebeu Emblema da Proteção (Brinco)");
			return true;
		}
#pragma endregion
	}
	Mob[clientId].GetCurrentScore(clientId);
	SendScore(clientId);
	SendEquip(clientId);
	return true;
}