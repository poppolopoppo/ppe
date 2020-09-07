#include "stdafx.h"

#include "BuildRobotApp.h"

#include "HAL/PlatformNotification.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformThread.h"

#include "ApplicationModule.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildRobotApp::FBuildRobotApp(const FModularDomain& domain)
:   parent_type(domain, L"BuildRobot") {
}
//----------------------------------------------------------------------------
FBuildRobotApp::~FBuildRobotApp() = default;
//----------------------------------------------------------------------------
void FBuildRobotApp::Start() {
    parent_type::Start();

    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"Worker listening for new tasks",
        L"BuildRobot" );

    ApplicationLoop(); // wait for Remoting events
}
//----------------------------------------------------------------------------
bool FBuildRobotApp::PumpMessages() NOEXCEPT {
    FPlatformProcess::Sleep(0.1f); // cool-down

    return FApplicationConsole::PumpMessages();
}
//----------------------------------------------------------------------------
void FBuildRobotApp::Shutdown() {
    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"Shutting down worker",
        L"BuildRobot" );

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
