#include "PipeServer.hpp"
#include "../Debug.h"

PipeServer::PipeServer(const char* name, int timeout) : pipe(INVALID_HANDLE_VALUE), initOl(nullptr), name(name), timeout(timeout) {}

PipeServer::~PipeServer() {
    if(this->pipe != INVALID_HANDLE_VALUE) {
        CancelIo(this->pipe);
        DisconnectNamedPipe(this->pipe);
        CloseHandle(this->pipe);
    }

    if(this->initOl != nullptr) {
        if(this->initOl->hEvent != INVALID_HANDLE_VALUE)
            CloseHandle(this->initOl->hEvent);

        delete this->initOl;
    }
}

bool PipeServer::BeginConnect() {
    const int msgBufferSize = 1023;
    this->pipe = 
        CreateNamedPipe(
                this->name, 
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 
                PIPE_TYPE_MESSAGE | PIPE_WAIT, 
                2, 
                msgBufferSize, 
                msgBufferSize, 
                this->timeout, 
                nullptr);

    if(this->pipe == INVALID_HANDLE_VALUE)
        return false;

    this->initOl = new OVERLAPPED;
    this->initOl->hEvent = CreateEvent(nullptr, true, false, nullptr);

    this->connected = ConnectNamedPipe(this->pipe, this->initOl) == TRUE;
    auto connectedError = GetLastError();

    if(!this->connected) {
        if(connectedError == ERROR_PIPE_CONNECTED) {
            this->connected = true;
        }
        else if(connectedError == ERROR_IO_PENDING);
        else {
            CancelIo(this->pipe);
            CloseHandle(this->pipe);
            this->pipe = INVALID_HANDLE_VALUE;

            CloseHandle(this->initOl->hEvent);
            delete this->initOl;
            this->initOl = nullptr;
            
            return false;
        }
    }

    if(this->connected) {
        CloseHandle(this->initOl->hEvent);
        delete this->initOl;
        this->initOl = nullptr;
    }

    return true;
}

bool PipeServer::EndConnect() {
    bool success = false;

    if(WaitForSingleObject(this->initOl->hEvent, this->timeout) == WAIT_OBJECT_0) {
        DWORD crap;
        success = GetOverlappedResult(this->pipe, this->initOl, &crap, FALSE) == TRUE;
    }
    
    CloseHandle(this->initOl->hEvent);
    delete this->initOl;
    this->initOl = nullptr;

    return success;
}

bool PipeServer::Send(void* msg, int len) {
    OVERLAPPED ol = {0};
    ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if(!WriteFile(this->pipe, msg, len, nullptr, &ol)) {
        if(GetLastError() != ERROR_IO_PENDING) {
            CloseHandle(ol.hEvent);
            return false;
        }

        if(WaitForSingleObject(ol.hEvent, timeout) != WAIT_OBJECT_0) {
            CloseHandle(ol.hEvent);
            return false;
        }

        DWORD ignored;

        if(!GetOverlappedResult(this->pipe, &ol, &ignored, TRUE)) {
            CloseHandle(ol.hEvent);
            return false;
        }
    }

    CloseHandle(ol.hEvent);
    return true;
}

void* PipeServer::ReceiveRaw(int numBytesExpected) {
    void* buf = calloc(1, numBytesExpected);
    if(numBytesExpected == 0)
        return buf;

    OVERLAPPED ol = {0};
    ol.hEvent = CreateEvent(nullptr, true, false, nullptr);
    
    DWORD bytesRead;

    if(ReadFile(this->pipe, buf, numBytesExpected, nullptr, &ol)) {
        CloseHandle(ol.hEvent);
        return buf;
    }

    auto err = GetLastError();

    if(err != ERROR_IO_PENDING) {
        dbg_printf("Error reading from pipe: %d\n", err);
        CloseHandle(ol.hEvent);
        free(buf);
        return nullptr;
    }

    if(WaitForSingleObject(ol.hEvent, this->timeout) != WAIT_OBJECT_0) {
        CloseHandle(ol.hEvent);
        free(buf);
        return nullptr;
    }

    if(!GetOverlappedResult(this->pipe, &ol, &bytesRead, TRUE)) {
        dbg_printf("Error reading from pipe: %d\n", GetLastError());

        CloseHandle(ol.hEvent);
        free(buf);
        return nullptr;
    }

    if(bytesRead != numBytesExpected) {
        dbg_printf("Error reading from. Got %d bytes, expected %d\n", bytesRead, numBytesExpected);

        CloseHandle(ol.hEvent);
        free(buf);
        return nullptr;
    }

    CloseHandle(ol.hEvent);
    return buf;
}