#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformApplicationMisc.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Linux/LinuxPlatformNotification.h"
#include "HAL/Linux/LinuxWindow.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLinuxPlatformApplicationMisc::Start() {
    FLinuxPlatformNotification::Start();
}
//----------------------------------------------------------------------------
void FLinuxPlatformApplicationMisc::Shutdown() {
    FLinuxPlatformNotification::Shutdown();
}
//----------------------------------------------------------------------------
bool FLinuxPlatformApplicationMisc::PickScreenColorAt(int x, int y, FColor* color) {
    UNUSED(x);
    UNUSED(y);
    UNUSED(color);
    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformApplicationMisc::PickScreenColorUnderMouse(FColor* color) {
    UNUSED(color);
    return false;
}
//----------------------------------------------------------------------------
void FLinuxPlatformApplicationMisc::PreventScreenSaver() {
    
}
//----------------------------------------------------------------------------
bool FLinuxPlatformApplicationMisc::SetHighDPIAwareness() {
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
