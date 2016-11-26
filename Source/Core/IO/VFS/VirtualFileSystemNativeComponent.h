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
    virtual IVirtualFileSystemComponentReadable* Readable() override final;
    virtual IVirtualFileSystemComponentWritable* Writable() override final;
    virtual IVirtualFileSystemComponentReadWritable* ReadWritable() override final;

    virtual FWString Unalias(const FFilename& aliased) const override final;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    // IVirtualFileSystemComponentReadable
    virtual bool DirectoryExists(const FDirpath& dirpath, ExistPolicy::EMode policy) override final;
    virtual bool FileExists(const FFilename& filename, ExistPolicy::EMode policy) override final;
    virtual bool FileStats(FFileStat* pstat, const FFilename& filename) override final;

    virtual size_t EnumerateFiles(const FDirpath& dirpath, bool recursive, const std::function<void(const FFilename&)>& foreach) override final;
    virtual size_t GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const std::function<void(const FFilename&)>& foreach) override final;

    virtual TUniquePtr<IVirtualFileSystemIStream> OpenReadable(const FFilename& filename, AccessPolicy::EMode policy) override final;

    // IVirtualFileSystemComponentWritable
    virtual bool TryCreateDirectory(const FDirpath& dirpath) override final;

    virtual TUniquePtr<IVirtualFileSystemOStream> OpenWritable(const FFilename& filename, AccessPolicy::EMode policy) override final;

    // IVirtualFileSystemComponentReadWritable
    virtual TUniquePtr<IVirtualFileSystemIOStream> OpenReadWritable(const FFilename& filename, AccessPolicy::EMode policy) override final;

private:
    const EOpenMode _mode;
    const FWString _target;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
