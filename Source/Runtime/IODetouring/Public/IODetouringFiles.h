#pragma once

#include "IODetouring.h"

#include "Allocator/SlabHeap.h"
#include "HAL/Windows/GlobalAllocator.h"
#include "IO/ConstChar.h"
#include "Meta/Singleton.h"
#include "Thread/ConcurrentHashMap.h"
#include "Thread/ThreadSafe.h"

#include <atomic>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Log all file accesses (read & write), can also alias files
//----------------------------------------------------------------------------
class PPE_IODETOURING_API FIODetouringFiles : PPE::Meta::TSingleton<FIODetouringFiles> {
    using singleton_type = PPE::Meta::TSingleton<FIODetouringFiles>;
    friend class singleton_type;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT; // for shared lib
public:
    using singleton_type::Get;
    using singleton_type::Destroy;

    static VOID Create(PCWSTR pwzStdin, PCWSTR pwzStdout, PCWSTR pwzStderr) {
        singleton_type::Create(pwzStdin, pwzStdout, pwzStderr);
    }

    VOID Dump();

    // filenames
    struct FFileInfo {
        PCWSTR  pwzPath{ nullptr };
        PBYTE   pbContent{ nullptr };

        DWORD   cbContent{ 0 };
        DWORD   nIndex{ 0 };
        DWORD   cbRead{ 0 };
        DWORD   cbWrite{ 0 };

        BOOL    bCantRead{ FALSE };        // Set for file that are opened Create
        BOOL    bRead{ FALSE };
        BOOL    bWrite{ FALSE };

        BOOL    bDelete{ FALSE };
        BOOL    bCleanup{ FALSE };
        BOOL    bSystemPath{ FALSE };
        BOOL    bTemporaryPath{ FALSE };
        BOOL    bTemporaryFile{ FALSE };
        BOOL    bPipe{ FALSE };
        BOOL    bStdio{ FALSE };

        BOOL    bAppend{ FALSE };
        BOOL    bAbsorbed{ FALSE };        // Absorbed by TraceBld.
        BOOL    bDirectory{ FALSE };
    };
    using PFileInfo = FFileInfo*;

    using FCaseInsensitiveConstChar = PPE::THashMemoizer<PPE::FConstWChar, // FS is case insensitive of Windows
        PPE::TConstCharHasher<wchar_t, PPE::ECase::Insensitive>,
        PPE::TConstCharEqualTo<wchar_t, PPE::ECase::Insensitive>>;

    using FFileKey = FCaseInsensitiveConstChar;

    PFileInfo FindFull(PCWSTR pwzPath);
    PFileInfo FindPartial(PCWSTR pwzPath);
    PFileInfo FindPartial(PCSTR pwzPath);

    // processes
    struct FProcInfo {
        HANDLE  hProc{ INVALID_HANDLE_VALUE };
        DWORD   nProcId{ 0 };
        DWORD   nIndex{ 0 };
    };
    using PProcInfo = FProcInfo*;

    PProcInfo CreateProc(HANDLE hProc, DWORD nProcId);
    BOOL Close(HANDLE hProc);

    // file handles
    struct FOpenFile {
        HANDLE hHandle{ INVALID_HANDLE_VALUE };
        PFileInfo pFile{ nullptr };
        PProcInfo pProc{ nullptr };
    };

    BOOL Forget(HANDLE hHandle);
    VOID SetRead(HANDLE hFile, DWORD cbData);
    VOID SetWrite(HANDLE hFile, DWORD cbData);
    PFileInfo RecallFile(HANDLE hFile) const NOEXCEPT;
    PProcInfo RecallProc(HANDLE hProc) const NOEXCEPT;
    BOOL Remember(HANDLE hFile, PFileInfo pInfo);
    BOOL Remember(HANDLE hProc, PProcInfo pInfo);

    // environment vars
    struct FEnvVar {
        BOOL bRead{ FALSE };
        DWORD nCount{ 0 };
    };

    VOID UseEnvVar(PCWSTR pwzVarname);
    VOID UseEnvVar(PCSTR pwzVarname);

    // path helpers
    static VOID StringCopy(PWCHAR pwzDst, PCWSTR pwzSrc);
    static DWORD StringLen(PCWSTR pwzSrc);
    static VOID EndInSlash(PWCHAR pwzPath) NOEXCEPT;
    static BOOL PrefixMatch(PCWSTR pwzPath, PCWSTR pwzPrefix) NOEXCEPT;
    static BOOL SuffixMatch(PCWSTR pwzPath, PCWSTR pwzSuffix) NOEXCEPT;

    // file access helpers
    static VOID NoteRead(PCSTR psz) {
        PFileInfo const pInfo = Get().FindPartial(psz);
        pInfo->bRead = TRUE;
    }

    static VOID NoteRead(PCWSTR pwz) {
        PFileInfo const pInfo = Get().FindPartial(pwz);
        pInfo->bRead = TRUE;
    }

    static VOID NoteWrite(PCSTR psz) {
        PFileInfo const pInfo = Get().FindPartial(psz);
        pInfo->bWrite = TRUE;
        if (!pInfo->bRead) {
            pInfo->bCantRead = TRUE;
        }
    }

    static VOID NoteWrite(PCWSTR pwz) {
        PFileInfo const pInfo = Get().FindPartial(pwz);
        pInfo->bWrite = TRUE;
        if (!pInfo->bRead) {
            pInfo->bCantRead = TRUE;
        }
    }

    static VOID NoteDelete(PCSTR psz) {
        PFileInfo const pInfo = Get().FindPartial(psz);
        if (pInfo->bWrite || pInfo->bRead) {
            pInfo->bCleanup = TRUE;
        }
        else {
            pInfo->bDelete = TRUE;
        }
        if (!pInfo->bRead) {
            pInfo->bCantRead = TRUE;
        }
    }

    static VOID NoteDelete(PCWSTR pwz) {
        PFileInfo const pInfo = Get().FindPartial(pwz);
        if (pInfo->bWrite || pInfo->bRead) {
            pInfo->bCleanup = TRUE;
        }
        else {
            pInfo->bDelete = TRUE;
        }
        if (!pInfo->bRead) {
            pInfo->bCantRead = TRUE;
        }
    }

    static VOID NoteCleanup(PCSTR psz) {
        PFileInfo const pInfo = Get().FindPartial(psz);
        pInfo->bCleanup = TRUE;
    }

    static VOID NoteCleanup(PCWSTR pwz) {
        PFileInfo const pInfo = Get().FindPartial(pwz);
        pInfo->bCleanup = TRUE;
    }

private:
    FIODetouringFiles(PCWSTR pwzStdin, PCWSTR pwzStdout, PCWSTR pwzStderr);
    ~FIODetouringFiles();

    VOID InitSystemPaths_();
    VOID InitFileInfo_(FFileInfo& file, PPE::FConstWChar pwzPath) NOEXCEPT;

    using heap_type = PPE::TThreadSafe<PPE::TSlabHeap<PPE::FWin32GlobalAllocatorPtr>, PPE::EThreadBarrier::CriticalSection>;
    static PPE::FConstWChar AllocString_(const heap_type::FExclusiveLock& heap, PCWSTR pwzPath);
    static void ReleaseStringCopy_(const heap_type::FExclusiveLock& heap, PPE::FConstWChar key);

    heap_type _heap;

    struct FHandleHash_ {
        PPE::hash_t operator ()(HANDLE hHandle) const NOEXCEPT {
            return PPE::hash_size_t_constexpr(uintptr_t(hHandle));
        }
    };

    using files_type = PPE::TConcurrentHashMap<FFileKey, FFileInfo,
        PPE::Meta::THash<FFileKey>,
        PPE::Meta::TEqualTo<FFileKey>,
        PPE::FWin32GlobalAllocatorPtr>;
    files_type _files;
    std::atomic<DWORD> _nFiles;

    using procs_type = PPE::TConcurrentHashMap<HANDLE, FProcInfo,
        FHandleHash_,
        PPE::Meta::TEqualTo<HANDLE>,
        PPE::FWin32GlobalAllocatorPtr>;
    procs_type _procs;
    std::atomic<DWORD> _nProcs;

    using opens_type = PPE::TConcurrentHashMap<HANDLE, FOpenFile,
        FHandleHash_,
        PPE::Meta::TEqualTo<HANDLE>,
        PPE::FWin32GlobalAllocatorPtr>;
    opens_type _opens;

    using envs_type = PPE::TConcurrentHashMap<FCaseInsensitiveConstChar, FEnvVar,
        PPE::Meta::THash<FCaseInsensitiveConstChar>,
        PPE::Meta::TEqualTo<FCaseInsensitiveConstChar>,
        PPE::FWin32GlobalAllocatorPtr>;
    envs_type _envs;

    WCHAR _wzSysPath[MAX_PATH];
    WCHAR _wzS64Path[MAX_PATH];
    WCHAR _wzTmpPath[MAX_PATH];
    WCHAR _wzExePath[MAX_PATH];
    DWORD _wcSysPath;
    DWORD _wcS64Path;
    DWORD _wcTmpPath;
    DWORD _wcExePath;

    PCWSTR _pwzStdin = L"";
    PCWSTR _pwzStdout = L"";
    PCWSTR _pwzStderr = L"";

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
