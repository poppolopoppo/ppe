#pragma once

#include "HAL/TargetPlatform.h"

#include "IO/StreamPolicies.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformLowLevelIO {
public: // must be defined for every platform

    using FHandle = int;

    static constexpr FHandle Stdin = 0;
    static constexpr FHandle Stdout = 1;
    static constexpr FHandle Stderr = 2;
    static constexpr FHandle InvalidHandle = FHandle(-1);

    static bool Access(const wchar_t* entity, EExistPolicy exists) = delete;

    static FHandle Open(const wchar_t* filename, EOpenPolicy mode, EAccessPolicy flags) = delete;
    static bool Close(FHandle handle) = delete;

    static bool SetMode(FHandle handle, EAccessPolicy flags) = delete;
    static bool Eof(FHandle handle) = delete;

    static std::streamoff Tell(FHandle handle) = delete;
    static std::streamoff Seek(FHandle handle, std::streamoff offset, ESeekOrigin origin) = delete;

    static std::streamsize Read(FHandle handle, void* dst, std::streamsize sizeInBytes) = delete;
    static std::streamsize Write(FHandle handle, const void* src, std::streamsize sizeInBytes) = delete;

    static bool Commit(FHandle handle) = delete;

    static FHandle Dup(FHandle handle) = delete;
    static bool Dup2(FHandle handleSrc, FHandle handleDst) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
