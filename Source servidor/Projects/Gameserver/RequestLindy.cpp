#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestLindy(PacketHeader *Header)
{
	pCompor *p = (pCompor*)Header;

	st_Mob *player = &Mob[clientId].Mobs.Player;

	for (int i = 0; i < 7; i++)
	{
		if (p->slot[i] < 0 || p->slot[i] > 60)
		{
			Log(clientId, LOG_HACK, "[HACK] Banido por enviar índice inválido - NPC Lindy - %d", p->slot[i]);
			Log(SERVER_SIDE, LOG_HACK, "[HACK] %s - Banido por enviar índice inválido - NPC Lindy - %d", player->Name, p->slot[i]);

			SendCarry(clientId);

			return true;
		}

		if (memcmp(&player->Inventory[p->slot[i]], &p->items[i], 8) != 0)
		{
			Log(clientId, LOG_HACK, "Banido por enviar item inexistente - NPC Lindy - %d", p->items[i].Index);
			Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item inexistente - NPC Lindy - %d", player->Name, p->items[i].Index);

			SendCarry(clientId);

			return true;
		}

		for (int y = 0; y < 7; y++)
		{
			if (y == i)
				continue;

			if (p->slot[i] == p->slot[y])
			{
				Log(clientId, LOG_HACK, "Banido por enviar item com mesmo slotId - NPC Lindy - %d", p->items[i].Index);
				Log(SERVER_SIDE, LOG_HACK, "%s - Banido por enviar item com mesmo slotId  - NPC Lindy- %d", player->Name, p->items[i].Index);

				CloseUser(clientId);
				return true;
			}
		}
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");

		return true;
	}


	for (int i = 0; i < 7; i++)
	{
		Log(clientId, LOG_COMP, "Lindy - %d - %s [%d] [%d %d %d %d %d %d] - %d", i, ItemList[p->items[i].Index].Name, p->items[i].Index,
			p->items[i].Effect[0].Index, p->items[i].Effect[0].Value, p->items[i].Effect[1].Index, p->items[i].Effect[1].Value, p->items[i].Effect[2].Index,
			p->items[i].Effect[2].Value, p->slot[i]);
	}

	if (Trade.ClientId != 0)
	{
		RemoveTrade(Trade.ClientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	if (((player->bStatus.Level == 354 && !Mob[clientId].Mobs.Info.Unlock354) || (player->bStatus.Level == 369 && !Mob[clientId].Mobs.Info.Unlock369)) && Mob[clientId].Mobs.Info.LvBlocked)
	{
		if (p->items[0].Index == 413 && p->items[1].Index == 413 && p->items[2].Index == 4127 && p->items[3].Index == 413 && p->items[4].Index == 413 &&
			p->items[5].Index == 413 && p->items[6].Index == 413)
		{
			// Fecha o inventário
			SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);

			// Checa se o item possui 10 unidades
			INT32 amount_1 = GetItemAmount(&p->items[0]);
			INT32 amount_2 = GetItemAmount(&p->items[1]);

			if (amount_1 != 10 || amount_2 != 10 || !Mob[clientId].Mobs.Info.LvBlocked || (player->bStatus.Level == 369 && Mob[clientId].Mobs.Fame < 1))
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

				return true;
			}

			// Remove os itens, independente se deu certo ou não
			for (int i = 0; i < 7; i++)
			{
				memset(&player->Inventory[p->slot[i]], 0, sizeof st_Item);

				SendItem(clientId, SlotType::Inv, p->slot[i], &player->Inventory[p->slot[i]]);
			}

			if (player->bStatus.Level == 369)
				Mob[clientId].Mobs.Fame -= 1;

			INT32 _rand = Rand() % 100;

			// Desbloqueou com sucesso, amigo.
			if (_rand <= 90)
			{
				Mob[clientId].Mobs.Info.LvBlocked = false;

				if (Mob[clientId].Mobs.Player.bStatus.Level == 354)
					Mob[clientId].Mobs.Info.Unlock354 = TRUE;
				else
					Mob[clientId].Mobs.Info.Unlock369 = TRUE;

				INT32 capeId = Mob[clientId].Mobs.Player.Equip[15].Index;
				if ((capeId != 3191 && capeId != 3192 && capeId != 3193) || capeId == 0)
				{
					INT32 newCape = 0;
					if (Mob[clientId].Mobs.Player.CapeInfo == 7)
						newCape = 3191;
					else if (Mob[clientId].Mobs.Player.CapeInfo == 8)
						newCape = 3192;
					else if (Mob[clientId].Mobs.Player.CapeInfo == 9 || capeId == 0)
						newCape = 3193;

					memset(&Mob[clientId].Mobs.Player.Equip[15], 0, sizeof st_Item);

					Mob[clientId].Mobs.Player.Equip[15].Index = newCape;
					Mob[clientId].Mobs.Player.Equip[15].Effect[0].Index = EF_SANC;
					Mob[clientId].Mobs.Player.Equip[15].Effect[0].Value = 0;

					Log(clientId, LOG_INGAME, "Alterado a capa para %d", newCape);
				}

				if (player->bStatus.Level == 369)
				{
					Mob[clientId].Mobs.Player.Equip[15].Effect[1].Index = EF_RESISTALL;
					Mob[clientId].Mobs.Player.Equip[15].Effect[1].Value = 10;

					Log(clientId, LOG_INGAME, "Adicionado adicional de resistência na capa");
				}

				SendClientMessage(clientId, g_pLanguageString[_NN_Success_Comp]);
				Log(clientId, LOG_COMP, "Composição de desbloqueio 355 efetuada com SUCESSO 0- %d/90", _rand);

				SendItem(clientId, SlotType::Equip, 15, &Mob[clientId].Mobs.Player.Equip[15]);
			}
			else
			{
				SendClientMessage(clientId, g_pLanguageString[_NN_CombineFailed]);

				Log(clientId, LOG_COMP, "Composição de desbloqueio 355 efetuada com FALHA - %d/90", _rand);
			}
		}
	}
	else
		SendClientMessage(clientId, g_pLanguageString[_NN_IncorrectComp]);

	SaveUser(clientId, 0);
	return true;
}