#include "stdafx.h"

#include "RHIVulkanModule.h"

#include "HAL/VulkanTargetRHI.h"

#include "Vulkan/Vulkan_fwd.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanInstance.h"
#include "Vulkan/VulkanFrameGraph.h"

#include "RHI/FrameGraph.h"
#include "RHI/PipelineCompiler.h"

#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FRHIVulkanModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FRHIVulkanModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::System,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList )
};
//----------------------------------------------------------------------------
FRHIVulkanModule& FRHIVulkanModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FRHIVulkanModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FRHIVulkanModule::FRHIVulkanModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FRHIVulkanModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace RHI;
    FRHIModule& rhi = FRHIModule::Get(domain);

    AssertRelease(not _instance);
    _instance = MakeUnique<FVulkanInstance>();

    if (FVulkanInstance::Create(_instance.get(), rhi.SystemFlags())) {
        using FPipelineCompilerFactory = FRHIModule::FPipelineCompilerFactory;
        using FFrameGraphFactory = FRHIModule::FFrameGraphFactory;

        rhi.RegisterTarget(ETargetRHI::Vulkan, &FVulkanTargetRHI::Get());
        rhi.RegisterCompiler(ETargetRHI::Vulkan, FPipelineCompilerFactory::Bind<&FRHIVulkanModule::CreatePipelineCompiler>(this));
        rhi.RegisterFrameGraph(ETargetRHI::Vulkan, FFrameGraphFactory::Bind<&FRHIVulkanModule::CreateFrameGraph>(this));

        LOG(RHI, Info, L"successfuly created Vulkan instance v{0}.{1}.{2}",
            VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
            VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
            VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE) );
    }
    else {
        LOG_DIRECT(RHI, Error, L"failed to initialize Vulkan instance");
    }
}
//----------------------------------------------------------------------------
void FRHIVulkanModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    if (_instance) {
        FRHIModule& rhi = FRHIModule::Get(domain);

        rhi.UnregisterFrameGraph(ETargetRHI::Vulkan);
        rhi.UnregisterCompiler(ETargetRHI::Vulkan);
        rhi.UnregisterTarget(ETargetRHI::Vulkan);

        AssertRelease(_instance);
        _instance.reset();
    }
}
//----------------------------------------------------------------------------
void FRHIVulkanModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FRHIVulkanModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
bool FRHIVulkanModule::VulkanAvailable() const {
    return (!!_instance);
}
//----------------------------------------------------------------------------
RHI::FVulkanInstance& FRHIVulkanModule::VulkanInstance() const {
    return (*_instance);
}
//----------------------------------------------------------------------------
bool FRHIVulkanModule::CreatePipelineCompiler(RHI::PPipelineCompiler* pcompiler) const {
    Assert(pcompiler);
    pcompiler->reset();
    AssertNotImplemented(); // #TODO
}
//----------------------------------------------------------------------------
bool FRHIVulkanModule::CreateFrameGraph(RHI::PFrameGraph* pfg, ERHIFeature features, RHI::FWindowHandle mainWindow) const {
    Assert(pfg);
    UNUSED(features);

    RHI::PVulkanFrameGraph pVulkanFrameGraph;
    if (_instance && RHI::FVulkanFrameGraph::Create(&pVulkanFrameGraph, *_instance, features, mainWindow)) {
        Assert(pVulkanFrameGraph);
        *pfg = pVulkanFrameGraph;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
