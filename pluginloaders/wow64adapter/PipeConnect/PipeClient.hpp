#ifndef __PIPE_CLIENT_H__
#define __PIPE_CLIENT_H__
#include <functional>
#include "PipeConnect.h"

class PipeClient {
private:
    HANDLE pipe;
    bool isConnected;
    bool abort;

public:
    PipeClient(const char* pipename);
    ~PipeClient();

    bool IsConnected();

    template<typename Ret=char>
    Ret* Receive(int expectedNumBytes = sizeof(Ret)) {
        return reinterpret_cast<Ret*>(this->ReceiveRaw(expectedNumBytes));
    }

    template<typename MsgT>
    bool Send(MsgT msg) {
        bool ret;
        
        auto serialized = MessageSerializer::Serialize(msg);
        ret = this->Send(serialized, sizeof(MessageHeader) + serialized->len);
        delete serialized;
        
        return ret;
    }

    bool Send(void* msg, int len);
private:
    void IsConnected(bool value);

    void* ReceiveRaw(int expectedNumBytes);
};

#endif