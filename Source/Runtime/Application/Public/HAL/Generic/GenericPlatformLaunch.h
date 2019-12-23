#pragma once

#ifndef PPE_APPLICATIONLAUNCH_TYPE
#   error "must define PPE_APPLICATIONLAUNCH_TYPE to implement main function"
#endif

#ifdef PPE_APPLICATIONLAUNCH_INCLUDED
#   error "should be included only once inside your main.cpp"
#endif
#define PPE_APPLICATIONLAUNCH_INCLUDED

#include "Application.h"

#include "ModuleManager.h"

#include "HAL/PlatformApplication.h"
#include "HAL/PlatformProcess.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGenericPlatformLaunch {
public:
    using application_type = PPE_APPLICATIONLAUNCH_TYPE;

    static auto& AppDomain() {
        static PPE_STATICMODULES_STARTUP GModules;
        return GModules;
    }

    static void OnPlatformLaunch(
        void* appHandle, int nShowCmd,
        const wchar_t* filename, size_t argc, const wchar_t* const* argv ) {
        FPlatformProcess::OnProcessStart(appHandle, nShowCmd, filename, argc, argv);
    }

    static void OnPlatformShutdown() {
        FPlatformProcess::OnProcessShutdown();
    }

    template <typename _ApplicationType>
    static int RunApplication() {
        int exitCode;
        AppDomain().Start(FModuleManager::Get());
        {
            _ApplicationType app;
            exitCode = LaunchApplication(&app);
        }
        AppDomain().Shutdown(FModuleManager::Get());
        return exitCode;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
