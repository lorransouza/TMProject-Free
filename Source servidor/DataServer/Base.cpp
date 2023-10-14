#include <io.h>
#include <fcntl.h>

#include "framework.h"
#include "DataServer.h"
#include "CPSock.h"
#include "CUser.h"
#include "Base.h"
#include "CFileDB.h"

STRUCT_GUI_INFO GUI;
stServer			cServer{};
CUser				pUser[MAX_SERVERNUMBER]{};
CPSock				Server[MAX_SERVERNUMBER]{};


unsigned char		LocalIP[4] = { 0, };

char				g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64] = { 0, };
INT32				ChargedGuildList[MAX_SERVERGROUP][5]{};

st_Mob				pBaseSet[4]{};

int Rand()
{
	static long long i = 115;

	i = ((i * 214013) + 253101111);
	return ((i >> 16) & 0x7FFF);
}

void GetFirstKey(const char* source, char* dest)
{
	if ((source[0] >= 'A' && source[0] <= 'Z') || (source[0] >= 'a' && source[0] <= 'z'))
	{
		dest[0] = source[0];
		dest[1] = 0;
	}
	else
		strncpy_s(dest, 4, "etc", 3);
}

int WriteAccount(char* file, stAccount Pointer)
{
	int Handle = _open(file, O_RDWR | O_CREAT | O_BINARY, _S_IREAD | _S_IWRITE);

	if (Handle == -1)
	{
		if (errno == EEXIST)
			Log("err writeaccount EEXIST", file, 0);
		if (errno == EACCES)
			Log("err writeaccount EACCES", file, 0);
		if (errno == EINVAL)
			Log("err writeaccount EINVAL", file, 0);
		if (errno == EMFILE)
			Log("err writeaccount EMFILE", file, 0);
		if (errno == ENOENT)
			Log("err writeaccount ENOENT", file, 0);
		else
			Log("err writeaccount UNKNOWN", file, 0);

		return FALSE;
	}

	int ret = _lseek(Handle, 0, SEEK_SET);

	if (ret == -1)
	{
		Log("err,writeaccount lseek fail", file, 0);
		_close(Handle);
		return FALSE;
	}

	ret = _write(Handle, &Pointer, sizeof(stAccount));

	if (ret == -1)
	{
		_close(Handle);

		Log("CreateAccount write fail", file, 0);

		if (errno == EEXIST)
			Log("CreateAccount EEXIST", file, 0);
		if (errno == EACCES)
			Log("CreateAccount EACCES", file, 0);
		if (errno == EINVAL)
			Log("CreateAccount EINVAL", file, 0);
		if (errno == EMFILE)
			Log("CreateAccount EMFILE", file, 0);
		if (errno == ENOENT)
			Log("CreateAccount ENOENT", file, 0);

		return FALSE;
	}

	_close(Handle);

	return TRUE;
}

int WriteChar(char* file, stCharInfo Pointer)
{
	int Handle = _open(file, O_RDWR | O_CREAT | O_BINARY, _S_IREAD | _S_IWRITE);

	if (Handle == -1)
	{
		if (errno == EEXIST)
			Log("err writeaccount EEXIST", file, 0);
		if (errno == EACCES)
			Log("err writeaccount EACCES", file, 0);
		if (errno == EINVAL)
			Log("err writeaccount EINVAL", file, 0);
		if (errno == EMFILE)
			Log("err writeaccount EMFILE", file, 0);
		if (errno == ENOENT)
			Log("err writeaccount ENOENT", file, 0);
		else
			Log("err writeaccount UNKNOWN", file, 0);

		return FALSE;
	}

	int ret = _lseek(Handle, 0, SEEK_SET);

	if (ret == -1)
	{
		Log("err,writeaccount lseek fail", file, 0);
		_close(Handle);
		return FALSE;
	}

	ret = _write(Handle, &Pointer, sizeof(stCharInfo));

	if (ret == -1)
	{
		_close(Handle);

		Log("CreateAccount write fail", file, 0);

		if (errno == EEXIST)
			Log("CreateAccount EEXIST", file, 0);
		if (errno == EACCES)
			Log("CreateAccount EACCES", file, 0);
		if (errno == EINVAL)
			Log("CreateAccount EINVAL", file, 0);
		if (errno == EMFILE)
			Log("CreateAccount EMFILE", file, 0);
		if (errno == ENOENT)
			Log("CreateAccount ENOENT", file, 0);

		return FALSE;
	}

	_close(Handle);

	return TRUE;
}

int ReadAccount(char* file, stAccount* Pointer)
{
	int Handle = _open(file, O_RDONLY | O_BINARY);

	if (Handle == -1)
	{
		if (errno == EINVAL)
			Log("err readaccount EINVAL", file, 0);
		else if (errno == EMFILE)
			Log("err readaccount EEMFILE", file, 0);
		else
			Log("err readaccount UNKNOW", file, 0);

		return FALSE;
	}

	int length	= _filelength(Handle);
	int reqsz	= sizeof(stAccount);
	int ret		= _read(Handle, Pointer, reqsz);

	if (ret == -1)
	{
		_close(Handle);
		return FALSE;
	}

	_close(Handle);
	return TRUE;
}

int ReadChar(char* file, stCharInfo* Pointer)
{
	int Handle = _open(file, O_RDONLY | O_BINARY);

	if (Handle == -1)
	{
		if (errno == EINVAL)
			Log("err readaccount EINVAL", file, 0);
		else if (errno == EMFILE)
			Log("err readaccount EEMFILE", file, 0);
		else
			Log("err readaccount UNKNOW", file, 0);

		return FALSE;
	}

	int length = _filelength(Handle);
	int reqsz = sizeof(stCharInfo);
	int ret = _read(Handle, Pointer, reqsz);

	if (ret == -1)
	{
		_close(Handle);
		return FALSE;
	}

	_close(Handle);
	return TRUE;
}



void ClearItem(st_Item* item)
{
	item->Index = 0;
	item->EF1 = 0;
	item->EF2 = 0;
	item->EF3 = 0;
	item->EFV1 = 0;
	item->EFV2 = 0;
	item->EFV3 = 0;
}

void ClearMob(st_Mob* player)
{
	memset(player, 0, sizeof st_Mob);

	player->Last.X = 2100;
	player->Last.Y = 2100;

	memset(&player->bStatus, 0, sizeof st_Status);
	memset(&player->Status, 0, sizeof st_Status);

	for (INT32 i = 0; i < 18; i++)
		ClearItem(&player->Equip[i]);

	for (INT32 i = 0; i < 64; i++)
		ClearItem(&player->Inventory[i]);

	player->SkillBar1[0] = -1;
	player->SkillBar1[1] = -1;
	player->SkillBar1[2] = -1;
	player->SkillBar1[3] = -1;
}

double TimeRemaining(int dia, int mes, int ano, int hora, int min)
{
	time_t rawnow = time(NULL);
	struct tm now;
	localtime_s(&now, &rawnow);

	int month = now.tm_mon; //0 Janeiro, 1 Fev
	int day = now.tm_mday;
	int year = now.tm_year;

	struct std::tm a = { 0, now.tm_min, now.tm_hour, day, month, year };
	struct std::tm b = { 0, min, hora, dia, mes - 1, ano - 1900 };

	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	if (x != (std::time_t)(-1) && y != (std::time_t)(-1))
	{
		double difference = (std::difftime(y, x) / (60.0 * 60.0 * 24.0));
		return difference;
	}

	return 0;
}

INT32 ReadConfig()
{
	FILE* pFile = NULL;
	fopen_s(&pFile, "Config.txt", "rt");

	if (pFile)
	{
		char str[255] = { 0 };
		while (fgets(str, 255, pFile))
		{
			char cmd[64];
			int value = 0;
			if (fscanf_s(pFile, "%s %d", cmd, 64, &value) != 2)
				continue;

			std::string command{ cmd };
			if (command == "Sapphire")
				cServer.Sapphire = value;
			else if (command == "LastSeal")
				cServer.LastSeal = value;
			/*else if (command == "LastMerida")
				cServer.LastMerida = value;*/
		}

		fclose(pFile);
		return true;
	}

	return false;
}

INT32 WriteConfig()
{
	FILE* pFile = NULL;
	fopen_s(&pFile, "Config.txt", "w+");

	if (pFile)
	{
		fprintf(pFile, "Sapphire %d\r\n", cServer.Sapphire);
		fprintf(pFile, "LastSeal %d\r\n", cServer.LastSeal);
		/*fprintf(pFile, "LastMerida %d\r\n", cServer.LastMerida);*/

		fclose(pFile);
		return true;
	}

	return false;
}

int ReadCharBase(int index, const char* file)
{
	FILE* pFile = NULL;

	fopen_s(&pFile, file, "rb");
	if (!pFile)
		return FALSE;

	fread(&pBaseSet[index], sizeof(st_Mob), 1, pFile);
	fclose(pFile);

	return TRUE;
}

INT32 InitializeBaseDef()
{
	memset(ChargedGuildList, 0, sizeof ChargedGuildList);
	return InitializeServerList();
}

INT32 InitializeServerList()
{
	FILE* pFile = NULL;
	fopen_s(&pFile, "serverlist.txt", "rt");

	if (!pFile)
		fopen_s(&pFile, "../../serverlist.txt", "rt");

	if (!pFile)
	{
		MessageBox(NULL, "Can't open serverlist.txt", "-system", MB_OK | MB_APPLMODAL);
		return FALSE;
	}

	memset(g_pServerList, 0, sizeof g_pServerList);

	char str[256];
	char address[64];
	while (1)
	{
		char* ret = fgets(str, 255, pFile);
		if (ret == NULL)
			break;

		INT32 serverGroup = -1;
		INT32 serverNumber = -1;
		address[0] = 0;

		sscanf_s(str, "%d %d %s", &serverGroup, &serverNumber, address, 64);

		if (serverGroup < 0 || serverGroup >= MAX_SERVERGROUP || serverNumber < 0 || serverNumber >= MAX_SERVERNUMBER)
			continue;

		strncpy_s(g_pServerList[serverGroup][serverNumber], address, 64);
	}

	return TRUE;
}

INT32 GetUserFromSocket(INT32 soc)
{
	for (INT32 i = 0; i < MAX_SERVERGROUP; i++)
	{
		if (pUser[i].Sock.Sock == soc)
			return i;
	}

	return -1;
}

void ProcessClientMessage(INT32 conn, char* msg)
{
	cFileDB.ProcessMessage(msg, conn);
}

