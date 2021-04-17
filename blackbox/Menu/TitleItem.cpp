/* ==========================================================================

  This file is part of the bbLean source code
  Copyright � 2001-2003 The Blackbox for Windows Development Team
  Copyright � 2004-2009 grischka

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

// the menu title bar

#include "../BB.h"
#include "../Settings.h"
#include "Menu.h"

//===========================================================================

void TitleItem::Paint(HDC hDC)
{
    RECT rect;
    StyleItem *pSI = &mStyle.MenuTitle;
    int indent = MenuInfo.nTitleIndent;
    int justify = pSI->Justify | DT_MENU_STANDARD;
    int bw = pSI->borderWidth;
    COLORREF bc = pSI->borderColor;

    GetItemRect(&rect);

    if (false == pSI->parentRelative) {
        MakeStyleGradient(hDC, &rect, pSI, mStyle.menuTitleLabel && pSI->bordered);
        if (false == mStyle.menuTitleLabel && this->next && bw)
            draw_line_h(hDC, rect.left, rect.right, rect.bottom, bw, bc);
    }

    rect.left  += indent;
    rect.right -= indent;

    if (pSI->parentRelative) {
        draw_line_h(hDC, rect.left, rect.right, rect.bottom, bw, bc);
    }

    //bbDrawText(hDC, GetDisplayString(), &rect, justify, pSI->TextColor);
	/* BlackboxZero 1.5.2012 */
	if (pSI->FontHeight > 1)
		BBDrawTextAlt(hDC, GetDisplayString(), -1, &rect, justify, pSI);
}

//===========================================================================

HCURSOR get_moving_cursor(void)
{
    HCURSOR hC = NULL;
	bool lHand = ReadBool(extensionsrcPath(), "blackbox.options.LeftHandedCursor:", false);
    if (false == Settings_useDefCursor)
		hC = LoadCursor(hMainInstance, lHand ? (char*)IDC_MOVEMENUL : (char*)IDC_MOVEMENU);
    if (NULL == hC)
        hC = LoadCursor(NULL, IDC_SIZEALL);
    return hC;
}

//===========================================================================

void TitleItem::Mouse(HWND hwnd, UINT uMsg, DWORD wParam, DWORD lParam)
{
    Menu *p = m_pMenu;

    switch(uMsg) {
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        if (m_bActive) {
            if (wParam & MK_SHIFT) {
                if (p->m_pidl_list)
                    ShowContextMenu(NULL, first_pidl(p->m_pidl_list));
            } else {
                p->HideNow();
            }
        }
        break;

    case WM_LBUTTONDBLCLK:
        p->m_bOnTop = false == p->m_bOnTop;
        p->SetZPos();
        if (wParam & MK_SHIFT) {
            p->m_bIconized = false == p->m_bIconized;
            p->Redraw(0);
        }
        break;

    case WM_LBUTTONDOWN:
        UpdateWindow(hwnd);
        p->SetPinned(true);
        SetCursor(get_moving_cursor());
        DefWindowProc(hwnd, WM_SYSCOMMAND, 0xf012, 0);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        for (;;) {
            p->SetZPos();
            if (p->m_bOnTop || window_under_mouse() == p->m_hwnd)
                break;
            // if the menu was moved over a topmost window, make it topmost also.
            p->m_bOnTop = true;
        }
        break;

    case WM_MOUSEMOVE:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        Active(1);
        break;
    }
}

//===========================================================================
void MenuGrip::Paint(HDC hDC)
{
  StyleItem *pSI = &mStyle.MenuGrip;
  RECT rect; GetItemRect(&rect);
  //int spacing = MenuInfo.nTitleIndent;
  int spacing = pSI->marginWidth;
  int just = pSI->Justify | DT_MENU_STANDARD;
  int bw = pSI->borderWidth;
  COLORREF bc = pSI->borderColor;

  //Adjust to for gradient smaller than the menu window?
  rect.left  += spacing;
  rect.right -= spacing;

  rect.bottom -= bw;
  if (false == pSI->parentRelative)
  {
    MakeStyleGradient(hDC, &rect, pSI, pSI->marginWidth?(pSI->bordered):(false));
    if ( pSI->bordered && !(pSI->marginWidth) ) {
      draw_line_h(hDC, rect.left, rect.right, rect.top, bw, bc);
    }
  }

  if (pSI->parentRelative)
  {
    draw_line_h(hDC, rect.left, rect.right, rect.top, bw, bc);
    //rect.bottom -= MenuInfo.nTitlePrAdjust; // mojmir: nTitlePrAdjust unknown
    just = just & ~DT_VCENTER | DT_BOTTOM;
  }

  BBDrawTextAlt(hDC, GetDisplayString(), -1, &rect, just, pSI);
}

void MenuGrip::Mouse(HWND hwnd, UINT uMsg, DWORD wParam, DWORD lParam)
{
    //@FIXME: this is 1:1 copy of TitleItem::Mouse.. do something about it
    Menu *p = m_pMenu;

    switch(uMsg) {
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        if (m_bActive) {
            if (wParam & MK_SHIFT) {
                if (p->m_pidl_list)
                    ShowContextMenu(NULL, first_pidl(p->m_pidl_list));
            } else {
                p->HideNow();
            }
        }
        break;

    case WM_LBUTTONDBLCLK:
        p->m_bOnTop = false == p->m_bOnTop;
        p->SetZPos();
        if (wParam & MK_SHIFT) {
            p->m_bIconized = false == p->m_bIconized;
            p->Redraw(0);
        }
        break;

    case WM_LBUTTONDOWN:
        UpdateWindow(hwnd);
        p->SetPinned(true);
        SetCursor(get_moving_cursor());
        DefWindowProc(hwnd, WM_SYSCOMMAND, 0xf012, 0);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        for (;;) {
            p->SetZPos();
            if (p->m_bOnTop || window_under_mouse() == p->m_hwnd)
                break;
            // if the menu was moved over a topmost window, make it topmost also.
            p->m_bOnTop = true;
        }
        break;

    case WM_MOUSEMOVE:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        Active(1);
        break;
    }

}







