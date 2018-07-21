#pragma once

#include "Core/HAL/Generic/GenericPlatformLowLevelIO.h"

#ifdef PLATFORM_WINDOWS

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FWindowsPlatformLowLevelIO : FGenericPlatformLowLevelIO {
public:

    using FGenericPlatformLowLevelIO::FHandle;

    using FGenericPlatformLowLevelIO::Stdin;
    using FGenericPlatformLowLevelIO::Stdout;
    using FGenericPlatformLowLevelIO::Stderr;
    using FGenericPlatformLowLevelIO::InvalidHandle;

    static bool Access(const wchar_t* entity, EExistPolicy exists);

    static FHandle Open(const wchar_t* filename, EOpenPolicy mode, EAccessPolicy flags);
    static bool Close(FHandle handle);

    static bool SetMode(FHandle handle, EAccessPolicy flags);
    static bool Eof(FHandle handle);

    static std::streamoff Tell(FHandle handle);
    static std::streamoff Seek(FHandle handle, std::streamoff offset, ESeekOrigin origin);

    static std::streamsize Read(FHandle handle, void* dst, std::streamsize sizeInBytes);
    static std::streamsize Write(FHandle handle, const void* src, std::streamsize sizeInBytes);

    static bool Commit(FHandle handle);

    static FHandle Dup(FHandle handle);
    static bool Dup2(FHandle handleSrc, FHandle handleDst);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS
