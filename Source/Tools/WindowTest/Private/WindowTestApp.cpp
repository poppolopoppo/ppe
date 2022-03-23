#include "stdafx.h"

#include "WindowTestApp.h"
#include "Test_Includes.h"

namespace PPE {
LOG_CATEGORY(, WindowTest)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern bool Test_Draw1_(FWindowTestApp& app);
extern bool Test_Draw2_(FWindowTestApp& app);
extern bool Test_Draw3_(FWindowTestApp& app);
extern bool Test_Draw4_(FWindowTestApp& app);
extern bool Test_Draw5_(FWindowTestApp& app);
//----------------------------------------------------------------------------
FWindowTestApp::FWindowTestApp(const FModularDomain& domain)
:   parent_type(domain, "WindowTest", true) {

#if 0 //%_NOCOMMIT%
    FLogger::SetGlobalVerbosity(ELoggerVerbosity::None);
#endif
}
//----------------------------------------------------------------------------
FWindowTestApp::~FWindowTestApp() = default;
//----------------------------------------------------------------------------
void FWindowTestApp::Start() {
    parent_type::Start();

    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"HelloWorld!",
        L"WindowTest");

    auto launchTest = [this](FWStringView name, auto&& test) {
        UNUSED(name);
        LOG(WindowTest, Emphasis, L"start framegraph test <{0}> ...", name);
        RHI::IFrameGraph& fg = *RHI().FrameGraph();
        fg.PrepareNewFrame();
        if (Likely(test(*this))) {
            ONLY_IF_RHIDEBUG(fg.LogFrame());
            LOG(WindowTest, Info, L"frame graph test <{0}> [PASSED]", name);
            return true;
        }
        LOG(WindowTest, Error, L"frame graph test <{0}> [FAILED]", name);
        return false;
    };

#define LAUNCH_TEST_(_Func) launchTest(WSTRINGIZE(_Func), _Func)

    static volatile bool enabled = true;
    if (enabled) {
        LAUNCH_TEST_(&Test_Draw1_);
        LAUNCH_TEST_(&Test_Draw2_);
        LAUNCH_TEST_(&Test_Draw3_);
        LAUNCH_TEST_(&Test_Draw4_);
        LAUNCH_TEST_(&Test_Draw5_);
    }

#undef LAUNCH_TEST_

    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FWindowTestApp::Shutdown() {
    Application::FPlatformNotification::NotifySystray(
        Application::FPlatformNotification::ENotificationIcon::Warning,
        L"Here I go",
        L"WindowTest");

    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
