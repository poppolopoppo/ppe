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
class FVirtualFileSystemNativeFileIStream : public IVirtualFileSystemIStream, public Meta::FThreadResource {
public:
    FVirtualFileSystemNativeFileIStream(const FFilename& filename, const wchar_t* native, AccessPolicy::EMode policy);
    virtual ~FVirtualFileSystemNativeFileIStream();

    FVirtualFileSystemNativeFileIStream(FVirtualFileSystemNativeFileIStream&& rvalue);
    FVirtualFileSystemNativeFileIStream& operator =(FVirtualFileSystemNativeFileIStream&& rvalue) = delete;

    virtual const FFilename& SourceFilename() const override { return _filename; }

    virtual bool Bad() const override;

    virtual bool IsSeekableI(ESeekOrigin ) const override { return true; }

    virtual std::streamoff TellI() const override;
    virtual bool SeekI(std::streamoff offset, ESeekOrigin policy) override;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override;
    virtual std::streamsize ReadSome(void* storage, size_t eltsize, std::streamsize count) override;

    virtual bool Peek(char& ch) override;
    virtual bool Peek(wchar_t& ch) override;

    virtual bool Eof() const override;
    virtual std::streamsize SizeInBytes() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FILE *_handle;
    const FFilename _filename;
};
//----------------------------------------------------------------------------
class FVirtualFileSystemNativeFileOStream : public IVirtualFileSystemOStream, public Meta::FThreadResource {
public:
    FVirtualFileSystemNativeFileOStream(const FFilename& filename, const wchar_t* native, AccessPolicy::EMode policy);
    virtual ~FVirtualFileSystemNativeFileOStream();

    FVirtualFileSystemNativeFileOStream(FVirtualFileSystemNativeFileOStream&& rvalue);
    FVirtualFileSystemNativeFileOStream& operator =(FVirtualFileSystemNativeFileOStream&& rvalue) = delete;

    virtual const FFilename& SourceFilename() const override { return _filename; }

    virtual bool Bad() const override;

    virtual bool IsSeekableO(ESeekOrigin ) const override { return true; }

    virtual std::streamoff TellO() const override;
    virtual bool SeekO(std::streamoff offset, ESeekOrigin policy) override;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override;
    virtual bool WriteSome(const void* storage, size_t eltsize, std::streamsize count) override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FILE *_handle;
    const FFilename _filename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
