#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"


void SortTradeItem(st_Item *item, char type)
{
	INT32 iterator; //LOCAL_1
	INT32 buffer [15]; //LOCAL9
	memset(buffer, -1, sizeof buffer);

	for(iterator = 0; iterator < 15 ; iterator ++)
	{
		if (item[iterator].Index == 0)
		{
			buffer[iterator] = -1;

			continue;
		}

		buffer[iterator] = GetItemAbility(&item[iterator], type);
	}

	st_Item itemTemp[15]; // LOCAL25
	memset(itemTemp, 0, sizeof itemTemp);

	for(iterator = 0; iterator < 15; iterator++)
	{
		INT32 maxBufferIndex = 0; //LOCAL26
		INT32 maxBuffer = -1; // LOCAL27

		for (INT32 i = 0; i < 15; i++) // local28
		{
			if (buffer[i] > maxBuffer)
			{
				maxBufferIndex = i;
				maxBuffer = buffer[i];
			}
		}

		if (maxBuffer == -1)
			break;

		buffer[maxBufferIndex] = -1;
		memcpy(&itemTemp[iterator], &item[maxBufferIndex], sizeof st_Item);
	}

	memcpy(item, itemTemp, sizeof st_Item * 15);
}

INT32 CanTrade(st_Item *dest, st_Item *inv, char* myTrade, st_Item *opponentTrade)
{
	st_Item opponentTemp[15]; //local16
	memcpy(&opponentTemp, opponentTrade, sizeof st_Item * 15);

	memcpy(dest, inv, sizeof st_Item * 64);

	for(INT32 i = 0; i < 15; i ++) // local17
	{
		INT32 slot = myTrade[i]; // local18
		if (slot == -1)
			continue;

		memset(&dest[slot], 0, sizeof st_Item);
	}

	// 00409BE6
	SortTradeItem(opponentTemp, EF_GRID);
	for (INT32 i = 0; i < 15; i++) // local17
	{
		if (opponentTemp[i].Index == 0)
			continue;

		INT32 freeSlot = -1; // LOCAL19
		for(int i = 0 ; i < 30; i++)
		{
			if(dest[i].Index == 0)
			{
				freeSlot = i;

				break;
			}
		}

		if(freeSlot == -1 && dest[60].Index == 3467)
		{
			float remainig = TimeRemaining(dest[60].EFV1, dest[60].EFV2, dest[60].EFV3 + 1900);
			if(remainig > 0.0f)
			{
				for(int i = 30; i < 45; i++)
				{
					if(dest[i].Index == 0)
					{
						freeSlot = i;
						break;
					}
				}
			}
		}

		if(freeSlot == -1 && dest[61].Index == 3467)
		{
			float remainig = TimeRemaining(dest[61].EFV1, dest[61].EFV2, dest[61].EFV3 + 1900);
			if(remainig > 0.0f)
			{
				for(int i = 45; i < 60; i++)
				{
					if(dest[i].Index == 0)
					{
						freeSlot = i;
						break;
					}
				}
			}
		} 


		if(freeSlot != -1)
		{
			memcpy(&dest[freeSlot], &opponentTemp[i], sizeof st_Item);

			continue;
		}

		return false;
	}

	return true;
}

bool CUser::RequestTrade(PacketHeader *Header)
{
	p383 *msg = (p383*)(Header); // LOCAL1348

	if(Mob[clientId].Mobs.Player.Status.curHP <= 0 || Users[clientId].Status != USER_PLAY)
	{
		Log(clientId, LOG_ERROR, "Tentando usar o trade sem estar no jogo ou com o HP zerado.");
		SendHpMode(clientId);
		RemoveTrade(clientId);
		return true;
	}
	
	INT32 opponentId = msg->ClientId; // LOCAL1349
	if(opponentId <= 0 || opponentId >= MAX_PLAYER)
	{
		Log(clientId, LOG_ERROR, "Enviando oponente com indice irregular.");

		RemoveTrade(clientId);
		return true;
	}

	if(Users[opponentId].Status != USER_PLAY)
	{
		RemoveTrade(clientId);

		return true;
	}

	if(User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}

	if(msg->Gold < 0 || msg->Gold > Mob[clientId].Mobs.Player.Gold)
	{
		RemoveTrade(clientId);
		RemoveTrade(opponentId);

		return true;
	}
	
	for(INT32 i = 0; i < 15; i ++) // local1350
	{
		if(msg->Item[i].Index != 0)
		{
			INT32 slot = msg->Slot[i]; // local1351
			if (slot < 0 || slot >= 60)
				return true;
			
			INT32 verify = memcmp(&Mob[clientId].Mobs.Player.Inventory[slot], &msg->Item[i], 8); // local1352
			if (verify != 0)
			{
				Log(clientId, LOG_ERROR, "Retirou ou modificou o item enquanto estava no trade.");
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}	
		}
		/// 43262B
		if(Users[opponentId].Trade.Item[i].Index != 0)
		{
			INT32 slot = Users[opponentId].Trade.Slot[i]; // local1353
			if (slot < 0 || slot > 60)
				return true;

			INT32 verify = memcmp(&Mob[opponentId].Mobs.Player.Inventory[slot], &Users[opponentId].Trade.Item[i], 8); // local1354
			if (verify != 0)
			{
				Log(opponentId, LOG_ERROR, "Retirou ou modificou o item enquanto estava no trade.");

				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}
		}
	}

	INT32 lastOpponent = Users[clientId].Trade.ClientId; //local1355
	if(lastOpponent == opponentId)
	{
		for(INT32 i = 0; i < 15; i++) // local1356
		{
			INT32 verify = 0; // local1357
			if(Users[clientId].Trade.Item[i].Index != 0)
				verify = memcmp(&Users[clientId].Trade.Item[i], &msg->Item[i], 8);

			if(Users[clientId].Trade.Gold  != 0 && msg->Gold != Users[clientId].Trade.Gold)
				verify = 1;

			if (verify != 0)
			{
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}
		}
	}
	else
	{
		if(lastOpponent != 0)
		{
			SendClientMessage(clientId,  g_pLanguageString[_NN_Already_Trading]);

			RemoveTrade(clientId);
			return true;
		}
	}
	
	if (Users[opponentId].AllStatus.PK)
	{
		SendClientMessage(clientId, "O outro jogador está com o modo PvP ativo");
		
		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	if (Users[clientId].AllStatus.PK)
	{
		SendClientMessage(clientId, "Não é possível trocar com o modo PvP ativo");
		
		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	if (Users[opponentId].AllStatus.Whisper)
	{
		SendClientMessage(clientId, "O outro jogador está com o chat desativado");
		
		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	if (Users[clientId].AllStatus.Whisper)
	{
		SendClientMessage(clientId, "Não é possível trocar com o chat desativado");

		RemoveTrade(opponentId);
		RemoveTrade(clientId);
		return true;
	}

	INT32 clientGuildId = Mob[clientId].Mobs.Player.GuildIndex; // local1358
	INT32 opponentGuildId = Mob[opponentId].Mobs.Player.GuildIndex; // local1359

	// 004328C3
	INT32 clientMemberType = Mob[clientId].Mobs.Player.GuildMemberType; // local1360
	INT32 opponentMemberType = Mob[opponentId].Mobs.Player.GuildMemberType; // local1361

	for(INT32 i = 0; i < 15; i ++) // local1362
	{
		if(msg->Item[i].Index == 747 || msg->Item[i].Index == 3993 || msg->Item[i].Index == 3994)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Only_With_Guild_Master]);
			SendClientMessage(opponentId, g_pLanguageString[_NN_Only_With_Guild_Master]);

			RemoveTrade(clientId);
			if(Users[opponentId].Trade.ClientId == clientId)
				RemoveTrade(opponentId);

			return true;
		}
		/*
		if(msg->Item[i].Index == 508 || msg->Item[i].Index == 522 || 
			(msg->Item[i].Index >= 506 && msg->Item[i].Index < 537) || msg->Item[i].Index == 446)
		{
			INT32 guild = GetGuild(&msg->Item[i]); // local1363

			if (guild == clientGuildId && clientMemberType != 0)
				continue;

			if (guild == opponentGuildId && opponentMemberType != 0)
				continue;

			SendClientMessage(clientId, g_pLanguageString[_NN_Only_With_Guild_Master]);
			SendClientMessage(opponentId, g_pLanguageString[_NN_Only_With_Guild_Master]);

			RemoveTrade(clientId);

			if(Users[opponentId].Trade.ClientId == clientId)
				RemoveTrade(opponentId);

			return true;
		}
		*/
		if (msg->Item[i].Index && GetItemAbility(&msg->Item[i], EF_NOTRADE))
		{
			SendClientMessage(clientId, "Este item não pode ser trocado.");
			SendClientMessage(opponentId, "Este item não pode ser trocado.");

			RemoveTrade(clientId);
			if (Users[opponentId].Trade.ClientId == clientId)
				RemoveTrade(opponentId);

			return true;
		}
	}
	// 00432AC0

	if(Users[opponentId].Trade.ClientId == 0)
	{
		auto now = std::chrono::steady_clock::now();
		if (now - LastTrade <= 1s)
		{
			SendClientMessage(clientId, "Aguarde para enviar novamente");

			LastTrade = now;
			return true;
		}

		memcpy(&Users[clientId].Trade, msg, sizeof p383);

		Users[clientId].Trade.Confirm = 0;
		Users[clientId].Trade.Gold = 0;

		msg->Header.ClientId = opponentId;
		msg->ClientId = clientId;

		Users[opponentId].AddMessage((BYTE*)msg, sizeof p383);

		Users[opponentId].Trade.Confirm = 0;
		Users[opponentId].Trade.Gold = 0;

		LastTrade = now;
		return true;
	}

	if(Users[opponentId].Trade.ClientId != clientId)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_Already_Trading]);

		RemoveTrade(clientId);
		return true;
	}

	// 00432BCF
	for(INT32 i = 0; i < 15; i++) // local1364
	{
		INT32 slot = msg->Slot[i]; // local1365
		if(slot == -1)
		{
			memset(&msg->Item[i], 0, sizeof st_Item);

			continue;
		}

		if(slot < -1  || slot >= 60)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Trade_Packet1]);
			RemoveTrade(clientId);

			return true;
		}

		for(INT32 j = 0; j < 15; j ++) // local1366
		{
			if(j == i)
				continue;

			if(slot == msg->Slot[j])
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Wrong_Trade_Packet1]);
				RemoveTrade(clientId);

				return true;
			}
		}

		memcpy(&msg->Item[i], &Mob[clientId].Mobs.Player.Inventory[slot], sizeof st_Item);
	}

	if(msg->Confirm == 1)
	{
		Users[clientId].Trade.Confirm = true;
		Users[clientId].Trade.Header.Key = msg->Header.Key;
		Users[clientId].Trade.Header.CheckSum = msg->Header.CheckSum;
		Users[clientId].Trade.Header.TimeStamp = msg->Header.TimeStamp;

		int verify = memcmp(&Users[clientId].Trade, msg, sizeof p383); // local1367
		if (verify != 0)
		{
			RemoveTrade(clientId);
			RemoveTrade(opponentId);

			Log(clientId, LOG_ERROR, "Tentou tirar/mudar o item enquanto estava com trade.");
			return false;
		}

		if(Users[opponentId].Trade.Confirm == 1)
		{
			if(Users[clientId].Trade.Gold < 0 || Users[clientId].Trade.Gold > 2000000000)
			{
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
			}

			if(Users[opponentId].Trade.Gold < 0 || Users[opponentId].Trade.Gold > 2000000000)
			{
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
			}

			INT32 totalGoldClient = Mob[clientId].Mobs.Player.Gold - Users[clientId].Trade.Gold + Users[opponentId].Trade.Gold; // local1368
			if (totalGoldClient > 2000000000 || totalGoldClient < 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);
				
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}

			INT32 totalGoldOpponent = Mob[opponentId].Mobs.Player.Gold - Users[opponentId].Trade.Gold + Users[clientId].Trade.Gold; // local1369
			if (totalGoldOpponent > 2000000000 || totalGoldOpponent < 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Cant_get_more_than_2G]);
				
				RemoveTrade(clientId);
				RemoveTrade(opponentId);
				return true;
			}

			if(Users[clientId].Trade.Gold > Mob[clientId].Mobs.Player.Gold)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Havent_Money_So_Much]);
				SendClientMessage(opponentId, g_pLanguageString[_NN_Opponent_Havent_Money]);

				return true;
			}

			if(Users[opponentId].Trade.Gold > Mob[opponentId].Mobs.Player.Gold)
			{
				SendClientMessage(opponentId, g_pLanguageString[_NN_Havent_Money_So_Much]);
				SendClientMessage(clientId, g_pLanguageString[_NN_Opponent_Havent_Money]);

				return true;
			}

			st_Item clientDest[64]; // local1497
			st_Item opponentDest[64]; // local1625
			// 00433036
			INT32 clientTrade = CanTrade(clientDest, Mob[clientId].Mobs.Player.Inventory, Users[clientId].Trade.Slot, Users[opponentId].Trade.Item); // local1626
			INT32 opponentTrade = CanTrade(opponentDest, Mob[opponentId].Mobs.Player.Inventory, Users[opponentId].Trade.Slot, Users[clientId].Trade.Item);; // BASE_CanTrade local1627

			if (clientTrade == 0 && opponentTrade == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_Each_Of_You_Havent_Space]);
				SendClientMessage(opponentId, g_pLanguageString[_NN_Each_Of_You_Havent_Space]);

				return true;
			}

			if (clientTrade == 0)
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);
				SendClientMessage(opponentId, g_pLanguageString[_NN_Opponent_Have_No_Space_To]);

				return true;
			}

			if (opponentTrade == 0)
			{
				SendClientMessage(opponentId, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);
				SendClientMessage(clientId, g_pLanguageString[_NN_Opponent_Have_No_Space_To]);

				return true;
			}
			
			Log(clientId, LOG_INGAME, "Trade com conta %s", Users[opponentId].User.Username);
			Log(opponentId, LOG_INGAME, "Trade com conta %s", Users[clientId].User.Username);

			for(INT32 i = 0 ; i < 15; i++)
			{
				st_Item *item = &Users[opponentId].Trade.Item[i];
				if(item->Index > 0 && item->Index < 6500)
				{ // Está enviando item
					Log(opponentId, LOG_INGAME, "Trade - %d - Enviado %s para %s [%d] [%d %d %d %d %d %d]", i, ItemList[item->Index].Name, Mob[clientId].Mobs.Player.Name, item->Index, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					Log(clientId, LOG_INGAME, "Trade - %d - Recebido %s de %s [%d] [%d %d %d %d %d %d]", i, ItemList[item->Index].Name, Mob[opponentId].Mobs.Player.Name, item->Index, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					
					LogPlayer(opponentId, "Enviado o item %s para %s por trade.", ItemList[item->Index].Name, Mob[clientId].Mobs.Player.Name);
					LogPlayer(clientId, "Recebido o item %s para %s por trade.", ItemList[item->Index].Name, Mob[opponentId].Mobs.Player.Name);
				}
			}

			for(INT32 i = 0 ; i < 15; i++)
			{
				st_Item *item = &Users[clientId].Trade.Item[i];
				if(item->Index > 0 && item->Index < 6500)
				{ // Está enviando item
					Log(clientId, LOG_INGAME, "Trade - %d - Enviado %s para %s [%d] [%d %d %d %d %d %d]", i, ItemList[item->Index].Name, Mob[opponentId].Mobs.Player.Name, item->Index, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					Log(opponentId, LOG_INGAME, "Trade - %d - Recebido %s de %s [%d] [%d %d %d %d %d %d]", i, ItemList[item->Index].Name, Mob[clientId].Mobs.Player.Name, item->Index, item->EF1, item->EFV1,
						item->EF2, item->EFV2, item->EF3, item->EFV3);
					
					LogPlayer(opponentId, "Recebido o item %s para %s por trade.", ItemList[item->Index].Name, Mob[clientId].Mobs.Player.Name);
					LogPlayer(clientId, "Enviado o item %s para %s por trade.", ItemList[item->Index].Name, Mob[opponentId].Mobs.Player.Name);
				}
			}

			// 00433169
			memcpy(Mob[clientId].Mobs.Player.Inventory, clientDest, sizeof st_Item * 64);
			memcpy(Mob[opponentId].Mobs.Player.Inventory, opponentDest, sizeof st_Item * 64);
			
			Log(clientId, LOG_INGAME, "Trade - Enviado %d de gold para o usuário", Users[clientId].Trade.Gold);
			Log(opponentId, LOG_INGAME, "Trade - Recebido %d de gold do usuário", Users[clientId].Trade.Gold);

			Log(opponentId, LOG_INGAME, "Trade - Enviado %d de gold para o usuário", Users[opponentId].Trade.Gold);
			Log(clientId, LOG_INGAME, "Trade - Recebido %d de gold do usuário", Users[opponentId].Trade.Gold);

			LogPlayer(clientId, "Enviado %d de gold para o usuário", Users[clientId].Trade.Gold);
			LogPlayer(opponentId, "Recebido %d de gold do usuário", Users[clientId].Trade.Gold);

			LogPlayer(opponentId, "Enviado %d de gold para o usuário", Users[opponentId].Trade.Gold);
			LogPlayer(clientId, "Recebido %d de gold do usuário", Users[opponentId].Trade.Gold);

			Mob[clientId].Mobs.Player.Gold = totalGoldClient;
			Mob[opponentId].Mobs.Player.Gold = totalGoldOpponent;

			SendCarry(clientId);
			SendCarry(opponentId);

			RemoveTrade(clientId);
			RemoveTrade(opponentId);

			SaveUser(clientId, 0);
			SaveUser(opponentId, 0);

			return true;
		}
		
		SendSignal(clientId, clientId, 0x386);

		msg->ClientId = clientId;
		msg->Header.ClientId = opponentId;

		Users[opponentId].AddMessage((BYTE*)msg, sizeof p383);
		return true;
	}

	memcpy(&Users[clientId].Trade, msg, sizeof p383);

	Users[clientId].Trade.Confirm = false;
	Users[opponentId].Trade.Confirm = false;

	
	msg->Header.ClientId = opponentId;
	msg->ClientId = clientId;

	Users[opponentId].AddMessage((BYTE*)msg, sizeof p383);
	return true;
}