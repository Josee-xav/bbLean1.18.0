#include "PipeClient.hpp"
#include "../Debug.h"

PipeClient::PipeClient(const char* pipename) : abort(false) {
    dbg_printf("Connecting to control pipe: %s\n", pipename);

    this->pipe = CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
    auto err = GetLastError();

    if(this->pipe == INVALID_HANDLE_VALUE) {
        this->IsConnected(false);
        dbg_printf("Connecting to pipe failed: %d\n", err);
    } else {
        this->IsConnected(true);
        dbg_printf("Successfully connected to pipe\n");
    }
}

PipeClient::~PipeClient() {
    this->abort = true;
    Sleep(600);
    CloseHandle(this->pipe);
}

bool PipeClient::IsConnected() {
    return this->isConnected;
}

void PipeClient::IsConnected(bool value) {
    this->isConnected = value;
}

void* PipeClient::ReceiveRaw(int numBytesExpected) {
    char* buf = (char*)malloc(numBytesExpected);
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

    while(!this->abort) {
        if(WaitForSingleObject(ol.hEvent, 500) == WAIT_OBJECT_0)
            break;
    }

    if(this->abort) {
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

bool PipeClient::Send(void* msg, int len) {
    OVERLAPPED ol = {0};
    ol.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if(!WriteFile(this->pipe, msg, len, nullptr, &ol)) {
        auto err = GetLastError();
        
        if(err != ERROR_IO_PENDING) {
            dbg_printf("Error sending message: 0x%04X\n", err);
            
            CloseHandle(ol.hEvent);
            return false;
        }

        while(!this->abort) {
            if(WaitForSingleObject(ol.hEvent, 500) == WAIT_OBJECT_0)
                break;
        }

        if(this->abort) {
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