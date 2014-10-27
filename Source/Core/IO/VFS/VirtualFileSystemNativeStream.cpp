#include "stdafx.h"

#include "VirtualFileSystemNativeStream.h"

namespace Core {
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
static bool OpenFileHandle_(FILE **handle, const wchar_t *filename, bool readIfFalseElseWrite, AccessPolicy::Mode policy) {
    Assert(filename);

    const wchar_t *openFlags = nullptr;

    if (readIfFalseElseWrite) {
        // write
        if (AccessPolicy::Create == (policy & AccessPolicy::Create) ||
            AccessPolicy::Truncate == (policy & AccessPolicy::Truncate) ) {
            openFlags = (AccessPolicy::Binary == (policy & AccessPolicy::Binary) )
                 ? L"wb"
                 : L"w";
        }
        else {
            openFlags = (AccessPolicy::Binary == (policy & AccessPolicy::Binary) )
                 ? L"ab"
                 : L"a";
        }
    }
    else {
        // read

        openFlags = (AccessPolicy::Binary == (policy & AccessPolicy::Binary) )
            ? L"rb"
            : L"r";
    }

    if (_wfopen_s(handle, filename, openFlags))
        return false;
    Assert(*handle);

    if (AccessPolicy::Ate == (policy & AccessPolicy::Ate))
        fseek(*handle, 0, SEEK_END);

    Assert(0 == ferror(*handle));
    return true;
}
//----------------------------------------------------------------------------
static void CloseFileHandle_(FILE **handle) {
    Assert(handle);
    Assert(IsValidFileHandle_(*handle));

    fclose(*handle);

    *handle = nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileIStream::VirtualFileSystemNativeFileIStream(const wchar_t* filename, AccessPolicy::Mode policy)
:   _handle(nullptr) {
    if (!OpenFileHandle_(&_handle, filename, false, policy))
        AssertNotReached();
}
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileIStream::~VirtualFileSystemNativeFileIStream() {
    CloseFileHandle_(&_handle);
}
//----------------------------------------------------------------------------
std::streamoff VirtualFileSystemNativeFileIStream::TellI() {
    Assert(IsValidFileHandle_(_handle));
    return checked_cast<std::streamoff>(_ftelli64(_handle));
}
//----------------------------------------------------------------------------
void VirtualFileSystemNativeFileIStream::SeekI(std::streamoff offset) {
    Assert(IsValidFileHandle_(_handle));
    if (0 != _fseeki64(_handle, checked_cast<__int64>(offset), SEEK_SET))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void VirtualFileSystemNativeFileIStream::Read(void* storage, std::streamsize count) {
    Assert(IsValidFileHandle_(_handle));
    Assert(storage);
    Assert(count);

    const std::streamsize read = checked_cast<std::streamsize>(fread(storage, 1, checked_cast<size_t>(count), _handle));
    Assert(IsValidFileHandle_(_handle));
    AssertRelease(read == count);
}
//----------------------------------------------------------------------------
char VirtualFileSystemNativeFileIStream::PeekChar() {
    Assert(IsValidFileHandle_(_handle));

    const int ch = fgetc(_handle);
    if (EOF == ch)
        return '\0';

    if (ch != ungetc(ch, _handle))
        AssertNotReached();

    return checked_cast<char>(ch);
}
//----------------------------------------------------------------------------
std::streamsize VirtualFileSystemNativeFileIStream::ReadSome(void* storage, std::streamsize count) {
    Assert(IsValidFileHandle_(_handle));
    Assert(storage);
    Assert(count);

    const std::streamsize read = checked_cast<std::streamsize>(fread(storage, 1, checked_cast<size_t>(count), _handle));
    Assert(IsValidFileHandle_(_handle));

    return read;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeFileIStream::Eof() const {
    Assert(IsValidFileHandle_(_handle));
    return 0 != feof(_handle);
}
//----------------------------------------------------------------------------
std::streamsize VirtualFileSystemNativeFileIStream::Size() const {
    Assert(IsValidFileHandle_(_handle));

    const long long offset = _ftelli64(_handle);

    if (0 != _fseeki64(_handle, 0, SEEK_END))
        AssertNotReached();

    const std::streamsize size = checked_cast<std::streamsize>(_ftelli64(_handle));

    if (0 != _fseeki64(_handle, offset, SEEK_SET))
        AssertNotReached();

    return (size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileOStream::VirtualFileSystemNativeFileOStream(const wchar_t* filename, AccessPolicy::Mode policy)
:   _handle(nullptr) {
    if (!OpenFileHandle_(&_handle, filename, true, policy))
        AssertNotReached();
}
//----------------------------------------------------------------------------
VirtualFileSystemNativeFileOStream::~VirtualFileSystemNativeFileOStream() {
    CloseFileHandle_(&_handle);
}
//----------------------------------------------------------------------------
std::streamoff VirtualFileSystemNativeFileOStream::TellO() {
    Assert(IsValidFileHandle_(_handle));
    return checked_cast<std::streamoff>(_ftelli64(_handle));
}
//----------------------------------------------------------------------------
void VirtualFileSystemNativeFileOStream::SeekO(std::streamoff offset) {
    Assert(IsValidFileHandle_(_handle));
    if (0 != _fseeki64(_handle, checked_cast<__int64>(offset), SEEK_SET))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void VirtualFileSystemNativeFileOStream::Write(const void* storage, std::streamsize count) {
    Assert(IsValidFileHandle_(_handle));
    Assert(storage);
    Assert(count);

    if (count != checked_cast<std::streamsize>(fwrite(storage, 1, checked_cast<size_t>(count), _handle)))
        AssertNotReached();

    Assert(IsValidFileHandle_(_handle));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
