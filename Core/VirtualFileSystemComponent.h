#pragma once

#include "Core.h"
#include "RefPtr.h"
#include "UniquePtr.h"
#include "Dirpath.h"

#include "VirtualFileSystemPolicies.h"
#include "VirtualFileSystemStream.h"

#include <functional>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(VirtualFileSystemComponent);
class Filename;
class IVirtualFileSystemComponentReadable;
class IVirtualFileSystemComponentWritable;
class IVirtualFileSystemComponentReadWritable;
//----------------------------------------------------------------------------
class VirtualFileSystemComponent : public RefCountable {
public:
    virtual ~VirtualFileSystemComponent() {}

    VirtualFileSystemComponent(const VirtualFileSystemComponent& other) = delete;
    VirtualFileSystemComponent& operator =(const VirtualFileSystemComponent& other) = delete;

    virtual IVirtualFileSystemComponentReadable* Readable() = 0;
    virtual IVirtualFileSystemComponentWritable* Writable() = 0;
    virtual IVirtualFileSystemComponentReadWritable* ReadWritable() = 0;

    virtual WString Unalias(const Filename& aliased) const = 0;

    const Dirpath& Alias() const { return _alias; }

protected:
    explicit VirtualFileSystemComponent(const Dirpath& alias);

    Dirpath _alias;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVirtualFileSystemComponentReadable {
public:
    virtual ~IVirtualFileSystemComponentReadable() {}

    virtual bool DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy) = 0;
    virtual bool FileExists(const Filename& filename, ExistPolicy::Mode policy) = 0;

    virtual size_t EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) = 0;

    virtual UniquePtr<IVirtualFileSystemIStream> OpenReadable(const Filename& filename, AccessPolicy::Mode policy) = 0;
};
//----------------------------------------------------------------------------
class IVirtualFileSystemComponentWritable {
public:
    virtual ~IVirtualFileSystemComponentWritable() {}

    virtual bool CreateDirectory(const Dirpath& dirpath) = 0;

    virtual UniquePtr<IVirtualFileSystemOStream> OpenWritable(const Filename& filename, AccessPolicy::Mode policy) = 0;
};
//----------------------------------------------------------------------------
class IVirtualFileSystemComponentReadWritable :
    public IVirtualFileSystemComponentReadable
,   public IVirtualFileSystemComponentWritable {
public:
    virtual ~IVirtualFileSystemComponentReadWritable() {}

    virtual UniquePtr<IVirtualFileSystemIOStream> OpenReadWritable(const Filename& filename, AccessPolicy::Mode policy) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VirtualFileSystemComponentStartup {
public:
    static void Start(VirtualFileSystemComponent *component);
    static void Shutdown(VirtualFileSystemComponent *component);

    VirtualFileSystemComponentStartup(VirtualFileSystemComponent *component) : _component(component) { Start(component); }
    ~VirtualFileSystemComponentStartup() { Shutdown(_component); }

private:
    PVirtualFileSystemComponent _component;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
