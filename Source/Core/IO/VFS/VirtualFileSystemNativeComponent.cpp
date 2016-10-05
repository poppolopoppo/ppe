#include "stdafx.h"

#include "VirtualFileSystemNativeComponent.h"

#include "VirtualFileSystemNativeStream.h"
#include "VirtualFileSystemTrie.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/FS/FileStat.h"
#include "IO/FS/FileSystemToken.h"
#include "IO/FS/FileSystemTrie.h"
#include "IO/FileSystem.h"
#include "IO/VirtualFileSystem.h"
#include "IO/Stream.h"

// _waccess()
#include <io.h>
#include <wchar.h>

// EnumerateFiles()
#ifdef OS_WINDOWS
#   include <windows.h>
#   include <tchar.h>
#   include <stdio.h>
#endif

#define NATIVE_ENTITYNAME_MAXSIZE 1024

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(FVirtualFileSystem, FVirtualFileSystemNativeComponent, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FWString SanitizeTarget_(FWString&& target) {
    static_assert(FileSystem::Separator == L'/', "invalid FileSystem::Separator");
    Assert(target.size());

    const wchar_t *p = target.c_str();
    while (nullptr != (p = StrChr(p, L'\\')) )
        *const_cast<wchar_t *>(p) = L'/';

    if (target.back() != L'/')
        target.insert(target.end(), L'/');

    return std::move(target);
}
//----------------------------------------------------------------------------
static FWString SanitizeTarget_(const FWString& target) {
    FWString result = target;
    return SanitizeTarget_(std::move(result));
}
//----------------------------------------------------------------------------
static void Unalias_(
    FWOCStrStream& oss,
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target) {
    Assert(alias.PathNode());
    Assert(aliased.PathNode());
    Assert(L'/' == target.back());

    oss << target;
    if (aliased.PathNode() == alias.PathNode())
        return;

    Assert(aliased.PathNode()->Depth() >= alias.PathNode()->Depth());
    const size_t length = aliased.PathNode()->Depth() - alias.PathNode()->Depth();
    if (length > 0) {
        STACKLOCAL_POD_ARRAY(FFileSystemToken, subpath, length);
        const size_t k = FFileSystemPath::Instance().Expand(subpath, alias.PathNode(), aliased.PathNode());

        for (size_t i = 0; i < k; ++i)
            oss << subpath[i] << wchar_t(FileSystem::Separator);
    }
}
//----------------------------------------------------------------------------
static void Unalias_(
    wchar_t *storage, size_t capacity,
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const wchar_t *suffix = nullptr) {
    FWOCStrStream oss(storage, capacity);
    Unalias_(oss, aliased, alias, target);
    if (suffix)
        oss << suffix;
}
//----------------------------------------------------------------------------
static void Unalias_(
    wchar_t *storage, size_t capacity,
    const FFilename& aliased,
    const FDirpath& alias, const FWString& target) {
    FWOCStrStream oss(storage, capacity);
    Unalias_(oss, aliased.Dirpath(), alias, target);
    oss << aliased.Basename();
}
//----------------------------------------------------------------------------
static size_t GlobFiles_Windows_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const std::function<void(const FFilename&)>& foreach,
    VECTOR_THREAD_LOCAL(FileSystem, FDirpath)& subDirectories,
    const wchar_t *pattern,
    bool recursive
    ) {
    WIN32_FIND_DATAW ffd;

    wchar_t nativeGlob[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeGlob, lengthof(nativeGlob), aliased, alias, target, pattern);

    HANDLE hFind = ::FindFirstFileExW(  nativeGlob,
                                        FindExInfoBasic,
                                        &ffd,
                                        FindExSearchNameMatch,
                                        NULL,
                                        FIND_FIRST_EX_LARGE_FETCH);

    if (INVALID_HANDLE_VALUE == hFind) {
        AssertNotReached();
        return 0;
    }

    size_t total = 0;

    do {
        const FileSystem::FStringView fname = MakeStringView(ffd.cFileName, Meta::noinit_tag());
        if (FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes) {
            if (!recursive)
                continue;

            if (2 == fname.size() && L'.' == fname[0] && L'.' == fname[1])
                continue;

            subDirectories.emplace_back(aliased, FDirname(fname));
        }
        else {
            foreach(FFilename(aliased, fname));
            ++total;
        }
    } while (::FindNextFileW(hFind, &ffd));

#ifdef WITH_CORE_ASSERT
    DWORD dwError = GetLastError();
    Assert(ERROR_NO_MORE_FILES == dwError);
#endif

    ::FindClose(hFind);
    return total;
}
//----------------------------------------------------------------------------
static size_t GlobFiles_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& destination,
    const std::function<void(const FFilename&)>& foreach,
    const FWStringView& pattern,
    bool recursive
    ) {
    VECTOR_THREAD_LOCAL(FileSystem, FDirpath) subDirectories;
    if (recursive)
        subDirectories.reserve(32);

    subDirectories.push_back(aliased);

    size_t total = 0;

    do {
        FDirpath dirpath(std::move(subDirectories.back()) );
        subDirectories.pop_back();

#ifdef OS_WINDOWS
        total += GlobFiles_Windows_(
            dirpath,
            alias, destination,
            foreach,
            subDirectories,
            pattern.Pointer(),
            recursive );
#else
        // TODO (12/13) : no uniform support
#   error "TODO"
#endif
    } while (subDirectories.size());

    return total;
}
//----------------------------------------------------------------------------
static bool EntityExists_(const wchar_t *nativeName, ExistPolicy::EMode policy) {
    Assert(nativeName);

    static_assert(0 == ExistPolicy::Exists, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");
    static_assert(2 == ExistPolicy::WriteOnly, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");
    static_assert(4 == ExistPolicy::ReadOnly, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");
    static_assert(6 == ExistPolicy::ReadWrite, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");

    return 0 == ::_waccess(nativeName, policy);
}
//----------------------------------------------------------------------------
static bool TryCreateDirectory_(const wchar_t *nativeDirpath) {
    if (::CreateDirectoryW(nativeDirpath, NULL))
        return true;

    const DWORD dwError = GetLastError();
    Assert(ERROR_ALREADY_EXISTS == dwError);

    return (ERROR_ALREADY_EXISTS == dwError);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::FVirtualFileSystemNativeComponent(const FDirpath& alias, FWString&& target, EOpenMode mode /* = EMode::ReadWritable */)
:   FVirtualFileSystemComponent(alias)
,   _mode(mode), _target(SanitizeTarget_(std::move(target)) ) {
    Assert(!_target.empty());
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::FVirtualFileSystemNativeComponent(const FDirpath& alias, const FWString& target, EOpenMode mode /* = EMode::ReadWritable */)
:   FVirtualFileSystemComponent(alias)
,   _mode(mode), _target(SanitizeTarget_(target)) {
    Assert(!_target.empty());
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::~FVirtualFileSystemNativeComponent() {}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentReadable* FVirtualFileSystemNativeComponent::Readable() {
    return (ModeReadable & _mode) ? this : nullptr;
}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentWritable* FVirtualFileSystemNativeComponent::Writable() {
    return (ModeWritable & _mode) ? this : nullptr;
}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentReadWritable* FVirtualFileSystemNativeComponent::ReadWritable() {
    return (ModeReadWritable & _mode) ? this : nullptr;
}
//----------------------------------------------------------------------------
FWString FVirtualFileSystemNativeComponent::Unalias(const FFilename& aliased) const {
    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, lengthof(nativeDirpath), aliased, _alias, _target);
    return nativeDirpath;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::DirectoryExists(const FDirpath& dirpath, ExistPolicy::EMode policy) {
    Assert(ModeReadable & _mode);

    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, lengthof(nativeDirpath), dirpath, _alias, _target);

    return EntityExists_(nativeDirpath, policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileExists(const FFilename& filename, ExistPolicy::EMode policy) {
    Assert(ModeReadable & _mode);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, lengthof(nativeFilename), filename, _alias, _target);

    return EntityExists_(nativeFilename, policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileStats(FFileStat* pstat, const FFilename& filename) {
    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, lengthof(nativeFilename), filename, _alias, _target);
    return FFileStat::FromNativePath(pstat, nativeFilename);
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::EnumerateFiles(const FDirpath& dirpath, bool recursive, const std::function<void(const FFilename&)>& foreach) {
    Assert(ModeReadable & _mode);
    return GlobFiles_(dirpath, _alias, _target, foreach, L"*", recursive);
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const std::function<void(const FFilename&)>& foreach) {
    Assert(ModeReadable & _mode);
    Assert(pattern.size());
    return GlobFiles_(dirpath, _alias, _target, foreach, pattern, recursive);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> FVirtualFileSystemNativeComponent::OpenReadable(const FFilename& filename, AccessPolicy::EMode policy) {
    Assert(ModeReadable & _mode);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, lengthof(nativeFilename), filename, _alias, _target);
    LOG(Info, L"[VFS] OpenNativeReadable('{0}')", nativeFilename);

    TUniquePtr<IVirtualFileSystemIStream> result;
    FVirtualFileSystemNativeFileIStream tmp(filename, nativeFilename, policy);
    if (false == tmp.Bad())
        result.reset(new FVirtualFileSystemNativeFileIStream(std::move(tmp)));

    return result;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::TryCreateDirectory(const FDirpath& dirpath) {
    Assert(ModeWritable & _mode);

    bool result = false;

    STACKLOCAL_WOCSTRSTREAM(oss, NATIVE_ENTITYNAME_MAXSIZE);
    oss << _target;
    result |= TryCreateDirectory_(oss.NullTerminatedStr());

    Assert(dirpath.PathNode());

    STACKLOCAL_POD_ARRAY(FFileSystemToken, subpath, FDirpath::MaxDepth);

    const size_t k = FFileSystemPath::Instance().Expand(subpath, _alias.PathNode(), dirpath.PathNode());

    for (size_t i = 0; i < k; ++i) {
        oss.RemoveEOS();
        oss << subpath[i] << std::char_traits<wchar_t>::to_char_type(FileSystem::Separator);
        result |= TryCreateDirectory_(oss.NullTerminatedStr());
    }

    return result;
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> FVirtualFileSystemNativeComponent::OpenWritable(const FFilename& filename, AccessPolicy::EMode policy) {
    Assert(ModeWritable & _mode);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, lengthof(nativeFilename), filename, _alias, _target);
    LOG(Info, L"[VFS] OpenNativeWritable('{0}')", nativeFilename);

    TUniquePtr<IVirtualFileSystemOStream> result;
    FVirtualFileSystemNativeFileOStream tmp(filename, nativeFilename, policy);
    if (false == tmp.Bad())
        result.reset(new FVirtualFileSystemNativeFileOStream(std::move(tmp)));

    return result;
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIOStream> FVirtualFileSystemNativeComponent::OpenReadWritable(const FFilename&/* filename */, AccessPolicy::EMode/* policy */) {
    Assert(ModeReadWritable & _mode);

    // TODO (12/13) : not supported
    AssertNotImplemented();

    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
