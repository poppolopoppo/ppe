#include "stdafx.h"

#include "BuildRobotApp.h"

#include "HAL/PlatformNotification.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildRobotApp::FBuildRobotApp()
    : parent_type(L"BuildRobot") {
}
//----------------------------------------------------------------------------
FBuildRobotApp::~FBuildRobotApp() = default;
//----------------------------------------------------------------------------
void FBuildRobotApp::Start() {
    parent_type::Start();

    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"Worker listening for new tasks",
        L"BuildRobot");
}
//----------------------------------------------------------------------------
void FBuildRobotApp::Shutdown() {
    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"Shutting down worker",
        L"BuildRobot");

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
