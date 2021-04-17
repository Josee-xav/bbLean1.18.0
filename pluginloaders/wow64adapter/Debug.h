#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef _DEBUG
void dbg_printf(const char* fmt, ...);
#else
#define dbg_printf(...)
#endif

#endif