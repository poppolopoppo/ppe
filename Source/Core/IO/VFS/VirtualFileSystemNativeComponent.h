#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/VFS/VirtualFileSystemComponent.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFilename;
class FFileStat;
//----------------------------------------------------------------------------
class FVirtualFileSystemNativeComponent : public FVirtualFileSystemComponent, IVirtualFileSystemComponentReadWritable {
public:
    enum EOpenMode {
        ModeReadable        = 1,
        ModeWritable        = 2,
        ModeReadWritable    = 3
    };

    FVirtualFileSystemNativeComponent(const FDirpath& alias, FWString&& target, EOpenMode mode = ModeReadWritable);
    FVirtualFileSystemNativeComponent(const FDirpath& alias, const FWString& target, EOpenMode mode = ModeReadWritable);
    virtual ~FVirtualFileSystemNativeComponent();

    EOpenMode EMode() const { return _mode; }
    const FWString& Target() const { return _target; }

    // FVirtualFileSystemComponent
    virtual IVirtualFileSystemComponentReadable* Readable() override;
    virtual IVirtualFileSystemComponentWritable* Writable() override;
    virtual IVirtualFileSystemComponentReadWritable* ReadWritable() override;

    virtual FWString Unalias(const FFilename& aliased) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    // IVirtualFileSystemComponentReadable
    virtual bool DirectoryExists(const FDirpath& dirpath, ExistPolicy::EMode policy) override;
    virtual bool FileExists(const FFilename& filename, ExistPolicy::EMode policy) override;
    virtual bool FileStats(FFileStat* pstat, const FFilename& filename) override;

    virtual size_t EnumerateFiles(const FDirpath& dirpath, bool recursive, const std::function<void(const FFilename&)>& foreach) override;
    virtual size_t GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const std::function<void(const FFilename&)>& foreach) override;

    virtual TUniquePtr<IVirtualFileSystemIStream> OpenReadable(const FFilename& filename, AccessPolicy::EMode policy) override;

    // IVirtualFileSystemComponentWritable
    virtual bool TryCreateDirectory(const FDirpath& dirpath) override;

    virtual TUniquePtr<IVirtualFileSystemOStream> OpenWritable(const FFilename& filename, AccessPolicy::EMode policy) override;

    // IVirtualFileSystemComponentReadWritable
    virtual TUniquePtr<IVirtualFileSystemIOStream> OpenReadWritable(const FFilename& filename, AccessPolicy::EMode policy) override;

private:
    const EOpenMode _mode;
    const FWString _target;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
