#ifndef __PluginManager_Types_H__
#define __PluginManager_Types_H__

enum plugin_errors {
    error_plugin_is_built_in       = 1,
    error_plugin_dll_not_found     ,
    error_plugin_dll_needs_module  ,
    error_plugin_does_not_load     ,
    error_plugin_missing_entry     ,
    error_plugin_fail_to_load      ,
    error_plugin_crash_on_load     ,
    error_plugin_crash_on_unload   ,
    error_plugin_message
};

struct PluginList
{
    struct PluginList *next;

    char *name;     // display name as in the menu, NULL for comments
    char *path;     // as in plugins.rc, entire line for comments

    bool isEnabled;
    bool canUseSlit;
    bool useSlit;
    bool inSlit;

    int n_instance; // if the same plugin name is used more than once

    void* loaderInfo; // place for the loader to put loader-specific data
};

struct PluginPtr {
    struct PluginPtr* next;
    struct PluginList* entry;
};

struct PluginLoaderList {
    struct PluginLoaderList* next;
	struct PluginList* parent;
    
	HMODULE module;
	const char* name;
	const char* api;
		
	struct PluginPtr* plugins;
		
	bool (*Init)(char* workingDir);
	void (*Finalize)();
		
	const char* (*GetName)();
    const char* (*GetApi)();

    const char* (*GetPluginInfo)(struct PluginList* plugin, int factId);

	int (*LoadPlugin)(struct PluginList* plugin, HWND hSlit, char** errorMsg);
	int (*UnloadPlugin)(struct PluginList* plugin, char** errorMsg);
};

static const char* const pluginLoaderFunctionNames[] = {
    "Init",
    "Finalize",
    "GetName",
    "GetApi",
    "LoadPlugin",
    "UnloadPlugin",
    NULL
};

#define LoadFunction(pll, fn) (*(FARPROC*)&pll->fn = (pll->fn) ? ((FARPROC)pll->fn) : (GetProcAddress(pll->module, #fn)))
#define FailWithMsg(msgVar, msg) { if(msgVar) \
                                     *msgVar = msg; \
                                   return error_plugin_message; \
                                 }
#define BreakWithCode(errVar, code) { errVar = code; break; }

#endif