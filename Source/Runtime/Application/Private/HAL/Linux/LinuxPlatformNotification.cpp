#include "stdafx.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformNotification.h"

#include "ApplicationBase.h"

#include "Container/SparseArray.h"
#include "Container/Stack.h"
#include "Container/StringHashMap.h"
#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/Logger.h"
#include "Meta/AlignedStorage.h"
#include "Meta/Singleton.h"
#include "Misc/Function.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

#include "HAL/Linux/Errno.h"
#include "HAL/PlatformApplication.h"
#include "HAL/PlatformIncludes.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformWindow.h"

namespace PPE {
namespace Application {
LOG_CATEGORY(, Notification)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLinuxPlatformNotification::ShowSystray() {

}
//----------------------------------------------------------------------------
void FLinuxPlatformNotification::HideSystray() {

}
//----------------------------------------------------------------------------
void FLinuxPlatformNotification::NotifySystray(ENotificationIcon icon, const FWStringView& title, const FWStringView& text) {
    UNUSED(icon);
    UNUSED(title);
    UNUSED(text);
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformNotification::AddSystrayCommand(
    const FWStringView& category,
    const FWStringView& label,
    FSystrayDelegate&& cmd ) {
    UNUSED(category);
    UNUSED(label);
    UNUSED(cmd);
    return INDEX_NONE;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformNotification::RemoveSystrayCommand(size_t index) {
    UNUSED(index);
    return false;
}
//----------------------------------------------------------------------------
void FLinuxPlatformNotification::SetTaskbarState(ETaskbarState state) {
    UNUSED(state);
}
//----------------------------------------------------------------------------
void FLinuxPlatformNotification::SetTaskbarProgress(size_t completed, size_t total) {
    UNUSED(completed);
    UNUSED(total);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_LINUX
