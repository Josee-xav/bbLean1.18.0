#include "SearchItem.h"
#include "../BB.h"
#include "../Settings.h"
#include "Menu.h"
#include "RecentItem.h"
#include <Search/lookup.h>
#include <Search/config.h>
#include <Search/tmp.h>
#include <Search/complete.h>
#include <Shlwapi.h>
//#include <richedit.h>
/*HWND CreateRichEdit(HWND hwndOwner,        // Dialog box handle.  int x, int y,          // Location.  int width, int height, // Dimensions.  HINSTANCE hinst)       // Application or DLL instance.
{
    LoadLibrary(TEXT("Msftedit.dll"));
    HWND hwndEdit= CreateWindowEx(0, TEXT("RICHEDIT50W"), TEXT("Type here"),
        ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        x, y, width, height,
        hwndOwner, NULL, hinst, NULL);
    return hwndEdit;
}*/

namespace {
	MenuItem * MakeMenuItemSearch(Menu * PluginMenu, const char * Title, const char * Cmd, const char * init_string)
	{
		return helper_menu(PluginMenu, Title, MENU_ID_STRING, new SearchItem(Cmd, init_string));
	}

	MenuItem * MakeMenuItemResultContext(Menu * PluginMenu, const char * Title, const char * Cmd
			, E_ResultItemAction action, tstring const & typed, tstring const & fname, tstring const & fpath)
	{
		ResultItemContext * r = new ResultItemContext(Cmd, NLS1(Title));
		r->m_action = action;
		r->m_typed = typed;
		r->m_fname = fname;
		r->m_fpath = fpath;
		PluginMenu->AddMenuItem(r);
		return r;
	}
	MenuItem * MakeMenuResultItem (Menu * PluginMenu, const char * Title, const char * Cmd
			, tstring const & typed, tstring const & fname, tstring const & fpath)
	{
		ResultItem * r = new ResultItem(Cmd, NLS1(Title), false);
		r->m_typed = typed;
		r->m_pidl_list = get_folder_pidl_list(fpath.c_str());
		return PluginMenu->AddMenuItem(r);
	}
}

SearchItem::SearchItem (const char* pszCommand, const char *init_string)
	: StringItem(pszCommand, init_string)
	, m_bbPath(getBlackboxPath())
	, m_hText(0)
	, m_wpEditProc(0)
	, m_textrect()
{
	bb::search::startLookup(m_bbPath);
}

SearchItem::~SearchItem ()
{
	if (m_hText)
	{
		disableCompletion();
		DestroyWindow(m_hText);
		m_hText = NULL;
	}
}

void SearchItem::Paint (HDC hDC)
{
	RECT r;
	HFONT hFont;
	int x, y, w, h, padd;
	if (Settings_menu.showBroams)
	{
		if (m_hText)
		{
			DestroyWindow(m_hText);
			m_hText = NULL;
		}
		m_Justify = MENUITEM_STANDARD_JUSTIFY;
		MenuItem::Paint(hDC);
		return;
	}

	m_Justify = MENUITEM_CUSTOMTEXT;
	MenuItem::Paint(hDC);

	GetTextRect(&r);
	if (EqualRect(&m_textrect, &r))
		return;

	m_textrect = r;

	if (NULL == m_hText)
	{
		TCHAR const indexing_msg[] = TEXT("Indexing...");


		TCHAR const * text = 0;
		if (bb::search::getLookup().IsIndexing())
			text = indexing_msg;
		else
			text = m_pszTitle; //@TODO: last search?

		//m_hText = CreateRichEdit(m_pMenu->m_hwnd, 0, 0, 0, 0, hMainInstance);
		m_hText = CreateWindow(TEXT("EDIT"), text, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE, 0, 0, 0, 0, m_pMenu->m_hwnd, (HMENU)1234, hMainInstance, NULL);
		SetWindowLongPtr(m_hText, GWLP_USERDATA, (LONG_PTR)this);
		m_wpEditProc = (WNDPROC)SetWindowLongPtr(m_hText, GWLP_WNDPROC, (LONG_PTR)EditProc);
		int const n = GetWindowTextLength(m_hText);
		SendMessage(m_hText, EM_SETSEL, 0, n);
		SendMessage(m_hText, EM_SCROLLCARET, 0, 0);
		m_pMenu->m_hwndChild = m_hText;
		if (GetFocus() == m_pMenu->m_hwnd)
			SetFocus(m_hText);

		bool rebuilding = false;
		if (!bb::search::getLookup().IsIndexing() && !bb::search::getLookup().IsLoaded())
		{
			//@TODO: ask user to rebuild index?
			bool const loaded = bb::search::getLookup().LoadOrBuild(false);
			if (!loaded)
			{
				rebuilding = true;
				// not loaded, have to build index
				size_t const ln = sizeof(indexing_msg) / sizeof(*indexing_msg);
				SendMessage(m_hText, WM_SETTEXT, (WPARAM)ln + 1, (LPARAM)indexing_msg);
				SendMessage(hText, EM_SETREADONLY, 0, TRUE);
			}
		}

		if (!rebuilding && bb::search::getLookup().IsLoaded())
		{
			tstrings tmp;
			tmp.reserve(bb::search::getLookup().m_index.m_props.size());
			for (bb::search::Props const & p : bb::search::getLookup().m_index.m_props)
				tmp.push_back(p.m_fname);
			enableCompletion(m_hText, tmp);
		}
	}

	hFont = MenuInfo.hFrameFont;
	SendMessage(m_hText, WM_SETFONT, (WPARAM)hFont, 0);

	x = r.left - 1;
	y = r.top + 2;
	h = r.bottom - r.top - 4;
	w = r.right - r.left + 2;

	SetWindowPos(m_hText, NULL, x, y, w, h, SWP_NOZORDER);

	padd = imax(0, (h - get_fontheight(hFont)) / 2);
	r.left	= padd+2;
	r.right = w - (padd+2);
	r.top		= padd;
	r.bottom = h - padd;
	SendMessage(m_hText, EM_SETRECT, 0, (LPARAM)&r);
}

void SearchItem::Measure (HDC hDC, SIZE * size, StyleItem * pSI)
{
		StringItem::Measure(hDC, size, pSI);
}

void SearchItem::Invoke (int button)
{
	if (button == INVOKE_RET)
		OnInput();
}

void SearchItem::OnInput ()
{
	int const len = SendMessage(m_hText, WM_GETTEXTLENGTH, 0, 0);
	char * buffer = static_cast<char *>(alloca(sizeof(char) * (len + 1)));
	SendMessage(m_hText, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)buffer);

	if (bb::search::getLookup().IsIndexing())
	{
		TCHAR const indexing_msg[] = TEXT("Indexing...");

		size_t const ln = sizeof(indexing_msg) / sizeof(*indexing_msg);
		SendMessage(m_hText, WM_SETTEXT, (WPARAM)ln + 1, (LPARAM)indexing_msg);
		SendMessage(hText,  EM_SETREADONLY, 0, TRUE);
		return;
	}
	SendMessage(hText,  EM_SETREADONLY, 0, FALSE);

	// query
	tstring const what(buffer);
	std::vector<tstring> hkeys;
	std::vector<tstring> hres;
	std::vector<tstring> ikeys;
	std::vector<tstring> ires;
	bb::search::getLookup().Find(what, hkeys, hres, ikeys, ires);

	// result menu creation
	Menu * menu = MakeNamedMenu(NLS0("Search results"), "Search_results", true);

	char broam[1024];
	char text[1024];

	if (hres.size() > 0)
		MakeMenuNOP(menu, TEXT("History"));
	bool first_activated = false;
	for (size_t i = 0, ie = hres.size(); i < ie; ++i)
	{
		// result menu item
		_snprintf_s(broam, 1024, "@BBCore.Exec \"%s\"", hres[i].c_str());
		_snprintf_s(text, 1024, "%s", hres[i].c_str());
		MenuItem * mi = MakeMenuResultItem(menu, text, broam, what, hkeys[i], hres[i]);
		MenuItemOption(mi, BBMENUITEM_JUSTIFY, DT_RIGHT);

		if (i == 0)
		{
			mi->Active(true);
			first_activated = true;
		}

		// context menu for result item
		Menu * ctx = MakeNamedMenu(NLS0("Result context"), NULL, true);
		MakeMenuItem(ctx, TEXT("run"), broam, false);
		MakeMenuItemResultContext(ctx, TEXT("run in cmd"), NULL, e_RunInCmd, what, hkeys[i], hres[i]);
		MakeMenuItemResultContext(ctx, TEXT("run as admin"), NULL, e_RunAsAdmin, what, hkeys[i], hres[i]);
		MakeMenuItemResultContext(ctx, TEXT("unpin from history"), NULL, e_UnpinFromHistory, what, hkeys[i], hres[i]);

		_snprintf_s(broam, 1024, "@BBCore.Exec explorer /select,\"%s\"", ires[i].c_str()); // explorer /select,c:\windows\calc.exe 
		MakeMenuItem(ctx, TEXT("open explorer here"), broam, false);
		mi->m_pRightmenu = ctx;
		//MenuOption(ctx, BBMENU_MAXWIDTH | BBMENU_CENTER | BBMENU_ONTOP, 512);
	}

	if (hres.size() > 0)
	{
		MakeMenuNOP(menu, nullptr);
	}
	MakeMenuNOP(menu, TEXT("Index"));
	for (size_t i = 0, ie = ires.size(); i < ie; ++i)
	{
		// result menu item
		_snprintf_s(broam, 1024, "@BBCore.Exec \"%s\"", ires[i].c_str());
		_snprintf_s(text, 1024, "%s", ires[i].c_str());
		MenuItem * mi = MakeMenuResultItem(menu, text, broam, what, ikeys[i], ires[i]);
		MenuItemOption(mi, BBMENUITEM_JUSTIFY, DT_RIGHT);
		if (i == 0 && !first_activated)
		{
			mi->Active(true);
			first_activated = true;
		}

		// context menu for result item
		Menu * ctx = MakeNamedMenu(NLS0("Result context"), NULL, true);
		MakeMenuItem(ctx, TEXT("run"), broam, false);
		MakeMenuItemResultContext(ctx, TEXT("run in cmd"), NULL, e_RunInCmd, what, ikeys[i], ires[i]);
		MakeMenuItemResultContext(ctx, TEXT("run as admin"), NULL, e_RunAsAdmin, what, ikeys[i], ires[i]);
		MakeMenuItemResultContext(ctx, TEXT("pin to history"), NULL, e_PinToHistory, what, ikeys[i], ires[i]);
		MakeMenuItemResultContext(ctx, TEXT("pin to iconbox"), NULL, e_PinToIconBox, what, ikeys[i], ires[i]);
		MakeMenuItemResultContext(ctx, TEXT("forget"), NULL, e_UnpinFromIndex, what, ikeys[i], ires[i]);

		_snprintf_s(broam, 1024, "@BBCore.Exec explorer /select,\"%s\"", ires[i].c_str()); // explorer /select,c:\windows\calc.exe 
		MakeMenuItem(ctx, TEXT("open explorer here"), broam, false);
		mi->m_pRightmenu = ctx;
	}

	MenuOption(menu, BBMENU_MAXWIDTH | BBMENU_CENTER | BBMENU_ONTOP, 512);
	//MenuOption(menu, BBMENU_MAXWIDTH | BBMENU_CENTER | BBMENU_PINNED | BBMENU_ONTOP, 512);
	ShowMenu(menu);

	//		if (pRect)
	//MenuOption(m, BBMENU_RECT | BBMENU_NOTITLE | BBMENU_SYSMENU, pRect);
}

//===========================================================================
LRESULT CALLBACK SearchItem::EditProc(HWND hText, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SearchItem * pItem = reinterpret_cast<SearchItem *>(GetWindowLongPtr(hText, GWLP_USERDATA));
	LRESULT r = 0;
	Menu * pMenu = pItem->m_pMenu;

	pMenu->incref();
	switch(msg)
	{
		// Send Result
		case WM_MOUSEMOVE:
			PostMessage(pMenu->m_hwnd, WM_MOUSEMOVE, wParam, MAKELPARAM(10, pItem->m_nTop+2));
			break;

		// Key Intercept
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_DOWN:
				case VK_UP:
				case VK_TAB:
					pItem->Invoke(0);
					pItem->next_item(wParam);
					goto leave;

				case VK_RETURN:
					pItem->Invoke(INVOKE_RET);
					pItem->next_item(0);
					goto leave;

				case VK_ESCAPE:
					SetWindowText(hText, pItem->m_pszTitle);
					pItem->next_item(0);
					goto leave;
			}
			//pItem->OnInput();
			break;
		}
		case WM_CHAR:
		{
			switch (wParam)
			{
				case 'A' - 0x40: // ctrl-A: select all
						SendMessage(hText, EM_SETSEL, 0, GetWindowTextLength(hText));
				case 13:
				case 27:
						goto leave;
			}
			break;
		}
		// --------------------------------------------------------
		// Paint

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT r;
			StyleItem *pSI;

			hdc = BeginPaint(hText, &ps);
			GetClientRect(hText, &r);
			static int i = 0;
			if (bb::search::getLookup().IsIndexing())
			{
				pSI = &mStyle.MenuFrame;
				COLORREF c0 = RGB(128, 0, 0);
				COLORREF c1 = RGB(128, 64, 64);
				COLORREF cs0 = RGB(64, 0, 0);
				COLORREF cs1 = RGB(64, 32, 32);
				MakeGradientEx(hdc, r, pSI->type, c0, c1, cs0, cs1, pSI->interlaced, BEVEL_SUNKEN, BEVEL1, 0, 0, 0);
			}
			else
			{
				pSI = &mStyle.MenuFrame;
				MakeGradientEx(hdc, r, pSI->type, pSI->Color, pSI->ColorTo, pSI->ColorSplitTo, pSI->ColorToSplitTo, pSI->interlaced, BEVEL_SUNKEN, BEVEL1, 0, 0, 0);
			}

			CallWindowProc(pItem->m_wpEditProc, hText, msg, (WPARAM)hdc, lParam);
			EndPaint(hText, &ps);
			goto leave;
		}

		case WM_ERASEBKGND:
			r = TRUE;
			goto leave;

		case WM_DESTROY:
			pItem->hText = NULL;
			pMenu->m_hwndChild = NULL;
			break;

		case WM_SETFOCUS:
			break;

		case WM_KILLFOCUS:
			pItem->Invoke(0);
			break;
	}
	r = CallWindowProc(pItem->m_wpEditProc, hText, msg, wParam, lParam);
leave:
	pMenu->decref();
	return r;
}

//////////////////////////////////////////////////////////////////////////////

ResultItem::ResultItem (const char * pszCommand, const char * pszTitle, bool bChecked)
	: CommandItem(pszCommand, pszTitle, bChecked)
{ }
ResultItem::~ResultItem ()
{ }
void ResultItem::Mouse (HWND hwnd, UINT uMsg, DWORD wParam, DWORD lParam)
{
	CommandItem::Mouse(hwnd, uMsg, wParam, lParam);
}
void ResultItem::Invoke (int button)
{
	CommandItem::Invoke(button);

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	errno_t err = _splitpath_s(m_pszTitle, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT );

	//tstring const fpath = m_pszTitle;
	//bb::search::getLookup().m_history.Insert(m_typed, fname, fpath);
	//bb::search::getLookup().m_history.Save();
}

ResultItemContext::ResultItemContext (const char* pszCommand, const char* pszTitle)
	: CommandItem(pszCommand, pszTitle, false)
	, m_action(e_Run)
{ }
ResultItemContext::~ResultItemContext ()
{ }
void ResultItemContext::Mouse (HWND hwnd, UINT uMsg, DWORD wParam, DWORD lParam)
{
	CommandItem::Mouse(hwnd, uMsg, wParam, lParam);
}

// CreateLink - Uses the Shell's IShellLink and IPersistFile interfaces to create and store a shortcut to the specified object. 
//
// Returns the result of calling the member functions of the interfaces. 
//
// Parameters:
// lpszPathObj  - Address of a buffer that contains the path of the object, including the file name.
// lpszPathLink - Address of a buffer that contains the path where the Shell link is to be stored, including the file name.
// lpszDesc     - Address of a buffer that contains a description of the Shell link, stored in the Comment field of the link properties.
HRESULT CreateLink(LPCTSTR lpszPathObj, LPCTSTR lpszPathLink, LPCTSTR lpszDesc) 
{ 
    HRESULT hres; 
    IShellLink * psl = 0;
 
    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl); 
    if (SUCCEEDED(hres)) 
    { 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target and add the description. 
        psl->SetPath(lpszPathObj); 
        psl->SetDescription(lpszDesc); 
 
        // Query IShellLink for the IPersistFile interface, used for saving the shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
 
        if (SUCCEEDED(hres)) 
        { 
            WCHAR wsz[MAX_PATH]; 
 
            // Ensure that the string is Unicode. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH); 
            // @TODO: Add code here to check return value from MultiByteWideChar for success.
            
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(wsz, TRUE); 
            ppf->Release(); 
        } 
        psl->Release(); 
    } 
    return hres;
}

void ResultItemContext::Invoke (int button)
{
	CommandItem::Invoke(button);

	switch (m_action)
	{
		case e_RunInCmd:
		{
			//cmd.exe /k "foo.exe"
		} break;
		case e_PinToHistory:
		{
			bb::search::getLookup().m_history.Insert(m_typed, m_fname, m_fpath);
			bb::search::getLookup().m_history.Save();
		} break;
		case e_UnpinFromHistory:
		{
			bb::search::getLookup().m_history.Remove(m_typed, m_fname, m_fpath);
			bb::search::getLookup().m_history.Save();
		} break;
		case e_PinToIconBox:
		{
			TCHAR bbpath[1024];
			GetBlackboxPath(bbpath, 1024);
			tstring path(bbpath); // @TODO: tmp
			path += "search";
			CreateDirectory(path.c_str(), NULL);
			post_command_fmt("@bbIconBox.create.unique %s", path.c_str());

			tstring linkfpath = path + "\\" + m_fname + ".lnk";
			if (!PathFileExists(linkfpath.c_str()))
			{
				CreateLink(m_fpath.c_str(), linkfpath.c_str(), TEXT("pinned search result"));
				dbg_printf("create link src=%s tgt=%s", m_fpath.c_str(), linkfpath.c_str());
			}
			else
			{
				for (int i = 1; i < 32; ++i)
				{
					linkfpath = path + "\\" + m_fname + "_" + std::to_string(i) + ".lnk";
					if (PathFileExists(linkfpath.c_str()))
						continue;
					CreateLink(m_fpath.c_str(), linkfpath.c_str(), TEXT("pinned search result"));
					dbg_printf("create link src=%s tgt=%s", m_fpath.c_str(), linkfpath.c_str());
					break;
				}
			}
		} break;
		case e_UnpinFromIndex:
		{
			bb::search::getLookup().m_index.Forget(m_fpath);
			bb::search::getLookup().m_index.RemoveFromIndex(m_fname, m_fpath);
			bb::search::getLookup().m_index.SaveForget();
			bb::search::getLookup().m_index.Save();
		} break;
	}
}

ArgItem::ArgItem (const char* pszCommand, const char* pszTitle)
	: CommandItem(pszCommand, pszTitle, false)
{ }
ArgItem::~ArgItem ()
{
	if (m_combo)
	{
		disableCompletion();
		DestroyWindow(m_combo);
		m_combo = NULL;
	}

}
/*
TCHAR A[16]; 
int  k = 0; 

memset(&A,0,sizeof(A));       
for (k = 0; k <= 8; k += 1)
{
    wcscpy_s(A, sizeof(A)/sizeof(TCHAR),  (TCHAR*)Planets[k]);

    // Add string to combobox.
    SendMessage(hWndComboBox,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) A); 
}
  
// Send the CB_SETCURSEL message to display an initial item 
//  in the selection field  
SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)2, (LPARAM)0);
*/

void ArgItem::Paint (HDC hDC)
{
	RECT r;
	HFONT hFont;
	int x, y, w, h, padd;
	if (Settings_menu.showBroams)
	{
		if (m_combo)
		{
			DestroyWindow(m_combo);
			m_combo = NULL;
		}
		m_Justify = MENUITEM_STANDARD_JUSTIFY;
		MenuItem::Paint(hDC);
		return;
	}

	m_Justify = MENUITEM_CUSTOMTEXT;
	MenuItem::Paint(hDC);

	GetTextRect(&r);
	if (EqualRect(&m_comboRect, &r))
		return;

	m_comboRect = r;

	if (NULL == m_combo)
	{
		HWND m_combo = CreateWindow(WC_COMBOBOX, TEXT(""), 
							 CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
							 0, 0, 0, 0, m_pMenu->m_hwnd, NULL, hMainInstance, NULL);

		SetWindowLongPtr(m_combo, GWLP_USERDATA, (LONG_PTR)this);
		m_comboProc = (WNDPROC)SetWindowLongPtr(m_combo, GWLP_WNDPROC, (LONG_PTR)ComboProc);
		//int const n = GetWindowTextLength(m_combo);
		//SendMessage(m_combo, EM_SETSEL, 0, n);
		//SendMessage(m_combo, EM_SCROLLCARET, 0, 0);
		m_pMenu->m_hwndChild = m_combo;
		if (GetFocus() == m_pMenu->m_hwnd)
			SetFocus(m_combo);
	}

	hFont = MenuInfo.hFrameFont;
	SendMessage(m_combo, WM_SETFONT, (WPARAM)hFont, 0);

	x = r.left - 1;
	y = r.top + 2;
	h = r.bottom - r.top - 4;
	w = r.right - r.left + 2;

	SetWindowPos(m_combo, NULL, x, y, w, h, SWP_NOZORDER);

	padd = imax(0, (h - get_fontheight(hFont)) / 2);
	r.left	= padd+2;
	r.right = w - (padd+2);
	r.top		= padd;
	r.bottom = h - padd;
	SendMessage(m_combo, EM_SETRECT, 0, (LPARAM)&r);
}

void ArgItem::Measure (HDC hDC, SIZE * size, StyleItem * pSI)
{
	CommandItem::Measure(hDC, size, pSI);
}

void ArgItem::Invoke (int button)
{
	if (button == INVOKE_RET)
		OnInput();
}

void ArgItem::OnInput ()
{
	int const len = SendMessage(m_combo, WM_GETTEXTLENGTH, 0, 0);
	char * buffer = static_cast<char *>(alloca(sizeof(char) * (len + 1)));
	SendMessage(m_combo, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)buffer);
}

LRESULT CALLBACK ArgItem::ComboProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ArgItem * pItem = reinterpret_cast<ArgItem *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	LRESULT r = 0;
	Menu * pMenu = pItem->m_pMenu;

	pMenu->incref();
	switch(msg)
	{
		// Send Result
		case WM_MOUSEMOVE:
			PostMessage(pMenu->m_hwnd, WM_MOUSEMOVE, wParam, MAKELPARAM(10, pItem->m_nTop+2));
			break;

		// Key Intercept
		case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_DOWN:
				case VK_UP:
				case VK_TAB:
					pItem->Invoke(0);
					pItem->next_item(wParam);
					goto leave;

				case VK_RETURN:
					pItem->Invoke(INVOKE_RET);
					pItem->next_item(0);
					goto leave;

				case VK_ESCAPE:
					SetWindowText(hwnd, pItem->m_pszTitle);
					pItem->next_item(0);
					goto leave;
			}
			//pItem->OnInput();
			break;
		}
		case WM_COMMAND:

			if(HIWORD(wParam) == CBN_SELCHANGE)
			// If the user makes a selection from the list:
			//   Send CB_GETCURSEL message to get the index of the selected list item.
			//   Send CB_GETLBTEXT message to get the item.
			//   Display the item in a messagebox.
			{ 
				int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				TCHAR  ListItem[256];
				SendMessage((HWND) lParam, (UINT) CB_GETLBTEXT, (WPARAM) ItemIndex, (LPARAM) ListItem);
				//MessageBox(hwnd, (LPCWSTR) ListItem, TEXT("Item Selected"), MB_OK);                        
				goto leave;
			}
			break;
		// --------------------------------------------------------
		// Paint
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT r;
			StyleItem *pSI;

			hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &r);
			pSI = &mStyle.MenuFrame;
			MakeGradientEx(hdc, r, pSI->type, pSI->Color, pSI->ColorTo, pSI->ColorSplitTo, pSI->ColorToSplitTo, pSI->interlaced, BEVEL_SUNKEN, BEVEL1, 0, 0, 0);
			CallWindowProc(pItem->m_comboProc, hwnd, msg, (WPARAM)hdc, lParam);
			EndPaint(hwnd, &ps);
			goto leave;
		}

		case WM_DESTROY:
			pItem->m_combo = NULL;
			pMenu->m_hwndChild = NULL;
			break;

		case WM_SETFOCUS:
			break;

		case WM_KILLFOCUS:
			pItem->Invoke(0);
			break;
	}
	r = CallWindowProc(pItem->m_comboProc, hwnd, msg, wParam, lParam);
leave:
	pMenu->decref();
	return r;
}


