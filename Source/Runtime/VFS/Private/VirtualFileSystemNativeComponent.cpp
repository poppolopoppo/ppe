// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VirtualFileSystemNativeComponent.h"

#include "VirtualFileSystem.h"
#include "VirtualFileSystemTrie.h"

#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformLowLevelIO.h"
#include "IO/FileSystemToken.h"
#include "IO/FileSystemTrie.h"
#include "IO/FileStream.h"
#include "IO/FileSystem.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"

#define NATIVE_ENTITYNAME_MAXSIZE FPlatformFile::MaxPathLength

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_VFS_API, VFS)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Unalias_(
    TBasicTextWriter<FileSystem::char_type>& oss,
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target) {
    Assert(alias.PathNode());
    Assert(aliased.PathNode());
    Assert(FileSystem::Separator == target.back());

    oss << target;
    if (aliased.PathNode() == alias.PathNode())
        return;

    Assert(aliased.PathNode()->Depth() >= alias.PathNode()->Depth());
    const size_t length = aliased.PathNode()->Depth() - alias.PathNode()->Depth();
    if (length > 0) {
        auto sep = Fmt::NotFirstTime(FileSystem::char_type(FileSystem::Separator));
        for (const FFileSystemToken& it : aliased.PathNode()->MakeView().LastNElements(length))
            oss << sep << it;
    }
}
//----------------------------------------------------------------------------
static void Unalias_(
    TBasicTextWriter<FileSystem::char_type>& oss,
    const FFilename& aliased,
    const FDirpath& alias, const FWString& target) {
    Unalias_(oss, aliased.Dirpath(), alias, target);
    oss << FileSystem::char_type(FileSystem::Separator)
        << aliased.Basename();
}
//----------------------------------------------------------------------------
static void Unalias_(
    const TMemoryView<FileSystem::char_type>& storage,
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target ) {
    TBasicFixedSizeTextWriter<FileSystem::char_type> oss(storage);
    Unalias_(oss, aliased, alias, target);
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
template <typename _Predicate>
static size_t EnumerateFilesNonRec_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const TFunction<void(const FDirpath&)>& onDirectory,
    const TFunction<void(const FFilename&)>& onFile,
    _Predicate&& pred ) {
    size_t total = 0;

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, aliased, alias, target);

    FPlatformFile::EnumerateDir(nativeDirpath,
        [&](const FWStringView& file) {
            if (pred(file)) {
                ++total;

                if (onFile.Valid())
                    onFile(FFilename(aliased, FBasename(file)));
            }
        },
        [&](const FWStringView& dirname) {
            if (onDirectory.Valid())
                onDirectory(FDirpath(aliased, FDirname(dirname)));
        });

    return total;
}
//----------------------------------------------------------------------------
template <typename _Predicate>
static size_t EnumerateFilesRec_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const TFunction<void(const FDirpath&)>& onDirectory,
    const TFunction<void(const FFilename&)>& onFile,
    _Predicate&& pred ) {
    size_t total = 0;
    VECTORINSITU(FileSystem, FDirpath, 16) subDirectories;

    subDirectories.push_back(aliased);

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    do {
        const FDirpath dirpath = std::move(subDirectories.back());
        subDirectories.pop_back();

        Unalias_(nativeDirpath, dirpath, alias, target);

        FPlatformFile::EnumerateDir(nativeDirpath,
            [&](const FWStringView& file) {
                if (pred(file)) {
                    ++total;

                    if (onFile.Valid())
                        onFile(FFilename(dirpath, FBasename(file)));
                }
            },
            [&](const FWStringView& subdir) {
                subDirectories.emplace_back(dirpath, FDirname(subdir));

                if (onDirectory.Valid())
                    onDirectory.Invoke(subDirectories.back());
            });

    } while (not subDirectories.empty());

    return total;
}
//----------------------------------------------------------------------------
template <typename _Predicate>
static size_t EnumerateDir_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const TFunction<void(const FDirpath&)>& onDirectory,
    const TFunction<void(const FFilename&)>& onFile,
    bool recursive,
    _Predicate&& pred
) {
    return (recursive
        ? EnumerateFilesRec_(aliased, alias, target, onDirectory, onFile, pred)
        : EnumerateFilesNonRec_(aliased, alias, target, onDirectory, onFile, pred) );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::FVirtualFileSystemNativeComponent(const FDirpath& alias, FWString&& target, EOpenPolicy openMode /* = EOpenPolicy::ReadWritable */)
:   FVirtualFileSystemComponent(alias)
,   _openMode(openMode)
,   _target(std::move(target)) {
    Assert(!_target.empty());
    Verify(FPlatformFile::NormalizePath(_target));
    Assert(FPlatformFile::PathSeparator != _target.back());
    _target += FPlatformFile::PathSeparator;
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::FVirtualFileSystemNativeComponent(const FDirpath& alias, const FWString& target, EOpenPolicy openMode /* = EOpenPolicy::ReadWritable */)
:   FVirtualFileSystemNativeComponent(alias, FWString(target), openMode)
{}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::~FVirtualFileSystemNativeComponent() = default;
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
FWString FVirtualFileSystemNativeComponent::Unalias(const FDirpath& aliased) const {
    TBasicStringBuilder<FileSystem::char_type> nativePath;
    Unalias_(nativePath, aliased, _alias, _target);
    return nativePath.ToString();
}
//----------------------------------------------------------------------------
FWString FVirtualFileSystemNativeComponent::Unalias(const FFilename& aliased) const {
    TBasicStringBuilder<FileSystem::char_type> nativePath;
    Unalias_(nativePath, aliased, _alias, _target);
    return nativePath.ToString();
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::DirectoryExists(const FDirpath& dirpath, EExistPolicy policy) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Readable);

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, dirpath, _alias, _target);

    return FPlatformFile::DirectoryExists(&nativeDirpath[0], policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileExists(const FFilename& filename, EExistPolicy policy) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Readable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    return FPlatformFile::FileExists(nativeFilename, policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileStats(FFileStat* pstat, const FFilename& filename) {
    Assert(pstat);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);
    return FPlatformFile::Stat(pstat, nativeFilename);
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::EnumerateDir(const FDirpath& dirpath, bool recursive, const TFunction<void(const FDirpath&)>& onDirectory, const TFunction<void(const FFilename&)>& onFile) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Readable);

    const size_t n = EnumerateDir_(dirpath, _alias, _target, onDirectory, onFile, recursive, [](const FWStringView&) {
        return true;
    });
    PPE_LOG(VFS, Debug, "enumerated {0} files in native directory '{1}' (recursive={2:A})", n, dirpath, recursive);

    return n;
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Readable);

    const size_t n = EnumerateDir_(dirpath, _alias, _target, NoFunction, foreach, recursive, [](const FWStringView&) {
        return true;
    });
    PPE_LOG(VFS, Debug, "enumerated {0} files in native directory '{1}' (recursive={2:A})", n, dirpath, recursive);

    return n;
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Readable);
    Assert_NoAssume(not pattern.empty());

    const size_t n = EnumerateDir_(dirpath, _alias, _target, NoFunction, foreach, recursive, [&pattern](const FWStringView& fname) {
        return WildMatchI(pattern, fname);
    });
    PPE_LOG(VFS, Debug, "globbed {0} files with pattern '{1}' in native directory '{2}' (recursive={3:A})", n, pattern, dirpath, recursive);

    return n;
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::MatchFiles(const FDirpath& dirpath, const FWRegexp& re, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Readable);

    const size_t n = EnumerateDir_(dirpath, _alias, _target, NoFunction, foreach, recursive, [&re](const FWStringView& fname) {
        return re.Match(fname);
    });
    PPE_LOG(VFS, Debug, "matched {0} files with regex in native directory '{1}' (recursive={2:A})", n, dirpath, recursive);

    return n;
}
//----------------------------------------------------------------------------
UStreamReader FVirtualFileSystemNativeComponent::OpenReadable(const FFilename& filename, EAccessPolicy policy) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Readable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    UStreamReader result;
    FFileStreamReader tmp = FFileStream::OpenRead(nativeFilename, policy);
    if (tmp.Good()) {
        PPE_LOG(VFS, Debug, "open native readable '{0}' : {1}", filename, policy);
        result.create<FFileStreamReader>(std::move(tmp));
    }
    else {
        PPE_LOG(VFS, Error, "failed to open native readable '{0}' : {1}", filename, policy);
    }

    return result;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::CreateDirectory(const FDirpath& dirpath) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type buffer[NATIVE_ENTITYNAME_MAXSIZE];
    FWFixedSizeTextWriter nativeDirpath(buffer);
    Unalias_(nativeDirpath, dirpath, _alias, _target);
    nativeDirpath << Eos;

    bool existed = false;
    if (FPlatformFile::CreateDirectoryRecursively(nativeDirpath.data(), &existed)) {
        PPE_CLOG(not existed, VFS, Debug, "created native directory '{0}'", dirpath);
        PPE_CLOG(existed, VFS, Debug, "native directory '{0}' already existed", dirpath);
        return true;
    }

    PPE_LOG(VFS, Error, "failed to create native directory '{0}' ({1})",
        dirpath, MakeCStringView(buffer));
    return false;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::MoveFile(const FFilename& src, const FFilename& dst) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeSrc[NATIVE_ENTITYNAME_MAXSIZE];
    FileSystem::char_type nativeDst[NATIVE_ENTITYNAME_MAXSIZE];

    Unalias_(nativeSrc, src, _alias, _target);
    Unalias_(nativeDst, dst, _alias, _target);

    if (FPlatformFile::MoveFile(nativeSrc, nativeDst)) {
        PPE_LOG(VFS, Debug, "move native file '{0}' -> '{1}'", src, dst);
        return true;
    }

    PPE_LOG(VFS, Error, "failed to move native file '{0}' -> '{1}'", src, dst);
    return false;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveDirectory(const FDirpath& dirpath, bool force) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, dirpath, _alias, _target);

    if (FPlatformFile::RemoveDirectory(nativeDirpath, force)) {
        PPE_LOG(VFS, Debug, "remove native directory '{0}' (force={1:A})", dirpath, force);
        return true;
    }

    PPE_LOG(VFS, Error, "failed remove native directory '{0}' (force={1:A})", dirpath, force);
    return false;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveFile(const FFilename& filename) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    if (FPlatformFile::RemoveFile(nativeFilename)) {
        PPE_LOG(VFS, Debug, "remove native file '{0}'", filename);
        return true;
    }
    PPE_LOG(VFS, Error, "failed to remove native file '{0}'", filename);
    return false;
}
//----------------------------------------------------------------------------
UStreamWriter FVirtualFileSystemNativeComponent::OpenWritable(const FFilename& filename, EAccessPolicy policy) {
    Assert_NoAssume(_openMode ^ EOpenPolicy::Writable);

    UStreamWriter result;

    if (policy ^ (EAccessPolicy::Create + EAccessPolicy::Truncate)) {
        if (not CreateDirectory(filename.Dirpath()))
            return result;
    }

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    if (policy ^ EAccessPolicy::Roll) {
        if (not FPlatformFile::RollFile(nativeFilename)) {
            PPE_LOG(VFS, Error, "failed to roll native file '{0}' : {1}", filename, nativeFilename);
            return result;
        }
    }

    FFileStreamWriter tmp = FFileStream::OpenWrite(nativeFilename, policy);
    if (tmp.Good()) {
        PPE_LOG(VFS, Debug, "open native writable '{0}' : {1}", filename, policy);
        result.create<FFileStreamWriter>(std::move(tmp));
    }
    else {
        PPE_LOG(VFS, Error, "failed to open native writable '{0}' : {1}", filename, policy);
    }

    return result;
}
//----------------------------------------------------------------------------
UStreamReadWriter FVirtualFileSystemNativeComponent::OpenReadWritable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode == EOpenPolicy::ReadWritable);

    UStreamReadWriter result;

    if (policy ^ (EAccessPolicy::Create + EAccessPolicy::Truncate)) {
        if (not CreateDirectory(filename.Dirpath()))
            return result;
    }

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    FFileStreamReadWriter tmp = FFileStream::OpenReadWrite(nativeFilename, policy);
    if (tmp.Good()) {
        PPE_LOG(VFS, Debug, "open native read/writable '{0}' : {1}", filename, policy);
        result.create<FFileStreamReadWriter>(std::move(tmp));
    }
    else {
        PPE_LOG(VFS, Error, "failed to open native read/writable '{0}' : {1}", filename, policy);
    }

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
