#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef _DEBUG
void dbg_printf(const char* fmt, ...);
void dbg_dump(unsigned int n, int s, int w, unsigned char* d);
#else
#define dbg_printf(...)
#define dbg_dump(...)
#endif

#endif