#pragma once
// The item classes stuff:

#include "BBApi.h"
#include "win0x500.h"
#include "bblib.h"
#include "bbPlugin.h"
#include "drawico.h"
#ifndef NO_TIPS
#	include "tooltips.h"
#endif
#include <shellapi.h>
#include <shlobj.h>
#include <time.h>
#include "Workspaces.h"

#include "bbLeanClasses.h"

#define CLASS_NAME_MAX 80
#define MY_BROAM "@BlackBoxBar"

#include "BuffBmp.h"
#ifndef NO_DROP
#	include "TinyDropTarg.h"
#endif

// flags for invalidation
#define UPD_DRAW 0x180
#define UPD_NEW  0x181

enum E_BarTimers {
	CLOCK_TIMER = 2,
	LANGUAGE_TIMER = 3,
	LABEL_TIMER = 4,
	TASK_RISE_TIMER = 5,
	TASKLOCK_TIMER = 6,
	GESTURE_TIMER = 7,
	CHECK_FULLSCREEN_TIMER = 8
};

//====================
extern HWND BBhwnd;
extern int currentScreen;
extern WCHAR screenName[256];

extern StyleItem NTaskStyle;
extern StyleItem ATaskStyle;

extern StyleItem* I;
extern int styleBevelWidth;
extern int styleBorderWidth;
extern COLORREF styleBorderColor;
extern int TBJustify;

//====================
#define NIF_INFO 0x00000010
#define NIN_BALLOONSHOW (WM_USER + 2)
#define NIN_BALLOONHIDE (WM_USER + 3)
#define NIN_BALLOONTIMEOUT (WM_USER + 4)
#define NIN_BALLOONUSERCLICK (WM_USER + 5)



struct barinfo : plugin_info {
#define BARINFO_FIRSTITEM cfg_list

	struct config* cfg_list;
	struct pmenu* cfg_menu;
	class BuffBmp* pBuff;
	class LeanBar* pLeanBar;
	class TinyDropTarget* m_TinyDropTarget;
	struct tasklist* taskList;
	struct ToolbarInfo* TBInfo;

	// configuration
	int  widthPercent;
	int  minHeight;
	bool reverseTasks;
	int  saturationValue;
	int  hueIntensity;
	int iconSize;
	bool arrowBullets;
	char strftimeFormat[60];
	bool taskSysmenu;
	int TaskStyle;
	int taskMaxWidth;
	bool currentOnly;
	bool task_with_border;
	bool autoFullscreenHide;
	bool setDesktopMargin;
	bool sendToSwitchTo;
	bool sendToGesture;
	bool clockTips;
	char strftimeTipsFormat[60];

	// clock click
	char clkBtn1Click[MAX_PATH];
	char clkBtn2Click[MAX_PATH];
	char clkBtn3Click[MAX_PATH];
	char clkBtn4Click[MAX_PATH];
	char clkBtn5Click[MAX_PATH];
	char clkBtn6Click[MAX_PATH];

	struct trayNode {
		trayNode* next;
		bool hidden;
		bool mark;
		bool tip_checked;
		HWND hWnd;
		unsigned uID;
		int index;
		char class_name[CLASS_NAME_MAX];
	};

	// another list for the info as loaded from/saved to bbleanbar.rc
	struct traySave {
		traySave* next;
		unsigned uID;
		char class_name[CLASS_NAME_MAX];
	};

	// painting
	HFONT hFont;
	HDC hdcPaint;
	RECT* p_rcPaint;

	// metrics
	int buttonH;
	int labelH;
	int bbLeanBarLineHeight[8];
	int BarLines;
	int TASK_ICON_SIZE;
	int TRAY_ICON_SIZE;
	int buttonBorder;
	int labelIndent;
	int labelBorder;

	// runtime temp variables
	int old_place;
	WCHAR clockTime[80];
	WCHAR clockTimeTips[80];
	WCHAR windowlabel[1024];
	int labelWidth;
	int clockWidth;
	bool force_button_pressed;
	bool ShowingExternalLabel;
	bool near_top;
	bool on_multimon;
	bool has_tray;
	bool has_tasks;
	bool show_kbd_layout;
	bool gesture_lock;
	bool full_screen_hidden;
	int mon_top, mon_bottom;

	HWND task_over_hwnd;
	HWND task_lock;

	char instance_key[40];
	int n_index;

	// The string, which describes the outlay of the bar
	char item_string[32];
	// an item has captured the mouse
	class baritem* capture_item;

	void about_box();

	// --------------------------------------------------------------
	// class ctor & dtor

	barinfo();

	~barinfo();

	// --------------------------------------------------------------
	// class methods

	void process_broam(const char* temp, int f);
	LRESULT wnd_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* ret);
	void show_menu(bool);
	void desktop_margin_fn();
	void pos_changed();
	void set_tbinfo();
	void reset_tbinfo();

	void set_screen_info();
	void set_clock_string();
	void update_windowlabel();
	//int get_text_width (const char * cp, StyleItem * si);
	int GetTextWidth(WCHAR const* cp, StyleItem* si);

	void update(int);

	void post_trayicon_message(UINT wParam);
	bool check_fullscreen_window();
	void GetRCSettings();
	void WriteRCSettings();
	void GetStyleSettings();
	void make_cfg();

	// --------------------------------------------------------------
	// tasklist support - wrapper for the GetTask... functions
	// to implement the 'current tasks only' feature

	const struct tasklist* GetTaskListPtrEx();

	static BOOL task_enum_func(const tasklist* tl, LPARAM lParam);

	void DelTasklist();

	void NewTasklist();

	tasklist* GetTaskPtrEx(int pos);

	int GetTaskListSizeEx();

	HWND GetTaskWindowEx(int i);

	// --------------------------------------------------------------
	// traylist support - wrapper for the SystemTray functions
	// to implement the 'hide icons' feature


// there are two lists that hold the additional information

// one list corresponding to all icons from the core.
// hWnd and uId are to identify the icon for lookup, index is
// its orignal index in the core's list, true for hidden means the
// icon is not shown unless trayShowAll is true.


	traySave* traySaveList;
	trayNode* trayList;
	int trayVisiblesCount;
	bool trayShowAll;

	// clean up everything with the tray wrapper
	void DelTraylist();

	// -----------------------------------------------
	// these three are the wrappers that replace the
	// previous plain implementation

	int GetTraySizeEx();

	systemTray* GetTrayIconEx(int i);

	// -----------------------------------------------

	// get icon node from index, counting only visibles
	trayNode* GetTrayNode(int vis_index);

	// get real index from visibles index
	int RealTrayIndex(int vis_index);

	// build / refresh secondary trayList
	void NewTraylist();

	// change hidden state:  1:show,  0:hide, -1:toggle
	void trayShowIcon(int index, int state);

	// change show all state:  1:on,  0:off, -1:toggle
	void trayToggleShowAll(int state);

	// save info about hidden icons to rc (by uID and window classname)
	void traySaveHidden(void);

	// load hidden icons' info from rc (uID and window classname)
	void trayLoadHidden(void);

	// some icons dont have a tip until mouse hoover.
	// So try to get the needed tips for the menu with some faked mouse moves
	void forceTips(void);

	// show menu with tray icons
	void trayMenu(bool pop);
};