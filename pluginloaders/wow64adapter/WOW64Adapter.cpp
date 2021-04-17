#include <map>
#include <string>

#include <strsafe.h>
#include <stdlib.h>
#include <string.h>
#include <functional>

#include "BBApi.h"

#include "WOW64Adapter.h"
#include "PipeConnect/PipeConnect.h"

using namespace std;

char name[1024];

const int plugInfoBuffersize = 1024;
const int errorBuffersize = 1024;

char plugInfoBuffer[plugInfoBuffersize];
char errorBuffer[errorBuffersize];

static const int timeout = INFINITE;

static PipeServer *controlPipe;
static const char* controlPipeName = "\\\\.\\pipe\\bbWow64AdapterControlPipe";
bool abortApiPump;

static const char* hostName = "PluginHostPrototype.exe";
static HANDLE pluginHostProcess = INVALID_HANDLE_VALUE;

static map<string, HeavyFunction> ipcApi;

DWORD WINAPI ApiThread(LPVOID par);

bool LaunchHostProcess(const char* dir, const char* file, char* args, HANDLE* process) {
    char* path = (char*)malloc(strlen(dir) + 1 + strlen(file) + 1);
    sprintf(path, "%s\\%s", dir, file);
    
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startinfo;
    memset(&startinfo, 0, sizeof(STARTUPINFO));
    
    bool success = CreateProcessA(path, args, nullptr, nullptr, true, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &startinfo, &procInfo) == TRUE;

    free(path);

    if(success && process)
        *process = procInfo.hProcess;

    return success;
}

bool Init(char* workingDir) {
    //ipcApi["GetUnderExplorer"] = HeavyFunction(Wrap(GetUnderExplorer), sizeof(bool));
    //ipcApi["GetBBWnd"] = HeavyFunction(Wrap(GetBBWnd), sizeof(HWND));

    abortApiPump = false;
    sprintf(name, "Wow64 Adapter (32bit plugin loader) v0.1(Alpha)");

    controlPipe = new PipeServer(controlPipeName, timeout);

    if(!controlPipe->BeginConnect()) {
        delete controlPipe;
        return false;
    }

    //if(!LaunchHostProcess(workingDir, hostName, (char*)controlPipeName, &pluginHostProcess)) {
    //    delete controlPipe;
    //    return false;
    //}

    if(!controlPipe->EndConnect()) {
        TerminateProcess(pluginHostProcess, -1);
        delete controlPipe;
        return false;
    }

    GetApiPipeMessage getApi = { timeout };

    auto getApiMsg = MessageSerializer::Serialize(getApi);
    if(!controlPipe->Send(getApiMsg, sizeof(MessageHeader) + getApiMsg->len)) {
        delete getApiMsg;

        TerminateProcess(pluginHostProcess, -1);
        delete controlPipe;
        return false;
    }

    GetApiPipeResult apiResult;
    OkMessage ok;
    ErrMessage errMsg;

    if(!controlPipe->Expect(&apiResult, &errMsg)) {
        delete errMsg.error;

        TerminateProcess(pluginHostProcess, -1);
        delete controlPipe;
        return false;
    }

    // TODO: start thread which connects to api pipe...

    auto apiThread = CreateThread(nullptr, 0, &ApiThread, apiResult.pipeName, 0, nullptr);
    apiResult.pipeName = nullptr;
    
    if(!controlPipe->Expect(&ok, &errMsg)) {
        delete errMsg.error;

        TerminateProcess(pluginHostProcess, -1);
        delete controlPipe;
        return false;
    }

    return true;
}

void Finalize() {
    abortApiPump = true;
    
    if(pluginHostProcess != INVALID_HANDLE_VALUE)
        TerminateProcess(pluginHostProcess, 0);

    delete controlPipe;
}

const char *GetName() {
    return name;
}

const char *GetApi() {
    // TODO: concatenate apis of loaded 32bit pluginloaders in host
    return "32bit multi";
}

const char *GetPluginInfo(struct PluginList* plugin, int factId) {
    //PipeSendMessage(pipe, msgQueryPlugin, plugin, factId);

    //auto err = PipeGetMessage(pipe, msgQueryResult, timeout, nullptr, plugInfoBuffer, plugInfuBuffersize);

    //if(err == 0)
    //    return plugInfoBuffer;
    //else if(err == 1) {
    //    // TODO: handle timeout
    //}

    return nullptr;
}

int LoadPlugin(struct PluginList* plugin, HWND hSlit, char** errorMsg) {
    ApplyPluginStateMessage msg;
    ErrMessage err;

    msg.name = plugin->name != nullptr ? (char*)malloc(strlen(plugin->name) + 1) : nullptr;
    msg.path = plugin->path != nullptr ? (char*)malloc(strlen(plugin->path) + 1) : nullptr;
    
    msg.hSlit = hSlit;

    msg.isEnabled = plugin->isEnabled;
    msg.canUseSlit = plugin->canUseSlit;
    msg.useSlit = plugin->useSlit;
    msg.inSlit = plugin->inSlit;
    msg.n_instance = plugin->n_instance;
    msg.loaderInfo = plugin->loaderInfo == nullptr ? 0 : (unsigned int)plugin->loaderInfo;

    if(plugin->name != nullptr)
        strcpy(msg.name, plugin->name);

    if(plugin->path != nullptr)
        strcpy(msg.path, plugin->path);

    controlPipe->Send(msg);

    free(msg.name);
    free(msg.path);

    if(!controlPipe->Expect(&msg, &err)) {
        strncpy(errorBuffer, err.error, errorBuffersize);
        errorBuffer[errorBuffersize-1] = 0;
    
        plugin->isEnabled = false;
        plugin->loaderInfo = nullptr;
        
        return error_plugin_does_not_load;
    }

    plugin->isEnabled = msg.isEnabled;
    plugin->canUseSlit = msg.canUseSlit;
    plugin->useSlit = plugin->useSlit;
    plugin->inSlit = msg.inSlit;
    plugin->n_instance = msg.n_instance;
    plugin->loaderInfo = (void*)msg.loaderInfo;

    return 0;
}

int UnloadPlugin(struct PluginList* plugin, char** errorMsg) {
    //bool wasEnabled = plugin->isEnabled;

    //plugin->isEnabled = false;

    //PipeSendMessage(pipe, msgApplyPluginState, plugin);

    //auto err = PipeGetMessage(pipe, msgApplyPluginStateResult, timeout, errorMsg, plugin);

    //if(err == 0)
    //    return 0;

    //if(err == 1) {
    //    // TODO: handle timeout
    //}

    //plugin->isEnabled = wasEnabled;

    //return !!errorMsg ? error_plugin_message : error_plugin_crash_on_unload;
    return 0;
}

bool HandleApiRequest(CallRequestMessage* request, CallResponseMessage* resp) {
    if(ipcApi.find(string(request->function)) == ipcApi.end())
        return false;

    HeavyFunction& heavy = ipcApi[string(request->function)];
   
    resp->argc = 0;
    resp->argSize = nullptr;
    resp->argv = nullptr;

    resp->resultSize = heavy.returnSize;
    resp->result = heavy.returnSize > 0 ? (char*)malloc(heavy.returnSize) : nullptr;

    SetLastError(0);
    heavy.fn((void*)resp->result, (void**)request->argv);
    resp->lastError = GetLastError();

    return true;
}

DWORD WINAPI ApiThread(LPVOID par) {
    const char* pipeName = (const char*)par;
    auto apiPipe = new PipeClient(pipeName);
    delete pipeName;
    
    if(!apiPipe->IsConnected()) {
        delete apiPipe;
        
        return -1;
    }

    while(!abortApiPump) {
        auto header = apiPipe->Receive<MessageHeader>();
        if(header == nullptr)
            break;
        
        auto content = apiPipe->Receive(header->len);
        if(content == nullptr) {
            delete header;
            break;
        }

        CallRequestMessage callRequest;
        CallResponseMessage callResponse;
        PingMessage ping;
        PongMessage pong;
        ErrMessage err;

        switch(header->tag) {
            case CallRequestMessage::tag:
                abortApiPump |= !MessageSerializer::Deserialize(header, content, &callRequest);
                if(abortApiPump)
                    break;

                if(HandleApiRequest(&callRequest, &callResponse)) {
                    abortApiPump |= !apiPipe->Send(callResponse);

                    free(callResponse.result);
                } else {
                    err.error = "Function not implemented";
                    abortApiPump |= !apiPipe->Send(err);
                }
                break;
            case PingMessage::tag:
                abortApiPump |= !MessageSerializer::Deserialize(header, content, &ping);
                if(abortApiPump)
                    break;

                pong.challenge = ping.challenge;
                abortApiPump |= !apiPipe->Send(pong);
                break;
            default:
                err.error = "Message not understood";
                abortApiPump |= !apiPipe->Send(err);
                break;
        }

        delete header;
        delete content;
    }

    delete apiPipe;
}
