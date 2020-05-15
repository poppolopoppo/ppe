#pragma once

#include "HAL/Generic/GenericPlatformLaunch.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWindowsPlatformLaunch : public FGenericPlatformLaunch {
public:
    using FGenericPlatformLaunch::RunApplication;

    static PPE_APPLICATION_API void OnPlatformLaunch(
        void* appHandle, int nShowCmd,
        const wchar_t* filename, size_t argc, const wchar_t* const* argv);
    static PPE_APPLICATION_API void OnPlatformShutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
