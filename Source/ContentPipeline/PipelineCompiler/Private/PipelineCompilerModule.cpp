// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
#include "Diagnostic/CurrentProcess.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace RHI {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_PIPELINECOMPILER_API, PipelineCompiler)
//----------------------------------------------------------------------------
inline CONSTEXPR const EShaderLangFormat GVulkanPipelineFormat_ =
    EShaderLangFormat::SPIRV |
    EShaderLangFormat::ShaderModule;
//----------------------------------------------------------------------------
static void CreateVulkanDeviceCompiler_(const FVulkanDevice& device, ERHIFeature features) {
    EShaderCompilationFlags compilationFlags = (
        EShaderCompilationFlags::Optimize |
        EShaderCompilationFlags::ParseAnnotations |
        EShaderCompilationFlags::UseCurrentDeviceLimits );

#if USE_PPE_FINAL_RELEASE
    Unused(features);
    compilationFlags += EShaderCompilationFlags::Quiet;

#else
    if (features & ERHIFeature::Debugging)
        compilationFlags += EShaderCompilationFlags::GenerateDebug | EShaderCompilationFlags::Validate;

    const FCurrentProcess& proc = FCurrentProcess::Get();
    if (proc.HasArgument(L"-SPIRVDebug"))
        compilationFlags += EShaderCompilationFlags::GenerateDebug;
    if (proc.HasArgument(L"-SPIRVNoDebug"))
        compilationFlags -= EShaderCompilationFlags::GenerateDebug;
    if (proc.HasArgument(L"-SPIRVOptimize"))
        compilationFlags += EShaderCompilationFlags::Optimize;
    if (proc.HasArgument(L"-SPIRVNoOptimize"))
        compilationFlags -= EShaderCompilationFlags::Optimize;
    if (proc.HasArgument(L"-SPIRVQuiet"))
        compilationFlags += EShaderCompilationFlags::Quiet;
    if (proc.HasArgument(L"-SPIRVNoQuiet"))
        compilationFlags -= EShaderCompilationFlags::Quiet;
    if (proc.HasArgument(L"-SPIRVValidate"))
        compilationFlags += EShaderCompilationFlags::Validate;
    if (proc.HasArgument(L"-SPIRVNoValidate"))
        compilationFlags -= EShaderCompilationFlags::Validate;

#endif

    TRefPtr compiler{ NEW_REF(PipelineCompiler, FVulkanPipelineCompiler, device) };
    compiler->SetCompilationFlags(compilationFlags);

    auto& rhiModule = FRHIModule::Get(FModularDomain::Get());
    rhiModule.RegisterCompiler(GVulkanPipelineFormat_, PPipelineCompiler{ std::move(compiler) });
}
//----------------------------------------------------------------------------
static void TearDownVulkanDeviceCompiler_(const FVulkanDevice& ) {

    auto& rhiModule = FRHIModule::Get(FModularDomain::Get());
    rhiModule.UnregisterCompiler(GVulkanPipelineFormat_);
}
//------------------------------------------------------------;/----------------
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
