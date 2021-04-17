#ifndef __MESSAGES_H__
#define __MESSAGES_H__

enum class MessageTag : char {
    Ping,
    Pong,
    
    Ok,
    Err,

    ApplyPluginState,

    QueryPlugin,
    QueryResult,

    GetApiPipe,
    GetApiPipeResult,

    CallRequest,
    CallResponse,

    Finalize,
};

typedef struct MessageHeader {
    MessageTag tag;
    unsigned int len;

    MessageHeader() {}
    MessageHeader(MessageTag tag) : tag(tag) {}
    MessageHeader(MessageTag tag, unsigned int len) : tag(tag), len(len) {}
} MessageHeader;

struct OkMessage {
    static const MessageTag tag = MessageTag::Ok;
};

struct ErrMessage {
    static const MessageTag tag = MessageTag::Err;
    
    char* error;
};

struct GetApiPipeMessage {
    static const MessageTag tag = MessageTag::GetApiPipe;

    int timeoutMillis;
};

struct GetApiPipeResult {
    static const MessageTag tag = MessageTag::GetApiPipeResult;

    char* pipeName;
};

struct PingMessage {
    static const MessageTag tag = MessageTag::Ping;

    int challenge;
};

struct PongMessage {
    static const MessageTag tag = MessageTag::Pong;

    int challenge;
};

struct CallRequestMessage {
    static const MessageTag tag = MessageTag::CallRequest;

    char* function;
    int argc;
    unsigned int* argSize;
    void** argv;
};

struct CallResponseMessage {
    static const MessageTag tag = MessageTag::CallResponse;

    int argc;
    unsigned int* argSize;
    void** argv;

    unsigned int resultSize;
    char* result;
    DWORD lastError;
};

struct ApplyPluginStateMessage {
    static const MessageTag tag = MessageTag::ApplyPluginState;

    char *name;
    char *path;

    HWND hSlit;

    bool isEnabled;
    bool canUseSlit;
    bool useSlit;
    bool inSlit;

    int n_instance;

    unsigned int loaderInfo;
};

#endif