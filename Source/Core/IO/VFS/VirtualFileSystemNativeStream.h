#pragma once

#include "Core/Core.h"

#include "Core/IO/Stream.h"

#include "Core/IO/FS/Filename.h"

#include "Core/IO/FS/Policies.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"

#include "Core/Allocator/PoolAllocator.h"

#include <cstdio>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVirtualFileSystemNativeFileIStream : public IVirtualFileSystemIStream, public Meta::FThreadResource {
public:
    FVirtualFileSystemNativeFileIStream(const FFilename& filename, const wchar_t* native, EAccessPolicy policy);
    virtual ~FVirtualFileSystemNativeFileIStream();

    FVirtualFileSystemNativeFileIStream(FVirtualFileSystemNativeFileIStream&& rvalue);
    FVirtualFileSystemNativeFileIStream& operator =(FVirtualFileSystemNativeFileIStream&& rvalue) = delete;

    virtual const FFilename& Fragment() const override final { return _filename; }

    virtual bool Bad() const override final;

    virtual bool IsSeekableI(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellI() const override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin policy) override final;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

    virtual bool Eof() const override final;
    virtual std::streamsize SizeInBytes() const override final;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    int _handle;
    const FFilename _filename;
};
//----------------------------------------------------------------------------
class FVirtualFileSystemNativeFileOStream : public IVirtualFileSystemOStream, public Meta::FThreadResource {
public:
    FVirtualFileSystemNativeFileOStream(const FFilename& filename, const wchar_t* native, EAccessPolicy policy);
    virtual ~FVirtualFileSystemNativeFileOStream();

    FVirtualFileSystemNativeFileOStream(FVirtualFileSystemNativeFileOStream&& rvalue);
    FVirtualFileSystemNativeFileOStream& operator =(FVirtualFileSystemNativeFileOStream&& rvalue) = delete;

    virtual const FFilename& Fragment() const override final { return _filename; }

    virtual bool Bad() const override final;

    virtual bool IsSeekableO(ESeekOrigin ) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin policy) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    int _handle;
    const FFilename _filename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
