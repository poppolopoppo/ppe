#pragma once

#include "IODetouring.h"

#include "Meta/Singleton.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VOID PPE_IODETOURING_API VSafePrintf(PCSTR pszMsg, va_list args, PCHAR pszBuffer, LONG cbBuffer);
PCHAR PPE_IODETOURING_API SafePrintf(PCHAR pszBuffer, LONG cbBuffer, PCSTR pszMsg, ...);
//----------------------------------------------------------------------------
class FIODetouringTblog {
public:
    STATIC_CONST_INTEGRAL(int, PayloadExitCode, -42);

    struct FMessage {
        DWORD   nBytes{ 0 };
        CHAR    szMessage[32764]; // 32768 - sizeof(nBytes)
    };
    using PMessage = FMessage*;

    enum class EOptions : DWORD {
        None = 0,

        IgnoreAbsorbed      = 1<<0,
        IgnoreCleanup       = 1<<1,
        IgnoreDelete        = 1<<2,
        IgnoreDirectory     = 1<<3,
        IgnoreDoNone        = 1<<4,
        IgnorePipe          = 1<<5,
        IgnoreTemporary     = 1<<6,
        IgnoreStdio         = 1<<7,
        IgnoreSystem        = 1<<8,

        AppendStdout        = 1<<9,
        AppendStderr        = 1<<10,
    };
    ENUM_FLAGS_FRIEND(EOptions);

    struct FPayload {
        WCHAR       wzzMountedPaths[4096] = L"";
        WCHAR       wzzIgnoredApplications[1024] = L"";
        WCHAR       wzNamedPipe[256] = L"";
        WCHAR       wzStdin[256] = L"";
        WCHAR       wzStdout[256] = L"";
        WCHAR       wzStderr[256] = L"";
        DWORD       rGeneology[64] = { 0 };
        DWORD       nGeneology = 0;
        DWORD       nParentProcessId = 0;
        DWORD       nTraceProcessId = 0;
        EOptions    nPayloadOptions = EOptions::None;
    };
    using PPayload = FPayload*;

    const FPayload& Payload() const { return _payload; }

    PPE_IODETOURING_API static void Create();
    PPE_IODETOURING_API static void Destroy();
    NODISCARD PPE_IODETOURING_API static FIODetouringTblog& Get() NOEXCEPT;

    PPE_IODETOURING_API void CopyPayload(PPayload pPayload);
    NODISCARD PPE_IODETOURING_API BOOL ChildPayload(HANDLE hProcess, DWORD nProcessId, PCHAR pszId, HANDLE hStdin, HANDLE hStdout, HANDLE hStderr);

    NODISCARD PPE_IODETOURING_API BOOL Open();
    PPE_IODETOURING_API VOID Close();

    PPE_IODETOURING_API VOID PrintV(PCSTR pszMsgf, va_list args);
    PPE_IODETOURING_API VOID Printf(PCSTR pszMsgf, ...);

private:
    FIODetouringTblog();
    ~FIODetouringTblog();

    FPayload _payload;

    CRITICAL_SECTION _csChildPayload;
    FPayload _childPayload;

    CRITICAL_SECTION _csPipe;
    HANDLE _hPipe = INVALID_HANDLE_VALUE;
    FMessage _rMessage{};

    DWORD _nTraceProcessId = 0;
    LONG _nChildCnt = 0;
};
//----------------------------------------------------------------------------
using FIODetouringMessage = FIODetouringTblog::FMessage;
using EIODetouringOptions = FIODetouringTblog::EOptions;
using FIODetouringPayload = FIODetouringTblog::FPayload;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
