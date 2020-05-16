#include "stdafx.h"

#include "BuildGraphModule.h"

#include "BuildGraph.h"
#include "Memory/MemoryDomain.h"
#include "RTTI/Module-impl.h"

#include "Modular/ModuleRegistration.h"

#include "Diagnostic/Logger.h"

#include "BuildModules.generated.h"

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
        STRINGIZE(BUILD_TARGET_DEPS) )
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
}
//----------------------------------------------------------------------------
void FBuildGraphModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace ContentPipeline;

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
