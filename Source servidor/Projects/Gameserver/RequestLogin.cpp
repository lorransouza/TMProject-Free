#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "UOD_GUID.h"
#include <sstream>
void EncryptVersion(UINT32 *pVersion)
{
	INT32 random = (Rand() % 7) + 1;

	INT32 randIterator = Rand() & 0x80000003;
	if (randIterator < 0)
	{
		randIterator--;
		randIterator |= 0xFFFFFFFC;
		randIterator++;
	}

	*pVersion = ((*pVersion << (byte)(random + 5)) | (random << 2)) | randIterator;
}

void DecryptVersion(UINT32 *pVersion)
{
	*pVersion >>= (byte)(((*pVersion & 28) >> 2) + 5);
}

bool CUser::RequestLogin(PacketHeader *Header)
{
	p20D *p = (p20D*)Header;

	// Decripta a versão enviada pelo cliente
	//DecryptVersion(&p->CliVer);

	printf("%d \n", p->CliVer);

	p->Username[15] = 0;
	p->Password[11] = 0;

#ifndef _DEBUG
	// Checa com a versão do servidor
	if(p->CliVer != sServer.CliVer)
	{
		//SendClientMessage(clientId, g_pLanguageString[_NN_Version_Not_Match_Rerun]);
		SendClientMessage(clientId, "Versao de cliente %d incompativel!", p->CliVer);
		Log(clientId, LOG_INGAME, "Tentativa de logar com cliver incorreto. Cliver: %u", p->CliVer);
		SendMessageA();
		return true;
	}
#endif 

	// Caso ele não esteja no modo correto
	if(Status != USER_ACCEPT)
	{
		SendClientMessage(clientId, "Login now, wait a moment.");

		SendMessageA();
		return true;
	}

	std::vector<CUser*> users;
	INT32 total = 0;
	for(INT32 i = 1; i < MAX_PLAYER; i++)
	{
		if(Users[i].Status < USER_SELCHAR || i == clientId)
			continue;

		if (memcmp(Users[i].MacAddress, p->AdapterInfos, 8) == 0)
		{
			total++;

			users.push_back(&Users[i]);
		}
	}

	std::stringstream str;
	for (const auto& user : users)
	{
		str << "Conta " << user->User.Username << " logado na conta\n";
		str << "Status da conta: ";
		if (user->Status == USER_PLAY)
		{
			str << "JOGANDO\n";
			str << "Personagem: " << Mob[user->clientId].Mobs.Player.Name;
		}
		else if (user->Status == USER_SELCHAR)
			str << "SELEÇÃO DE PERSONAGEM";
		else
			str << "Outro (" << user->Status << ")";

		str << "\n";

		Log(user->clientId, LOG_INGAME, "O login %s tentou logar na conta com o mesmo mac.", p->Username);
	}

#if !defined(_DEBUG)
	if (total >= 10)
	{
		SendClientMessage(clientId, "Limite de 10 contas por computador");

		SendMessageA();
		return true;
	}
#endif

	memcpy(Users[clientId].MacAddress, p->AdapterInfos, 8);

	// Caso exista aspas ou espaço no login, excluirá
	for(INT32 i = 0; i < 16;i ++)
	{
		if(p->Username[i] == '\'')
			p->Username[i] = 0;

		if(p->Username[i] == ' ')
			p->Username[i] = 0;
	}

	_strupr_s(p->Username);
	strncpy_s(User.Username, p->Username, 16);

	NormalLog = std::make_unique<TOD_Log>("..\\Logs\\Players", User.Username);
	HackLog = std::make_unique<TOD_Log>("..\\Logs\\Hack", User.Username);

	Log(clientId, LOG_INGAME, "=========================== NOVA SESSÃO ==========================\nTentativa de logar na conta - IP: %s Mac: (%02X:%02X:%02X:%02X:%02X:%02X). ClientId: %d", IP, (int)p->AdapterInfos[0], (int)p->AdapterInfos[1], (int)p->AdapterInfos[2],
		(int)p->AdapterInfos[3], (int)p->AdapterInfos[4], (int)p->AdapterInfos[5], clientId);

	if(!str.str().empty())
		Log(clientId, LOG_INGAME, str.str().c_str());

	// Checa se a senha já foi errada mais de três vezes
	// 00422D52
	INT32 checkFail = CheckFailAccount(p->Username); // local29
	if (checkFail >= 3)
	{
		SendClientMessage(clientId, g_pLanguageString[_NN_3_Tims_Wrong_Pass]);

		SendMessageA();
		return true;
	}
	
	// Envia o pacote a DBsrv
	p->Header.ClientId = clientId;
	p->Header.PacketId = 0x803;

	AddMessageDB((BYTE*)Header, sizeof p20D);

	Status = USER_LOGIN;
	Mob[clientId].Mode = 0;
	Mob[clientId].clientId = clientId;
	return true;
}

bool CUser::Msg_Numeric(PacketHeader* Header)
{
	pFDE* p = (pFDE*)Header;

	if (Status != 11)
	{
		CloseUser(clientId);
		return false;
	}

	if (p->Header.Size != sizeof pFDE)
	{
		return false;
	}

	p->Header.ClientId = clientId;


	if (p->RequestChange == 1 && TokenOk)
	{
		AddMessageDB((BYTE*)p, sizeof pFDE);

		return false;
	}

	AddMessageDB((BYTE*)p, sizeof pFDE);
	return true;
}