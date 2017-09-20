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
#include "Misc/TargetPlatform.h"

// EnumerateFiles()
#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#   include <tchar.h>
#   include <stdio.h>
#endif

#define NATIVE_ENTITYNAME_MAXSIZE 1024

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(VirtualFileSystem, FVirtualFileSystemNativeComponent, );
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
static FWStringView Unalias_(
    const TMemoryView<wchar_t>& storage,
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const wchar_t *suffix = nullptr) {
    FWOCStrStream oss(storage);
    Unalias_(oss, aliased, alias, target);
    if (suffix)
        oss << suffix;
    return oss.MakeView();
}
//----------------------------------------------------------------------------
static FWStringView Unalias_(
    const TMemoryView<wchar_t>& storage,
    const FFilename& aliased,
    const FDirpath& alias, const FWString& target) {
    FWOCStrStream oss(storage);
    Unalias_(oss, aliased.Dirpath(), alias, target);
    oss << aliased.Basename();
    return oss.MakeView();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Windows platform
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
namespace {
//----------------------------------------------------------------------------
static size_t GlobFiles_Windows_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const Meta::TFunction<void(const FFilename&)>& foreach,
    VECTOR_THREAD_LOCAL(FileSystem, FDirpath)& subDirectories,
    const wchar_t *pattern,
    bool recursive
    ) {
    WIN32_FIND_DATAW ffd;

    wchar_t nativeGlob[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeGlob, aliased, alias, target, pattern);

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
        const FileSystem::FStringView fname = MakeStringView(ffd.cFileName, Meta::FForceInit{});
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
static bool EntityExists_(const wchar_t *nativeName, EExistPolicy policy) {
    Assert(nativeName);

    return FPlatformIO::Access(nativeName, policy);
}
//----------------------------------------------------------------------------
static bool CreateDirectory_(const wchar_t *nativeDirpath, bool& existed) {
    if (::CreateDirectoryW(nativeDirpath, NULL)) {
        existed = false;
        return true;
    }
    else if (::GetLastError() == ERROR_ALREADY_EXISTS) {
        existed = true;
        return true;
    }
    else {
        existed = false;
        return false;
    }
}
//----------------------------------------------------------------------------
static bool RemoveDirectory_(const wchar_t* nativeDirpath) {
    return (::RemoveDirectoryW(nativeDirpath));
}
//----------------------------------------------------------------------------
static bool RemoveFile_(const wchar_t* nativeFilename) {
    return (::DeleteFileW(nativeFilename));
}
//----------------------------------------------------------------------------
} //!namespace
#endif //!PLATFORM_WINDOWS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static size_t GlobFiles_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& destination,
    const Meta::TFunction<void(const FFilename&)>& foreach,
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

#ifdef PLATFORM_WINDOWS
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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::FVirtualFileSystemNativeComponent(const FDirpath& alias, FWString&& target, EOpenPolicy openMode /* = EOpenPolicy::ReadWritable */)
:   FVirtualFileSystemComponent(alias)
,   _openMode(openMode), _target(SanitizeTarget_(std::move(target)) ) {
    Assert(!_target.empty());
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::FVirtualFileSystemNativeComponent(const FDirpath& alias, const FWString& target, EOpenPolicy openMode /* = EOpenPolicy::ReadWritable */)
:   FVirtualFileSystemComponent(alias)
,   _openMode(openMode), _target(SanitizeTarget_(target)) {
    Assert(!_target.empty());
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::~FVirtualFileSystemNativeComponent() {}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentReadable* FVirtualFileSystemNativeComponent::Readable() {
    return (_openMode ^ EOpenPolicy::Readable ? this : nullptr);
}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentWritable* FVirtualFileSystemNativeComponent::Writable() {
    return (_openMode ^ EOpenPolicy::Writable ? this : nullptr);
}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentReadWritable* FVirtualFileSystemNativeComponent::ReadWritable() {
    return (_openMode ^ EOpenPolicy::ReadWritable ? this : nullptr);
}
//----------------------------------------------------------------------------
FWString FVirtualFileSystemNativeComponent::Unalias(const FFilename& aliased) const {
    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, aliased, _alias, _target);
    return nativeDirpath;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::DirectoryExists(const FDirpath& dirpath, EExistPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, dirpath, _alias, _target);

    return EntityExists_(nativeDirpath, policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileExists(const FFilename& filename, EExistPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    return EntityExists_(nativeFilename, policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileStats(FFileStat* pstat, const FFilename& filename) {
    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    return FFileStat::FromNativePath(pstat, nativeFilename);
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::EnumerateFiles(const FDirpath& dirpath, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach) {
    Assert(_openMode ^ EOpenPolicy::Readable);
    return GlobFiles_(dirpath, _alias, _target, foreach, L"*", recursive);
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach) {
    Assert(_openMode ^ EOpenPolicy::Readable);
    Assert(pattern.size());
    return GlobFiles_(dirpath, _alias, _target, foreach, pattern, recursive);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIStream> FVirtualFileSystemNativeComponent::OpenReadable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    LOG(Debug, L"[VFS] OpenNativeReadable('{0}')", nativeFilename);

    TUniquePtr<IVirtualFileSystemIStream> result;
    FVirtualFileSystemNativeFileIStream tmp(filename, nativeFilename, policy);
    if (false == tmp.Bad())
        result.reset(new FVirtualFileSystemNativeFileIStream(std::move(tmp)));

    return result;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::CreateDirectory(const FDirpath& dirpath) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    bool result = false;

    wchar_t buffer[NATIVE_ENTITYNAME_MAXSIZE];
    const FWStringView nativeDirpath = Unalias_(buffer, dirpath, _alias, _target);

    if (not EntityExists_(nativeDirpath.data(), EExistPolicy::Exists)) {
        bool existed = false;

        if (CreateDirectory_(buffer, existed)) {
            LOG(Debug, L"[VFS] CreateDirectory('{0}')", nativeDirpath);
            return true;
        }

        forrange(i, _target.size() - 1, nativeDirpath.size() + 1) {
            if (nativeDirpath.size() == i || FileSystem::Separators().Contains(nativeDirpath[i])) {
                buffer[i] = L'\0';

                if (CreateDirectory_(buffer, existed)) {
                    if (nativeDirpath.size() != i)
                        buffer[i] = FileSystem::Separator;
                    if (not existed)
                        LOG(Debug, L"[VFS] CreateDirectory('{0}')", nativeDirpath.CutBefore(i));
                }
                else {
                    LOG(Error, L"[VFS] Failed to create directory '{0}'", nativeDirpath.CutBefore(i));
                    return false;
                }
            }
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveDirectory(const FDirpath& dirpath) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, dirpath, _alias, _target);
    LOG(Debug, L"[VFS] RemoveNaticeDirectory('{0}')", nativeDirpath);

    return RemoveDirectory_(nativeDirpath);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveFile(const FFilename& filename) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    LOG(Debug, L"[VFS] RemoveNativeFile('{0}')", nativeFilename);

    return RemoveFile_(nativeFilename);
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemOStream> FVirtualFileSystemNativeComponent::OpenWritable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    TUniquePtr<IVirtualFileSystemOStream> result;

    if (policy ^ EAccessPolicy::Create) {
        if (not CreateDirectory(filename.Dirpath()))
            return result;
    }

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    LOG(Debug, L"[VFS] OpenNativeWritable('{0}')", nativeFilename);

    FVirtualFileSystemNativeFileOStream tmp(filename, nativeFilename, policy);
    if (false == tmp.Bad())
        result.reset(new FVirtualFileSystemNativeFileOStream(std::move(tmp)));

    return result;
}
//----------------------------------------------------------------------------
TUniquePtr<IVirtualFileSystemIOStream> FVirtualFileSystemNativeComponent::OpenReadWritable(const FFilename&/* filename */, EAccessPolicy/* policy */) {
    Assert(_openMode == EOpenPolicy::ReadWritable);

    // TODO (12/13) : not supported/required atm
    AssertNotImplemented();

    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
