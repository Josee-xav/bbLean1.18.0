//#include "stdafx.h" 
#include "MessageBox.h" 
 
////////////////////////////////////////////////////////////////////// 
// Construction/Destruction 
////////////////////////////////////////////////////////////////////// 
 
CMessageBox::CMessageBox() 
{ 
  ::ZeroMemory(static_cast<MSGBOXPARAMS*>(this), sizeof(MSGBOXPARAMS)); 
 
  cbSize = sizeof(MSGBOXPARAMS); 
  dwLanguageId = (DWORD)MAKEINTRESOURCE(LANG_NEUTRAL); 
 
} 
 
CMessageBox::CMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, HINSTANCE hInst) 
{ 
  ::ZeroMemory(static_cast<MSGBOXPARAMS*>(this), sizeof(MSGBOXPARAMS)); 
  cbSize = sizeof(MSGBOXPARAMS); 
  dwLanguageId = (DWORD)MAKEINTRESOURCE(LANG_NEUTRAL); 
  hInstance = hInst; 
  hwndOwner = hWnd; 
  lpszText = lpText; 
  lpszCaption = lpCaption; 
  dwStyle = (DWORD)uType; 
 
} 
 
CMessageBox::CMessageBox(HWND hWnd, UINT uText, UINT uCaption, UINT uType, HINSTANCE hInst) 
{ 
  ::ZeroMemory(static_cast<MSGBOXPARAMS*>(this), sizeof(MSGBOXPARAMS)); 
 
  cbSize = sizeof(MSGBOXPARAMS); 
  dwLanguageId = (DWORD)MAKEINTRESOURCE(LANG_NEUTRAL); 
  hInstance = hInst; 
  hwndOwner = hWnd; 
  lpszText = MAKEINTRESOURCE(uText); 
  lpszCaption = MAKEINTRESOURCE(uCaption); 
 
  dwStyle = (DWORD)uType; 
} 
 
void CMessageBox::SetIcon(LPCTSTR lpIcon, HINSTANCE hInst) 
{ 
  dwStyle |= MB_USERICON; 
  hInstance = hInst; 
  lpszIcon = lpIcon; 
} 
 
void CMessageBox::SetIcon(UINT uIcon, HINSTANCE hInst) 
{ 
  dwStyle |= MB_USERICON; 
  hInstance = hInst; 
  lpszIcon = MAKEINTRESOURCE(uIcon); 
} 
