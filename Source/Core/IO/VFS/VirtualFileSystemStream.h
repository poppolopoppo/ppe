#pragma once

#include "Core/Core.h"

#include "Core/Memory/UniqueView.h"
#include "Core/IO/StreamProvider.h"
#include "Core/IO/VFS/VirtualFileSystemPolicies.h"

#include <iosfwd>

namespace Core {
class Filename;
template <typename T, typename _Allocator>
class RawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVirtualFileSystemBaseStream {
protected:
    IVirtualFileSystemBaseStream() {}
public:
    virtual ~IVirtualFileSystemBaseStream() {}

    virtual const Filename& SourceFilename() const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVirtualFileSystemIStream : public IVirtualFileSystemBaseStream, public IStreamReader {
protected:
    IVirtualFileSystemIStream() {}
public:
    virtual ~IVirtualFileSystemIStream() {}

    IVirtualFileSystemIStream(const IVirtualFileSystemIStream&) = delete;
    IVirtualFileSystemIStream& operator =(const IVirtualFileSystemIStream&) = delete;
};
//----------------------------------------------------------------------------
class IVirtualFileSystemOStream : public IVirtualFileSystemBaseStream, public IStreamWriter {
protected:
    IVirtualFileSystemOStream() {}
public:
    virtual ~IVirtualFileSystemOStream() {}

    IVirtualFileSystemOStream(const IVirtualFileSystemOStream&) = delete;
    IVirtualFileSystemOStream& operator =(const IVirtualFileSystemOStream&) = delete;
};
//----------------------------------------------------------------------------
class IVirtualFileSystemIOStream :
    public IVirtualFileSystemIStream
,   public IVirtualFileSystemOStream {
protected:
    IVirtualFileSystemIOStream() {}
public:
    virtual ~IVirtualFileSystemIOStream() {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
