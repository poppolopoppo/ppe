#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/Dirpath.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Function.h"

#include "Core/IO/VFS/VirtualFileSystemPolicies.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"

#include <functional>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(VirtualFileSystemComponent);
class FDirpath;
class FFilename;
class FFileStat;
class IVirtualFileSystemComponentReadable;
class IVirtualFileSystemComponentWritable;
class IVirtualFileSystemComponentReadWritable;
//----------------------------------------------------------------------------
class FVirtualFileSystemComponent : public FRefCountable {
public:
    virtual ~FVirtualFileSystemComponent() {}

    FVirtualFileSystemComponent(const FVirtualFileSystemComponent& other) = delete;
    FVirtualFileSystemComponent& operator =(const FVirtualFileSystemComponent& other) = delete;

    virtual IVirtualFileSystemComponentReadable* Readable() = 0;
    virtual IVirtualFileSystemComponentWritable* Writable() = 0;
    virtual IVirtualFileSystemComponentReadWritable* ReadWritable() = 0;

    virtual FWString Unalias(const FFilename& aliased) const = 0;

    const FDirpath& Alias() const { return _alias; }

protected:
    explicit FVirtualFileSystemComponent(const FDirpath& alias);

    const FDirpath _alias;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVirtualFileSystemComponentReadable {
public:
    virtual ~IVirtualFileSystemComponentReadable() {}

    virtual bool DirectoryExists(const FDirpath& dirpath, EExistPolicy policy) = 0;
    virtual bool FileExists(const FFilename& filename, EExistPolicy policy) = 0;
    virtual bool FileStats(FFileStat* pstat, const FFilename& filename) = 0;

    virtual size_t EnumerateFiles(const FDirpath& dirpath, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach) = 0;
    virtual size_t GlobFiles(const FDirpath& dirpath, const FWStringView& pattern, bool recursive, const Meta::TFunction<void(const FFilename&)>& foreach) = 0;

    virtual TUniquePtr<IVirtualFileSystemIStream> OpenReadable(const FFilename& filename, EAccessPolicy policy) = 0;
};
//----------------------------------------------------------------------------
class IVirtualFileSystemComponentWritable {
public:
    virtual ~IVirtualFileSystemComponentWritable() {}

    virtual bool CreateDirectory(const FDirpath& dirpath) = 0;
    virtual bool RemoveDirectory(const FDirpath& dirpath) = 0;
    virtual bool RemoveFile(const FFilename& filename) = 0;

    virtual TUniquePtr<IVirtualFileSystemOStream> OpenWritable(const FFilename& filename, EAccessPolicy policy) = 0;
};
//----------------------------------------------------------------------------
class IVirtualFileSystemComponentReadWritable :
    public IVirtualFileSystemComponentReadable
,   public IVirtualFileSystemComponentWritable {
public:
    virtual ~IVirtualFileSystemComponentReadWritable() {}

    virtual TUniquePtr<IVirtualFileSystemIOStream> OpenReadWritable(const FFilename& filename, EAccessPolicy policy) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVirtualFileSystemComponentStartup {
public:
    static void Start(FVirtualFileSystemComponent *component);
    static void Shutdown(FVirtualFileSystemComponent *component);

    FVirtualFileSystemComponentStartup(FVirtualFileSystemComponent *component) : _component(component) { Start(component); }
    ~FVirtualFileSystemComponentStartup() { Shutdown(_component.get()); }

private:
    PVirtualFileSystemComponent _component;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
