#pragma once

#include "Core/Core.h"

#include "Core/Memory/UniqueView.h"
#include "Core/IO/FS/Policies.h"
#include "Core/IO/StreamProvider.h"

#include <iosfwd>

namespace Core {
class FFilename;
template <typename T, typename _Allocator>
class TRawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IVirtualFileSystemBaseStream {
protected:
    IVirtualFileSystemBaseStream() {}
public:
    virtual ~IVirtualFileSystemBaseStream() {}

    virtual bool Bad() const = 0;
    virtual const FFilename& Fragment() const = 0;
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
