#include "cServer.h"
#include "Basedef.h"
#include "SendFunc.h"
#include <new>          // std::bad_alloc
#include <chrono>
int		x = 0;
int		y = 0;
HDC		hDC = NULL;
HFONT	hFont = NULL;
HFONT	h = NULL;
LONG APIENTRY WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		CUser* user;
		switch (Message)
		{
		case WM_TIMER:
		{
			if (wParam == TIMER_SEC)
				ProcessSecTimer();
			else if (wParam == TIMER_MIN)
				ProcessMinTimer();
			else if (wParam == TIMER_HOUR)
				ProcessHourTimer();
			else if (wParam == TIMER_SEND)
				ProcessSendTimer();
		}
		break;

		case WSA_READDB:
		{
			if (WSAGETSELECTEVENT(lParam) != FD_READ)
			{
				closesocket(sData.Socket);

				return true;
			}

			int ret = Receive();
			if (ret == FALSE)
			{
				ret = WSAGetLastError();

				break;
			}

			else if (ret == -1)
			{
				ret = WSAGetLastError();

				break;
			}

			int Error = FALSE;
			int ErrorCode = FALSE;
			while (TRUE)
			{
				char* Msg = ReadMessageDB(&Error, &ErrorCode);
				if (Msg == NULL)
					break;

				PacketControl((BYTE*)Msg, ErrorCode);
			}

		}
		break;
		// Aceitação de um usuário ao tentar-se conectar
		case WSA_ACCEPT:
		{
			if (WSAGETSELECTERROR(lParam) == 0)
			{
				int userId = GetEmptyUser();
				if (userId == 0)
				{
					Log(SERVER_SIDE, LOG_INGAME, "Cant find a slot to player");

					break;
				}

				int ret = Users[userId].AcceptUser(sServer.Socket);
				if (userId >= MAX_PLAYER - 10)
				{
					SendClientMessage(userId, "Servidor cheio. Tente novamente mais tarde");
					Users[userId].SendMessageA();

					CloseUser(userId);
					return true;
				}
				
				Users[userId].clientId = userId;
				Users[userId].LastReceive = sServer.ServerTime;
			}
		}
		break;

		case WSA_READ:
		{
			user = GetUserBySocket(wParam);
			if (user == NULL)
			{
				closesocket(wParam);
				break;
			}

			if (WSAGETSELECTEVENT(lParam) != FD_READ)
			{
				Log(user->clientId, LOG_INGAME, "Recebeu lParam (WndProc) como %hu. Erro possível: %hu. Desconectando o usuário.", WSAGETSELECTEVENT(lParam), WSAGETSELECTERROR(lParam));

				CloseUser(user->clientId);
				break;
			}

			int ret = user->Receive();
			if (ret == FALSE)
			{
				ret = WSAGetLastError();

				if (user->User.Username[0])
					Log(user->clientId, LOG_INGAME, "Usuário foi desconectado. Recebeu o erro %d no Receive", ret);
				else
					Log(SERVER_SIDE, LOG_INGAME, "Um usuário foi desconectado pois recebeu o erro %d", ret);

				CloseUser(user->clientId);
				break;
			}
			else if (ret == -1)
			{
				ret = WSAGetLastError();

				if (user->User.Username[0])
					Log(user->clientId, LOG_INGAME, "Usuário foi desconectado. Recebeu o erro %d no Receive", ret);
				else
					Log(SERVER_SIDE, LOG_INGAME, "Um usuário foi desconectado pois recebeu o erro %d", ret);

				CloseUser(user->clientId);
				break;
			}

			int Error = FALSE;
			int ErrorCode = FALSE;
			while (TRUE)
			{
				char* Msg = user->ReadMessage(&Error, &ErrorCode);
				if (Msg == NULL)
					break;

				if (Error == 2)
				{
					CloseUser(user->clientId);

					Log(user->clientId, LOG_INGAME, "Desconectado por Error 2 - ReadMessage");
					break;
				}

				user->PacketControl((BYTE*)Msg, ErrorCode);
			}
		}
		break;

		case WM_COMMAND:
		{
			if ((HWND)lParam == GUI.hButton[0])
			{
				bool success = true;

				if (!LoadConfig())
					success = false;
				else if (!LoadDataServer())
					success = false;
				/*else if (!ReadMySQL())
					success = false;
				else if (!ConnectSQL())
					success = false;*/
				else if (!CreateServer())
					success = false;
				else if (!ConnectDB())
					success = false;

				if (success)
				{
					BASE_InitializeHitRate();
					ReadAttributeMap();
					ReadHeightMap();
					ReadLanguageFile();
					ReadItemList();
					ReadSkillData();
					ReadTeleport();
					ReadGameConfig();
					ReadNPCBase();
					ReadGuild();
					ReadInitItem();
					LoadGuild();
					ReadArenaConfig();
					for (UINT32 LOCAL_5 = 0; LOCAL_5 < sServer.InitCount; LOCAL_5++)
					{
						st_Item LOCAL_7;
						memset(&LOCAL_7, 0, sizeof st_Item);

						LOCAL_7.Index = g_pInitItem[LOCAL_5].Index;

						INT32 LOCAL_8 = CreateItem(g_pInitItem[LOCAL_5].PosX, g_pInitItem[LOCAL_5].PosY, &LOCAL_7, g_pInitItem[LOCAL_5].Rotate, 3);
						if (LOCAL_8 >= 4096 || LOCAL_8 <= 0)
							continue;

						INT32 LOCAL_9 = GetItemAbility(&LOCAL_7, EF_KEYID);
						if (LOCAL_9 != 0 && LOCAL_9 < 15)
						{
							int LOCAL_10;
							UpdateItem(LOCAL_8, 3, &LOCAL_10);
						}
					}

					// Cria as torres doidonas
					for (INT32 i = 0; i < 5; i++)
					{
						sGuildZone* zone = &g_pCityZone[i];

						st_Item item;
						memset(&item, 0, sizeof st_Item);

						item.Index = 3145 + (zone->win_count);

						if (ChargedGuildList[sServer.Channel - 1][i] != 0)
						{
							item.EF1 = 56;
							item.EFV1 = zone->owner_index / 257;

							item.EF2 = 57;
							item.EFV2 = zone->owner_index;

							item.EF3 = 59;
							item.EFV3 = Rand() % 255;
						}

						CreateItem(zone->tower_x, zone->tower_y, &item, 3, 0);
					}

					SetArenaDoor(3);

					// Lê os arquivos de itens
					mGener.ReadNPCGener();
				}
				else
				{
					if (sServer.Socket != INVALID_SOCKET)
						closesocket(sServer.Socket);

					WSACleanup();
				}
			}
		}
		break;


		case WM_SIZE:
			// Redesenhar a configuração quando a janela for redimensionada
			DrawConfig();
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
		// Event to destroy GUI
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
			// Default value - Probably when user use mouse in form
		default:
			break;
		}
	}
	catch (std::exception& e)
	{
		printf("Excecao em WndProc %s", e.what());
	}   
	
	return DefWindowProc(hWnd, Message, wParam, lParam);

}
void TextOutWind(char* str, int color)
{
	SetTextColor(hDC, color);
	char* nextToken = NULL;
	char* token = strtok_s(str, "\n", &nextToken);
	while (token != NULL) {
		TextOutA(hDC, x, y, token, strlen(token));
		y += 16; // Ou qualquer outro valor de deslocamento vertical desejado
		token = strtok_s(NULL, "\n", &nextToken);
	}
}

void DrawConfig()
{
	// Carrega as configurações do srv
	ReadGameConfigv2();
	ReadGameConfig();

	// Definição das cores
	int VERMELHO = RGB(255, 0, 0);
	int PRETO = RGB(0, 0, 0);
	int AZUL = RGB(0, 0, 255);

	// String para armazenar informações
	char String[1024];
	memset(String, 0, sizeof(String));

	// Configurações da fonte
	int TAMANHO_LETRA = 16; // Tamanho da letra em pontos
	int NEGRITO = FW_BOLD;  // Peso da fonte negrito

	// Variável para o status do boss Kefra
	int kefranameplay = sServer.KefraKiller;

	// Reinicialize todas as variáveis necessárias
	x = 0;
	y = 0;
	hDC = GetDC(GUI.hGUI);

	if (hDC == NULL)
		return;

	if (hFont == NULL)
		return;

	if (SelectObject(hDC, hFont) != NULL)
		h = (HFONT)SelectObject(hDC, hFont);

	SelectObject(hDC, CreateFont(TAMANHO_LETRA, 0, 0, 0, NEGRITO, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial"));

	// Configura a cor do título em vermelho
	SetTextColor(hDC, VERMELHO);
	TextOutWind((char*)"\n         ", 0);
	TextOutWind((char*)"SEJA FELIZ E AME SEU PRÓXIMO!", AZUL);

	// Início do bloco de informações do servidor
	TextOutWind((char*)"#Versao Cliente:", AZUL);
	sprintf_s(String, sizeof(String), "   Cliver atual [%d]", sServer.CliVer);
	TextOutWind(String, PRETO);
	// Fim do bloco

	// Configura a cor do título em vermelho para "Item Drop Bonus Settings"
	TextOutWind((char*)"#Item Drop Rate Settings:", AZUL);

	for (int i = 0; i < 4; i++)
	{
		int pos = 16 * i;
		sprintf_s(String, sizeof(String), "   %3d %3d %3d %3d %3d %3d %3d %3d   %3d %3d %3d %3d %3d %3d %3d %3d",
			Taxes[0 + pos], Taxes[1 + pos], Taxes[2 + pos], Taxes[3 + pos],
			Taxes[4 + pos], Taxes[5 + pos], Taxes[6 + pos], Taxes[7 + pos],
			Taxes[8 + pos], Taxes[9 + pos], Taxes[10 + pos], Taxes[11 + pos],
			Taxes[12 + pos], Taxes[13 + pos], Taxes[14 + pos], Taxes[15 + pos]);

		// Configura a cor do texto em preto para os valores de taxa de drop
		SetTextColor(hDC, PRETO);

		TextOutWind(String, PRETO);
	}
	// Fim do bloco de informações do servidor

	// Início do bloco de informações de bônus de experiência
	TextOutWind((char*)"#Bonus experiencia:", AZUL);
	sprintf_s(String, sizeof(String), "   %d%%", sServer.BonusEXP);
	TextOutWind(String, PRETO);
	// Fim do bloco de informações de bônus de experiência

	// Início do bloco
	TextOutWind((char*)"#Level Novato:", AZUL);
	sprintf_s(String, sizeof(String), "   %d", sServer.NewbieZone);
	TextOutWind(String, PRETO);
	// Fim do bloco

	// Início do bloco
	TextOutWind((char*)"#Kefra status:", AZUL);
	if (kefranameplay > 1)
		sprintf_s(String, sizeof(String), "   %s matou Kefra", g_pGuild[sServer.KefraKiller].Name.c_str());
	else
		sprintf_s(String, sizeof(String), "   Kefra esta vivo");
	TextOutWind(String, PRETO);
	// Fim do bloco

	// Início do bloco
	int HourArena1 = 11;
	int HourArena2 = 18;
	int HourArena3 = 22;
	TextOutWind((char*)"#Guerras:", AZUL);
	sprintf_s(String, sizeof(String), "   Torre: %d:00 - City: %d:00 - Noatun: %d:00 - RvR: %d:00 ", sServer.TowerWar.Hour, sServer.WeekHour, sServer.CastleHour, sServer.RvR.Hour);
	TextOutWind(String, PRETO);
	sprintf_s(String, sizeof(String), "   Arena Real(1): %d:00 - Arena Real(2): %d:00 - Arena Real(3): %d:00", HourArena1, HourArena2, HourArena3);
	TextOutWind(String, PRETO);
	// Fim do bloco

	// Início do bloco
	TextOutWind((char*)"#Mobs Lan [N] / Lan[M]:", AZUL);
	sprintf_s(String, sizeof(String), "   Lan[N]: %d - Lan[M] %d", sServer.LanHouseN.TotalToKill, sServer.LanHouseM.TotalToKill);
	TextOutWind(String, PRETO);
	// Fim do bloco

	// Início do bloco
	TextOutWind((char*)"#Limite Agua:", AZUL);
	sprintf_s(String, sizeof(String), "   %d", sServer.MaxWaterEntrance);
	TextOutWind(String, PRETO);
	// Fim do bloco

	// Início do bloco de informações de Refinação Abençoada
	TextOutWind((char*)"#Refinacao Abencoada:", AZUL);

	sprintf_s(String, sizeof(String), "   Lv 40 - 150: %d%%", sServer.RateRef[0]);
	TextOutWind(String, PRETO);

	sprintf_s(String, sizeof(String), "   Lv 150 - 170: %d%%", sServer.RateRef[1]);
	TextOutWind(String, PRETO);

	sprintf_s(String, sizeof(String), "   Lv 171 - 180: %d%%", sServer.RateRef[2]);
	TextOutWind(String, PRETO);

	sprintf_s(String, sizeof(String), "   Lv 181 - 191: %d%%", sServer.RateRef[3]);
	TextOutWind(String, PRETO);

	sprintf_s(String, sizeof(String), "   Lv 190 - 400: %d%%", sServer.RateRef[4]);
	TextOutWind(String, PRETO);
	// Fim do bloco de informações de Refinação Abençoada

	if (hFont && h)
		h = (HFONT)SelectObject(hDC, h);

	ReleaseDC(GUI.hGUI, hDC);
}
