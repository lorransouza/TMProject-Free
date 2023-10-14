#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include "GetFunc.h"
#include "CNPCGener.h"

bool CUser::RequestFriendListUpdate(PacketHeader *Header)
{
	_MSG_FRIENDLIST_UPDATE *p = (_MSG_FRIENDLIST_UPDATE*)(Header);

	if(p->Type == 1)
	{
		INT32 userId = GetUserByName(p->Name);
		if(userId <= 0)
		{
			SendClientMessage(clientId, g_pLanguageString[_NN_Not_Connected] );
			
			return true;
		}

		if(Users[userId].IsAdmin || Users[userId].User.AccessLevel != 0)
			return true;
		
		if(userId == clientId)
		{
			SendClientMessage(clientId, "Não é possível adicionar você mesmo como amigo");

			return true;
		}
		
		INT32 i = -1;
		for(i = 0; i < 30 ; i++)
		{
			if(!strncmp(User.Friends[i], p->Name, 16))
			{
				SendClientMessage(clientId, "Este usuário já é seu amigo");

				return true;
			}
		}

		for(i = 0 ; i < 30; i ++)
		{
			if(!User.Friends[i][0])
				break;
		}

		if(i == 30)
		{
			SendClientMessage(clientId, "Você não tem espaço para novos amigos");
			return true;
		}

		p->Server = sServer.Channel;
		AddMessage((BYTE*)p, sizeof _MSG_FRIENDLIST_UPDATE);

		strncpy_s(User.Friends[i], p->Name, 16);
		return true;
	}
	else if(p->Type == 2)
	{
		INT32 i = 0;
		for( ; i < 30; i ++)
		{
			if(!strncmp(p->Name, User.Friends[i], 16))
				break;
		}

		if(i == 30)
			return true;
		
		AddMessage((BYTE*)p, sizeof _MSG_FRIENDLIST_UPDATE);
		strncpy_s(User.Friends[i], "", 16);
	}

	return true;
}