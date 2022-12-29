#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/PipelineDesc.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanShaderModule : public IShaderData<FShaderModule> {
public:
    using IShaderData<FShaderModule>::FDataRef;
    using IShaderData<FShaderModule>::FFingerprint;

    FVulkanShaderModule(
        VkShaderModule vkShaderModule,
        FFingerprint sourceFingerprint,
        FStringView entryPoint ) NOEXCEPT;

    virtual ~FVulkanShaderModule() NOEXCEPT override;

    VkShaderModule vkShaderModule() const { return _vkShaderModule; }

    virtual FDataRef Data() const NOEXCEPT override final { return reinterpret_cast<FDataRef>(&_vkShaderModule); }
    virtual FConstChar EntryPoint() const NOEXCEPT override final { return _entryPoint.c_str(); }
    virtual FFingerprint Fingerprint() const NOEXCEPT override final { return _fingerprint; }

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT override final { return _debugName; }

    virtual bool ParseDebugOutput(TAppendable<FString> outp, EShaderDebugMode mode, FRawMemoryConst trace) override;
#endif

    NODISCARD bool Construct(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FStringView debugName));
    void TearDown(const FVulkanDevice& device);

private:
    VkShaderModule _vkShaderModule;
    FFingerprint _fingerprint;
    TStaticString<64> _entryPoint;
#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
