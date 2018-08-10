#pragma once

#include "Core.h"

#include "IO/StreamProvider.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "HAL/PlatformLowLevelIO.h"

#ifndef FINAL_RELEASE
#   define WITH_PPE_FILESTREAM_FILENAMEDBG 1 // %_NOCOMMIT%
#else
#   define WITH_PPE_FILESTREAM_FILENAMEDBG 0
#endif

#if WITH_PPE_FILESTREAM_FILENAMEDBG
#   include "IO/String.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FFileStream {
public:
    using FFileHandle = FPlatformLowLevelIO::FHandle;
protected:
    explicit FFileStream(FFileHandle handle) : _handle(handle) {}
public:
    ~FFileStream();

    FFileStream(const FFileStream&) = delete;
    FFileStream& operator =(const FFileStream&) = delete;

    FFileStream(FFileStream&& rvalue);
    FFileStream& operator =(FFileStream&& rvalue);

    FFileHandle Handle() const { return _handle; }

#if WITH_PPE_FILESTREAM_FILENAMEDBG
    FWStringView FilenameForDebug() const { return MakeStringView(_filenameForDebug); }
#endif

    bool Good() const;
    bool Bad() const { return (not Good()); }

    bool Close();
    bool Commit();
    bool Dup2(FFileStream& other) const;

    static class FFileStreamReader OpenRead(const wchar_t* filename, EAccessPolicy flags);
    static class FFileStreamWriter OpenWrite(const wchar_t* filename, EAccessPolicy flags);
    static class FFileStreamReadWriter OpenReadWrite(const wchar_t* filename, EAccessPolicy flags);

    static void Start();
    static void Shutdown();

protected:
    FFileHandle _handle;
#if WITH_PPE_FILESTREAM_FILENAMEDBG
private:
    FWString _filenameForDebug;
#endif
};
//----------------------------------------------------------------------------
class PPE_CORE_API FFileStreamReader : public IStreamReader, public FFileStream {
public:
    using FFileStream::FFileHandle;
    explicit FFileStreamReader(FFileHandle handle) : FFileStream(handle) {}

public: // IStreamReader
    virtual bool Eof() const override final;

    virtual bool IsSeekableI(ESeekOrigin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellI() const override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;
};
//----------------------------------------------------------------------------
class PPE_CORE_API FFileStreamWriter : public IStreamWriter, public FFileStream {
public:
    using FFileStream::FFileHandle;
    explicit FFileStreamWriter(FFileHandle handle) : FFileStream(handle) {}

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;
};
//----------------------------------------------------------------------------
class PPE_CORE_API EMPTY_BASES FFileStreamReadWriter : public IStreamReadWriter, public FFileStream {
public:
    using FFileStream::FFileHandle;
    explicit FFileStreamReadWriter(FFileHandle handle) : FFileStream(handle) {}

public: // IStreamReader
    virtual bool Eof() const override final;

    virtual bool IsSeekableI(ESeekOrigin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellI() const override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
