#include "stdafx.h"

#include "PipelineReflectionModule.h"

#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace RHI {
LOG_CATEGORY(PPE_PIPELINEREFLECTION_API, PipelineReflection)
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FPipelineReflectionModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FPipelineReflectionModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Framework,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList )
};
//----------------------------------------------------------------------------
FPipelineReflectionModule::FPipelineReflectionModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FPipelineReflectionModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

}
//----------------------------------------------------------------------------
void FPipelineReflectionModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

}
//----------------------------------------------------------------------------
void FPipelineReflectionModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FPipelineReflectionModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE