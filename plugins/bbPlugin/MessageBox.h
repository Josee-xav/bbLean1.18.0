// MessageBox.h: Wrapper class for MessageBoxIndirect and MSGBOXPARAMS
//
// (C) 2000, Peter Kenyon <wacs@pcnet.co.nz>
//
// Features: 1) Easy way to create message boxes with custom icons
//           2) Allows you to use string resources in a message box
//			 3) MFC-like : Simply construct a CMessageBox object and call
//				 DoModal().
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEBOX_H__A2893FB7_C936_11D3_B0FA_0040054C5E60__INCLUDED_)
#define AFX_MESSAGEBOX_H__A2893FB7_C936_11D3_B0FA_0040054C5E60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <tchar.h>

class CMessageBox : public MSGBOXPARAMS  
{
public:
	CMessageBox();
	CMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType = MB_OK, HINSTANCE hInst = GetModuleHandle(NULL));
	CMessageBox(HWND hWnd, UINT uText, UINT uCaption, UINT uType = MB_OK, HINSTANCE hInst = GetModuleHandle(NULL));

	void SetIcon(LPCTSTR lpIcon, HINSTANCE hInstance = GetModuleHandle(NULL));
	void SetIcon(UINT uIcon, HINSTANCE hInstance = GetModuleHandle(NULL));
	void SetLangID(DWORD dwLang) { dwLanguageId = dwLang; } ;
	void SetHelpContext(DWORD dwHC) { dwContextHelpId = dwHC; } ;
	void SetStyle(DWORD dwMBStyle) { dwStyle = dwMBStyle; } ;
	void SetHelpCallback(MSGBOXCALLBACK mbCallback) { lpfnMsgBoxCallback = mbCallback; } ;
	void SetText(LPCTSTR pszText) { lpszText = pszText; }
	void SetText(UINT uText) { lpszText = MAKEINTRESOURCE(uText); }
	void SetCaption(LPCTSTR pszCaption) { lpszCaption = pszCaption; }
	void SetCaption(UINT uCaption) { lpszCaption = MAKEINTRESOURCE(uCaption); }

	int DoModal() {	return ::MessageBoxIndirect(this); } ;


};

#endif // !defined(AFX_MESSAGEBOX_H__A2893FB7_C936_11D3_B0FA_0040054C5E60__INCLUDED_)
