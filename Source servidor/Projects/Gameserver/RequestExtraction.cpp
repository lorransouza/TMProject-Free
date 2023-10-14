#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestExtraction(PacketHeader *Header)
{
	p2D4 *p = (p2D4*)(Header);

	INT32 slotId = p->slotId;
	if(slotId < 0 || slotId >= 64)
		return true;

	st_Item *item = &Mob[clientId].Mobs.Player.Inventory[slotId];
	if(item->Index <= 0 || item->Index >= MAX_ITEMLIST)
		return true;

	INT32 unique = ItemList[item->Index].Pos;
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

	if(unique < 3021 || unique >= 3026)
	{
		SendClientMessage(clientId, "Só é possível utilizar em equipamentos.");

		return true;
	}
	
	INT32 wiseStone = GetFirstSlot(clientId, 1774);
	if(wiseStone == -1)
	{
		SendClientMessage(clientId, "Para realizar a composição é necessário [01] Pedra do Sábio");

		return true;
	}
	
	if(ItemList[item->Index].Unique >= 41 && ItemList[item->Index].Unique <= 49)
	{
		if(ItemList[item->Index].Grade >= 5 && ItemList[item->Index].Grade <= 8)
		{
			SendClientMessage(clientId, "Não é possível usar em itens ancts");

			return true;
		}
	}
	
	if(GetItemSanc(item) > 9)
	{
		SendClientMessage(p->Header.ClientId, "Não é possível tirar extração de itens acima de +9");

		return true;
	}

	if (IsImpossibleToRefine(item))
	{
		SendClientMessage(clientId, "Item impossível de refinar");

		return true;
	}

	INT32 _rand  = Rand() % 100;
	INT32 chance = 3 + (Mob[clientId].Mobs.Player.Status.Mastery[2] * 2 / 11);
	if ((Mob[clientId].Mobs.Player.Learn[0] & 0x8000))
		chance += 10;

	if(_rand > chance)
	{
		Log(clientId, LOG_INGAME, "EXTRAÇÃO - Destruiu o item %s [%d] [%d %d %d %d %d %d] - %d/%d", ItemList[item->Index].Name,
			item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, _rand, chance);

		memset(item, 0, sizeof item);
		memset(&Mob[clientId].Mobs.Player.Inventory[wiseStone], 0, sizeof st_Item);

		SendItem(clientId, SlotType::Inv, slotId, item);
		SendItem(clientId, SlotType::Inv, wiseStone, &Mob[clientId].Mobs.Player.Inventory[wiseStone]);

		SendClientMessage(clientId, "Item destruído");
		return true;
	}
	
	Log(clientId, LOG_INGAME, "EXTRAÇÃO - Usado no item %s [%d] [%d %d %d %d %d %d] - %d/%d", ItemList[item->Index].Name,
		item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, _rand, chance);

	BOOL success = false;
	for(int i = 0; i < 3;i++)
	{
		if(item->Effect[i].Index == 43 || (item->Effect[i].Index >= 116 && item->Effect[i].Index <= 125))
		{
			int value   = GetEffectValueByIndex(item->Index, EF_UNKNOW1);
			int mobtype = GetEffectValueByIndex(item->Index, EF_MOBTYPE);

			int sanc = GetItemSanc(item);
			if(value <= 5 && mobtype == 0)
			{ // Itens <= [E] e é item mortal
				value = value;
			}
			
			if(value == 0)
			{
				SendClientMessage(clientId, "Necessário que o item possua alguma classe ([A],[B],[C] ....)");
				
				return true;
			}

			// Cálculo realizado - Item entregue
			item->Effect[i].Index = 87;
			item->Effect[i].Value = value;
			success = true;
			break;
		}
	}

	if(!success)
	{
		for(int i = 0; i < 3; i++)
		{
			if(item->Effect[i].Index == 0)
			{
				int value   = GetEffectValueByIndex(item->Index, EF_UNKNOW1);
				int mobtype = GetEffectValueByIndex(item->Index, EF_MOBTYPE);
				int sanc    = GetItemSanc(item);
				if(value <= 5 && mobtype == 0)
				{ // Itens <= [E] e é item mortal
					value = value;
				}

				if(value == 0)
				{
					SendClientMessage(clientId, "Necessário que o item possua alguma classe ([A],[B],[C] ....)");

					return true;
				}

				// Cálculo realizado - Item entregue
				item->Effect[i].Index = 87;
				item->Effect[i].Value = value;
				success = true;
				break;
			}
		}
	}


	if(!success)
	{
		SendClientMessage(clientId, "Item impossível de refinar");
		return true;
	}
	
	if(item->Effect[1].Index == EF_DAMAGE)
		item->Effect[1].Value += GetStaticItemAbility(item, EF_DAMAGE);
	
	if(item->Effect[2].Index == EF_DAMAGE)
		item->Effect[2].Value += GetStaticItemAbility(item, EF_DAMAGE);
	
	item->Index = unique;

	Log(clientId, LOG_INGAME, "EXTRAÇÃO - Resultado %s [%d] [%d %d %d %d %d %d] - %d/%d", ItemList[item->Index].Name,
		item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3, _rand, chance);

	
	memset(&Mob[clientId].Mobs.Player.Inventory[wiseStone], 0, sizeof st_Item);

	SendItem(clientId, SlotType::Inv, slotId, item);
	SendItem(clientId, SlotType::Inv, wiseStone, &Mob[clientId].Mobs.Player.Inventory[wiseStone]);
	
	SendClientMessage(clientId, "Extração criada");

	Log(clientId, LOG_INGAME, "Extração criada : %s [%d] [%d %d %d %d %d %d]", ItemList[item->Index].Name,
		item->Index, item->EF1, item->EFV1, item->EF2, item->EFV2, item->EF3, item->EFV3);
	return true;
}