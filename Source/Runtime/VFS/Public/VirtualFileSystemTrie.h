#pragma once

#include "VirtualFileSystem_fwd.h"

#include "Container/AssociativeVector.h"
#include "IO/MountingPoint.h"
#include "IO/String.h"
#include "Memory/RefPtr.h"
#include "Thread/ReadWriteLock.h"

// #TODO : DAWG as a map
// http://stevehanov.ca/blog/index.php?id=115

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(VirtualFileSystemComponent);
//----------------------------------------------------------------------------
class PPE_VFS_API FVirtualFileSystemTrie {
public:
    typedef ASSOCIATIVE_VECTORINSITU(FileSystem, FMountingPoint, PVirtualFileSystemComponent, 8) nodes_type;

    FVirtualFileSystemTrie();
    ~FVirtualFileSystemTrie();

    FVirtualFileSystemTrie(const FVirtualFileSystemTrie& other) = delete;
    FVirtualFileSystemTrie& operator =(const FVirtualFileSystemTrie& other) = delete;

    bool DirectoryExists(const FDirpath& dirpath, EExistPolicy policy = EExistPolicy::Exists) const;
    bool FileExists(const FFilename& filename, EExistPolicy policy = EExistPolicy::Exists) const;
    bool FileStats(FFileStat* pstat, const FFilename& filename) const;

    size_t EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach) const;
    size_t GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach) const;
    size_t MatchFiles(const FDirpath& dirpath, const FWRegexp& re, bool recursive, const TFunction<void(const FFilename&)>& foreach) const;

    bool CreateDirectory(const FDirpath& dirpath) const;
    bool MoveFile(const FFilename& src, const FFilename& dst) const;
    bool RemoveDirectory(const FDirpath& dirpath, bool force = true) const;
    bool RemoveFile(const FFilename& filename) const;

    UStreamReader OpenReadable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None) const;
    UStreamWriter OpenWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None) const;
    UStreamReadWriter OpenReadWritable(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None) const;

    FWString Unalias(const FFilename& aliased) const;

    void Clear();

    void Mount(FVirtualFileSystemComponent* component);
    void Unmount(FVirtualFileSystemComponent* component);

    FVirtualFileSystemComponent* MountNativePath(const FDirpath& alias, const FWStringView& nativepPath);
    FVirtualFileSystemComponent* MountNativePath(const FDirpath& alias, FWString&& nativepPath);
    FVirtualFileSystemComponent* MountNativePath(const FDirpath& alias, const FWString& nativepPath);

private:
    FReadWriteLock _barrier;
    nodes_type _nodes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
