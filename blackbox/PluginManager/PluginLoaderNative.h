#ifndef __PluginLoaderNative_H__
#define __PluginLoaderNative_H__

#include "Types.h"

extern struct PluginLoaderList nativeLoader;

struct NativePluginInfo {
    HMODULE module;

    int (*beginPlugin)(HINSTANCE);
    int (*beginPluginEx)(HINSTANCE, HWND);
    int (*beginSlitPlugin)(HINSTANCE, HWND);
    int (*endPlugin)(HINSTANCE);
    const char* (*pluginInfo)(int);
};

static const char* const pluginFunctionNames[] = {
    "beginPlugin"       ,
    "beginPluginEx"     ,
    "beginSlitPlugin"   ,
    "endPlugin"         ,
    "pluginInfo"        ,
    NULL
};

#endif