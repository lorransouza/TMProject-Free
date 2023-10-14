#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

void GiveTime(p376 *p)
{
	INT32 clientId = p->Header.ClientId;
	// Serve para quando equipar a Esfera dê a validade ao item
	int days = 0;

	//if (p->SrcType != (unsigned char)SlotType::Equip && p->DstType != (unsigned char)SlotType::Equip && p->DstType != (unsigned char)SlotType::MeridaEquip && p->SrcType != (unsigned char)SlotType::MeridaEquip)
	//	return;

	st_Item* item = nullptr;
	SlotType type;
	int slot;
	if (p->DstType == (unsigned char)SlotType::Equip /*|| p->DstType == (unsigned char)SlotType::MeridaEquip*/)
	{
		item = GetItemPointer(clientId, p->DstType, p->DstSlot);
		type = (SlotType)p->SrcType;
		slot = p->SrcSlot;
	}
	else if (p->SrcType == (unsigned char)SlotType::Equip /*|| p->SrcType == (unsigned char)SlotType::MeridaEquip*/)
	{
		item = GetItemPointer(clientId, p->SrcType, p->SrcSlot);
		type = (SlotType)p->DstType;
		slot = p->DstSlot;
	}

	if (item == nullptr)
		return;

	switch (item->Index)
	{
	case 3916:
	case 3917:
		days = 1;
		break;

	case 3980:
	case 3981:
	case 3982:
	case 3900:
	case 3901:
	case 3902:
	case 3998:
		days = 3;
		break;
	case 3903:
	case 3904:
	case 3905:
	case 3990: // Grande lobo
		days = 5;
		break;
	case 3906:
	case 3907:
	case 3908:
	case 3911:
	case 3997:
		days = 7;
		break;
	case 3983:
	case 3984:
	case 3985:
	case 3912:
	case 3996:
		days = 15;
		break;
	case 3986:
	case 3987:
	case 3988:
	case 3989:
	case 3913:
	case 4151:
		//case 4152: // valq
		//case 4153: // esqueleto
	case 4154:
		//case 4155:
		//case 4156:
	case 4157:
	case 4158:
	case 4159:
	case 4160:
	case 4161:
	case 4162:
	case 4163:
	case 4164:
	case 4165:
	case 4166:
	case 4167:
	case 4168:
	case 4169:
	case 4170:
	case 4171:
	case 4172:
	case 4173:
	case 4174:
	case 4175:
	case 4176:
	case 4177:
	case 4178:
	case 4179:
	case 4180:
	case 4181:
	case 4182:
	case 4183:
	case 4184:
	case 4185:
	case 4186:
	case 4187:
	case 4188:
	case 4189:
	case 4190:
	case 4191:
	case 4192:
	case 3995:
	case 4229:
	case 4240:
	case 4241:
	case 4235:
	case 4236:
	case 4237:
	case 4233:
	case 4224:
	case 4231:
	case 4230:
	case 4243:
	case 4242:
	case 4244:
	case 4225:
	case 4226:
	case 4227:
    case 4245:
    case 4246:
    case 4247:
		days = 30;
		break;

	default:
		return;
	}

	time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);

	int cMes = now.tm_mon + 1; //0 Janeiro, 1 Fev
	int cDia = now.tm_mday;
	int cAno = now.tm_year;;

	int mes = 0, dia = 0, ano = 0;

	mes = cMes;
	dia = cDia + days;
	ano = cAno + 1900;

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

	bool isCostume = IsCostume(item);
	if ((item->Index >= 3980 && item->Index <= 3999) || (item->Index >= 3995 && item->Index <= 3995) || isCostume)
	{
		if ((item->EF1 == 106 && item->EF2 == 110 && item->EF3 == 109) || (isCostume && item->EFV1 != 0 && item->EFV2 != 0 && item->EFV3 != 0))
		{
			float difDays = TimeRemaining(item->EFV1, item->EFV2, (item->EFV3 + 2000));
			if (difDays <= 0.0f)
			{
				SendClientMessage(p->Header.ClientId, "Esfera / Traje expirou...");
				Log(clientId, LOG_INGAME, "Traje / Esfera expirado. Expira em: %d/%d/%d", item->EFV1, item->EFV2, item->EFV3);

				memset(item, 0, sizeof st_Item);

				SendItem(clientId, type, slot, item);
				return;
			}
			else
			{
				if (isCostume && item->EF1 == 106 && item->EF2 == 110 && item->EF3 == 109)
				{
					Log(clientId, LOG_INGAME, "Removido os adicionais de tempo do traje %s %s", ItemList[item->Index].Name, item->toString().c_str());

					item->EF1 = 0;
					item->EF2 = 0;
					item->EF3 = 0;
				}
			}

			return;
		}
	}
	else if ((item->Index >= 3900 && item->Index <= 3908) || item->Index == 3916 || item->Index == 3917)
	{
		if (item->EF1 != 0)
			return;

		item->EF1 = 106;
		item->EFV1 = days;

		SendItem(clientId, type, slot, item);
		return;
	}

	if (!isCostume)
	{
		item->EF1 = 106;
		item->EFV1 = dia;

		item->EF2 = 110;
		item->EFV2 = mes;

		item->EF3 = 109;
		item->EFV3 = (ano - 2000);
	}
	else
	{
		item->EFV1 = dia;
		item->EFV2 = mes;
		item->EFV3 = (ano - 2000);
	}

	SendItem(clientId, type, slot, item);
	Log(clientId, LOG_INGAME, "Aplicado tempo no item %s %s - %d/%d/%d", ItemList[item->Index].Name, item->toString().c_str(), dia, mes, ano);
}

bool CUser::RequestMoveItem(PacketHeader *header)
{
	static const char *szSlotType[] = { "EQUIPAMENTO", "INVENTÁRIO", "BANCO", "RUNA" , "MERIDA INVENTÁRIO", "MERIDA EQUIPS"};

	p376* p = (p376*)header;
	if (!Mob[clientId].Mobs.Player.Status.curHP || Status != USER_PLAY)
	{
		SendHpMode(clientId);

		AddCrackError(clientId, 1, CRACK_USER_STATUS);
		return true;
	}

	if (Users[clientId].Trade.ClientId != MOB_EMPTY)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, CRACK_TRADE_NOTEMPTY);
		return true;
	}

	if (IsAutoTrading)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_CantWhenAutoTrade]);

		return true;
	}

	if (User.Block.Blocked)
	{
		SendClientMessage(clientId, "Desbloqueie seus itens para poder movimentá-los");	

		return true;
	}

	p->Header.ClientId = clientId;
	st_Item *mountSlot = &Mob[clientId].Mobs.Player.Equip[14]; // LOCAL1684

	st_Item *equipDstSlot; // LOCAL1649
	st_Item *equipSrcSlot; // LOCAL1650
	INT32 needSendEquip = 0; // LOCAL1651
	INT32 needSendScore = 0; // LOCAL1652

	if (p->DstType == (unsigned char)SlotType::Equip)
	{
		if (p->DstSlot <= 0 || p->DstSlot > 17)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);
			return false;
		}

		if (p->DstSlot == 15)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);
			Log(clientId, LOG_HACK, "Tentativa de mover a capa do personagem");
			return false;
		}

		equipDstSlot = &Mob[clientId].Mobs.Player.Equip[p->DstSlot];

		needSendEquip = 1;
		needSendScore = 1;
	}
	else if (p->DstType == (unsigned char)SlotType::Inv)
	{
		if (p->DstSlot < 0 || p->DstSlot >= 60)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		if (p->DstSlot >= 30 && p->DstSlot < 45)
		{
			if (TimeRemaining(Mob[clientId].Mobs.Player.Inventory[60].EFV1, Mob[clientId].Mobs.Player.Inventory[60].EFV2, Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) <= 0)
				return false;
		}
		else if (p->DstSlot >= 45 && p->DstSlot < 60)
		{
			if (TimeRemaining(Mob[clientId].Mobs.Player.Inventory[61].EFV1, Mob[clientId].Mobs.Player.Inventory[61].EFV2, Mob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900) <= 0)
				return false;
		}

		equipDstSlot = &Mob[clientId].Mobs.Player.Inventory[p->DstSlot];
	}
	else if (p->DstType == (unsigned char)SlotType::Storage)
	{
		if (p->DstSlot < 0 || p->DstSlot >= 120)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		equipDstSlot = &Users[clientId].User.Storage.Item[p->DstSlot];
	}
	else
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	if (p->SrcType == (unsigned char)SlotType::Equip)
	{
		if (p->SrcSlot <= 0 || p->SrcSlot > 17)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		if (p->SrcSlot == 15)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			Log(clientId, LOG_HACK, "Tentativa de mover a capa do personagem");
			return false;
		}

		equipSrcSlot = &Mob[clientId].Mobs.Player.Equip[p->SrcSlot];

		needSendEquip = 1;
		needSendScore = 1;
	}
	else if (p->SrcType == (unsigned char)SlotType::Inv)
	{
		if (p->SrcSlot < 0 || p->SrcSlot >= 60)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}
		/*
		if (p->SrcSlot >= 30 && p->SrcSlot < 45)
		{
			if (TimeRemaining(Mob[clientId].Mobs.Player.Inventory[60].EFV1, Mob[clientId].Mobs.Player.Inventory[60].EFV2, Mob[clientId].Mobs.Player.Inventory[60].EFV3 + 1900) <= 0)
				return false;
		}
		else if (p->SrcSlot >= 45 && p->SrcSlot < 60)
		{
			if (TimeRemaining(Mob[clientId].Mobs.Player.Inventory[61].EFV1, Mob[clientId].Mobs.Player.Inventory[61].EFV2, Mob[clientId].Mobs.Player.Inventory[61].EFV3 + 1900) <= 0)
				return false;
		}
		*/
		equipSrcSlot = &Mob[clientId].Mobs.Player.Inventory[p->SrcSlot];
	}
	else if (p->SrcType == (unsigned char)SlotType::Storage)
	{
		if (p->SrcSlot < 0 || p->SrcSlot >= 120)
		{
			AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

			return false;
		}

		equipSrcSlot = &Users[clientId].User.Storage.Item[p->SrcSlot];
	}
	else
	{
		AddCrackError(clientId, 2, CRACK_USER_PKTHACK);

		return false;
	}

	// Testa se são os mesmos itens
	if (equipSrcSlot->Index == equipDstSlot->Index)
	{
		st_Item* srcItem = GetItemPointer(p->Header.ClientId, p->SrcType, p->SrcSlot);
		st_Item* dstItem = GetItemPointer(p->Header.ClientId, p->DstType, p->DstSlot);

		if (AgroupItem(clientId, srcItem, dstItem))
		{
			Log(clientId, LOG_INGAME, "O agrupamento foi efetuado de slot %d tipo %s para slot %d tipo %s", p->SrcSlot, szSlotType[p->SrcType], p->DstSlot, szSlotType[p->DstType]);

			SendItem(p->Header.ClientId, (SlotType)p->SrcType, p->SrcSlot, srcItem);
			SendItem(p->Header.ClientId, (SlotType)p->DstType, p->DstSlot, dstItem);

			return true;
		}
	}

	if ((equipSrcSlot->Index >= 5110 && equipSrcSlot->Index <= 5133) && equipDstSlot->Index == 4854)
	{
		EnergizeEmptyRune(clientId, p);

		return true;
	}

	if (equipDstSlot->Index == 747 || equipSrcSlot->Index == 747 ||
		equipDstSlot->Index == 3993 || equipSrcSlot->Index == 3993 ||
		equipDstSlot->Index == 3994 || equipSrcSlot->Index == 3994)
	{
		if (p->DstType == 2 || p->SrcType == 2)
		{
			INT32 guildId = Mob[clientId].Mobs.Player.GuildIndex;
			if (guildId <= 0)
				return false;

			for (INT32 i = 0; i < 10; i++)
			{
				if (ChargedGuildList[i][4] == guildId)
					return true;
			}

			if (IsAdmin)
				return true;

			return false;
		}
	}

	if (equipDstSlot->Index == 877 || equipSrcSlot->Index == 877)
		return true;

	st_Item newEquipSrcSlot; // LOCAL1655
	st_Item newEquipDstSlot; // LOCAL1657
	memcpy(&newEquipSrcSlot, equipSrcSlot, sizeof st_Item);
	memcpy(&newEquipDstSlot, equipDstSlot, sizeof st_Item);

	memset(equipSrcSlot, 0, sizeof st_Item);
	memset(equipDstSlot, 0, sizeof st_Item);

	// 00433D7B
	INT32 canMove1 = 1; // LOCAL1659
	INT32 canMove2 = 1; // LOCAL1660

	if (newEquipDstSlot.Index != 0)
	{
		INT32 error = -2; // LOCAL1661
		if (p->SrcType == (unsigned char)SlotType::Equip)
			canMove1 = CanEquip(&newEquipDstSlot, &Mob[clientId].Mobs.Player, p->SrcSlot, Mob[clientId].Mobs.Player.ClassInfo, p, Mob[clientId].Mobs.Player.Equip[0].EFV2);

		else if (p->SrcType == (unsigned char)SlotType::Inv)
		{
			canMove1 = CanCarry(&newEquipDstSlot, Mob[clientId].Mobs.Player.Inventory, p->SrcSlot % 9, p->SrcSlot / 9, &error);

			if (canMove1 == 0 && error > 0 && error <= 64)
			{
				error--;

				SendItem(clientId, SlotType::Inv, error, &Mob[clientId].Mobs.Player.Inventory[error]);
			}
		}

		else if (p->SrcType == (unsigned char)SlotType::Storage)
			canMove1 = CanCargo(&newEquipDstSlot, Users[clientId].User.Storage.Item, (p->SrcSlot % 9), (p->SrcSlot / 9));
	}

	if (newEquipSrcSlot.Index != 0)
	{
		INT32 error = -2; // LOCAL1662
		if (p->DstType == (unsigned char)SlotType::Equip)
			canMove2 = CanEquip(&newEquipSrcSlot, &Mob[clientId].Mobs.Player, p->DstSlot, Mob[clientId].Mobs.Player.ClassInfo, p, Mob[clientId].Mobs.Player.Equip[0].EFV2);

		else if (p->DstType == (unsigned char)SlotType::Inv)
		{
			canMove2 = CanCarry(&newEquipSrcSlot, Mob[clientId].Mobs.Player.Inventory, p->DstSlot % 9, p->DstSlot / 9, &error);

			if (canMove2 == 0 && error > 0 && error < 60)
			{
				error--;

				SendItem(clientId, SlotType::Inv, error, &Mob[clientId].Mobs.Player.Inventory[error]);
			}
		}

		else if (p->DstType == (unsigned char)SlotType::Storage)
			canMove2 = CanCargo(&newEquipSrcSlot, Users[clientId].User.Storage.Item, (p->DstSlot % 9), (p->DstSlot / 9));
	}

	if (canMove1 == 0 || canMove2 == 0)
	{
		memcpy(equipSrcSlot, &newEquipSrcSlot, 8);
		memcpy(equipDstSlot, &newEquipDstSlot, 8);

		return true;
	}
	else
	{
		// Checa se está movendo fora de uma cidade
		if ((p->DstType == (unsigned char)SlotType::Storage || p->SrcType == (unsigned char)SlotType::Storage) && GetVillage(Mob[clientId].Target.X, Mob[clientId].Target.Y) == 5)
		{
			auto posX = Mob[clientId].Target.X;
			auto posY = Mob[clientId].Target.Y;

			bool canMove = false;
			if ((posX >= 2085 && posX <= 2150 && posY >= 2016 && posY <= 2051) ||
				(posX >= 3209 && posX <= 3273 && posY >= 1672 && posY <= 1712))
				canMove = true;

			if (!canMove)
			{
				memcpy(equipSrcSlot, &newEquipSrcSlot, 8);
				memcpy(equipDstSlot, &newEquipDstSlot, 8);

				Log(SERVER_SIDE, LOG_HACK, "[%s] - O usuário moveu o item %s fora da cidade. Posição: %ux %uy", User.Username, equipSrcSlot->toString().c_str(), Mob[clientId].Target.X, Mob[clientId].Target.Y);
				Log(clientId, LOG_HACK, "[%s] - O usuário moveu o item %s fora da cidade. Posição: %ux %uy", User.Username, equipSrcSlot->toString().c_str(), Mob[clientId].Target.X, Mob[clientId].Target.Y);
				return true;
			}
		}

		memcpy(equipSrcSlot, &newEquipDstSlot, 8);
		memcpy(equipDstSlot, &newEquipSrcSlot, 8);

		Log(p->Header.ClientId, LOG_INGAME, "Moveu item %s [%d] [%d %d %d %d %d %d] do tipo %s do slot %d para tipo %s slot %d", ItemList[equipDstSlot->Index].Name, equipDstSlot->Index, equipDstSlot->EF1,
			equipDstSlot->EFV1, equipDstSlot->EF2, equipDstSlot->EFV2, equipDstSlot->EF3, equipDstSlot->EFV3, szSlotType[p->SrcType], p->SrcSlot, szSlotType[p->DstType], p->DstSlot);

		GiveTime(p);
	}

	if (equipSrcSlot->Index <= 40)
		memset(equipSrcSlot, 0, 8);
	if (equipDstSlot->Index <= 40)
		memset(equipDstSlot, 0, 8);

	Mob[clientId].GetCurrentScore(clientId);
	SendScore(clientId);

	AddMessage((BYTE*)p, sizeof p376);

	//434182
	if (Mob[clientId].Mobs.Player.Equip[6].Index == 0 && Mob[clientId].Mobs.Player.Equip[7].Index != 0)
	{
		INT32 itemAbility = GetItemAbility(&Mob[clientId].Mobs.Player.Equip[7], 17); // LOCAL1663

		if (itemAbility != 128)
		{
			p->DstType = 0;
			p->DstSlot = 6;
			p->SrcType = 0;
			p->SrcSlot = 7;

			AddMessage((BYTE*)p, sizeof p376);

			memcpy(&Mob[clientId].Mobs.Player.Equip[6], &Mob[clientId].Mobs.Player.Equip[7], 8);
			memset(&Mob[clientId].Mobs.Player.Equip[7], 0, 8);
		}
	}

	if (needSendScore == 1)
		SendScore(clientId);

	if (needSendEquip == 1)
		SendEquip(clientId);

	MountProcess(clientId, mountSlot);
	return true;
}
