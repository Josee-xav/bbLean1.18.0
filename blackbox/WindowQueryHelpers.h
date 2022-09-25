#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "BB.h"

typedef struct {
	int nAttribute;
	PVOID pData;
	ULONG ulDataSize;
} WINCOMPATTRDATA;
extern BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

extern BOOL(WINAPI* pIsShellManagedWindow)(HWND);
extern BOOL(WINAPI* pIsShellFrameWindow)(HWND);

extern HWND(WINAPI* pHungWindowFromGhostWindow)(HWND);
extern HWND(WINAPI* pGhostWindowFromHungWindow)(HWND);

namespace WindowQueryHelper {
	BOOL initWinHelper();

	bool inspectWindows10AppWindow(HWND hWnd, bool cloakTest);
	bool isAppWindow(HWND hwnd, bool checkWin10 = true);
	__int64 __fastcall IsTaskWindow(HWND a2);

}; // namespace WindowQueryHelper
