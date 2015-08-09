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
class VirtualFileSystemNativeFileIStream : public IVirtualFileSystemIStream {
public:
    VirtualFileSystemNativeFileIStream(const Filename& filename, const wchar_t* native, AccessPolicy::Mode policy);
    virtual ~VirtualFileSystemNativeFileIStream();

    virtual const Filename& SourceFilename() const override { return _filename; }

    virtual std::streamoff TellI() override;
    virtual void SeekI(std::streamoff offset) override;

    virtual void Read(void* storage, std::streamsize count) override;
    virtual std::streamsize ReadSome(void* storage, std::streamsize count) override;

    virtual char PeekChar() override;

    virtual bool Eof() const override;
    virtual std::streamsize Size() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FILE *_handle;
    Filename _filename;
};
//----------------------------------------------------------------------------
class VirtualFileSystemNativeFileOStream : public IVirtualFileSystemOStream {
public:
    VirtualFileSystemNativeFileOStream(const Filename& filename, const wchar_t* native, AccessPolicy::Mode policy);
    virtual ~VirtualFileSystemNativeFileOStream();

    virtual const Filename& SourceFilename() const override { return _filename; }

    virtual std::streamoff TellO() override;
    virtual void SeekO(std::streamoff offset) override;
    virtual void Write(const void* storage, std::streamsize count) override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FILE *_handle;
    Filename _filename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
