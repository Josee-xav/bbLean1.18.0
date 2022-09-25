#ifndef __BBAPI_PLUGINLOADER_H__
#define __BBAPI_PLUGINLOADER_H__

/*------------------------------------------ */
/* windows include */
/*------------------------------------------ */
#ifndef WINVER
  #define WINVER 0x0500
  #define _WIN32_WINNT 0x0500
  #define _WIN32_IE 0x0501
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "PluginManager/Types.h"

#ifndef DLL_EXPORT
    #define DLL_EXPORT __declspec(dllexport)
#endif

#ifdef __BBCORE__
    #undef DLL_EXPORT
    #define DLL_EXPORT
#endif

#ifndef __cplusplus
  typedef char bool;
  #define false 0
  #define true 1
  #define class struct
#else
extern "C" {
#endif

DLL_EXPORT bool Init(char* workingDir);
DLL_EXPORT void Finalize();
DLL_EXPORT const char *GetName();
DLL_EXPORT const char *GetApi();
DLL_EXPORT const char *GetPluginInfo(struct PluginList* plugin, int factId);
DLL_EXPORT int LoadPlugin(struct PluginList* plugin, HWND hSlit, char** errorMsg);
DLL_EXPORT int UnloadPlugin(struct PluginList* plugin, char** errorMsg);

#ifdef __cplusplus
}
#endif

#endif