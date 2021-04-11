
#include "HAL/Linux/LinuxPlatformLaunch.h"
#include "HAL/Generic/GenericPlatformLaunch-impl.h"

int main(int argc, const wchar_t** argv) {
    const wchar_t* filename = argv[0];
    argv = &argv[1];
    argc--;

    using namespace PPE;
    using namespace PPE::Application;

    int exitCode = 0;

    FLinuxPlatformLaunch::OnPlatformLaunch(filename, argc, argv);
    {
        FApplicationDomain appDomain;
        appDomain.LoadDependencies();

        exitCode  = FLinuxPlatformLaunch::RunApplication<PPE_APPLICATIONLAUNCH_TYPE>(appDomain);

        appDomain.UnloadDependencies();
    }

    FLinuxPlatformLaunch::OnPlatformShutdown();

    return exitCode;
}
