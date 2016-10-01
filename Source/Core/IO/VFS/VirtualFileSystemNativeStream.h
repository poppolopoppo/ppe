#pragma once

#include "Core/Core.h"

#include "Core/IO/Stream.h"

#include "Core/IO/FS/Filename.h"

#include "Core/IO/VFS/VirtualFileSystemPolicies.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"

#include "Core/Allocator/PoolAllocator.h"

#include <cstdio>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VirtualFileSystemNativeFileIStream : public IVirtualFileSystemIStream, public Meta::ThreadResource {
public:
    VirtualFileSystemNativeFileIStream(const Filename& filename, const wchar_t* native, AccessPolicy::Mode policy);
    virtual ~VirtualFileSystemNativeFileIStream();

    VirtualFileSystemNativeFileIStream(VirtualFileSystemNativeFileIStream&& rvalue);
    VirtualFileSystemNativeFileIStream& operator =(VirtualFileSystemNativeFileIStream&& rvalue) = delete;

    virtual const Filename& SourceFilename() const override { return _filename; }

    virtual bool Bad() const override;

    virtual bool IsSeekableI(SeekOrigin ) const override { return true; }

    virtual std::streamoff TellI() const override;
    virtual bool SeekI(std::streamoff offset, SeekOrigin policy) override;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override;

    virtual bool Peek(char& ch) override;
    virtual bool Peek(wchar_t& ch) override;

    virtual bool Eof() const override;
    virtual std::streamsize SizeInBytes() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FILE *_handle;
    const Filename _filename;
};
//----------------------------------------------------------------------------
class VirtualFileSystemNativeFileOStream : public IVirtualFileSystemOStream, public Meta::ThreadResource {
public:
    VirtualFileSystemNativeFileOStream(const Filename& filename, const wchar_t* native, AccessPolicy::Mode policy);
    virtual ~VirtualFileSystemNativeFileOStream();

    VirtualFileSystemNativeFileOStream(VirtualFileSystemNativeFileOStream&& rvalue);
    VirtualFileSystemNativeFileOStream& operator =(VirtualFileSystemNativeFileOStream&& rvalue) = delete;

    virtual const Filename& SourceFilename() const override { return _filename; }

    virtual bool Bad() const override;

    virtual bool IsSeekableO(SeekOrigin ) const override { return true; }

    virtual std::streamoff TellO() const override;
    virtual bool SeekO(std::streamoff offset, SeekOrigin policy) override;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override;
    virtual bool WriteSome(const void* storage, size_t eltsize, std::streamsize count) override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FILE *_handle;
    const Filename _filename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
