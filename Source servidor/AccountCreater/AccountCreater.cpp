// AccountCreater.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//
#include "../Common/stBase.h"
#include <io.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <direct.h>tes
#include <io.h>
#include <fcntl.h>

#define _read(fh, buff, count)          _read_nolock(fh, buff, count)
using namespace std;

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
		return FALSE;
	}

	int ret = _lseek(Handle, 0, SEEK_SET);

	if (ret == -1)
	{
		_close(Handle);
		return FALSE;
	}

	ret = _write(Handle, &Pointer, sizeof(stAccount));

	if (ret == -1)
	{
		_close(Handle);
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
		return FALSE;
	}

	int ret = _lseek(Handle, 0, SEEK_SET);

	if (ret == -1)
	{
		_close(Handle);
		return FALSE;
	}

	ret = _write(Handle, &Pointer, sizeof(stCharInfo));

	if (ret == -1)
	{
		_close(Handle);
		return FALSE;
	}

	_close(Handle);

	return TRUE;
}

int main()
{
	char login[16]{};
	char password[12]{};

	std::cout << "Informe o Usuário. \n";
	std::cin >> login;

	std::cout << "Informe a Senha. \n";
	std::cin >> password;


	std::cout << "Usuário: " << login << " Senha: " << password << "\n";

	char firstKey[10]{};

	memset(firstKey, 0, sizeof firstKey);
	GetFirstKey(login, firstKey);

	_strupr_s(login);


	char temp[128]{};

	sprintf_s(temp, "./account/%s/%s", firstKey, login);

	int check;
	check = mkdir(temp);

	bool exist = false;
	// check if directory is created or not
	if (check)
		exist = true;
	
	if (exist)
		printf("Conta já existe.");
	else
	{
		sprintf_s(temp, "./account/%s/%s/Chars", firstKey, login);
		mkdir(temp);

		stAccount account{};

		strcpy(account.Username, login);
		strcpy(account.Password, password);
		

		sprintf_s(temp, "./Account/%s/%s/Account.bin", firstKey, login);
		int retn = WriteAccount(temp, account);

		if (!retn)
			return FALSE;

		for (int i = 0; i < 4; i++)
		{
			stCharInfo mob{};

			sprintf_s(temp, "./Account/%s/%s/Chars/%d.bin", firstKey, login, i);
			retn = WriteChar(temp, mob);

			if (!retn)
				return FALSE;
		}
	}

	std::cout << "Conta criada com Sucesso.";
	int a = getchar();
}

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
