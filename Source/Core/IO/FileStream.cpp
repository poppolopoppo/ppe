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
namespace {
//----------------------------------------------------------------------------
constexpr size_t GNativeStreamBufferSize_ = (ALLOCATION_GRANULARITY); // <=> 64kb
//----------------------------------------------------------------------------
struct FNativeStreamReader_ {
    FFileStreamReader Native;
    FBufferedStreamReader Buffered;
    FNativeStreamReader_(FFileStreamReader&& native)
        : Native(std::move(native))
        , Buffered(&Native, GNativeStreamBufferSize_)
    {}
};
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicNativeStreamWriter_ {
    FFileStreamWriter Native;
    FBufferedStreamWriter Buffered;
    TBasicTextWriter<_Char> TextWriter;
    TBasicNativeStreamWriter_(FFileStreamWriter&& native)
        : Native(std::move(native))
        , Buffered(&Native, GNativeStreamBufferSize_)
        , TextWriter(&Buffered)
    {}
};
using FNativeStreamWriter_ = TBasicNativeStreamWriter_<char>;
using FWNativeStreamWriter_ = TBasicNativeStreamWriter_<wchar_t>;
//----------------------------------------------------------------------------
FNativeStreamReader_& StdinReader_() {
    ONE_TIME_INITIALIZE_THREAD_LOCAL(FNativeStreamReader_, GStdin_, FFileStream::OpenStdin(EAccessPolicy::TextU8));
    return GStdin_;
}
//----------------------------------------------------------------------------
FNativeStreamWriter_& StdoutWriter_() {
    ONE_TIME_INITIALIZE_THREAD_LOCAL(FNativeStreamWriter_, GStdout_, FFileStream::OpenStdout(EAccessPolicy::TextU8));
    return GStdout_;
}
//----------------------------------------------------------------------------
FNativeStreamWriter_& StderrWriter_() {
    ONE_TIME_INITIALIZE_THREAD_LOCAL(FNativeStreamWriter_, GStderr_, FFileStream::OpenStderr(EAccessPolicy::TextU8));
    return GStderr_;
}
//----------------------------------------------------------------------------
FWNativeStreamWriter_& WStdoutWriter_() {
    ONE_TIME_INITIALIZE_THREAD_LOCAL(FWNativeStreamWriter_, GWStdout_, FFileStream::OpenStdout(EAccessPolicy::TextW));
    return GWStdout_;
}
//----------------------------------------------------------------------------
FWNativeStreamWriter_& WStderrWriter_() {
    ONE_TIME_INITIALIZE_THREAD_LOCAL(FWNativeStreamWriter_, GWStderr_, FFileStream::OpenStderr(EAccessPolicy::TextW));
    return GWStderr_;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& GStdout = StdoutWriter_().TextWriter;
//----------------------------------------------------------------------------
FTextWriter& GStderr = StderrWriter_().TextWriter;
//----------------------------------------------------------------------------
FWTextWriter& GWStdout = WStdoutWriter_().TextWriter;
//----------------------------------------------------------------------------
FWTextWriter& GWStderr = WStderrWriter_().TextWriter;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFileStream::~FFileStream() {
    if (Good())
        VerifyRelease(FPlatformIO::Close(_handle));
}
//----------------------------------------------------------------------------
FFileStream::FFileStream(FFileStream&& rvalue)
    : _handle(rvalue._handle) {
    rvalue._handle = FPlatformIO::InvalidHandle;
}
//----------------------------------------------------------------------------
FFileStream& FFileStream::operator =(FFileStream&& rvalue) {
    if (Good())
        VerifyRelease(FPlatformIO::Close(_handle));

    Assert(Bad());
    std::swap(_handle, rvalue._handle);

    return (*this);
}
//----------------------------------------------------------------------------
bool FFileStream::Good() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return (FPlatformIO::InvalidHandle != _handle);
}
//----------------------------------------------------------------------------
bool FFileStream::Close() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (Good()) {
        bool succeed = FPlatformIO::Close(_handle);
        _handle = FPlatformIO::InvalidHandle;
        return succeed;
    }
    return false;
}
//----------------------------------------------------------------------------
bool FFileStream::Commit() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());

    return FPlatformIO::Commit(_handle);
}
//----------------------------------------------------------------------------
bool FFileStream::Dup2(FFileStream& other) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());
    Assert(other.Good());

    return FPlatformIO::Dup2(_handle, other._handle);
}
//----------------------------------------------------------------------------
FFileStreamReader FFileStream::OpenRead(const FWStringView& filename, EAccessPolicy flags) {
    Assert(not filename.empty());

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(filename);

    FFileStreamReader reader(FPlatformIO::Open(filenameForDebug.c_str(), EOpenPolicy::Readable, flags));
    reader._filenameForDebug = std::move(filenameForDebug);

    return reader;

#else
    return FFileStreamReader(FPlatformIO::Open(
        NullTerminated(INLINE_MALLOCA(wchar_t, filename.size() + 1), filename),
        EOpenPolicy::Readable,
        flags ));

#endif
}
//----------------------------------------------------------------------------
FFileStreamWriter FFileStream::OpenWrite(const FWStringView& filename, EAccessPolicy flags) {
    Assert(not filename.empty());

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(filename);

    FFileStreamWriter writer(FPlatformIO::Open(filenameForDebug.c_str(), EOpenPolicy::Writable, flags));
    writer._filenameForDebug = std::move(filenameForDebug);

    return writer;

#else
    return FFileStreamWriter(FPlatformIO::Open(
        NullTerminated(INLINE_MALLOCA(wchar_t, filename.size() + 1), filename),
        EOpenPolicy::Writable,
        flags ));

#endif
}
//----------------------------------------------------------------------------
FFileStreamReadWriter FFileStream::OpenReadWrite(const FWStringView& filename, EAccessPolicy flags) {
    Assert(not filename.empty());

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    FWString filenameForDebug;
    filenameForDebug.assign(filename);

    FFileStreamReadWriter readWriter(FPlatformIO::Open(filenameForDebug.c_str(), EOpenPolicy::ReadWritable, flags));
    readWriter._filenameForDebug = std::move(filenameForDebug);

    return readWriter;

#else
    return FFileStreamReadWriter(FPlatformIO::Open(
        NullTerminated(INLINE_MALLOCA(wchar_t, filename.size() + 1), filename),
        EOpenPolicy::ReadWritable,
        flags ));

#endif
}
//----------------------------------------------------------------------------
FFileStreamReader FFileStream::OpenStdin(EAccessPolicy flags) {
    const FPlatformIO::FHandle dup = FPlatformIO::Dup(FPlatformIO::Stdin);
    AssertRelease(FPlatformIO::InvalidHandle != dup);
    VerifyRelease(FPlatformIO::SetMode(dup, flags));

    FFileStreamReader reader(dup);
    Assert(reader.Good());

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    reader._filenameForDebug = StringFormat(L"$stdin[{0}]", flags);
#endif

    return reader;
}
//----------------------------------------------------------------------------
FFileStreamWriter FFileStream::OpenStdout(EAccessPolicy flags) {
    const FPlatformIO::FHandle dup = FPlatformIO::Dup(FPlatformIO::Stdout);
    AssertRelease(FPlatformIO::InvalidHandle != dup);
    VerifyRelease(FPlatformIO::SetMode(dup, flags));

    FFileStreamWriter writer(dup);
    Assert(writer.Good());

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    writer._filenameForDebug = StringFormat(L"$stdout[{0}]", flags);
#endif

    return writer;
}
//----------------------------------------------------------------------------
FFileStreamWriter FFileStream::OpenStderr(EAccessPolicy flags) {
    const FPlatformIO::FHandle dup = FPlatformIO::Dup(FPlatformIO::Stderr);
    AssertRelease(FPlatformIO::InvalidHandle != dup);
    VerifyRelease(FPlatformIO::SetMode(dup, flags));

    FFileStreamWriter writer(dup);
    Assert(writer.Good());

#if WITH_CORE_FILESTREAM_FILENAMEDBG
    writer._filenameForDebug = StringFormat(L"$stderr[{0}]", flags);
#endif

    return writer;
}
//----------------------------------------------------------------------------
FBufferedStreamReader* FFileStream::StdinReader() {
    return (&StdinReader_().Buffered);
}
//----------------------------------------------------------------------------
FBufferedStreamWriter* FFileStream::StdoutWriter() {
    return (&StdoutWriter_().Buffered);
}
//----------------------------------------------------------------------------
FBufferedStreamWriter* FFileStream::StderrWriter() {
    return (&StderrWriter_().Buffered);
}
//----------------------------------------------------------------------------
void FFileStream::Start() {
    // Set std streams mode to wide chars :
    Verify(FPlatformIO::SetMode(FPlatformIO::Stdin, EAccessPolicy::TextU8));
    Verify(FPlatformIO::SetMode(FPlatformIO::Stdout, EAccessPolicy::TextU8));
    Verify(FPlatformIO::SetMode(FPlatformIO::Stderr, EAccessPolicy::TextU8));
}
//----------------------------------------------------------------------------
void FFileStream::Shutdown() {
    // Force to commit std streams on exit :
    Verify(FPlatformIO::Commit(FPlatformIO::Stdout));
    Verify(FPlatformIO::Commit(FPlatformIO::Stderr));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter Stdout() {
    return FTextWriter(FFileStream::StdoutWriter());
}
//----------------------------------------------------------------------------
FTextWriter Stderr() {
    return FTextWriter(FFileStream::StderrWriter());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FFileStreamReader::Eof() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());

    return FPlatformIO::Eof(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReader::TellI() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReader::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
std::streamsize FFileStreamReader::SizeInBytes() const {
    THIS_THREADRESOURCE_CHECKACCESS();
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
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(sizeInBytes);

    std::streamsize read = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return (sizeInBytes == read);
}
//----------------------------------------------------------------------------
size_t FFileStreamReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
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
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamWriter::SeekO(std::streamoff offset, ESeekOrigin origin) {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FFileStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(sizeInBytes);

    std::streamsize written = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformIO::Write(_handle, storage, sizeInBytes);

    return (sizeInBytes == written);
}
//----------------------------------------------------------------------------
size_t FFileStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
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
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());

    return FPlatformIO::Eof(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::TellI() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Good());

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
std::streamsize FFileStreamReadWriter::SizeInBytes() const {
    THIS_THREADRESOURCE_CHECKACCESS();
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
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(sizeInBytes);

    std::streamsize read = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return (sizeInBytes == read);
}
//----------------------------------------------------------------------------
size_t FFileStreamReadWriter::ReadSome(void* storage, size_t eltsize, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(eltsize);
    Assert(count);

    std::streamsize read = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-READ", FilenameForDebug(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return checked_cast<size_t>(read / eltsize);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::TellO() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FFileStreamReadWriter::SeekO(std::streamoff offset, ESeekOrigin origin) {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FFileStreamReadWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(sizeInBytes);

    std::streamsize written = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", FilenameForDebug(), &written);

    written = FPlatformIO::Write(_handle, storage, sizeInBytes);

    return (sizeInBytes == written);
}
//----------------------------------------------------------------------------
size_t FFileStreamReadWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
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
