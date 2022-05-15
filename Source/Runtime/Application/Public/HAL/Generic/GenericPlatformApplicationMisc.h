#pragma once

#include "Application.h"

#include "Color/Color_fwd.h"
#include "Maths/MathHelpers.h"

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

    STATIC_CONST_INTEGRAL(u32, DefaultScreenDPI, 96);

    static float DPIScaleFactor(u32 dpi) {
        return (static_cast<float>(dpi) / DefaultScreenDPI);
    }

    static int ApplyDPIScale(int pixels, u32 dpi) {
        return RoundToInt(static_cast<float>(dpi * pixels) / DefaultScreenDPI);
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
