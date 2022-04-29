#include "stdafx.h"

#include "BuildGraphModule.h"

#include "BuildGraph.h"
#include "Diagnostic/Logger.h"
#include "Memory/MemoryDomain.h"
#include "RTTI/Module-impl.h"
#include "RTTI/Service.h"

#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace ContentPipeline {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_BUILDGRAPH_API, BuildGraph)
//----------------------------------------------------------------------------
RTTI_MODULE_DEF(PPE_BUILDGRAPH_API, BuildGraph, BuildGraph);
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FBuildGraphModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FBuildGraphModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FBuildGraphModule::FBuildGraphModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FBuildGraphModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace ContentPipeline;

    RTTI_MODULE(BuildGraph).Start();

    domain.Services().Get<IRTTIService>().RegisterModule(this, RTTI_MODULE(BuildGraph));
}
//----------------------------------------------------------------------------
void FBuildGraphModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace ContentPipeline;

    domain.Services().Get<IRTTIService>().UnregisterModule(this, RTTI_MODULE(BuildGraph));

    RTTI_MODULE(BuildGraph).Shutdown();
}
//----------------------------------------------------------------------------
void FBuildGraphModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FBuildGraphModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
