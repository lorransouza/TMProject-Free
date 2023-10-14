#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"

bool CUser::RequestDeclareWar(PacketHeader *Header)
{
	pED7 *p = (pED7*)(Header);

	if(p->server > sServer.TotalServer)
	{
		AddCrackError(clientId, 5, CRACK_USER_PKTHACK);
		return true;
	}

	if(Mob[clientId].Mobs.Player.GuildMemberType != 9 && !IsAdmin)
		return true;

	if (ChargedGuildList[sServer.Channel - 1][4] != Mob[clientId].Mobs.Player.GuildIndex && !IsAdmin) // Somente caso possua a coroa
		return true;

	int slot = GetFirstSlot(clientId, 4030);
	if (slot == -1) // Mandou o pacote sem possuir declaração
		return true;

	AmountMinus(&Mob[clientId].Mobs.Player.Inventory[slot]);
	SendItem(clientId, SlotType::Inv, slot, &Mob[clientId].Mobs.Player.Inventory[slot]);

	// Envia o pacote declarando para a DBsrv
	UpdateWarDeclaration(1);

	return true;
}