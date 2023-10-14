#include "Basedef.h"
#include "GetFunc.h"
#include "SendFunc.h"

bool CUser::RequestRedeemGriffin(PacketHeader* header)
{
	pMsgSignal3* packet = reinterpret_cast<pMsgSignal3*>(header);

	if (packet->Value2 < 0 || packet->Value2 >= 60)
	{
		Log(clientId, LOG_INGAME, "Enviado slot %d inválido na hora de capturar o Grifo", packet->Value);

		return false;
	}

	int slotId = packet->Value2;
	st_Item& ticketItem = Mob[clientId].Mobs.Player.Inventory[slotId];

	if ((packet->Value == 0 && ticketItem.Index != 4716) || (packet->Value == 1 && ticketItem.Index != 4731))
	{
		Log(clientId, LOG_INGAME, "Enviado slot %d. Esperava 4716 no slot mas há %s", slotId, ticketItem.toString().c_str());

		return false;
	}

	int selectedIndex = packet->Value3;
	if ((packet->Value == 0 && (selectedIndex < 0 || selectedIndex >= 5)) || (packet->Value == 1 && (selectedIndex < 0 || selectedIndex >= 2)))
	{
		Log(clientId, LOG_INGAME, "Enviado índice de escolha %d", selectedIndex);

		SendClientMessage(clientId, "Escolha incorreta");
		return false;
	}

	int costumeItemId = -1;
    switch (packet->Value)
    {
    case 0:
    {
        switch (selectedIndex)
        {
        case 0:
            costumeItemId = 4208;
            break;
        case 1:
            costumeItemId = 5711;
            break;
        case 2:
            costumeItemId = 4209;
            break;
        case 3:
            costumeItemId = 5710;
            break;
        case 4:
            costumeItemId = 4207;
            break;
        }
    }
    break;
    case 1:
        switch (selectedIndex)
        {
        case 0:
            costumeItemId = 5715;
            break;
        case 1:
            costumeItemId = 5714;
            break;
        }
    }

	int amount = GetItemAmount(&ticketItem);
	if (amount == 1)
	{
		ticketItem = st_Item{};
		ticketItem.Index = costumeItemId;

		SendItem(clientId, SlotType::Inv, slotId, &ticketItem);
	}
	else
	{
		int newSlotId = GetFirstSlot(clientId, 0);
		if (newSlotId == -1)
		{
			SendClientMessage(clientId, "Sem espaço livre no inventário");

			return true;
		}

		st_Item& item = Mob[clientId].Mobs.Player.Inventory[newSlotId];
		item = st_Item{};
		item.Index = costumeItemId;

		SendItem(clientId, SlotType::Inv, newSlotId, &item);

		AmountMinus(&ticketItem);
		SendItem(clientId, SlotType::Inv, slotId, &ticketItem);
	}

	Log(clientId, LOG_INGAME, "Resgatado o item %s com sucesso", ItemList[costumeItemId].Name);
	SendClientMessage(clientId, "Resgatado com sucesso");

	SendSignal(clientId, clientId, RedeemGriffinClosePacket);
	return true;
}