#include "stdafx.h"

#include "VirtualFileSystemNativeComponent.h"

#include "VirtualFileSystemTrie.h"

#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformLowLevelIO.h"
#include "IO/FS/FileSystemToken.h"
#include "IO/FS/FileSystemTrie.h"
#include "IO/FileStream.h"
#include "IO/FileSystem.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "IO/VirtualFileSystem.h"
#include "Memory/MemoryProvider.h"

#define NATIVE_ENTITYNAME_MAXSIZE FPlatformFile::MaxPathLength

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_API, VFS)
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
        STACKLOCAL_ASSUMEPOD_ARRAY(FFileSystemToken, subpath, length);
        const size_t k = FFileSystemPath::Get().Expand(subpath, alias.PathNode(), aliased.PathNode());

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
    const FileSystem::FStringView suffix = FileSystem::FStringView() ) {
    TBasicFixedSizeTextWriter<FileSystem::char_type> oss(storage);
    Unalias_(oss, aliased, alias, target);
    if (not suffix.empty())
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
static size_t GlobFiles_(
    const FDirpath& aliased,
    const FDirpath& alias, const FWString& target,
    const TFunction<void(const FFilename&)>& foreach,
    const FWStringView& pattern,
    bool recursive
    ) {
    struct FContext_ {
        size_t Total = 0;
        FDirpath Dirpath;
        VECTORINSITU(FileSystem, FDirpath, 16) SubDirectories;
    }   ctx;

    ctx.SubDirectories.push_back(aliased);

    const TFunction<void(const FWStringView&)> onFile = [&ctx, &pattern, &foreach](const FWStringView& file) {
        if (pattern.empty() || WildMatchI(pattern, file)) {
            ctx.Total++;
            const FBasename basename(file);
            foreach(FFilename(ctx.Dirpath, basename));
        }
    };
    TFunction<void(const FWStringView&)> onSubDir;
    if (recursive) {
        onSubDir = [&ctx](const FWStringView& subdir) {
            const FDirname dirname(subdir);
            ctx.SubDirectories.emplace_back(ctx.Dirpath, dirname);
        };
    }

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    do {
        ctx.Dirpath = std::move(ctx.SubDirectories.back());
        ctx.SubDirectories.pop_back();

        Unalias_(nativeDirpath, ctx.Dirpath, alias, target);

        FPlatformFile::EnumerateDir(nativeDirpath, onFile, onSubDir);

    } while (not ctx.SubDirectories.empty());

    return ctx.Total;
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
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeComponent::FVirtualFileSystemNativeComponent(const FDirpath& alias, const FWString& target, EOpenPolicy openMode /* = EOpenPolicy::ReadWritable */)
:   FVirtualFileSystemComponent(alias)
,   _openMode(openMode)
,   _target(target) {
    Assert(!_target.empty());
    Verify(FPlatformFile::NormalizePath(_target));
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

    return FPlatformFile::DirectoryExists(&nativeDirpath[0], policy);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::FileExists(const FFilename& filename, EExistPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

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
size_t FVirtualFileSystemNativeComponent::EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    const size_t n = GlobFiles_(dirpath, _alias, _target, foreach, FWStringView(), recursive);
    LOG(VFS, Debug, L"enumerated {0} files in native directory '{1}' (recursive={2:A})", n, dirpath, recursive);

    return n;
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeComponent::GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach) {
    Assert(_openMode ^ EOpenPolicy::Readable);
    Assert(pattern.size());

    const size_t n = GlobFiles_(dirpath, _alias, _target, foreach, pattern, recursive);
    LOG(VFS, Debug, L"globbed {0} files with pattern '{1}' in native directory '{2}' (recursive={3:A})", n, pattern, dirpath, recursive);

    return n;
}
//----------------------------------------------------------------------------
UStreamReader FVirtualFileSystemNativeComponent::OpenReadable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Readable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    UStreamReader result;
    FFileStreamReader tmp = FFileStream::OpenRead(nativeFilename, policy);
    if (tmp.Good()) {
        LOG(VFS, Debug, L"open native readable '{0}' : {1}", filename, policy);
        result.reset(new FFileStreamReader(std::move(tmp)));
    }
    else {
        LOG(VFS, Error, L"failed to open native readable '{0}' : {1}", filename, policy);
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

    if (not FPlatformFile::DirectoryExists(nativeDirpath.data(), EExistPolicy::Exists)) {
        bool existed = false;

        if (FPlatformFile::CreateDirectory(nativeDirpath.data(), &existed)) {
            LOG(VFS, Debug, L"create native directory '{0}'", dirpath);
            return true;
        }

        const size_t l = nativeDirpath.size();
        forrange(i, _target.size() - 1, l + 1) {
            if (l == i || FileSystem::Separators().Contains(buffer[i])) {
                buffer[i] = L'\0';

                if (FPlatformFile::CreateDirectory(nativeDirpath.data(), &existed)) {
                    if (l != i)
                        buffer[i] = FileSystem::Separator;
                    if (not existed)
                        LOG(VFS, Debug, L"create native directory '{0}'", MakeCStringView(buffer));
                }
                else {
                    LOG(VFS, Error, L"failed to create native directory '{0}'", MakeCStringView(buffer));
                    return false;
                }
            }
        }
    }
    else {
        LOG(VFS, Debug, L"native directory '{0}' already exists", dirpath);
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

    if (FPlatformFile::MoveFile(nativeSrc, nativeDst)) {
        LOG(VFS, Debug, L"move native file '{0}' -> '{1}'", src, dst);
        return true;
    }
    else {
        LOG(VFS, Error, L"failed to move native file '{0}' -> '{1}'", src, dst);
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveDirectory(const FDirpath& dirpath, bool force) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, dirpath, _alias, _target);

    if (FPlatformFile::RemoveDirectory(nativeDirpath, force)) {
        LOG(VFS, Debug, L"remove native directory '{0}' (force={1:A})", dirpath, force);
        return true;
    }
    else {
        LOG(VFS, Error, L"failed remove native directory '{0}' (force={1:A})", dirpath, force);
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeComponent::RemoveFile(const FFilename& filename) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    if (FPlatformFile::RemoveFile(nativeFilename)) {
        LOG(VFS, Debug, L"remove native file '{0}'", filename);
        return true;
    }
    else {
        LOG(VFS, Error, L"failed to remove native file '{0}'", filename);
        return false;
    }
}
//----------------------------------------------------------------------------
UStreamWriter FVirtualFileSystemNativeComponent::OpenWritable(const FFilename& filename, EAccessPolicy policy) {
    Assert(_openMode ^ EOpenPolicy::Writable);

    UStreamWriter result;

    if (policy ^ (EAccessPolicy::Create + EAccessPolicy::Truncate)) {
        if (not CreateDirectory(filename.Dirpath()))
            return result;
    }

    FileSystem::char_type nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, filename, _alias, _target);

    FFileStreamWriter tmp = FFileStream::OpenWrite(nativeFilename, policy);
    if (tmp.Good()) {
        LOG(VFS, Debug, L"open native writable '{0}' : {1}", filename, policy);
        result.reset(new FFileStreamWriter(std::move(tmp)));
    }
    else {
        LOG(VFS, Error, L"failed to open native writable '{0}' : {1}", filename, policy);
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
        LOG(VFS, Debug, L"open native read/writable '{0}' : {1}", filename, policy);
        result.reset(new FFileStreamReadWriter(std::move(tmp)));
    }
    else {
        LOG(VFS, Error, L"failed to open native read/writable '{0}' : {1}", filename, policy);
    }

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
