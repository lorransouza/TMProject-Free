#include "Basedef.h"
#include "CUser.h"
#include "Server.h"
#include "CFileDB.h"
#include "CPSock.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch (Message)
		{
		case WM_TIMER:
			if (wParam == TIMER_SEC)
				ProcessSecTimer();
			break;

		case WSA_READ:
		{
			INT32 user = GetUserFromSocket(wParam); // LOCAL_267
			if (user == -1)
			{
				closesocket(wParam);

				//Log( err, wsa_read unregistered game server socket
				break;
			}

			if (WSAGETSELECTEVENT(lParam) != FD_READ)
			{
				// "clo, server fd %d", user
				pUser[user].Sock.CloseSocket();
				pUser[user].Mode = 0;
				break;
			}

			sServer.CurrentTime = GetTickCount();

			if (!pUser[user].Sock.Receive())
			{
				Log("Falha ao tentar dar Receive()");

				break;
			}

			int code;
			int type;
			do
			{
				char* packet = pUser[user].Sock.ReadMessage(&code, &type);
				if (packet == NULL)
					break;

				//checkpacket
				if (code == 1 || code == 2)
					break;

				ProcessClientMessage(user, packet);
			} while (1);
		}
		break;
		case WSA_READADMIN:
		{
			int User = GetAdminFromSocket(wParam);

			if (User == -1)
			{
				closesocket(wParam);

				//Log("err, wsa_read unregistered sever socket", "-system", 0);

				break;
			}

			if (WSAGETSELECTEVENT(lParam) != FD_READ)
			{
				char temp[256];
				sprintf_s(temp, "admin close fd %d", User);
				//Log(temp, "-system", 0);

				pAdmin[User].Sock.CloseSocket();
				pAdmin[User].Mode = USER_EMPTY;
				pAdmin[User].Name[0] = 0;

				break;
			}

			//CurrentTime = GetTickCount();
			sServer.CurrentTime = GetTickCount();
			if (pAdmin[User].Sock.Receive() != 1)
			{
				//char temp[256];
				//sprintf_s(temp, "admin %d close receive", User);
				//Log(temp, "-system", 0);

				break;
			}

			int Error;
			int ErrorCode;

			while (1)
			{
				char* Msg = pAdmin[User].Sock.ReadMessage(&Error, &ErrorCode);

				if (Msg == NULL)
					break;

				if (Error == 1 || Error == 2)
				{
					//sprintf_s(temp, "err,wsa_Readadmin (%d),%d", Error, ErrorCode);
					//Log(temp, "-system", 0);

					break;
				}

				int ret = ProcessAdminMessage(User, Msg);

				if (ret == FALSE)
				{
					pAdmin[User].Sock.CloseSocket();
					pAdmin[User].Mode = USER_EMPTY;
					pAdmin[User].Name[0] = 0;

					break;
				}
			};

		} break;

		case WSA_ACCEPTADMIN:
		{
			ReadAdmin();

			if (WSAGETSELECTERROR(lParam))
			{
				//Log("err,accept_admin accepting fails", "-system", 0);

				break;
			}

			TempUser.AcceptUser(AdminSocket.Sock, WSA_READADMIN);

			unsigned char* cIP = (unsigned char*) & (TempUser.IP);
			int User = -1;
			int i = 0;

			for (i = 0; i <= MAX_ADMIN; i++)
			{
				if (pAdmin[i].IP == TempUser.IP)
				{
					User = i;
					break;
				}
			}

			if (User == -1)
			{
				int i = 0;

				for (i = 0; i < MAX_ADMIN; i++)
				{
					//	int a, b, c, d; a = b = c = d = 0;

					if (pAdminIP[i] == 0)
						continue;

					if (pAdminIP[i] == TempUser.IP)
					{
						User = i;
						break;
					}
				}

				if (i == MAX_ADMIN)
				{
					TempUser.Sock.CloseSocket();

					//char temp[256];
					//sprintf_s(temp, "err, wsa_acceptadmin request accept from  %d.%d.%d.%d", cIP[0], cIP[1], cIP[2], cIP[3]);
					//Log(temp, "-system", 0);
				}
			}

			if (User >= 0 && User < MAX_ADMIN)
			{
				if (pAdmin[User].Mode != USER_EMPTY)
				{
					/* char temp[256];
					sprintf_s(temp, "err, wsa_acceptadmin not empty previous slot %d", i);
					Log(temp, "-system", 0);*/

					TempUser.Sock.CloseSocket();

					pAdmin[User].Sock.CloseSocket();
					pAdmin[User].Mode = USER_EMPTY;

					break;
				}
			}
			else
			{
				//Log("err,accept_admin unknown attempt", "-system", 0);

				TempUser.Sock.CloseSocket();

				break;
			}

			if (User == -1)
			{
				//Log("err,accept_admin no empty", "-system", 0);
				break;
			}

			pAdmin[User].Sock.recvBuffer = new char[RECV_BUFFER_SIZE];
			pAdmin[User].Sock.sendBuffer = new char[RECV_BUFFER_SIZE];

			pAdmin[User].Level = -1;
			pAdmin[User].DisableID = 0;
			pAdmin[User].IP = TempUser.IP;
			pAdmin[User].Mode = TempUser.Mode;
			pAdmin[User].Name[0] = 0;
			pAdmin[User].Sock.Sock = TempUser.Sock.Sock;
			pAdmin[User].Sock.nRecvPosition = TempUser.Sock.nRecvPosition;
			pAdmin[User].Sock.nProcPosition = TempUser.Sock.nProcPosition;
			pAdmin[User].Sock.nSendPosition = TempUser.Sock.nSendPosition;

			cIP = (unsigned char*) & (pAdmin[User].IP);

			//char tmp[100];
			//sprintf_s(tmp, "sys,wsa_acceptadmin %d.%d.%d.%d <%d>", cIP[0], cIP[1], cIP[2], cIP[3], User);
			//Log(tmp, "-system", 0);

			pAdmin[User].Encode1 = rand() % 10000;
			pAdmin[User].Encode2 = rand() % 10000;

			MSG_NPIDPASS sm;
			memset(&sm, 0, sizeof(MSG_NPIDPASS));

			sm.Header.PacketId = _MSG_NPReqIDPASS;
			sm.Header.Size = sizeof(MSG_NPIDPASS);
			sm.Header.ClientId = 0;

			sm.Encode1 = pAdmin[User].Encode1;
			sm.Encode2 = pAdmin[User].Encode2;

			pAdmin[User].Sock.SendOneMessage((char*)& sm, sizeof(MSG_NPIDPASS));
		} break;

		case WSA_ACCEPT:
		{
			CUser TempUser;
			TempUser.AcceptUser(Server[0].Sock, WSA_READ);

			char stringIp[32];
			char* numberIp = (char*)&TempUser.IP;
			sprintf_s(stringIp, "%d.%d.%d.%d", (UINT8)numberIp[0], (UINT8)numberIp[1], (UINT8)numberIp[2], (UINT8)numberIp[3]);

			printf("%s\n", stringIp);
			INT32 userId = -1;
			INT32 iterator = 0;

			for (; iterator < MAX_SERVERGROUP; iterator++)
			{
				if (pUser[iterator].IP == TempUser.IP)
				{
					userId = iterator;
					break;
				}
			}

			if (userId == -1)
			{
				INT32 i = 1;
				for (; i < MAX_SERVERNUMBER; i++)
				{
					if (!strcmp(g_pServerList[sServer.ServerIndex][i], stringIp))
						break;
				}

				if (i == MAX_SERVERNUMBER)
				{
					TempUser.Sock.CloseSocket();

					break;
				}

				userId = i - 1;
			}

			else if (userId >= 0 && userId < MAX_SERVERGROUP)
			{
				if (pUser[userId].Mode != 0)
				{
					printf("err, wsa_accept no previous slot %s", stringIp);

					TempUser.Sock.CloseSocket();
					break;
				}
			}
			else
			{
				printf("err, wsa_accept unknow attempt %s", stringIp);

				TempUser.Sock.CloseSocket();
			}

			if (userId == -1)
			{
				printf("err, wsa_accept no empty %s", stringIp);
				break;
			}

			pUser[userId].Sock.recvBuffer = new char[RECV_BUFFER_SIZE];
			pUser[userId].Sock.sendBuffer = new char[RECV_BUFFER_SIZE];

			pUser[userId].IP = TempUser.IP; // 
			pUser[userId].Mode = TempUser.Mode;// 1168D6C
			pUser[userId].Sock.Sock = TempUser.Sock.Sock;
			pUser[userId].Sock.Port = TempUser.Sock.Sock;
			pUser[userId].Sock.nRecvPosition = TempUser.Sock.nRecvPosition;
			pUser[userId].Sock.nProcPosition = TempUser.Sock.nProcPosition;
			pUser[userId].Sock.nSendPosition = TempUser.Sock.nSendPosition;

			cFileDB.SendDBSignalParm2(userId, 0, 0x423, 1, sServer.Sapphire);
		}
		break;

		case WM_DESTROY:
			WSACleanup();
			PostQuitMessage(0);
			break;

		default:
			return (DefWindowProc(hWnd, Message, wParam, lParam));
		}

		return 0;
	}
	catch (std::exception& e)
	{
		printf("Excecao aconteceu: %s", e.what());

		return 0;
	}
}

