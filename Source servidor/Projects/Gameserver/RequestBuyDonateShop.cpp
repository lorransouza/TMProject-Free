#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestBuyDonateShop(PacketHeader *Header)
{
	MSG_BUYOVERSTORE *p = (MSG_BUYOVERSTORE*)(Header);
	if (!GetAttribute(Mob[clientId].Target.X, Mob[clientId].Target.Y).Village)
	{
		SendClientMessage(clientId, "Não é possível comprar fora da cidade");

		return true;
	}

	p->Nickname[15] = '\0';

	auto& donateStore = sServer.DonateStore;

	TOD_OverStore_ItemInfo* itemInfo = nullptr;
	for (auto& category : donateStore.Categories)
	{
		auto itemInfoIt = std::find_if(std::begin(category.Items), std::end(category.Items), [p](const TOD_OverStore_ItemInfo& itemInfo) {
			return memcmp(&itemInfo.Item, &p->Item, sizeof st_Item) == 0;
		});

		if (itemInfoIt != std::end(category.Items))
		{
			itemInfo = &*itemInfoIt;

			break;
		}
	}

	if (itemInfo == nullptr)
	{
		SendClientMessage(clientId, "Item não encontrado");

		return true;
	}

	if(itemInfo->Available != -1 && itemInfo->Available <= 0)
	{
		SendClientMessage(clientId, "Item esgotado no momento. Aguarde a reposição.");

		return true;
	}

	BYTE giftId = 0;
	bool isGift = itemInfo->Item.hasAdd(EF_GIFTID, giftId);

	unsigned int totalPrice = itemInfo->Price * p->Quantity;
	if(isGift)
	{
		// se for, verificamos agora se ele já efetuou esta compra
		if (User.SingleGift & (uint64_t(1) << giftId))
		{
			SendClientMessage(clientId, "Este presente só pode ser recebido uma única vez");

			return true;
		}

		p->Quantity = 1;
	}

	if (itemInfo->Available != -1)
		p->Quantity = 1;
	
	int userId = clientId;
	if (p->Nickname[0])
	{
		userId = GetUserByName(p->Nickname);
		if (userId == 0)
		{
			SendClientMessage(clientId, "Usuário não está conectado");

			return true;
		}

		if (isGift)
		{
			SendClientMessage(clientId, "Presentes da loja não podem ser enviados a outros jogadores");

			return true;
		}

		totalPrice += (p->Quantity * 50);
	}
	
	if (User.Cash < totalPrice)
	{
		SendClientMessage(clientId, "Você não possui OverPoints suficientes.");

		return true;
	}

	if (itemInfo->Available != -1 && itemInfo->Available < p->Quantity)
	{
		SendClientMessage(clientId, "Não existe esta quantidade disponível");

		return true;
	}

	int freeSlot = GetInventoryAmount(userId, 0);
	if (freeSlot < p->Quantity)
	{
		if (userId != clientId)
			SendClientMessage(clientId, "O outro jogador não possui espaço no inventário para receber o presente!");
		else
			SendClientMessage(clientId, "Não possui espaço suficiente no inventário");

		return true;
	}

	for (int i = 0; i < p->Quantity; ++i)
	{
		INT32 slotId = GetFirstSlot(userId, 0);
		if (slotId == -1)
		{
			SendClientMessage(clientId, "Não possui espaço suficiente no inventário");

			return true;
		}

		Mob[userId].Mobs.Player.Inventory[slotId] = itemInfo->Item;

		if(itemInfo->Available != -1)
			itemInfo->Available--;

		SendItem(userId, SlotType::Inv, slotId, &Mob[userId].Mobs.Player.Inventory[slotId]);

		if (clientId != userId)
		{
			Log(userId, LOG_INGAME, "Recebido presente de %s. Item %s %s no slot %d", Mob[clientId].Mobs.Player.Name, ItemList[itemInfo->Item.Index].Name, itemInfo->Item.toString().c_str(), slotId);
			User.Cash -= 50;
		}
		else
			Log(userId, LOG_INGAME, "Adicionado o item %s %s no slot %d", itemInfo->Item.toString().c_str(), ItemList[itemInfo->Item.Index].Name, slotId);

		if (!isGift)
		{
			User.Cash -= itemInfo->Price;

			if (userId != clientId)
				SendClientMessage(userId, "!Você recebeu um presente de %s da OverStore: [%s]", Mob[clientId].Mobs.Player.Name, ItemList[itemInfo->Item.Index].Name);
		}
		else
			User.SingleGift |= (uint64_t(1) << giftId);
	}

	Log(clientId, LOG_INGAME, "Compra item DONATE [%s] %s preço %hd. Quantidade comprado: %d. Enviado para (%s) %s", ItemList[itemInfo->Item.Index].Name,
		itemInfo->Item.toString().c_str(), itemInfo->Price, p->Quantity, Mob[userId].Mobs.Player.Name, Users[userId].User.Username);

	Log(SERVER_SIDE, LOG_INGAME, "%s - Compra item DONATE [%s]%s preço %hd. Quantidade comprado: %d. Quantidades restantes: %d. Enviado para (%s) %s", User.Username, ItemList[itemInfo->Item.Index].Name,
		itemInfo->Item.toString().c_str(), itemInfo->Price, p->Quantity, itemInfo->Available, Mob[userId].Mobs.Player.Name, Users[userId].User.Username);

	Log(clientId, LOG_INGAME, "Cash restante na conta: %u", User.Cash);
	LogPlayer(clientId, "Comprou o item %s na loja cash por %hd", ItemList[itemInfo->Item.Index].Name, itemInfo->Price);
	
	SendClientMessage(clientId, "Você comprou %d %s por um total de %d. Restam %d OverPoints.", p->Quantity, ItemList[itemInfo->Item.Index].Name, totalPrice, User.Cash);
	SendSignalParm(clientId, clientId, RefreshGoldPacket, User.Cash);
	return true;
}