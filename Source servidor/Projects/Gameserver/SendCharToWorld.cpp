#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "UOD_EventManager.h"
#include "UOD_ChristmasMission.h"

bool CUser::SendCharToWorld(PacketHeader* Header)
{
	p114* LOCAL_215 = (p114*)Header;
	INT32 LOCAL_216 = 0;
	
	auto fixCostume = [](int clientId, st_Item* item) 
	{

		bool isUsingCostume = (item->Index >= 4151 && item->Index <= 4189) || (item->Index >= 4210 && item->Index <= 4229) || (item->Index >= 4230 && item->Index <= 4241);

		if (isUsingCostume && item->EF1 == 106 && item->EF2 == 110 && item->EF3 == 109)
		{
			Log(clientId, LOG_INGAME, "Removido os adicionais de tempo do traje %s %s", ItemList[item->Index].Name, item->toString().c_str());

			item->EF1 = 0;
			item->EF2 = 0;
			item->EF3 = 0;
		}
	};

	auto fixBoots = [](int clientId, st_Item* item) 
	{
		auto temporaryItem = *item;
		int totalDamage = GetItemAbilityNoSanc(item, EF_DAMAGE) + GetItemAbilityNoSanc(item, EF_DAMAGE2);

		int maxDamage = 30;
		if (GetItemAbility(item, EF_MOBTYPE) == 1)
			maxDamage = 42;

		if (totalDamage > maxDamage)
		{
			int i = 0;
			for (; i < 3; ++i)
			{
				if (item->Effect[i].Index == EF_DAMAGE)
				{
					item->Effect[i].Index = EF_DAMAGE2;
					item->Effect[i].Value = maxDamage;

					break;
				}
			}

			if (i != 3)
				Log(clientId, LOG_INGAME, "O item %s %s teve seu adicionaal corrigido. Adicional anterior: %d. Informações do item: %s", ItemList[item->Index].Name, item->toString().c_str(), totalDamage, temporaryItem.toString().c_str());

			return i != 3;
		}

		return false;
	};

	for (; LOCAL_216 < 18; LOCAL_216++)
	{
		st_Item* LOCAL_217 = &LOCAL_215->Mob.Equip[LOCAL_216];
		if (LOCAL_217->Index <= 0 || LOCAL_217->Index > MAX_ITEMLIST)
			continue;

		INT32 LOCAL_218 = ItemList[LOCAL_217->Index].Pos;
		if (LOCAL_218 == 64 || LOCAL_218 == 192)
		{
			if (LOCAL_217->Effect[0].Index == EF_DAMAGE2 || LOCAL_217->Effect[0].Index == EF_DAMAGEADD)
				LOCAL_217->Effect[0].Index = EF_DAMAGE;

			if (LOCAL_217->Effect[1].Index == EF_DAMAGE2 || LOCAL_217->Effect[1].Index == EF_DAMAGEADD)
				LOCAL_217->Effect[1].Index = EF_DAMAGE;

			if (LOCAL_217->Effect[2].Index == EF_DAMAGE2 || LOCAL_217->Effect[2].Index == EF_DAMAGEADD)
				LOCAL_217->Effect[2].Index = EF_DAMAGE;
		}

		if (LOCAL_218 <= 32)
		{ // Armaduras
			if (LOCAL_217->Effect[0].Index == EF_CRITICAL)
			{
				LOCAL_217->Effect[0].Index = EF_CRITICAL2;

				INT32 value = LOCAL_217->Effect[0].Value + GetEffectValueByIndex(LOCAL_217->Index, EF_CRITICAL);
				LOCAL_217->Effect[0].Value = value;
			}
			if (LOCAL_217->Effect[1].Index == EF_CRITICAL)
			{
				LOCAL_217->Effect[1].Index = EF_CRITICAL2;

				INT32 value = LOCAL_217->Effect[1].Value + GetEffectValueByIndex(LOCAL_217->Index, EF_CRITICAL);
				LOCAL_217->Effect[1].Value = value;
			}

			if (LOCAL_217->Effect[2].Index == EF_CRITICAL)
			{
				LOCAL_217->Effect[2].Index = EF_CRITICAL2;

				INT32 value = LOCAL_217->Effect[2].Value + GetEffectValueByIndex(LOCAL_217->Index, EF_CRITICAL);
				LOCAL_217->Effect[2].Value = value;
			}
		}

		bool isUsingCostume = (LOCAL_217->Index >= 4151 && LOCAL_217->Index <= 4189) || (LOCAL_217->Index >= 4210 && LOCAL_217->Index <= 4229) || (LOCAL_217->Index >= 4230 && LOCAL_217->Index <= 4241);
		if ((LOCAL_217->Index >= 3980 && LOCAL_217->Index <= 3999) || isUsingCostume)
		{
			if ((LOCAL_217->EF1 == 106 && LOCAL_217->EF2 == 110 && LOCAL_217->EF3 == 109) || (isUsingCostume && LOCAL_217->EFV1 != 0 && LOCAL_217->EFV2 != 0 && LOCAL_217->EFV3 != 0))
			{
				float difDays = TimeRemaining(LOCAL_217->EFV1, LOCAL_217->EFV2, (LOCAL_217->EFV3 + 2000));
				if (difDays <= 0.0f)
				{
					SendClientMessage(clientId, "Esfera / Traje expirou...");
					Log(clientId, LOG_INGAME, "Traje / Esfera expirado. Expira em: %d/%d/%d", LOCAL_217->EFV1, LOCAL_217->EFV2, LOCAL_217->EFV3);

					memset(LOCAL_217, 0, sizeof st_Item);
					continue;
				}
			}
		}

		fixCostume(clientId, LOCAL_217);

		if (LOCAL_218 == 32)
			fixBoots(clientId, LOCAL_217);
	}

	for (LOCAL_216 = 0; LOCAL_216 < 64; LOCAL_216++)
	{
		st_Item* LOCAL_219 = &LOCAL_215->Mob.Inventory[LOCAL_216];
		if (LOCAL_219->Index <= 0 || LOCAL_219->Index >= MAX_ITEMLIST)
			continue;

		INT32 LOCAL_220 = ItemList[LOCAL_219->Index].Pos;
		if (LOCAL_220 == 64 || LOCAL_220 == 192)
		{
			if (LOCAL_219->Effect[0].Index == EF_DAMAGE2 || LOCAL_219->Effect[0].Index == EF_DAMAGEADD)
				LOCAL_219->Effect[0].Index = EF_DAMAGE;

			if (LOCAL_219->Effect[1].Index == EF_DAMAGE2 || LOCAL_219->Effect[1].Index == EF_DAMAGEADD)
				LOCAL_219->Effect[1].Index = EF_DAMAGE;

			if (LOCAL_219->Effect[2].Index == EF_DAMAGE2 || LOCAL_219->Effect[2].Index == EF_DAMAGEADD)
				LOCAL_219->Effect[2].Index = EF_DAMAGE;
		}

		if (LOCAL_220 <= 32)
		{ // Armaduras
			if (LOCAL_219->Effect[0].Index == EF_CRITICAL)
			{
				LOCAL_219->Effect[0].Index = EF_CRITICAL2;

				INT32 value = LOCAL_219->Effect[0].Value + GetEffectValueByIndex(LOCAL_219->Index, EF_CRITICAL);
				LOCAL_219->Effect[0].Value = value;
			}
			if (LOCAL_219->Effect[1].Index == EF_CRITICAL)
			{
				LOCAL_219->Effect[1].Index = EF_CRITICAL2;

				INT32 value = LOCAL_219->Effect[1].Value + GetEffectValueByIndex(LOCAL_219->Index, EF_CRITICAL);
				LOCAL_219->Effect[1].Value = value;
			}

			if (LOCAL_219->Effect[2].Index == EF_CRITICAL)
			{
				LOCAL_219->Effect[2].Index = EF_CRITICAL2;

				INT32 value = LOCAL_219->Effect[2].Value + GetEffectValueByIndex(LOCAL_219->Index, EF_CRITICAL);
				LOCAL_219->Effect[2].Value = value;
			}
		}

		if (LOCAL_220 == 32)
			fixBoots(clientId, LOCAL_219);

		fixCostume(clientId, LOCAL_219);
	}

	for (int i = 0; i < 120; ++i)
	{
		auto item = &User.Storage.Item[i];
		if (item->Index <= 0 || item->Index >= MAX_ITEMLIST || ItemList[item->Index].Pos != 32)
			continue;

		if (fixBoots(clientId, item))
			SendItem(clientId, SlotType::Storage, i, item);
	}

	// 0044D940
	if (true) // evOn
	{
		for (INT32 LOCAL_221 = 0; LOCAL_221 < 64; LOCAL_221++)
		{
			if (LOCAL_215->Mob.Inventory[LOCAL_221].Index == 470 || LOCAL_215->Mob.Inventory[LOCAL_221].Index == 500)
				LOCAL_215->Mob.Inventory[LOCAL_221].Index = 0;
		}
	}

	// Quando troca de servidor já seta como true
	// para não dar problema ao dar personagem
	// Só é setado quando é realmente enviado para o game,
	// então não tem problema
	TokenOk = true;

	Mob[clientId].Mobs.Player = LOCAL_215->Mob;
	Mob[clientId].Mode = 2;

	LOCAL_215->Mob.Last.X = LOCAL_215->Mob.Last.X + (Rand() % 5) - 2;
	LOCAL_215->Mob.Last.Y = LOCAL_215->Mob.Last.Y + (Rand() % 5) - 2;

	LOCAL_215->Header.PacketId = 0x114;
	LOCAL_215->Header.ClientId = 0x7531;

	int cityId = Mob[clientId].Mobs.Player.Info.CityID;
	LOCAL_215->WorldPos.X = g_pCityZone[cityId].city_x + Rand() % 10;
	LOCAL_215->WorldPos.Y = g_pCityZone[cityId].city_y + Rand() % 10;

	//0044D885
	LOCAL_215->ClientIndex = clientId;

	Mob[clientId].Last.Time = CurrentTime;
	Mob[clientId].Target.X = LOCAL_215->Mob.Last.X;
	Mob[clientId].Last.X = Mob[clientId].Target.X;

	Mob[clientId].Target.Y = LOCAL_215->Mob.Last.Y;
	Mob[clientId].Last.Y = Mob[clientId].Target.Y;

	LOCAL_215->Mob.Equip[0].EFV2 = Mob[clientId].Mobs.Player.Equip[0].EFV2;

	Mob[clientId].clientId = clientId;

	Mob[clientId].Mobs.Player.Equip[0].EFV2 = Mob[clientId].Mobs.Player.Equip[0].EFV2;
	LOCAL_215->Mob.Equip[0].EFV2 = Mob[clientId].Mobs.Player.Equip[0].EFV2;

	if (Mob[clientId].Mobs.Player.Equip[0].EFV2 >= 3)
	{
		if (!(Mob[clientId].Mobs.Player.Learn[0] & 0x40000000ull))
			Mob[clientId].Mobs.Player.Learn[0] |= 0x40000000ull;
	}

	// 0044DC30
	if (!Mob[clientId].Mobs.Player.Inventory[63].Index)
	{
		memset(&Mob[clientId].Mobs.Player.Inventory[63], 0, 8);

		Mob[clientId].Mobs.Player.Inventory[63].Index = 547;

		Mob[clientId].Mobs.Player.Inventory[63].EF1 = 43;
		Mob[clientId].Mobs.Player.Inventory[63].EF2 = 76;
		Mob[clientId].Mobs.Player.Inventory[63].EF3 = 77;

		SetPKPoint(clientId, 75);
	}

	Users[clientId].Challenger.Mode = 0;
	Users[clientId].LastWhisper = 0;
	Users[clientId].Socket.Error = 0;
	inGame.CharSlot = static_cast<char>(LOCAL_215->	SlotIndex);

	{
		auto now = std::chrono::steady_clock::now();
		Users[clientId].MacIntegrity.loginTime = now;
		Times.LastDeletedItem = now;
		Times.LastUsedItem = now;
	}

	Users[clientId].User.CharSlot = inGame.CharSlot;

	Users[clientId].CrackCount = 0;
	//Users[clientId].Unknow_1796 = 0;
	Users[clientId].Movement.PacketId = 0x366;
	Users[clientId].Movement.TimeStamp = 0xE0A1ACA;
	//Users[clientId].Attack.LastType = 0;
	//Users[clientId].Attack.TimeStamp = 0xE0A1ACA;
	//Users[clientId].IlussionTime = 0xE0A1ACA;
	//Users[clientId].Challanger.Index = 0;
	//Users[clientId].Challanger.ClassId = 0;

	//*(DWORD*)&Users[clientId].Unknow_2744[0] = 0;
	//memset(&Users[clientId].Unknow[4], -1, 400);

	if (Mob[clientId].Mobs.Player.Status.curHP <= 0)
		Mob[clientId].Mobs.Player.Status.curHP = 2;

	memset(&Users[clientId].AutoTrade, 0, sizeof Users[clientId].AutoTrade);

	INT32 LOCAL_222 = 0;
	for (; LOCAL_222 < 15; LOCAL_222++)
		Users[clientId].Trade.Slot[LOCAL_222] = -1;

	for (LOCAL_222 = 0; LOCAL_222 < 12; LOCAL_222++)
		Users[clientId].AutoTrade.Slots[LOCAL_222] = -1;

	IsAutoTrading = 0;

	Mob[clientId].GuildDisable = 0;
	AllStatus.PK = 0;
	AllStatus.Chat = 0;
	AllStatus.Citizen = 0;
	AllStatus.Guild = 0;
	AllStatus.Kingdom = 0;
	AllStatus.Whisper = 0;

	UINT32 LOCAL_223 = LOCAL_215->WorldPos.X;
	UINT32 LOCAL_224 = LOCAL_215->WorldPos.Y;

	INT32 LOCAL_225 = Mob[clientId].Mobs.Player.Info.CityID;

	//0044DEBB
	LOCAL_223 = g_pCityZone[LOCAL_225].city_x + (Rand() % 14);
	LOCAL_224 = g_pCityZone[LOCAL_225].city_y + (Rand() % 14);

	INT32 LOCAL_226 = Mob[clientId].Mobs.Player.GuildIndex;
	INT32 LOCAL_227 = Mob[clientId].Mobs.Player.ClassInfo;

	if (LOCAL_227 < 0 || LOCAL_227 > 3)
	{
		//TODO :  error, login undefined class
		Log(clientId, LOG_INGAME, "login undefined class %d", LOCAL_227);
		CloseUser(clientId);
		return false;
	}

	for (INT32 LOCAL_228 = 0; LOCAL_228 < 5; LOCAL_228++)
	{
		if (LOCAL_226 != 0 && LOCAL_226 == ChargedGuildList[sServer.Channel - 1][LOCAL_228])
		{
			LOCAL_223 = g_pCityZone[LOCAL_228].area_guild_x;
			LOCAL_224 = g_pCityZone[LOCAL_228].area_guild_y;

			break;
		}
	}

	if (Mob[clientId].Mobs.Player.Equip[0].EFV2 == MORTAL && Mob[clientId].Mobs.Player.bStatus.Level < sServer.NewbieZone)
	{ // Área de treinamento
		LOCAL_223 = 2112;
		LOCAL_224 = 2042;
	}

	INT32 LOCAL_229 = GetEmptyMobGrid(clientId, &LOCAL_223, &LOCAL_224);
	if (LOCAL_229 == 0)
	{
		// TODO : Error can't start can't get mobgrid
		Log(clientId, LOG_INGAME, "Não encontrado espaço no grid em %dx %dy", LOCAL_223, LOCAL_224);

		CloseUser(clientId);
		return false;
	}

	LOCAL_215->WorldPos.X = LOCAL_223;
	LOCAL_215->WorldPos.Y = LOCAL_224;

	Mob[clientId].Target.X = LOCAL_223;
	Mob[clientId].Last.X = Mob[clientId].Target.X;

	Mob[clientId].Target.Y = LOCAL_224;
	Mob[clientId].Last.Y = Mob[clientId].Target.Y;

	this->Status = USER_PLAY;

	Users[clientId].nTargetX = 0;
	Users[clientId].nTargetY = 0;
	Users[clientId].Trade.ClientId = 0;
	Mob[clientId].Lifes = 0;

	Mob[clientId].GetCurrentScore(clientId);

	lastLogEquipAndInventory = std::chrono::high_resolution_clock::now();
	LogEquipsAndInventory(true);

	//*(DWORD*)&LOCAL_215->unknow[0] = Users[clientId].inGame.MagicIncrement;

	AddMessage((BYTE*)LOCAL_215, 1712);

	RequestUpdateFriendList(clientId);

	int christmasStatus = 2;
	if (Christmas.Mission.Status == TOD_ChristmasMission_Status::Accepted)
		christmasStatus = 1;
	else if(Christmas.Mission.Status == TOD_ChristmasMission_Status::WaitingAnswer)
		christmasStatus = 0;

	SendChristmasMission(clientId, christmasStatus);

	Users[clientId].AttackCount = 0;

	Users[clientId].GoldCount = 0;
	Users[clientId].Gold = 0;

	Users[clientId].PremierStore.Status = 0;
	Users[clientId].PremierStore.Time = 0;
	Users[clientId].PremierStore.Count = 0;
	Users[clientId].PremierStore.Wait = 0;

	Users[clientId].aHack.Question = -1;
	Users[clientId].aHack.Response = 0;
	Users[clientId].aHack.Error = 0;
	Users[clientId].aHack.Next = 60;
	Users[clientId].aHack.Last = sServer.SecCounter;

	SummonedUser = 0;
	Users[clientId].Potion.CountHp = Mob[clientId].Mobs.Player.Status.curHP;
	Users[clientId].Potion.CountMp = Mob[clientId].Mobs.Player.Status.curMP;
	Users[clientId].Potion.bQuaff = 0;

	Users[clientId].Damage.TorreErion = 0;

	Mob[clientId].SpawnType = 2;

	p364 LOCAL_256{};
	GetCreateMob(clientId, (BYTE*)&LOCAL_256);
	Mob[clientId].SpawnType = 0;

	g_pMobGrid[LOCAL_215->WorldPos.Y][LOCAL_215->WorldPos.X] = clientId;

	if (!Users[clientId].IsBanned)
		GridMulticast_2(LOCAL_215->WorldPos.X, LOCAL_215->WorldPos.Y, (BYTE*)& LOCAL_256, 0);
	else
		AddMessage(reinterpret_cast<BYTE*>(&LOCAL_256), sizeof p364);

	SendMissionInfo(clientId);
	SendDailyRewardInfo(clientId);
	SendGridMob(clientId);
	SendScore(clientId);
	SendEtc(clientId);
	SendAutoPartyInfo(clientId);

	Log(clientId, LOG_INGAME, "Personagem %s enviado ao mundo %dx %dy - Canal %d - Cash da conta: %d. OverPower: %d", LOCAL_215->Mob.Name, LOCAL_215->WorldPos.X, LOCAL_215->WorldPos.Y, sServer.Channel, User.Cash, GetOverPower(Mob[clientId]));
	Log(clientId, LOG_INGAME, "Gold atual: %d. Classe: %d Evolução: %d. Experiência: %I64d. Nível: %d. Info: %I64d. GuildID: %d. CP: %d. Pontos de Contribuição: %d. Limite: %d", LOCAL_215->Mob.Gold, Mob[clientId].Mobs.Player.ClassInfo, Mob[clientId].Mobs.Player.Equip[0].EFV2, LOCAL_215->Mob.Exp,
		LOCAL_215->Mob.bStatus.Level, Mob[clientId].Mobs.Info.Value, Mob[clientId].Mobs.Player.GuildIndex, GetPKPoint(clientId) - 75, Mob[clientId].Mobs.Contribuition.Points, Mob[clientId].Mobs.Contribuition.Limit);

	LogPlayer(clientId, "Entrou no jogo com o personagem %s", LOCAL_215->Mob.Name);
	
	Mob[clientId].Jewel = -1;

	for (int i = 0; i < 32; i++)
	{
		if (Mob[clientId].Mobs.Affects[i].Index == 0)
			continue;

		if (Mob[clientId].Mobs.Affects[i].Index == 30)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com frango ativo. Tempo: %d", LOCAL_215->Mob.Name, Mob[clientId].Mobs.Affects[i].Time);

		else if (Mob[clientId].Mobs.Affects[i].Index == 39)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com baú de experiência ativo. Tempo: %d", LOCAL_215->Mob.Name, Mob[clientId].Mobs.Affects[i].Time);

		else if (Mob[clientId].Mobs.Affects[i].Index == 34)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poção divina ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.Name, Users[clientId].User.Divina.Dia,
				Users[clientId].User.Divina.Mes, Users[clientId].User.Divina.Ano, Users[clientId].User.Divina.Hora, Users[clientId].User.Divina.Minuto, Users[clientId].User.Divina.Segundo);

		else if (Mob[clientId].Mobs.Affects[i].Index == 51)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poção revigorante ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.Name, Mob[clientId].Mobs.Revigorante.Dia,
				Mob[clientId].Mobs.Revigorante.Mes, Mob[clientId].Mobs.Revigorante.Ano, Mob[clientId].Mobs.Revigorante.Hora, Mob[clientId].Mobs.Revigorante.Minuto, Mob[clientId].Mobs.Revigorante.Segundo);

		else if (Mob[clientId].Mobs.Affects[i].Index == 4)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poção sephira ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.Name, Users[clientId].User.Sephira.Dia,
				Users[clientId].User.Sephira.Mes, Users[clientId].User.Sephira.Ano, Users[clientId].User.Sephira.Hora, Users[clientId].User.Sephira.Minuto, Users[clientId].User.Sephira.Segundo);

		else if (Mob[clientId].Mobs.Affects[i].Index == 35)
			Log(clientId, LOG_INGAME, "Logou no personagem %s com poção saúde ativo. Validade: %d/%d/%d %d:%d:%d", LOCAL_215->Mob.Name, Mob[clientId].Mobs.Saude.Dia,
				Mob[clientId].Mobs.Saude.Mes, Mob[clientId].Mobs.Saude.Ano, Mob[clientId].Mobs.Saude.Hora, Mob[clientId].Mobs.Saude.Minuto, Mob[clientId].Mobs.Saude.Segundo);

		else if (Mob[clientId].Mobs.Affects[i].Index == 8)
			Mob[clientId].Jewel = i;
	}

	SendWarInfo(clientId, sServer.CapeWin);

	if(sServer.CastleState)
		SendSignalParm(clientId, 0x7530, 0x3AC, 1);

	SendSignalParm(clientId, 0x7530, RefreshGoldPacket, User.Cash);

	ClearCrown(clientId);

	if (Mob[clientId].Mobs.Player.Equip[6].Index == 877)
	{
		memset(&Mob[clientId].Mobs.Player.Equip[6], 0, sizeof st_Item);

		SendItem(clientId, SlotType::Equip, 6, &Mob[clientId].Mobs.Player.Equip[6]);
	}
		
	RemoveParty(clientId);

	MountProcess(clientId, 0);

	INT32 guildId = Mob[clientId].Mobs.Player.GuildIndex;
	if(guildId != 0)
	{
		INT32 capeInfo = Mob[clientId].Mobs.Player.CapeInfo;
		if(capeInfo != g_pGuild[guildId].Kingdom)
		{
			int capeId = Mob[clientId].Mobs.Player.Equip[15].Index;
			bool isWhite = std::find(std::begin(g_pCapesID[2]), std::end(g_pCapesID[2]), capeId) != std::end(g_pCapesID[2]);
			unsigned int _index = capeInfo - CAPE_BLUE;
			unsigned int otherIndex = g_pGuild[guildId].Kingdom - CAPE_BLUE;

			if (isWhite)
				_index = 2;

			if ((isWhite || capeId != 4006) && _index >= 0 && _index < g_pCapesID.size() && otherIndex >= 0 && otherIndex <= g_pCapesID.size())
			{
				for (INT32 i = 0; i < 10; i++)
				{
					if (capeId == g_pCapesID[_index][i])
					{
						Mob[clientId].Mobs.Player.Equip[15].Index = g_pCapesID[otherIndex][i];

						Log(clientId, LOG_INGAME, "Trocado a capa %d para %hu devido a guild ter reino diferente", capeId, Mob[clientId].Mobs.Player.Equip[15].Index);
						SendItem(clientId, SlotType::Equip, 15, &Mob[clientId].Mobs.Player.Equip[15]);
						break;
					}
				}
			}
		}
	}

    if (Mob[clientId].Mobs.Player.GetEvolution() == TOD_Class::SubCelestial && Mob[clientId].Mobs.GetTotalResets() == 3 && Mob[clientId].Mobs.Info.Unlock200)
    {
        if (!Mob[clientId].Mobs.Sub.Info.Unlock200)
        {
            Mob[clientId].Mobs.Sub.Info.Unlock200 = 1;
            Log(clientId, LOG_INGAME, "Desbloqueado o nível 200 do personagem Celestial armazenado na Pedra Misteriosa");
        }
    }

	if (Mob[clientId].Mobs.Player.GetEvolution() >= Celestial && Mob[clientId].Mobs.Player.bStatus.Level == 199 && Mob[clientId].Mobs.Player.bStatus.Level < 399 && !Mob[clientId].Mobs.Info.Unlock200 && !Mob[clientId].Mobs.Info.LvBlocked && Mob[clientId].Mobs.GetTotalResets() == 3)
	{
		Log(clientId, LOG_INGAME, "Bloqueado o nível 200 do personagem devido a não estar bloqueado e não ter desbloqueado.");

		Mob[clientId].Mobs.Info.LvBlocked = 1;
		Mob[clientId].Mobs.Player.Exp = g_pNextLevel[Mob[clientId].Mobs.Player.Equip[0].EFV2][Mob[clientId].Mobs.Player.bStatus.Level - 1];

		SendEtc(clientId);
	}

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	if (now.tm_yday != User.Water.Day)
	{
		Log(clientId, LOG_INGAME, "Resetado a contagem de entradas água! Day: %d-%d. Total: %d", User.Water.Day, now.tm_yday, User.Water.Total);
		LogPlayer(clientId, "Resetado a contagem de entradas água");

		User.Water.Day = now.tm_yday;
		User.Water.Total = 0;
	}

	if (Mob[clientId].Mobs.Player.Inventory[60].Index == 3467 && !Mob[clientId].isBagActive(TOD_Bag::FirstBag))
	{
		Log(clientId, LOG_INGAME, "Bolsa do Andarilho (1) expirou. %s", Mob[clientId].Mobs.Player.Inventory[60].toString().c_str());

		memset(&Mob[clientId].Mobs.Player.Inventory[60], 0, sizeof st_Item);
		SendItem(clientId, SlotType::Inv, 60, &Mob[clientId].Mobs.Player.Inventory[60]);
	}

	if (Mob[clientId].Mobs.Player.Inventory[61].Index == 3467 && !Mob[clientId].isBagActive(TOD_Bag::SecondBag))
	{
		Log(clientId, LOG_INGAME, "Bolsa do Andarilho (2) expirou. %s", Mob[clientId].Mobs.Player.Inventory[61].toString().c_str());

		memset(&Mob[clientId].Mobs.Player.Inventory[61], 0, sizeof st_Item);
		SendItem(clientId, SlotType::Inv, 61, &Mob[clientId].Mobs.Player.Inventory[61]);
	}

	if(!User.Block.Pass[0])
		SendSignal(clientId, clientId, 0x904);
	
	if(sServer.KefraKiller != 0)
		SendClientMessage(clientId, g_pLanguageString[_NN_Kefra_GuildKill], g_pGuild[sServer.KefraKiller].Name.c_str());
	else if(sServer.KefraKiller == 0 && sServer.KefraDead)
		SendClientMessage(clientId, g_pLanguageString[_NN_Kefra_PlayerKill]);

	if (sServer.KingdomBattle.Winner == CAPE_BLUE || sServer.KingdomBattle.Winner == CAPE_RED)
		SendClientMessage(clientId, "O Reino %s é o atual dominante", sServer.KingdomBattle.Winner == CAPE_BLUE ? "Blue" : "Red");

	if(g_pGuildNotice[guildId][0])
		SendChatGuild(clientId, guildId, "--Aviso: %s", g_pGuildNotice[guildId]);

	SendKingdomBattleInfo(clientId, CAPE_BLUE, sServer.KingdomBattle.Info[0].Status);
	SendKingdomBattleInfo(clientId, CAPE_RED, sServer.KingdomBattle.Info[1].Status);

	if(!User.Unique.Novato)
		SendSignal(clientId, SERVER_SIDE, WelcomePacket);

	if(sServer.PromotionStatus)
		SendSignal(clientId, SERVER_SIDE, PromotionPacket);

	auto& mob = Mob[clientId].Mobs;
	if (mob.Player.Equip[0].EFV2 >= CELESTIAL)
	{
		if (mob.Sub.Status == 1 || mob.Player.Equip[0].EFV2 == SUBCELESTIAL)
		{
			st_Item& capeOne = mob.Player.Equip[15];
			st_Item& capeTwo = mob.Sub.Equip[1];

			Log(clientId, LOG_INGAME, "Evolução logada: %d", (int)mob.Player.Equip[0].EFV2);
			Log(clientId, LOG_INGAME, "Dados da capa um: %s", capeOne.toString().c_str());
			Log(clientId, LOG_INGAME, "Dados da capa dois: %s", capeTwo.toString().c_str());

			if (GetItemSanc(&capeTwo) > GetItemSanc(&capeOne))
			{
				Log(clientId, LOG_INGAME, "Trocando para capa dois pois a refinação é maior.");
				capeOne = capeTwo;

				SendItem(clientId, SlotType::Equip, 15, &capeOne);
			}
			else
				Log(clientId, LOG_INGAME, "Não foi efetuado a troca pois a refinação atual é maior");
		}
	}

	{
		int totalDivinas = 0;
		int totalSephiras = 0;
		for (int i = 0; i < 120; ++i)
		{
			st_Item* item = &User.Storage.Item[i];
			if (item->Index == 3379)
				totalDivinas++;
			else if (item->Index == 3361)
				totalSephiras++;
		}

		for (int i = 0; i < 60; ++i)
		{
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[i];
			if (item->Index == 3379)
				totalDivinas++;
			else if (item->Index == 3361)
				totalSephiras++;
		}

		if (totalDivinas >= 2 || totalSephiras >= 2)
		{
			Log(clientId, LOG_INGAME, "Foram encontrados um total de %d Divinas e %d Sephiras", totalDivinas, totalSephiras);

			for (int i = 0; i < 120; ++i)
			{
				st_Item* item = &User.Storage.Item[i];
				if (item->Index == 3379 && totalDivinas > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (banco) do slot %d", item->toString().c_str(), i);

					*item = st_Item{};
					SendItem(clientId, SlotType::Storage, i, item);
					totalDivinas--;
				}
				else if (item->Index == 3361 && totalSephiras > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (banco) do slot %d", item->toString().c_str(), i);
					*item = st_Item{};
					SendItem(clientId, SlotType::Storage, i, item);

					totalSephiras--;
				}
				else if (item->Index == 3379 || item->Index == 3361)
				{
					item->Effect[0].Index = EF_NOTRADE;
					item->Effect[0].Value = 1;

					SendItem(clientId, SlotType::Storage, i, item);
				}
			}

			for (int i = 0; i < 60; ++i)
			{
				st_Item* item = &Mob[clientId].Mobs.Player.Inventory[i];
				if (item->Index == 3379 && totalDivinas > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (inventário) do slot %d", item->toString().c_str(), i);

					*item = st_Item{};
					SendItem(clientId, SlotType::Inv, i, item);
					totalDivinas--;
				}
				else if (item->Index == 3361 && totalSephiras > 1)
				{
					Log(clientId, LOG_INGAME, "Removido %s (inventário) do slot %d", item->toString().c_str(), i);
					*item = st_Item{};
					SendItem(clientId, SlotType::Inv, i, item);

					totalSephiras--;
				}
				else if (item->Index == 3379 || item->Index == 3361)
				{
					item->Effect[0].Index = EF_NOTRADE;
					item->Effect[0].Value = 1;

					SendItem(clientId, SlotType::Inv, i, item);
				}
			}
		}
	}

	std::string playerName{ mob.Player.Name };
	for (int i = 0; i < 3; ++i)
	{
		const auto& nightmare = sServer.Nightmare[i];
		if(std::find(std::begin(nightmare.MembersName), std::end(nightmare.MembersName), playerName) == std::end(nightmare.MembersName))
			continue;

		SendSignalParm2(clientId, SERVER_SIDE, NightmareCanEnterWarnPacket, i, nightmare.TimeLeft);
	}

	SendLojaDonate(clientId);//aqui
	SendClientPacket(clientId);//aqui

//#if defined(_DEBUG)
//	IsAdmin = true;
//	AccessLevel = 100;
//#endif

	return true;
}