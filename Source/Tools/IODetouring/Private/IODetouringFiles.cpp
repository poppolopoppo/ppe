#include "IODetouringFiles.h"

#include "IODetouringDebug.h"
#include "IODetouringTblog.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Log all file accesses (read & write), can also alias files
//----------------------------------------------------------------------------
FIODetouringFiles::FIODetouringFiles(PCWSTR pwzStdin, PCWSTR pwzStdout, PCWSTR pwzStderr)
: _files(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<files_type::FEntry*>(4049))
, _procs(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<procs_type::FEntry*>(4049))
, _opens(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<opens_type::FEntry*>(4049))
, _envs(PPE::TStaticAllocator<PPE::FWin32GlobalAllocatorPtr>::AllocateT<envs_type::FEntry*>(319))
, _pwzStdin(pwzStdin)
, _pwzStdout(pwzStdout)
, _pwzStderr(pwzStderr)
{
    InitSystemPaths_();
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
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreAbsorbed) && (file.bAbsorbed); // ignore response files
        bIgnoreFile |= (nOptions & EIODetouringOptions::IgnoreDoNone) && not (file.bRead || file.bWrite || file.bExecute); // ignore do "none" files

        if (not bIgnoreFile) {
            const CHAR nAccess = IODetouring_FileAccessToChar(
                file.bRead,
                file.bWrite,
                file.bExecute);
            Tblog.Printf("%c%ls", nAccess, file.pwzPath);
        }
    });
}
//----------------------------------------------------------------------------
// Filenames:
//----------------------------------------------------------------------------
auto FIODetouringFiles::FindFull(PCWSTR pwzPath) -> PFileInfo {
    const FFileKey key{pwzPath};
    const auto exclusiveHeap = _heap.LockExclusive();

    bool bAdded = false;
    auto& it = _files.FindOrAdd(key, &bAdded);

    if (bAdded) {

        if (it.first.Value().c_str() == pwzPath/* some other thread could have already allocated content */) {
            // copy the key once we sure we need to add it
            const_cast<FFileKey&>(it.first) = FFileKey{
                AllocString_(exclusiveHeap, pwzPath),
                it.first.Hash()};

            // init basic file info properties with path
            InitFileInfo_(it.second, it.first);
        }
    }

    return &it.second;
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::FindPartial(PCWSTR pwzPath) -> PFileInfo {
    WCHAR wzFullPath[MAX_PATH];
    PWCHAR pwzFile = NULL;

    if (!GetFullPathNameW(pwzPath, ARRAYSIZE(wzFullPath), wzFullPath, &pwzFile))
        return FindFull(pwzPath);
    else
        return FindFull(wzFullPath);
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::FindPartial(PCSTR pszPath) -> PFileInfo {
    WCHAR wzPath[MAX_PATH];
    PWCHAR pwzFile = wzPath;

    while (*pszPath)
        *pwzFile++ = *pszPath++;
    *pwzFile = '\0';

    return FindPartial(wzPath);
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::InitFileInfo_(FFileInfo& file, PPE::FConstWChar pwzPath) NOEXCEPT {
    file.pwzPath = pwzPath;
    file.nIndex = _nFiles.fetch_add(1);

    file.bSystemPath = (
        PrefixMatch(file.pwzPath, _wzSysPath) ||
        PrefixMatch(file.pwzPath, _wzS64Path));
    file.bTemporaryPath = PrefixMatch(file.pwzPath, _wzTmpPath);
    file.bTemporaryFile = SuffixMatch(file.pwzPath, L".tmp");
    file.bPipe = PrefixMatch(file.pwzPath, L"\\\\.\\pipe\\");
    file.bStdio = (
        PrefixMatch(file.pwzPath, _pwzStdin) ||
        PrefixMatch(file.pwzPath, _pwzStdout) ||
        PrefixMatch(file.pwzPath, _pwzStderr) ||
        SuffixMatch(file.pwzPath, L"\\conout$") ||
        SuffixMatch(file.pwzPath, L"\\conin$") ||
        SuffixMatch(file.pwzPath, L"\\nul"));

    IODETOURING_DEBUGPRINTF("found new file '%ls' (sys:%d, tmp:%d, pipe:%d, stdio:%d)\n",
        pwzPath,
        file.bSystemPath,
        file.bTemporaryFile||file.bTemporaryPath,
        file.bPipe,
        file.bStdio);
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::InitSystemPaths_() {
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
BOOL FIODetouringFiles::ShouldDetourApplication(PCWSTR pwzPath) NOEXCEPT {
    PCWSTR pzzIgnoredApplication = FIODetouringTblog::Get().Payload().wzzIgnoredApplications;

    while (*pzzIgnoredApplication) {
        // check if this application was explicitly ignored in detours payload
        if (SuffixMatch(pwzPath, pzzIgnoredApplication)) {
            IODETOURING_DEBUGPRINTF("ignore IO detouring on child process '%ls'\n", pwzPath);
            return FALSE;
        }

        // pzzIgnoredApplication is null-terminated list of null-terminated strings
        while (*pzzIgnoredApplication++);
    }

    return TRUE;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::ShouldDetourApplication(PCSTR pszPath) NOEXCEPT {
    WCHAR wzPath[MAX_PATH];
    PWCHAR pwzFile = wzPath;

    while (*pszPath)
        *pwzFile++ = *pszPath++;
    *pwzFile = '\0';

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

    const heap_type::FExclusiveLock exclusiveHeap{ _heap };

    bool added = false;
    envs_type::public_type& it = _envs.FindOrAdd(key, &added);

    if (added) {
        const_cast<FCaseInsensitiveConstChar&>(it.first) = FCaseInsensitiveConstChar{
            AllocString_(exclusiveHeap, pwzVarname),
            key.Hash() };

        IODETOURING_DEBUGPRINTF("use environment variable '%ls'\n", pwzVarname);
    }

    it.second.bRead = true;
    InterlockedIncrement(&it.second.nCount);
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::UseEnvVar(PCSTR pwzVarname) {
    WCHAR wzVarname[MAX_PATH];
    PWCHAR pwzPath = wzVarname;

    while (*pwzVarname)
        *pwzPath++ = *pwzVarname++;
    *pwzPath = L'\0';

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

    TCHAR wzModulePath[MAX_PATH];
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
// Path helpers:
//----------------------------------------------------------------------------
VOID FIODetouringFiles::StringCopy(PWCHAR pwzDst, PCWSTR pwzSrc) NOEXCEPT {
    while (*pwzSrc)
        *pwzDst++ = *pwzSrc++;
    *pwzDst = '\0';
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::StringCopy(PCHAR pszDst, PCSTR pszSrc) NOEXCEPT {
    while (*pszSrc)
        *pszDst++ = *pszSrc++;
    *pszDst = '\0';
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
    PCWSTR pwzFileBeg = pwzPath;
    while (*pwzPath)
        pwzPath++;

    PCWSTR pwzSuffixBeg = pwzSuffix;
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

    return PPE::FConstWChar(pwzCopy);
}
//----------------------------------------------------------------------------
void* FIODetouringFiles::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared lib
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------