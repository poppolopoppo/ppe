#include "IODetouringFiles.h"

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

        BOOL bIgnoreFile = false;
        bIgnoreFile |= (file.bDirectory); // ignore directories
        bIgnoreFile |= (file.bDelete | file.bCleanup); // ignore files deleted
        bIgnoreFile |= (file.bTemporaryFile | file.bTemporaryPath); // ignore temporary files
        bIgnoreFile |= (file.bPipe | file.bStdio); // ignore pipes and stdio handles
        bIgnoreFile |= (file.bAbsorbed); // ignore response files
        bIgnoreFile |= not (file.bRead|file.bWrite|file.bDelete|file.bCleanup); // ignore do "none" files

        if (not bIgnoreFile) {
            const PCSTR ppzFlags[4] = {"--", "R-", "-W", "RW"};
            Tblog.Printf("%ls,%s\n", file.pwzPath, ppzFlags[((file.bRead ? 1 : 0) | (file.bWrite ? 2 : 0))]);
        }
    });
}
//----------------------------------------------------------------------------
// Filenames:
//----------------------------------------------------------------------------
auto FIODetouringFiles::FindFull(PCWSTR pwzPath) -> PFileInfo {
    const FFileKey key{pwzPath};
    const auto exclusiveHeap = _heap.LockExclusive();

    bool added = false;
    auto& it = _files.FindOrAdd(key, &added);

    if (added) {

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
auto FIODetouringFiles::FindPartial(PCSTR pwzPath) -> PFileInfo {
    WCHAR wzPath[MAX_PATH];
    PWCHAR pwzFile = wzPath;

    while (*pwzPath)
        *pwzFile++ = *pwzPath++;
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
    file.bStdio = (
        PrefixMatch(file.pwzPath, _pwzStdin) ||
        PrefixMatch(file.pwzPath, _pwzStdout) ||
        PrefixMatch(file.pwzPath, _pwzStderr) ||
        SuffixMatch(file.pwzPath, L"\\conout$") ||
        SuffixMatch(file.pwzPath, L"\\conin$") ||
        SuffixMatch(file.pwzPath, L"\\nul"));
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
    bool added = false;
    auto& it = _procs.FindOrAdd(hProc, &added);

    if (added) {
        if (it.second.hProc == INVALID_HANDLE_VALUE) {
            it.second.hProc = hProc;
            it.second.nProcId = nProcId;
            it.second.nIndex = _nProcs.fetch_add(1);
        }
    }

    return &it.second;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Close(HANDLE hProc) {
    return _procs.Erase(hProc);
}
//----------------------------------------------------------------------------
// Open files:
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Forget(HANDLE hHandle) {
    return _opens.Erase(hHandle);
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::SetRead(HANDLE hFile, DWORD cbData) {
    if (PFileInfo const pInfo = RecallFile(hFile)) {
        pInfo->bRead = true;
        pInfo->cbRead += cbData;
    }
}
//----------------------------------------------------------------------------
VOID FIODetouringFiles::SetWrite(HANDLE hFile, DWORD cbData) {
    if (PFileInfo const pInfo = RecallFile(hFile)) {
        pInfo->bWrite = true;
        pInfo->cbWrite += cbData;
    }
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::RecallFile(HANDLE hFile) const NOEXCEPT -> PFileInfo {
    if (auto* it = _opens.Lookup(hFile))
        return it->second.pFile;
    return nullptr;
}
//----------------------------------------------------------------------------
auto FIODetouringFiles::RecallProc(HANDLE hProc) const NOEXCEPT -> PProcInfo {
    if (auto* it = _opens.Lookup(hProc))
        return it->second.pProc;
    return nullptr;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Remember(HANDLE hFile, PFileInfo pInfo) {
    bool added = false;
    auto& it = _opens.FindOrAdd(hFile, &added);

    if (added) {
        it.second.hHandle = hFile;
        it.second.pFile = pInfo;
        it.second.pProc = nullptr;
    }

    return added;
}
//----------------------------------------------------------------------------
BOOL FIODetouringFiles::Remember(HANDLE hProc, PProcInfo pInfo) {
    bool added = false;
    auto& it = _opens.FindOrAdd(hProc, &added);

    if (added) {
        it.second.hHandle = hProc;
        it.second.pFile = nullptr;
        it.second.pProc = pInfo;
    }

    return added;
}
//----------------------------------------------------------------------------
// Environment variables:
//----------------------------------------------------------------------------
VOID FIODetouringFiles::UseEnvVar(PCWSTR pwzVarname) {
    FCaseInsensitiveConstChar key{ pwzVarname };

    const heap_type::FExclusiveLock exclusiveHeap{ _heap };

    bool added = false;
    envs_type::public_type& it = _envs.FindOrAdd(key, &added);

    if (added)
        const_cast<FCaseInsensitiveConstChar&>(it.first) = FCaseInsensitiveConstChar{
            AllocString_(exclusiveHeap, pwzVarname),
            key.Hash()};

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
// Path helpes:
//----------------------------------------------------------------------------
VOID FIODetouringFiles::StringCopy(PWCHAR pwzDst, PCWSTR pwzSrc) {
    while (*pwzSrc)
        *pwzDst++ = *pwzSrc++;
    *pwzDst = '\0';
}
//----------------------------------------------------------------------------
DWORD FIODetouringFiles::StringLen(PCWSTR pwzSrc) {
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
