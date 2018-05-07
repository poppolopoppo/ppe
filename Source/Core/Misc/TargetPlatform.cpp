#include "stdafx.h"

#include "TargetPlatform.h"

#include "Diagnostic/DbgHelpWrapper.h"
#include "Diagnostic/Logger.h"
#include "Diagnostic/MiniDump.h"
#include "IO/TextWriter.h"

#ifdef USE_DEBUG_LOGGER
#   include "IO/FormatHelpers.h"
#endif

#include <errno.h>

#ifdef PLATFORM_WINDOWS
#   include "Misc/Platform_Windows.h"
#   include <fcntl.h>
#   include <io.h>
#   include <wchar.h>
#endif

#define USE_CORE_DEBUGGER_PRESENT 1 // set to 0 to fake non-attached to a debugger behavior %_NOCOMMIT%

#ifndef ARCH_X64
#   define USE_VECTORED_EXCEPTION_HANDLER 0//1 // bad idea : vectored exception are used internally by windows
#else
#   define USE_VECTORED_EXCEPTION_HANDLER 0
#endif

namespace Core {
LOG_CATEGORY(CORE_API, Platform)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr ETargetPlatform GTargetPlatforms[] = {
    ETargetPlatform::PC,
    ETargetPlatform::PS4,
    ETargetPlatform::XONE,
    ETargetPlatform::MAC,
    ETargetPlatform::LINUX
};
//----------------------------------------------------------------------------
#if     defined(PLATFORM_WINDOWS)
struct FWindowsSystemInfo_ : FPlatformMisc::FSystemInfo {
    FWindowsSystemInfo_() {
        ::SYSTEM_INFO st;
        ::GetSystemInfo(&st);

        AllocationGranularity = checked_cast<size_t>(st.dwAllocationGranularity);
        PageSize = checked_cast<size_t>(st.dwPageSize);
        ProcessorsCount = checked_cast<size_t>(st.dwNumberOfProcessors);

        AssertRelease(PAGE_SIZE == PageSize);
        AssertRelease(ALLOCATION_GRANULARITY == AllocationGranularity);
    }
};
static const FWindowsSystemInfo_ GSystemInfo;
#else
#   error "unsupported platform"
#endif
//----------------------------------------------------------------------------
#ifdef USE_DEBUG_LOGGER
struct FErrno_ {
    int Num;
    static FErrno_ LastError() { return FErrno_{ errno }; }
    inline friend FWTextWriter& operator <<(FWTextWriter& oss, FErrno_ err) {
        wchar_t buffer[1024];
        return (_wcserror_s(buffer, err.Num)
            ? oss << L"unknown error"
            : oss << MakeCStringView(buffer) );
    }
};
#endif //!USE_DEBUG_LOGGER
//----------------------------------------------------------------------------
#ifdef USE_DEBUG_LOGGER
struct FHandle_ {
    FPlatformIO::FHandle Handle;
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
TMemoryView<const ETargetPlatform> EachTargetPlatform() {
    return MakeView(GTargetPlatforms);
}
//----------------------------------------------------------------------------
FStringView TargetPlatformToCStr(ETargetPlatform platform) {
    switch (platform)
    {
    case Core::ETargetPlatform::PC:
        return FStringView("PC");
    case Core::ETargetPlatform::PS4:
        return FStringView("PS4");
    case Core::ETargetPlatform::XONE:
        return FStringView("XONE");
    case Core::ETargetPlatform::MAC:
        return FStringView("MAC");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FWStringView TargetPlatformToWCStr(ETargetPlatform platform) {
    switch (platform)
    {
    case Core::ETargetPlatform::PC:
        return FWStringView(L"PC");
    case Core::ETargetPlatform::PS4:
        return FWStringView(L"PS4");
    case Core::ETargetPlatform::XONE:
        return FWStringView(L"XONE");
    case Core::ETargetPlatform::MAC:
        return FWStringView(L"MAC");
    default:
        AssertNotImplemented();
    }
    return FWStringView();
}
//----------------------------------------------------------------------------
EEndianness TargetPlatformEndianness(ETargetPlatform platform) {
    switch (platform)
    {
    case Core::ETargetPlatform::PC:
        return EEndianness::LittleEndian;
    case Core::ETargetPlatform::PS4:
        return EEndianness::LittleEndian;
    case Core::ETargetPlatform::XONE:
        return EEndianness::LittleEndian;
    case Core::ETargetPlatform::MAC:
        return EEndianness::LittleEndian;
    default:
        AssertNotImplemented();
    }
    return EEndianness::LittleEndian;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, ETargetPlatform platform) {
    return oss << TargetPlatformToCStr(platform);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, ETargetPlatform platform) {
    return oss << TargetPlatformToWCStr(platform);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FPlatformMisc::FSystemInfo& FPlatformMisc::SystemInfo = GSystemInfo;
//----------------------------------------------------------------------------
void FPlatformMisc::Sleep(size_t ms) {
#ifdef PLATFORM_WINDOWS
    if (ms)
        ::Sleep((::DWORD)ms);
    else
        ::SwitchToThread();
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatformMisc::CheckMemory() {
#ifdef PLATFORM_WINDOWS
    _CrtCheckMemory();
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatformMisc::DebugBreak() {
#ifdef PLATFORM_WINDOWS
    ::DebugBreak();
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatformMisc::DebugBreakAttach() {
#ifdef PLATFORM_WINDOWS
    if (::IsDebuggerPresent()) {
        ::DebugBreak();
    }
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
bool FPlatformMisc::IsDebuggerAttached() {
#if USE_CORE_DEBUGGER_PRESENT
#   ifdef PLATFORM_WINDOWS
        return ::IsDebuggerPresent() ? true : false;
#   else
#       error "no support"
#   endif
#else
    return false;
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatformMisc::OutputDebug(const char* text) {
#ifdef PLATFORM_WINDOWS
    return ::OutputDebugStringA(text);
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void FPlatformMisc::OutputDebug(const wchar_t* text) {
#ifdef PLATFORM_WINDOWS
    return ::OutputDebugStringW(text);
#else
#   error "no support"
#endif
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
static int OpenPolicyToPMode_(EOpenPolicy openMode) {
    switch (openMode) {
    case EOpenPolicy::Readable: return _S_IREAD;
    case EOpenPolicy::Writable: return _S_IWRITE;
    case EOpenPolicy::ReadWritable: return _S_IREAD | _S_IWRITE;
    };
    AssertNotImplemented();
    return -1;
}
#endif
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
static int AccessPolicyToOFlag_(EAccessPolicy accessFlags) {
    int oflag = 0;

    if (accessFlags ^ EAccessPolicy::Binary)        oflag |= _O_BINARY;
    if (accessFlags ^ EAccessPolicy::Text)          oflag |= _O_TEXT;
    if (accessFlags ^ EAccessPolicy::TextU8)        oflag |= _O_U8TEXT;
    if (accessFlags ^ EAccessPolicy::TextU16)       oflag |= _O_U16TEXT;
    if (accessFlags ^ EAccessPolicy::TextW)         oflag |= _O_WTEXT;

    Assert(Meta::popcnt(size_t(oflag)) <= 1); // non overlapping options !

    if (accessFlags ^ EAccessPolicy::Create)        oflag |= _O_CREAT;
    if (accessFlags ^ EAccessPolicy::Append)        oflag |= _O_APPEND;
    if (accessFlags ^ EAccessPolicy::Truncate)      oflag |= _O_CREAT | _O_TRUNC;

    if (accessFlags ^ EAccessPolicy::Random)        oflag |= _O_RANDOM;
    if (accessFlags ^ EAccessPolicy::Sequential)    oflag |= _O_SEQUENTIAL;

    Assert(Meta::popcnt(size_t(oflag) & (_O_RANDOM | _O_SEQUENTIAL)) <= 1); // non overlapping options !

    if (accessFlags ^ EAccessPolicy::ShortLived)    oflag |= _O_SHORT_LIVED;
    if (accessFlags ^ EAccessPolicy::Temporary)     oflag |= _O_TEMPORARY;
    if (accessFlags ^ EAccessPolicy::Exclusive)     oflag |= _O_EXCL;

    Assert(Meta::popcnt(size_t(oflag) & (_O_SHORT_LIVED | _O_TEMPORARY)) <= 1); // non overlapping options !
    Assert(!(oflag & (_O_SHORT_LIVED | _O_TEMPORARY | _O_EXCL)) || (oflag & _O_CREAT)); // must use _O_CREAT with these flags !

    return oflag;
}
#endif
//----------------------------------------------------------------------------
bool FPlatformIO::Access(const wchar_t* entity, EExistPolicy exists) {
#ifdef PLATFORM_WINDOWS
    int mode = 0;

    //if (exists ^ EExistPolicy::Exists) mode |= 0; // on by default
    if (exists ^ EExistPolicy::WriteOnly)   mode |= 2;
    if (exists ^ EExistPolicy::ReadOnly)    mode |= 4;
    if (exists ^ EExistPolicy::ReadWrite)   mode |= 6;

    return (0 == ::_waccess(entity, mode));
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
auto FPlatformIO::Open(const wchar_t* filename, EOpenPolicy openMode, EAccessPolicy accessFlags) -> FHandle {
#ifdef PLATFORM_WINDOWS
    int oflag = AccessPolicyToOFlag_(accessFlags);

    switch (openMode) {
    case EOpenPolicy::Readable:                     oflag |= _O_RDONLY; break;
    case EOpenPolicy::Writable:                     oflag |= _O_WRONLY; break;
    case EOpenPolicy::ReadWritable:                 oflag |= _O_RDWR; break;
    default:
        AssertNotImplemented();
        break;
    }

    // can allow other process to read the file while reading/writing
    const int shareFlag = (accessFlags ^ EAccessPolicy::ShareRead ? _SH_DENYWR : _SH_DENYRW);

    FHandle handle;
    if (::_wsopen_s(&handle, filename, oflag, shareFlag, OpenPolicyToPMode_(openMode)) != 0) {
        LOG(Platform, Error, L"failed to open file '{0}' : {1}", filename, FErrno_::LastError());
        Assert(InvalidHandle == handle);
    }
    LOG(Platform, Info, L"opened file '{0}': {1} ({2}) -> {3}", filename, openMode, accessFlags, FHandle_{ handle });
    return handle;
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
bool FPlatformIO::SetMode(FHandle handle, EAccessPolicy accessFlags) {
#ifdef PLATFORM_WINDOWS
    int oflag = AccessPolicyToOFlag_(accessFlags);
    const int res = _setmode(handle, oflag);
    CLOG(-1 == res, Platform, Error, L"failed to set handle {0} mode to {1} : {2}", FHandle_{ handle }, accessFlags, FErrno_::LastError());
    return (res != -1);
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
bool FPlatformIO::Close(FHandle handle) {
    Assert(InvalidHandle != handle);
#ifdef PLATFORM_WINDOWS
    const int res = ::_close(handle);
    CLOG(0 != res, Platform, Error, L"failed to close handle {0} : {1}", FHandle_{ handle }, FErrno_::LastError());
    return (0 == res);
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
bool FPlatformIO::Eof(FHandle handle) {
    Assert(InvalidHandle != handle);
#ifdef PLATFORM_WINDOWS
    return (0 < ::_eof(handle));
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
std::streamoff FPlatformIO::Tell(FHandle handle) {
    Assert(InvalidHandle != handle);
#ifdef PLATFORM_WINDOWS
    const i64 off = ::_telli64(handle);
    CLOG(-1L == off, Platform, Error, L"failed to tell handle {0} : {1}", FHandle_{ handle }, FErrno_::LastError());
    return off;
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
std::streamoff FPlatformIO::Seek(FHandle handle, std::streamoff offset, ESeekOrigin origin) {
    Assert(InvalidHandle != handle);
#ifdef PLATFORM_WINDOWS
    int _origin;
    switch (origin) {
    case ESeekOrigin::Begin:    _origin = SEEK_SET; break;
    case ESeekOrigin::Relative: _origin = SEEK_CUR; break;
    case ESeekOrigin::End:      _origin = SEEK_END; break;
    default:
        AssertNotImplemented();
        _origin = 0;
        break;
    }

    const i64 off = ::_lseeki64(handle, checked_cast<int>(offset), _origin);
    CLOG(-1L == off, Platform, Error, L"failed to seek handle {0} : {1}", FHandle_{ handle }, FErrno_::LastError());
    return off;
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
std::streamsize FPlatformIO::Read(FHandle handle, void* dst, std::streamsize sizeInBytes) {
    Assert(InvalidHandle != handle);
    Assert(dst);
    Assert(sizeInBytes);
#ifdef PLATFORM_WINDOWS
    const i64 sz = ::_read(handle, dst, checked_cast<unsigned int>(sizeInBytes));
    CLOG(-1L == sz, Platform, Error, L"failed to read {0} from handle {1} : {2}", Fmt::SizeInBytes(sizeInBytes), FHandle_{ handle }, FErrno_::LastError());
    return sz;
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
std::streamsize FPlatformIO::Write(FHandle handle, const void* src, std::streamsize sizeInBytes) {
    Assert(InvalidHandle != handle);
    Assert(src);
    Assert(sizeInBytes);
#ifdef PLATFORM_WINDOWS
    const i64 sz = ::_write(handle, src, checked_cast<unsigned int>(sizeInBytes));
    CLOG(-1L == sz, Platform, Error, L"failed to write {0} to handle {1} : {2}", Fmt::SizeInBytes(sizeInBytes), FHandle_{ handle }, FErrno_::LastError());
    return sz;
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
bool FPlatformIO::Commit(FHandle handle) {
    Assert(InvalidHandle != handle);
#ifdef PLATFORM_WINDOWS
    const int res = ::_commit(handle);
    CLOG(0 != res, Platform, Error, L"failed to commit handle {0} : {1}", handle, FErrno_::LastError());
    return (0 == res);
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
auto FPlatformIO::Dup(FHandle handle) -> FHandle {
    Assert(InvalidHandle != handle);
#ifdef PLATFORM_WINDOWS
    return ::_dup(handle);
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
bool FPlatformIO::Dup2(FHandle handleSrc, FHandle handleDst) {
    Assert(InvalidHandle != handleSrc);
    Assert(InvalidHandle != handleDst);
#ifdef PLATFORM_WINDOWS
    return (0 != _dup2(handleSrc, handleDst));
#else
#   error "no support"
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
#ifdef PLATFORM_WINDOWS
static void DumpException_(PEXCEPTION_POINTERS pExceptionInfo) {
    time_t tNow = ::time(NULL);
    struct tm pTm;
    ::localtime_s(&pTm, &tNow);

    wchar_t filename[MAX_PATH];
    {
        wchar_t modulePath[MAX_PATH];
        if (0 == ::GetModuleFileNameW(NULL, modulePath, MAX_PATH))
            return;

        ::swprintf_s(filename, MAX_PATH,
            L"%s.%02d%02d%04d_%02d%02d%02d.dmp",
            modulePath,
            pTm.tm_year, pTm.tm_mon, pTm.tm_mday,
            pTm.tm_hour, pTm.tm_min, pTm.tm_sec);
    }

    MiniDump::Write(filename, MiniDump::InfoLevel::Large, pExceptionInfo, true);

    ::abort(); // abort program execution forcibly !
}
LONG WINAPI OnUnhandledException_(PEXCEPTION_POINTERS pExceptionInfo) {
    DumpException_(pExceptionInfo);
    return EXCEPTION_CONTINUE_EXECUTION;
}
#   if USE_VECTORED_EXCEPTION_HANDLER
static volatile LPVOID GHandleVectoredExceptionHandler = nullptr;
LONG WINAPI OnVectoredHandler_(PEXCEPTION_POINTERS pExceptionInfo) {
    DumpException_(pExceptionInfo);
    return EXCEPTION_CONTINUE_EXECUTION;
}
#   endif //!USE_VECTORED_EXCEPTION_HANDLER
#endif //!PLATFORM_WINDOWS
} //!namespace
//----------------------------------------------------------------------------
void FPlatformCrashDump::SetExceptionHandlers() {
#ifdef PLATFORM_WINDOWS
#   if USE_VECTORED_EXCEPTION_HANDLER
    ::AddVectoredExceptionHandler(0, &OnVectoredHandler_);
#   endif
    ::SetUnhandledExceptionFilter(&OnUnhandledException_);
#endif
}
//----------------------------------------------------------------------------
void FPlatformCrashDump::AbortProgramWithDump() {
    throw std::runtime_error("abort program with dump");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
