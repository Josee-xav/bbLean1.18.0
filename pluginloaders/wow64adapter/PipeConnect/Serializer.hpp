#ifndef __SERIALIZER__H__
#define __SERIALIZER__H__
#include <vector>
#include "Messages.hpp"
#include "Debug.h"

template<typename T>
struct TypeSerializer {
    static void Serialize(unsigned int* len, char** values, T value) {
        auto valueLen = sizeof(T);

        if(valueLen == 0)
            return;

        auto newValues = (char*) malloc(*len + valueLen);

        if(*values != nullptr) {
            memcpy(newValues, *values, *len);
            free(*values);
        }

        memcpy(newValues+(*len), &value, valueLen);

        *len += valueLen;
        *values = newValues;
    }

    static void Deserialize(char* values, unsigned int* off, T* value) {
        auto valueLen = sizeof(T);

        if(valueLen == 0)
            return;

        memcpy(value, values + (*off), valueLen);
        
        *off += valueLen;
    }
};

class Serializer {
private:
    unsigned int length;
    char* value;
    unsigned int offset;
public:
    Serializer() : length(0), value(nullptr), offset(0) {}
    Serializer(const Serializer &other) {
        if(other.length) {
            this->value = (char*)malloc(other.length);
            memcpy(this->value, other.value, other.length);
        } else {
            this->value = nullptr;
        }

        this->length = other.length;
        this->offset = other.offset;
    }
    Serializer(unsigned int len, char const* content) : length(len), offset(0) {
        this->value = (char*)malloc(len);
        memcpy(this->value, content, len);
    }

    ~Serializer() {
        free(this->value);
    }

    unsigned int Length() {
        return this->length;
    }

    char* Value() {
        char* r = new char[this->length];
        memcpy(r, this->value, this->length);
        return r;
    }

    char const* ValueRaw() {
        return this->value;
    }

    template<typename T>
    Serializer& operator<<(T value) {
        TypeSerializer<T>::Serialize(&this->length, &this->value, value);
        return *this;
    }

    template<typename T>
    Serializer& operator>>(T* value) {
        TypeSerializer<T>::Deserialize(this->value, &this->offset, value);
        return *this;
    }
};

template<>
struct TypeSerializer<Serializer> {
    static void Serialize(unsigned int* fstLen, char** fst, Serializer value) {
        auto sndLen = value.Length();

        if(sndLen == 0)
            return;

        auto snd = value.ValueRaw();

        auto merged = (char*)malloc(*fstLen + sndLen);

        if(*fst != nullptr) {
            memcpy(merged, *fst, *fstLen);
            free(*fst);
        }
        
        memcpy(merged+(*fstLen), snd, sndLen);

        *fst = merged;
        *fstLen += sndLen;
    }
};

template<>
struct TypeSerializer<char*> {
    static void Serialize(unsigned int* headLen, char** head, char* value) {
        unsigned int valueLen = strlen(value) + 1;
        auto tailLen = sizeof(unsigned int) + valueLen;

        auto merged = (char*)malloc(*headLen + tailLen);
        if(*head != nullptr) {
            memcpy(merged, *head, *headLen);
            free(*head);
        }

        *(unsigned int*)(merged + *headLen) = valueLen;
        memcpy(merged+(*headLen)+sizeof(unsigned int), value, valueLen);
        
        *head = merged;
        *headLen += tailLen;
    }

    static void Deserialize(char* values, unsigned int* off, char** value) {
        auto valueLen = *(unsigned int*)(values + *off);
        *off += sizeof(unsigned int);

        *value = (char*)malloc(valueLen);
        memcpy(*value, values + (*off), valueLen);
        *off += valueLen;
    }
};

struct Pack {
    unsigned int len;
    char* content;
};

template<>
struct TypeSerializer<Pack> {
    static void Serialize(unsigned int* headLen, char** head, Pack value) {
        unsigned int valueLen = value.len;
        auto tailLen = sizeof(unsigned int) + valueLen;

        auto merged = (char*)malloc(*headLen + tailLen);
        if(*head != nullptr) {
            memcpy(merged, *head, *headLen);
            free(*head);
        }

        *(unsigned int*)(merged + *headLen) = valueLen;
        memcpy(merged+(*headLen)+sizeof(unsigned int), value.content, valueLen);

        *head = merged;
        *headLen += tailLen;
    }

    static void Deserialize(char* values, unsigned int* off, Pack* value) {
        auto valueLen = *(unsigned int*)values;
        *off += sizeof(unsigned int);

        value->content = (char*)malloc(valueLen);
        memcpy(value->content, values + (*off), valueLen);
        *off += valueLen;
    }
};

template<typename MsgT>
struct _MessageSerializer {
    static MessageHeader* Serialize(MsgT message) {
        Serializer content;
        content << message;
        
        Serializer msg; 
        msg << MessageHeader(MsgT::tag, content.Length()) << content;

        return reinterpret_cast<MessageHeader*>(msg.Value());
    }

    static bool Deserialize(MessageHeader* header, char* content, MsgT* msgOut) {
        if(header->tag != MsgT::tag)
            return false;

        Serializer(header->len, content) >> msgOut;
        return true;
    }
};

template<>
struct _MessageSerializer<ErrMessage> {
    static MessageHeader* Serialize(ErrMessage message) {
        Serializer content;
        content << message.error;

        Serializer msg;
        msg << MessageHeader(ErrMessage::tag, content.Length()) << content;

        return reinterpret_cast<MessageHeader*>(msg.Value());
    }

    static bool Deserialize(MessageHeader* header, char* content, ErrMessage* msgOut) {
        if(header->tag != ErrMessage::tag)
            return false;

        Serializer(header->len, content) >> &msgOut->error;
        return true;
    }
};

template<>
struct _MessageSerializer<GetApiPipeResult> {
    static MessageHeader* Serialize(GetApiPipeResult message) {
        Serializer content;
        content << message.pipeName;

        Serializer msg;
        msg << MessageHeader(GetApiPipeResult::tag, content.Length()) << content;

        return reinterpret_cast<MessageHeader*>(msg.Value());
    }

    static bool Deserialize(MessageHeader* header, char* content, GetApiPipeResult* msgOut) {
        if(header->tag != GetApiPipeResult::tag)
            return false;

        Serializer(header->len, content) >> &msgOut->pipeName;
        return true;
    }
};

template<>
struct _MessageSerializer<CallRequestMessage> {
    static MessageHeader* Serialize(CallRequestMessage message) {
        Serializer content;
        content << message.function;
        content << message.argc;

        for(int i = 0; i < message.argc; i++) {
            Pack p = { message.argSize[i], (char*)message.argv[i] };
            content << p;
        }

        Serializer msg;
        msg << MessageHeader(CallRequestMessage::tag, content.Length()) << content;

        return reinterpret_cast<MessageHeader*>(msg.Value());
    }

    static bool Deserialize(MessageHeader* header, char* content, CallRequestMessage* msgOut) {
        if(header->tag != CallRequestMessage::tag)
            return false;

        Serializer& ser = Serializer(header->len, content);

        ser >> &msgOut->function;
        ser >> &msgOut->argc;

        msgOut->argSize = (unsigned int*)malloc(sizeof(unsigned int) * msgOut->argc);
        msgOut->argv = (void**)malloc(sizeof(void*) * msgOut->argc);

        for(int i = 0; i < msgOut->argc; i++) {
            Pack p;
            ser >> &p;

            msgOut->argSize[i] = p.len;
            msgOut->argv[i] = (void*)p.content;
        }

        return true;
    }
};


template<>
struct _MessageSerializer<CallResponseMessage> {
    static MessageHeader* Serialize(CallResponseMessage message) {
        Serializer content;
        content << message.resultSize;

        Pack p = { message.resultSize, message.result };
        content << p;
        
        content << message.lastError;

        Serializer msg;
        msg << MessageHeader(CallResponseMessage::tag, content.Length()) << content;

        return reinterpret_cast<MessageHeader*>(msg.Value());
    }

    static bool Deserialize(MessageHeader* header, char* content, CallResponseMessage* msgOut) {
        if(header->tag != CallResponseMessage::tag)
            return false;

        Serializer& ser = Serializer(header->len, content);

        ser >> &msgOut->resultSize;

        Pack p;
        ser >> &p;
        msgOut->result = p.content;

        ser >> &msgOut->lastError;

        return true;
    }
};

template<>
struct _MessageSerializer<ApplyPluginStateMessage> {
    static MessageHeader* Serialize(ApplyPluginStateMessage message) {
        Serializer content;
        content << message.name << message.path;
        content << (DWORD)message.hSlit;
        content << message.isEnabled << message.canUseSlit << message.useSlit << message.inSlit;
        content << message.n_instance;
        content << message.loaderInfo;

        Serializer msg;
        msg << MessageHeader(ApplyPluginStateMessage::tag, content.Length()) << content;

        return reinterpret_cast<MessageHeader*>(msg.Value());
    }

    static bool Deserialize(MessageHeader* header, char* content, ApplyPluginStateMessage* msgOut) {
        if(header->tag != ApplyPluginStateMessage::tag)
            return false;

        Serializer& ser = Serializer(header->len, content);
        ser >> &msgOut->name >> &msgOut->path;
        ser >> (DWORD*)&msgOut->hSlit;
        ser >> &msgOut->isEnabled >> &msgOut->canUseSlit >> &msgOut->useSlit >> &msgOut->inSlit;
        ser >> &msgOut->n_instance;
        ser >> &msgOut->loaderInfo;

        return true;
    }
};

struct MessageSerializer {
    template<typename MsgT>
    static MessageHeader* Serialize(MsgT message) {
        return _MessageSerializer<MsgT>::Serialize(message);
    }

    template<typename MsgT>
    static bool Deserialize(MessageHeader* header, char* content, MsgT* msgOut) {
        return _MessageSerializer<MsgT>::Deserialize(header, content, msgOut);
    }
};

#endif