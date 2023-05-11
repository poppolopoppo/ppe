#pragma once

#include "IODetouring.h"

#include "Meta/Singleton.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VOID PPE_IODETOURING_API VSafePrintf(PCSTR pszMsg, va_list args, PCHAR pszBuffer, LONG cbBuffer);
PCHAR PPE_IODETOURING_API SafePrintf(PCHAR pszBuffer, LONG cbBuffer, PCSTR pszMsg, ...);
//----------------------------------------------------------------------------
class PPE_IODETOURING_API FIODetouringTblog : PPE::Meta::TSingleton<FIODetouringTblog> {
    using singleton_type = PPE::Meta::TSingleton<FIODetouringTblog>;
    friend class singleton_type;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT; // for shared lib
public:
    struct FMessage {
        DWORD   nBytes;
        CHAR    szMessage[32764]; // 32768 - sizeof(nBytes)
    };
    using PMessage = FMessage*;

    struct FPayload {
        DWORD   nParentProcessId;
        DWORD   nTraceProcessId;
        DWORD   nGeneology;
        DWORD   rGeneology[64];
        WCHAR   wzParents[256];
        WCHAR   wzStdin[256];
        WCHAR   wzStdout[256];
        WCHAR   wzStderr[256];
        BOOL    bStdoutAppend;
        BOOL    bStderrAppend;
        WCHAR   wzzDrop[1024];  // Like an environment: zero terminated strings with a last zero.
        WCHAR   wzzEnvironment[32768];
    };
    using PPayload = FPayload*;

    using singleton_type::Get;
    using singleton_type::Destroy;

    static VOID Create() { singleton_type::Create(); }

    void CopyPayload(PPayload pPayload);
    BOOL ChildPayload(HANDLE hProcess, DWORD nProcessId, PCHAR pszId, HANDLE hStdin, HANDLE hStdout, HANDLE hStderr);

    BOOL Open();
    VOID Close();

    VOID PrintV(PCSTR pszMsgf, va_list args);
    VOID Printf(PCSTR pszMsgf, ...);

private:
    FIODetouringTblog();
    ~FIODetouringTblog();

    FPayload _payload;

    CRITICAL_SECTION _csChildPayload;
    FPayload _childPayload;

    CRITICAL_SECTION _csPipe;
    HANDLE _hPipe = INVALID_HANDLE_VALUE;
    FMessage _rMessage;

    DWORD _nTraceProcessId = 0;
    LONG _nChildCnt = 0;
};
//----------------------------------------------------------------------------
using FIODetouringMessage = FIODetouringTblog::FMessage;
using FIODetouringPayload = FIODetouringTblog::FPayload;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
