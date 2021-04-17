/* ==========================================================================

  This file is part of the bbLean source code
  Copyright � 2001-2003 The Blackbox for Windows Development Team
  Copyright � 2004-2009 grischka

  http://bb4win.sourceforge.net/bblean
  http://developer.berlios.de/projects/bblean
  http://blackbox4windows.com

  bbLean is free software, released under the GNU General Public License
  (GPL version 2). For details see: http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  ========================================================================== */
/* BB.h - global defines aside from the plugin-api */
#pragma once

/* Always includes */
#include "BBApi.h"
#include "win0x500.h"       // some later windows APIs
#include "bblib.h"
#include <cassert>
#define NO_INTSHCUT_GUIDS // Don't include internet shortcut GUIDs
#define NO_SHDOCVW_GUIDS // Don't include 'FolderItem' with shlobj.h

// ==============================================================
/* compiler specifics */

#ifdef __BORLANDC__
  #define TRY __try
  #define EXCEPT(n) __except(n)
  #define FINALLY(n) __finally(n)
#endif

#ifdef _MSC_VER
  #define TRY __try
  #define EXCEPT(n) __except(n)
  #define FINALLY(n) __finally(n)
//  #pragma warning(disable: 4100) /* unreferenced function parameter */
#endif

#ifndef TRY
  #define TRY if (1)
  #define EXCEPT(n) if (0)
  #define FINALLY if (0)
#endif

#define SIZEOFPART(s,e) (offsetof(s,e) + sizeof(((s*)0)->e))

#ifdef __BBCORE__
#include "DrawText.h"
// ==============================================================
/* global variables */

extern HINSTANCE hMainInstance;
extern HWND BBhwnd;
extern DWORD BBThreadId;
extern OSVERSIONINFO osInfo;
extern unsigned g_WM_ShellHook;
extern int VScreenX, VScreenY, VScreenWidth, VScreenHeight;
extern bool g_usingNT, g_usingXP, g_usingVista, g_usingWin7;
extern bool g_underExplorer;
extern bool g_multimon;
extern bool g_bbactive;
extern BOOL (WINAPI* pSwitchToThisWindow) (HWND, int);
extern BOOL (WINAPI* pTrackMouseEvent) (LPTRACKMOUSEEVENT lpEventTrack);
extern BOOL (WINAPI* pAllowSetForegroundWindow) (DWORD dwProcessId);
extern DWORD (WINAPI* pGetLongPathName) (LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);

extern char pluginrc_path[];
extern char defaultrc_path[];

// ==============================================================
/* definitions */

#include "bbversion.h"

/* Blackbox window timers */
#define BB_RUNSTARTUP_TIMER     1
#define BB_ENDSTARTUP_TIMER     2
#define BB_TASKUPDATE_TIMER     3

/* SetDesktopMargin internal flags */
#define BB_DM_REFRESH -1
#define BB_DM_RESET -2

/* resource.rc */
#define IDI_BLACKBOX 101 // blackbox.ico
#define IDC_MOVEMENU 102 // MoveMenu.cur
#define IDC_MOVEMENUL 104 // MoveMenuL.cur
#define IDC_EDITBOX 103 // MoveMenu.cur


// ==============================================================
/* workspaces and tasks */

struct hwnd_list
{
    struct hwnd_list * next;
		HWND hwnd;
};
void get_window_icon (HWND hwnd, HICON * picon);
void get_window_text (HWND hwnd, char * buffer, int size);
void getWindowText (HWND hwnd, WCHAR * buffer, size_t size);

// ==============================================================
/* BBApi.cpp - some (non api) utils */

BOOL BBExecute_string (const char * s, int flags);
int BBExecute_pidl (const char * verb, const void * pidl);

/* drawing */
void read_pix ();
void reset_pix ();
void unregister_fonts ();
void register_fonts ();
void arrow_bullet (HDC buf, int x, int y, int d);
//void draw_line_h (HDC hDC, int x1, int x2, int y, int w, COLORREF C);
char * replace_arg1 (const char * fmt, const char * in);
COLORREF Settings_MapShadowColor (StyleItem *si, StyleItem *ri);
//void BBDrawText(HDC hDC, LPCTSTR lpString, LPRECT rc, UINT uFormat, StyleItem * pSI);

/* other */
int BBMessageBox (int flg, const char * fmt, ...);
BOOL BBRegisterClass (const char * classname, WNDPROC wndproc, int flags);
#define BBCS_VISIBLE 1
#define BBCS_EXTRA 2
#define BBCS_DROPSHADOW 4
int EditBox (const char * caption, const char * message, const char * initvalue, char * buffer);
int GetOSVersion(void);

/* Logging */
void _log_printf(int flag, const char *fmt, ...);
#define LOG_PLUGINS 1
#define LOG_STARTUP 2
#define LOG_TRAY 4
#define LOG_SHUTDOWN 8
#define LOG_GRADIENTS 16
#define log_printf(args) _log_printf args

void bb_rcreader_init(void);

// ==============================================================
/* Blackbox.cpp */

void post_command(const char *cmd);
void post_command_fmt(const char *fmt, ...);
int get_workspace_number(const char *s);
void set_opaquemove(int);
void set_focus_model(const char *fm_string);

/* Menu */
bool Menu_IsA(HWND hwnd);

// ==============================================================
/* Native Language Support (see code in utils.cpp) */

#define BBOPT_SUPPORT_NLS
#ifdef BBOPT_SUPPORT_NLS
	const char *nls1(const char *p);
	const char *nls2a(const char *i, const char *p);
	const char *nls2b(const char *p);
	void free_nls(void);
#	define NLS0(S) S
#	define NLS1(S) nls1(S)
#	define NLS2(I,S) nls2b(I S)
#else
#	define free_nls()
#	define NLS0(S) S
#	define NLS1(S) (S)
#	define NLS2(I,S) (S)
#endif

#endif /*def __BBCORE__ */



// ==============================================================
/* Some enumeration function */

typedef BOOL (*TASKENUMPROC)(tasklist const *, LPARAM);
void EnumTasks (TASKENUMPROC lpEnumFunc, LPARAM lParam);

typedef BOOL (*DESKENUMPROC)(DesktopInfo const *, LPARAM);
void EnumDesks (DESKENUMPROC lpEnumFunc, LPARAM lParam);

typedef BOOL (*TRAYENUMPROC)(systemTray const *, LPARAM);
void EnumTray (TRAYENUMPROC lpEnumFunc, LPARAM lParam);

struct PluginList;
typedef BOOL (*PLUGINENUMPROC)(PluginList const *, LPARAM);
void EnumPlugins (PLUGINENUMPROC lpEnumFunc, LPARAM lParam);
