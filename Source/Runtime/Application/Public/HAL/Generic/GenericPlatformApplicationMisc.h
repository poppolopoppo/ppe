#pragma once

#include "Application.h"

#include "Color/Color_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlatformApplicationMisc {
public: // must be defined for every platform

    static void Start() = delete;
    static void Shutdown() = delete;

    static bool PickScreenColorAt(int x, int y, FColor* color) = delete;
    static bool PickScreenColorUnderMouse(FColor* color) = delete;
    static void PreventScreenSaver() = delete;
    static bool SetHighDPIAwareness() = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
