#include "cServer.h"
#include "Basedef.h"

bool CUser::RequestCreateChar(PacketHeader *Header)
{
	p20F *p = (p20F*)Header;

	// Faz as checagens de segurança, somente após isto
	// É feito as checagens de índice inválido

	if (p->ClassID < 0 || p->ClassID > 3)
		return false;

	if (p->SlotID < 0 || p->SlotID > 3)
		return false;

	// BUG Da própria TMsrv
	// Checagem se o usuário está logado
	// Caso não esteja, ele retorna falso pois não é possível criar personagem ingame.
	if(Status != USER_SELCHAR)
		return false;

	// Null character
	p->Nick[12] = '\0';

	p->Header.PacketId = 0x802;

	AddMessageDB((BYTE*)Header, sizeof p20F);

	// Seta como quem está criando um personagem
	Status = USER_CREWAIT;

	return true;
}