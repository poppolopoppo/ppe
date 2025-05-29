#pragma once

#include "Application.h"

#include "HAL/PlatformDialog.h"
#include "HAL/PlatformProcess.h"

#include "Diagnostic/CurrentProcess.h"
#include "Diagnostic/FeedbackContext.h"
#include "Modular/ModularDomain.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGenericPlatformLaunch {
public:
    static void OnPlatformLaunch(
        void* appHandle, int nShowCmd,
        const wchar_t* filename, size_t argc, const wchar_t* const* argv ) {
        FPlatformProcess::OnProcessStart(appHandle, nShowCmd, filename, argc, argv);

        FGlobalFeedbackContext::Start();

        ReportAllTrackingData();

#if !USE_PPE_FINAL_RELEASE
        // Detect debugger for QOL
        if (FCurrentProcess::StartedWithDebugger()) {
            // Do not show startup splash screen if a debuger is attached
            FPlatformDialog::ToggleSplashScreenEnabled(false);
        }
#endif
    }

    static void OnPlatformShutdown() {
        ReportAllTrackingData();

        FGlobalFeedbackContext::Shutdown();

        FPlatformProcess::OnProcessShutdown();
    }

    template <typename _ApplicationType>
    static int RunApplication(FModularDomain& domain) {
        int exitCode;
        FModularDomain::Start(domain);
        {
            _ApplicationType app(domain);
            exitCode = LaunchApplication(&app);
        }
        FModularDomain::Shutdown(domain);
        return exitCode;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
