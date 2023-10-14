#pragma once

#include "resource.h"
#include "Log.h"

#define MAX_LOADSTRING			100
#define MAX_SERVER				2
#define MAX_SERVERGROUP			10
#define MAX_GUILD				65535
#define MAX_SERVERNUMBER		(MAX_SERVER + 3)
#define	MAX_ADMIN				10 
#define MAX_PLAYER				1000
#define MAX_DBACCOUNT			(MAX_PLAYER * MAX_SERVER)
#define	ACCOUNTNAME_LENGTH		16		// Max length of the account login
#define	ACCOUNTPASS_LENGTH		12      // Max length of the account password
#define TIMER_SEC				0

extern HINSTANCE hInst;                                // instância atual
extern WCHAR szTitle[MAX_LOADSTRING];                  // O texto da barra de título
extern WCHAR szWindowClass[MAX_LOADSTRING];            // o nome da classe da janela principal

extern int		x;
extern int		y;
extern HDC		hDC;
extern HFONT	hFont;
extern HFONT	h;
extern HWND		hWndMain;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void				DrawConfig();
void				Log(const char* msg, ...);
void                ImportUser();
void                ImportPass();
void                ImportBan();
void                ImportItem();
void                ImportCash();
