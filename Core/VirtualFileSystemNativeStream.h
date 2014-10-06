#pragma once

#include "Core.h"
#include "Stream.h"

#include "VirtualFileSystemPolicies.h"
#include "VirtualFileSystemStream.h"

#include <cstdio>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VirtualFileSystemNativeFileIStream : public IVirtualFileSystemIStream {
public:
    VirtualFileSystemNativeFileIStream(const wchar_t* filename, AccessPolicy::Mode policy);
    virtual ~VirtualFileSystemNativeFileIStream();

    virtual std::streamoff TellI() override;
    virtual void SeekI(std::streamoff offset) override;

    virtual void Read(void* storage, std::streamsize count) override;
    virtual std::streamsize ReadSome(void* storage, std::streamsize count) override;

    virtual char PeekChar() override;

    virtual bool Eof() const override;
    virtual std::streamsize Size() const override;

private:
    FILE *_handle;
};
//----------------------------------------------------------------------------
class VirtualFileSystemNativeFileOStream : public IVirtualFileSystemOStream {
public:
    VirtualFileSystemNativeFileOStream(const wchar_t* filename, AccessPolicy::Mode policy);
    virtual ~VirtualFileSystemNativeFileOStream();

    virtual std::streamoff TellO() override;
    virtual void SeekO(std::streamoff offset) override;
    virtual void Write(const void* storage, std::streamsize count) override;

private:
    FILE *_handle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
