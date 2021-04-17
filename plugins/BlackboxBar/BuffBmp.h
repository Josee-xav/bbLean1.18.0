/*
 ============================================================================
  This file is part of the bbLeanBar+ source code.

  bbLeanBar+ is a plugin for BlackBox for Windows
  Copyright © 2003-2009 grischka
  Copyright © 2008-2009 The Blackbox for Windows Development Team

  http://bb4win.sourceforge.net/bblean/

  bbLeanBar+ is free software, released under the GNU General Public License
  (GPL version 2). See for details:

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

 ============================================================================
*/
#pragma once
#include <Windows.h>
//#include <DrawText.h>
#include <bblib.h>
#include <BBApi.h>
#include "BlackboxBar.h"
// Auto-buffered wrapper for MakeStyleGradient

// call instead:
// void BuffBmp::MakeStyleGradient(HDC hdc, RECT *rc, StyleItem *pSI, bool bordered);

// call after everything is drawn, and once at end of program to cleanup
// void BuffBmp::ClearBitmaps(void);

class BuffBmp {
	struct Bmp {
		struct Bmp* next;

		/* 0.0.80 */
		int bevelstyle;
		int bevelposition;
		int type;
		bool parentRelative;
		bool interlaced;

		/* 0.0.90 */
		COLORREF Color;
		COLORREF ColorTo;

		int borderWidth;
		COLORREF borderColor;
		COLORREF ColorSplitTo;
		COLORREF ColorToSplitTo;

		RECT r;
		HBITMAP bmp;
		bool in_use;
	};

	struct Bmp* g_Buffers;

public:
	BuffBmp();
	~BuffBmp();

	void MakeStyleGradient(HDC hdc, RECT* rc, StyleItem* pSI, bool withBorder);
	void ClearBitmaps(bool force = false);
};

