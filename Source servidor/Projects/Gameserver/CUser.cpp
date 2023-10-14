#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

std::array<CUser, MAX_PLAYER> Users;

CUser::CUser()
{
	Status = USER_EMPTY;
	Socket.Socket = 0;

	hashIncrement = 0;
	Keys[0] = 0;

	clientId = 0;

	IsAdmin = false;
	IsAutoTrading = false;
	AccountID = 0;
	AccessLevel = 0;

	CrackCount = 0;

	Socket.recvBuffer = NULL;
	Socket.sendBuffer = NULL;
	Arena.GroupIndex = MAXUINT32;
}

CUser::~CUser()
{
	if (Socket.recvBuffer != NULL)
	{
		delete[] Socket.recvBuffer;
		Socket.recvBuffer = NULL;
	}

	if (Socket.sendBuffer != NULL)
	{
		delete[] Socket.sendBuffer;
		Socket.sendBuffer = NULL;
	}

	Socket.SizeOfSend = 0;
	Status = USER_EMPTY;
}

BYTE CUser::GetHashKey()
{
	if(*Keys == 0)
		return -1;

	BYTE key = 0;
	if (hashIncrement <= 15)
	{
		key = Keys[hashIncrement];

		hashIncrement++;
	}
	else
	{
		switch (Keys[15] & 0x80000001)
		{
		case 0:
			key = (BYTE)(Keys[1] + Keys[3] + Keys[5] - 87);
			break;
		case 1:
			key = (BYTE)(Keys[13] + Keys[11] - Keys[9] + 4);
			break;
		}
	}

	return BYTE(key ^ 255);
}


//bool CUser::CloseUser()
//{
//	// Usuário está ingame
//	if(Status == USER_PLAY)
//	{
//		RemoveParty(clientId, 0);
//		
//		DeleteMob(clientId, 0);
//		// Checa se a posição é válida
//		// Se o usuário estiver dentro de uma posição válida e se é o usuário que está registrado, 
//		// apaga ele do local onde lee está
//		// Para os outros usuários então não verem ele
//		INT32 posX = Mob[clientId].Target.X;
//		INT32 posY = Mob[clientId].Target.Y;
//		if(posX >= 0 && posX < 4096 && posY >= 0 && posY < 4096 && g_pMobGrid[posY][posX] == clientId)
//			g_pMobGrid[posY][posX] = 0;
//		
//		DeleteMob(clientId, 2);
//		
//		stSaveChar packet;
//		memset(&packet, 0, sizeof packet);
//
//		packet.Header.PacketId = 0x800;
//		packet.Header.ClientId = clientId;
//
//		packet.CharSlot = inGame.CharSlot;
//		packet.Coin = User.Storage.Coin;
//
//		memcpy(&packet.Storage, &User.Storage.Item, sizeof st_Item * 128);
//		memcpy(&packet.Mob, &Mob[clientId].Mobs, sizeof stCharInfo);
//		memcpy(packet.SkillBar, &Mob[clientId].Mobs.Player.SkillBar1, 4);
//		memcpy(&packet.SkillBar[4], Mob[clientId].Mobs.SkillBar, 16);
//		strncpy_s(packet.User, this->User.Username, 16);
//
//		memcpy(packet.Affect, Mob[clientId].Mobs.Affects, sizeof st_Affect * 32);
//
//		packet.Arg2 = 2;
//
//		AddMessageDB((BYTE*)&packet, sizeof stSaveChar);
//	}
//	else if(Status > 0)
//	{
//		stSaveChar packet;
//		memset(&packet, 0, sizeof packet);
//
//		packet.Header.PacketId = 0x800;
//		packet.Header.ClientId = clientId;
//
//		packet.CharSlot = -1;
//		packet.Coin = User.Storage.Coin;
//
//		memcpy(&packet.Storage, &User.Storage.Item, sizeof st_Item * 128);
//		strncpy_s(packet.User, this->User.Username, 16);
//
//		packet.Arg2 = 2;
//
//		AddMessageDB((BYTE*)&packet, sizeof stSaveChar);
//	}
//	
//	Log(clientId, LOG_INGAME, "Personagem finalizado.");
//
//	IsAutoTrading = false;
//	
//	// Zera o acesso a quest
//	QuestAccess = 0;
//
//	// Apaga o SND do usuário
//	SNDMessage[0] = 0;
//	// Apaga a última pessoa que o usuário conversou
//	LastWhisper = 0;
//
//	// 
//	Mob[clientId].Tab[0] = 0;
//
//	// Zera o tempo que o usuário está online
//	CharLoginTime = 0;
//
//	TimeStamp.LastReceiveTime = 0;
//	TimeStamp.LastAttack = 0;
//
//	inGame.incorrectNumeric = 0;
//
//	Socket.SizeOfSend = 0;
//	Status = USER_EMPTY;
//	
//	hashIncrement = 0;
//	memset(&Keys[0], 0, 16);
//
//	TokenOk = false;
//		
//	Socket.Init = 0;
//	closesocket(Socket.Socket);
//	return true;2
//}

bool CUser::CloseSocket()
{
	std::lock_guard<decltype(CUser::messageMutex)> mutex{ messageMutex };

	Socket.SizeOfSend = 0;
	Socket.Init = 0;
	Socket.nProcPosition = 0;
	Socket.nRecvPosition = 0;

	if (Socket.Socket)
	{
		closesocket(Socket.Socket);
		Socket.Socket = 0;
	}

	if (Socket.recvBuffer != nullptr)
	{
		delete[] Socket.recvBuffer;
		Socket.recvBuffer = nullptr;
	}

	if (Socket.sendBuffer != nullptr)
	{
		delete[] Socket.sendBuffer;
		Socket.sendBuffer = nullptr;
	}
	
	Socket.Socket = 0;

	return 1;
}

bool CUser::CloseUser_OL1()
{
	CloseSocket();

	Socket.Socket = 0;
	Status = USER_EMPTY;
	User.Username[0] = 0;
	return 1;
}

void CUser::RefreshSendBuffer()
{
	INT32 LOCAL_2 = Socket.nSendPosition - Socket.nSentPosition;
	if(LOCAL_2 <= 0)
		return;

	memcpy(Socket.sendBuffer, Socket.sendBuffer + Socket.nSentPosition, LOCAL_2);
	Socket.nSentPosition = 0;
	Socket.nSendPosition -= LOCAL_2;
}

bool CUser::canConnectionContinue(int error) const
{
	constexpr std::array allowedErrors{ 10035 };
	
	return std::find(allowedErrors.begin(), allowedErrors.end(), error) != allowedErrors.end();
}

BOOL CUser::SendMessageA(void)
{
	if(Socket.Socket <= 0)
	{
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;
		
		return false;
	}

	//std::lock_guard<decltype(messageMutex)> mutex{ messageMutex };

	if(Socket.nSentPosition > 0)
		RefreshSendBuffer();

	if(Socket.nSendPosition > MAX_BUFFER || Socket.nSendPosition < 0)
	{
		Log(clientId, LOG_INGAME, "Buffer inválido ao tentar enviar pacote. nSendPosition: %d", Socket.nSendPosition);
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;

		return true;
	}

	if(Socket.nSentPosition > Socket.nSendPosition || Socket.nSentPosition > MAX_BUFFER || Socket.nSentPosition < 0)
	{
		Log(clientId, LOG_INGAME, "Buffer inválido ao tentar enviar pacote. nSendPosition: %d. nSentPosition: %d", Socket.nSendPosition, Socket.nSentPosition);
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;
	}

	INT32 err = 0;
	for(INT32 i = 0; i < 1; i++)
	{
		INT32 LOCAL_68 = Socket.nSendPosition - Socket.nSentPosition;

		INT32 LOCAL_69 = send(Socket.Socket, (char*)Socket.sendBuffer + Socket.nSentPosition, LOCAL_68, 0);
		if(LOCAL_69 != -1)
			Socket.nSentPosition += LOCAL_69;
		else
			err = WSAGetLastError();
	}

	if (err != 0)
	{
		if (!canConnectionContinue(err))
		{
			Log(clientId, LOG_INGAME, "Erro no envio do pacote. Desconectando usuário. ErrorCode: %d", err);
			return false;
		}

		CheckIdle(clientId);

		Socket.Error++;
		if (Socket.Error < 10)
			Log(clientId, LOG_INGAME, "Erro no envio do pacote. Total a ser enviado: %d. Error: %d", Socket.nSendPosition, err);
	}
	else if (Socket.Error != 0)
		Socket.Error = 0;

	if(Socket.nSentPosition >= Socket.nSendPosition || err != 0)
	{
		Socket.nSendPosition = 0;
		Socket.nSentPosition = 0;

		return true;
	}

	if (Socket.nSendPosition > MAX_BUFFER)
	{
		Log(clientId, LOG_INGAME, "Buffer de envio cheio");

		return false;
	}

	return true;
}

BOOL CUser::SendOneMessage(BYTE *packet, INT32 size)
{
	AddMessage(packet, size);

	return SendMessageA();
}

std::string CUser::LogSameAccounts() const
{
	std::stringstream str;
	auto users = GetSameMACUsers(*this);
	
	str << "[USUÁRIOS LOGADOS]" << std::endl;

	if (users.size() > 0)
	{
		for (const auto& user : users)
		{
			str << "[" << user->User.Username << "]\t";

			if (user->Status == USER_SELCHAR)
				str << "SELCHAR\t";
			else if (user->Status == USER_PLAY)
			{
				const auto& mob = Mob[user->clientId].Mobs.Player;
				str << "USER_PLAY\t" << "[" << Mob[user->clientId].Mobs.Player.Name << "] (level: " << mob.Status.Level << " ev: " << (int)mob.GetEvolution() << ")\t";
			}

			str << std::endl;
		}
	}
	else
		str << "Nenhuma outra conta logada " << std::endl;;


	return str.str().c_str();
}

void CUser::LogEquipsAndInventory(bool force) const
{
	if (std::chrono::high_resolution_clock::now() - lastLogEquipAndInventory < 30min && !force)
		return;

	std::stringstream strItems;
	if (Status == USER_SELCHAR)
	{
		strItems << "[INFORMAÇÕES DA CONTA]\n";
		strItems << "Personagem na seleção de personagem. Sem informações no momento\n";
		strItems << "Tempo online até o momento: "
			<< std::setfill('0') << std::setw(2) << (Users[clientId].Time / 86400) << " dias e "
			<< std::setfill('0') << std::setw(2) << (Users[clientId].Time / 3600) % 24
			<< ":" << std::setfill('0') << std::setw(2) << (Users[clientId].Time / 60) % 60
			<< ":" << std::setfill('0') << std::setw(2) << (Users[clientId].Time % 60) << " horas\n";
	}
	else if(Status == USER_PLAY)
	{
        auto evolution = Mob[clientId].Mobs.Player.GetEvolution();
		strItems << "[INFORMAÇÕES DA CONTA]\n";
		strItems << "Personagem online: " << Mob[clientId].Mobs.Player.Name << "\n";
		strItems << "Gold atual: " << Mob[clientId].Mobs.Player.Gold << "\n";
		strItems << "Level atual: " << Mob[clientId].Mobs.Player.bStatus.Level << "\n";
		strItems << "Experiência atual: " << Mob[clientId].Mobs.Player.Exp << "\n";
		strItems << "Evolução atual: " << GetEvolutionName(evolution) << "\n";
		strItems << "Tempo online até o momento: "
			<< std::setfill('0') << std::setw(2) << (Users[clientId].Time / 86400) << " dias e "
			<< std::setfill('0') << std::setw(2) << (Users[clientId].Time / 3600) % 24
			<< ":" << std::setfill('0') << std::setw(2) << (Users[clientId].Time / 60) % 60
			<< ":" << std::setfill('0') << std::setw(2) << (Users[clientId].Time % 60) << " horas\n";
		strItems << "Posição atual: " << Mob[clientId].Target.X << "x " << Mob[clientId].Target.Y << "y\n\n";
		strItems << "Pontos NT: " << ((int)Mob[clientId].Mobs.PesaEnter) << "\n";
		strItems << "Entradas Hall: " << Mob[clientId].Mobs.HallEnter << "\n";
		strItems << "Info: " << Mob[clientId].Mobs.Info.Value << "\n";
		strItems << "Soul: " << ((int)Mob[clientId].Mobs.Soul) << "\n";
		strItems << "Divina: " << Mob[clientId].Mobs.Divina.toString().c_str() << "\n";
		strItems << "Sephira: " << Mob[clientId].Mobs.Sephira.toString().c_str() << "\n";
		strItems << "Saude: " << Mob[clientId].Mobs.Saude.toString().c_str() << "\n";
		strItems << "Escritura: " << Mob[clientId].Mobs.Escritura.toString().c_str() << "\n";
		strItems << "Revigorante: " << Mob[clientId].Mobs.Revigorante.toString().c_str() << "\n";
		strItems << "LastGuildKickout: " << Mob[clientId].Mobs.LastGuildKickOut.toString().c_str() << "\n";

        // verifica se está no sub ou se tem subcelestial
        if (evolution == TOD_Class::SubCelestial || Mob[clientId].Mobs.Sub.Status == 1)
        {
            const auto& sub = Mob[clientId].Mobs.Sub;
            strItems << "\n[PEDRA MISTERIOSA]\n";
            strItems << "Evolução: " << GetEvolutionName(static_cast<TOD_Class>(sub.Equip[0].EFV2)) << "\n";
            strItems << "Level atual: " << sub.SubStatus.Level << "\n";
            strItems << "Experiência atual: " << sub.Exp << "\n";
            strItems << "Learn: " << sub.Learn << "\n";
            strItems << "SecLearn: " << sub.SecLearn << "\n";
            strItems << "Info: " << sub.Info.Value << "\n";
            strItems << "Soul: " << ((int)sub.Soul) << "\n\n";
        }

		strItems << "[EQUIPAMENTOS]\n";

		for (int i = 0; i < 18; i++)
		{
			st_Item* item = &Mob[clientId].Mobs.Player.Equip[i];
			if (item->Index <= 0 || item->Index > MAX_ITEMLIST)
				continue;

			strItems << "[" << std::setw(2) << i << "] - " << ItemList[item->Index].Name << item->toString().c_str() << "\n";
		}

		strItems << "\n[INVENTÁRIO]\n";

		for (int i = 0; i < 64; i++)
		{
			st_Item* item = &Mob[clientId].Mobs.Player.Inventory[i];
			if (item->Index <= 0 || item->Index > MAX_ITEMLIST)
				continue;

			strItems << "[" << std::setw(2) << i << "] - " << ItemList[item->Index].Name << item->toString().c_str() << "\n";
		}
	}
	else
	{
		strItems << "[INFORMAÇÕES DA CONTA]\n";
		strItems << "Personagem com status \"" << Status << "\". Sem informações no momento\n";
		strItems << "Tempo online até o momento: "
			<< std::setfill('0') << std::setw(2) << (Users[clientId].Time / 86400) << " dias e "
			<< std::setfill('0') << std::setw(2) << (Users[clientId].Time / 3600) % 24
			<< ":" << std::setfill('0') << std::setw(2) << (Users[clientId].Time / 60) % 60
			<< ":" << std::setfill('0') << std::setw(2) << (Users[clientId].Time % 60) << " horas\n";
	}

	strItems << std::endl << LogSameAccounts();

	Log(clientId, LOG_INGAME, strItems.str().c_str());
	lastLogEquipAndInventory = std::chrono::high_resolution_clock::now();
}

std::tuple<bool, std::chrono::milliseconds> CUser::CheckIfIsTooFast(const st_Item* item, int slotId) 
{
	// Verifica na lista de itens recentes deletados se consta o mesmo
	auto result = std::find_if(std::begin(Dropped.Items), std::end(Dropped.Items), [&](DroppedItem& droppedItem) {
		return memcmp(&droppedItem.Item, item, sizeof st_Item) == 0 && droppedItem.SlotId == slotId;
	});

	if (result != std::end(Dropped.Items))
	{
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - result->Time);

		result->Item = st_Item{};
		return std::make_pair(time < 700ms, time);
	}

	return std::make_pair(false, std::chrono::milliseconds(0));
}

void CUser::GenerateNewAutoTradeTime()
{
	bool hasPotion = false;
	for (int i = 0; i < 32; i++)
	{
		if (Mob[clientId].Mobs.Affects[i].Index == 59)
		{
			hasPotion = true;

			break;
		}
	}

	int time;
	if (!hasPotion)
		time = (Rand() % 11) + 15;
	else
		time = (Rand() % 8) + 7;

	EventAutoTrade.TimeToWin = std::chrono::milliseconds(time * 1000 * 60);
	Log(clientId, LOG_INGAME, "Espera aproximadamente para receber item do evento: %d minutos.", time);
}