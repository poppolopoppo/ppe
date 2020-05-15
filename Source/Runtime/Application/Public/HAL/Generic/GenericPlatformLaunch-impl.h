
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

CONSTEXPR const PPE::FStringView GApplicationDependencies[] = {
#define DUMMY_EXPAND(...) PP_FOREACH_ARGS(STRINGIZE, __VA_ARGS__)
    DUMMY_EXPAND(BUILD_TARGET_DEPS)
#undef DUMMY_EXPAND
};

class FApplicationDomain : public PPE::FModularDomain {
public:
    FApplicationDomain() NOEXCEPT
    :   PPE::FModularDomain(STRINGIZE(BUILD_TARGET_NAME), PPE::EModuleUsage::PPE_TARGET_USAGE) {
        Generated::Build_RegisterStaticModules();
    }

    ~FApplicationDomain() {
        Generated::Build_UnregisterStaticModules();
    }

    void LoadDependencies() {
        foreachitem(name, GApplicationDependencies)
            FModularDomain::LoadModule(*name);
    }

    void UnloadDependencies() {
        reverseforeachitem(name, GApplicationDependencies)
            FModularDomain::UnloadModule(*name);
    }
};
