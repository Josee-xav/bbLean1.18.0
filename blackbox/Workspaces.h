/* ==========================================================================

  This file is part of the bbLean source code
  Copyright © 2001-2003 The Blackbox for Windows Development Team
  Copyright © 2004-2009 grischka

  http://bb4win.sourceforge.net/bblean
  http://developer.berlios.de/projects/bblean
  ========================================================================== */
#pragma once
#include "BB.h"

struct toptask {
    toptask  * next;
    tasklist * task;
};
struct StickyNode {
    StickyNode * next;
    HWND hwnd;
};
struct onbg_node {
    onbg_node * next;
    HWND hwnd;
};

class Workspaces
{
    int nScreens;
    int nScreensX, nScreensY;
    int currentScreen, lastScreen;
    int VScreenX, VScreenY, VScreenWidth, VScreenHeight;

    string_node * deskNames;          // workspace names
    string_node * stickyNamesList;    // application listed in StickyWindows.ini
    string_node * onBGNamesList;      // application listed in BGWindows.ini
    tasklist *    taskList;           // the list of tasks, in order as they were added
    toptask *     pTopTask;           // the list of tasks, in order as they were recently active
    HWND          activeTaskWindow;   // the current active taskwindow or NULL
    list_node *   toggled_windows;    // minimized windows by 'MinimizeAllWindows'
    StickyNode *  sticky_list;        // Sticky plugins & apps
    onbg_node *   onbg_list;          // onbg plugins & apps

public:
    Workspaces ();
    ~Workspaces ();
    void Init (int nostartup);
    void Reconfigure ();
    void Exit ();

    int GetScreenCount () const { return nScreens; }
    int GetScreenCurrent () const { return currentScreen; }
    void SetScreenCurrent (int n) { currentScreen = n; }
    void SetScreenLast (int n) { lastScreen = n; }
    int GetScreenCountX () const { return nScreensX; }
    int GetScreenCountY () const { return nScreensY; }
    int GetVScreenX () const { return VScreenX; }
    int GetVScreenY () const { return VScreenY; }
    int GetVScreenWidth () const { return VScreenWidth; }
    int GetVScreenHeight () const { return VScreenHeight; }

    bool GetScreenMetrics ();
    // activate the topwindow in the z-order of the current workspace
    bool FocusTopWindow ();
    void GatherWindows ();
    void GetCaptions ();
    LRESULT Command (UINT msg, WPARAM wParam, LPARAM lParam);
    void ToggleWindowVisibility (HWND);
    HWND GetActiveTaskWindow ();
    void TaskProc (WPARAM, HWND);
    void MakeSticky (HWND hwnd);
    void RemoveSticky (HWND hwnd);
    bool CheckSticky (HWND hwnd);
    bool CheckOnBG (HWND hwnd);
    void RemoveOnBG (HWND hwnd);
    void MakeOnBG (HWND hwnd);
    bool CheckStickyPlugin (HWND hwnd);
    bool CheckStickyName (HWND hwnd);
    bool CheckOnBgName (HWND hwnd);
    void SwitchToBBWnd () const;

    //@FIXME these may be private
    void SetTopTask (tasklist * tl, int set_where);
    bool mr_checktask (HWND hwnd);
    void get_desktop_info (DesktopInfo & deskInfo, int i) const;
    tasklist * AddTask (HWND hwnd);
    API_EXPORT tasklist const * GetTaskListPtr () const;
    int GetTaskListSize () const;
    API_EXPORT HWND GetTask (int index) const;
    API_EXPORT int GetActiveTask () const;
    API_EXPORT void GetDesktopInfo (DesktopInfo & deskInfo) const;
    void send_desk_refresh () const;
    void send_task_refresh () const;
    void workspaces_set_desk ();
    API_EXPORT bool GetTaskLocation(HWND hwnd, taskinfo * t);
    API_EXPORT bool SetTaskLocation (HWND hwnd, taskinfo const * t, UINT flags);
    void EnumTasks (TASKENUMPROC lpEnumFunc, LPARAM lParam);

protected:
    void DeskSwitch (int);
    void CleanTasks ();
    //void Workspaces_handletimer ();
    void SwitchToWindow (HWND hwnd);
    int NextDesk (int d);

private:
    void SetNames ();
    void AddDesktop (int);
    void SetWorkspaceNames (const char * names);
    void WS_ShadeWindow (HWND hwnd);
    void WS_GrowWindowHeight (HWND hwnd);
    void WS_GrowWindowWidth (HWND hwnd);
    void WS_LowerWindow (HWND hwnd);
    void WS_RaiseWindow (HWND hwnd);
    void WS_MaximizeWindow (HWND hwnd);
    void WS_MinimizeWindow (HWND hwnd);
    void WS_CloseWindow (HWND hwnd);
    void WS_RestoreWindow (HWND hwnd);
    void WS_BringToFront (HWND hwnd, bool to_current);
    void WS_LoadStickyNamesList ();
    void WS_LoadOnBGNamesList ();
    bool check_onbg_plugin (HWND hwnd);

    void min_rest_helper (int cmd);
    void MinimizeAllWindows ();
    void RestoreAllWindows ();

    HWND get_top_window (int scrn) const;
    void MoveWindowToWkspc (HWND hwnd, int desk, bool switchto);
    void NextWindow (bool allDesktops, int dir);
    void del_from_toptasks (tasklist * tl);
    void get_caption (tasklist * tl, int force);
    void RemoveTask (tasklist * tl);
    int FindTask (HWND hwnd);
    void exit_tasks ();
    void init_tasks ();

    void send_task_message (HWND hwnd, UINT msg) const;
    HWND get_default_window (HWND hwnd) const;
    void switchToDesktop (int n) const;
    void setDesktop (HWND hwnd, int n, bool switchto) const;
};

API_EXPORT Workspaces & getWorkspaces ();
