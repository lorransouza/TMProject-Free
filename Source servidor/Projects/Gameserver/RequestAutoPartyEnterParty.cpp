#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"
#include <ctime>
#include <algorithm>
#include <sstream>

bool CUser::RequestAutoPartyEnterParty(PacketHeader* header)
{
	MSG_AUTOPARTY_ENTERPARTY* packet = reinterpret_cast<MSG_AUTOPARTY_ENTERPARTY*>(header);
	packet->Nickname[15] = '\0';
	packet->Password[15] = '\0';

	int userId = GetUserByName(packet->Nickname);
	if (userId <= 0 || userId >= MAX_PLAYER)
	{
		SendClientMessage(clientId, "O usuário não está conectado.");

		return true;
	}

	CMob& mob = Mob[userId];
	if (mob.Leader != 0)
	{
		SendClientMessage(clientId, "Usuário não é líder de grupo");

		return true;
	}

	const auto& autoParty = Users[userId].AutoParty;
	if (!autoParty.Password[0])
	{
		SendClientMessage(clientId, "O usuário não está com autogrupo ligado");

		return true;
	}

	std::string password = packet->Password;
	if (password != autoParty.Password)
	{
		SendClientMessage(clientId, "A senha está incorreta");

		return true;
	}

	if (GetDistance(mob.Target.X, mob.Target.Y, Mob[clientId].Target.X, Mob[clientId].Target.Y) > 50)
	{
		SendClientMessage(clientId, "Distância muito grande para entrar no grupo. Aproxime-se do personagem");

		return true;
	}

	if (!autoParty.EnableAll)
	{
		auto selfName = std::string(Mob[clientId].Mobs.Player.Name);
		if (std::find(std::begin(autoParty.Nicknames), std::end(autoParty.Nicknames), selfName) == std::end(autoParty.Nicknames))
		{
			SendClientMessage(clientId, "Você não está habilitado a entrar no grupo");

			return true;
		}
	}

	p3AB acceptPacket{};
	acceptPacket.Header.PacketId = 0x3AB;
	acceptPacket.Header.Size = sizeof p3AB;

	acceptPacket.liderID = userId;
	strncpy_s(acceptPacket.nickName, packet->Nickname, 16);

	PacketControl(reinterpret_cast<BYTE*>(&acceptPacket), sizeof p3AB);
	return true;
}