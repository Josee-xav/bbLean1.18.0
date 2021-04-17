#include "Debug.h"
#include <stdio.h>
#include <stdarg.h>

void dbg_printf(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}