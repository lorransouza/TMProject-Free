#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestReqChallange(PacketHeader *Header)
{
	pMsgSignal *p = (pMsgSignal*)Header;
	
	INT32 value = p->Value;
	if(value <= 0 || value >= MAX_SPAWN_MOB)
		return false;

	INT32 level = Mob[value].Mobs.Player.bStatus.Level;
	if(level < 0 || level >= 5)
		return false;

	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	Log(clientId, LOG_INGAME, "Clicado no NPC de Imposto");

	if (now.tm_wday &&  now.tm_wday != 6)
	{
		// Dia da semana
		char result[32];
		memset(result, 0, 32);

		sprintf_s(result, "%I64d", g_pCityZone[level].impost);

		NumberFormat(result);

		SendSay(value, "Impostos recolhidos: %s", result);
		return true;
	}
		
	if (now.tm_wday == 6)
	{
		// Sábado
		if (level == 4)
			return false;

		return Challange(clientId, value, 100);
	}

	auto owner = ChargedGuildList[sServer.Channel - 1][level];
	if (owner == 0)
		owner = g_pCityZone[level].chall_index_2;

	// Domingo
	char ownerGuildName[32] = { 0 };
	if(owner != 0)
		sprintf_s(ownerGuildName, "%s", g_pGuild[owner].Name.c_str());

	char challengeGuildName[32] = { 0 };
	sprintf_s(challengeGuildName, "%s", g_pGuild[g_pCityZone[level].chall_index].Name.c_str());

	if((sServer.WeekMode == 0 || sServer.WeekMode == 1 || sServer.WeekMode == 2 || sServer.WeekMode == 3))
	{
		if(owner == 0 && g_pCityZone[level].chall_index != 0 && g_pCityZone[level].chall_index_2 == 0)
			SendSay(value, g_pLanguageString[_SN_Winner_Is], challengeGuildName);
		else if(owner != 0 && g_pCityZone[level].chall_index != 0)
			SendSay(value, g_pLanguageString[_SS_Champion_And_Challanger], ownerGuildName, challengeGuildName);
		else 
			SendSay(value, g_pLanguageString[_SN_No_Challanger]);

		return true;
	}
	else if(sServer.WeekMode == 4 || sServer.CastleState == 4)
	{
		// Checa se o cabra é líder da cidade do NPC clicado
		if(ChargedGuildList[sServer.Channel - 1][level] == Mob[clientId].Mobs.Player.GuildIndex && Mob[clientId].Mobs.Player.GuildMemberType == 9)
		{
			Log(clientId, LOG_INGAME, "Clicou no NPC sendo líder de Guild da cidade %d. Imposto: %I64d", level, g_pCityZone[level].impost);

			if((level >= 0 && level <= 3 && sServer.WeekMode == 4) || (level == 4 && sServer.WarChannel == 1 && sServer.CastleState == 4))
			{
				Log(clientId, LOG_INGAME, "O recolhimento da cidade %s feita com sucesso parte 1. Imposto: %I64d", szCitys[level], g_pCityZone[level].impost);

				INT64 gold = g_pCityZone[level].impost;
				if(gold >= 1000000000LL)
				{ // gold maior que 1bi
					// Quantidade de barras que será dado ao usuário
					INT64 bis = gold / 1000000000LL;

					// Quantidade de gold líquido que será dado ao usuário
					gold -= (bis * 1000000000LL);

					if(static_cast<long long>(Mob[clientId].Mobs.Player.Gold) + gold > 2000000000LL)
					{
						SendSay(value, g_pLanguageString[_NN_Cant_get_more_than_2G]);

						Log(clientId, LOG_INGAME, "Falha no recolhimento. Falta de espaço para o gold líquido! Gold atual: %d. Gold liquido: %I64d", Mob[clientId].Mobs.Player.Gold, gold);
						return true;
					}

					if(bis > 59)
					{
						INT64 agrupped = bis / 3LL;
						INT64 alone    = bis % 3LL;
						INT64 null     = GetInventoryAmount(clientId, 0);

						if(null < (agrupped + alone))
						{
							SendSay(value, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);

							Log(clientId, LOG_INGAME, "Falha no recolhimento. Falta de espaço para receber as baras. Agrupadas: %d. Sozinhas: %d. Espaço: %d", agrupped, alone, null);
							return true;
						}
						
						// Entrega as barras agrupadas
						for(INT32 i = 0; i < agrupped; i++)
						{
							INT32 slotId = GetFirstSlot(clientId, 0);
							if(slotId == -1)
								break;

							memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

							Mob[clientId].Mobs.Player.Inventory[slotId].Index = 4011;
							Mob[clientId].Mobs.Player.Inventory[slotId].EF1   = EF_AMOUNT;
							Mob[clientId].Mobs.Player.Inventory[slotId].EFV1  = 3;
						}
						
						// Entrega as barras não agrupadas
						for(INT32 i = 0; i < alone; i++)
						{
							INT32 slotId = GetFirstSlot(clientId, 0);
							if(slotId == -1)
								break;

							memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

							Mob[clientId].Mobs.Player.Inventory[slotId].Index = 4011;
						}
						
						Log(clientId, LOG_GUILD, "Imposto recolhido na cidade %d. Total gold: %I64d. Bis agrupados: %d. Bis soltos: %d.	Gold líquido: %I64d", level, g_pCityZone[level].impost,
							agrupped, alone, gold);
						Log(SERVER_SIDE, LOG_GUILD, "Imposto recolhido na cidade %d por %s. Total gold: %I64d. Bis agrupados: %d. Bis soltos: %d.Gold líquido: %I64d", level, Mob[clientId].Mobs.Player.Name, g_pCityZone[level].impost,
							agrupped, alone, gold);
					}
					else
					{
						INT32 null     = GetInventoryAmount(clientId, 0);
						if(null < bis)
						{
							SendSay(value, g_pLanguageString[_NN_You_Have_No_Space_To_Trade]);
							
							Log(clientId, LOG_INGAME, "Falha no recolhimento. Falta de espaço para receber as baras. Barras: %d Espaço: %d", bis, null);
							return true;
						}

						// Entrega as barras não agrupadas
						for(INT32 i = 0; i < bis; i++)
						{
							INT32 slotId = GetFirstSlot(clientId, 0);
							if(slotId == -1)
								break;

							memset(&Mob[clientId].Mobs.Player.Inventory[slotId], 0, sizeof st_Item);

							Mob[clientId].Mobs.Player.Inventory[slotId].Index = 4011;
						}
						
						Log(clientId, LOG_GUILD, "Imposto recolhido na cidade %d. Total gold: %I64d. Bis soltos: %d.Gold líquido: %I64d", level, g_pCityZone[level].impost,
							bis, gold);
						Log(SERVER_SIDE, LOG_GUILD, "Imposto recolhido na cidade %d por %s. Total gold: %I64d. Bis soltos: %d.Gold líquido: %I64d", level, Mob[clientId].Mobs.Player.Name, g_pCityZone[level].impost,
							bis, gold);
					}

					Mob[clientId].Mobs.Player.Gold += static_cast<int>(gold);
				}
				else
				{
					if(static_cast<long long>(Mob[clientId].Mobs.Player.Gold) + gold > 2000000000LL)
					{
						SendSay(value, g_pLanguageString[_NN_Cant_get_more_than_2G]);
						
						Log(clientId, LOG_INGAME, "Falha no recolhimento. Falta de espaço para o gold líquido! Gold atual: %d. Gold liquido: %I64d", Mob[clientId].Mobs.Player.Gold, gold);
						return true;
					}
					
					Mob[clientId].Mobs.Player.Gold += static_cast<int>(gold);
						
					Log(clientId, LOG_GUILD, "Imposto recolhido na cidade %d. Total gold: %I64d. Gold líquido: %I64d", level, g_pCityZone[level].impost, gold);
					Log(SERVER_SIDE, LOG_GUILD, "Imposto recolhido na cidade %d por %s. Total gold: %I64d. Gold líquido: %I64d", level, Mob[clientId].Mobs.Player.Name, g_pCityZone[level].impost, gold);
				}
				
				// Seta o imposto da cidade como 0 já que o mesmo foi recolhido
				g_pCityZone[level].impost = 0;

				// Seta o estado da guerra de castelos como 0
				// Se chegou até esta parte do código quer dizer que 
				// era possível recolher os impostos de Noatun
				if(level == 4)
					sServer.CastleState = 0;
				
				SendSay(value, "Impostos recolhidos");
				// Atualiza o inventário + gold do inventário
				SendCarry(clientId);
				SendEtc(clientId);
				return true;
			}
			else if(level == 4)
				Log(clientId, LOG_INGAME, "Tentativa de recolher imposto de noatun falhou. NPC: %d. WarChannel: %d. CastleState: %d.", level, sServer.WarChannel, sServer.CastleState);
		}

		if(ChargedGuildList[sServer.Channel - 1][level] != 0)
			SendSay(value, g_pLanguageString[_SN_Winner_Is], ownerGuildName);
		else
			SendSay(value, g_pLanguageString[_SN_No_Challanger]);

		return true;
	}
	else if(sServer.WeekMode == 5)
	{
		SendSignal(clientId, 0, 0x18D);

		return true;
	}

	return true;
}
