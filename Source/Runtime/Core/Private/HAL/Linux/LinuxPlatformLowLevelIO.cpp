#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformLowLevelIO.h"

#ifdef PLATFORM_LINUX

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "HAL/PlatformMaths.h"

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"
#include "HAL/Linux/LinuxPlatformFile.h"

#include <fcntl.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static int OpenPolicyToPMode_(EOpenPolicy openMode) {
    switch (openMode) {
    case EOpenPolicy::Readable: return O_RDONLY;
    case EOpenPolicy::Writable: return O_WRONLY;
    case EOpenPolicy::ReadWritable: return O_RDWR;
    default:
        AssertNotImplemented();
    };
}
//----------------------------------------------------------------------------
static int AccessPolicyToOFlag_(EAccessPolicy accessFlags) {
    int oflag = O_DIRECT | O_SYNC;

    Assert(FPlatformMaths::popcnt(size_t(oflag)) <= 1); // non overlapping options !

    if (accessFlags ^ EAccessPolicy::Create)        oflag |= O_CREAT;
    if (accessFlags ^ EAccessPolicy::Append)        oflag |= O_APPEND;
    if (accessFlags ^ EAccessPolicy::Truncate)      oflag |= O_CREAT | O_TRUNC;

    return oflag;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLinuxPlatformLowLevelIO::Access(const wchar_t* entity, EExistPolicy exists) {
    int mode = 0;

    //if (exists ^ EExistPolicy::Exists) mode |= 0; // on by default
    if (exists ^ EExistPolicy::WriteOnly)   mode |= S_IRUSR;
    if (exists ^ EExistPolicy::ReadOnly)    mode |= S_IWUSR;
    if (exists ^ EExistPolicy::ReadWrite)   mode |= S_IWUSR|S_IRUSR;

    return (0 == ::access(WCHAR_TO_UTF_8<FLinuxPlatformFile::MaxPathLength>(entity), mode));
}
//----------------------------------------------------------------------------
auto FLinuxPlatformLowLevelIO::Open(const wchar_t* filename, EOpenPolicy mode, EAccessPolicy flags) -> FHandle {
    int fd = ::open(
        WCHAR_TO_UTF_8<FLinuxPlatformFile::MaxPathLength>(filename),
        AccessPolicyToOFlag_(flags) | OpenPolicyToPMode_(mode),
        S_IWUSR|S_IRUSR );

    if (-1 != fd) {
        LOG(HAL, Info, L"opened file '{0}': {1} ({2}) -> {3}", filename, mode, flags, fd);
    }
    else {
        LOG(HAL, Error, L"open({0}) failed with errno: {1}",
            MakeCStringView(filename), FErrno{} );
        Assert(InvalidHandle == fd);
    }

    return fd;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformLowLevelIO::Close(FHandle handle) {
    Assert(InvalidHandle != handle);

    const int res = ::close(handle);
    CLOG(0 != res, HAL, Error, L"failed to close handle {0} : {1}", handle, FErrno{});

    return (0 == res);
}
//----------------------------------------------------------------------------
bool FLinuxPlatformLowLevelIO::SetMode(FHandle handle, EAccessPolicy flags) {
    Assert(InvalidHandle != handle);
    UNUSED(handle);
    UNUSED(flags);
    return true; // irrelevant on linux
}
//----------------------------------------------------------------------------
bool FLinuxPlatformLowLevelIO::Eof(FHandle handle) {
    Assert(InvalidHandle != handle);
    const i64 old = ::lseek64(handle, 0, SEEK_END);
    const i64 end = ::lseek64(handle, old, SEEK_SET);
    return ((old != -1) & (old == end));
}
//----------------------------------------------------------------------------
std::streamoff FLinuxPlatformLowLevelIO::Tell(FHandle handle) {
    Assert(InvalidHandle != handle);

    const i64 off = ::lseek64(handle, 0, SEEK_CUR);
    CLOG(-1L == off, HAL, Error, L"lseek64(0) failed to tell handle {0} with errno: {1}", handle, FErrno{});

    return off;
}
//----------------------------------------------------------------------------
std::streamoff FLinuxPlatformLowLevelIO::Seek(FHandle handle, std::streamoff offset, ESeekOrigin origin) {
    Assert(InvalidHandle != handle);

    int _origin;
    switch (origin) {
    case ESeekOrigin::Begin:    _origin = SEEK_SET; break;
    case ESeekOrigin::Relative: _origin = SEEK_CUR; break;
    case ESeekOrigin::End:      _origin = SEEK_END; break;
    default:
        AssertNotReached();
    }

    const i64 off = ::lseek64(handle, checked_cast<loff_t>(offset), _origin);
    CLOG(-1L == off, HAL, Error, L"lseek64(0) failed to seek handle {0}@{1:X} with errno: {2}", handle, offset, FErrno{});

    return off;
}
//----------------------------------------------------------------------------
std::streamsize FLinuxPlatformLowLevelIO::Read(FHandle handle, void* dst, std::streamsize sizeInBytes) {
    Assert(InvalidHandle != handle);
    Assert(dst);
    Assert(sizeInBytes);

    const i64 sz = ::read(handle, dst, checked_cast<size_t>(sizeInBytes));
    CLOG(-1L == sz, HAL, Error, L"failed to read {0} from handle {1} : {2}", Fmt::SizeInBytes(sizeInBytes), handle, FErrno());

    return sz;
}
//----------------------------------------------------------------------------
std::streamsize FLinuxPlatformLowLevelIO::Write(FHandle handle, const void* src, std::streamsize sizeInBytes) {
    Assert(InvalidHandle != handle);
    Assert(src);
    Assert(sizeInBytes);

    const i64 sz = ::write(handle, src, checked_cast<size_t>(sizeInBytes));
    CLOG(-1L == sz, HAL, Error, L"failed to write {0} to handle {1} : {2}", Fmt::SizeInBytes(sizeInBytes), handle, FErrno());

    return sz;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformLowLevelIO::Commit(FHandle handle) {
    Assert(InvalidHandle != handle);
    UNUSED(handle);
    return true; // irrelevant for linux
}
//----------------------------------------------------------------------------
auto FLinuxPlatformLowLevelIO::Dup(FHandle handle) -> FHandle {
    Assert(InvalidHandle != handle);

    return ::dup(handle);
}
//----------------------------------------------------------------------------
bool FLinuxPlatformLowLevelIO::Dup2(FHandle handleSrc, FHandle handleDst) {
    Assert(InvalidHandle != handleSrc);
    Assert(InvalidHandle != handleDst);

    return (0 != ::dup2(handleSrc, handleDst));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
