#pragma once

#include "Application.h"

#include "HAL/Generic/GenericPlatformApplicationMisc.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FWindowsPlatformApplicationMisc : public FGenericPlatformApplicationMisc {
public: // must be defined for every platform

    static void Start();
    static void Shutdown();

    static bool PickScreenColorAt(int x, int y, FColor* color);
    static bool PickScreenColorUnderMouse(FColor* color);
    static void PreventScreenSaver();

    static bool SetHighDPIAwareness();

    using FGenericPlatformApplicationMisc::DPIScaleFactor;
    static int ApplyDPIScale(int pixels, u32 dpi);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
