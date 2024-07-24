#pragma once

#include "Application.h"

#include "HAL/PlatformApplication.h"
#include "HAL/PlatformProcess.h"

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
