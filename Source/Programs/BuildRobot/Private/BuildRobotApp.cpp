// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "BuildRobotApp.h"

#include "ApplicationModule.h"

#include "MetaObject.h"
#include "MetaTransaction.h"
#include "RTTI/Macros.h"
#include "RTTI/Macros-impl.h"
#include "RTTI/Module.h"
#include "RTTI/Module-impl.h"

#include "HAL/PlatformNotification.h"
#include "HAL/PlatformProcess.h"
#include "Time/Timestamp.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_MODULE_DECL(, BuildRobot);
RTTI_MODULE_DEF(, BuildRobot, MetaObject);
//----------------------------------------------------------------------------
using ENotificationIcon = Application::FPlatformNotification::ENotificationIcon;
namespace Application {
RTTI_ENUM_HEADER(, ENotificationIcon);
RTTI_ENUM_BEGIN(BuildRobot, ENotificationIcon)
RTTI_ENUM_VALUE(None)
RTTI_ENUM_VALUE(Info)
RTTI_ENUM_VALUE(Warning)
RTTI_ENUM_VALUE(Error)
RTTI_ENUM_END()
} //!namespace Application
//----------------------------------------------------------------------------
class FBuildRobot : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(, FBuildRobot, RTTI::FMetaObject);

    FString AppName;

    FTimestamp Now() const {
        return FTimestamp::Now();
    }

    void Notify(ENotificationIcon icon, const FWString& title, const FWString& text) const {
        Application::FPlatformNotification::NotifySystray(icon, title, text);
    }

};
RTTI_CLASS_BEGIN(BuildRobot, FBuildRobot, Concrete)
RTTI_FUNCTION(Notify, (icon, title, text))
RTTI_FUNCTION(Now, ())
RTTI_PROPERTY_PUBLIC_FIELD(AppName)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildRobotApp::FBuildRobotApp(FModularDomain& domain)
:   parent_type(domain, "BuildRobot") {
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

    RTTI_MODULE(BuildRobot).Start();

    RTTI::FMetaTransaction transaction{ "BuildRobot"_rtti };
    auto buildRobot = NEW_REF(UserDomain, FBuildRobot);
    buildRobot->RTTI_Export("BuildRobot"_rtti);
    buildRobot->AppName = Name();
    transaction.Add(std::move(buildRobot));
    transaction.LoadAndMount();

    ApplicationLoop(); // wait for Remoting events

    transaction.UnmountAndUnload();
    RTTI_MODULE(BuildRobot).Shutdown();
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
