#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

int GetItemType(int ItemID)
{
	int unique = ItemList[ItemID].Pos;
	if(unique == 2) 
		unique = 3021;
	else if(unique == 4) 
		unique = 3022;
	else if(unique == 8 ) 
		unique = 3023;
	else if(unique == 16) 
		unique = 3024;
	else if(unique == 32) 
		unique = 3025;
	else if(unique == 64 || unique == 192) 
		unique = 3026;

	return unique;
}

bool CUser::RequestOdin(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	st_Mob *player = &Mob[clientId].Mobs.Player;

	for(int i = 0;i < 7; i++)
	{
		if(p->slot[i] < 0 || p->slot[i] >= 60)
		{			
			Log(clientId, LOG_HACK, "[HACK] Banido por enviar índice inválido - NPC Lindy - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "[HACK] %s - Banido por enviar índice inválido - NPC Lindy - %d", player->Name, p->slot[i]);
			
			SendCarry(clientId);

			return true;
		}

		if(memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Lindy - %d", p->items[i].Index);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Lindy - %d", player->Name, p->items[i].Index);
			
			SendCarry(clientId);

			return true;
		}
		
		for(int y = 0; y < 7; y++)
		{
			if(y == i)
				continue;

			if(p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Lindy - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Lindy- %d", player->Name, p->items[i].Index);

				CloseUser(clientId);
				return true;
			}
		}
	}
	
	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	for(int i = 0 ; i < 7; i ++)
	{
		if (p->slot[i] == -1)
		{
			Log(clientId, LOG_COMP, "Alq. Odin - %d - Sem item", i);

			continue;
		}

		Log(clientId, LOG_COMP, "Alq. Odin - %d - %s %s - %hhd", i, ItemList[p->items[i].Index].Name, p->items[i].toString().c_str(), p->slot[i]);
	}

	// Fecha o trade caso esteja aberto
	if(Trade.ClientId != 0)
	{
		RemoveTrade(Trade.ClientId);
		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
	}
	
	// Fecha o inventário
	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
	
	if((p->items[0].Index == 4043 && p->items[1].Index == 4043) || (p->items[0].Index == 413 && GetItemAmount(&p->items[0]) == 10 && p->items[1].Index == 413 && GetItemAmount(&p->items[1]) == 10))
	{
		INT32 sanc = GetItemSanc(&p->items[2]);
		if(sanc >= 12) //Limite de refinação = 13
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_More]);

			return true;
		}
		
		if(sanc <= 10) 
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Minimum_AlqOdin_Ref_Is_10]);

			return true;
		}
		
		int mobType = GetEffectValueByIndex(p->items[2].Index, EF_MOBTYPE);
		if(mobType == 3)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_With_Me]);

			return true;
		}
		
		int goldRequired = 1000000000;
		if(Mob[clientId].Mobs.Player.Gold < goldRequired)
		{
			SendClientMessage(clientId, "Gold insuficiente");

			return true;
		}

		bool canBreak = true;
		bool extracao = false;
		int rate = 0;

		if (sanc == 11)
			rate = 4;
		else if (sanc == 12)
			rate = 3;
		else if (sanc == 13)
			rate = 2;
		else
			rate = 1;

		if (p->items[0].Index == 4043 && p->items[1].Index == 4043)
		{
			extracao = true;

			rate += 2;
		}

		if(p->items[2].Index >= 3500 && p->items[2].Index <= 3507)
		{
			extracao = false;
			canBreak = false;
		}

		if(ItemList[p->items[2].Index].Pos == 128)
		{
			extracao = false;
			canBreak = false;
		}

		bool secrets{ false };
		for(int i = 0; i < 4; i++)
		{
			if(p->items[3 + i].Index >= 5334 && p->items[3 + i].Index <= 5337)
			{
				rate += 1;

				secrets = true;
				continue;
			}

			if (p->items[3 + i].Index != 3338)
				continue;

			if (secrets)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

				return true;
			}

			rate += ReturnChance(&p->items[3 + i]);

			int tmpSanc = GetItemSanc(&p->items[3 + i]);
			if(tmpSanc != 0)
			{
				canBreak = false;
				extracao = false;
			}
		}
		
		int pos = ItemList[p->items[2].Index].Pos;
		if(pos > 192)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CantRefine]);

			return true;
		}

		if(rate > 100) //padrão : 65
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Bad_Network_Packets]);

			return true;
		}
		
		if(rate < 0)
			rate = 4;

		Mob[clientId].Mobs.Player.Gold -= goldRequired;
		SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

		st_Item *item = player->Inventory;
		int _rand = Rand() % 100;
		if(_rand <= rate)
		{ // Sucesso na composição
			
			for(int i = 0 ; i < 3; i++)
			{
				if(item[p->slot[2]].Effect[i].Index == 43 || (item[p->slot[2]].Effect[i].Index >= 116 && item[p->slot[2]].Effect[i].Index <= 125))
				{
					item[p->slot[2]].Effect[i].Value += 4;
					break;
				}
			}
			
			char tmp[128];
			sprintf_s(tmp, g_pLanguageString[_NN_Odin_CombineSucceed], player->Name, sanc + 1);
			SendServerNotice(tmp);	

			Log(clientId, LOG_COMP, "Alquimista Odin - Refinado com sucesso %s para %d (%d/%d)", ItemList[item[p->slot[2]].Index].Name, sanc + 1, _rand, rate);
			LogPlayer(clientId, "Refinado com sucesso %s para +%d", ItemList[item[p->slot[2]].Index].Name, sanc + 1);
		}
		else
		{
			SendServerNotice("%s falhou a refinação de %s para %d", player->Name, ItemList[item[p->slot[2]].Index].Name, sanc + 1);

			if(canBreak && !(_rand % 5)) 
			{ // Se pode quebrar, vemos a chance para tal acontecer
				if(extracao)
				{
					int value = 0;
					for(int i = 0; i < 3; i++)
						{
						if(item[p->slot[2]].Effect[i].Index == 43 || (item[p->slot[2]].Effect[i].Index >= 116 && item[p->slot[2]].Effect[i].Index <= 125))
						{
							value = GetEffectValueByIndex(item[p->slot[2]].Index, EF_UNKNOW1);
							int mobtype = GetEffectValueByIndex(item[p->slot[2]].Index, EF_MOBTYPE);

							if(value <= 5 && mobtype == 0)
							{ // Itens <= [E] e é item mortal
								value = value;
							}
							else
							{
								if(value == 6)
								{
									if(sanc <= 9 && mobtype == 1)
										value = 10;
									else if(mobtype == 1 && sanc >= 10) // Item arch e +9 ou superior
										value = 11;
									else
										value = 6 ; // Item apenas anct
								}
								else if(mobtype == 1) // Item arch não anct
								{
									if(sanc >= 10)
										value = 9;
									else
										value = 8;
								}
								else
									NULL;
							}

							// Cálculo realizado - Item entregue
							item[p->slot[2]].Effect[i].Index = 87;
							item[p->slot[2]].Effect[i].Value = value;
							break;
						}
					}

					for(int i = 0; i < 3; i++)
					{
						if(item[p->slot[2]].Effect[i].Index == 43 || (item[p->slot[2]].Effect[i].Index >= 116 && item[p->slot[2]].Effect[i].Index <= 125))
							continue;

						if(item[p->slot[2]].Effect[i].Index == 0)
							continue;

						if(item[p->slot[2]].Effect[i].Index == 87)
							continue;

						if(ItemList[item[p->slot[2]].Index].Pos > 32)
							continue;

						int value = GetEffectValueByIndex(item[p->slot[2]].Index, item[p->slot[2]].Effect[i].Index);

						item[p->slot[2]].Effect[i].Value += value;
					}

					item[p->slot[2]].Index = GetItemType(item[p->slot[2]].Index);	

					Log(clientId, LOG_COMP, "Alquimista Odin - Extração criada. Tipo: %d", value);
					LogPlayer(clientId, "Extração criada no Alquimista Odin com a falha na composição de %s para +%d", ItemList[item[p->slot[2]].Index].Name, sanc + 1);
				}
				else
				{
					memset(&item[p->slot[2]], 0, sizeof st_Item);

					Log(clientId, LOG_COMP, "Alquimista Odin - Item quebrado, malz fera. %d/%d", _rand, rate);
				}
			}
			else
			{ // Falhou apenas, refinação volta
				for(int i = 0 ; i < 3; i++)
				{
					if(item[p->slot[2]].Effect[i].Index == 43 || (item[p->slot[2]].Effect[i].Index >= 116 && item[p->slot[2]].Effect[i].Index <= 125))
					{
						item[p->slot[2]].Effect[i].Value -= 4;

						break;
					}
				}
				LogPlayer(clientId, "Falha na composição de %s para +%d", ItemList[item[p->slot[2]].Index].Name, sanc +1);
				Log(clientId, LOG_COMP, "Alquimista Odin - Refinado com falha %s para %d. %d/%d", ItemList[item[p->slot[2]].Index].Name, sanc + 1, _rand, rate);
			}
		}
			
		for(int i = 0; i < 7; i++) 
		{
			if(i == 2)
			{
				SendItem(clientId, SlotType::Inv, p->slot[i], &item[p->slot[i]]);

				continue;
			}

			memset(&item[p->slot[i]], 0, sizeof st_Item);	
			SendItem(clientId, SlotType::Inv, p->slot[i], &item[p->slot[i]]);
		}

		SaveUser(clientId, 0);
	}
	else if(p->items[0].Index == 4127 && p->items[1].Index == 4127 && p->items[2].Index == 5135 && p->items[3].Index == 5113 &&
		p->items[4].Index == 5129 && p->items[5].Index == 5112 && p->items[6].Index == 5110)
	{
		if(Mob[clientId].Mobs.Player.Equip[0].EFV2 != CELESTIAL || player->bStatus.Level != 39 || !Mob[clientId].Mobs.Info.LvBlocked || Mob[clientId].Mobs.Info.Unlock39)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		// Remove os itens, independente se deu certo ou não
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		INT32 _rand = Rand() % 100;
		if(_rand <= 95)
		{
			Mob[clientId].Mobs.Info.LvBlocked = false;
			Mob[clientId].Mobs.Info.Unlock39  = true;

			SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
			Log(clientId, LOG_COMP, "Composição de desbloqueio 40 efetuada com SUCESSO");
		}
		else
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);

			Log(clientId, LOG_COMP, "Composição de desbloqueio 40 fetuada com FALHA");
		}

		SaveUser(clientId, 0);
	}
	else if(p->items[0].Index == 5125 && p->items[1].Index == 5115 && p->items[2].Index == 5111 && p->items[3].Index == 5112 && p->items[4].Index == 5120 && 
		p->items[5].Index == 5128 && p->items[6].Index == 5119)
	{
		int _rand = Rand() % 100;
		
		// Remove os itens, independente se deu certo ou não
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		// Seta o item no inventário
		player->Inventory[p->slot[0]].Index = 3020;

		// Atualiza o inventário
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

		Log(clientId, LOG_INGAME, "Sucesso na composição de Pedra da Fúria");
		LogPlayer(clientId, "Sucesso na composição da pedra da Fúria");

		SaveUser(clientId, 0);
	}
	else if(p->items[0].Index == 4127 && p->items[1].Index == 4127 && p->items[2].Index == 5135 && p->items[3].Index == 413 &&
		p->items[4].Index == 413 && p->items[5].Index == 413 && p->items[6].Index == 413)
	{
		if(Mob[clientId].Mobs.Player.Equip[0].EFV2 < CELESTIAL)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		INT32 sanc = GetItemSanc(&player->Equip[15]);
		if(sanc >= 9)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Cant_Refine_More]);

			return true;
		}
		
		// Remove os itens, independente se deu certo ou não
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}

		SetItemSanc(&player->Equip[15], sanc + 1, 0);
			
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
		SendItem(clientId, SlotType::Equip, 15, &player->Equip[15]);

		Log(clientId, LOG_COMP, "Refinação da capa obtida com sucesso: %d - Capa: %d. Ev: %d", sanc + 1, player->Equip[15].Index, Mob[clientId].Mobs.Player.Equip[0].EFV2);

		Mob[clientId].GetCurrentScore(clientId);
		SendScore(clientId);

		SaveUser(clientId, 0);
	}
	else if(p->items[0].Index == 413 && p->items[1].Index == 413 && p->items[2].Index == 413 && p->items[3].Index == 413 && p->items[4].Index == 413 && p->items[5].Index == 413 && 
		p->items[6].Index == 413)
	{
		// Remove os itens, independente se deu certo ou não
		for(int i = 0 ; i < 7 ; i++)
		{
			memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

			SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
		}
		
		// Seta o item no inventário
		player->Inventory[p->slot[0]].Index = 5134;

		// Atualiza o inventário
		SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);
		
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_INGAME, "Sucesso na composição de Pista de Runas");
		LogPlayer(clientId, "Sucesso na composição da Pista de Runas");

		SaveUser(clientId, 0);
		return true;
	}
	else if(p->items[0].Index == 674) 
	{
		// Composição da PEdra de Kersef
		INT32 nail		= GetInventoryAmount(clientId, 674); // 5x unha de Kefra
		INT32 heart		= GetInventoryAmount(clientId, 675); // 2x Coração de Sombra Negra
		INT32 hair		= GetInventoryAmount(clientId, 676); // 3x Cabelo do Beriel Amald
		INT32 heartBer	= GetInventoryAmount(clientId, 677); // 01x Coração do Beriel
		INT32 seal		= GetInventoryAmount(clientId, 4127); // 2x Pergaminho Selado
		INT32 leaf		= GetInventoryAmount(clientId, 770); // 5x Folha de Mandrágora

		if(nail < 5 || heart < 2 || hair < 3 || heartBer < 1 || seal < 2 || leaf < 5)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

			return true;
		}

		if(Mob[clientId].Mobs.Player.Gold < 100000000)
		{
			SendClientMessage(clientId, "Gold insuficiente!");

			return true;
		}

		INT32 slotId = GetFirstSlot(clientId, 0);
		if(slotId == -1)
			return false;

		Mob[clientId].Mobs.Player.Gold -= 100000000;

		INT32 _rand = Rand() % 100;
		if(_rand >= 80)
		{
			INT32 totalRemoved = 0;

			while(totalRemoved != 5)
			{
				INT32 itemId = 0;
				_rand = Rand() % 6;
				if(_rand == 0)
					itemId = 674;
				else if(_rand == 1)
					itemId = 675;
				else if(_rand == 2)
					itemId = 676;
				else if(_rand == 3)
					itemId = 677;
				else if(_rand == 4)
					itemId = 4127;
				else if(_rand == 5)
					itemId = 770;

				totalRemoved++;
				RemoveAmount(clientId, itemId, 1);

				Log(clientId, LOG_INGAME, "Removido %s (%d) por falha na composição", ItemList[itemId].Name);
			}
		
			SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);
			Log(clientId, LOG_INGAME, "Falha na composição da Pedra de Kersef lv0. %d/80", _rand);
			return true;
		}
		
		// removetodos os itens
		RemoveAmount(clientId, 674, 5);
		RemoveAmount(clientId, 675, 2);
		RemoveAmount(clientId, 676, 3);
		RemoveAmount(clientId, 677, 1);
		RemoveAmount(clientId, 4127, 2);
		RemoveAmount(clientId, 770, 5);

		memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

		Mob[clientId].Mobs.Player.Inventory[slotId].Index = 4552;
		SendItem(clientId, SlotType::Inv, slotId, &Mob[clientId].Mobs.Player.Inventory[slotId]);
	
		SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);

		Log(clientId, LOG_INGAME, "Composto com sucesso Pedra de Kersef (lv0)");
		return true;
	}
	else
	{
		bool any = false;
		constexpr int secretStone [4][7] = 
		{
			{5126,5127,5121,5114,5125,5111,5118},
			{5131,5113,5115,5116,5125,5112,5114},
			{5110,5124,5117,5129,5114,5125,5128},
			{5122,5119,5132,5120,5130,5133,5123}
		};

		for(int y = 0 ; y < 4; y++)
		{
			if(p->items[0].Index == secretStone[y][0] && p->items[1].Index == secretStone[y][1] && p->items[2].Index == secretStone[y][2] && p->items[3].Index == secretStone[y][3] &&
				p->items[4].Index == secretStone[y][4] && p->items[5].Index == secretStone[y][5] && p->items[6].Index == secretStone[y][6])
			{
				if(player->Gold < 2000000)
				{
					SendClientMessage(clientId, "São necessários 2 milhões de godl");

					return true;
				}

				// Remove os itens, independente se deu certo ou não
				for(int i = 0 ; i < 7 ; i++)
				{
					memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

					SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
				}

				// Retira o gold
				player->Gold -= 2000000;

				// Atualiza o gold
				SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

				any = true;

				int rand = Rand() % 101;
				if (rand > 95)
				{
					SendClientMessage(clientId, "Falha na composição %d/95", rand);

					Log(clientId, LOG_INGAME, "Combinação falhou de secreta. A secreta que deveria vir era: %s", ItemList[5334 + y].Name);
					SendNotice(".%s falhou na composiçaõ de %s", Mob[clientId].Mobs.Player.Name, ItemList[5334 + y].Name);
				}
				else
				{
					// Seta a Pedra Secreta
					player->Inventory[p->slot[0]].Index = (5334 + y);

					SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

					// Envia a mensagem de sucesso
					SendClientMessage(clientId, "Composição concluída %d/95", rand);

					Log(clientId, LOG_INGAME, "Composto com sucesso %s. %d/95", ItemList[5334 + y].Name, rand);
					LogPlayer(clientId, "Composto com sucesso %s", ItemList[5334 + y].Name);

					SendNotice(".%s compôs com sucesso a %s", Mob[clientId].Mobs.Player.Name, ItemList[5334 + y].Name);
				}

				SaveUser(clientId, 0);
				return true;
			}
		}

		bool allIsRune = true;
		for (int i = 0; i < 7; i++)
		{
			if (p->items[i].Index < 5110 || p->items[i].Index > 5133)
				allIsRune = false;
		}

		// Tentando gerar uma Secreta aleatoriamente
		if (allIsRune)
		{
			if (player->Gold < 2000000)
			{
				SendClientMessage(clientId, "São necessários 2 milhões de godl");

				return true;
			}

			// Remove os itens, independente se deu certo ou não
			for (int i = 0; i < 7; i++)
			{
				memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

				SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
			}

			// Retira o gold
			player->Gold -= 2000000;

			// Atualiza o gold
			SendSignalParm(clientId, clientId, 0x3AF, player->Gold);

			int rand = Rand() % 101;
			if (rand <= 5)
			{
				int secretId = (5334 + (Rand() % 4));
				player->Inventory[p->slot[0]].Index = secretId;

				SendItem(clientId, SlotType::Inv, p->slot[0], &player->Inventory[p->slot[0]]);

				// Envia a mensagem de sucesso
				SendClientMessage(clientId, "Composição concluída %d/05", rand);

				Log(clientId, LOG_INGAME, "Composto com sucesso %s usando Runas aleatórias", ItemList[secretId].Name);
				LogPlayer(clientId, "Composto com sucesso %s usando Runas aleatórias", ItemList[secretId].Name);

				SendNotice(".%s compôs com sucesso a %s", Mob[clientId].Mobs.Player.Name, ItemList[secretId].Name);
			}
			else
			{
				SendClientMessage(clientId, "Houve uma falha na composição do item %d/05", rand);

				Log(clientId, LOG_INGAME, "Combinação falhou de secreta usando Runas aleatórias");
				SendNotice(".%s falhou na composição da Pedra Secreta", Mob[clientId].Mobs.Player.Name);
			}

			return true;
		}

		if(!any)
			SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);
	}

	return true;
}