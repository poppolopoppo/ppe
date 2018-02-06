#include "stdafx.h"

#include "VirtualFileSystemNativeComponent.h"

#include "VirtualFileSystemTrie.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/FS/FileStat.h"
#include "IO/FS/FileSystemToken.h"
#include "IO/FS/FileSystemTrie.h"
#include "IO/FileStream.h"
#include "IO/FileSystem.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "IO/VirtualFileSystem.h"
#include "Memory/MemoryProvider.h"
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

    target.gsub(L'\\', L'/');
    while (target.gsub(L"//", L"/"));

    if (target.back() != L'/')
        target.insert(target.end(), L'/');

    for (;;) {
        const size_t dotdot = target.find(L"/../");
        if (dotdot == FWString::npos)
            break;

        Assert(target[dotdot] == L'/');

        const size_t folder = target.rfind(L'/', dotdot - 1);
        AssertRelease(folder != FWString::npos);
        Assert(target[folder] == L'/');

        target.erase(target.begin() + folder, target.begin() + (dotdot + 3));
    }

    target.shrink_to_fit();

    return std::move(target);
}
//----------------------------------------------------------------------------
static FWString SanitizeTarget_(const FWString& target) {
    FWString result = target;
    return SanitizeTarget_(std::move(result));
}
//----------------------------------------------------------------------------
static void Unalias_(
    TBasicTextWriter<FileSystem::char_type>& oss,
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
            oss << subpath[i] << FileSystem::char_type(FileSystem::Separator);
    }
}
//----------------------------------------------------------------------------
static void Unalias_(
    TBasicTextWriter<FileSystem::char_type>& oss,
    const FFilename& aliased,
    const FDirpath& alias, const FWString& target) {
    Unalias_(oss, aliased.Dirpath(), alias, target);
    oss << aliased.Basename();
}
//----------------------------------------------------------------------------
static void Unalias_(
    const TMemoryView<FileSystem::char_type>& storage,
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const FileSystem::char_type *suffix = nullptr) {
    TBasicFixedSizeTextWriter<FileSystem::char_type> oss(storage);
    Unalias_(oss, aliased, alias, target);
    if (suffix)
        oss << suffix;
    oss << Eos;
}
//----------------------------------------------------------------------------
static void Unalias_(
    const TMemoryView<FileSystem::char_type>& storage,
    const FFilename& aliased,
    const FDirpath& alias, const FWString& target) {
    TBasicFixedSizeTextWriter<FileSystem::char_type> oss(storage);
    Unalias_(oss, aliased, alias, target);
    oss << Eos;
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
    const FileSystem::char_type *pattern,
    bool recursive
    ) {
    WIN32_FIND_DATAW ffd;

    FileSystem::char_type nativeGlob[NATIVE_ENTITYNAME_MAXSIZE];
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
        const auto fname = MakeCStringView(ffd.cFileName);
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
static bool EntityExists_(const FileSystem::char_type *nativeName, EExistPolicy policy) {
    Assert(nativeName);

    return FPlatformIO::Access(nativeName, policy);
}
//----------------------------------------------------------------------------
static bool CreateDirectory_(const FileSystem::char_type *nativeDirpath, bool& existed) {
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
static bool RemoveDirectory_(const FileSystem::char_type* nativeDirpath) {
    return (::RemoveDirectoryW(nativeDirpath));
}
//----------------------------------------------------------------------------
static bool RemoveFile_(const FileSystem::char_type* nativeFilename) {
    return (::DeleteFileW(nativeFilename));
}
//----------------------------------------------------------------------------
static bool MoveFile_(const FileSystem::char_type* nativeSrc, const FileSystem::char_type* nativeDst) {
    return (::MoveFileExW(
        nativeSrc,
        nativeDst,
        MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED));
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
    TBasicStringBuilder<FileSystem::char_type> nativeDirpath;
    Unalias_(nativeDirpath, aliased, _alias, _target);
    return nativeDirpath.ToString();
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::DirectoryExists(const FDirpath& dirpath, EExistPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, dirpath, _alias, _target);

    return EntityExists_(&nativeDirpath[0], policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileExists(const FFilename& filename, EExistPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    return EntityExists_(nativeFilename, policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileStats(FFileStat* pstat, const FFilename& filename) {
    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
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
UStreamReader FVirtualFileSystemNativeComponent::OpenReadable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    LOG(VFS, Debug, L"open native readable '{0}' : {1}", filename, policy);

    UStreamReader result;
    FFileStreamReader tmp = FFileStream::OpenRead(nativeFilename, policy);
    if (tmp.Good())
        result.reset(new FFileStreamReader(std::move(tmp)));

    return result;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::CreateDirectory(const FDirpath& dirpath) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type buffer[NATIVE_ENTITYNAME_MAXSIZE];
    FWFixedSizeTextWriter nativeDirpath(buffer);
    Unalias_(nativeDirpath, dirpath, _alias, _target);
    nativeDirpath << Eos;

    if (not EntityExists_(nativeDirpath.data(), EExistPolicy::Exists)) {
        bool existed = false;

        if (CreateDirectory_(nativeDirpath.data(), existed)) {
            LOG(VFS, Debug, L"create directory '{0}'", dirpath);
            return true;
        }

        const size_t l = nativeDirpath.size();
        forrange(i, _target.size() - 1, l + 1) {
            if (l == i || FileSystem::Separators().Contains(buffer[i])) {
                buffer[i] = L'\0';

                if (CreateDirectory_(nativeDirpath.data(), existed)) {
                    if (l != i)
                        buffer[i] = FileSystem::Separator;
                    if (not existed)
                        LOG(VFS, Debug, L"create directory '{0}'", MakeCStringView(buffer));
                }
                else {
                    LOG(VFS, Error, L"failed to create directory '{0}'", MakeCStringView(buffer));
                    return false;
                }
            }
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::MoveFile(const FFilename& src, const FFilename& dst) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeSrc[NATIVE_ENTITYNAME_MAXSIZE];
    FileSystem::char_type nativeDst[NATIVE_ENTITYNAME_MAXSIZE];

    Unalias_(nativeSrc, src, _alias, _target);
    Unalias_(nativeDst, dst, _alias, _target);
    LOG(VFS, Debug, L"move file '{0}' -> '{1}'", src, dst);

    return MoveFile_(nativeSrc, nativeDst);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveDirectory(const FDirpath& dirpath) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, dirpath, _alias, _target);
    LOG(VFS, Debug, L"remove directory '{0}'", dirpath);

    return RemoveDirectory_(nativeDirpath);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveFile(const FFilename& filename) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    LOG(VFS, Debug, L"remove file '{0}'", filename);

    return RemoveFile_(nativeFilename);
}
//----------------------------------------------------------------------------
UStreamWriter FVirtualFileSystemNativeComponent::OpenWritable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    UStreamWriter result;

    if (policy ^ EAccessPolicy::Create) {
        if (not CreateDirectory(filename.Dirpath()))
            return result;
    }

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    LOG(Debug, L"[VFS] OpenNativeWritable('{0}') : {1}", nativeFilename, policy);

    FFileStreamWriter tmp = FFileStream::OpenWrite(nativeFilename, policy);
    if (tmp.Good())
        result.reset(new FFileStreamWriter(std::move(tmp)));

    return result;
}
//----------------------------------------------------------------------------
UStreamReadWriter FVirtualFileSystemNativeComponent::OpenReadWritable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode == EOpenPolicy::ReadWritable);

    UStreamReadWriter result;

    if (policy ^ EAccessPolicy::Create) {
        if (not CreateDirectory(filename.Dirpath()))
            return result;
    }

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    LOG(Debug, L"[VFS] OpenNativeReadWritable('{0}') : {1}", nativeFilename, policy);

    FFileStreamReadWriter tmp = FFileStream::OpenReadWrite(nativeFilename, policy);
    if (tmp.Good())
        result.reset(new FFileStreamReadWriter(std::move(tmp)));

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
