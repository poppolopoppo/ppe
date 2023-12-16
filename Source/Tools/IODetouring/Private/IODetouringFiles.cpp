#include "IODetouringFiles.h"

#include <strsafe.h>

#include "IODetouringDebug.h"
#include "IODetouringTblog.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static POD_STORAGE(FIODetouringFiles) GIODetouringFilesStorage_;
//----------------------------------------------------------------------------
// Log all file accesses (read & write), can also alias files
//----------------------------------------------------------------------------
FIODetouringFiles::FIODetouringFiles(const FIODetouringPayload& payload)
:   _files(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<files_type::FEntry*>(4049))
,   _procs(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<procs_type::FEntry*>(4049))
,   _opens(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<opens_type::FEntry*>(4049))
,   _envs(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<envs_type::FEntry*>(319))
,   _pwzStdin(payload.wzStdin)
,   _pwzStdout(payload.wzStdout)
,   _pwzStderr(payload.wzStderr)
{
    InitSystemPaths_(payload);
}
//----------------------------------------------------------------------------
FIODetouringFiles::~FIODetouringFiles() {
    const auto exclusiveHeap = _heap.LockExclusive(); // lock for safety

    PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::DeallocateT(_files.Clear_ReleaseMemory());
    PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::DeallocateT(_procs.Clear_ReleaseMemory());
    PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::DeallocateT(_opens.Clear_ReleaseMemory());
    PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::DeallocateT(_envs.Clear_ReleaseMemory());

    PPE::Unused(exclusiveHeap);
}

//----------------------------------------------------------------------------
void FIODetouringFiles::Create(const FIODetouringPayload& payload) {
    new (static_cast<void*>(&GIODetouringFilesStorage_)) FIODetouringFiles(payload);
}
//----------------------------------------------------------------------------
FIODetouringFiles& FIODetouringFiles::Get() NOEXCEPT {
    return (*reinterpret_cast<FIODetouringFiles*>(&GIODetouringFilesStorage_));
}
//----------------------------------------------------------------------------
void FIODetouringFiles::Destroy() {
    reinterpret_cast<FIODetouringFiles*>(&GIODetouringFilesStorage_)->~FIODetouringFiles();
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::Dump() {
    FIODetouringTblog& Tblog = FIODetouringTblog::Get();

    _files.Foreach([&Tblog](const files_type::public_type& it) {
        const FFileInfo& file = it.second;
        const EIODetouringOptions nOptions = Tblog.Payload().nPayloadOptions;

        bool bIgnoreFile = false;
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreDirectory) && (file.bDirectory); // ignore directories
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreDelete) && (file.bDelete || file.bCleanup); // ignore files deleted
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreTemporary) && (file.bTemporaryFile || file.bTemporaryPath); // ignore temporary files
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreStdio) && (file.bPipe || file.bStdio); // ignore pipes and stdio handles
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreSystem) && (file.bSystemPath); // ignore system files
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreVolume) && (file.bVolume); // ignore \\.\\%c: system volume raw "files"
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreAbsorbed) && (file.bAbsorbed); // ignore response files
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreDoNone) && not (file.bRead || file.bWrite || file.bExecute); // ignore do "none" files

        if (not bIgnoreFile) {
            const CHAR nAccess = IODetouring_FileAccessToChar(
                file.bRead,
                file.bWrite,
                file.bExecute);
            Tblog.Printf("%c%ls", nAccess, file.pwzInputPath);
        }
    });
}
//----------------------------------------------------------------------------
// Filenames:
//----------------------------------------------------------------------------
class FIODetouringFiles::FLazyCopyFileKey {
public:
    FLazyCopyFileKey(FIODetouringFiles& owner, PCWSTR pwzPath)
        : _owner(owner), _transient(pwzPath)
    {}

    operator FFileKey() const {
        return AllocateKeyCopy();
    }

    FFileKey AllocateKeyCopy() const {
        const auto exclusiveHeap = _owner._heap.LockExclusive();
        // copy the key once we sure we need to add it
        return FFileKey{
            _owner.AllocString_(exclusiveHeap, _transient.Value()),
            _transient.Hash() };
    }

    friend bool operator==(const FFileKey& lhs, const FLazyCopyFileKey& rhs) {
        return (lhs == rhs._transient);
    }
    friend bool operator <(const FFileKey& lhs, const FLazyCopyFileKey& rhs) {
        return (lhs < rhs._transient);
    }

private:
    FIODetouringFiles& _owner;
    FFileKey _transient;
};
//----------------------------------------------------------------------------
auto FIODetouringFiles::FindFull(PCWSTR pwzPath) -> PFileInfo {
    const FLazyCopyFileKey key{*this, pwzPath};

    bool bAdded = false;
    auto& it = _files.FindOrAdd(key, &bAdded);

    if (bAdded) {
        // init basic file info properties with path
        InitFileInfo_(it.second, it.first);
    }

    return &it.second;
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::FindPartial(PCWSTR pwzPath) -> PFileInfo {
    WCHAR wzFullPath[MaxPath];
    PWCHAR pwzFile = NULL;

    if (!GetFullPathNameW(pwzPath, ARRAYSIZE(wzFullPath), wzFullPath, &pwzFile))
        return FindFull(pwzPath);
    else
        return FindFull(wzFullPath);
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::FindPartial(PCSTR pszPath) -> PFileInfo {
    WCHAR wzPath[MaxPath];
    StringCopy(wzPath, pszPath);

    return FindPartial(wzPath);
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::InitFileInfo_(FFileInfo& file, PPE::FConstWChar pwzPath) NOEXCEPT {
    file.pwzInputPath = pwzPath;
    file.pwzRealPath = pwzPath;
    file.nIndex = _nFiles.fetch_add(1);

    file.bSystemPath = (
        PrefixMatch(file.pwzInputPath, _wzSysPath) ||
        PrefixMatch(file.pwzInputPath, _wzS64Path));
    file.bTemporaryPath = PrefixMatch(file.pwzInputPath, _wzTmpPath);
    file.bTemporaryFile = SuffixMatch(file.pwzInputPath, L".tmp");
    file.bPipe = PrefixMatch(file.pwzInputPath, L"\\\\.\\pipe\\");
    file.bStdio = (
        PrefixMatch(file.pwzInputPath, _pwzStdin) ||
        PrefixMatch(file.pwzInputPath, _pwzStdout) ||
        PrefixMatch(file.pwzInputPath, _pwzStderr) ||
        SuffixMatch(file.pwzInputPath, L"\\conout$") ||
        SuffixMatch(file.pwzInputPath, L"\\conin$") ||
        SuffixMatch(file.pwzInputPath, L"\\nul"));

    if (PrefixMatch(file.pwzInputPath, L"\\\\.\\")) {
        if (SuffixMatch(file.pwzInputPath, L":"))
            file.bVolume = true;
    }

    for (const FMountedPath_& mount : _pzzMountedPaths) {
        if (SuffixMatch(pwzPath, mount.pzInputPath)) {
            const DWORD prefixLen = StringLen(mount.pzInputPath);

            WCHAR wzPath[MaxPath];
            StringCopy(StringCopy(wzPath, mount.pzRealPath), &pwzPath[prefixLen]);

            CHAR szPath[MaxPath];
            StringCopy(szPath, wzPath);

            const heap_type::FExclusiveLock exclusiveHeap{ _heap };
            file.pwzRealPath = AllocString_(exclusiveHeap, wzPath);
            file.pszRealPath = AllocString_(exclusiveHeap, szPath);

            IODETOURING_DEBUGPRINTF("mount '%ls' to '%ls'\n", file.pwzInputPath, file.pwzRealPath);
            break;
        }
    }

    IODETOURING_DEBUGPRINTF("found new file '%ls' (sys:%d, tmp:%d, pipe:%d, stdio:%d)\n",
        pwzPath,
        file.bSystemPath,
        file.bTemporaryFile||file.bTemporaryPath,
        file.bPipe,
        file.bStdio);
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::InitSystemPaths_(const FIODetouringPayload& payload) {
    _wzSysPath[0] = '\0';
    GetSystemDirectoryW(_wzSysPath, ARRAYSIZE(_wzSysPath));
    EndInSlash(_wzSysPath);

    _wzS64Path[0] = '\0';
    GetWindowsDirectoryW(_wzS64Path, ARRAYSIZE(_wzS64Path));
    EndInSlash(_wzS64Path);
    StringCopy(_wzS64Path + StringLen(_wzS64Path), L"SysWOW64\\");

    _wzTmpPath[0] = '\0';
    GetTempPathW(ARRAYSIZE(_wzTmpPath), _wzTmpPath);
    EndInSlash(_wzTmpPath);

    _wzExePath[0] = '\0';
    GetModuleFileNameW(NULL, _wzExePath, ARRAYSIZE(_wzExePath));
    PWCHAR pwzLast = _wzExePath;
    for (PWCHAR pwz = _wzExePath; *pwz; pwz++) {
        if (*pwz == '\\')
            pwzLast = pwz;
    }
    if (*pwzLast == '\\')
        *++pwzLast = '\0';

    _wcSysPath = StringLen(_wzSysPath);
    _wcS64Path = StringLen(_wzS64Path);
    _wcTmpPath = StringLen(_wzTmpPath);
    _wcExePath = StringLen(_wzExePath);

    u32 numIgnoredApplications = 0;
    for (PCWSTR pzzIgnoredApplication = payload.wzzIgnoredApplications; *pzzIgnoredApplication; ++numIgnoredApplications) {
        // pzzIgnoredApplication is null-terminated list of null-terminated strings
        while (*pzzIgnoredApplication++);
    }

    _pzzIgnoredApplications = _heap.LockExclusive()->AllocateT<PCWSTR>(numIgnoredApplications);

    numIgnoredApplications = 0;
    for (PCWSTR pzzIgnoredApplication = payload.wzzIgnoredApplications; *pzzIgnoredApplication; ++numIgnoredApplications) {
        // store the c-strings in an array for later
        _pzzIgnoredApplications[numIgnoredApplications] = pzzIgnoredApplication;
        while (*pzzIgnoredApplication++);
    }

    u32 numMountedPaths = 0;
    for (PCWSTR pzzMountedPath = payload.wzzMountedPaths; *pzzMountedPath; ++numMountedPaths) {
        // pzzMountedPath is null-terminated list of null-terminated strings
        while (*pzzMountedPath++);
    }

    _pzzMountedPaths = _heap.LockExclusive()->AllocateT<FMountedPath_>(numMountedPaths/2);

    numMountedPaths = 0;
    for (PCWSTR pzzMountedPath = payload.wzzMountedPaths; *pzzMountedPath; ++numMountedPaths) {
        // items must be grouped 2 by 2 in pairs
        FMountedPath_& pair = _pzzMountedPaths[numMountedPaths/2];
        if ((numMountedPaths % 2) == 0)
            pair.pzInputPath = pzzMountedPath;
        else
            pair.pzRealPath = pzzMountedPath;
        while (*pzzMountedPath++);
    }
}
//----------------------------------------------------------------------------
// Procs:
//----------------------------------------------------------------------------
auto FIODetouringFiles::CreateProc(HANDLE hProc, DWORD nProcId) -> PProcInfo {
    bool bAdded = false;
    if (hProc && hProc != INVALID_HANDLE_VALUE) {
        auto& it = _procs.FindOrAdd(hProc, FProcInfo{
            .hProc = hProc,
            .nProcId = nProcId,
            .nIndex = _nProcs.fetch_add(1),
        }, &bAdded);
        return &it.second;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Close(HANDLE hProc) {
    if (hProc && hProc != INVALID_HANDLE_VALUE)
        return _procs.Erase(hProc);
    return false;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::ShouldDetourApplication(PCWSTR pwzPath) const NOEXCEPT {
    for (PCWSTR pzIgnoredApplication : _pzzIgnoredApplications) {
        // check if this application was explicitly ignored in detours payload
        if (SuffixMatch(pwzPath, pzIgnoredApplication)) {
            IODETOURING_DEBUGPRINTF("ignore IO detouring on child process '%ls'\n", pwzPath);
            return FALSE;
        }
    }

    return TRUE;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::ShouldDetourApplication(PCSTR pszPath) const NOEXCEPT {
    WCHAR wzPath[MaxPath];
    StringCopy(wzPath, pszPath);

    return ShouldDetourApplication(wzPath);
}
//----------------------------------------------------------------------------
// Open files:
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Forget(HANDLE hHandle) {
    if (hHandle && hHandle != INVALID_HANDLE_VALUE)
        return _opens.Erase(hHandle);
    return false;
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::SetExecute(HANDLE hFile) {
    if (PFileInfo const pInfo = RecallFile(hFile)) {
        IODETOURING_DEBUGPRINTF("file '%ls' has been executed\n", pInfo->pwzPath);
        pInfo->bExecute = true;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::SetRead(HANDLE hFile, DWORD cbData) {
    if (PFileInfo const pInfo = RecallFile(hFile)) {
        IODETOURING_DEBUGPRINTF("file '%ls' has read %d bytes\n", pInfo->pwzPath, cbData);
        pInfo->bRead = true;
        pInfo->cbRead += cbData;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::SetWrite(HANDLE hFile, DWORD cbData) {
    if (PFileInfo const pInfo = RecallFile(hFile)) {
        IODETOURING_DEBUGPRINTF("file '%ls' has written %d bytes\n", pInfo->pwzPath, cbData);
        pInfo->bWrite = true;
        pInfo->cbWrite += cbData;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::SetStdio(HANDLE hFile) {
    if (PFileInfo const pInfo = RecallFile(hFile)) {
        IODETOURING_DEBUGPRINTF("file '%ls' was used as stdio\n", pInfo->pwzPath);
        pInfo->bStdio = true;
    }
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::RecallFile(HANDLE hFile) const NOEXCEPT -> PFileInfo {
    if (hFile && hFile != INVALID_HANDLE_VALUE) {
        if (auto* it = _opens.Lookup(hFile))
            return it->second.pFile;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::RecallProc(HANDLE hProc) const NOEXCEPT -> PProcInfo {
    if (hProc && hProc != INVALID_HANDLE_VALUE) {
        if (auto* it = _opens.Lookup(hProc))
            return it->second.pProc;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Remember(HANDLE hFile, PFileInfo pInfo) {
    bool bAdded = false;
    if (hFile && hFile != INVALID_HANDLE_VALUE) {
        _opens.FindOrAdd(hFile, FOpenFile{
            .hHandle = hFile,
            .pFile = pInfo,
            .pProc = nullptr,
        }, &bAdded);
    }
    return bAdded;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Remember(HANDLE hProc, PProcInfo pInfo) {
    bool bAdded = false;
    if (hProc && hProc != INVALID_HANDLE_VALUE) {
        _opens.FindOrAdd(hProc, FOpenFile{
            .hHandle = hProc,
            .pFile = nullptr,
            .pProc = pInfo,
        }, &bAdded);
    }
    return bAdded;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Duplicate(HANDLE hDst, HANDLE hSrc) {
    if (hSrc && hSrc != INVALID_HANDLE_VALUE) {
        if (auto* it = _opens.Lookup(hSrc)) {
            _opens.FindOrAdd(hDst, FOpenFile{ it->second });
            return TRUE;
        }
    }
    return FALSE;
}
//----------------------------------------------------------------------------
// Environment variables:
//----------------------------------------------------------------------------
VOID FIODetouringFiles::UseEnvVar(PCWSTR pwzVarname) {
    const FCaseInsensitiveConstChar key{ pwzVarname };

    bool added = false;
    envs_type::public_type& it = _envs.FindOrAdd(key, &added);

    if (added) {
        const_cast<FCaseInsensitiveConstChar&>(it.first) = FCaseInsensitiveConstChar{
            AllocString_(_heap.LockExclusive(), pwzVarname),
            key.Hash() };

        IODETOURING_DEBUGPRINTF("use environment variable '%ls'\n", pwzVarname);
    }

    it.second.bRead = true;
    InterlockedIncrement(&it.second.nCount);
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::UseEnvVar(PCSTR pszVarname) {
    WCHAR wzVarname[MaxPath];
    StringCopy(wzVarname, pszVarname);

    return UseEnvVar(wzVarname);
}
//----------------------------------------------------------------------------
// File access helpers:
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteRead(PCSTR psz) {
    if (PFileInfo const pInfo = FindPartial(psz)) {
        IODETOURING_DEBUGPRINTF("file '%s' note read\n", psz);
        pInfo->bRead = TRUE;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteRead(PCWSTR pwz) {
    if (PFileInfo const pInfo = FindPartial(pwz)) {
        IODETOURING_DEBUGPRINTF("file '%ls' note read\n", pwz);
        pInfo->bRead = TRUE;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteWrite(PCSTR psz) {
    if (PFileInfo const pInfo = FindPartial(psz)) {
        IODETOURING_DEBUGPRINTF("file '%s' note write\n", psz);
        pInfo->bWrite = TRUE;

        if (!pInfo->bRead) {
            pInfo->bCantRead = TRUE;
        }
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteWrite(PCWSTR pwz) {
    if (PFileInfo const pInfo = FindPartial(pwz)) {
        IODETOURING_DEBUGPRINTF("file '%ls' note write\n", pwz);
        pInfo->bWrite = TRUE;

        if (!pInfo->bRead) {
            pInfo->bCantRead = TRUE;
        }
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteExecute(HMODULE hModule) {
    // LoadLibraryA/W() is searching through %PATH%,
    // query actual Dll normalized path instead of trying to replicate this behavior
    // https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya#remarks

    TCHAR wzModulePath[MaxPath];
    const DWORD dwLen = GetModuleFileNameW(hModule, wzModulePath, ARRAYSIZE(wzModulePath));

    if (dwLen > 0 && dwLen <= ARRAYSIZE(wzModulePath)) {
        FIODetouringFiles& files = FIODetouringFiles::Get();

        const FIODetouringFiles::PFileInfo pFileInfo = files.FindFull(wzModulePath);
        pFileInfo->bExecute = TRUE;

        files.Remember(hModule, pFileInfo);

        IODETOURING_DEBUGPRINTF("module '%ls' note execute\n", pFileInfo->pwzPath);
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteDelete(PCSTR psz) {
    if (PFileInfo const pInfo = FindPartial(psz)) {
        IODETOURING_DEBUGPRINTF("file '%s' note delete\n", psz);
        if (pInfo->bWrite || pInfo->bRead)
            pInfo->bCleanup = TRUE;
        else
            pInfo->bDelete = TRUE;

        if (!pInfo->bRead)
            pInfo->bCantRead = TRUE;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteDelete(PCWSTR pwz) {
    if (PFileInfo const pInfo = FindPartial(pwz)) {
        IODETOURING_DEBUGPRINTF("file '%ls' note delete\n", pwz);
        if (pInfo->bWrite || pInfo->bRead)
            pInfo->bCleanup = TRUE;
        else
            pInfo->bDelete = TRUE;

        if (!pInfo->bRead)
            pInfo->bCantRead = TRUE;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteCleanup(PCSTR psz) {
    if (PFileInfo const pInfo = FindPartial(psz)) {
        IODETOURING_DEBUGPRINTF("file '%s' note cleanup\n", psz);
        pInfo->bCleanup = TRUE;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::NoteCleanup(PCWSTR pwz) {
    if (PFileInfo const pInfo = FindPartial(pwz)) {
        IODETOURING_DEBUGPRINTF("file '%ls' note cleanup\n", pwz);
        pInfo->bCleanup = TRUE;
    }
}
//----------------------------------------------------------------------------
// Path helpers: (do no want to be using any std libraries here)
//----------------------------------------------------------------------------
PWCHAR FIODetouringFiles::StringCopy(PWCHAR pwzDst, PCSTR pszSrc) NOEXCEPT {
    while (*pszSrc)
        *pwzDst++ = static_cast<WCHAR>(*pszSrc++);
    *pwzDst = '\0';
    return pwzDst;
}
//----------------------------------------------------------------------------
PCHAR FIODetouringFiles::StringCopy(PCHAR pszDst, PCWSTR pwzSrc) NOEXCEPT {
    while (*pwzSrc)
        *pszDst++ = static_cast<CHAR>(*pwzSrc++);
    *pszDst = '\0';
    return pszDst;
}
//----------------------------------------------------------------------------
PWCHAR FIODetouringFiles::StringCopy(PWCHAR pwzDst, PCWSTR pwzSrc) NOEXCEPT {
    while (*pwzSrc)
        *pwzDst++ = *pwzSrc++;
    *pwzDst = '\0';
    return pwzDst;
}
//----------------------------------------------------------------------------
PCHAR FIODetouringFiles::StringCopy(PCHAR pszDst, PCSTR pszSrc) NOEXCEPT {
    while (*pszSrc)
        *pszDst++ = *pszSrc++;
    *pszDst = '\0';
    return pszDst;
}
//----------------------------------------------------------------------------
DWORD FIODetouringFiles::StringLen(PCWSTR pwzSrc) NOEXCEPT {
    DWORD c = 0;
    while (pwzSrc[c])
        c++;
    return c;
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::EndInSlash(PWCHAR pwzPath) NOEXCEPT {
     if (*pwzPath) {
        while (*pwzPath)
            pwzPath++;

        if (pwzPath[-1] != '\\') {
            *pwzPath++ = '\\';
            *pwzPath = '\0';
        }
    }
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::PrefixMatch(PCWSTR pwzPath, PCWSTR pwzPrefix) NOEXCEPT {
    for (;;) {
        WCHAR cFile = *pwzPath++;
        WCHAR cPrefix = *pwzPrefix++;

        if (cFile >= 'A' && cFile <= 'Z')
            cFile += ('a' - 'A');
        if (cPrefix >= 'A' && cPrefix <= 'Z')
            cPrefix += ('a' - 'A');

        if (cPrefix == 0)
            return TRUE;
        if (cFile != cPrefix)
            return FALSE;
    }
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::SuffixMatch(PCWSTR pwzPath, PCWSTR pwzSuffix) NOEXCEPT {
    // Move both pointers to the end of the strings.
    const PCWSTR pwzFileBeg = pwzPath;
    while (*pwzPath)
        pwzPath++;

    const PCWSTR pwzSuffixBeg = pwzSuffix;
    while (*pwzSuffix)
        pwzSuffix++;

    // Now walk backwards comparing strings.
    for (;;) {
        WCHAR cFile = (pwzPath > pwzFileBeg) ? *--pwzPath : 0;
        WCHAR cSuffix = (pwzSuffix > pwzSuffixBeg) ? *--pwzSuffix : 0;

        if (cFile >= 'A' && cFile <= 'Z')
            cFile += ('a' - 'A');
        if (cSuffix >= 'A' && cSuffix <= 'Z')
            cSuffix += ('a' - 'A');

        if (cSuffix == 0)
            return TRUE;
        if (cFile != cSuffix)
            return FALSE;
    }
}
//----------------------------------------------------------------------------
PPE::FConstWChar FIODetouringFiles::AllocString_(const heap_type::FExclusiveLock& heap, PCWSTR pwzPath) {
    const PPE::FConstWChar in(pwzPath);
    const size_t len = in.length();

    PWSTR const pwzCopy = (PWSTR)heap->Allocate((len+1/* zero char */) * sizeof(wchar_t));
    StringCopy(pwzCopy, pwzPath);

    return { pwzCopy };
}
//----------------------------------------------------------------------------
PPE::FConstChar FIODetouringFiles::AllocString_(const heap_type::FExclusiveLock& heap, PCSTR pszPath) {
    const PPE::FConstChar in(pszPath);
    const size_t len = in.length();

    PSTR const pszCopy = (PSTR)heap->Allocate((len+1/* zero char */) * sizeof(char));
    StringCopy(pszCopy, pszPath);

    return { pszCopy };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
