#pragma once

#include "VirtualFileSystem_fwd.h"
#include "VirtualFileSystemComponent.h"

#include "IO/Dirpath.h"
#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_VFS_API FVirtualFileSystemNativeComponent : public FVirtualFileSystemComponent, IVirtualFileSystemComponentReadWritable {
public:
    FVirtualFileSystemNativeComponent(const FDirpath& alias, FWString&& target, EOpenPolicy openMode = EOpenPolicy::ReadWritable);
    FVirtualFileSystemNativeComponent(const FDirpath& alias, const FWString& target, EOpenPolicy openMode = EOpenPolicy::ReadWritable);
    virtual ~FVirtualFileSystemNativeComponent();

    EOpenPolicy OpenMode() const { return _openMode; }
    const FWString& Target() const { return _target; }

    // FVirtualFileSystemComponent
    virtual IVirtualFileSystemComponentReadable* Readable() override final;
    virtual IVirtualFileSystemComponentWritable* Writable() override final;
    virtual IVirtualFileSystemComponentReadWritable* ReadWritable() override final;

    virtual FWString Unalias(const FFilename& aliased) const override final;

private: // IVirtualFileSystemComponentReadable
    virtual bool DirectoryExists(const FDirpath& dirpath, EExistPolicy policy) override final;
    virtual bool FileExists(const FFilename& filename, EExistPolicy policy) override final;
    virtual bool FileStats(FFileStat* pstat, const FFilename& filename) override final;

    virtual size_t EnumerateFiles(const FDirpath& dirpath, bool recursive, const TFunction<void(const FFilename&)>& foreach) override final;
    virtual size_t GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const TFunction<void(const FFilename&)>& foreach) override final;

    virtual UStreamReader OpenReadable(const FFilename& filename, EAccessPolicy policy) override final;

private: // IVirtualFileSystemComponentWritable
    virtual bool CreateDirectory(const FDirpath& dirpath) override final;
    virtual bool MoveFile(const FFilename& src, const FFilename& dst) override final;
    virtual bool RemoveDirectory(const FDirpath& dirpath, bool force) override final;
    virtual bool RemoveFile(const FFilename& filename) override final;

    virtual UStreamWriter OpenWritable(const FFilename& filename, EAccessPolicy policy) override final;

private: // IVirtualFileSystemComponentReadWritable
    virtual UStreamReadWriter OpenReadWritable(const FFilename& filename, EAccessPolicy policy) override final;

private:
    const EOpenPolicy _openMode;
    FWString _target;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
