// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/FileStream.h"

#include "Allocator/Alloca.h"
#include "IO/BufferedStream.h"
#include "IO/TextWriter.h"

#include "Time/TimedScope.h"

#if WITH_PPE_FILESTREAM_FILENAMEDBG
#   include "IO/Format.h"
#   include "IO/StringBuilder.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFileStream::FFileStream(FFileHandle handle, bool autoClose/* = true */) NOEXCEPT
:   _handle(handle)
,   _autoClose(autoClose) {
    Assert(_handle == handle);
}
//----------------------------------------------------------------------------
FFileStream::~FFileStream() {
    if (Good() && _autoClose)
        VerifyRelease(FPlatformLowLevelIO::Close(_handle));
}
//----------------------------------------------------------------------------
FFileStream::FFileStream(FFileStream&& rvalue) NOEXCEPT
    : _handle(rvalue._handle)
    , _autoClose(rvalue._autoClose)
#if WITH_PPE_FILESTREAM_FILENAMEDBG
    , _filenameForDebug(std::move(rvalue._filenameForDebug))
#endif
{
    rvalue._handle = FPlatformLowLevelIO::InvalidHandle;
}
//----------------------------------------------------------------------------
FFileStream& FFileStream::operator =(FFileStream&& rvalue) NOEXCEPT {
    if (Good() && _autoClose)
        VerifyRelease(FPlatformLowLevelIO::Close(_handle));

    Assert(Bad());
    _handle = rvalue._handle;
    _autoClose = rvalue._autoClose;

#if WITH_PPE_FILESTREAM_FILENAMEDBG
    _filenameForDebug = std::move(rvalue._filenameForDebug);
#endif

    rvalue._handle = FPlatformLowLevelIO::InvalidHandle;
    return (*this);
}
//----------------------------------------------------------------------------
bool FFileStream::Good() const {
    return (FPlatformLowLevelIO::InvalidHandle != _handle);
}
//----------------------------------------------------------------------------
bool FFileStream::Close() {
    if (Good()) {
        bool succeed = FPlatformLowLevelIO::Close(_handle);
        _handle = FPlatformLowLevelIO::InvalidHandle;
        return succeed;
    }
    return false;
}
//----------------------------------------------------------------------------
bool FFileStream::Commit() {
    Assert(Good());

    return FPlatformLowLevelIO::Commit(_handle);
}
//----------------------------------------------------------------------------
bool FFileStream::Dup2(FFileStream& other) const {
    Assert(Good());
    Assert(other.Good());

    return FPlatformLowLevelIO::Dup2(_handle, other._handle);
}
//----------------------------------------------------------------------------
FFileStreamReader FFileStream::OpenRead(const wchar_t* filename, EAccessPolicy flags) {
    Assert(filename);

#if WITH_PPE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(MakeCStringView(filename));

    FFileStreamReader reader(FPlatformLowLevelIO::Open(filenameForDebug.c_str(), EOpenPolicy::Readable, flags));
    reader._filenameForDebug = std::move(filenameForDebug);

    return reader;

#else
    return FFileStreamReader(FPlatformLowLevelIO::Open(filename, EOpenPolicy::Readable, flags));

#endif
}
//----------------------------------------------------------------------------
FFileStreamWriter FFileStream::OpenWrite(const wchar_t* filename, EAccessPolicy flags) {
    Assert(filename);

#if WITH_PPE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(MakeCStringView(filename));

    FFileStreamWriter writer(FPlatformLowLevelIO::Open(filenameForDebug.c_str(), EOpenPolicy::Writable, flags));
    writer._filenameForDebug = std::move(filenameForDebug);

    return writer;

#else
    return FFileStreamWriter(FPlatformLowLevelIO::Open(filename, EOpenPolicy::Writable, flags));

#endif
}
//----------------------------------------------------------------------------
FFileStreamReadWriter FFileStream::OpenReadWrite(const wchar_t* filename, EAccessPolicy flags) {
    Assert(filename);

#if WITH_PPE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(MakeCStringView(filename));

    FFileStreamReadWriter readWriter(FPlatformLowLevelIO::Open(filenameForDebug.c_str(), EOpenPolicy::ReadWritable, flags));
    readWriter._filenameForDebug = std::move(filenameForDebug);

    return readWriter;

#else
    return FFileStreamReadWriter(FPlatformLowLevelIO::Open(filename, EOpenPolicy::ReadWritable, flags));

#endif
}
//----------------------------------------------------------------------------
void FFileStream::Start() {
    // Set std streams mode to utf-8 chars :
    /*
    Verify(FPlatformLowLevelIO::SetMode(FPlatformLowLevelIO::Stdin, EAccessPolicy::Text));
    Verify(FPlatformLowLevelIO::SetMode(FPlatformLowLevelIO::Stdout, EAccessPolicy::Text));
    Verify(FPlatformLowLevelIO::SetMode(FPlatformLowLevelIO::Stderr, EAccessPolicy::Text));
    */
}
//----------------------------------------------------------------------------
void FFileStream::Shutdown() {
    // Force to commit std streams on exit :
    /*
    Verify(FPlatformLowLevelIO::Commit(FPlatformLowLevelIO::Stdout));
    Verify(FPlatformLowLevelIO::Commit(FPlatformLowLevelIO::Stderr));
    */
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FFileStreamReader::Eof() const NOEXCEPT {
    Assert(Good());

    return FPlatformLowLevelIO::Eof(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReader::TellI() const NOEXCEPT {
    Assert(Good());

    return FPlatformLowLevelIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReader::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(Good());

    return FPlatformLowLevelIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
std::streamsize FFileStreamReader::SizeInBytes() const NOEXCEPT {
    Assert(Good());

    const std::streamoff offset = FPlatformLowLevelIO::Tell(_handle);
    const std::streamoff ate = FPlatformLowLevelIO::Seek(_handle, 0, ESeekOrigin::End);

    Assert(ate != std::streamoff(-1));

    if (ate != offset) {
        const std::streamoff restored = FPlatformLowLevelIO::Seek(_handle, offset, ESeekOrigin::Begin);
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

    read = FPlatformLowLevelIO::Read(_handle, storage, read);

    return (sizeInBytes == read);
}
//----------------------------------------------------------------------------
size_t FFileStreamReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize read = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformLowLevelIO::Read(_handle, storage, read);

    return checked_cast<size_t>(read / eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::streamoff FFileStreamWriter::TellO() const NOEXCEPT {
    return FPlatformLowLevelIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamWriter::SeekO(std::streamoff offset, ESeekOrigin origin) {
    return FPlatformLowLevelIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FFileStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    Assert(sizeInBytes);

    std::streamsize written = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformLowLevelIO::Write(_handle, storage, sizeInBytes);

    return (sizeInBytes == written);
}
//----------------------------------------------------------------------------
size_t FFileStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize written = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformLowLevelIO::Write(_handle, storage, written);

    return checked_cast<size_t>(written / eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FFileStreamReadWriter::Eof() const NOEXCEPT {
    Assert(Good());

    return FPlatformLowLevelIO::Eof(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::TellI() const NOEXCEPT {
    Assert(Good());

    return FPlatformLowLevelIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(Good());

    return FPlatformLowLevelIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
std::streamsize FFileStreamReadWriter::SizeInBytes() const NOEXCEPT {
    Assert(Good());

    const std::streamoff offset = FPlatformLowLevelIO::Tell(_handle);
    const std::streamoff ate = FPlatformLowLevelIO::Seek(_handle, 0, ESeekOrigin::End);

    Assert(ate != std::streamoff(-1));

    if (ate != offset) {
        const std::streamoff restored = FPlatformLowLevelIO::Seek(_handle, offset, ESeekOrigin::Begin);
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

    read = FPlatformLowLevelIO::Read(_handle, storage, read);

    return (sizeInBytes == read);
}
//----------------------------------------------------------------------------
size_t FFileStreamReadWriter::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize read = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformLowLevelIO::Read(_handle, storage, read);

    return checked_cast<size_t>(read / eltsize);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::TellO() const NOEXCEPT {
    return FPlatformLowLevelIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::SeekO(std::streamoff offset, ESeekOrigin origin) {
    return FPlatformLowLevelIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FFileStreamReadWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    Assert(sizeInBytes);

    std::streamsize written = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformLowLevelIO::Write(_handle, storage, sizeInBytes);

    return (sizeInBytes == written);
}
//----------------------------------------------------------------------------
size_t FFileStreamReadWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(eltsize);
    Assert(count);

    std::streamsize written = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformLowLevelIO::Write(_handle, storage, written);

    return checked_cast<size_t>(written / eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
