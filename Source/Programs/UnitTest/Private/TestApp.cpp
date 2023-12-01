// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TestApp.h"

#include "HAL/PlatformNotification.h"

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern void Test_Allocators();
extern void Test_Format();
extern void Test_Containers();
extern void Test_RTTI();
extern void Test_Maths();
extern void Test_Memory();
extern void Test_Network();
//extern void Test_Pixmap();
extern void Test_Opaq();
extern void Test_Process();
extern void Test_Thread();
extern void Test_XML();
// extern void Test_Lattice();
extern void Test_VFS();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTestApp::FTestApp(FModularDomain& domain)
:   parent_type(domain, "TestApp") {
}
//----------------------------------------------------------------------------
FTestApp::~FTestApp() = default;
//----------------------------------------------------------------------------
void FTestApp::Start() {
    parent_type::Start();

    using FAppNotify = Application::FPlatformNotification;

    typedef void(*test_t)();
    const test_t tests[] = {
        &Test_Allocators,
        &Test_Containers,
        &Test_Format,
        &Test_Thread,
        &Test_Maths,
        &Test_Memory,
        &Test_Opaq,
        &Test_VFS,
        &Test_Process,
        &Test_RTTI,
        &Test_XML,
        // &Test_Lattice,
        //&Test_Pixmap, // #TODO refactoring the asset generation pipeline
        &Test_Network
    };

    const size_t total = lengthof(tests);
    FAppNotify::SetTaskbarProgress(0, total);
    forrange(i, 0, total) {
        tests[i]();
        FAppNotify::SetTaskbarProgress(i + 1, total);
        ReportAllTrackingData();
    }

    FAppNotify::SetTaskbarState(FAppNotify::ETaskbarState::Indeterminate);
}
//----------------------------------------------------------------------------
void FTestApp::Shutdown() {
    using FAppNotify = Application::FPlatformNotification;

    FAppNotify::NotifySystray(
        FAppNotify::ENotificationIcon::Warning,
        L"Shutting down TestApp",
        L"Oh yeah");

    FAppNotify::SetTaskbarState(FAppNotify::ETaskbarState::Normal);

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
