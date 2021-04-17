#pragma once
#if defined __BBCORE__
#	include "BBApi.h" //@NOTE: cannot include BBApi because bbLeanSkin is using this file in a piggy way
#endif
#include <wtypes.h>
struct StyleItem;

#ifdef UNICODE
#define BBDrawTextAlt  BBDrawTextAltW
#else
#define BBDrawTextAlt  BBDrawTextAltA
#endif // !UNICODE
API_EXPORT int BBDrawTextAltA (HDC hDC, const char * lpString, int nCount, RECT * lpRect, unsigned uFormat, StyleItem * si);
API_EXPORT int BBDrawTextAltW (HDC hDC, LPCWSTR lpString, int nCount, RECT * lpRect, unsigned uFormat, StyleItem * si);

inline void _CopyOffsetRect (RECT * dst, RECT const * src, int dx, int dy)
{
  dst->left = src->left + dx;
  dst->right = src->right + dx;
  dst->top = src->top + dy;
  dst->bottom = src->bottom + dy;
}
inline void _OffsetRect (RECT * lprc, int dx, int dy)
{
  lprc->left += dx;
  lprc->right += dx;
  lprc->top += dy;
  lprc->bottom += dy;
}
