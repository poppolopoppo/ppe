#pragma once

#include "PipelineCompiler_fwd.h"

#include "Vulkan/Pipeline/VulkanShaderCompilationFlags.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

#include "RHI/PipelineCompiler.h"
#include "RHI/PipelineDesc.h"

#include "Container/HashMap.h"
#include "Container/HashHelpers.h"
#include "Container/Vector.h"
#include "IO/Dirpath.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVulkanSpirvCompiler;
//----------------------------------------------------------------------------
class PPE_PIPELINECOMPILER_API FVulkanPipelineCompiler final : public IPipelineCompiler {
public:
    using FShaderDataMap = FPipelineDesc::FShaderDataMap;

    explicit FVulkanPipelineCompiler(Meta::FForceInit/* non-device specific */);
    explicit FVulkanPipelineCompiler(const FVulkanDeviceInfo& deviceInfo);
    ~FVulkanPipelineCompiler() override;

    void AddDirectory(const FDirpath& path);

    EVulkanShaderCompilationFlags CompilationFlags() const { return _data.LockShared()->CompilationFlags; }
    void SetCompilationFlags(EVulkanShaderCompilationFlags flags);

#if USE_PPE_RHIDEBUG
    void SetDebugFlags(EShaderLangFormat flags);
#endif

    void ReleaseShaderCache();
    void ReleaseUnusedShaders();

public: // IPipelineCompiler
    FString DisplayName() const NOEXCEPT override { return "VulkanPipelineCompiler"; }
    ETargetRHI TargetRHI() const NOEXCEPT override { return ETargetRHI::Vulkan; }

    bool IsSupported(const FMeshPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT override;
    bool IsSupported(const FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT override;
    bool IsSupported(const FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT override;
    bool IsSupported(const FComputePipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT override;

    bool Compile(FMeshPipelineDesc& desc, EShaderLangFormat fmt) override;
    bool Compile(FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) override;
    bool Compile(FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) override;
    bool Compile(FComputePipelineDesc& desc, EShaderLangFormat fmt) override;

    void ReleaseUnusedMemory() override { ReleaseUnusedShaders(); }

private:
    struct FShaderBinaryDataTraits {
        // EqualTo:
        bool operator ()(const PShaderBinaryData& lhs, const PShaderBinaryData& rhs) const NOEXCEPT;
        // Hash:
        hash_t operator ()(const PShaderBinaryData& shaderData) const NOEXCEPT;
    };
    using FShaderBinaryMemoizedData = THashMemoizer<
        PShaderBinaryData,
        FShaderBinaryDataTraits,
        FShaderBinaryDataTraits >;
    using FShaderCompiledModuleCache = HASHMAP(
        PipelineCompiler,
        FShaderBinaryMemoizedData,
        PVulkanDebuggableShaderModule);

    struct FInternalData {
        VECTORINSITU(PipelineCompiler, FDirpath, 3) Directories;
        FShaderCompiledModuleCache ShaderCache;
        TUniquePtr<FVulkanSpirvCompiler> SpirvCompiler;
        EVulkanShaderCompilationFlags CompilationFlags{ Default };
    };

    NODISCARD bool CreateVulkanShader_(
        FPipelineDesc::FShader* shader,
        FShaderCompiledModuleCache& shaderCache ) const;

    const TPtrRef<const FVulkanDeviceInfo> _deviceInfo;

    TThreadSafe<FInternalData, EThreadBarrier::CriticalSection> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
