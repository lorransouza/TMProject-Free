#include "cServer.h"
#include "Basedef.h"
#include <fcntl.h>
#include <io.h>
#include "UOD_SEHTranslator.h"

void CreateConsole()
{
	AllocConsole();

	// Output fix 
	freopen_s((FILE * *)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE * *)stdin, "CONIN$", "r", stdin);
}

void myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function,
   const wchar_t* file,
   unsigned int line,
   uintptr_t pReserved)
{
   wprintf(L"Invalid parameter detected in function %s."
            L" File: %s Line: %d\n", function, file, line);
   wprintf(L"Expression: %s\n", expression);
}

int __stdcall WinMain(  HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	setlocale(LC_ALL, "Portuguese");
	TOD_SEHTranslator sehTranslator{};

	//CreateConsole();

	// Códido importante. Erros de parâmetros removem o "abort" dos mesmos.
	// Ler sobre esta API
	_invalid_parameter_handler oldHandler, newHandler;
	newHandler = myInvalidParameterHandler;
	oldHandler = _set_invalid_parameter_handler(newHandler);

	_setmaxstdio(8192);

	CreateGUI();

	while (true) 
	{
		try 
		{
			MSG Message{};
			while (GetMessage(&Message, NULL, NULL, NULL))
			{
				TranslateMessage(&Message);
				DispatchMessageA(&Message);
			}
		}
		catch (std::exception& e)
		{
			Log(SERVER_SIDE, LOG_INGAME, "Exception ocorrida: %s", e.what());
		}
	}
}