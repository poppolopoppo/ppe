#pragma once

#include "IODetouring.h"

#include "Allocator/SlabHeap.h"
#include "HAL/Windows/GlobalAllocator.h"

#include "IO/ConstChar.h"
#include "Thread/ConcurrentHashMap.h"
#include "Thread/ThreadSafe.h"

#include <atomic>

#include "IODetouringTblog.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Log all file accesses (read & write), can also alias files
//----------------------------------------------------------------------------
class FIODetouringFiles {
public:
    PPE_IODETOURING_API static void Create(const FIODetouringPayload& payload);
    PPE_IODETOURING_API static void Destroy();
    NODISCARD PPE_IODETOURING_API static FIODetouringFiles& Get() NOEXCEPT;

    PPE_IODETOURING_API VOID Dump();

    // filenames
    struct FFileFlags {
        bool    bCantRead : 1;        // Set for file that are opened Create
        bool    bCantWrite : 1;       // Set for file that are opened ReadOnly
        bool    bRead : 1;
        bool    bWrite : 1;
        bool    bExecute : 1;

        bool    bDelete : 1;
        bool    bCleanup : 1;
        bool    bSystemPath : 1;
        bool    bTemporaryPath : 1;
        bool    bTemporaryFile : 1;
        bool    bPipe : 1;
        bool    bStdio : 1;
        bool    bVolume : 1;

        bool    bAppend : 1;
        bool    bAbsorbed : 1;        // Absorbed by TraceBld.
        bool    bDirectory : 1;

        FFileFlags() {
            ZeroMemory(this, sizeof(*this));
        }
    };

    struct FFileInfo : FFileFlags {
        PCWSTR  pwzInputPath{ nullptr };
        PCWSTR  pwzRealPath{ nullptr };
        PCSTR   pszRealPath{ nullptr };
        PBYTE   pbContent{ nullptr };

        SIZE_T  cbRead{ 0 };
        SIZE_T  cbWrite{ 0 };
        SIZE_T  cbContent{ 0 };

        DWORD   nIndex{ 0 };

        bool WasMounted() const { return (pwzRealPath != pwzInputPath); }
    };
    using PFileInfo = FFileInfo*;

    using FCaseInsensitiveConstChar = PPE::THashMemoizer<PPE::FConstWChar, // FS is case insensitive of Windows
        PPE::TConstCharHasher<wchar_t, PPE::ECase::Insensitive>,
        PPE::TConstCharEqualTo<wchar_t, PPE::ECase::Insensitive>>;

    using FFileKey = FCaseInsensitiveConstChar;
    class FLazyCopyFileKey;

    NODISCARD PPE_IODETOURING_API PFileInfo FindFull(PCWSTR pwzPath);
    NODISCARD PPE_IODETOURING_API PFileInfo FindPartial(PCWSTR pwzPath);
    NODISCARD PPE_IODETOURING_API PFileInfo FindPartial(PCSTR pszPath);

    // processes
    struct FProcInfo {
        HANDLE  hProc{ INVALID_HANDLE_VALUE };
        DWORD   nProcId{ 0 };
        DWORD   nIndex{ 0 };
    };
    using PProcInfo = FProcInfo*;

    NODISCARD PPE_IODETOURING_API PProcInfo CreateProc(HANDLE hProc, DWORD nProcId);
    NODISCARD PPE_IODETOURING_API BOOL Close(HANDLE hProc);

    NODISCARD PPE_IODETOURING_API BOOL ShouldDetourApplication(PCWSTR pwzPath) const NOEXCEPT;
    NODISCARD PPE_IODETOURING_API BOOL ShouldDetourApplication(PCSTR pszPath) const NOEXCEPT;

    // file handles
    struct FOpenFile {
        HANDLE hHandle{ INVALID_HANDLE_VALUE };
        PFileInfo pFile{ nullptr };
        PProcInfo pProc{ nullptr };
    };

    PPE_IODETOURING_API BOOL Forget(HANDLE hHandle);
    PPE_IODETOURING_API VOID SetExecute(HANDLE hFile);
    PPE_IODETOURING_API VOID SetRead(HANDLE hFile, DWORD cbData);
    PPE_IODETOURING_API VOID SetWrite(HANDLE hFile, DWORD cbData);
    PPE_IODETOURING_API VOID SetStdio(HANDLE hFile);
    PPE_IODETOURING_API PFileInfo RecallFile(HANDLE hFile) const NOEXCEPT;
    PPE_IODETOURING_API PProcInfo RecallProc(HANDLE hProc) const NOEXCEPT;
    PPE_IODETOURING_API BOOL Remember(HANDLE hFile, PFileInfo pInfo);
    PPE_IODETOURING_API BOOL Remember(HANDLE hProc, PProcInfo pInfo);
    PPE_IODETOURING_API BOOL Duplicate(HANDLE hDst, HANDLE hSrc);

    // environment vars
    struct FEnvVar {
        BOOL bRead{ false };
        DWORD nCount{ 0 };
    };

    PPE_IODETOURING_API VOID UseEnvVar(PCWSTR pwzVarname);
    PPE_IODETOURING_API VOID UseEnvVar(PCSTR pwzVarname);

    // path helpers
    PPE_IODETOURING_API static PWCHAR StringCopy(PWCHAR pwzDst, PCSTR pszSrc) NOEXCEPT;
    PPE_IODETOURING_API static PCHAR StringCopy(PCHAR pwzDst, PCWSTR pwzSrc) NOEXCEPT;
    PPE_IODETOURING_API static PWCHAR StringCopy(PWCHAR pwzDst, PCWSTR pwzSrc) NOEXCEPT;
    PPE_IODETOURING_API static PCHAR StringCopy(PCHAR pszDst, PCSTR pszSrc) NOEXCEPT;

    NODISCARD PPE_IODETOURING_API static DWORD StringLen(PCWSTR pwzSrc) NOEXCEPT;
    PPE_IODETOURING_API static VOID EndInSlash(PWCHAR pwzPath) NOEXCEPT;
    NODISCARD PPE_IODETOURING_API static BOOL PrefixMatch(PCWSTR pwzPath, PCWSTR pwzPrefix) NOEXCEPT;
    NODISCARD PPE_IODETOURING_API static BOOL SuffixMatch(PCWSTR pwzPath, PCWSTR pwzSuffix) NOEXCEPT;

    // file access helpers
    PPE_IODETOURING_API VOID NoteRead(PCSTR psz);
    PPE_IODETOURING_API VOID NoteRead(PCWSTR pwz);

    PPE_IODETOURING_API VOID NoteWrite(PCSTR psz);
    PPE_IODETOURING_API VOID NoteWrite(PCWSTR pwz);

    PPE_IODETOURING_API VOID NoteExecute(HMODULE hModule);

    PPE_IODETOURING_API VOID NoteDelete(PCSTR psz);
    PPE_IODETOURING_API VOID NoteDelete(PCWSTR pwz);

    PPE_IODETOURING_API VOID NoteCleanup(PCSTR psz);
    PPE_IODETOURING_API VOID NoteCleanup(PCWSTR pwz);

    // https://learn.microsoft.com/en-us/dotnet/standard/io/file-path-formats#unc-paths
    STATIC_CONST_INTEGRAL(u32, MaxPath, 32000);

private:
    FIODetouringFiles(const FIODetouringPayload& payload);
    ~FIODetouringFiles();

    VOID InitSystemPaths_(const FIODetouringPayload& payload);
    VOID InitFileInfo_(FFileInfo& file, PPE::FConstWChar pwzPath) NOEXCEPT;

    using heap_type = PPE::TThreadSafe<PPE::TSlabHeap<PPE::FWin32GlobalAllocatorPtr>, PPE::EThreadBarrier::CriticalSection>;
    static PPE::FConstWChar AllocString_(const heap_type::FExclusiveLock& heap, PCWSTR pwzPath);
    static PPE::FConstChar AllocString_(const heap_type::FExclusiveLock& heap, PCSTR pszPath);

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

    PPE::TMemoryView<PCWSTR> _pzzIgnoredApplications;

    struct FMountedPath_ {
        PCWSTR pzInputPath;
        PCWSTR pzRealPath;
    };

    PPE::TMemoryView<FMountedPath_> _pzzMountedPaths;

    WCHAR _wzSysPath[MaxPath];
    WCHAR _wzS64Path[MaxPath];
    WCHAR _wzTmpPath[MaxPath];
    WCHAR _wzExePath[MaxPath];
    DWORD _wcSysPath{0};
    DWORD _wcS64Path{0};
    DWORD _wcTmpPath{0};
    DWORD _wcExePath{0};

    PCWSTR _pwzStdin = L"";
    PCWSTR _pwzStdout = L"";
    PCWSTR _pwzStderr = L"";

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
