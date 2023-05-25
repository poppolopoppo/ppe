#include "IODetouringTblog.h"

#include "IODetouringDebug.h"
#include "IODetouringFiles.h"
#include "IODetouringHooks.h"

#include "detours-external.h"

#include <stdio.h>
#include <strsafe.h>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static BOOL IsInherited_(HANDLE hHandle) {
    DWORD dwFlags{ 0 };
    if (GetHandleInformation(hHandle, &dwFlags)) {
        return ((dwFlags & HANDLE_FLAG_INHERIT) == HANDLE_FLAG_INHERIT);
    }
    return FALSE;
}
//----------------------------------------------------------------------------
static void LoadStdHandleName_(DWORD id, PCWSTR pwzBuffer, BOOL bAppend) {
    HANDLE hFile = GetStdHandle(id);

    if ((hFile != INVALID_HANDLE_VALUE) && pwzBuffer[0] != '\0') {
        FIODetouringFiles& files = FIODetouringFiles::Get();
        FIODetouringFiles::PFileInfo const pInfo = files.FindPartial(pwzBuffer);

        pInfo->bStdio = true;
        if (bAppend)
            pInfo->bAppend = TRUE;

        files.Remember(hFile, pInfo);
    }
}
//----------------------------------------------------------------------------
static void SaveStdHandleName_(HANDLE hFile, PWCHAR pwzBuffer, BOOL* bAppend) {
    pwzBuffer[0] = '\0';

    if ((hFile != INVALID_HANDLE_VALUE) && IsInherited_(hFile)) {
        if (FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().RecallFile(hFile)) {
            FIODetouringFiles::StringCopy(pwzBuffer, pInfo->pwzPath);

            pInfo->bStdio = true;
            if (pInfo->bAppend && bAppend != nullptr)
                *bAppend = TRUE;
        }
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FIODetouringTblog::FIODetouringTblog() {
    InitializeCriticalSection(&_csPipe);
    InitializeCriticalSection(&_csChildPayload);

    ZeroMemory(&_payload, sizeof(_payload));

    _payload.nPayloadOptions = (
        // ignore every "special" files by default
        EOptions::IgnoreAbsorbed    |
        EOptions::IgnoreCleanup     |
        EOptions::IgnoreDelete      |
        EOptions::IgnoreDirectory   |
        EOptions::IgnorePipe        |
        EOptions::IgnoreStdio       |
        EOptions::IgnoreSystem      );
}
//----------------------------------------------------------------------------
FIODetouringTblog::~FIODetouringTblog() {
    DeleteCriticalSection(&_csPipe);
    DeleteCriticalSection(&_csChildPayload);
}
//----------------------------------------------------------------------------
void* FIODetouringTblog::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared lib
}
//----------------------------------------------------------------------------
void FIODetouringTblog::CopyPayload(PPayload pPayload) {
    CopyMemory(&_payload, pPayload, sizeof(_payload));

    LoadStdHandleName_(STD_INPUT_HANDLE, _payload.wzStdin, FALSE);
    LoadStdHandleName_(STD_OUTPUT_HANDLE, _payload.wzStdout, _payload.nPayloadOptions & EOptions::AppendStdout);
    LoadStdHandleName_(STD_ERROR_HANDLE, _payload.wzStderr, _payload.nPayloadOptions & EOptions::AppendStderr);

    _nTraceProcessId = _payload.nTraceProcessId;
}
//----------------------------------------------------------------------------
BOOL FIODetouringTblog::ChildPayload(HANDLE hProcess, DWORD nProcessId, PCHAR pszId, HANDLE hStdin, HANDLE hStdout, HANDLE hStderr) {
    EnterCriticalSection(&_csChildPayload);

    FIODetouringFiles& files = FIODetouringFiles::Get();
    FIODetouringFiles::PProcInfo const pProc = files.CreateProc(hProcess, nProcessId);
    files.Remember(hProcess, pProc);

    ZeroMemory(&_childPayload, sizeof(_childPayload));
    CopyMemory(&_childPayload, &_payload, sizeof(_childPayload));

    _childPayload.nParentProcessId = GetCurrentProcessId();
    _childPayload.rGeneology[_childPayload.nGeneology++] = DWORD(InterlockedIncrement(&_nChildCnt));

    BOOL bStdoutAppend = FALSE, bStderrAppend = FALSE;
    SaveStdHandleName_(hStdin, _childPayload.wzStdin, NULL);
    SaveStdHandleName_(hStdout, _childPayload.wzStdout, &bStdoutAppend);
    SaveStdHandleName_(hStderr, _childPayload.wzStderr, &bStderrAppend);

    if (bStdoutAppend)
        _childPayload.nPayloadOptions += EOptions::AppendStdout;
    else
        _childPayload.nPayloadOptions -= EOptions::AppendStdout;

    if (bStderrAppend)
        _childPayload.nPayloadOptions += EOptions::AppendStderr;
    else
        _childPayload.nPayloadOptions -= EOptions::AppendStderr;

    DetourCopyPayloadToProcess(hProcess, GIODetouringGuid, &_childPayload, sizeof(_childPayload));

    for (DWORD i = 0; i < _childPayload.nGeneology; i++)
        pszId = SafePrintf(pszId, 16, "%d.", _childPayload.rGeneology[i]);
    *pszId = '\0';

    LeaveCriticalSection(&_csChildPayload);
    return TRUE;
}
//----------------------------------------------------------------------------
VOID FIODetouringTblog::PrintV(PCSTR pszMsgf, va_list args) {
    if (_hPipe == INVALID_HANDLE_VALUE) {
        return;
    }

    EnterCriticalSection(&_csPipe);

    DWORD cbWritten = 0;

    PCHAR pszBuf = _rMessage.szMessage;
    VSafePrintf(pszMsgf, args,
                pszBuf, (int)(_rMessage.szMessage + sizeof(_rMessage.szMessage) - pszBuf));

    PCHAR pszEnd = _rMessage.szMessage;
    for (; *pszEnd; pszEnd++) {
        // no internal contents.
    }
    _rMessage.nBytes = (DWORD)(pszEnd - ((PCSTR)&_rMessage));

    // If the write fails, then we abort
    if (_hPipe != INVALID_HANDLE_VALUE) {
        if (!IODETOURING_REAL(WriteFile, _hPipe, &_rMessage, _rMessage.nBytes, &cbWritten, NULL)) {
            IODETOURING_REAL(ExitProcess, 9991);
        }
    }

    LeaveCriticalSection(&_csPipe);
}
//----------------------------------------------------------------------------
VOID FIODetouringTblog::Printf(PCSTR pszMsgf, ...) {
    if (_hPipe == INVALID_HANDLE_VALUE) {
        return;
    }

    va_list args;
    va_start(args, pszMsgf);
    PrintV(pszMsgf, args);
    va_end(args);
}
//----------------------------------------------------------------------------
BOOL FIODetouringTblog::Open() {
    EnterCriticalSection(&_csPipe);

    WCHAR wzPipe[MAX_PATH] = TEXT("");
    StringCchPrintfW(wzPipe, ARRAYSIZE(wzPipe), L"\\\\.\\pipe\\%ls-%ls.%d",
        GIODetougingGlobalPrefix, _payload.wzNamedPipe, _payload.nTraceProcessId);

    for (int retries = 0; retries < 10; retries++) {
        WaitNamedPipeW(wzPipe, 10000); // Wait up to 10 seconds for a pipe to appear.

        _hPipe = IODETOURING_REAL(CreateFileW, wzPipe, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (_hPipe != INVALID_HANDLE_VALUE) {
            DWORD dwMode = PIPE_READMODE_MESSAGE;
            if (SetNamedPipeHandleState(_hPipe, &dwMode, NULL, NULL)) {
                LeaveCriticalSection(&_csPipe);
                return TRUE;
            }
        }
    }

    LeaveCriticalSection(&_csPipe);

    // Couldn't open pipe.
    PPE_DEBUG_BREAK();
    IODETOURING_REAL(ExitProcess, 9990);
    return FALSE;
}
//----------------------------------------------------------------------------
VOID FIODetouringTblog::Close() {
    EnterCriticalSection(&_csPipe);

    if (_hPipe != INVALID_HANDLE_VALUE) {
        DWORD cbWritten = 0;

        _rMessage.nBytes = 0;

        IODETOURING_REAL(WriteFile, _hPipe, &_rMessage, 4, &cbWritten, NULL);
        FlushFileBuffers(_hPipe);
        IODETOURING_REAL(CloseHandle, _hPipe);

        _hPipe = INVALID_HANDLE_VALUE;
    }

    LeaveCriticalSection(&_csPipe);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Completely side-effect free printf replacement (but no FP numbers).
// https://github.com/microsoft/Detours/blob/main/samples/tracebld/trcbld.cpp#L1644
//----------------------------------------------------------------------------
static PCHAR do_base(PCHAR pszOut, UINT64 nValue, UINT nBase, PCSTR pszDigits) {
    CHAR szTmp[96];
    int nDigit = sizeof(szTmp)-2;
    for (; nDigit >= 0; nDigit--) {
        szTmp[nDigit] = pszDigits[nValue % nBase];
        nValue /= nBase;
    }
    for (nDigit = 0; nDigit < sizeof(szTmp) - 2 && szTmp[nDigit] == '0'; nDigit++) {
        // skip leading zeros.
    }
    for (; nDigit < sizeof(szTmp) - 1; nDigit++) {
        *pszOut++ = szTmp[nDigit];
    }
    *pszOut = '\0';
    return pszOut;
}
//----------------------------------------------------------------------------
static PCHAR do_str(PCHAR pszOut, PCHAR pszEnd, PCSTR pszIn) {
    while (*pszIn && pszOut < pszEnd) {
        *pszOut++ = *pszIn++;
    }
    *pszOut = '\0';
    return pszOut;
}
//----------------------------------------------------------------------------
static PCHAR do_wstr(PCHAR pszOut, PCHAR pszEnd, PCWSTR pszIn) {
    while (*pszIn && pszOut < pszEnd) {
        *pszOut++ = (CHAR)*pszIn++;
    }
    *pszOut = '\0';
    return pszOut;
}
//----------------------------------------------------------------------------
#if _MSC_VER >= 1900
#pragma warning(push)
#pragma warning(disable:4456) // declaration hides previous local declaration
#endif
//----------------------------------------------------------------------------
VOID VSafePrintf(PCSTR pszMsg, va_list args, PCHAR pszBuffer, LONG cbBuffer) {
    PCHAR pszOut = pszBuffer;
    PCHAR pszEnd = pszBuffer + cbBuffer - 1;
    pszBuffer[0] = '\0';

    __try {
        while (*pszMsg && pszOut < pszEnd) {
            if (*pszMsg == '%') {
                CHAR szHead[4] = "";
                INT nLen;
                INT nWidth = 0;
                INT nPrecision = 0;
                BOOL fLeft = FALSE;
                BOOL fPositive = FALSE;
                BOOL fPound = FALSE;
                BOOL fBlank = FALSE;
                BOOL fZero = FALSE;
                BOOL fDigit = FALSE;
                BOOL fSmall = FALSE;
                BOOL fLarge = FALSE;
                BOOL f64Bit = FALSE;
                PCSTR pszArg = pszMsg;

                pszMsg++;

                for (; (*pszMsg == '-' ||
                        *pszMsg == '+' ||
                        *pszMsg == '#' ||
                        *pszMsg == ' ' ||
                        *pszMsg == '0'); pszMsg++) {
                    switch (*pszMsg) {
                      case '-': fLeft = TRUE; break;
                      case '+': fPositive = TRUE; break;
                      case '#': fPound = TRUE; break;
                      case ' ': fBlank = TRUE; break;
                      case '0': fZero = TRUE; break;
                    }
                }

                if (*pszMsg == '*') {
                    nWidth = va_arg(args, INT);
                    pszMsg++;
                }
                else {
                    while (*pszMsg >= '0' && *pszMsg <= '9') {
                        nWidth = nWidth * 10 + (*pszMsg++ - '0');
                    }
                }
                if (*pszMsg == '.') {
                    pszMsg++;
                    fDigit = TRUE;
                    if (*pszMsg == '*') {
                        nPrecision = va_arg(args, INT);
                        pszMsg++;
                    }
                    else {
                        while (*pszMsg >= '0' && *pszMsg <= '9') {
                            nPrecision = nPrecision * 10 + (*pszMsg++ - '0');
                        }
                    }
                }

                if (*pszMsg == 'h') {
                    fSmall = TRUE;
                    pszMsg++;
                }
                else if (*pszMsg == 'l') {
                    fLarge = TRUE;
                    pszMsg++;
                }
                else if (*pszMsg == 'I' && pszMsg[1] == '6' && pszMsg[2] == '4') {
                    f64Bit = TRUE;
                    pszMsg += 3;
                }

                if (*pszMsg == 's' || *pszMsg == 'e' || *pszMsg == 'c') {
                    // We ignore the length, precision, and alignment
                    // to avoid using a temporary buffer.

                    if (*pszMsg == 's') { // [GalenH] need to not use temp.
                        PVOID pvData = va_arg(args, PVOID);

                        pszMsg++;

                        if (fSmall) {
                            fLarge = FALSE;
                        }

                        __try {
                            if (pvData == NULL) {
                                pszOut = do_str(pszOut, pszEnd, "-NULL-");
                            }
                            else if (fLarge) {
                                pszOut = do_wstr(pszOut, pszEnd, (PWCHAR)pvData);
                            }
                            else {
                                pszOut = do_str(pszOut, pszEnd, (PCHAR)pvData);
                            }
                        } __except(EXCEPTION_EXECUTE_HANDLER) {
                            pszOut = do_str(pszOut, pszEnd, "-");
                            pszOut = do_base(pszOut, (UINT64)pvData, 16,
                                             "0123456789ABCDEF");
                            pszOut = do_str(pszOut, pszEnd, "-");
                        }
                    }
                    else if (*pszMsg == 'e')    {   // Escape the string.
                        PVOID pvData = va_arg(args, PVOID);

                        pszMsg++;

                        if (fSmall) {
                            fLarge = FALSE;
                        }

                        __try {
                            if (pvData == NULL) {
                                pszOut = do_str(pszOut, pszEnd, "-NULL-");
                            }
                            else if (fLarge) {
                                pszOut = do_wstr(pszOut, pszEnd, (PWCHAR)pvData);
                            }
                            else {
                                pszOut = do_str(pszOut, pszEnd, (PCHAR)pvData);
                            }
                        } __except(EXCEPTION_EXECUTE_HANDLER) {
                            pszOut = do_str(pszOut, pszEnd, "-");
                            pszOut = do_base(pszOut, (UINT64)pvData, 16, "0123456789ABCDEF");
                            pszOut = do_str(pszOut, pszEnd, "-");
                        }
                    }
                    else {
                        CHAR szTemp[2];
                        pszMsg++;

                        szTemp[0] = (CHAR)va_arg(args, INT);
                        szTemp[1] = '\0';
                        pszOut = do_str(pszOut, pszEnd, szTemp);
                    }
                }
                else if (*pszMsg == 'd' || *pszMsg == 'i' || *pszMsg == 'o' ||
                         *pszMsg == 'x' || *pszMsg == 'X' || *pszMsg == 'b' ||
                         *pszMsg == 'u') {
                    CHAR szTemp[128];
                    UINT64 value;
                    if (f64Bit) {
                        value = va_arg(args, UINT64);
                    }
                    else {
                        value = va_arg(args, UINT);
                    }

                    if (*pszMsg == 'x') {
                        pszMsg++;
                        nLen = (int)(do_base(szTemp, value, 16, "0123456789abcdef") - szTemp);
                        if (fPound && value) {
                            do_str(szHead, szHead + sizeof(szHead) - 1, "0x");
                        }
                    }
                    else if (*pszMsg == 'X') {
                        pszMsg++;
                        nLen = (int)(do_base(szTemp, value, 16, "0123456789ABCDEF") - szTemp);
                        if (fPound && value) {
                            do_str(szHead, szHead + sizeof(szHead) - 1, "0X");
                        }
                    }
                    else if (*pszMsg == 'd') {
                        pszMsg++;
                        if ((INT64)value < 0) {
                            value = -(INT64)value;
                            do_str(szHead, szHead + sizeof(szHead) - 1, "-");
                        }
                        else if (fPositive) {
                            if (value > 0) {
                                do_str(szHead, szHead + sizeof(szHead) - 1, "+");
                            }
                        }
                        else if (fBlank) {
                            if (value > 0) {
                                do_str(szHead, szHead + sizeof(szHead) - 1, " ");
                            }
                        }
                        nLen = (int)(do_base(szTemp, value, 10, "0123456789") - szTemp);
                        nPrecision = 0;
                    }
                    else if (*pszMsg == 'u') {
                        pszMsg++;
                        nLen = (int)(do_base(szTemp, value, 10, "0123456789") - szTemp);
                        nPrecision = 0;
                    }
                    else if (*pszMsg == 'o') {
                        pszMsg++;
                        nLen = (int)(do_base(szTemp, value, 8, "01234567") - szTemp);
                        nPrecision = 0;

                        if (fPound && value) {
                            do_str(szHead, szHead + sizeof(szHead) - 1, "0");
                        }
                    }
                    else if (*pszMsg == 'b') {
                        pszMsg++;
                        nLen = (int)(do_base(szTemp, value, 2, "01") - szTemp);
                        nPrecision = 0;

                        if (fPound && value) {
                            do_str(szHead, szHead + sizeof(szHead) - 1, "0b");
                        }
                    }
                    else {
                        pszMsg++;
                        if ((INT64)value < 0) {
                            value = -(INT64)value;
                            do_str(szHead, szHead + sizeof(szHead) - 1, "-");
                        }
                        else if (fPositive) {
                            if (value > 0) {
                                do_str(szHead, szHead + sizeof(szHead) - 1, "+");
                            }
                        }
                        else if (fBlank) {
                            if (value > 0) {
                                do_str(szHead, szHead + sizeof(szHead) - 1, " ");
                            }
                        }
                        nLen = (int)(do_base(szTemp, value, 10, "0123456789") - szTemp);
                        nPrecision = 0;
                    }

                    INT nHead = 0;
                    for (; szHead[nHead]; nHead++) {
                        // Count characters in head string.
                    }

                    if (fLeft) {
                        if (nHead) {
                            pszOut = do_str(pszOut, pszEnd, szHead);
                            nLen += nHead;
                        }
                        pszOut = do_str(pszOut, pszEnd, szTemp);
                        for (; nLen < nWidth && pszOut < pszEnd; nLen++) {
                            *pszOut++ = ' ';
                        }
                    }
                    else if (fZero) {
                        if (nHead) {
                            pszOut = do_str(pszOut, pszEnd, szHead);
                            nLen += nHead;
                        }
                        for (; nLen < nWidth && pszOut < pszEnd; nLen++) {
                            *pszOut++ = '0';
                        }
                        pszOut = do_str(pszOut, pszEnd, szTemp);
                    }
                    else {
                        if (nHead) {
                            nLen += nHead;
                        }
                        for (; nLen < nWidth && pszOut < pszEnd; nLen++) {
                            *pszOut++ = ' ';
                        }
                        if (nHead) {
                            pszOut = do_str(pszOut, pszEnd, szHead);
                        }
                        pszOut = do_str(pszOut, pszEnd, szTemp);
                    }
                }
                else if (*pszMsg == 'p') {
                    CHAR szTemp[64];
                    ULONG_PTR value;
                    value = va_arg(args, ULONG_PTR);

                    if (*pszMsg == 'p') {
                        pszMsg++;
                        nLen = (int)(do_base(szTemp, (UINT64)value, 16, "0123456789abcdef") - szTemp);
                        if (fPound && value) {
                            do_str(szHead, szHead + sizeof(szHead) - 1, "0x");
                        }
                    }
                    else {
                        pszMsg++;
                        nLen = (int)(do_base(szTemp, (UINT64)value, 16, "0123456789ABCDEF") - szTemp);
                        if (fPound && value) {
                            do_str(szHead, szHead + sizeof(szHead) - 1, "0x");
                        }
                    }

                    INT nHead = 0;
                    for (; szHead[nHead]; nHead++) {
                        // Count characters in head string.
                    }

                    if (nHead) {
                        pszOut = do_str(pszOut, pszEnd, szHead);
                        nLen += nHead;
                    }
                    for (; nLen < nWidth && pszOut < pszEnd; nLen++) {
                        *pszOut++ = '0';
                    }
                    pszOut = do_str(pszOut, pszEnd, szTemp);
                }
                else {
                    pszMsg++;
                    while (pszArg < pszMsg && pszOut < pszEnd) {
                        *pszOut++ = *pszArg++;
                    }
                }
            }
            else {
                if (pszOut < pszEnd) {
                    *pszOut++ = *pszMsg++;
                }
            }
        }
        *pszOut = '\0';
        pszBuffer[cbBuffer - 1] = '\0';
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        PCHAR pszOut = pszBuffer;
        *pszOut = '\0';
        pszOut = do_str(pszOut, pszEnd, "-exception:");
        pszOut = do_base(pszOut, (UINT64)GetExceptionCode(), 10, "0123456789");
        pszOut = do_str(pszOut, pszEnd, "-");
    }
}
//----------------------------------------------------------------------------
#if _MSC_VER >= 1900
#pragma warning(pop)
#endif
//----------------------------------------------------------------------------
PCHAR SafePrintf(PCHAR pszBuffer, LONG cbBuffer, PCSTR pszMsg, ...) {
    va_list args;
    va_start(args, pszMsg);
    VSafePrintf(pszMsg, args, pszBuffer, cbBuffer);
    va_end(args);

    while (*pszBuffer) {
        pszBuffer++;
    }
    return pszBuffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------