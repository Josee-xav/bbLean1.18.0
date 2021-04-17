#ifndef __PIPE_SERVER_H__
#define __PIPE_SERVER_H__
#include "PipeConnect.h"

class PipeServer {
private:
    HANDLE pipe;
    OVERLAPPED *initOl;

    const char* name;
    int timeout;
    bool connected;
public:
    PipeServer(const char* name, int timeout);
    ~PipeServer();

    bool BeginConnect();
    bool EndConnect();

    template<typename MsgT>
    bool Expect(MsgT* msg, ErrMessage* err) {
        err->error = nullptr;
        auto header = this->Receive<MessageHeader>();
        
        if(header == nullptr) {
            err->error = _strdup("No messageheader received.");
            return false;
        }

        auto content = this->Receive(header->len);
        if(content == nullptr) {
            err->error = _strdup("No content received.");
            delete header;
            return false;
        }

        if(header->tag == MsgT::tag) {
            auto deserializeOk = MessageSerializer::Deserialize(header, content, msg);

            delete header;
            delete content;

            if(!deserializeOk)
                err->error = _strdup("Could not deserialize message.");

            return deserializeOk;
        } else if(header->tag == ErrMessage::tag) {
            auto deserializeOk = MessageSerializer::Deserialize(header, content, err);

            delete header;
            delete content;

            if(!deserializeOk)
                err->error = _strdup("Could not deserialize error message.");

            return false;
        }

        err->error = _strdup("Unexpected message received.");

        delete header;
        delete content;

        return false;
    }

    template<typename Ret=char>
    Ret* Receive(int expectedNumBytes = sizeof(Ret)) {
        return reinterpret_cast<Ret*>(this->ReceiveRaw(expectedNumBytes));
    }
    
    template<typename MsgT>
    void Send(MsgT msg) {
        auto serialized = MessageSerializer::Serialize(msg);

        this->Send(serialized, sizeof(MessageHeader) + serialized->len);
        delete serialized;
    }

    bool Send(void* msg, int len);

private:
    void* ReceiveRaw(int expectedNumBytes);
};
#endif