#include "stdafx.h"

#include "TestApp.h"

#include "Diagnostic/CurrentProcess.h"
#include "HAL/PlatformCrash.h"
#include "HAL/PlatformDebug.h"
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
extern void Test_Network();
//extern void Test_Pixmap();
extern void Test_Thread();
extern void Test_XML();
extern void Test_Lattice();
extern void Test_VFS();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTestApp::FTestApp()
    : parent_type(L"TestApp") {
}
//----------------------------------------------------------------------------
FTestApp::~FTestApp() {}
//----------------------------------------------------------------------------
void FTestApp::Start() {
    parent_type::Start();

    using FAppNotify = Application::FPlatformNotification;

    FAppNotify::ShowSystray();

    FAppNotify::AddSystrayCommand(
        L"Debug",
        L"Create mini dump",
        []() {
        FPlatformCrash::WriteMiniDump();
    });
    FAppNotify::AddSystrayCommand(
        L"Debug",
        L"Debug break",
        []() {
        PPE_DEBUG_BREAK();
    });

#if USE_PPE_PLATFORM_DEBUG
    FAppNotify::AddSystrayCommand(
        L"Memory",
        L"Check memory",
        []() {
        FPlatformDebug::CheckMemory();
    });
#endif
#if !USE_PPE_FINAL_RELEASE
    FAppNotify::AddSystrayCommand(
        L"Memory",
        L"Dump memory leaks",
        []() {
        DumpMemoryLeaks();
    });
#endif
    FAppNotify::AddSystrayCommand(
        L"Memory",
        L"Report all tracking data",
        []() {
        ReportAllTrackingData();
    });

    FAppNotify::AddSystrayCommand(
        L"Process",
        L"Dump process infos",
        []() {
        FCurrentProcess::Get().DumpProcessInfos();
    });
    FAppNotify::AddSystrayCommand(
        L"Process",
        L"Dump process memory",
        []() {
        FCurrentProcess::Get().DumpMemoryStats();
    });

    FAppNotify::NotifySystray(
        FAppNotify::ENotificationIcon::Info,
        L"Starting TestApp",
        L"Unit testing will start with " WSTRINGIZE(BUILDCONFIG) L" build config" );

    constexpr size_t total = 9;

    FAppNotify::SetTaskbarProgress(0, total);
    Test_Allocators();
    FAppNotify::SetTaskbarProgress(1, total);
    Test_Containers();
    FAppNotify::SetTaskbarProgress(2, total);
    Test_Format();
    FAppNotify::SetTaskbarProgress(3, total);
    Test_Thread();
    FAppNotify::SetTaskbarProgress(4, total);
    Test_VFS();
    FAppNotify::SetTaskbarProgress(5, total);
    Test_RTTI();
    FAppNotify::SetTaskbarProgress(6, total);
    Test_XML();
    FAppNotify::SetTaskbarProgress(7, total);
    Test_Lattice();
    FAppNotify::SetTaskbarProgress(8, total);
    //Test_Pixmap();  #TODO refactoring the asset generation
    Test_Network();
    FAppNotify::SetTaskbarProgress(9, total);

    FAppNotify::SetTaskbarState(FAppNotify::ETaskbarState::Indeterminate);
}
//----------------------------------------------------------------------------
void FTestApp::Shutdown() {
    parent_type::Shutdown();

    using FAppNotify = Application::FPlatformNotification;

    FAppNotify::SetTaskbarState(FAppNotify::ETaskbarState::Normal);

    FAppNotify::NotifySystray(
        FAppNotify::ENotificationIcon::Warning,
        L"Shutting down TestApp",
        L"Oh yeah");

    FAppNotify::HideSystray();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
