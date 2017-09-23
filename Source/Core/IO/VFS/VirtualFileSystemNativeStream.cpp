#include "stdafx.h"

#include "VirtualFileSystemNativeStream.h"

#include "VirtualFileSystemTrie.h"

#include "Allocator/PoolAllocator-impl.h"
#include "IO/VirtualFileSystem.h"
#include "Misc/TargetPlatform.h"
#include "Time/TimedScope.h"

#ifndef FINAL_RELEASE
#   include "IO/Format.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(VirtualFileSystem, FVirtualFileSystemNativeFileIStream, );
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(VirtualFileSystem, FVirtualFileSystemNativeFileOStream, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemNativeFileIStream::FVirtualFileSystemNativeFileIStream(const FFilename& filename, const wchar_t* native, EAccessPolicy policy)
:   _handle(FPlatformIO::Open(native, EOpenPolicy::Readable, policy))
,   _filename(filename) {}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeFileIStream::~FVirtualFileSystemNativeFileIStream() {
    if (FPlatformIO::InvalidHandle != _handle)
        FPlatformIO::Close(_handle);
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeFileIStream::FVirtualFileSystemNativeFileIStream(FVirtualFileSystemNativeFileIStream&& rvalue)
:   _handle(rvalue._handle)
,   _filename(std::move(rvalue._filename)) {
    rvalue._handle = FPlatformIO::InvalidHandle;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeFileIStream::Bad() const {
    return (FPlatformIO::InvalidHandle == _handle);
}
//----------------------------------------------------------------------------
std::streamoff FVirtualFileSystemNativeFileIStream::TellI() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FVirtualFileSystemNativeFileIStream::SeekI(std::streamoff offset, ESeekOrigin origin) {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeFileIStream::Read(void* storage, std::streamsize sizeInBytes) {
    THIS_THREADRESOURCE_CHECKACCESS();

    std::streamsize read = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-READ", INPLACE_TO_WSTRINGVIEW(_filename, FileSystem::MaxPathLength).c_str(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return (sizeInBytes == read);
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeFileIStream::ReadSome(void* storage, size_t eltsize, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();

    std::streamsize read = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-READ", INPLACE_TO_WSTRINGVIEW(_filename, FileSystem::MaxPathLength).c_str(), &read);

    read = FPlatformIO::Read(_handle, storage, read);

    return checked_cast<size_t>(read / eltsize);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeFileIStream::Eof() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Eof(_handle);
}
//----------------------------------------------------------------------------
std::streamsize FVirtualFileSystemNativeFileIStream::SizeInBytes() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(FPlatformIO::InvalidHandle != _handle);

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualFileSystemNativeFileOStream::FVirtualFileSystemNativeFileOStream(const FFilename& filename, const wchar_t* native, EAccessPolicy policy)
:   _handle(FPlatformIO::Open(native, EOpenPolicy::Writable, policy))
,   _filename(filename) {}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeFileOStream::~FVirtualFileSystemNativeFileOStream() {
    if (FPlatformIO::InvalidHandle != _handle)
        FPlatformIO::Close(_handle);
}
//----------------------------------------------------------------------------
FVirtualFileSystemNativeFileOStream::FVirtualFileSystemNativeFileOStream(FVirtualFileSystemNativeFileOStream&& rvalue)
:   _handle(rvalue._handle)
,   _filename(std::move(rvalue._filename)) {
    rvalue._handle = FPlatformIO::InvalidHandle;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeFileOStream::Bad() const {
    return (FPlatformIO::InvalidHandle == _handle);
}
//----------------------------------------------------------------------------
std::streamoff FVirtualFileSystemNativeFileOStream::TellO() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Tell(_handle);
}
//----------------------------------------------------------------------------
std::streamoff FVirtualFileSystemNativeFileOStream::SeekO(std::streamoff offset, ESeekOrigin origin) {
    THIS_THREADRESOURCE_CHECKACCESS();

    return FPlatformIO::Seek(_handle, offset, origin);
}
//----------------------------------------------------------------------------
bool FVirtualFileSystemNativeFileOStream::Write(const void* storage, std::streamsize sizeInBytes) {
    THIS_THREADRESOURCE_CHECKACCESS();

    std::streamsize written = sizeInBytes;
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", INPLACE_TO_WSTRINGVIEW(_filename, FileSystem::MaxPathLength).c_str(), &written);

    written = FPlatformIO::Write(_handle, storage, sizeInBytes);

    return (sizeInBytes == written);
}
//----------------------------------------------------------------------------
size_t FVirtualFileSystemNativeFileOStream::WriteSome(const void* storage, size_t eltsize, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(eltsize);
    Assert(count);

    std::streamsize written = checked_cast<std::streamsize>(eltsize * count);
    IOBENCHMARK_SCOPE(L"NATIVE-WRITE", INPLACE_TO_WSTRINGVIEW(_filename, FileSystem::MaxPathLength).c_str(), &written);

    written = FPlatformIO::Write(_handle, storage, written);

    return checked_cast<size_t>(written / eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
