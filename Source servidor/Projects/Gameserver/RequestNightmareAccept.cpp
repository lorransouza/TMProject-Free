#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"

bool CUser::RequestNightmareAccept(PacketHeader* header)
{
	auto packet = reinterpret_cast<pMsgSignal*>(header);
	if (packet->Value < 0 || packet->Value >= 3)
	{
		Log(SERVER_SIDE, LOG_HACK, "Enviou índice inválido ao tentar entrar no Pesadelo novamente");

		return true;
	}

	int nightmareIndex = packet->Value;
	std::string playerName{ Mob[clientId].Mobs.Player.Name };
	auto& nightmare = sServer.Nightmare[nightmareIndex];

	auto it = std::find(std::begin(nightmare.MembersName), std::end(nightmare.MembersName), playerName);
	if (it == std::end(nightmare.MembersName))
	{
		Log(SERVER_SIDE, LOG_HACK, "Solicitou entrar em Pesadelo e o mesmo não estava presente na lista");

		return true;
	}

	int memberIndex = std::distance(std::begin(nightmare.MembersName), it);
	if (Mob[clientId].Mobs.Player.Equip[0].EFV2 >= CELESTIAL && Mob[clientId].Mobs.Player.bStatus.Level >= sServer.MaximumPesaLevel)
	{
		SendClientMessage(clientId, "Limite de nível atingido. Não é possível entrar na área do Pesadelo");

		return true;
	}

	int posX = 0;
	int posY = 0;
	if (nightmareIndex == 0)
	{
		int _rand = Rand() % 3;
		if (_rand >= 3)
			_rand = 2;

		posX = CoordsPesaN[_rand][0];
		posY = CoordsPesaN[_rand][1];
		if (Mob[clientId].Mobs.Nightmare[0].X != 0 && Mob[clientId].Mobs.Nightmare[0].Y != 0)
		{
			posX = Mob[clientId].Mobs.Nightmare[0].X;
			posY = Mob[clientId].Mobs.Nightmare[0].Y;
		}
	}
	else if (nightmareIndex == 1)
	{
		int _rand = Rand() % 6;
		if (_rand >= 6)
			_rand = 5;

		posX = CoordsPesaM[_rand][0];
		posY = CoordsPesaM[_rand][1];

		if (Mob[clientId].Mobs.Nightmare[1].X != 0 && Mob[clientId].Mobs.Nightmare[1].Y != 0)
		{
			posX = Mob[clientId].Mobs.Nightmare[1].X;
			posY = Mob[clientId].Mobs.Nightmare[1].Y;
		}
	}
	else if (nightmareIndex == 2)
	{
		posX = 1210;
		posY = 180;

		if (Mob[clientId].Mobs.Nightmare[2].X != 0 && Mob[clientId].Mobs.Nightmare[2].Y != 0)
		{
			posX = Mob[clientId].Mobs.Nightmare[2].X;
			posY = Mob[clientId].Mobs.Nightmare[2].Y;
		}
	}

	nightmare.Members[memberIndex] = clientId;
	Teleportar(clientId, posX, posY);

	SendSignalParm(clientId, SERVER_SIDE, 0x3A7, 2);
	SendSignalParm(clientId, clientId, 0x3A1, sServer.Nightmare[nightmareIndex].TimeLeft);
	return true;
}