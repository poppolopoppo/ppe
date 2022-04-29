#include "stdafx.h"

#include "PipelineCompilerModule.h"

#include "RHIModule.h"
#include "RHIVulkanModule.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Pipeline/VulkanPipelineCompiler.h"

#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace RHI {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_PIPELINECOMPILER_API, PipelineCompiler)
//----------------------------------------------------------------------------
CONSTEXPR const EShaderLangFormat GVulkanPipelineFormat_ =
    EShaderLangFormat::SPIRV |
    EShaderLangFormat::ShaderModule;
//----------------------------------------------------------------------------
static void CreateVulkanDeviceCompiler_(const FVulkanDeviceInfo& deviceInfo) {
    EVulkanShaderCompilationFlags compilationFlags = (
        EVulkanShaderCompilationFlags::Quiet |
        EVulkanShaderCompilationFlags::Optimize |
        EVulkanShaderCompilationFlags::ParseAnnotations |
        EVulkanShaderCompilationFlags::UseCurrentDeviceLimits );

#if !USE_PPE_FINAL_RELEASE
    if (deviceInfo.Features & ERHIFeature::Debugging)
        compilationFlags += EVulkanShaderCompilationFlags::Validate | EVulkanShaderCompilationFlags::GenerateDebug;
#endif

    TRefPtr<FVulkanPipelineCompiler> compiler{ NEW_REF(PipelineCompiler, FVulkanPipelineCompiler, deviceInfo) };
    compiler->SetCompilationFlags(compilationFlags);

    auto& rhiModule = FRHIModule::Get(FModularDomain::Get());
    rhiModule.RegisterCompiler(GVulkanPipelineFormat_, PPipelineCompiler{ std::move(compiler) });
}
//----------------------------------------------------------------------------
static void TearDownVulkanDeviceCompiler_(const FVulkanDeviceInfo& ) {

    auto& rhiModule = FRHIModule::Get(FModularDomain::Get());
    rhiModule.UnregisterCompiler(GVulkanPipelineFormat_);
}
//----------------------------------------------------------------------------
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FPipelineCompilerModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FPipelineCompilerModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Framework,
        EModuleUsage::Developer,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FPipelineCompilerModule::FPipelineCompilerModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FPipelineCompilerModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    Assert(not _onDeviceCreated);
    Assert(not _onDeviceTeardown);

    using namespace RHI;
    FRHIVulkanModule& vulkanRhi = FRHIVulkanModule::Get(domain);

    _onDeviceCreated = vulkanRhi.OnDeviceCreated().Add(&CreateVulkanDeviceCompiler_);
    _onDeviceTeardown = vulkanRhi.OnDeviceTearDown().Add(&TearDownVulkanDeviceCompiler_);
}
//----------------------------------------------------------------------------
void FPipelineCompilerModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    Assert(_onDeviceCreated);
    Assert(_onDeviceTeardown);

    using namespace RHI;
    FRHIVulkanModule& vulkanRhi = FRHIVulkanModule::Get(domain);

    vulkanRhi.OnDeviceCreated().Remove(_onDeviceCreated);
    vulkanRhi.OnDeviceTearDown().Remove(_onDeviceTeardown);
}
//----------------------------------------------------------------------------
void FPipelineCompilerModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FPipelineCompilerModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
