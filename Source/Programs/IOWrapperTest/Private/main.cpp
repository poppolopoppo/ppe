// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Core_fwd.h"
#include "HAL/PlatformIncludes.h"
#include "Meta/Utility.h" // ON_SCOPE_EXIT()

#include "IODetouring.h"
#include "IODetouringFiles.h"
#include "IODetouringTblog.h"

#include "detours-external.h"

#include <random>
#include <strsafe.h>

#define USE_IOWRAPPER_DEBUG     0 // %_NOCOMMIT%
#define USE_IOWRAPPER_THREAD    0 // %_NOCOMMIT%

#if USE_IOWRAPPER_DEBUG
#   define IOWRAPPER_LOG(_FMT, ...) LogPrintf_("IOWrapper: " _FMT, ## __VA_ARGS__)
#else
#   define IOWRAPPER_LOG(_FMT, ...) NOOP()
#endif

#if USE_IOWRAPPER_DEBUG
PRAGMA_DISABLE_OPTIMIZATION
static void LogPrintf_(const char* pzFmt, ...) {
    char szTmp[8192];

    va_list args;
    va_start(args, pzFmt);
    StringCchVPrintfA(szTmp, ARRAYSIZE(szTmp), pzFmt, args);
    va_end(args);

    OutputDebugStringA(szTmp);
}
#endif

enum ERunProcess {
    RP_NO_ERROR = 0,                    // success
    RP_BAD_ALLOC,                       // GlobalAlloc() failed
    RP_BROKEN_PIPE,                     // pipe connection ended abnormally
    RP_COMPLETION_PORT,                 // IO completion port error
    RP_CREATE_PIPE,                     // failed to create a named pipe
    RP_CREATE_PROCESS,                  // failed to create the process
    RP_CREATE_REDIRECT,                 // failed to create pipe redirection for STDOUT/STDERR
    RP_EXIT_CODE,                       // failed to retrieve process exit code
    RP_PAYLOAD_COPY,                    // failed to copy Detour payload to child process
    RP_WORKER_THREAD,                   // failed to create IO completion port worker thread (if USE_IOWRAPPER_THREAD)
};

#if USE_IOWRAPPER_THREAD
constexpr DWORD GIOWrapperTimeoutInMs = INFINITE;
#else
constexpr DWORD GIOWrapperTimeoutInMs = INFINITE;
#endif

static ERunProcess RunProcessWithDetours(
    int* pExitCode,                     // exit code returned by process, or GetLastError() when failed
    PCSTR pszIODetouringDll,            // path to the IODetouring-WinXX-XXX.dll to inject in child process
    PCWSTR pwzNamedPipePrefix,          // prefix of the pipe for this particular process, should be unique
    PCWSTR pwzExecutableName,           // executable path, provide an absolute path to bypass %PATH% resolution
    PCWSTR pwzCommandLine,              // process arguments as a string, separated by spaces and within quotes when needed
    PCWSTR pwzzIgnoredApplications,     // child application matching those name won't be detoured, as a null-terminated array of null-terminated strings
    EIODetouringOptions nOptions,       // file matching any of those attribute won't pass through `onFilename` event
    HANDLE hDependencyFile              // handle to dependency output file
);

int _tmain(int argc, TCHAR* argv[], TCHAR* []) {
    if (argc < 4) {
        fprintf(stderr, "%ls <dependency_path> <ignored_applications> <application_path> [application_args]*\n", argv[0]);
        return -1;
    }

    const int nCommandLineFirstArg = 4;
    PCWSTR const pwzDependenciesPath = argv[1];
    PCWSTR const pwzIgnoredApplications = argv[2];
    PCWSTR const pwzExecutableName = argv[3];

    IOWRAPPER_LOG("dependency file: '%ls'\n", pwzDependenciesPath);
    IOWRAPPER_LOG("ignored applications: '%ls'\n", pwzIgnoredApplications);
    IOWRAPPER_LOG("executable name: '%ls'\n", pwzExecutableName);

    // open output handle to write file dependencies
    HANDLE const hDependencyFile = CreateFile(pwzDependenciesPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hDependencyFile == NULL || hDependencyFile == INVALID_HANDLE_VALUE)
        return GetLastError();
    ON_SCOPE_EXIT([hDependencyFile]() {
        CloseHandle(hDependencyFile);
    });

    // look for IO detouring DLL next in the same folder than running application
    CHAR szIODetouringDll[MAX_PATH] = "";
    if (!GetModuleFileNameA(NULL, szIODetouringDll, ARRAYSIZE(szIODetouringDll)))
        return GetLastError();
    if (CHAR* pzFilename = strrchr(szIODetouringDll, '\\'))
        *pzFilename = '\0';
    else
        return -2;
    StringCchCatA(szIODetouringDll, ARRAYSIZE(szIODetouringDll), "\\Tools-IODetouring-" STRINGIZE(BUILD_FAMILY) ".dll");

    IOWRAPPER_LOG("detouring dll: '%s'\n", szIODetouringDll);

    // construct command-line for detoured process
    TCHAR wzCommandLine[8192] = TEXT("");
    for (int iArg = nCommandLineFirstArg - 1; iArg < argc; ++iArg) {
        PCWSTR pwzArg = pwzExecutableName;
        if (iArg >= nCommandLineFirstArg) {
            pwzArg = argv[iArg];
            StringCchCat(wzCommandLine, ARRAYSIZE(wzCommandLine), TEXT(" "));
        }

        if (wcschr(pwzArg, TEXT(' ')) != NULL || wcschr(pwzArg, TEXT('\t')) != NULL) {
            StringCchCat(wzCommandLine, ARRAYSIZE(wzCommandLine), TEXT("\""));
            StringCchCat(wzCommandLine, ARRAYSIZE(wzCommandLine), pwzArg);
            StringCchCat(wzCommandLine, ARRAYSIZE(wzCommandLine), TEXT("\""));
        }
        else {
            StringCchCat(wzCommandLine, ARRAYSIZE(wzCommandLine), pwzArg);
        }
    }

    IOWRAPPER_LOG("command-line: '%ls'\n", wzCommandLine);

    // construct ignored applications block
    TCHAR wzzIgnoredApplications[8192] = TEXT("");
    for (int nOff = 0; nOff < ARRAYSIZE(wzzIgnoredApplications) - 1; ) {
        wzzIgnoredApplications[nOff] = (pwzIgnoredApplications[nOff] != TEXT(',') ? pwzIgnoredApplications[nOff] : TEXT('\0'));
        if (pwzIgnoredApplications[++nOff] == TEXT('\0')) {
            wzzIgnoredApplications[nOff] = TEXT('\0');
            wzzIgnoredApplications[nOff + 1] = TEXT('\0');
            break;
        }
    }

    // construct a pseudo unique identifier for named pipe
    const u32 nRandomSeed{ u32(PPE::hash_size_t_constexpr(GetTickCount64(), GetCurrentProcessId(), PPE::FConstWChar(wzCommandLine).HashValue())) };
    wchar_t wzNamedPipePrefix[17] = TEXT("");
    constexpr wchar_t wzRandomCharset[] = TEXT(
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz");
    std::generate_n(wzNamedPipePrefix, 16, [
            rnd{std::default_random_engine{nRandomSeed}},
            distrib{ std::uniform_int_distribution<>{0, ARRAYSIZE(wzRandomCharset) - 2} }]
        () mutable -> wchar_t {
        return wzRandomCharset[distrib(rnd)];
    });
    wzNamedPipePrefix[16] = TEXT('\0');


    IOWRAPPER_LOG("ignored applications: '%ls'\n", pwzIgnoredApplications);

    // ignore all _special_ file types
    EIODetouringOptions nOptions = (
        // ignore every "special" files by default
        EIODetouringOptions::IgnoreAbsorbed    |
        EIODetouringOptions::IgnoreCleanup     |
        EIODetouringOptions::IgnoreDelete      |
        EIODetouringOptions::IgnoreDirectory   |
        EIODetouringOptions::IgnoreDoNone      |
        EIODetouringOptions::IgnorePipe        |
        EIODetouringOptions::IgnoreStdio       |
        EIODetouringOptions::IgnoreSystem      );

    // launch detoured process
    int nExitCode = 0;
    const ERunProcess nError = RunProcessWithDetours(
        &nExitCode,
        szIODetouringDll,
        wzNamedPipePrefix,
        pwzExecutableName,
        wzCommandLine,
        wzzIgnoredApplications,
        nOptions,
        hDependencyFile);

    IOWRAPPER_LOG("error code = %d (%d)\n", nError, nExitCode);
    if (nError == RP_NO_ERROR)
        return nExitCode;
    return -nError;
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static DWORD CopyNullTerminatedStringList_(PWCHAR pwzzOut, PCWSTR pwzzIn) {
    PCWSTR pwzzBeg = pwzzOut;
    while (*pwzzIn) {
        while (*pwzzIn) {
            *pwzzOut++ = *pwzzIn++;
        }
        *pwzzOut++ = *pwzzIn++;   // Copy zero.
    }
    *pwzzOut++ = TEXT('\0');    // Add last zero.

    return (DWORD)(pwzzOut - pwzzBeg);
}
//----------------------------------------------------------------------------
static bool CopyDetourPayloadToProcess_(
    HANDLE hProcess,
    PCWSTR pwzNamedPipePrefix,
    PCWSTR pwzzIgnoredApplications,
    EIODetouringOptions nPayloadOptions) {
    FIODetouringPayload payload;
    ZeroMemory(&payload, sizeof(payload));

    StringCchCopyW(payload.wzNamedPipe, ARRAYSIZE(payload.wzNamedPipe), pwzNamedPipePrefix);

    payload.nPayloadOptions = nPayloadOptions;
    payload.nParentProcessId = GetCurrentProcessId();
    payload.nTraceProcessId = GetCurrentProcessId();

    payload.nGeneology = 1;
    payload.rGeneology[0] = 0;

    StringCchCopyW(payload.wzStdin, ARRAYSIZE(payload.wzStdin), L"\\\\.\\CONIN$");
    StringCchCopyW(payload.wzStdout, ARRAYSIZE(payload.wzStdout), L"\\\\.\\CONOUT$");
    StringCchCopyW(payload.wzStderr, ARRAYSIZE(payload.wzStderr), L"\\\\.\\CONOUT$");

    CopyNullTerminatedStringList_(payload.wzzIgnoredApplications, pwzzIgnoredApplications);

    return (!!DetourCopyPayloadToProcess(hProcess, GIODetouringGuid, &payload, sizeof(payload)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FIOWrapperPipeClient : OVERLAPPED {
    CHAR szBuffer[32768];
    HANDLE hPipe;
    HANDLE hOutput;
    PCWSTR pwzNamedPipe;
    int nClientId;
    bool bAwaitingAccept;
};
//----------------------------------------------------------------------------
static void ClosePipeConnection_(FIOWrapperPipeClient** ppClient, int* pActiveClients) {
    if (*ppClient == nullptr)
        return;

    IOWRAPPER_LOG("close named pipe client #%d at %p\n", (*ppClient)->nClientId, *ppClient);

    if ((*ppClient)->hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe((*ppClient)->hPipe);
        CloseHandle((*ppClient)->hPipe);
        (*ppClient)->hPipe = INVALID_HANDLE_VALUE;
    }

    GlobalFree(*ppClient);
    *ppClient = nullptr;

    --(*pActiveClients);
}
//----------------------------------------------------------------------------
static bool ReadPipeContent_(FIOWrapperPipeClient** ppClient, int* pActiveClients) {
    SetLastError(NO_ERROR);

    IOWRAPPER_LOG("read named pipe client %d at %p\n", (*ppClient)->nClientId, *ppClient);

    DWORD nBytesRead = 0;
    const BOOL bSucceed = ReadFile((*ppClient)->hPipe, &(*ppClient)->szBuffer, sizeof((*ppClient)->szBuffer), &nBytesRead, *ppClient);

    const DWORD err = GetLastError();
    if (bSucceed && err == NO_ERROR)
        return true;

    IOWRAPPER_LOG("read named pipe client %d at %p failed with error [%d] !\n", (*ppClient)->nClientId, *ppClient, err);
    if (err == ERROR_IO_PENDING || err == ERROR_PIPE_LISTENING)
        return true;

    ClosePipeConnection_(ppClient, pActiveClients);
    return false;
}
//----------------------------------------------------------------------------
static ERunProcess CreateNamedPipeConnection_(FIOWrapperPipeClient** ppClient, HANDLE hCompletionPort, PCWSTR pwzNamedPipe, HANDLE hOutput, int* pActiveClients) {
    IOWRAPPER_LOG("create named pipe '%ls'\n", pwzNamedPipe);

    HANDLE hPipe = CreateNamedPipe(
        pwzNamedPipe,                // pipe name
        PIPE_ACCESS_INBOUND |       // read-only access
        FILE_FLAG_OVERLAPPED,       // overlapped mode
        PIPE_TYPE_MESSAGE |         // message-type pipe
        PIPE_READMODE_MESSAGE |     // message read mode
        PIPE_WAIT,                  // blocking mode
        PIPE_UNLIMITED_INSTANCES,   // unlimited instances
        0,                          // output buffer size
        0,                          // input buffer size
        GIOWrapperTimeoutInMs,      // client time-out (default: 50ms)
        NULL);                      // no security attributes
    if (hPipe == INVALID_HANDLE_VALUE)
        return RP_CREATE_PIPE;

    *ppClient = (FIOWrapperPipeClient*)GlobalAlloc(GPTR, sizeof(FIOWrapperPipeClient));
    if (*ppClient == NULL) {
        CloseHandle(hPipe);
        return RP_BAD_ALLOC;
    }

    ZeroMemory(*ppClient, sizeof(FIOWrapperPipeClient));
    (*ppClient)->hPipe = hPipe;
    (*ppClient)->hOutput = hOutput;
    (*ppClient)->pwzNamedPipe = pwzNamedPipe;
    (*ppClient)->nClientId = (*pActiveClients)++;
    (*ppClient)->bAwaitingAccept = true;

    IOWRAPPER_LOG("create pipe client #%d at %p\n", (*ppClient)->nClientId, *ppClient);

    if (!CreateIoCompletionPort((*ppClient)->hPipe, hCompletionPort, (ULONG_PTR)*ppClient, 0)) {
        const int lastError = GetLastError();
        IOWRAPPER_LOG("CreateIoCompletionPort: failed with %d", GetLastError());
        ClosePipeConnection_(ppClient, pActiveClients);
        SetLastError(lastError);
        return RP_COMPLETION_PORT;
    }

    if (ConnectNamedPipe(hPipe, *ppClient)) {
        (*ppClient)->bAwaitingAccept = false;
    }
    else {
        const DWORD lastError = GetLastError();
        if (lastError == ERROR_IO_PENDING)
            return RP_NO_ERROR;

        if (lastError == ERROR_PIPE_CONNECTED) {
            (*ppClient)->bAwaitingAccept = false;
        }
        else if (lastError != ERROR_IO_PENDING && lastError != ERROR_PIPE_LISTENING) {
            ClosePipeConnection_(ppClient, pActiveClients);
            SetLastError(lastError);
            return RP_BROKEN_PIPE;
        }
    }

    if (!(*ppClient)->bAwaitingAccept && !ReadPipeContent_(ppClient, pActiveClients)) {
        //ClosePipeConnection_(ppClient, pActiveClients); // <-- already cleaned by ReadPipeContent_()
        return RP_BROKEN_PIPE;
    }

    return RP_NO_ERROR;
}
//----------------------------------------------------------------------------
static void IOCompletionLoop_(HANDLE hCompletionPort, FIOWrapperPipeClient* pSpuriousClient, int* pActiveClients) {
    int completionStatuslastError = NO_ERROR;
    const auto fShouldContinue = [&]() -> bool {
        if (pSpuriousClient && *pActiveClients == 1) {
            IOWRAPPER_LOG("child process closed the connection, close spurious client #%d\n", pSpuriousClient->nClientId);
            ClosePipeConnection_(&pSpuriousClient, pActiveClients);
        }
        return (*pActiveClients > 0);
    };

    do {
        DWORD nBytesTransferred = 0;
        FIOWrapperPipeClient* pClient = nullptr;
        LPOVERLAPPED lpOverlapped = nullptr;

        SetLastError(NO_ERROR);
        const BOOL bSucceed = GetQueuedCompletionStatus(hCompletionPort, &nBytesTransferred, (PULONG_PTR)&pClient, &lpOverlapped, GIOWrapperTimeoutInMs);
        completionStatuslastError = GetLastError();

        IOWRAPPER_LOG("pipe completion loop: active clients = %d (error: %d)\n", *pActiveClients, completionStatuslastError);

        if (!bSucceed) {
            if (pClient != nullptr && completionStatuslastError != ERROR_IO_PENDING && completionStatuslastError != ERROR_PIPE_LISTENING) {
                IOWRAPPER_LOG("close client #%d with error %d\n", pClient->nClientId, completionStatuslastError);
                ClosePipeConnection_(&pClient, pActiveClients);
            }
            continue;
        }

        IOWRAPPER_LOG("pipe completion port: ping clients #%d\n", pClient->nClientId);

        if (pClient->bAwaitingAccept) {
            pClient->bAwaitingAccept = false;

            if (pClient == pSpuriousClient && pClient->pwzNamedPipe) {
                pSpuriousClient = nullptr; // create a new client for potential subprocesses
                for (;;) {
                    FIOWrapperPipeClient* pNew = nullptr;
                    if (CreateNamedPipeConnection_(&pNew, hCompletionPort, pClient->pwzNamedPipe, pClient->hOutput, pActiveClients) == RP_NO_ERROR) {
                        if (pNew->bAwaitingAccept) {
                            pSpuriousClient = pNew;
                            ReadPipeContent_(&pNew, pActiveClients);
                            break;
                        }
                    }
                }
            }
        }

        if (nBytesTransferred > 0) {
            if (nBytesTransferred <= offsetof(FIODetouringMessage, szMessage)) {
                ClosePipeConnection_(&pClient, pActiveClients);
                continue;
            }

            // dump file access message to dependency output file
            const FIODetouringMessage* const pMessage = (const FIODetouringMessage*)pClient->szBuffer;

            pClient->szBuffer[nBytesTransferred] = '\n';
            pClient->szBuffer[nBytesTransferred + 1] = '\0';

            // the first char is used to encode access mode
            bool bRead{ 0 }, bWrite{ 0 }, bExecute{ 0 };
            IODetouring_CharToFileAccess(pMessage->szMessage[0], &bRead, &bWrite, &bExecute);

            IOWRAPPER_LOG("[%c%c%c] %s",
                bRead ? 'R' : '-',
                bWrite ? 'W' : '-',
                bExecute ? 'X' : '-',
                &pMessage->szMessage[1]);

            // start an asynchronous/overlapped write through `writeClient`
            DWORD nBytesWritten = 0;
            WriteFile(pClient->hOutput, pMessage->szMessage, nBytesTransferred - offsetof(FIODetouringMessage, szMessage) + 1, &nBytesWritten, NULL);
        }

        ReadPipeContent_(&pClient, pActiveClients);

    } while (fShouldContinue());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static ERunProcess RunProcessWithDetours(
    int* pExitCode,                     // exit code returned by process, or GetLastError() when failed
    PCSTR pszIODetouringDll,            // path to the IODetouring-WinXX-XXX.dll to inject in child process
    PCWSTR pwzNamedPipePrefix,          // prefix of the pipe for this particular process, should be unique
    PCWSTR pwzExecutableName,           // executable path, provide an absolute path to bypass %PATH% resolution
    PCWSTR pwzCommandLine,              // process arguments as a string, separated by spaces and within quotes when needed
    PCWSTR pwzzIgnoredApplications,     // child application matching those name won't be detoured, as a null-terminated array of null-terminated strings
    EIODetouringOptions nOptions,       // file matching any of those attribute won't pass through `onFilename` event
    HANDLE hDependencyFile              // handle to dependency output file
) {
    // Format named pipe path
    WCHAR wzNamedPipe[MAX_PATH] = TEXT("");
    StringCchPrintfW(wzNamedPipe, ARRAYSIZE(wzNamedPipe), L"\\\\.\\pipe\\%ls-%ls.%d",
        GIODetougingGlobalPrefix, pwzNamedPipePrefix, GetCurrentProcessId());

    // Create IO completion port for named pipe
    HANDLE const hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
    if (hCompletionPort == NULL) {
        *pExitCode = GetLastError();
        return RP_COMPLETION_PORT;
    }
    ON_SCOPE_EXIT([hCompletionPort]() {
        CloseHandle(hCompletionPort);
    });

    // Create named pipe connection
    int nActiveClients = 0;
    FIOWrapperPipeClient* pSpuriousClient = nullptr;
    if (ERunProcess err = CreateNamedPipeConnection_(&pSpuriousClient, hCompletionPort, wzNamedPipe, hDependencyFile, &nActiveClients)) {
        *pExitCode = GetLastError();
        return err;
    }
    ON_SCOPE_EXIT([&pSpuriousClient, &nActiveClients]() {
        ClosePipeConnection_(&pSpuriousClient, &nActiveClients);
    });

#if USE_IOWRAPPER_THREAD
    // Launch IO completion port on a separate thread
    struct FIOCompletionParameter {
        HANDLE hCompletionPort;
        FIOWrapperPipeClient* pSpuriousClient;
        int* pActiveClients;
    } IOCompletionParameter{
        .hCompletionPort = hCompletionPort,
            .pSpuriousClient = pSpuriousClient,
            .pActiveClients = &nActiveClients,
    };

    IOWRAPPER_LOG("launch IO completion thread\n");
    pSpuriousClient = nullptr;

    DWORD dwIOCompletionThread = 0;
    HANDLE const hIOCompletionThread = CreateThread(
        NULL, 0,
        [](LPVOID lpParameter) -> DWORD {
            FIOCompletionParameter* const pIOCompletion = (FIOCompletionParameter*)lpParameter;
            IOCompletionLoop_(pIOCompletion->hCompletionPort, pIOCompletion->pSpuriousClient, pIOCompletion->pActiveClients);
            return 0;
        },
        &IOCompletionParameter,
        0, &dwIOCompletionThread);

    if (hIOCompletionThread == NULL || hIOCompletionThread == INVALID_HANDLE_VALUE) {
        *pExitCode = GetLastError();
        return RP_WORKER_THREAD;
    }
#endif

    // Prepare detoured process informations
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    constexpr DWORD dwProcessFlags =
        CREATE_DEFAULT_ERROR_MODE |
        CREATE_SUSPENDED;

    // Redirect child process output to current process
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    // Launch detoured process with DLL injection
    if (!DetourCreateProcessWithDllEx(
        pwzExecutableName,
        (LPWCH)pwzCommandLine,
        NULL, NULL, TRUE, dwProcessFlags,
        NULL, // inherit current process environment block
        NULL, // inherit current process working directory
        &si, &pi,
        pszIODetouringDll, NULL)) {
        *pExitCode = GetLastError();
        return RP_CREATE_PROCESS;
    }
    ON_SCOPE_EXIT([&pi]() {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    });

    // Copy detour payload to the new process
    IOWRAPPER_LOG("copy detour payload to process %08X\n", pi.hProcess);
    if (!CopyDetourPayloadToProcess_(pi.hProcess, pwzNamedPipePrefix, pwzzIgnoredApplications, nOptions)) {
        *pExitCode = GetLastError();
        return RP_PAYLOAD_COPY;
    }

    // Resume process execution and process messages
    IOWRAPPER_LOG("resume suspended child process %08X\n", pi.hProcess);
    ResumeThread(pi.hThread);

#if !USE_IOWRAPPER_THREAD
    // Consume all IO (every read requests goes through IO completion port with overlapped events)
    IOWRAPPER_LOG("launch pipe completion loop for process %08X\n", pi.hProcess);
    IOCompletionLoop_(hCompletionPort, pSpuriousClient, &nActiveClients);
    pSpuriousClient = nullptr;
#endif

    // Wait for process exit and cleanup resources
    IOWRAPPER_LOG("wait for process %08X exit\n", pi.hProcess);
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Try to retrieve exit code
    IOWRAPPER_LOG("get process %08X exit code\n", pi.hProcess);
    DWORD dwExitCode = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwExitCode)) {
        *pExitCode = GetLastError();
        return RP_EXIT_CODE;
    }

    IOWRAPPER_LOG("exit code: %d (%X)\n", dwExitCode, dwExitCode);
    *pExitCode = dwExitCode;

#if USE_IOWRAPPER_THREAD
    // Wait for worker thread to process all pipe messages
    IOWRAPPER_LOG("wait for worker thread %08X exit\n", hIOCompletionThread);
    WaitForSingleObject(hIOCompletionThread, INFINITE);
#endif

    return RP_NO_ERROR;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
