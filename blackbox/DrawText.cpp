#include "DrawText.h"
#include "Settings.h"

//===========================================================================
int DrawTextUTF8(HDC hDC, const char *s, int nCount, RECT *p, unsigned format)
{
    WCHAR wstr[1024];
    //SIZE size;
    //int x, y, n, r;

	int n = MultiByteToWideChar(CP_UTF8, 0, s, nCount, wstr, sizeof(wstr) / sizeof(*wstr));
    if (n) --n;

	return DrawTextW(hDC, wstr, n, p, format);

	//@NOTE mojmir: everybody is using NT these days
    //if (!g_usingNT)

    /*GetTextExtentPoint32W(hDC, wstr, n, &size);
    if (format & DT_CALCRECT) {
        p->right = p->left + size.cx;
        p->bottom = p->top + size.cy;
        return 1;
    }

    if (format & DT_RIGHT)
        x = imax(p->left, p->right - size.cx);
    else if (format & DT_CENTER)
        x = imax(p->left, (p->left + p->right - size.cx) / 2);
    else
        x = p->left;

    if (format & DT_BOTTOM)
        y = imax(p->top, p->bottom - size.cy);
    else if (format & DT_VCENTER)
        y = imax(p->top, (p->top + p->bottom - size.cy) / 2);
    else
        y = p->top;

    //SetTextAlign(hDC, TA_LEFT | TA_TOP);
    r = ExtTextOutW(hDC, x, y, ETO_CLIPPED, p, wstr, n, NULL);
    return r;*/
}

//===========================================================================
// API: bbDrawText
void bbDrawText (HDC hDC, const char *text, RECT *p_rect, unsigned format, COLORREF c)
{
    if (0 == (format & DT_CALCRECT))
        SetTextColor(hDC, c);

    if (Settings_UTF8Encoding)
        DrawTextUTF8(hDC, text, -1, p_rect, format);
    else
        DrawText(hDC, text, -1, p_rect, format);
}
void bbDrawText (HDC hDC, const char *text, RECT *p_rect, unsigned format)
{
	if (Settings_UTF8Encoding)
		DrawTextUTF8(hDC, text, -1, p_rect, format);
	else
		DrawText(hDC, text, -1, p_rect, format);
}

/*int BBDrawText (HDC hDC, const char *lpString, int nCount, LPRECT lpRect, UINT uFormat, StyleItem* pSI)
{
    bool bShadow = (pSI->validated & V_SHADOWCOLOR) && (pSI->ShadowColor != (CLR_INVALID));
    bool bOutline = (pSI->validated & V_OUTLINECOLOR) && (pSI->OutlineColor != (CLR_INVALID));

    if (bShadow)
    {
        // draw shadow
        RECT rcShadow;
        rcShadow.top = lpRect->top + pSI->ShadowY;
        rcShadow.bottom = lpRect->bottom + pSI->ShadowY;
        rcShadow.left = lpRect->left + pSI->ShadowX;
        rcShadow.right = lpRect->right + pSI->ShadowX;

        //SetTextColor(hDC, pSI->ShadowColor);
        bbDrawText(hDC, lpString, &rcShadow, uFormat, pSI->ShadowColor);
    }

    if (bOutline)
    {
        //Draw the outline
        RECT rcOutline;
        _CopyOffsetRect(&rcOutline, lpRect, 1, 0);
        //SetTextColor(hDC, pSI->OutlineColor);

        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
        _OffsetRect(&rcOutline,   0,  1);
        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
        _OffsetRect(&rcOutline,  -1,  0);
        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
        _OffsetRect(&rcOutline,  -1,  0);
        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
        _OffsetRect(&rcOutline,   0, -1);
        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
        _OffsetRect(&rcOutline,   0, -1);
        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
        _OffsetRect(&rcOutline,   1,  0);
        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
        _OffsetRect(&rcOutline,   1,  0);
        bbDrawText(hDC, lpString, &rcOutline, uFormat, pSI->OutlineColor);
    }
    // draw text
    //SetTextColor(hDC, pSI->TextColor);
    bbDrawText(hDC, lpString, lpRect, uFormat, pSI->TextColor);
    return 1; //FIXME: Supposed to be DrawText(); - Should probably not call into bbDrawText to do the dirty work
}*/

int BBDrawTextAltW (HDC hDC, LPCWSTR lpString, int nCount, RECT *lpRect, unsigned uFormat, StyleItem* si)
{
	bool const bShadow = (si->validated & V_SHADOWCOLOR) && (si->ShadowColor != (CLR_INVALID));
	if (bShadow)
	{
		// draw shadow
		int const x = si->ShadowX;
		int const y = si->ShadowY;
		SetTextColor(hDC, si->ShadowColor);
		if (si->FontShadow)
		{
			// draw shadow with outline
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					if (!((i | j) & 0x2)) continue;
					RECT rcShadow;
					_CopyOffsetRect(&rcShadow, lpRect, i, j);
					DrawTextW(hDC, lpString, -1, &rcShadow, uFormat);
				}
			}
		}
		else
		{
			RECT rcShadow;
			_CopyOffsetRect(&rcShadow, lpRect, x, y);
			DrawTextW(hDC, lpString, -1, &rcShadow, uFormat);
		}
	}

	bool const bOutline = (si->validated & V_OUTLINECOLOR) && (si->OutlineColor != (CLR_INVALID));
	if (bOutline)
	{
		// draw outline
		SetTextColor(hDC, si->OutlineColor);
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				if (!(i | j)) continue;
				RECT rcOutline;
				_CopyOffsetRect(&rcOutline, lpRect, i, j);
				DrawTextW(hDC, lpString, -1, &rcOutline, uFormat);
			}
		}
	}
	// draw text
	SetTextColor(hDC, si->TextColor);
	return DrawTextW(hDC, lpString, -1, lpRect, uFormat);
}

int BBDrawTextAltA (HDC hDC, const char * lpString, int nCount, RECT * lpRect, unsigned uFormat, StyleItem * si)
{
	bool const bShadow = (si->validated & V_SHADOWCOLOR) && (si->ShadowColor != (CLR_INVALID));
	if (bShadow)
	{
		// draw shadow
		int const x = si->ShadowX;
		int const y = si->ShadowY;
		SetTextColor(hDC, si->ShadowColor);
		if (si->FontShadow)
		{
			// draw shadow with outline
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					RECT rcShadow;
					if (!((i | j) & 0x2)) continue;
					_CopyOffsetRect(&rcShadow, lpRect, i - x, j - y);
					bbDrawText(hDC, lpString, &rcShadow, uFormat); // utf8 support
				}
			}
		}
		else
		{
			RECT rcShadow;
			_CopyOffsetRect(&rcShadow, lpRect, x, y);
			bbDrawText(hDC, lpString, &rcShadow, uFormat); // utf8 support
		}
	}

	bool const bOutline = (si->validated & V_OUTLINECOLOR) && (si->OutlineColor != (CLR_INVALID));
	if (bOutline)
	{
		// draw outline
		SetTextColor(hDC, si->OutlineColor);
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				if (!(i | j)) continue;
				RECT rcOutline;
				_CopyOffsetRect(&rcOutline, lpRect, i, j);
				bbDrawText(hDC, lpString, &rcOutline, uFormat); // utf8 support
			}
		}
	}
	// draw text
	SetTextColor(hDC, si->TextColor);
	return DrawText(hDC, lpString, -1, lpRect, uFormat);
}


