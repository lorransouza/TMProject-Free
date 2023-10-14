#include "framework.h"
#include "DataServer.h"
#include "ProcessTimer.h"
#include "CPSock.h"
#include "CUser.h"
#include "Base.h"
#include "CFileDB.h"

int		x		= 0;
int		y		= 0;
HDC		hDC		= NULL;
HFONT	hFont	= NULL;
HFONT	h		= NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DrawConfig();

	switch (message)
	{
	case WM_TIMER:
		if (wParam == TIMER_SEC)
			ProcessSecTimer();
		break;

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
				if (!strcmp(g_pServerList[cServer.ServerIndex][i], stringIp))
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

		cFileDB.SendDBSignalParm2(userId, 0, 0x423, 1, cServer.Sapphire);
	}
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

		cServer.CurrentTime = GetTickCount();

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


	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Analise as seleções do menu:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		memset(&ps, 0, sizeof(PAINTSTRUCT));

		BeginPaint(hWnd, &ps);

		 // |FF_DECORATIVE
		hFont = CreateFont(12, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Lucida Console");

		hDC = ps.hdc;

		DrawConfig();

		EndPaint(hWnd, &ps);

	}
	break;

	case WM_SIZE:
	{ 
		DrawConfig();
		break; 
	}
	break;
	case WM_CLOSE:
	case WM_DESTROY:
		WSACleanup();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Manipulador de mensagem para a caixa 'sobre'.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void TextOutWind(char* str, int color)
{
	char String[1024];
	SetTextColor(hDC, color);
	sprintf(String, str);
	int len = strlen(String);
	TextOutA(hDC, x, y, String, len);
	y += 16;
}

void DrawConfig()
{
	x = 0;
	y = 0;

	h = 0;

	int VERMELHO = 255;
	int PRETO = 0;

	char String[1024] = { 0, };
	int len = 0;

	hDC = GetDC(hWndMain);

	if (hDC == NULL)
		return;

	if (hFont == 0)
		return;

	if (SelectObject(hDC, hFont) != 0)
		h = (HFONT)SelectObject(hDC, hFont);
	
	TextOutWind((char*)"Server Zone Status:", VERMELHO);

	for (int i = 0; i < MAX_SERVER; i++)
	{
		unsigned char* cIP = (unsigned char*)&(pUser[i].IP);

		sprintf(String, " %d - IP: %3d. %3d. %3d. %3d  Socket: %3d  Guild: %4d %4d %4d %4d %4d  User: %4d    ", i, cIP[0], cIP[1], cIP[2], cIP[3], pUser[i].Sock.Sock, ChargedGuildList[i][0], ChargedGuildList[i][1], ChargedGuildList[i][2], ChargedGuildList[i][3], ChargedGuildList[i][4], pUser[i].Count);
		TextOutWind(String, PRETO);
	}

	if (hFont && h)
		h = (HFONT)SelectObject(hDC, h);

	ReleaseDC(hWndMain, hDC);
}