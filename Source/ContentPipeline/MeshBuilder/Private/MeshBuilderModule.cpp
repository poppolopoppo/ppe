// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MeshBuilderModule.h"

#include "Diagnostic/Logger.h"
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
LOG_CATEGORY(PPE_MESHBUILDER_API, MeshBuilder)
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FMeshBuilderModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FMeshBuilderModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Framework,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FMeshBuilderModule::FMeshBuilderModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FMeshBuilderModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

}
//----------------------------------------------------------------------------
void FMeshBuilderModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

}
//----------------------------------------------------------------------------
void FMeshBuilderModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FMeshBuilderModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
