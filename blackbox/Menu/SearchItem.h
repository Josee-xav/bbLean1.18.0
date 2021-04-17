#pragma once
# ifndef WINVER
#  define WINVER 0x0500
#  define _WIN32_WINNT 0x0501
#  define _WIN32_IE 0x0501
# endif
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <stdlib.h>
# include <stdio.h>
# ifndef BBLIB_STATIC
#  define BBLIB_EXPORT __declspec(dllexport)
# endif
#include <windef.h>
#include <bblib.h>
#include "Menu.h"
#include <vector>
#include <string>
#include <Search/lookup.h>
#include "ResultItemAction.h"

class SearchItem : public StringItem
{
  tstring m_bbPath;
  HWND m_hText;
  WNDPROC m_wpEditProc;
  RECT m_textrect;

public:
  SearchItem (const char * pszCommand, const char * init_string);
  ~SearchItem ();

  void Paint (HDC hDC);
  void Measure (HDC hDC, SIZE *size, StyleItem * pSI);
  void Invoke (int button);

  static LRESULT CALLBACK EditProc (HWND hText, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
  void OnInput ();
};

struct ResultItem : CommandItem
{
    tstring m_typed;
    tstring m_fname;
    tstring m_fpath;

    ResultItem (const char* pszCommand, const char* pszTitle, bool bChecked);
    ~ResultItem ();
    void Mouse (HWND hwnd, UINT uMsg, DWORD wParam, DWORD lParam);
    void Invoke (int button);
};

struct ResultItemContext : CommandItem
{
    tstring m_typed;
    tstring m_fname;
    tstring m_fpath;
    E_ResultItemAction m_action;

    ResultItemContext (const char* pszCommand, const char* pszTitle);
    ~ResultItemContext ();
    void Mouse (HWND hwnd, UINT uMsg, DWORD wParam, DWORD lParam);
    void Invoke (int button);
};

class ArgItem : public CommandItem
{
  tstring m_bbPath;
  HWND m_combo;
  WNDPROC m_comboProc;
  RECT m_comboRect;
  tstring m_typed;
  tstring m_fname;
  tstring m_fpath;
  tstring m_arg;
 
public:
  ArgItem (const char * pszCommand, const char * init_string);
  ~ArgItem ();

  virtual void Paint (HDC hDC);
  virtual void Measure (HDC hDC, SIZE *size, StyleItem * pSI);
  virtual void Invoke (int button);

  static LRESULT CALLBACK ComboProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
  void OnInput ();
};


