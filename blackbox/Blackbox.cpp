/* ==========================================================================

  This file is part of the bbLean source code
  Copyright © 2001-2003 The Blackbox for Windows Development Team
  Copyright © 2004-2009 grischka

  http://bb4win.sourceforge.net/bblean
  http://developer.berlios.de/projects/bblean

  bbLean is free software, released under the GNU General Public License
  (GPL version 2). For details see:

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  ========================================================================== */
#include <Windows.h>
#include "BB.h"
#include "Settings.h"
#include "MessageManager.h"
#include "PluginManager/PluginManager.h"
#include "Workspaces.h"
#include "Desk.h"
#include "Tray.h"
#include "Menu/MenuMaker.h"
#include "BBSendData.h"
#include "BImage.h"
#include "bbshell.h"
#include "bbrc.h"
#include <logging.h>

#include "WindowQueryHelpers.h"

#include <mmsystem.h>
#include <process.h>
#include <tlhelp32.h>
#include <locale.h>

  //====================

const char szBlackboxName[] = "Blackbox";
const char szBlackboxClass[] = "BlackboxClass";

HINSTANCE hMainInstance;
HWND BBhwnd = 0;
DWORD BBThreadId = 0;
unsigned g_WM_ShellHook = 0;
bool g_usingNT = false;
bool g_usingXP = false;
bool g_usingVista = false;
bool g_usingWin7 = false;
bool g_usingWin8 = false;
bool g_underExplorer = false;
bool g_PluginsHidden = false;
bool g_multimon = false;
bool g_bbactive = false; /* blackbox currently in foreground */

static bool g_nostartup = false;
static BOOL g_save_opaquemove = false;
static RECT g_OldDT = RECT();
static bool g_shutting_down = false;
static bool g_in_restart = false;
static unsigned g_stack_top = 0; //@FIXME: 64bit!

//====================

static char blackboxrc_path[MAX_PATH];
static char extensionsrc_path[MAX_PATH];
static char menurc_path[MAX_PATH];
static char stylerc_path[MAX_PATH];
static void reset_rcpaths();
char pluginrc_path[MAX_PATH];
char defaultrc_path[MAX_PATH];

//====================

void startup_blackbox();
void shutdown_blackbox();
void start_plugins();
void kill_plugins();
void set_misc_options();

void RunStartupStuff();
bool StartupHasBeenRun();

void show_run_dlg();

void about_style();
void edit_file(int id, const char* path);
bool exec_broam(const char* command);
void exec_command(const char* cmd);
int ShutdownWindows(int state, int no_msg);

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

#define BBOPT_QUIET 1
#define BBOPT_PAUSE 1


// delay-load some functions on runtime to keep compatibility with win9x/nt4

// undocumented function calls
BOOL(WINAPI* pMSWinShutdown) (HWND);
BOOL(WINAPI* pRunDlg) (HWND parent, HICON, LPCSTR directory, LPCSTR title, LPCSTR message, UINT flags);
BOOL(WINAPI* pRegisterShellHook) (HWND, DWORD);
BOOL(WINAPI* pSwitchToThisWindow) (HWND, int);
BOOL(WINAPI* pAllowSetForegroundWindow) (DWORD dwProcessId);
BOOL(WINAPI* pShellDDEInit) (BOOL bInit);

// "SHDOCVW" (char*)0x76 || "SHELL32" (char*)0xBC
BOOL(WINAPI* pLockWorkStation) ();
// WtsApi.dll 2kpro +
BOOL(WINAPI* pWTSDisconnectSession) (HANDLE hServer, DWORD SessionId, BOOL bWait);
BOOL(WINAPI* pWTSRegisterSessionNotification) (HWND, DWORD);
BOOL(WINAPI* pWTSUnregisterSessionNotification) (HWND);

//BOOL (WINAPI *pSetShellWindow)(HWND);
//HWND (WINAPI *pGetShellWindow) ();

/* ToolHelp Function Pointers. (not in NT4) */
extern HANDLE(WINAPI* pCreateToolhelp32Snapshot) (DWORD, DWORD);
extern BOOL(WINAPI* pModule32First) (HANDLE, LPMODULEENTRY32);
extern BOOL(WINAPI* pModule32Next) (HANDLE, LPMODULEENTRY32);
/* psapi (NT based versions only) */
extern DWORD(WINAPI* pGetModuleBaseName) (HANDLE, HMODULE, LPTSTR, DWORD);
extern BOOL(WINAPI* pEnumProcessModules) (HANDLE, HMODULE*, DWORD, LPDWORD);

/* transparency (win2k+ only) */
extern BOOL(WINAPI* pSetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD);

/* multimonitor api (not in win95) */
extern HMONITOR(WINAPI* pMonitorFromPoint) (POINT, DWORD);
extern HMONITOR(WINAPI* pMonitorFromWindow) (HWND, DWORD);
extern BOOL(WINAPI* pGetMonitorInfoA) (HMONITOR, LPMONITORINFO);
extern BOOL(WINAPI* pEnumDisplayMonitors) (HDC, LPCRECT, MONITORENUMPROC, LPARAM);

/* NT based versions only: */
BOOL(WINAPI* pSetProcessShutdownParameters) (DWORD dwLevel, DWORD dwFlags);

/* not in win95 */
BOOL(WINAPI* pTrackMouseEvent) (LPTRACKMOUSEEVENT lpEventTrack);
DWORD(WINAPI* pGetLongPathName) (LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);

/* The IsWow64Message function determines if the last message read from
   the current thread's queue originated from a WOW64 process. (User32.dll)
   (Appearantly WOW64 means a 32 bit process running under win64) */

#ifdef _WIN64
BOOL(WINAPI* pIsWow64Message) ();
#endif

/* ----------------------------------------------------------------------- */
static const char shell_lib[] = "SHELL32";
static const char user_lib[] = "USER32";
static const char kernel_lib[] = "KERNEL32";
static const char psapi_lib[] = "PSAPI";
static const char shdocvw_lib[] = "SHDOCVW";
static const char wtsapi_lib[] = "WTSAPI32";

struct proc_info {
	const char* m_lib;
	const char* m_procname;
	void* m_procadr;
};

static const struct proc_info g_rtl_list[] =
{
	{ user_lib, "SwitchToThisWindow", &pSwitchToThisWindow },
	{ user_lib, "SetLayeredWindowAttributes", &pSetLayeredWindowAttributes },
	{ user_lib, "GetMonitorInfoA", &pGetMonitorInfoA },
	{ user_lib, "MonitorFromWindow", &pMonitorFromWindow },
	{ user_lib, "MonitorFromPoint", &pMonitorFromPoint },
	{ user_lib, "EnumDisplayMonitors", &pEnumDisplayMonitors },
	{ user_lib, "TrackMouseEvent", &pTrackMouseEvent },
	{ user_lib, "AllowSetForegroundWindow", &pAllowSetForegroundWindow },
	{ user_lib, "LockWorkStation", &pLockWorkStation },
#ifdef _WIN64
	{ user_lib, "IsWow64Message", &pIsWow64Message },
#endif

	{ wtsapi_lib, "WTSDisconnectSession", &pWTSDisconnectSession },
	{ wtsapi_lib, "WTSRegisterSessionNotification", &pWTSRegisterSessionNotification },
	{ wtsapi_lib, "WTSUnregisterSessionNotification", &pWTSUnregisterSessionNotification },

	//{ user_lib ,"SetShellWindow", &pSetShellWindow },
	//{ user_lib ,"GetShellWindow", &pGetShellWindow },

#ifndef BBTINY
	{ shell_lib, (char*)0x3C, &pMSWinShutdown },
	{ shell_lib, (char*)0x3D, &pRunDlg },
	{ shell_lib, (char*)0xB5, &pRegisterShellHook },
#endif

	{ kernel_lib, "CreateToolhelp32Snapshot", &pCreateToolhelp32Snapshot },
	{ kernel_lib, "Module32First", &pModule32First },
	{ kernel_lib, "Module32Next", &pModule32Next },
	{ kernel_lib, "SetProcessShutdownParameters", &pSetProcessShutdownParameters },
	{ kernel_lib, "GetLongPathNameA", &pGetLongPathName },

	{ psapi_lib, "EnumProcessModules", &pEnumProcessModules },
	{ psapi_lib, "GetModuleBaseNameA", &pGetModuleBaseName },


	{ NULL, NULL, NULL }
};

void init_runtime_libs()
{
	TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "Initializing runtime libraries...");
	const struct proc_info* rtl_ptr = g_rtl_list;
	do
	{
		load_imp(rtl_ptr->m_procadr, rtl_ptr->m_lib, rtl_ptr->m_procname);
	} while ((++rtl_ptr)->m_lib);
}

/* ----------------------------------------------------------------------- */
// RegisterShellHook flags
enum E_RegisterShellHook {
	RSH_UNREGISTER = 0
	, RSH_REGISTER = 1
	, RSH_PROGMAN = 2
	, RSH_TASKMAN = 3
};

void register_shellhook(HWND hwnd)
{
	TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "Registering shell hook...");
	if (have_imp(pRegisterShellHook))
	{
		//pRegisterShellHook(NULL, 1);
		pRegisterShellHook(hwnd, g_usingNT ? 3 : 1);
	}
	g_WM_ShellHook = RegisterWindowMessage("SHELLHOOK");
	if (have_imp(pWTSRegisterSessionNotification))
	{
		pWTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
	}
}

void unregister_shellhook(HWND hwnd)
{
	TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "Unregistering shell hook...");
	if (have_imp(pRegisterShellHook))
		pRegisterShellHook(hwnd, 0);
	if (have_imp(pWTSUnregisterSessionNotification))
		pWTSUnregisterSessionNotification(hwnd);
}

//BOOL (*WinList_Init) ();
//BOOL (*WinList_Terminate) ();

void DDE_init()
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
#ifndef BBTINY
	if (g_underExplorer)
		return;
	if (Settings_disableDDE)
		return;
	if (FindWindow("Progman", NULL))
		return;
	if (load_imp(&pShellDDEInit, shdocvw_lib, (const char*)0x76/*118*/))
		pShellDDEInit(TRUE);
#endif
}

void DDE_exit()
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	if (have_imp(pShellDDEInit))
	{
		pShellDDEInit(FALSE);
		pShellDDEInit = NULL;
	}
}

void set_os_info()
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	DWORD version = GetVersion();
	g_usingNT = 0 == (version & 0x80000000);
	if (g_usingNT)
	{
		DWORD hex_version = ((version << 8) & 0xFF00) + ((version >> 8) & 0x00FF);
		// dbg_printf("hex_version %x", hex_version);
		g_usingXP = hex_version >= 0x501;
		g_usingVista = hex_version >= 0x600;
		g_usingWin7 = hex_version >= 0x601;
		g_usingWin8 = hex_version >= 0x602;
		TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Windows detection: ver=%x (xp=%i vista=%i win7=%i win8=%i)", hex_version, g_usingXP, g_usingVista, g_usingWin7, g_usingWin8);
	}
	else
	{
		TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Windows detection: ver=%x, pre-winNT. Prehistoric Win9x, WinME,...", version);
	}
}

/* Terminate the XP/Vista welcome screen */
void terminate_welcomescreen()
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	HANDLE h;
	if (g_usingVista)
		h = OpenEvent(EVENT_MODIFY_STATE, FALSE, "ShellDesktopSwitchEvent");
	else
		h = OpenEvent(EVENT_MODIFY_STATE, FALSE, "msgina: ShellReadyEvent");
	if (h) SetEvent(h), CloseHandle(h);
}




/* parse commandline options - returns true to exit immediately */

bool check_options(const char* lpCmdLine)
{
	while (*lpCmdLine)
	{
		const char* const options[] = {
			//"help"		,
			"rc"		,
			"install"	,
			"uninstall" ,
			"nostartup" ,
			"toggle"	,
			"exec"		,
			NULL
		};

		char option[MAX_PATH];
		NextToken(option, &lpCmdLine, NULL);

		if (option[0] == '-')
		{
			switch (get_string_index(&option[1], options))
			{
				case 1: // -rc <path>
					bbrcPath(unquote(NextToken(option, &lpCmdLine, NULL)));
					continue;

				case 2: // -install
				case 3: // -uninstall
					BBExecute_string("bsetshell.exe", RUN_SHOWERRORS);
					return true;

				case 4: // -nostartup
					g_nostartup = true;
					continue;

				case 5: // -toggle
					if (BBhwnd)
					{
						PostMessage(BBhwnd, BB_QUIT, 0, BBOPT_QUIET);
						return true;
					}
					g_nostartup = true;
					continue;

				case 6: // -exec <broam>
					if (BBhwnd)
					{
						// stop hour glass cursor:
						MSG m;
						PeekMessage(&m, NULL, 0, 0, PM_NOREMOVE);
						BBSendData(BBhwnd, BB_EXECUTE, 0, lpCmdLine, -1);
					}
					return true;
			}
		}
		else if (strchr(option, ':'))
		{
			// automatically open a style file (address flag)
			if (!BBSendData(FindWindow(szBlackboxClass, szBlackboxName), BB_SETSTYLE, 0, option, 1 + strlen(option)))
			{
				BBMessageBox(MB_OK, "Unknown type of style file: %s\t", option);
				TRACE_MSG(trace::e_Error, trace::CTX_BBCore, "Unknown type of style file: %s\t", option);
			}
			return true;
		}
		BBMessageBox(MB_OK, "Unknown commandline option: %s\t", option);
		TRACE_MSG(trace::e_Error, trace::CTX_BBCore, "Unknown commandline option: %s\t", option);
		return true;
	}
	return false;
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	TRACE_APPNAME("BlackBox");
	TRACE_CONNECT();
	TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Initializing BlackBox, version %s", BBLEAN_VERSION);

	hMainInstance = hInstance;
	BBThreadId = GetCurrentThreadId();
	g_stack_top = (DWORD_PTR)&hInstance; //@FIXME: 64bit
	set_os_info();
	bb_rcreader_init();

	for (;;)
	{
		BBhwnd = FindWindow(szBlackboxClass, szBlackboxName);
		if (NULL == BBhwnd)
			BBhwnd = FindWindow("xoblite", NULL);

		if (check_options(lpCmdLine))
			return 0;

		/* Check if Blackbox is already running... */
		if (NULL == BBhwnd)
			break;

		int const ret = BBMessageBox(MB_YESNO, NLS2("$Error_StartedTwice$",
			"Blackbox already running!"
			"\nDo you want to close the other instance?"
		));
		if (IDYES != ret)
			return 0;
		SendMessage(BBhwnd, BB_QUIT, 0, BBOPT_QUIET);
		BBSleep(1000);
		g_nostartup = true;
	}

	/* Give the user a chance to escape from a broken installation */
	if (false == g_nostartup && (0x8000 & GetKeyState(VK_CONTROL)))
	{
		terminate_welcomescreen();
		int const ret = BBMessageBox(MB_YESNO, NLS2("$Query_Escape$",
			"Control Key was held down."
			"\nDo you want to start an explorer window instead of Blackbox?"
		));
		if (IDYES == ret)
		{
			BBExecute_string("explorer.exe", RUN_SHOWERRORS);
			return 0;
		}
	}

	init_runtime_libs();
	g_multimon = have_imp(pGetMonitorInfoA);
	/* Are we running on top of Explorer? */
	g_underExplorer = NULL != FindWindow("Shell_TrayWnd", NULL);
	//nostartup |= underExplorer || StartupHasBeenRun();

#ifndef BBTINY
	OleInitialize(0);
#endif

	/* Set "Hide minimized windows" system option... */
	MINIMIZEDMETRICS mm;
	memset(&mm, 0, sizeof(mm));
	mm.cbSize = sizeof(mm);
	SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(mm), &mm, 0);
	mm.iArrange |= ARW_HIDE; /* ARW_HIDE = 8 */
	/* the shell-hook notification will not work unless this is done. */
	SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(mm), &mm, 0);

	/* WinNT based systems only: */
	if (false == g_underExplorer && have_imp(pSetProcessShutdownParameters))
		pSetProcessShutdownParameters(2, 0);

	/* Give a slightly higher priority to the shell */
	//Disabled because this may lock up the computer if blackbox or
	//a plugin causes an infinite loop
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	/* This seems to inizialize some things and free some memory at startup. */
	SendMessage(GetDesktopWindow(), 0x400, 0, 0); /* 0x400 = WM_USER */

	/* save desktop margins and DRAGFULLWINDOWS option */
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&g_OldDT, 0);
	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &g_save_opaquemove, 0);

	/* get screenwidth/height */
	getWorkspaces().GetScreenMetrics();

	MessageManager_Init();

	/* create the main message window... */
	BBRegisterClass(szBlackboxClass, MainWndProc, 0);
	CreateWindowEx(
		WS_EX_TOOLWINDOW,
		szBlackboxClass,
		szBlackboxName,
		WS_POPUP | WS_DISABLED,
		// sizes are assigned for cursor behaviour with
		// AutoRaise Focus on winME, win2k
		getWorkspaces().GetVScreenX(), getWorkspaces().GetVScreenY(), getWorkspaces().GetVScreenWidth(), getWorkspaces().GetVScreenHeight(),
		NULL,
		NULL,
		hMainInstance,
		NULL
	);
	WindowQueryHelper::initWinHelper();
	Settings_ReadRCSettings();
	HideExplorer();
	DDE_init();
	startup_blackbox();

	MSG msg;
	msg.wParam = 0;
	TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Entering main message loop...");
	TRY{
		/* Main message loop */
		for (;;)
		{
			if (GetMessage(&msg, NULL, 0, 0) <= 0)
				break;
			//dbg_printf("hwnd %x, msg %d wp %x lp %x", msg.hwnd, msg.message, msg.wParam, msg.lParam);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} EXCEPT((
		getWorkspaces().GatherWindows(),
		EXCEPTION_CONTINUE_SEARCH))
		/* On crash: gather windows, then pass it to the OS */
	{
	}
	TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Main message loop terminated...");

	UnregisterClass(szBlackboxClass, hMainInstance);
	DDE_exit();
	ShowExplorer();
	MessageManager_Exit();
	free_nls();
	reset_pix();
#ifndef BBTINY
	OleUninitialize();
#endif
	TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Terminating log and exiting BlackBox...");
	TRACE_DISCONNECT();

	m_alloc_check_leaks("bbCore");
	return msg.wParam;
}

//=====================================================
void start_plugins()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	//beginToolbar(hMainInstance);
	PluginManager_Init();
}

//=====================================================
void kill_plugins()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	PluginManager_Exit();
	//endToolbar(hMainInstance);
	SetDesktopMargin(NULL, BB_DM_RESET, 0);
	reset_rcreader();
	reset_rcpaths();
}

//=====================================================
void startup_blackbox()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	g_shutting_down = false;
	register_fonts();
	Settings_ReadStyleSettings();
	set_misc_options();
	getWorkspaces().Init(g_nostartup);
	Desk_Init();
	Menu_Init();
	start_plugins();
	Tray_Init();
	BBSleep(0);
	terminate_welcomescreen();
	SetTimer(BBhwnd, BB_RUNSTARTUP_TIMER, 2000, NULL);
}

//=====================================================
void shutdown_blackbox()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	if (g_shutting_down)
		return;
	g_shutting_down = true;

	kill_plugins();
	Tray_Exit();
	Menu_Exit();
	Desk_Exit();
	getWorkspaces().GatherWindows();
	getWorkspaces().Exit();
	set_focus_model("");
	set_opaquemove(g_save_opaquemove);
	SystemParametersInfo(SPI_SETWORKAREA, 0, (PVOID)&g_OldDT, 0);
	unregister_fonts();
	DestroyWindow(BBhwnd);
}

//=====================================================
void reconfigure_blackbox()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	free_nls();
	reset_pix();

	Settings_ReadRCSettings();
	Settings_ReadStyleSettings();

	set_misc_options();
	getWorkspaces().Reconfigure();
	Menu_Reconfigure();
	Menu_All_Redraw(0);

	Desk_new_background(NULL);
}


void set_focus_model(const char* fm_string)
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	int fm;
	fm = 0 == _stricmp(fm_string, "SloppyFocus") ? 1
		: 0 == _stricmp(fm_string, "AutoRaise") ? 3
		: 0
		;
	SystemParametersInfo(SPI_SETACTIVEWNDTRKTIMEOUT, 0,
		(PVOID)(fm ? Settings_autoRaiseDelay : 0), 0);
	SystemParametersInfo(SPI_SETACTIVEWINDOWTRACKING, 0,
		(PVOID)!!(fm & 1), 0);
	SystemParametersInfo(SPI_SETACTIVEWNDTRKZORDER, 0,
		(PVOID)!!(fm & 2), 0);
}

void set_opaquemove(int opaque)
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	static int prev_om = 2;
	if (prev_om != (int)opaque)
	{
		prev_om = opaque;
		SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, opaque, NULL, 0);
	}
}

void set_misc_options()
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	set_focus_model(Settings_focusModel);
	set_opaquemove(Settings_opaqueMove);
	SetDesktopMargin(NULL, BB_DM_REFRESH, 0);
	const char* p = ReadString(extensionsrcPath(NULL), "blackbox.options.locale:", "");
	if (NULL == setlocale(LC_TIME, p))
		setlocale(LC_TIME, "");
}


// Load a new_style
void reset_extended_rootCommand()
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	// on style changes
	if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && 0 == (GetAsyncKeyState(VK_LWIN) & 0x8000))
	{
		// with key held down:
		// keep background as is, that is we set the custom
		// rootcommand option to what was in the previous style
		if (0 == _stricmp(Desk_extended_rootCommand(NULL), "style"))
			Desk_extended_rootCommand(mStyle.rootCommand);
	}
	else
	{
		//if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		// key not held down: reset custom rootcommand to use style now
		Desk_extended_rootCommand("style");
	}
}

void set_style(const char* filename)
{
	TRACE_SCOPE_MSG(trace::e_Info, trace::CTX_BBCore, "Setting style %s", filename);
	char path[MAX_PATH];
	if (false == FindRCFile(path, filename, NULL))
		MBoxErrorFile(path);
	else
	{
		WriteString(bbrcPath(NULL), "session.styleFile", get_relative_path(NULL, path));
		reset_extended_rootCommand();
		Desk_Reset(false);
	}
}


// handle WM_COPYDATAs from bbStyleMakers
int handle_received_data(HWND hwnd, UINT msg, WPARAM wParam, const void* data, unsigned data_size)
{
	if (BB_SETSTYLESTRUCT == msg)
	{
		// dbg_printf("BB_SETSTYLESTRUCT %d %d %d (%d)", msg, wParam, data_size, STYLESTRUCTSIZE);
		if (SN_STYLESTRUCT == wParam && data_size >= STYLESTRUCTSIZE)
		{
			// bbStyleMaker 1.3
			StyleStruct* const pss = &mStyle;
			bool const is_070 = pss->is_070;
			memcpy(pss, data, STYLESTRUCTSIZE);
			if (is_070 != pss->is_070)
				bimage_init(Settings_imageDither, pss->is_070);
		}
		return 1;
	}
	return 0;
}

void restart(const char* str, LPARAM lParam)
{
	// we dont want plugins being loaded twice.
	if (g_in_restart)
		return;

	g_in_restart = true;
	MessageManager_Send(BB_EXITTYPE, 0, B_RESTART);
	kill_plugins();
	BBSleep(100);

	if (lParam == BBOPT_PAUSE || (GetAsyncKeyState(VK_SHIFT) & 0x8000))
	{
		BBMessageBox(MB_OK, NLS2("$Restart_Paused$",
			"Restart paused, press OK to continue..."));
	}

	if (str)
	{
		WriteString(extensionsrcPath(NULL), "blackbox.theme:", str);
		m_free((char*)str);
	}

	register_fonts();
	Settings_menu.showBroams = false;
	Menu_All_Toggle(g_PluginsHidden = false);
	reconfigure_blackbox();
	MessageManager_Send(BB_RECONFIGURE, 0, 0);
	start_plugins();
	Menu_Update(MENU_UPD_ROOT);

	BBSleep(100);
	g_in_restart = false;
}



LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT r;
	const char* str;
	const WPARAM ID_HOTKEY = 3;

#ifdef LOG_BB_MESSAGES
	//dbg_printf("hwnd %04x msg %d wp %x lp %x", hwnd, uMsg, wParam, lParam);
	if (uMsg >= BB_MSGFIRST && uMsg < BB_MSGLAST)
		log_BBMessage(uMsg, wParam, lParam, g_stack_top - (DWORD_PTR)&hwnd);
#endif

	switch (uMsg)
	{
		// first the BB_... internal messages
		//===================================
		case BB_QUIT:
			// dont quit, if started as the main shell
			if (false == g_underExplorer
				&& lParam != BBOPT_QUIET
				&& IDOK != BBMessageBox(MB_OKCANCEL,
					NLS2("$Query_Quit$",
						"Are you sure you want to quit the shell?"
					)))
				break;

			if (g_in_restart)
				break;

			MessageManager_Send(BB_EXITTYPE, 0, B_QUIT);
			/* clean up */
			shutdown_blackbox();
			PostQuitMessage(0);
			break;

			//====================
		case BB_SHUTDOWN:
			Menu_All_Hide();
			ShutdownWindows(wParam, lParam == BBOPT_QUIET);
			break;

			//====================
		case BB_SETSTYLE:
			if (lParam)
				set_style((const char*)lParam);
			PostMessage(hwnd, BB_RECONFIGURE, 0, 0);
			break;

			//====================
		case BB_ABOUTPLUGINS:
			PluginManager_aboutPlugins();
			break;

		case BB_ABOUTSTYLE:
			about_style();
			break;

			//====================
		case BB_EDITFILE:
			edit_file(wParam, (const char*)lParam);
			break;

			//====================
		case BB_RUN:
			show_run_dlg();
			break;

			//====================
			// Execute a string (shellcommand or broam)
		case BB_EXECUTE:
			exec_command((const char*)lParam);
			break;

		case BB_POSTSTRING: // posted command-string, from menu click
			exec_command((const char*)lParam);
			m_free((char*)lParam);
			break;

		case BB_EXECUTEASYNC:
			post_command((const char*)lParam);
			break;

			//====================
		case BB_BROADCAST:
			if (false == exec_broam((const char*)lParam))
				return MessageManager_Send(uMsg, wParam, lParam);
			break;

			//====================
		case BB_TOGGLEPLUGINS:
			SendMessage(hwnd, BB_BROADCAST, 0, (LPARAM)
				((wParam ? (int)wParam > 0 : g_PluginsHidden)
					? "@BBShowPlugins" : "@BBHidePlugins"));
			return MessageManager_Send(uMsg, wParam, lParam);

			//====================
		case BB_DESKCLICK:
			// dbg_printf("BB_DESKCLICK %d %d", wParam, lParam);
			if (0 == lParam)
			{ // left down
				bool e = Menu_Exists(false);
				getWorkspaces().SwitchToBBWnd();
				PostMessage(hwnd, BB_HIDEMENU, 0, 0);
				if (e) break; // there are menus to hide, so we stop for now
				Menu_All_BringOnTop(); // raise menus
				return MessageManager_Send(uMsg, wParam, lParam); // click-raise plugins
			}
			Desk_mousebutton_event(lParam);
			break;

			//====================
			// Menu
		case BB_MENU:
			if (MenuMaker_ShowMenu(wParam, (const char*)lParam))
				return MessageManager_Send(uMsg, wParam, lParam);
			break;

		case BB_HIDEMENU:
			Menu_All_Hide();
			return MessageManager_Send(uMsg, wParam, lParam);

			//======================================================
		case BB_SETTHEME:
			str = (const char*)lParam;
			// do_restart
		case BB_RESTART:
			str = NULL;
			restart(str, lParam);
			break;

			//======================================================
		case BB_RECONFIGURE:
			if (0 == g_in_restart && PluginManager_RCChanged())
			{
				// when the plugin.rc file was edited by the user,
				// do a restart to update the plugin config
				str = NULL;
				restart(str, lParam);
				break;
			}

			reconfigure_blackbox();
			Menu_Update(MENU_UPD_ROOT);

			// if toolbar is disabled, update ToolbarInfo.
			// (if it is enabled, it will do so itself).
			/*if (false == Settings_toolbar.enabled)
				Toolbar_UpdatePosition();*/

			if (1 == wParam) // sent from exec_cfg_command
				break;

			return MessageManager_Send(uMsg, wParam, lParam);
			// dispatch_bb_message;

			//====================

		case BB_REDRAWGUI:
			if (wParam & BBRG_MENU)
			{
				Menu_Reconfigure();
				Menu_All_Redraw(wParam);
			}
			return MessageManager_Send(uMsg, wParam, lParam);
			// dispatch_bb_message;

			//======================================================
			// forward these to Workspace.cpp

		case BB_WINDOWLOWER:
		case BB_WINDOWRAISE:
		case BB_WINDOWSHADE:
		case BB_WINDOWGROWHEIGHT:
		case BB_WINDOWGROWWIDTH:
			if (0 == lParam && IsWindow((HWND)wParam))
				lParam = wParam;

		case BB_WORKSPACE:
		case BB_SWITCHTON:
		case BB_LISTDESKTOPS:
		case BB_SENDWINDOWTON:
		case BB_MOVEWINDOWTON:
		case BB_BRINGTOFRONT:
		case BB_WINDOWMINIMIZE:
		case BB_WINDOWMAXIMIZE:
		case BB_WINDOWRESTORE:
		case BB_WINDOWCLOSE:
		case BB_WINDOWMOVE:
		case BB_WINDOWSIZE:
			r = getWorkspaces().Command(uMsg, wParam, lParam);
			if (r != -1) 
				return r;
			return MessageManager_Send(uMsg, wParam, lParam);

			//====================
			// Updating of the workspaces/task menu is delayed in order
			// to get the correct window states with active or iconized.
		case BB_DESKTOPINFO:
		case BB_TASKSUPDATE:
			SetTimer(hwnd, BB_TASKUPDATE_TIMER, 200, NULL);
			// dispatch_bb_message;
			return MessageManager_Send(uMsg, wParam, lParam);
			//====================
		case BB_REGISTERMESSAGE:
			MessageManager_Register((HWND)wParam, (UINT*)lParam, true);
			break;

		case BB_UNREGISTERMESSAGE:
			MessageManager_Register((HWND)wParam, (UINT*)lParam, false);
			break;

			//==============================================================
			// COPYDATA stuff, for passing information from/to other processes
			// (i.e. bbStyleMaker, BBNote)

		case BB_GETSTYLE:
			return BBSendData((HWND)lParam, BB_SENDDATA, wParam, stylePath(NULL), -1);

		case BB_GETSTYLESTRUCT:
			return BBSendData((HWND)lParam, BB_SENDDATA, wParam, &mStyle, STYLESTRUCTSIZE);

			// done with BB_messages,
			//==============================================================

			//==============================================================
			// now for the WM_... messages

		case WM_CREATE:

			BBhwnd = hwnd;
			MakeSticky(hwnd);
			register_shellhook(hwnd);
			RegisterHotKey(hwnd, ID_HOTKEY, MOD_CONTROL | MOD_ALT, VK_F1);
			break;

			//====================
		case WM_DESTROY:
			unregister_shellhook(hwnd);
			RemoveSticky(hwnd);
			BBhwnd = NULL;
			break;

			//====================
		case WM_ENDSESSION:
			if (wParam)
				shutdown_blackbox();
			break;

		case WM_QUERYENDSESSION:
			getWorkspaces().GatherWindows();
			return TRUE;

		case WM_CLOSE:
			break;

		case WM_HOTKEY:
			if (ID_HOTKEY == wParam)
			{
				// bb_quit;
				MessageManager_Send(BB_EXITTYPE, 0, B_QUIT);
				/* clean up */
				shutdown_blackbox();
				PostQuitMessage(0);
			}
			break;

			//====================
		case WM_MOUSEACTIVATE:
			return MA_NOACTIVATE;

			//====================
		case WM_ACTIVATEAPP:
			if ((0 != wParam) != g_bbactive)
			{
				g_bbactive = 0 != wParam;
				if (false == g_bbactive)
					Menu_All_Hide();
			}
			break;

			//======================================================
		case WM_DISPLAYCHANGE:
			if (g_shutting_down)
				break;
			Desk_Reset(true);
			PostMessage(hwnd, BB_RECONFIGURE, 0, 0);
			break;

		case WM_WTSSESSION_CHANGE:
			switch (wParam)
			{
				case WTS_CONSOLE_CONNECT:
				case WTS_SESSION_LOGON:
				case WTS_SESSION_UNLOCK:
					// PlaySound("xxx.wav", NULL, SND_FILENAME|SND_ASYNC);
					break;
				case WTS_CONSOLE_DISCONNECT:
				case WTS_SESSION_LOGOFF:
				case WTS_SESSION_LOCK:
					// PlaySound("yyy.wav", NULL, SND_FILENAME|SND_ASYNC);
					break;
			}
			return 0;

			//====================
		case WM_TIMER:
			KillTimer(hwnd, wParam);
			switch (wParam)
			{
				case BB_RUNSTARTUP_TIMER:

#ifdef _DEBUG
					break; // skip startup code when in debug.
#endif
					if (false == g_nostartup)
						RunStartupStuff();
					break;

				case BB_ENDSTARTUP_TIMER:
					exec_command(ReadString(extensionsrcPath(NULL), "blackbox.startup.run", NULL));
					break;

				case BB_TASKUPDATE_TIMER:
					Menu_Update(MENU_UPD_TASKS);
					break;
			}
			break;

			//====================
		case WM_COPYDATA:
			return BBReceiveData(hwnd, lParam, handle_received_data);

		default:
			if (uMsg == g_WM_ShellHook || uMsg == (WM_USER + 44))
			{
				if (g_shutting_down)
					break;

				uMsg = 0;
				switch (wParam & 0x7fff)
				{
					case HSHELL_WINDOWCREATED:			uMsg = BB_ADDTASK; break;
					case HSHELL_WINDOWDESTROYED:		uMsg = BB_REMOVETASK; break;
					case HSHELL_ACTIVATESHELLWINDOW:	uMsg = BB_ACTIVATESHELLWINDOW; break;
					case HSHELL_WINDOWACTIVATED:		uMsg = BB_ACTIVETASK; break;
					case HSHELL_GETMINRECT:				uMsg = BB_MINMAXTASK; break;
					case HSHELL_REDRAW:					uMsg = BB_REDRAWTASK; break;
					case HSHELL_TASKMAN:
						MessageManager_Send(BB_WINKEY, 0, 0);
						break;
				}

				getWorkspaces().TaskProc(wParam, (HWND)lParam);
				if (uMsg)
					MessageManager_Send(uMsg, lParam, wParam);
				break;

				//====================
				//dispatch_bb_message:
				//return MessageManager_Send(uMsg, wParam, lParam);
			}

			if (uMsg >= BB_MSGFIRST && uMsg < BB_MSGLAST)
				return MessageManager_Send(uMsg, wParam, lParam);

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}


void about_style()
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	const char* cp = stylePath(NULL);
	BBMessageBox(MB_OK,
		"#", BBAPPNAME, " - %s#"
		"%s"
		"\n%s %s"
		"\n%s"
		"\n%s"
		"\n%s",
		NLS2("$About_Style_Title$", "Style Information"),
		ReadString(cp, "style.name", NLS2("$About_Style_NoName$", "[Style name not specified]")), NLS2("$About_Style_By$", "by"),
		ReadString(cp, "style.author", NLS2("$About_Style_NoAuthor$", "[Author not specified]")),
		ReadString(cp, "style.date", ""),
		ReadString(cp, "style.credits", ""),
		ReadString(cp, "style.comments", "")
	);
}

void edit(char* editor, char* buffer, const char* path)
{
	GetBlackboxEditor(editor);
	sprintf(buffer, "\"%s\" \"%s\"", editor, path);
	BBExecute_string(buffer, RUN_SHOWERRORS);
}

//=====================================================
void edit_file(int id, const char* path)
{
	TRACE_SCOPE(trace::e_Debug, trace::CTX_BBCore);
	char fullpath[MAX_PATH];
	char editor[MAX_PATH];
	char buffer[2 * MAX_PATH];
	switch (id)
	{
		case 0:
			path = stylePath(NULL);
			edit(editor, buffer, path);
			break;
		case 1:
			path = menuPath(NULL);
			edit(editor, buffer, path);
			break;
		case 2:
			path = plugrcPath(NULL);
			edit(editor, buffer, path);
			break;
		case 3:
			path = extensionsrcPath(NULL);
			edit(editor, buffer, path);
			break;
		case 4:
			path = bbrcPath(NULL);
			edit(editor, buffer, path);
			break;
		case -1:
			FindRCFile(fullpath, path, NULL);
			path = fullpath;
			edit(editor, buffer, path);
			break;
	}
}


void ShowAppnames()
{
	HWND hwnd_task = NULL;
	int l = 4096;
	int x = 0;
	char* msg = static_cast<char*>(c_alloc(l));
	for (int i = 0; NULL != (hwnd_task = getWorkspaces().GetTask(i)); i++)
	{
		char appname[MAX_PATH];
		char caption[60];

		GetAppByWindow(hwnd_task, appname);
		GetWindowText(hwnd_task, caption, sizeof(caption));
		if (l - x < MAX_PATH + (int)sizeof(caption) + 100)
		{
			l *= 2;
			msg = static_cast<char*>(m_realloc(msg, l));
		}

		if (x)
			msg[x++] = '\n';
		x += sprintf(msg + x, "%-16s\t : \"%s\"", appname, caption);
	}
	BBMessageBox(0, NLS2("$Show_Apps$",
		"Current Applications:"
		"\n(Use the name from the first column for StickyWindows.ini)"
		"\n%s"
	), msg);
	m_free(msg);
}

/* execute a command, wait until execution finished (unless it's a shell command) */
void exec_command(const char* cmd)
{
	TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Executing command: %s", cmd);
	if (NULL == cmd || 0 == cmd[0])
		return;
	if ('@' == cmd[0])
		SendMessage(BBhwnd, BB_BROADCAST, 0, (LPARAM)cmd);
	else
		BBExecute_string(cmd, RUN_SHOWERRORS);
}

/* post a formatted command, dont wait for execution but return immediately */
void post_command_fmt(const char* fmt, ...)
{
	va_list arg_list;
	va_start(arg_list, fmt);
	//TRACE_MSG_VA(trace::e_Info, trace::CTX_BBCore, "Post command: %s", );
	PostMessage(BBhwnd, BB_POSTSTRING, 0, (LPARAM)m_formatv(fmt, arg_list));
}

/* post a command, dont wait for execution but return immediately */
void post_command(const char* cmd)
{
	post_command_fmt("%s", cmd);
}

/* check and parse "Workspace1" etc. strings */
int get_workspace_number(const char* s)
{
	if (0 == _memicmp(s, "workspace", 9))
	{
		int n = atoi(s + 9);
		if (n >= 1 && n <= 9)
			return n - 1;
	}
	return -1;
}


enum shutdown_modes {
	BBSD_SHUTDOWN = 0,
	BBSD_REBOOT = 1,
	BBSD_LOGOFF = 2,
	BBSD_HIBERNATE = 3,
	BBSD_SUSPEND = 4,
	BBSD_LOCKWS = 5,
	BBSD_SWITCHUSER = 6,
	BBSD_EXITWIN = 7,
};

enum corebroam_cases {
	e_false = 0,
	e_true = 1,

	e_checkworkspace,
	e_rootCommand,
	e_Message,
	e_ShowAppnames,
	e_About,
	e_Nop,
	e_Pause,
	e_Crash,
	e_ShowRecoverMenu,
	e_RecoverWindow,
	e_Test,

	e_quiet,
	e_pause,
	e_bool,
};

enum corebroam_flags {
	e_mask = 0x01F,
	e_post = 0x020,
	e_lpstr = 0x040,
	e_lpnum = 0x080,
	e_lptask = 0x100,
	e_wpnum = 0x200,
	e_lpint = 0x400,
};

struct corebroam_table {
	const char* m_str;
	unsigned short m_msg;
	unsigned short m_flag;
	short m_wParam;
};
static const corebroam_table g_corebroam_table[] = {
	// one specific window
	{ "Raise",					BB_WINDOWRAISE,		e_lptask, 0 },
	{ "Lower",					BB_WINDOWLOWER,		e_lptask, 0 },
	{ "Close",					BB_WINDOWCLOSE,		e_lptask, 0 },
	{ "Minimize",				BB_WINDOWMINIMIZE,	e_lptask, 0 },
	{ "Maximize",				BB_WINDOWMAXIMIZE,	e_lptask, 0 },
	{ "Restore",				BB_WINDOWRESTORE,	e_lptask, 0 },
	{ "Resize",					BB_WINDOWSIZE,		e_lptask, 0 },
	{ "Move",					BB_WINDOWMOVE,		e_lptask, 0 },
	{ "Shade",					BB_WINDOWSHADE,		e_lptask, 0 },
	// duplicates of the above -->
	{ "RaiseWindow",			BB_WINDOWRAISE,		e_lptask, 0 },
	{ "LowerWindow",			BB_WINDOWLOWER,		e_lptask, 0 },
	{ "CloseWindow",			BB_WINDOWCLOSE,		e_lptask, 0 },
	{ "MinimizeWindow",			BB_WINDOWMINIMIZE,	e_lptask, 0 },
	{ "MaximizeWindow",			BB_WINDOWMAXIMIZE,	e_lptask, 0 },
	{ "RestoreWindow",			BB_WINDOWRESTORE,	e_lptask, 0 },
	{ "ResizeWindow",			BB_WINDOWSIZE,		e_lptask, 0 },
	{ "MoveWindow",				BB_WINDOWMOVE,		e_lptask, 0 },
	{ "ShadeWindow",			BB_WINDOWSHADE,		e_lptask, 0 },
	// <--

	{ "MaximizeVertical",		BB_WINDOWGROWHEIGHT,e_lptask, 0 },
	{ "MaximizeHorizontal",		BB_WINDOWGROWWIDTH, e_lptask, 0 },
	{ "ActivateWindow",			BB_BRINGTOFRONT,	e_lptask, 0},

	{ "StickWindow",			BB_WORKSPACE,		e_lptask, BBWS_TOGGLESTICKY },
	{ "OnTopWindow",			BB_WORKSPACE,		e_lptask, BBWS_TOGGLEONTOP },
	{ "OnBackgroundWindow",		BB_WORKSPACE,		e_lptask, BBWS_TOGGLEONBG },
	{ "MoveWindowLeft",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWLEFT },
	{ "MoveWindowRight",		BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWRIGHT },
	{ "MoveWindowUp",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWUP },
	{ "MoveWindowDown",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWDOWN },
	{ "MoveWindowToWS",			BB_MOVEWINDOWTON,	e_lptask | e_wpnum, 0},
	{ "SendWindowToWS",			BB_SENDWINDOWTON,	e_lptask | e_wpnum, 0},

	// cycle windows
	{ "PrevWindow",				BB_WORKSPACE,		0, BBWS_PREVWINDOW },
	{ "NextWindow",				BB_WORKSPACE,		0, BBWS_NEXTWINDOW },
	{ "PrevWindowAllWorkspaces",BB_WORKSPACE,		e_true, BBWS_PREVWINDOW },
	{ "NextWindowAllWorkspaces",BB_WORKSPACE,		e_true, BBWS_NEXTWINDOW },

	// all windows
	{ "MinimizeAll",			BB_WORKSPACE,		0, BBWS_MINIMIZEALL },
	{ "RestoreAll",				BB_WORKSPACE,		0, BBWS_RESTOREALL },
	{ "Cascade",				BB_WORKSPACE,		0, BBWS_CASCADE },
	{ "TileVertical",			BB_WORKSPACE,		0, BBWS_TILEVERTICAL },
	{ "TileHorizontal",			BB_WORKSPACE,		0, BBWS_TILEHORIZONTAL },

	// workspaces
	{ "LeftWorkspace",			BB_WORKSPACE,		0, BBWS_DESKLEFT },
	{ "PrevWorkspace",			BB_WORKSPACE,		0, BBWS_DESKLEFT },
	{ "UpWorkspace",			BB_WORKSPACE,		0, BBWS_DESKUP },
	{ "DownWorkspace",			BB_WORKSPACE,		0, BBWS_DESKDOWN },
	{ "RightWorkspace",			BB_WORKSPACE,		0, BBWS_DESKRIGHT },
	{ "NextWorkspace",			BB_WORKSPACE,		0, BBWS_DESKRIGHT },
	{ "LastWorkspace",			BB_WORKSPACE,		0, BBWS_LASTDESK },
	{ "SwitchToWorkspace",		BB_WORKSPACE,		e_lpnum,  BBWS_SWITCHTODESK },

	{ "Gather",					BB_WORKSPACE,		0, BBWS_GATHERWINDOWS },
	{ "GatherWindows",			BB_WORKSPACE,		0, BBWS_GATHERWINDOWS },
	{ "AddWorkspace",			BB_WORKSPACE,		0, BBWS_ADDDESKTOP },
	{ "DelWorkspace",			BB_WORKSPACE,		0, BBWS_DELDESKTOP },
	{ "EditWorkspaceNames",		BB_WORKSPACE,		e_post, BBWS_EDITNAME },
	{ "SetWorkspaceNames",		BB_WORKSPACE,		e_lpstr, BBWS_EDITNAME },

	// menu
	{ "ShowMenu",				BB_MENU,			e_lpstr,  BB_MENU_BROAM },
	{ "ShowWorkspaceMenu",		BB_MENU,			0, BB_MENU_TASKS },
	{ "ShowIconMenu",			BB_MENU,			0, BB_MENU_ICONS },
	{ "HideMenu",				BB_HIDEMENU,		0, 0 },

	// blackbox
	{ "TogglePlugins",			BB_TOGGLEPLUGINS,	e_bool, 0 },
	{ "ToggleTray",				BB_TOGGLETRAY,		0, 0 },
	{ "AboutStyle",				BB_ABOUTSTYLE,		e_post, 0 },
	{ "AboutPlugins",			BB_ABOUTPLUGINS,	e_post, 0 },
	{ "Reconfig",				BB_RECONFIGURE,		e_post, 0 },
	{ "Reindex",                BB_REINDEX,         e_post, 0 },
	{ "AbortIndex",             BB_ABORTINDEX,      e_post, 0 },
	{ "Reconfigure",			BB_RECONFIGURE,		e_post, 0 },
	{ "Restart",				BB_RESTART,			e_post | e_pause, 0 },
	{ "Exit",					BB_QUIT,			e_post | e_quiet, 0 },
	{ "Quit",					BB_QUIT,			e_post | e_quiet, 0 },
	{ "Run",					BB_RUN,				e_post, 0 },
	{ "Theme",					BB_SETTHEME,		e_post | e_lpstr, 0 },

	// edit
	{ "Edit",					BB_EDITFILE,		e_lpstr, -1},
	{ "EditStyle",				BB_EDITFILE,		0, 0 },
	{ "EditMenu",				BB_EDITFILE,		0, 1 },
	{ "EditPlugins",			BB_EDITFILE,		0, 2 },
	{ "EditExtensions",			BB_EDITFILE,		0, 3 },
	{ "EditBlackbox",			BB_EDITFILE,		0, 4 },

	// shutdown
	{ "Shutdown",				BB_SHUTDOWN,		e_post | e_quiet, BBSD_SHUTDOWN	},
	{ "Reboot",					BB_SHUTDOWN,		e_post | e_quiet, BBSD_REBOOT		},
	{ "Logoff",					BB_SHUTDOWN,		e_post | e_quiet, BBSD_LOGOFF		},
	{ "Hibernate",				BB_SHUTDOWN,		e_post | e_quiet, BBSD_HIBERNATE	},
	{ "Suspend",				BB_SHUTDOWN,		e_post | e_quiet, BBSD_SUSPEND	},
	{ "LockWorkstation",		BB_SHUTDOWN,		e_post | e_quiet, BBSD_LOCKWS		},
	{ "SwitchUser",				BB_SHUTDOWN,		e_post | e_quiet, BBSD_SWITCHUSER },
	{ "ExitWindows",			BB_SHUTDOWN,		e_post | e_quiet, BBSD_EXITWIN	},

	// miscellaneous
	{ "Style",					BB_SETSTYLE,		e_lpstr, 0 },
	{ "Exec",					BB_EXECUTE,			e_lpstr, 0 },
	{ "Post",					BB_EXECUTEASYNC,	e_lpstr, 0 },
	{ "Label",					BB_SETTOOLBARLABEL, e_lpstr, 0 },

	{ "rootCommand",			0, e_rootCommand	, 0 },
	{ "Message",				0, e_Message		, 0 },
	{ "ShowAppnames",			0, e_ShowAppnames	, 0 },
	{ "ShowRecoverMenu",		0, e_ShowRecoverMenu , 0 },
	{ "RecoverWindow",			0, e_RecoverWindow	, 0 },
	{ "Pause",					0, e_Pause			, 0 },
	{ "Nop",					0, e_Nop			, 0 },
	{ "Crash",					0, e_Crash			, 0 },
	{ "Test",					0, e_Test			, 0 },

	{ NULL /*"Workspace#"*/,	BB_WORKSPACE, e_checkworkspace,  BBWS_SWITCHTODESK },
};


int exec_core_broam(const char* broam)
{
	TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);

	char buffer[MAX_PATH], num[MAX_PATH];
	char const* core_args = broam + sizeof "@BBCore." - 1;
	char* core_cmd = NextToken(buffer, &core_args, NULL);

	corebroam_table const* action = g_corebroam_table;
	do
	{
		if (0 == _stricmp(action->m_str, core_cmd))
			break;
	} while ((++action)->m_str);

	WPARAM wParam = action->m_wParam;
	LPARAM lParam = 0;
	UINT msg = action->m_msg;

	if (action->m_flag & e_wpnum)
		wParam |= atoi(NextToken(num, &core_args, NULL)) - 1;

	if (action->m_flag & e_lpstr)
		lParam = (LPARAM)(action->m_flag & e_post ? new_str(core_args) : core_args);
	else
		if (action->m_flag & e_lpnum)
			lParam = atoi(core_args) - 1;
		else
			if (action->m_flag & e_lpint)
				lParam = atoi(core_args);
			else
				if (action->m_flag & e_lptask)
					lParam = (LPARAM)getWorkspaces().GetTask(atoi(core_args) - 1);

	switch (action->m_flag & e_mask)
	{
		case e_checkworkspace:
			{
				int const n = get_workspace_number(core_cmd);
				if (-1 == n)
					return 0;
				lParam = n;
				break;
			}
		case e_quiet:
			// check for 'no confirmation' option
			if (0 == _memicmp(core_args, "-q"/*uiet*/, 2))
				lParam = BBOPT_QUIET;
			break;

		case e_pause:
			// check for 'pause restart' option
			if (0 == _memicmp(core_args, "-p"/*ause*/, 2))
				lParam = BBOPT_PAUSE;
			break;

		case e_true: // ...AllWorkspaces option
			lParam = 1;
			break;

		case e_bool:
			wParam = 1 + get_false_true(core_args);
			break;

			// --- special (no message) commands ---
		case e_rootCommand:
			Desk_new_background(core_args);
			PostMessage(BBhwnd, BB_MENU, BB_MENU_UPDATE, 0);
			break;

		case e_Message:
			BBMessageBox(MB_OK, "%s", core_args);
			break;

		case e_ShowAppnames:
			ShowAppnames();
			break;

			//case e_About:
			//	bb_about();
			//	break;

		case e_Nop:
			break;

		case e_Pause:
			BBSleep(atoi(core_args));
			break;

		case e_Crash:
			*(DWORD*)0 = 0x11111111;
			break;

		case e_ShowRecoverMenu:
			ShowRecoverMenu();
			break;

		case e_RecoverWindow:
			{
				HWND hwnd;
				if (sscanf(core_args, "%p", &hwnd))
					getWorkspaces().ToggleWindowVisibility(hwnd);
				break;
			}
		case e_Test:
			break;
	}

	if (msg)
	{
		// Some things need to be 'Post'ed, i.e. to return from plugins
		// before they are unloaded with restart, quit etc.
		if (action->m_flag & e_post)
			PostMessage(BBhwnd, msg, wParam, lParam);
		else
			SendMessage(BBhwnd, msg, wParam, lParam);
	}
	return 1;
}

/* ----------------------------------------------------------------------- */
/* This is for the menu checkmarks in the styles and backgrounds folders */
bool get_opt_command(char* opt_cmd, const char* cmd)
{
	TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "cmd = %s", cmd);
	if (0 == opt_cmd[0])
	{
		if (0 == _memicmp(cmd, "@BBCore.", 8))
		{
			// internals, currently for style and rootcommand
#define CHECK_BROAM(broam) 0 == _memicmp(cmd, s = broam, sizeof broam - 3)
			const char* s;
			if (CHECK_BROAM(MM_STYLE_BROAM))
				sprintf(opt_cmd, s, stylePath(NULL));
			else if (CHECK_BROAM(MM_THEME_BROAM))
				sprintf(opt_cmd, s, defaultrc_path[0] ? defaultrc_path : "default");
			else if (CHECK_BROAM(MM_ROOT_BROAM))
				sprintf(opt_cmd, s, Desk_extended_rootCommand(NULL));
			else
				return false;
#undef CHECK_BROAM
		}
		else if (0 == MessageManager_Send(BB_GETBOOL, (WPARAM)opt_cmd, (LPARAM)cmd))
			return false; // nothing found
		else if (opt_cmd[0] == 1)
		{
			opt_cmd[0] = 0; // recheck next time;
			return true;
		}
	}
	return opt_cmd[0] && 0 == _stricmp(opt_cmd, cmd);
}


bool exec_script(const char* broam)
{
	TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);
	const char* p = 0, * a = 0;
	char* s = 0;
	int n = 0, c = 0;
	if ('[' != skip_spc(&broam))
		return false;
	for (n = strlen(++broam); n && (c = broam[--n], IS_SPC(c));)
		;
	if (0 == n || ']' != c)
		return false;
	s = new_str_n(broam, n);
	for (p = s; 0 != (n = nexttoken(&a, &p, "|"));)
	{
		s[a - s + n] = 0;
		exec_command(a);
	}
	m_free(s);
	return true;
}


// returns 'true' for done, 'false' for pass on to plugins
bool exec_broam(const char* broam)
{
	TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);
	if (0 == _memicmp(broam, "@BBCore.", 8))
	{
		if (0 == exec_core_broam(broam))
		{
			BBMessageBox(MB_OK, NLS2("$Error_UnknownBroam$", "Error: Unknown Broadcast Message:\n%s"), broam);
			TRACE_MSG(trace::e_Error, trace::CTX_BBCore, "Error: Unknown Broadcast Message: %s", broam);
			return false;
		}
	}
	else if (0 == _memicmp(broam, "@BBCfg.", 7))
	{
		if (0 == exec_cfg_command(broam + 7))
		{
			BBMessageBox(MB_OK, NLS2("$Error_UnknownBroam$", "Error: Unknown Broadcast Message:\n%s"), broam);
			TRACE_MSG(trace::e_Error, trace::CTX_BBCore, "Error: Unknown Broadcast Message: %s", broam);
			return false;
		}
	}
	else if (0 == _memicmp(broam, "@Script", 7))
	{
		exec_script(broam + 7);
		return true;
	}
	else if (0 == _stricmp(broam, "@BBHidePlugins"))
	{
		Menu_All_Toggle(g_PluginsHidden = true);
	}
	else if (0 == _stricmp(broam, "@BBShowPlugins"))
	{
		Menu_All_Toggle(g_PluginsHidden = false);
	}
	return false;
}


// ShutdownWindows stuff....

static const char* const shutdn_cmds_display[] =
{
	NLS0("shut down"),
	NLS0("reboot"),
	NLS0("log off"),
	NLS0("hibernate"),
	NLS0("suspend"),
	NLS0("lock workstation"),
	NLS0("exit windows")
	NLS0("switch user")
};

DWORD WINAPI ShutdownThread(void* mode)
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	switch ((DWORD_PTR)mode)
	{
		case BBSD_SHUTDOWN:
			if (ExitWindowsEx(EWX_SHUTDOWN | EWX_POWEROFF, 0))
				return 0;
			break;

		case BBSD_REBOOT:
			if (ExitWindowsEx(EWX_REBOOT, 0))
				return 0;
			break;

		case BBSD_LOGOFF:
			if (ExitWindowsEx(EWX_LOGOFF, 0))
			{
				if (false == g_usingNT)
					PostMessage(BBhwnd, WM_QUIT, 0, 0);
				return 0;
			}
			break;
	}
	return 0;
}

int ShutdownWindows(int mode, int no_msg)
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	switch (mode)
	{
		case BBSD_SHUTDOWN:
		case BBSD_REBOOT:
		case BBSD_LOGOFF:
		case BBSD_HIBERNATE:
		case BBSD_SUSPEND:
			if (0 == no_msg && IDYES != BBMessageBox(MB_YESNO,
				NLS2("$Query_Shutdown$", "Are you sure you want to %s?"),
				NLS1(shutdn_cmds_display[mode])
			)) return 0;
			break;

		case BBSD_LOCKWS:
			if (have_imp(pLockWorkStation))
				pLockWorkStation();
			return 1;

		case BBSD_SWITCHUSER:
			if (have_imp(pWTSDisconnectSession))
				pWTSDisconnectSession(
					WTS_CURRENT_SERVER_HANDLE,
					WTS_CURRENT_SESSION,
					FALSE
				);
			return 1;

		case BBSD_EXITWIN: // Standard Windows shutdown menu
			if (have_imp(pMSWinShutdown))
			{
				if (g_usingVista)
				{
					//@ W U NO WORK?! FIXME
					//return static_cast<BOOL(WINAPI *) (HWND, int)>(pMSWinShutdown)(BBhwnd, 0);
					return ((BOOL(WINAPI*) (HWND, int))pMSWinShutdown)(BBhwnd, 0);
				}
				else
				{
					return static_cast<BOOL(WINAPI*) (HWND)>(pMSWinShutdown)(BBhwnd);
				}
			}
			return 0;
	}

	switch (mode)
	{
		case BBSD_LOGOFF:
			break;

		default:
			if (g_usingNT)
			{
				// bool success = false;
				// Under WinNT/2k/XP we need to adjust privileges to be able to
				// shutdown/reboot/hibernate/suspend...
				HANDLE hToken;
				if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
				{
					TOKEN_PRIVILEGES tkp;
					// Get the LUID for the shutdown privilege...
					LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
					tkp.PrivilegeCount = 1;  // one privilege to set
					tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
					// Get the shutdown privileges for this process...
					AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
					// success = GetLastError() == ERROR_SUCCESS;
					CloseHandle(hToken);
				}
			}
			break;
	}

	switch (mode)
	{
		case BBSD_SUSPEND:
			if (SetSystemPowerState(TRUE, FALSE))
				return 1;
			return 0;

		case BBSD_HIBERNATE:
			if (SetSystemPowerState(FALSE, FALSE))
				return 1;
			return 0;
	}

	getWorkspaces().GatherWindows();
	DWORD tid;
	CloseHandle(CreateThread(NULL, 0, ShutdownThread, (LPVOID)mode, 0, &tid));
	return 1;
}


//
// RunStartupStuff stuff....
//

#ifndef BBTINY

#define RE_ONCE 1
#define RE_WAIT 2
#define RE_CHCK 4

static int exec_startup(const char* line, int flags)
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	int r = BBExecute_string(line, flags | RUN_WINDIR | RUN_NOERRORS/*|RUN_NOSUBST*/);
	log_printf((LOG_STARTUP, "\t\tRun (%s): %s", r ? "ok" : "failed", line));
	return r;
}

bool RunEntriesIn(HKEY root_key, const char* subpath, UINT flags)
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	char path[MAX_PATH];
	sprintf(path, "Software\\Microsoft\\Windows\\CurrentVersion\\%s", subpath);
	log_printf((LOG_STARTUP, "\tfrom registry: %s\\%s", HKEY_LOCAL_MACHINE == root_key ? "HKLM" : "HKCU", path));

	int const f = (flags & RE_ONCE) ? KEY_READ | KEY_WRITE : KEY_READ;
	HKEY hk1;
	if (ERROR_SUCCESS != RegOpenKeyEx(root_key, path, 0, f, &hk1))
		return false;

	for (int index = 0; ;)
	{
		char szValueName[MAX_PATH];
		char szData[MAX_PATH];

		DWORD cbValueName = sizeof(szValueName);
		DWORD cbData = sizeof(szData);
		DWORD dwDataType;
		if (ERROR_SUCCESS != RegEnumValue(hk1, index, szValueName, &cbValueName,
			0, &dwDataType, (LPBYTE)szData, &cbData))
			break;

		if (0 == (flags & RE_CHCK))
			exec_startup(szData, (flags & RE_WAIT) ? RUN_WAIT : 0);

		if (flags & RE_ONCE)
			if (ERROR_SUCCESS == RegDeleteValue(hk1, szValueName))
				continue;
		++index;
	}
	RegCloseKey(hk1);
	return true;
}

void RunFolderContents(const char* szParams)
{
	char szPath[MAX_PATH];
	int x = strlen(strcpy(szPath, szParams));
	strcpy(szPath + x++, "\\*");

	WIN32_FIND_DATA findData;
	HANDLE hSearch = FindFirstFile(szPath, &findData);
	if (hSearch == INVALID_HANDLE_VALUE)
		return;

	log_printf((LOG_STARTUP, "\tfrom folder: %s", szParams));
	do
	{
		if (findData.dwFileAttributes & (
			FILE_ATTRIBUTE_SYSTEM
			| FILE_ATTRIBUTE_DIRECTORY
			| FILE_ATTRIBUTE_HIDDEN))
			continue;

		strcpy(szPath + x, findData.cFileName);
		exec_startup(szPath, RUN_NOARGS);

	} while (FindNextFile(hSearch, &findData));

	FindClose(hSearch);
}

DWORD WINAPI RunStartupThread(void* pv)
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	short const startuptable[] = {
		0x0018,		//CSIDL_COMMON_STARTUP,
		// 0x001e,	//CSIDL_COMMON_ALTSTARTUP,
		0x0007,		//CSIDL_STARTUP,
		// 0x001d	//CSIDL_ALTSTARTUP
	};

	log_printf((LOG_STARTUP, "Startup: running startup items:"));
	if (RunEntriesIn(HKEY_LOCAL_MACHINE, "RunOnceEx", RE_CHCK))
	{
		log_printf((LOG_STARTUP, "\t\tStarting 'RunOnceExProcess'"));
		exec_startup("RunDLL32.EXE iernonce.dll,RunOnceExProcess", RUN_WAIT);
	}
	RunEntriesIn(HKEY_LOCAL_MACHINE, "RunOnce", RE_ONCE | RE_WAIT);
	RunEntriesIn(HKEY_LOCAL_MACHINE, "Run", 0);
	RunEntriesIn(HKEY_CURRENT_USER, "Run", 0);
	for (int i = 0; i < array_count(startuptable); ++i)
	{
		char folder[1024];
		folder[0] = 0;
		if (sh_getfolderpath(folder, sizeof(folder) / sizeof(*folder), startuptable[i]) && folder[0])
			RunFolderContents(folder);
	}
	RunEntriesIn(HKEY_CURRENT_USER, "RunOnce", RE_ONCE);
	log_printf((LOG_STARTUP, "Startup: finished"));
	SetTimer(BBhwnd, BB_ENDSTARTUP_TIMER, 100, NULL);
	return 0;
}


void RunStartupStuff()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	DWORD threadid;
	CloseHandle(CreateThread(NULL, 0, RunStartupThread, NULL, 0, &threadid));
}

//========================================================================

bool StartupHasBeenRun()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	HANDLE hToken;
	TOKEN_STATISTICS tStats;
	DWORD dwOutSize;
	char regpath[200];
	HKEY hkStartup;
	DWORD dwDisposition;
	LONG lResult;

	bool bReturn = false;

	if (false == g_usingNT)
		return bReturn;

	if (FALSE == OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return bReturn;

	if (FALSE == GetTokenInformation(hToken, TokenStatistics, &tStats, sizeof(tStats), &dwOutSize))
	{
		CloseHandle(hToken);
		return bReturn;
	}
	sprintf(regpath,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"
		"\\SessionInfo\\%08lx%08lx\\StartupHasBeenRun",
		tStats.AuthenticationId.HighPart,
		tStats.AuthenticationId.LowPart
	);

	lResult = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		regpath,
		0,
		NULL,
		REG_OPTION_VOLATILE,
		KEY_WRITE,
		NULL,
		&hkStartup,
		&dwDisposition
	);

	if (lResult != ERROR_SUCCESS)
	{
		CloseHandle(hToken);
		return bReturn;
	}

	if (dwDisposition != REG_CREATED_NEW_KEY)
		bReturn = true;

	RegCloseKey(hkStartup);
	CloseHandle(hToken);
	return bReturn;
}



void show_run_dlg()
{
	TRACE_SCOPE(trace::e_Info, trace::CTX_BBCore);
	POINT pt;
	RECT m;
	HWND hwnd;

	if (!have_imp(pRunDlg))
		return;

	GetCursorPos(&pt);
	GetMonitorRect(&pt, &m, GETMON_FROM_POINT | GETMON_WORKAREA);
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &m, 0);
	hwnd = CreateWindow(
		"STATIC",
		NULL,
		WS_POPUP,
		(m.left + m.right - 360) / 2, (m.top + m.bottom - 200) / 2,
		0, 0,
		NULL,
		NULL,
		NULL,
		NULL
	);
	pRunDlg(hwnd, NULL, NULL, NULL, NULL, 0);
	DestroyWindow(hwnd);
}

#endif //ndef BBTINY


// API: GetBBVersion
// Purpose: Returns the current version
// In: None
// Out: const char* = Formatted Version String

const char* GetBBVersion() { return BBAPPVERSION; }


// API: GetBBWnd
// Purpose: Returns the handle to the main Blackbox window

HWND GetBBWnd() { return BBhwnd; }


// API: GetUnderExplorer

bool GetUnderExplorer() { return g_underExplorer; }


// API: GetOSInfo

LPCSTR GetOSInfo()
{
	OSVERSIONINFO osInfo;
	memset(&osInfo, 0, sizeof(osInfo));
	osInfo.dwOSVersionInfoSize = sizeof(osInfo);
	GetVersionEx(&osInfo);

	const char* s = 0;
	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		if (osInfo.dwMinorVersion >= 90)
			s = "ME";
		else if (osInfo.dwMinorVersion >= 10)
			s = "98";
		else
			s = "95";
	}
	else if (osInfo.dwMajorVersion == 5)
	{
		if (osInfo.dwMinorVersion >= 1)
			s = "XP";
		else
			s = "2000";
	}
	else
		s = "NT";

	static char osinfo_buf[32];
	sprintf(osinfo_buf, "Windows %s", s);
	return osinfo_buf;
}


// API: GetBlackboxPath
// Purpose: Copies the path of the Blackbox executable to the specified buffer


char* WINAPI GetBlackboxPath(char* pszPath, int nMaxLen)
{
	GetModuleFileName(NULL, pszPath, nMaxLen);
	if (have_imp(pGetLongPathName))
		pGetLongPathName(pszPath, pszPath, nMaxLen);
	*(char*)file_basename(pszPath) = 0;
	return pszPath;
}


// API: GetBlackboxEditor

void GetBlackboxEditor(char* editor)
{
	replace_shellfolders(editor, Settings_preferredEditor, true);
}



static void reset_rcpaths()
{
	defaultrc_path[0] = 0;
	blackboxrc_path[0] = 0;
	extensionsrc_path[0] = 0;
	menurc_path[0] = 0;
	pluginrc_path[0] = 0;
	stylerc_path[0] = 0;
}

static const char* bbPath(const char* new_name, char* path, const char* default_name)
{
	if (new_name)
		FindRCFile(path, new_name, NULL);
	else if (0 == path[0] && default_name)
		FindRCFile(path, default_name, hMainInstance);
	return path;
}

const char* defaultrcPath()
{
	return defaultrc_path[0] ? defaultrc_path : NULL;
}


// API: bbrcPath

const char* bbrcPath(const char* other)
{
	return bbPath(other, blackboxrc_path, "blackbox");
}


// API: extensionsrcPath
// Purpose: Returns the handle to the extensionsrc file that is being used

const char* extensionsrcPath(const char* other)
{
	return bbPath(other, extensionsrc_path, "extensions");
}


// API: plugrcPath
// Purpose: Returns the handle to the plugins rc file that is being used

const char* plugrcPath(const char* other)
{
	return bbPath(other, pluginrc_path, "plugins");
}


// API: menuPath
// Purpose: Returns the handle to the menu file that is being used

const char* menuPath(const char* other)
{
	return bbPath(other, menurc_path, "menu");
}


// API: stylePath
// Purpose: Returns the handle to the style file that is being used

const char* stylePath(const char* other)
{
	return bbPath(other, stylerc_path, NULL);
}

