#pragma once

#include "Core/Core.h"

#include "Core/IO/StreamProvider.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Misc/TargetPlatform.h"
#include "Core/Meta/ThreadResource.h"

#ifndef FINAL_RELEASE
#   define WITH_CORE_FILESTREAM_FILENAMEDBG 1 // %_NOCOMMIT%
#else
#   define WITH_CORE_FILESTREAM_FILENAMEDBG 0
#endif

#if WITH_CORE_FILESTREAM_FILENAMEDBG
#   include "Core/IO/String.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern CORE_API FTextWriter& GStdout;
extern CORE_API FTextWriter& GStderr;
//----------------------------------------------------------------------------
extern CORE_API FWTextWriter& GWStdout;
extern CORE_API FWTextWriter& GWStderr;
//----------------------------------------------------------------------------
class CORE_API FFileStream : protected Meta::FThreadResource {
protected:
    explicit FFileStream(FPlatformIO::FHandle handle) : _handle(handle) {}
public:
    ~FFileStream();

    FFileStream(const FFileStream&) = delete;
    FFileStream& operator =(const FFileStream&) = delete;

    FFileStream(FFileStream&& rvalue);
    FFileStream& operator =(FFileStream&& rvalue);

    FPlatformIO::FHandle Handle() const { return _handle; }

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    FWStringView FilenameForDebug() const { return MakeStringView(_filenameForDebug); }
#endif

    bool Good() const;
    bool Bad() const { return (not Good()); }

    bool Close();
    bool Commit();
    bool Dup2(FFileStream& other) const;

    static class FFileStreamReader OpenRead(const FWStringView& filename, EAccessPolicy flags);
    static class FFileStreamWriter OpenWrite(const FWStringView& filename, EAccessPolicy flags);
    static class FFileStreamReadWriter OpenReadWrite(const FWStringView& filename, EAccessPolicy flags);

    static class FFileStreamReader OpenStdin(EAccessPolicy flags = EAccessPolicy::TextU8);
    static class FFileStreamWriter OpenStdout(EAccessPolicy flags = EAccessPolicy::TextU8);
    static class FFileStreamWriter OpenStderr(EAccessPolicy flags = EAccessPolicy::TextU8);

    static class FBufferedStreamReader* StdinReader();
    static class FBufferedStreamWriter* StdoutWriter();
    static class FBufferedStreamWriter* StderrWriter();

    static void Start();
    static void Shutdown();

protected:
    FPlatformIO::FHandle _handle;
#if WITH_CORE_FILESTREAM_FILENAMEDBG
private:
    FWString _filenameForDebug;
#endif
};
//----------------------------------------------------------------------------
class CORE_API FFileStreamReader : public IStreamReader, public FFileStream {
public:
    explicit FFileStreamReader(FPlatformIO::FHandle handle) : FFileStream(handle) {}

public: // IStreamReader
    virtual bool Eof() const override final;

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellI() const override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;
};
//----------------------------------------------------------------------------
class CORE_API FFileStreamWriter : public IStreamWriter, public FFileStream {
public:
    explicit FFileStreamWriter(FPlatformIO::FHandle handle) : FFileStream(handle) {}

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;
};
//----------------------------------------------------------------------------
class CORE_API EMPTY_BASES FFileStreamReadWriter : public IStreamReadWriter, public FFileStream {
public:
    explicit FFileStreamReadWriter(FPlatformIO::FHandle handle) : FFileStream(handle) {}

public: // IStreamReader
    virtual bool Eof() const override final;

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellI() const override final;
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual std::streamsize SizeInBytes() const override final;

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const override final { return true; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
