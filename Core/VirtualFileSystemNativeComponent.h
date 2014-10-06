#pragma once

#include "Core.h"
#include "Dirpath.h"
#include "VirtualFileSystemComponent.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Filename;
//----------------------------------------------------------------------------
class VirtualFileSystemNativeComponent : public VirtualFileSystemComponent, IVirtualFileSystemComponentReadWritable {
public:
    enum OpenMode {
        ModeReadable        = 1,
        ModeWritable        = 2,
        ModeReadWritable    = 3
    };

    VirtualFileSystemNativeComponent(const Dirpath& alias, WString&& target, OpenMode mode = ModeReadWritable);
    VirtualFileSystemNativeComponent(const Dirpath& alias, const WString& target, OpenMode mode = ModeReadWritable);
    virtual ~VirtualFileSystemNativeComponent();

    OpenMode Mode() const { return _mode; }
    const WString& Target() const { return _target; }

    // VirtualFileSystemComponent
    virtual IVirtualFileSystemComponentReadable* Readable() override;
    virtual IVirtualFileSystemComponentWritable* Writable() override;
    virtual IVirtualFileSystemComponentReadWritable* ReadWritable() override;

    virtual WString Unalias(const Filename& aliased) const override;

private:
    // IVirtualFileSystemComponentReadable
    virtual bool DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy) override;
    virtual bool FileExists(const Filename& filename, ExistPolicy::Mode policy) override;

    virtual size_t EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) override;

    virtual UniquePtr<IVirtualFileSystemIStream> OpenReadable(const Filename& filename, AccessPolicy::Mode policy) override;

    // IVirtualFileSystemComponentWritable
    virtual bool CreateDirectory(const Dirpath& dirpath) override;

    virtual UniquePtr<IVirtualFileSystemOStream> OpenWritable(const Filename& filename, AccessPolicy::Mode policy) override;

    // IVirtualFileSystemComponentReadWritable
    virtual UniquePtr<IVirtualFileSystemIOStream> OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy) override;

private:
    OpenMode _mode;
    WString _target;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
