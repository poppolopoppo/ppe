#include "stdafx.h"

#include "FileStream.h"

#include "Allocator/Alloca.h"
#include "IO/BufferedStream.h"
#include "IO/TextWriter.h"
#include "Misc/TargetPlatform.h"

#include "Time/TimedScope.h"

#if WITH_CORE_FILESTREAM_FILENAMEDBG
#   include "IO/Format.h"
#   include "IO/StringBuilder.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFileStream::~FFileStream() {
    if (Good())
        VerifyRelease(FPlatformIO::Close(_handle));
}
//----------------------------------------------------------------------------
FFileStream::FFileStream(FFileStream&& rvalue)
    : _handle(rvalue._handle) 
#if WITH_CORE_FILESTREAM_FILENAMEDBG
    , _filenameForDebug(std::move(rvalue._filenameForDebug))
#endif
{
    rvalue._handle = FPlatformIO::InvalidHandle;
}
//----------------------------------------------------------------------------
FFileStream& FFileStream::operator =(FFileStream&& rvalue) {
    if (Good())
        VerifyRelease(FPlatformIO::Close(_handle));

    Assert(Bad());
    std::swap(_handle, rvalue._handle);
#if WITH_CORE_FILESTREAM_FILENAMEDBG
    _filenameForDebug = std::move(rvalue._filenameForDebug);
#endif

    return (*this);
}
//----------------------------------------------------------------------------
bool FFileStream::Good() const {
    return (FPlatformIO::InvalidHandle != _handle);
}
//----------------------------------------------------------------------------
bool FFileStream::Close() {
    if (Good()) {
        bool succeed = FPlatformIO::Close(_handle);
        _handle = FPlatformIO::InvalidHandle;
        return succeed;
    }
    return false;
}
//----------------------------------------------------------------------------
bool FFileStream::Commit() {
    Assert(Good());

    return FPlatformIO::Commit(_handle);
}
//----------------------------------------------------------------------------
bool FFileStream::Dup2(FFileStream& other) const {
    Assert(Good());
    Assert(other.Good());

    return FPlatformIO::Dup2(_handle, other._handle);
}
//----------------------------------------------------------------------------
FFileStreamReader FFileStream::OpenRead(const wchar_t* filename, EAccessPolicy flags) {
    Assert(filename);

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(MakeCStringView(filename));

    FFileStreamReader reader(FPlatformIO::Open(filenameForDebug.c_str(), EOpenPolicy::Readable, flags));
    reader._filenameForDebug = std::move(filenameForDebug);

    return reader;

#else
    return FFileStreamReader(FPlatformIO::Open(filename, EOpenPolicy::Readable, flags));

#endif
}
//----------------------------------------------------------------------------
FFileStreamWriter FFileStream::OpenWrite(const wchar_t* filename, EAccessPolicy flags) {
    Assert(filename);

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(MakeCStringView(filename));

    FFileStreamWriter writer(FPlatformIO::Open(filenameForDebug.c_str(), EOpenPolicy::Writable, flags));
    writer._filenameForDebug = std::move(filenameForDebug);

    return writer;

#else
    return FFileStreamWriter(FPlatformIO::Open(filename, EOpenPolicy::Writable, flags));

#endif
}
//----------------------------------------------------------------------------
FFileStreamReadWriter FFileStream::OpenReadWrite(const wchar_t* filename, EAccessPolicy flags) {
    Assert(filename);

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(MakeCStringView(filename));

    FFileStreamReadWriter readWriter(FPlatformIO::Open(filenameForDebug.c_str(), EOpenPolicy::ReadWritable, flags));
    readWriter._filenameForDebug = std::move(filenameForDebug);

    return readWriter;

#else
    return FFileStreamReadWriter(FPlatformIO::Open(filename, EOpenPolicy::ReadWritable, flags));

#endif
}
//----------------------------------------------------------------------------
void FFileStream::Start() {
    // Set std streams mode to utf-8 chars :
    /*
    Verify(FPlatformIO::SetMode(FPlatformIO::Stdin, EAccessPolicy::Text));
    Verify(FPlatformIO::SetMode(FPlatformIO::Stdout, EAccessPolicy::Text));
    Verify(FPlatformIO::SetMode(FPlatformIO::Stderr, EAccessPolicy::Text));
    */
}
//----------------------------------------------------------------------------
void FFileStream::Shutdown() {
    // Force to commit std streams on exit :
    /*
    Verify(FPlatformIO::Commit(FPlatformIO::Stdout));
    Verify(FPlatformIO::Commit(FPlatformIO::Stderr));
    */
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FFileStreamReader::Eof() const {
    Assert(Good());

    return FPlatformIO::Eof(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReader::TellI() const {
    Assert(Good());

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReader::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(Good());

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
std::streamsize FFileStreamReader::SizeInBytes() const {
    Assert(Good());

    const std::streamoff offset = FPlatformIO::Tell(_handle);
    const std::streamoff ate = FPlatformIO::Seek(_handle, 0, ESeekOrigin::End);

    Assert(ate != std::streamoff(-1));

    if (ate != offset) {
        const std::streamoff restored = FPlatformIO::Seek(_handle, offset, ESeekOrigin::Begin);
        Assert(restored == offset);
    }

    Assert(ate >= 0);
    return checked_cast<std::streamsize>(ate);
}
//----------------------------------------------------------------------------
bool FFileStreamReader::Read(void* storage, std::streamsize sizeInBytes) {
    Assert(sizeInBytes);

    std::streamsize read = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return (sizeInBytes == read);
}
//----------------------------------------------------------------------------
size_t FFileStreamReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize read = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return checked_cast<size_t>(read / eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::streamoff FFileStreamWriter::TellO() const {
    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamWriter::SeekO(std::streamoff offset, ESeekOrigin origin) {
    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FFileStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    Assert(sizeInBytes);

    std::streamsize written = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformIO::Write(_handle, storage, sizeInBytes);

    return (sizeInBytes == written);
}
//----------------------------------------------------------------------------
size_t FFileStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize written = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformIO::Write(_handle, storage, written);

    return checked_cast<size_t>(written / eltsize);
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FFileStreamReadWriter::Eof() const {
    Assert(Good());

    return FPlatformIO::Eof(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::TellI() const {
    Assert(Good());

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(Good());

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
std::streamsize FFileStreamReadWriter::SizeInBytes() const {
    Assert(Good());

    const std::streamoff offset = FPlatformIO::Tell(_handle);
    const std::streamoff ate = FPlatformIO::Seek(_handle, 0, ESeekOrigin::End);

    Assert(ate != std::streamoff(-1));

    if (ate != offset) {
        const std::streamoff restored = FPlatformIO::Seek(_handle, offset, ESeekOrigin::Begin);
        Assert(restored == offset);
    }

    Assert(ate >= 0);
    return checked_cast<std::streamsize>(ate);
}
//----------------------------------------------------------------------------
bool FFileStreamReadWriter::Read(void* storage, std::streamsize sizeInBytes) {
    Assert(sizeInBytes);

    std::streamsize read = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return (sizeInBytes == read);
}
//----------------------------------------------------------------------------
size_t FFileStreamReadWriter::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize read = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return checked_cast<size_t>(read / eltsize);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::TellO() const {
    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::SeekO(std::streamoff offset, ESeekOrigin origin) {
    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FFileStreamReadWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    Assert(sizeInBytes);

    std::streamsize written = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformIO::Write(_handle, storage, sizeInBytes);

    return (sizeInBytes == written);
}
//----------------------------------------------------------------------------
size_t FFileStreamReadWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize written = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformIO::Write(_handle, storage, written);

    return checked_cast<size_t>(written / eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
