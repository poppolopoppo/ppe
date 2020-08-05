
#include "HAL/Generic/GenericPlatformLaunch.h"

#include "HAL/PlatformIncludes.h"
#include "Modular/ModularDomain.h"

#ifndef PPE_APPLICATIONLAUNCH_TYPE
#   error "must define PPE_APPLICATIONLAUNCH_TYPE to implement main function"
#endif

#ifdef PPE_APPLICATIONLAUNCH_INCLUDED
#   error "should be included only once inside your main.cpp"
#endif
#define PPE_APPLICATIONLAUNCH_INCLUDED

#include "BuildModules.generated.h"

class FApplicationDomain : public PPE::FModularDomain {
public:
    FApplicationDomain() NOEXCEPT
    :   FModularDomain(STRINGIZE(BUILD_TARGET_NAME), PPE::EModuleUsage::PPE_TARGET_USAGE) {
        using namespace PPE;
        Generated::RegisterStaticModules();
        Generated::RegisterDynamicModules();
    }

    ~FApplicationDomain() {
        using namespace PPE;
        Generated::UnregisterDynamicModules();
        Generated::UnregisterStaticModules();
    }

    void LoadDependencies() {
        using namespace PPE;
        foreachitem(name, Generated::DependencyList)
            FModularDomain::LoadModule(*name);
    }

    void UnloadDependencies() {
        using namespace PPE;
        reverseforeachitem(name, Generated::DependencyList)
            FModularDomain::UnloadModule(*name);
    }
};
