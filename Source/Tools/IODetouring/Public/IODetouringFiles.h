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

        SIZE_T  cbRead{ 0 };
        SIZE_T  cbWrite{ 0 };
        SIZE_T  cbContent{ 0 };

        DWORD   nIndex{ 0 };

        bool    bCantRead{ false };        // Set for file that are opened Create
        bool    bCantWrite{ false };       // Set for file that are opened ReadOnly
        bool    bRead{ false };
        bool    bWrite{ false };
        bool    bExecute{ false };

        bool    bDelete{ false };
        bool    bCleanup{ false };
        bool    bSystemPath{ false };
        bool    bTemporaryPath{ false };
        bool    bTemporaryFile{ false };
        bool    bPipe{ false };
        bool    bStdio{ false };

        bool    bAppend{ false };
        bool    bAbsorbed{ false };        // Absorbed by TraceBld.
        bool    bDirectory{ false };
    };
    using PFileInfo = FFileInfo*;

    using FCaseInsensitiveConstChar = PPE::THashMemoizer<PPE::FConstWChar, // FS is case insensitive of Windows
        PPE::TConstCharHasher<wchar_t, PPE::ECase::Insensitive>,
        PPE::TConstCharEqualTo<wchar_t, PPE::ECase::Insensitive>>;

    using FFileKey = FCaseInsensitiveConstChar;

    PFileInfo FindFull(PCWSTR pwzPath);
    PFileInfo FindPartial(PCWSTR pwzPath);
    PFileInfo FindPartial(PCSTR pszPath);

    // processes
    struct FProcInfo {
        HANDLE  hProc{ INVALID_HANDLE_VALUE };
        DWORD   nProcId{ 0 };
        DWORD   nIndex{ 0 };
    };
    using PProcInfo = FProcInfo*;

    PProcInfo CreateProc(HANDLE hProc, DWORD nProcId);
    BOOL Close(HANDLE hProc);

    BOOL ShouldDetourApplication(PCWSTR pwzPath) NOEXCEPT;
    BOOL ShouldDetourApplication(PCSTR pszPath) NOEXCEPT;

    // file handles
    struct FOpenFile {
        HANDLE hHandle{ INVALID_HANDLE_VALUE };
        PFileInfo pFile{ nullptr };
        PProcInfo pProc{ nullptr };
    };

    BOOL Forget(HANDLE hHandle);
    VOID SetExecute(HANDLE hFile);
    VOID SetRead(HANDLE hFile, DWORD cbData);
    VOID SetWrite(HANDLE hFile, DWORD cbData);
    VOID SetStdio(HANDLE hFile);
    PFileInfo RecallFile(HANDLE hFile) const NOEXCEPT;
    PProcInfo RecallProc(HANDLE hProc) const NOEXCEPT;
    BOOL Remember(HANDLE hFile, PFileInfo pInfo);
    BOOL Remember(HANDLE hProc, PProcInfo pInfo);
    BOOL Duplicate(HANDLE hDst, HANDLE hSrc);

    // environment vars
    struct FEnvVar {
        BOOL bRead{ false };
        DWORD nCount{ 0 };
    };

    VOID UseEnvVar(PCWSTR pwzVarname);
    VOID UseEnvVar(PCSTR pwzVarname);

    // path helpers
    static VOID StringCopy(PWCHAR pwzDst, PCWSTR pwzSrc) NOEXCEPT;
    static VOID StringCopy(PCHAR pszDst, PCSTR pszSrc) NOEXCEPT;
    static DWORD StringLen(PCWSTR pwzSrc) NOEXCEPT;
    static VOID EndInSlash(PWCHAR pwzPath) NOEXCEPT;
    static BOOL PrefixMatch(PCWSTR pwzPath, PCWSTR pwzPrefix) NOEXCEPT;
    static BOOL SuffixMatch(PCWSTR pwzPath, PCWSTR pwzSuffix) NOEXCEPT;

    // file access helpers
    VOID NoteRead(PCSTR psz);
    VOID NoteRead(PCWSTR pwz);

    VOID NoteWrite(PCSTR psz);
    VOID NoteWrite(PCWSTR pwz);

    VOID NoteExecute(HMODULE hModule);

    VOID NoteDelete(PCSTR psz);
    VOID NoteDelete(PCWSTR pwz);

    VOID NoteCleanup(PCSTR psz);
    VOID NoteCleanup(PCWSTR pwz);

private:
    FIODetouringFiles(PCWSTR pwzStdin, PCWSTR pwzStdout, PCWSTR pwzStderr);
    ~FIODetouringFiles();

    VOID InitSystemPaths_();
    VOID InitFileInfo_(FFileInfo& file, PPE::FConstWChar pwzPath) NOEXCEPT;

    using heap_type = PPE::TThreadSafe<PPE::TSlabHeap<PPE::FWin32GlobalAllocatorPtr>, PPE::EThreadBarrier::CriticalSection>;
    static PPE::FConstWChar AllocString_(const heap_type::FExclusiveLock& heap, PCWSTR pwzPath);

    heap_type _heap;

    struct FHandleHash_ {
        PPE::hash_t operator ()(HANDLE hHandle) const NOEXCEPT {
            return PPE::hash_size_t_constexpr(uintptr_t(hHandle));
        }
    };

    template <typename _Key, typename _Value, typename _KeyHash = PPE::Meta::THash<_Key>>
    using concurrent_hashmap_t = PPE::TConcurrentHashMap<_Key, _Value,
        _KeyHash,
        PPE::Meta::TEqualTo<_Key>,
        PPE::FWin32GlobalAllocatorPtr>;

    using files_type = concurrent_hashmap_t<FFileKey, FFileInfo>;
    files_type _files;
    std::atomic<DWORD> _nFiles;

    using procs_type = concurrent_hashmap_t<HANDLE, FProcInfo, FHandleHash_>;
    procs_type _procs;
    std::atomic<DWORD> _nProcs;

    using opens_type = concurrent_hashmap_t<HANDLE, FOpenFile, FHandleHash_>;
    opens_type _opens;

    using envs_type = concurrent_hashmap_t<FCaseInsensitiveConstChar, FEnvVar>;
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
