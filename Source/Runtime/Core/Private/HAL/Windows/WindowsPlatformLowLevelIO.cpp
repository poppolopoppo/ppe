#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformLowLevelIO.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "HAL/PlatformMaths.h"

#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <fcntl.h>
#include <io.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static int OpenPolicyToPMode_(EOpenPolicy openMode) {
    switch (openMode) {
    case EOpenPolicy::Readable: return _S_IREAD;
    case EOpenPolicy::Writable: return _S_IWRITE;
    case EOpenPolicy::ReadWritable: return _S_IREAD | _S_IWRITE;
    default:
        AssertNotImplemented();
    };
}
//----------------------------------------------------------------------------
static int AccessPolicyToOFlag_(EAccessPolicy accessFlags) {
    int oflag = 0;

    if (accessFlags ^ EAccessPolicy::Binary)        oflag |= _O_BINARY;
    if (accessFlags ^ EAccessPolicy::Text)          oflag |= _O_TEXT;
    if (accessFlags ^ EAccessPolicy::TextU8)        oflag |= _O_U8TEXT;
    if (accessFlags ^ EAccessPolicy::TextU16)       oflag |= _O_U16TEXT;
    if (accessFlags ^ EAccessPolicy::TextW)         oflag |= _O_WTEXT;

    Assert(FPlatformMaths::popcnt(size_t(oflag)) <= 1); // non overlapping options !

    if (accessFlags ^ EAccessPolicy::Create)        oflag |= _O_CREAT;
    if (accessFlags ^ EAccessPolicy::Append)        oflag |= _O_APPEND;
    if (accessFlags ^ EAccessPolicy::Truncate)      oflag |= _O_CREAT | _O_TRUNC;

    if (accessFlags ^ EAccessPolicy::Random)        oflag |= _O_RANDOM;
    if (accessFlags ^ EAccessPolicy::Sequential)    oflag |= _O_SEQUENTIAL;

    Assert(FPlatformMaths::popcnt(size_t(oflag) & (_O_RANDOM | _O_SEQUENTIAL)) <= 1); // non overlapping options !

    if (accessFlags ^ EAccessPolicy::ShortLived)    oflag |= _O_SHORT_LIVED;
    if (accessFlags ^ EAccessPolicy::Temporary)     oflag |= _O_TEMPORARY;
    if (accessFlags ^ EAccessPolicy::Exclusive)     oflag |= _O_EXCL;

    Assert(FPlatformMaths::popcnt(size_t(oflag) & (_O_SHORT_LIVED | _O_TEMPORARY)) <= 1); // non overlapping options !
    Assert(!(oflag & (_O_SHORT_LIVED | _O_TEMPORARY | _O_EXCL)) || (oflag & _O_CREAT)); // must use _O_CREAT with these flags !

    return oflag;
}
//----------------------------------------------------------------------------
#ifdef USE_DEBUG_LOGGER
struct FErrno_ {
    int Num;
    static FErrno_ LastError() { return FErrno_{ errno }; }
    inline friend FWTextWriter& operator <<(FWTextWriter& oss, FErrno_ err) {
        wchar_t buffer[1024];
        return (_wcserror_s(buffer, err.Num)
            ? oss << L"unknown error"
            : oss << MakeCStringView(buffer));
    }
};
#endif //!USE_DEBUG_LOGGER
//----------------------------------------------------------------------------
#ifdef USE_DEBUG_LOGGER
struct FHandle_ {
    int Handle;
    inline friend FWTextWriter& operator <<(FWTextWriter& oss, FHandle_ handle) {
        return oss << L'<' << handle.Handle << L'>';
    }
};
#endif //!USE_DEBUG_LOGGER
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWindowsPlatformLowLevelIO::Access(const wchar_t* entity, EExistPolicy exists) {
    int mode = 0;

    //if (exists ^ EExistPolicy::Exists) mode |= 0; // on by default
    if (exists ^ EExistPolicy::WriteOnly)   mode |= 2;
    if (exists ^ EExistPolicy::ReadOnly)    mode |= 4;
    if (exists ^ EExistPolicy::ReadWrite)   mode |= 6;

    return (0 == ::_waccess(entity, mode));
}
//----------------------------------------------------------------------------
auto FWindowsPlatformLowLevelIO::Open(const wchar_t* filename, EOpenPolicy mode, EAccessPolicy flags) -> FHandle {
    int oflag = AccessPolicyToOFlag_(flags);

    switch (mode) {
    case EOpenPolicy::Readable:                     oflag |= _O_RDONLY; break;
    case EOpenPolicy::Writable:                     oflag |= _O_WRONLY; break;
    case EOpenPolicy::ReadWritable:                 oflag |= _O_RDWR; break;
    default:
        AssertNotImplemented();
        break;
    }

    // can allow other process to read the file while reading/writing
    const int shareFlag = (flags ^ EAccessPolicy::ShareRead ? _SH_DENYWR : _SH_DENYRW);

    FHandle handle;
    if (::_wsopen_s(&handle, filename, oflag, shareFlag, OpenPolicyToPMode_(mode)) != 0) {
        LOG(HAL, Error, L"failed to open file '{0}' : {1}", filename, FErrno_::LastError());
        Assert(InvalidHandle == handle);
    }

    LOG(HAL, Info, L"opened file '{0}': {1} ({2}) -> {3}", filename, mode, flags, FHandle_{ handle });
    return handle;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformLowLevelIO::Close(FHandle handle) {
    Assert(InvalidHandle != handle);

    const int res = ::_close(handle);
    CLOG(0 != res, HAL, Error, L"failed to close handle {0} : {1}", FHandle_{ handle }, FErrno_::LastError());

    return (0 == res);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformLowLevelIO::SetMode(FHandle handle, EAccessPolicy flags) {
    Assert(InvalidHandle != handle);

    int oflag = AccessPolicyToOFlag_(flags);

    const int res = _setmode(handle, oflag);
    CLOG(-1 == res, HAL, Error, L"failed to set handle {0} mode to {1} : {2}", FHandle_{ handle }, flags, FErrno_::LastError());

    return (res != -1);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformLowLevelIO::Eof(FHandle handle) {
    Assert(InvalidHandle != handle);

    return (0 < ::_eof(handle));
}
//----------------------------------------------------------------------------
std::streamoff FWindowsPlatformLowLevelIO::Tell(FHandle handle) {
    Assert(InvalidHandle != handle);

    const i64 off = ::_telli64(handle);
    CLOG(-1L == off, HAL, Error, L"failed to tell handle {0} : {1}", FHandle_{ handle }, FErrno_::LastError());

    return off;
}
//----------------------------------------------------------------------------
std::streamoff FWindowsPlatformLowLevelIO::Seek(FHandle handle, std::streamoff offset, ESeekOrigin origin) {
    Assert(InvalidHandle != handle);

    int _origin;
    switch (origin) {
    case ESeekOrigin::Begin:    _origin = SEEK_SET; break;
    case ESeekOrigin::Relative: _origin = SEEK_CUR; break;
    case ESeekOrigin::End:      _origin = SEEK_END; break;
    default:
        AssertNotReached();
    }

    const i64 off = ::_lseeki64(handle, checked_cast<int>(offset), _origin);
    CLOG(-1L == off, HAL, Error, L"failed to seek handle {0} : {1}", FHandle_{ handle }, FErrno_::LastError());

    return off;
}
//----------------------------------------------------------------------------
std::streamsize FWindowsPlatformLowLevelIO::Read(FHandle handle, void* dst, std::streamsize sizeInBytes) {
    Assert(InvalidHandle != handle);
    Assert(dst);
    Assert(sizeInBytes);

    const i64 sz = ::_read(handle, dst, checked_cast<unsigned int>(sizeInBytes));
    CLOG(-1L == sz, HAL, Error, L"failed to read {0} from handle {1} : {2}", Fmt::SizeInBytes(sizeInBytes), FHandle_{ handle }, FErrno_::LastError());

    return sz;
}
//----------------------------------------------------------------------------
std::streamsize FWindowsPlatformLowLevelIO::Write(FHandle handle, const void* src, std::streamsize sizeInBytes) {
    Assert(InvalidHandle != handle);
    Assert(src);
    Assert(sizeInBytes);

    const i64 sz = ::_write(handle, src, checked_cast<unsigned int>(sizeInBytes));
    CLOG(-1L == sz, HAL, Error, L"failed to write {0} to handle {1} : {2}", Fmt::SizeInBytes(sizeInBytes), FHandle_{ handle }, FErrno_::LastError());

    return sz;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformLowLevelIO::Commit(FHandle handle) {
    Assert(InvalidHandle != handle);

    const int res = ::_commit(handle);
    CLOG(0 != res, HAL, Error, L"failed to commit handle {0} : {1}", handle, FErrno_::LastError());

    return (0 == res);
}
//----------------------------------------------------------------------------
auto FWindowsPlatformLowLevelIO::Dup(FHandle handle) -> FHandle {
    Assert(InvalidHandle != handle);

    return ::_dup(handle);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformLowLevelIO::Dup2(FHandle handleSrc, FHandle handleDst) {
    Assert(InvalidHandle != handleSrc);
    Assert(InvalidHandle != handleDst);

    return (0 != _dup2(handleSrc, handleDst));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
