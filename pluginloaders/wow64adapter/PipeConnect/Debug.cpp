#include "Debug.h"
#include <stdio.h>
#include <stdarg.h>

void dbg_printf(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

void dbg_dump(unsigned int len, int separator, int width, unsigned char* data) {
    for(unsigned int i = 0; i < len; i+=width) {
        dbg_printf("\n    ");
        for(unsigned int j = 0; j < len && j < width; j++) {
            if(j % separator == 0)
                dbg_printf(" ");

            dbg_printf("%hhX", data[i+j]);
        }
        dbg_printf("  ");
        for(unsigned int j = 0; j < len && j < width; j++) {
            if(data[i+j] < 0x20 || data[i+j] > 0x7F)
                dbg_printf(".");
            else
                dbg_printf("%c", data[i+j]);
        }
    }
    dbg_printf("\n");
}