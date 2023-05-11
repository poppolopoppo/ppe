#pragma once

#include "Runtime/Core/Public/HAL/PlatformIncludes.h"
#include "Runtime/IODetouring/Public/IODetouring.h"
#include "Runtime/IODetouring/Public/IODetouringTblog.h"

#include "detours-external.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>

class FIOWrapper {
public:
    enum {
        CLIENT_AWAITING_PIPE_ACCEPT = 0x21,
        CLIENT_AWAITING_PIPE_DATA   = 0x22,
    };

    typedef struct _CLIENT : OVERLAPPED
    {
        FIODetouringMessage Message;

        HANDLE          hPipe{ INVALID_HANDLE_VALUE };
        HANDLE          hFile{ INVALID_HANDLE_VALUE };
        PVOID           Zero{ NULL };
        LONG            nClient{ 0 };
        BOOL            bAwaitingAccept{ FALSE };
        BOOL            bVerbose{ FALSE };

        BOOL LogMessage(FIODetouringMessage* pMessage, DWORD nBytes);
        BOOL LogMessageV(PCCH pszMsg, ...);
    } CLIENT, *PCLIENT;

    void OnExit(PCSTR pszMsg);

    BOOL CloseConnection(PCLIENT pClient);
    PCLIENT CreatePipeConnection(LONG nClient);
    BOOL DoRead(PCLIENT pClient);

    DWORD main(int argc, TCHAR **argv);

private:
    HANDLE CreateLogFile();
    HANDLE CreateWorkerThread();
    static DWORD WINAPI WorkerThread(LPVOID pvVoid);

    void VerbosePrintf(PCSTR pszMsg, ...) const;

    HANDLE      _hCompletionPort{ INVALID_HANDLE_VALUE };
    HANDLE      _hWorkerThread{ INVALID_HANDLE_VALUE };
    HANDLE      _hLogFile{ INVALID_HANDLE_VALUE };
    TCHAR       _szLogFile[MAX_PATH];
    CHAR        _szPipe[MAX_PATH];
    LONG        _nActiveClients = 0;
    LONG        _nTotalClients = 0;
    LONGLONG    _llStartTime = 0;
    BOOL        _bVerbose{ FALSE };
    FIODetouringPayload _payload{};
};