// DataServer.cpp : Define o ponto de entrada para o aplicativo.
//

#include "framework.h"

#include "CPSock.h"
#include "CUser.h"
#include "DataServer.h"
#include "Base.h"
#include "CFileDB.h"
#include <io.h>
#include <sStruct.h>



HINSTANCE					hInst;
HWND						hWndMain;
WCHAR						szTitle[MAX_LOADSTRING];
WCHAR						szWindowClass[MAX_LOADSTRING];

std::unique_ptr<doLog>		serverLog;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_DATASERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DATASERVER));

	MSG msg;

	serverLog = std::make_unique<doLog>("..\\Logs\\DataBase");

	if (!InitializeBaseDef())
	{
		MessageBoxA(NULL, "err, InitializeBaseDef fail", "-system", MB_OK);
		return false;
	}

	if (!ReadCharBase(0, "npc\\TK"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo TRANSKNIGHT", "Error : read", MB_OK);
		return false;
	}
	else if (!ReadCharBase(1, "npc\\FM"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo FOEMA", "Error : read", MB_OK);
		return false;
	}
	else if (!ReadCharBase(2, "npc\\BM"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo BEASTMASTER", "Error : read", MB_OK);
		return false;
	}
	else if (!ReadCharBase(3, "npc\\HT"))
	{
		MessageBoxA(NULL, "Nâo foi possível ler o arquivo HUNTER", "Error : read", MB_OK);
		return false;
	}
	
	INT32 wsaInitialize = Server[0].WSAInitialize();
	if (wsaInitialize == 0)
	{
		MessageBoxA(NULL, "err, wsainitialize fail", "-system", MB_OK);
		return false;
	}
	
	char name[255];
	if (!gethostname(name, 255))
	{
		hostent* hretn = gethostbyname(name);
		if (hretn)
		{
			char* cIp = inet_ntoa(*(struct in_addr*)*hretn->h_addr_list);

			sscanf_s(cIp, "%hhd %hhd %hhd %hhd", &LocalIP[0], &LocalIP[1], &LocalIP[2], &LocalIP[3]);
			strncpy_s(name, cIp, 255);
		}
	}

	if (!LocalIP[0])
	{
		MessageBox(NULL, "Can't get local address", "Reboot error", MB_OK);
		return false;
	}

	for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
	{
		if (!strcmp(g_pServerList[i][0], name))
		{
			cServer.ServerIndex = i;
			break;
		}
	}

	if (cServer.ServerIndex == -1)
	{
		MessageBoxA(NULL, "Can't get Server Group Index LOCALIP:", "", MB_OK | MB_SYSTEMMODAL);
		return true;
	}


	ReadConfig();
	Server[0].ListenServer(hWndMain, 0, 7514, WSA_ACCEPT);

	Log("Dbsrv Iniciado com sucesso");
	SetTimer(hWndMain, TIMER_SEC, 1000, NULL);

	// Loop de mensagem principal:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DATASERVER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DATASERVER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Armazenar o identificador de instância em nossa variável global

	HWND hWnd = CreateWindowW(szWindowClass, (LPCWSTR)"DataServer - WYD2 v759", WS_OVERLAPPED | WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, 680, 420, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	hWndMain = hWnd;

	return TRUE;
}

void Log(const char* msg, ...)
{
	if (!(bool)serverLog)
		return;

	// Inicia a lista de argumentos
	va_list arglist;
	va_start(arglist, msg);

	serverLog->Log(msg, arglist);
	va_end(arglist);
}

void ImportUser()
{
	char line[256];
	char path[256];
	char arq[256];
	strncpy_s(path, "./ImportUser/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(path, &file);

	if (findFile == -1)
		return;

	do
	{
		if (file.name[0] == '.')
		{
			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;

			continue;
		}

		sprintf_s(arq, "./ImportUser/%s", file.name);
		FILE* pFile;
		fopen_s(&pFile, arq, "rt");

		if (pFile != NULL)
		{
			char* result = fgets(line, 127, pFile);
			if (result == 0)
			{
				fclose(pFile);

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			char user[16];
			char pass[16];

			INT32 access = 0;
			INT32 ret = sscanf_s(line, "%s %s %d", user, 16, pass, 12, &access);
			if (ret != 2 && ret != 3)
			{
				fclose(pFile);

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			fclose(pFile);

			_strupr_s(user);

			stAccountDB account;
			memset(&account, 0, sizeof account);

			strncpy_s(account.Account.Username, user, 16);
			strncpy_s(account.Account.Password, pass, 12);

			if (access != 0)
				account.Account.AccessLevel = access;

			INT32 suc = cFileDB.DBWriteAccount(&account);
			if (suc)
				DeleteFileA(arq);

			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;
		}
	} while (true);

	if (findFile != -1)
		_findclose(findFile);
}

void ImportPass()
{
	char line[256] = { 0 };
	char path[256] = { 0 };
	char arq[256] = { 0 };
	strncpy_s(path, "./ImportPass/*.txt", 256);

	struct _finddata_t file;
	INT32 findFile = _findfirst(path, &file);

	if (findFile == -1)
		return;

	do
	{
		if (file.name[0] == '.')
		{
			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;

			continue;
		}

		sprintf_s(arq, "./ImportPass/%s", file.name);
		FILE* pFile;
		fopen_s(&pFile, arq, "rt");

		if (pFile != NULL)
		{
			char* result = fgets(line, 127, pFile);
			if (result == 0)
			{
				fclose(pFile);

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			fclose(pFile);

			char user[16] = { 0 };
			char pass[16] = { 0 };
			char num[16] = { 0 };

			INT32 ret = sscanf_s(line, "%s %s %s", user, 16, pass, 16, num, 6);
			if (ret != 2 && ret != 3)
			{
				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			INT32 idxName = cFileDB.GetIndex(user);
			if (idxName > 0 && idxName < MAX_DBACCOUNT)
			{
				INT32 srv = idxName / MAX_PLAYER;
				INT32 id = idxName % MAX_PLAYER;

				cFileDB.SendLog(srv, id, "A conta estava online. Não é possível trocar a senha.");

				INT32 t = _findnext(findFile, &file);
				if (t != 0)
					break;

				continue;
			}

			stAccountDB account{};
			_strupr_s(user);
			memcpy(account.Account.Username, user, 16);

			INT32 readedAcc = cFileDB.DBReadAccount(&account);
			if (readedAcc == 0)
			{
				INT32 tryFindFile = _findnext(findFile, &file);
				if (tryFindFile != 0)
					break;

				continue;
			}

			std::stringstream str;
			str << "A senha da conta foi trocada para " << pass;

			strncpy_s(account.Account.Password, pass, 16);

			if (num[0])
			{
				strncpy_s(account.Account.SecondPass, num, 6);

				str << std::endl << "A senha numérica foi trocada para " << num;
			}

			INT32 suc = cFileDB.DBWriteAccount(&account);
			if (suc)
				DeleteFileA(arq);

			Log(account.Account.Username, str.str().c_str());

			INT32 t = _findnext(findFile, &file);
			if (t != 0)
				break;
		}
	} while (true);

	if (findFile != -1)
		_findclose(findFile);
}

//pCOF
void ImportItem() 
{
	char pathSucess[256];
	char temp[256];
	char pathError[256];
	strncpy_s(temp, "./ImportItem/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(temp, &file);
	if (findFile == -1)
		return;

	INT32 serverId = 0;
	while (true)
	{
		serverId++;
		if (serverId > MAX_SERVERGROUP)
			break;

		if (file.name[0] == '.')
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sprintf_s(pathSucess, "./ImportItem/%s", file.name);
		sprintf_s(pathError, "./ImportItem/err/%s", file.name);

		FILE* pFile;
		fopen_s(&pFile, pathSucess, "rt");
		if (pFile == nullptr)
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		char accountName[256];
		accountName[0] = 0; // EBP - 52C

		INT32 itemId = 0,
			ef1 = 0,
			ef2 = 0,
			ef3 = 0,
			efv1 = 0,
			efv2 = 0,
			efv3 = 0;

		char* line = fgets(temp, 256, pFile);
		if (line == 0)
		{
			fclose(pFile);

			MoveFileA(pathSucess, pathError);
			//Log(-import no contents
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sscanf_s(temp, "%s %d %d %d %d %d %d %d", accountName, 16, &itemId, &ef1, &efv1, &ef2, &efv2, &ef3, &efv3);
		fclose(pFile);

		//Log import starting
		_strupr_s(accountName);
		if (itemId < 0) //|| LOCAL_332[0] >= 6500 || LOCAL_332[1] < 0 || LOCAL_332[3] < 0 || LOCAL_332[5] < 0 || LOCAL_332[1] > 255 || LOCAL_332[3] > 255 || LOCAL_332[5] > 255)
		{
			MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		bool runTimeSent = false;
		bool success = false;
		INT32 idxName = cFileDB.GetIndex(accountName);
		if (idxName > 0 && idxName < MAX_DBACCOUNT && cFileDB.Account[idxName].Slot >= 0 && cFileDB.Account[idxName].Slot <= 3)
		{
			INT32 srv = idxName / MAX_PLAYER;
			INT32 id = idxName % MAX_PLAYER;

			pCOF packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xC0F;
			packet.Header.Size = sizeof pCOF;
			packet.Header.ClientId = id;

			strncpy_s(packet.Username, accountName, 16);

			packet.item.Index = itemId;
			packet.item.Effect[0].Index = ef1;
			packet.item.Effect[0].Value = efv1;
			packet.item.Effect[1].Index = ef2;
			packet.item.Effect[1].Value = efv2;
			packet.item.Effect[2].Index = ef3;
			packet.item.Effect[2].Value = efv3;

			pUser[srv].Sock.SendOneMessage((char*)&packet, sizeof pCOF);
			runTimeSent = 1;

			std::stringstream str;
			str << "Recebido o item através da DBsrv " << packet.item.toString().c_str();

			cFileDB.SendLog(srv, id, str.str().c_str());
		}

		if (!runTimeSent)
		{
			stAccountDB account{};
			_strupr_s(accountName);
			memcpy(account.Account.Username, accountName, 16);

			INT32 readedAccount = cFileDB.DBReadAccount(&account);
			if (readedAccount == 0)
			{
				// Log err- import item success - account save fail
				MoveFileA(pathSucess, pathError);

				INT32 tryFindNext = _findnext(findFile, &file);
				if (tryFindNext != 0)
					break;

				continue;
			}


			st_Item item{};
			item.Index = itemId;
			item.Effect[0].Index = ef1;
			item.Effect[0].Value = efv1;
			item.Effect[1].Index = ef2;
			item.Effect[1].Value = efv2;
			item.Effect[2].Index = ef3;
			item.Effect[2].Value = efv3;

			INT32 pos = -1;
			for (INT32 i = 0; i < 120; i++)
			{
				if (account.Account.Storage.Item[i].Index != 0)
					continue;

				pos = i;
				break;
			}

			if (pos == -1)
			{
				INT32 moveRet = MoveFileA(pathSucess, pathError);
				Log(accountName, "Sem espaço para receber o item %s.", item.toString().c_str());

				if (moveRet == 0)
				{
					Log("no space, move failed. Arquivo: %s. Item: %s GetLastError: %d.\n", pathSucess, item.toString().c_str(), GetLastError());
					Log(accountName, "no space, move failed. Arquivo %s. Item: %s. GetLastError %s.", pathSucess, item.toString().c_str(), GetLastError());
				}
				else Log("no space, move failed. Arquivo: %s. Item: %s GetLastError: %d.\n", pathSucess, item.toString().c_str(), GetLastError());

				INT32 tryFindNext = _findnext(findFile, &file);
				if (tryFindNext != 0)
					break;

				continue;
			}

			account.Account.Storage.Item[pos] = item;
			Log(accountName, "Item %s entregue para o personagem", item.toString().c_str());

			INT32 tryReadAccount = cFileDB.DBWriteAccount(&account);
			if (tryReadAccount == 0)
			{
				// Log err- import item success - account save fail
				if (runTimeSent == 0)
					MoveFileA(pathSucess, pathError);

				tryReadAccount = _findnext(findFile, &file);
				if (tryReadAccount != 0)
					break;
			}
		}

		auto tryReadAccount = DeleteFileA(pathSucess);
		if (tryReadAccount == 0)
		{
			if (!MoveFileA(pathSucess, pathError))
			{
				printf("Import item sucess - file not deleted\n");
			}
			else
			{
				printf("Import item sucess - file moved WARNING\n");
			}
		}

		tryReadAccount = _findnext(findFile, &file);
		if (tryReadAccount != 0)
			break;
	}

	if (findFile != -1)
		_findclose(findFile);
}
//pC10
void ImportBan()
{
	char pathSucess[256];
	char temp[256];
	char pathError[256];
	strncpy_s(temp, "./ImportBan/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(temp, &file);

	if (findFile == -1)
		return;

	INT32 serverId = 0;
	while (true)
	{
		serverId++;
		if (serverId > MAX_SERVERGROUP)
			break;

		if (file.name[0] == '.')
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sprintf_s(pathSucess, "./ImportBan/%s", file.name);
		sprintf_s(pathError, "./ImportBan/err/%s", file.name);

		FILE* pFile;
		fopen_s(&pFile, pathSucess, "rt");
		if (pFile == 0)
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		char accountName[256] = { 0 };

		INT32 type = 0, day = 0, mounth = 0, year = 0;
		char* line = fgets(temp, 256, pFile);
		if (line == 0)
		{
			fclose(pFile);

			MoveFileA(pathSucess, pathError);
			//Log(-import no contents
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sscanf_s(temp, "%s %d %d/%d/%d", accountName, 16, &type, &day, &mounth, &year);
		fclose(pFile);

		//Log import starting
		_strupr_s(accountName);
		if (type < 0)
		{
			MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		time_t rawnow = time(NULL);
		struct tm now;
		localtime_s(&now, &rawnow);

		INT32 runTimeSent = 0;
		INT32 idxName = cFileDB.GetIndex(accountName);
		// Dá DC no hotário
		if (idxName > 0 && idxName < MAX_DBACCOUNT)
		{
			INT32 srv = idxName / MAX_PLAYER;
			INT32 id = idxName % MAX_PLAYER;

			pC10 packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xC10;
			packet.Header.Size = sizeof pC10;
			packet.Header.ClientId = id;

			packet.BanType = type;
			packet.Ban.Dia = day;
			packet.Ban.Mes = mounth;
			packet.Ban.Ano = year;
			packet.Ban.Minuto = now.tm_min;
			packet.Ban.Hora = now.tm_hour;
			packet.Ban.Segundo = now.tm_sec;
			strncpy_s(packet.Username, accountName, 16);

			pUser[srv].Sock.SendOneMessage((char*)&packet, sizeof pC10);
			runTimeSent = 1;
		}

		stAccountDB account;

		memset(&account, 0, sizeof stAccount);
		_strupr_s(accountName);
		memcpy(account.Account.Username, accountName, 16);

		INT32 readedAccount = cFileDB.DBReadAccount(&account);
		if (readedAccount == 0)
		{
			if (runTimeSent == 0)
				MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		account.Account.BanType = type;
		account.Account.Ban.Dia = day;
		account.Account.Ban.Mes = mounth;
		account.Account.Ban.Ano = year;
		account.Account.Ban.Minuto = now.tm_min;
		account.Account.Ban.Hora = now.tm_hour;
		account.Account.Ban.Segundo = now.tm_sec;


		INT32 tryReadAccount = cFileDB.DBWriteAccount(&account);
		if (tryReadAccount == 0)
		{
			// Log err- import item success - account save fail
			if (runTimeSent == 0)
				MoveFileA(pathSucess, pathError);

			tryReadAccount = _findnext(findFile, &file);
			if (tryReadAccount != 0)
				break;
		}

		tryReadAccount = DeleteFileA(pathSucess);
		if (tryReadAccount == 0)
		{
			if (!MoveFileA(pathSucess, pathError))
			{
				printf("Import Ban sucess - file not deleted\n");
			}
			else
			{
				printf("Import Ban sucess - file moved WARNING\n");
			}
		}

		tryReadAccount = _findnext(findFile, &file);
		if (tryReadAccount != 0)
			break;
	}

	if (findFile != -1)
		_findclose(findFile);
}
//pCOE
void ImportCash()
{
	char pathSucess[256];
	char temp[256];
	char pathError[256];
	strncpy_s(temp, "./ImportCash/*.txt", 20);

	struct _finddata_t file;
	INT32 findFile = _findfirst(temp, &file);

	if (findFile == -1)
		return;

	INT32 serverId = 0;
	while (true)
	{
		serverId++;
		if (serverId > MAX_SERVERGROUP)
			break;

		if (file.name[0] == '.')
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sprintf_s(pathSucess, "./ImportCash/%s", file.name);
		sprintf_s(pathError, "./ImportCash/err/%s", file.name);

		FILE* pFile;
		fopen_s(&pFile, pathSucess, "rt");
		if (pFile == 0)
		{
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		char accountName[256];
		accountName[0] = 0; // EBP - 52C

		INT32 cash = 0;
		char* line = fgets(temp, 256, pFile);
		if (line == 0)
		{
			fclose(pFile);

			MoveFileA(pathSucess, pathError);
			//Log(-import no contents
			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		sscanf_s(temp, "%s %d", accountName, 16, &cash);

		fclose(pFile);

		//Log import starting
		_strupr_s(accountName);
		if (cash < 0) //|| LOCAL_332[0] >= 6500 || LOCAL_332[1] < 0 || LOCAL_332[3] < 0 || LOCAL_332[5] < 0 || LOCAL_332[1] > 255 || LOCAL_332[3] > 255 || LOCAL_332[5] > 255)
		{
			MoveFileA(pathSucess, pathError);

			INT32 tryFindNext = _findnext(findFile, &file);
			if (tryFindNext != 0)
				break;

			continue;
		}

		INT32 runTimeSent = 0;
		INT32 idxName = cFileDB.GetIndex(accountName);
		if (idxName > 0 && idxName < MAX_DBACCOUNT)
		{
			INT32 srv = idxName / MAX_PLAYER;
			INT32 id = idxName % MAX_PLAYER;

			pCOE packet;
			memset(&packet, 0, sizeof packet);

			packet.Header.PacketId = 0xC0e;
			packet.Header.Size = sizeof pCOE;
			packet.Header.ClientId = id;

			packet.Cash = cash;
			strncpy_s(packet.Username, accountName, 16);

			pUser[srv].Sock.SendOneMessage((char*)&packet, sizeof pCOE);
			runTimeSent = 1;

			std::stringstream str;
			str << "Enviado pela DBsrv " << cash;
			cFileDB.SendLog(srv, id, str.str().c_str());
		}
		else
		{
			stAccountDB account{};
			_strupr_s(accountName);
			memcpy(account.Account.Username, accountName, 16);

			INT32 readedAccount = cFileDB.DBReadAccount(&account);
			if (readedAccount == 0)
			{
				if (runTimeSent == 0)
					MoveFileA(pathSucess, pathError);

				INT32 tryFindNext = _findnext(findFile, &file);
				if (tryFindNext != 0)
					break;

				continue;
			}

			Log(accountName, "Foi entregue um total de %d cash para o usuário. Total: %d", cash, account.Account.Cash + cash);

			account.Account.Cash += cash;

			INT32 tryReadAccount = cFileDB.DBWriteAccount(&account);
			if (tryReadAccount == 0)
			{
				// Log err- import item success - account save fail
				if (runTimeSent == 0)
					MoveFileA(pathSucess, pathError);

				tryReadAccount = _findnext(findFile, &file);
				if (tryReadAccount != 0)
					break;
			}
		}

		bool wasDeleted = DeleteFileA(pathSucess) != 0;
		if (!wasDeleted)
		{
			auto le = GetLastError();
			if (!MoveFileA(pathSucess, pathError))
				Log("Import item sucess - file not deleted. Arquivo: %s. Error on delete: %d. Error on move: %d\n", pathSucess, le, GetLastError());
			else
				Log("Import item sucess - file moved WARNING Arquivo: %s\n. Error: %d", pathSucess, le);
		}

		auto nextAccount = _findnext(findFile, &file);
		if (nextAccount != 0)
			break;
	}

	if (findFile != -1)
		_findclose(findFile);
}