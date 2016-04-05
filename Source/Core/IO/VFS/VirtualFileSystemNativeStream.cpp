#include "stdafx.h"

#include "VirtualFileSystemNativeStream.h"

#include "VirtualFileSystemTrie.h"

#include "Allocator/PoolAllocator-impl.h"
#include "IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(VirtualFileSystem, VirtualFileSystemNativeFileIStream, );
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(VirtualFileSystem, VirtualFileSystemNativeFileOStream, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool IsValidFileHandle_(FILE *handle) {
    return  handle &&
            0 == ferror(handle);
}
//----------------------------------------------------------------------------
static FILE* OpenFileHandle_(const wchar_t *filename, bool readIfFalseElseWrite, AccessPolicy::Mode policy) {
    Assert(filename);

    FILE* phandle = nullptr;

    STACKLOCAL_WOCSTRSTREAM(openFlags, 32);
    if (readIfFalseElseWrite) {
        // write
        if (AccessPolicy::Create == (policy & AccessPolicy::Create) ||
            AccessPolicy::Truncate == (policy & AccessPolicy::Truncate) )
            openFlags << L'w';
        else
            openFlags << L'a';

        if (AccessPolicy::ShortLived == (policy & AccessPolicy::ShortLived))
            openFlags << L'T'; // Specifies a file as temporary. If possible, it is not flushed to disk.
        else if (AccessPolicy::Temporary == (policy & AccessPolicy::ShortLived))
            openFlags << L'D'; // Specifies a file as temporary. It is deleted when the last file pointer is closed.
    }
    else {
        // read
        openFlags << L'r';
    }

    if (AccessPolicy::Binary == (policy & AccessPolicy::Binary) )
        openFlags << L'b';

    if (AccessPolicy::Random == (policy & AccessPolicy::Random))
        openFlags << L'R'; // Optimize for random access
    else
        openFlags << L'S'; // Optimize for sequential accesss (by default)

    if (::_wfopen_s(&phandle, filename, openFlags.NullTerminatedStr()))
        return nullptr;
    Assert(phandle);

    if (AccessPolicy::Ate == (policy & AccessPolicy::Ate))
        fseek(phandle, 0, SEEK_END);

    Assert(IsValidFileHandle_(phandle));
    return phandle;
}
//----------------------------------------------------------------------------
static void CloseFileHandle_(FILE* phandle) {
    Assert(phandle);
    Assert(IsValidFileHandle_(phandle));

    if (0 != fclose(phandle))
        AssertNotReached();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileIStream::VirtualFileSystemNativeFileIStream(const Filename& filename, const wchar_t* native, AccessPolicy::Mode policy)
:   _handle(OpenFileHandle_(native, false, policy))
,   _filename(filename) {}
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileIStream::~VirtualFileSystemNativeFileIStream() {
    if (false == Bad()) {
        Assert(IsValidFileHandle_(_handle));
        CloseFileHandle_(_handle);
    }
    else {
        Assert(false == IsValidFileHandle_(_handle));
    }
}
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileIStream::VirtualFileSystemNativeFileIStream(VirtualFileSystemNativeFileIStream&& rvalue)
:   _handle(rvalue._handle)
,   _filename(std::move(rvalue._filename)) {
    rvalue._handle = nullptr;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileIStream::Bad() const {
    return (false == IsValidFileHandle_(_handle));
}
//----------------------------------------------------------------------------
std::streamoff VirtualFileSystemNativeFileIStream::TellI() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    return checked_cast<std::streamoff>(_ftelli64(_handle));
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileIStream::SeekI(std::streamoff offset, SeekOrigin origin) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    STATIC_ASSERT(SEEK_SET == int(SeekOrigin::Begin));
    STATIC_ASSERT(SEEK_CUR == int(SeekOrigin::Relative));
    STATIC_ASSERT(SEEK_END == int(SeekOrigin::End));

    if (0 != _fseeki64(_handle, checked_cast<__int64>(offset), int(origin)) )
        return false;

    Assert(IsValidFileHandle_(_handle));
    return true;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileIStream::Read(void* storage, std::streamsize sizeInBytes) {
    return (sizeInBytes == VirtualFileSystemNativeFileIStream::ReadSome(storage, 1, sizeInBytes));
}
//----------------------------------------------------------------------------
std::streamsize VirtualFileSystemNativeFileIStream::ReadSome(void* storage, size_t eltsize, std::streamsize count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));
    Assert(storage);
    Assert(count);

    const std::streamsize read = checked_cast<std::streamsize>(fread(storage, eltsize, checked_cast<size_t>(count), _handle));

    Assert(IsValidFileHandle_(_handle));
    return read;
}
//----------------------------------------------------------------------------
char VirtualFileSystemNativeFileIStream::PeekChar() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    const int ch = fgetc(_handle);
    if (EOF == ch)
        return '\0';

    if (ch != ungetc(ch, _handle))
        AssertNotReached();

    return checked_cast<char>(ch);
}
//----------------------------------------------------------------------------
wchar_t VirtualFileSystemNativeFileIStream::PeekCharW() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    const ::wint_t ch = fgetwc(_handle);
    if (::wint_t(EOF) == ch)
        return '\0';

    if (ch != ungetwc(ch, _handle))
        AssertNotReached();

    return checked_cast<wchar_t>(ch);
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileIStream::Eof() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    return 0 != feof(_handle);
}
//----------------------------------------------------------------------------
std::streamsize VirtualFileSystemNativeFileIStream::SizeInBytes() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    const long long offset = _ftelli64(_handle);

    if (0 != _fseeki64(_handle, 0, SEEK_END))
        AssertNotReached();

    const std::streamsize size = checked_cast<std::streamsize>(_ftelli64(_handle));

    if (0 != _fseeki64(_handle, offset, SEEK_SET))
        AssertNotReached();

    Assert(IsValidFileHandle_(_handle));
    return (size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileOStream::VirtualFileSystemNativeFileOStream(const Filename& filename, const wchar_t* native, AccessPolicy::Mode policy)
:   _handle(OpenFileHandle_(native, true, policy))
,   _filename(filename) {}
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileOStream::~VirtualFileSystemNativeFileOStream() {
    if (false == Bad()) {
        Assert(IsValidFileHandle_(_handle));
        CloseFileHandle_(_handle);
    }
    else {
        Assert(false == IsValidFileHandle_(_handle));
    }
}
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileOStream::VirtualFileSystemNativeFileOStream(VirtualFileSystemNativeFileOStream&& rvalue)
:   _handle(rvalue._handle)
,   _filename(std::move(rvalue._filename)) {
    rvalue._handle = nullptr;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileOStream::Bad() const {
    return (false == IsValidFileHandle_(_handle));
}
//----------------------------------------------------------------------------
std::streamoff VirtualFileSystemNativeFileOStream::TellO() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    return checked_cast<std::streamoff>(_ftelli64(_handle));
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileOStream::SeekO(std::streamoff offset, SeekOrigin origin) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));

    STATIC_ASSERT(SEEK_SET == int(SeekOrigin::Begin));
    STATIC_ASSERT(SEEK_CUR == int(SeekOrigin::Relative));
    STATIC_ASSERT(SEEK_END == int(SeekOrigin::End));

    if (0 != _fseeki64(_handle, checked_cast<long>(offset), int(origin)) )
        return false;

    Assert(IsValidFileHandle_(_handle));
    return true;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileOStream::Write(const void* storage, std::streamsize sizeInBytes) {
    return VirtualFileSystemNativeFileOStream::WriteSome(storage, 1, sizeInBytes);
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileOStream::WriteSome(const void* storage, size_t eltsize, std::streamsize count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsValidFileHandle_(_handle));
    Assert(storage);
    Assert(count);

    if (count != checked_cast<std::streamsize>(fwrite(storage, eltsize, checked_cast<size_t>(count), _handle)))
        return false;

    Assert(IsValidFileHandle_(_handle));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
