#pragma once

#include "Application.h"

#include "HAL/Generic/GenericPlatformApplicationMisc.h"

#ifndef PLATFORM_LINUX
#   error "invalid include for current platform"
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FLinuxPlatformApplicationMisc : public FGenericPlatformApplicationMisc {
public: // must be defined for every platform

    static void Start();
    static void Shutdown();

    static bool PickScreenColorAt(int x, int y, FColor* color);
    static bool PickScreenColorUnderMouse(FColor* color);
    static void PreventScreenSaver();
    static bool SetHighDPIAwareness();

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
