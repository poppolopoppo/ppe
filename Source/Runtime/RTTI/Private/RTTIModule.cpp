#include "stdafx.h"

#include "RTTIModule.h"

#include "RTTI/Module.h"
#include "RTTI/Module-impl.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Service.h"

#include "MetaDatabase.h"
#include "MetaObject.h"

#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "Diagnostic/Logger.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace RTTI {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
// This the module used for RTTI backend
RTTI_MODULE_DEF(PPE_RTTI_API, RTTI, MetaObject);
//----------------------------------------------------------------------------
#if USE_PPE_RTTI_CHECKS
extern void RTTI_UnitTests();
#endif
//----------------------------------------------------------------------------
} //!namespace RTTI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FRTTIModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FRTTIModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FRTTIModule::FRTTIModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FRTTIModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace RTTI;

    TypeNamesStart();
    FName::Start();

    FMetaDatabase::Create();

    RTTI_MODULE(RTTI).Start();

    TUniquePtr<IRTTIService> rtti;
    IRTTIService::MakeDefault(&rtti);
    rtti->RegisterModule(this, RTTI_MODULE(RTTI));
    domain.Services().Add<IRTTIService>(std::move(rtti));

#if USE_PPE_RTTI_CHECKS
    RTTI_UnitTests();
#endif
}
//----------------------------------------------------------------------------
void FRTTIModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace RTTI;

    domain.Services().Get<IRTTIService>().UnregisterModule(this, RTTI_MODULE(RTTI));
    domain.Services().Remove<IRTTIService>();

    RTTI_MODULE(RTTI).Shutdown();

    FMetaDatabase::Destroy();

    FName::Shutdown();
    TypeNamesShutdown();
}
//----------------------------------------------------------------------------
void FRTTIModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FRTTIModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
