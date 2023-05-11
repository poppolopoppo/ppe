// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IOWrapper.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE      /*hInstance*/,
    _In_opt_ HINSTANCE  /*hPrevInstance*/,
    _In_ LPWSTR         /*lpCmdLine*/,
    _In_ int            /*nCmdShow*/) {
    FIOWrapper ioWrapper;
    return ioWrapper.main(__argc, __wargv);
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BOOL FIOWrapper::CLIENT::LogMessageV(PCCH pszMsg, ...) {
    DWORD cbWritten = 0;
    CHAR szBuf[1024];
    PCHAR pcchEnd = szBuf + ARRAYSIZE(szBuf) - 2;
    PCHAR pcchCur = szBuf;
    HRESULT hr;

    va_list args;
    va_start(args, pszMsg);
    hr = StringCchVPrintfExA(pcchCur, pcchEnd - pcchCur,
                             &pcchCur, NULL, STRSAFE_NULL_ON_FAILURE,
                             pszMsg, args);
    va_end(args);
    if (FAILED(hr)) {
        goto cleanup;
    }

    hr = StringCchPrintfExA(pcchCur, szBuf + (ARRAYSIZE(szBuf)) - pcchCur,
                            &pcchCur, NULL, STRSAFE_NULL_ON_FAILURE,
                            "\n");

  cleanup:
    WriteFile(hFile, szBuf, (DWORD)(pcchCur - szBuf), &cbWritten, NULL);
    return TRUE;
}
//----------------------------------------------------------------------------
BOOL FIOWrapper::CLIENT::LogMessage(FIODetouringMessage* pMessage, DWORD nBytes) {
    // Sanity check the size of the message.
    //
    if (nBytes > pMessage->nBytes) {
        nBytes = pMessage->nBytes;
    }
    if (nBytes >= sizeof(*pMessage)) {
        nBytes = sizeof(*pMessage) - 1;
    }

    // Don't log message if there isn't and message text.
    //
    DWORD cbWrite = nBytes - offsetof(FIODetouringMessage, szMessage);
    if (cbWrite <= 0 ) {
        return TRUE;
    }

    if (bVerbose) {
        printf("IOWrapper: `%s`", pMessage->szMessage);
    }

    DWORD cbWritten = 0;
    WriteFile(hFile, pMessage->szMessage, cbWrite, &cbWritten, NULL);
    return TRUE;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VOID FIOWrapper::OnExit(PCSTR pszMsg) {
    DWORD error = GetLastError();

    fprintf(stderr, "IOWrapper: Error %ld in %s.\n", error, pszMsg);
    fflush(stderr);
    exit(1);
}
//----------------------------------------------------------------------------
BOOL FIOWrapper::CloseConnection(PCLIENT pClient) {
    InterlockedDecrement(&_nActiveClients);
    if (pClient != NULL) {
        if (pClient->hPipe != INVALID_HANDLE_VALUE) {
            //FlushFileBuffers(pClient->hPipe);
            if (!DisconnectNamedPipe(pClient->hPipe)) {
                DWORD error = GetLastError();
                fprintf(stderr, "IOWrapper: error %d in DisconnectNamedPipe\n", error);
            }
            CloseHandle(pClient->hPipe);
            pClient->hPipe = INVALID_HANDLE_VALUE;
        }
        // if (pClient->hFile != INVALID_HANDLE_VALUE) {
        //     CloseHandle(pClient->hFile);
        //     pClient->hFile = INVALID_HANDLE_VALUE;
        // }
        pClient->hFile = INVALID_HANDLE_VALUE;
        GlobalFree(pClient);
        pClient = NULL;
    }
    return TRUE;
}
//----------------------------------------------------------------------------
// Creates a pipe instance and initiate an accept request.
//----------------------------------------------------------------------------
auto FIOWrapper::CreatePipeConnection(LONG nClient) -> PCLIENT {
    HANDLE hPipe = CreateNamedPipeA(_szPipe,                   // pipe name
                                    PIPE_ACCESS_INBOUND |       // read-only access
                                    FILE_FLAG_OVERLAPPED,       // overlapped mode
                                    PIPE_TYPE_MESSAGE |         // message-type pipe
                                    PIPE_READMODE_MESSAGE |     // message read mode
                                    PIPE_WAIT,                  // blocking mode
                                    PIPE_UNLIMITED_INSTANCES,   // unlimited instances
                                    0,                          // output buffer size
                                    0,                          // input buffer size
                                    20000,                      // client time-out
                                    NULL);                      // no security attributes
    if (hPipe == INVALID_HANDLE_VALUE) {
        OnExit("CreateNamedPipe");
    }

    VerbosePrintf("IOWrapper: create named pipe %s for client #%d\n", _szPipe, nClient);

    // Allocate the client data structure.
    //
    PCLIENT pClient = (PCLIENT) GlobalAlloc(GPTR, sizeof(CLIENT));
    if (pClient == NULL) {
        OnExit("GlobalAlloc pClient");
    }

    ZeroMemory(pClient, sizeof(*pClient));
    pClient->hPipe = hPipe;
    pClient->nClient = nClient;
    pClient->bAwaitingAccept = TRUE;
    pClient->bVerbose = _bVerbose;
    pClient->hFile = _hLogFile;

    VerbosePrintf("IOWrapper: create client #%d\n", nClient);

    // Associate file with our complietion port.
    //
    if (!CreateIoCompletionPort(pClient->hPipe, _hCompletionPort, (ULONG_PTR)pClient, 0)) {
        OnExit("CreateIoComplietionPort pClient");
    }

    if (!ConnectNamedPipe(hPipe, pClient)) {
        DWORD error = GetLastError();

        if (error == ERROR_IO_PENDING) {
            VerbosePrintf("IOWrapper: pending client #%d\n", nClient);
            return NULL;
        }
        if (error == ERROR_PIPE_CONNECTED) {
            pClient->bAwaitingAccept = FALSE;
            VerbosePrintf("IOWrapper: client #%d awaiting accept\n", nClient);
        }
        else if (error != ERROR_IO_PENDING &&
                 error != ERROR_PIPE_LISTENING) {

            OnExit("ConnectNamedPipe");
        }
    }
    else {
        VerbosePrintf("IOWrapper: ConnectNamedPipe accepted immediately.\n");
        pClient->bAwaitingAccept = FALSE;
    }
    return pClient;
}
//----------------------------------------------------------------------------
BOOL FIOWrapper::DoRead(PCLIENT pClient) {
    SetLastError(NO_ERROR);
    DWORD nBytes = 0;
    BOOL b = ReadFile(pClient->hPipe, &pClient->Message, sizeof(pClient->Message),
                      &nBytes, pClient);

    DWORD error = GetLastError();

    if (b && error == NO_ERROR) {
        return TRUE;
    }
    if (error == ERROR_BROKEN_PIPE) {
        fprintf(stderr, "IOWrapper: ERROR_BROKEN_PIPE [%d]\n", nBytes);
        CloseConnection(pClient);
        return TRUE;
    }
    else if (error == ERROR_INVALID_HANDLE) {
        // ?
        fprintf(stderr, "IOWrapper: ERROR_INVALID_HANDLE\n");
        // I have no idea why this happens.  Our remedy is to drop the connection.
        return TRUE;
    }
    else if (error != ERROR_IO_PENDING) {
        if (b)
            fprintf(stderr, "IOWrapper: ReadFile succeeded %d\n", error);
        else
            fprintf(stderr, "IOWrapper: ReadFile failed %d\n", error);
        CloseConnection(pClient);
    }
    return TRUE;
}
//----------------------------------------------------------------------------
DWORD WINAPI FIOWrapper::WorkerThread(LPVOID pvVoid) {
    PCLIENT pClient;
    BOOL b;
    LPOVERLAPPED lpo;
    DWORD nBytes;
    FIOWrapper& ioWrapper = *(FIOWrapper*)pvVoid;

    for (BOOL fKeepLooping = TRUE; fKeepLooping; fKeepLooping = (ioWrapper._nActiveClients > 0)) {
        pClient = NULL;
        lpo = NULL;
        nBytes = 0;
        b = GetQueuedCompletionStatus(ioWrapper._hCompletionPort,
                                      &nBytes, (PULONG_PTR)&pClient, &lpo, INFINITE);

        if (!b) {
            if (pClient) {
                if (GetLastError() == ERROR_BROKEN_PIPE)
                    ioWrapper.VerbosePrintf("IOWrapper: client closed pipe\n");
                else
                    fprintf(stderr, "IOWrapper: GetQueuedCompletionStatus failed %d\n", GetLastError());
                ioWrapper.CloseConnection(pClient);
            }
            continue;
        }

        if (pClient->bAwaitingAccept) {
            BOOL fAgain = TRUE;
            while (fAgain) {
                LONG nClient = InterlockedIncrement(&ioWrapper._nTotalClients);
                InterlockedIncrement(&ioWrapper._nActiveClients);
                pClient->bAwaitingAccept = FALSE;

                PCLIENT pNew = ioWrapper.CreatePipeConnection(nClient);

                fAgain = FALSE;
                if (pNew != NULL) {
                    ioWrapper.VerbosePrintf("IOWrapper: new client #%d for pipe connection\n", pNew->nClient);
                    fAgain = !pNew->bAwaitingAccept;
                    ioWrapper.DoRead(pNew);
                }
            }
        }
        else {
            ioWrapper.VerbosePrintf("IOWrapper: read %d bytes from client #%d with '%s' message\n", nBytes, pClient->nClient, pClient->Message.szMessage);
            if (nBytes <= offsetof(FIODetouringMessage, szMessage)) {
                ioWrapper.VerbosePrintf("IOWrapper: close client #%d\n", pClient->nClient);
                ioWrapper.CloseConnection(pClient);
                continue;
            }
            pClient->LogMessage(&pClient->Message, nBytes);
        }

        ioWrapper.DoRead(pClient);
    }
    return 0;
}
//----------------------------------------------------------------------------
HANDLE FIOWrapper::CreateLogFile() {
    HANDLE const hFile = CreateFileW(_szLogFile,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL |
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "IOWrapper: error opening output file: %ls: %ld\n\n", _szLogFile, GetLastError());
        fflush(stderr);
        OnExit("CreateFile");
    }

    return hFile;
}
//----------------------------------------------------------------------------
HANDLE FIOWrapper::CreateWorkerThread() {
    SYSTEM_INFO SystemInfo;

    GetSystemInfo(&SystemInfo);

    DWORD dwThread = 0;
    const HANDLE hThread = CreateThread(NULL, 0, FIOWrapper::WorkerThread, this, 0, &dwThread);
    if (!hThread) {
        OnExit("CreateThread WorkerThread");
        // Unreachable: return FALSE;
    }
    // CloseHandle(hThread);
    return hThread;
}
//----------------------------------------------------------------------------
void FIOWrapper::VerbosePrintf(PCSTR pszMsg, ...) const {
    if (_bVerbose) {
        va_list args;
        va_start(args, pszMsg);
        vfprintf_s(stderr, pszMsg, args);
        va_end(args);
    }
}
//----------------------------------------------------------------------------
static DWORD CopyEnvironment(PWCHAR pwzzOut, PCWSTR pwzzIn) {
    PCWSTR pwzzBeg = pwzzOut;
    while (*pwzzIn) {
        while (*pwzzIn) {
            *pwzzOut++ = *pwzzIn++;
        }
        *pwzzOut++ = *pwzzIn++;   // Copy zero.
    }
    *pwzzOut++ = '\0';    // Add last zero.

    return (DWORD)(pwzzOut - pwzzBeg);
}
//----------------------------------------------------------------------------
DWORD FIOWrapper::main(int argc, TCHAR **argv) {
    BOOL fNeedHelp = FALSE;
    WCHAR wzzDrop[1024] = L"build\0nmake\0";

    TCHAR szCurrentDirectory[2048];
    szCurrentDirectory[0] = L'\0';
    GetCurrentDirectoryW(ARRAYSIZE(szCurrentDirectory), szCurrentDirectory);


    GetSystemTimeAsFileTime((FILETIME *)&_llStartTime);
    StringCchPrintfA(_szPipe, ARRAYSIZE(_szPipe), "%s.%d", TBLOG_PIPE_NAMEA, GetCurrentProcessId());

    StringCchCopyW(_szLogFile, ARRAYSIZE(_szLogFile), L"IOWrapper.log");

    int arg = 1;
    for (; arg < argc && (argv[arg][0] == '-' || argv[arg][0] == '/'); arg++) {
        TCHAR *argn = argv[arg] + 1;
        TCHAR *argp = argn;
        while (*argp && *argp != ':' && *argp != '=') {
            argp++;
        }
        if (*argp == ':' || *argp == '=') {
            *argp++ = '\0';
        }

        switch (argn[0]) {
        case 'd':                                     // Drop Processes
        case 'D':
            if (*argp) {
                PWCHAR pwz = wzzDrop;
                while (*argp) {
                    if (*argp == ';') {
                        *pwz++ = '\0';
                    }
                    else {
                        *pwz++ = *argp++;
                    }
                }
                *pwz++ = '\0';
                *pwz = '\0';
            }
        case 'o':                                 // Output file.
        case 'O':
            StringCchCopyW(_szLogFile, ARRAYSIZE(_szLogFile), argp);
            break;

        case 'w':                                 // Working directory
        case 'W':
            StringCchCopyW(szCurrentDirectory, ARRAYSIZE(szCurrentDirectory), argp);
            break;

        case 'v':                                 // Verbose
        case 'V':
            _bVerbose = TRUE;
            break;

        case '?':                                 // Help.
            fNeedHelp = TRUE;
            break;

        default:
            fNeedHelp = TRUE;
            fprintf(stderr, "IOWrapper: Bad argument: %ls:%ls\n", argn, argp);
            break;
        }
    }

    if (arg >= argc) {
        fNeedHelp = TRUE;
    }

    if (fNeedHelp) {
        printf("Usage:\n"
               "    iowrapper [options] command {command arguments}\n"
               "Options:\n"
               "    /o:file    Log all events to the output files.\n"
               "    /?         Display this help message.\n"
               "Summary:\n"
               "    Runs the build commands and figures out which files have dependencies..\n"
               "\n");
        exit(9001);
    }

    // Create the log file for output
    _hLogFile = CreateLogFile();
    if (_hCompletionPort == NULL) {
        OnExit("CreateLogFile");
    }

    // Create the completion port.
    _hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    if (_hCompletionPort == NULL) {
        OnExit("CreateIoCompletionPort");
    }

    // Create completion port worker threads.
    //
    _hWorkerThread = CreateWorkerThread();
    CreatePipeConnection(0);

    VerbosePrintf("IOWrapper: Ready for clients.  Press Ctrl-C to stop.\n");

    /////////////////////////////////////////////////////////// Validate DLLs.
    //
    TCHAR szTmpPath[MAX_PATH];
    TCHAR szExePath[MAX_PATH];
    TCHAR szDllPath[MAX_PATH];
    PTCHAR pszFilePart = NULL;

    if (!GetModuleFileName(NULL, szTmpPath, ARRAYSIZE(szTmpPath))) {
        fprintf(stderr, "IOWrapper: couldn't retrieve executable name.\n");
        return 9002;
    }
    if (!GetFullPathName(szTmpPath, ARRAYSIZE(szExePath), szExePath, &pszFilePart) ||
        pszFilePart == NULL) {
        fprintf(stderr, "IOWrapper: %ls is not a valid path name..\n", szTmpPath);
        return 9002;
    }

    StringCchCopy(pszFilePart, szExePath + ARRAYSIZE(szExePath) - pszFilePart,
             TEXT("Runtime-IODetouring-" STRINGIZE(BUILD_FAMILY) ".dll"));
    StringCchCopy(szDllPath, ARRAYSIZE(szDllPath), szExePath);

    //////////////////////////////////////////////////////////////////////////
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR szCommand[2048];
    TCHAR szExe[MAX_PATH];
    TCHAR szFullExe[MAX_PATH] = TEXT("\0");
    PTCHAR pszFileExe = NULL;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    szCommand[0] = L'\0';

    StringCchCopy(szExe, sizeof(szExe), argv[arg]);
    for (; arg < argc; arg++) {
        if (wcschr(argv[arg], L' ') != NULL || wcschr(argv[arg], L'\t') != NULL) {
            StringCchCat(szCommand, sizeof(szCommand), TEXT("\""));
            StringCchCat(szCommand, sizeof(szCommand), argv[arg]);
            StringCchCat(szCommand, sizeof(szCommand), TEXT("\""));
        }
        else {
            StringCchCat(szCommand, sizeof(szCommand), argv[arg]);
        }

        if (arg + 1 < argc) {
            StringCchCat(szCommand, sizeof(szCommand), TEXT(" "));
        }
    }
    VerbosePrintf("IOWrapper: Starting: `%ls'\n", szCommand);
    VerbosePrintf("IOWrapper:   with `%ls'\n", szDllPath);
    fflush(stdout);

    SetLastError(0);
    SearchPath(NULL, szExe, TEXT(".exe"), ARRAYSIZE(szFullExe), szFullExe, &pszFileExe);

    CHAR szDllPathA[2048];
    size_t count = 0;
    if (const errno_t converr = wcstombs_s(&count, szDllPathA, szDllPath, ARRAYSIZE(szDllPathA))) {
        fprintf(stderr, "IOWrapper: wcstombs_s failed: %ld\n", converr);
        ExitProcess(9006);
    }

    const DWORD dwProcessFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;
    if (!DetourCreateProcessWithDllEx( szFullExe[0] ? szFullExe : NULL, szCommand,
                                       NULL, NULL, TRUE, dwProcessFlags,
                                       NULL, szCurrentDirectory,
                                       &si, &pi, szDllPathA, NULL)) {
        fprintf(stderr, "IOWrapper: DetourCreateProcessWithDllEx failed: %ld\n", GetLastError());
        ExitProcess(9007);
    }

    ZeroMemory(&_payload, sizeof(_payload));
    _payload.nParentProcessId = GetCurrentProcessId();
    _payload.nTraceProcessId = GetCurrentProcessId();
    _payload.nGeneology = 1;
    _payload.rGeneology[0] = 0;
    StringCchCopyW(_payload.wzStdin, ARRAYSIZE(_payload.wzStdin), L"\\\\.\\CONIN$");
    StringCchCopyW(_payload.wzStdout, ARRAYSIZE(_payload.wzStdout), L"\\\\.\\CONOUT$");
    StringCchCopyW(_payload.wzStderr, ARRAYSIZE(_payload.wzStderr), L"\\\\.\\CONOUT$");
    StringCchCopyW(_payload.wzParents, ARRAYSIZE(_payload.wzParents), L"");
    CopyEnvironment(_payload.wzzDrop, wzzDrop);
    LPWCH pwStrings = GetEnvironmentStringsW();
    CopyEnvironment(_payload.wzzEnvironment, pwStrings);
    FreeEnvironmentStringsW(pwStrings);

    if (!DetourCopyPayloadToProcess(pi.hProcess, GIODetouringGuid,
                                    &_payload, sizeof(_payload))) {
        fprintf(stderr, "IOWrapper: DetourCopyPayloadToProcess failed: %ld\n", GetLastError());
        ExitProcess(9008);
    }

    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD dwResult = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwResult)) {
        fprintf(stderr, "IOWrapper: GetExitCodeProcess failed: %ld\n", GetLastError());
        return 9008;
    }

    WaitForSingleObject(_hWorkerThread, INFINITE);
    CloseHandle(_hWorkerThread);
    CloseHandle(_hLogFile);

    VerbosePrintf("IOWrapper: %ld processes, exit code = %08X.\n", _nTotalClients, dwResult);

    return dwResult;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
