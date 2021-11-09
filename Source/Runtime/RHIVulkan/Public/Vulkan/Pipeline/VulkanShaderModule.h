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
        FStringView entryPoint
        ARGS_IF_RHIDEBUG(FStringView debugName) ) NOEXCEPT;

    virtual ~FVulkanShaderModule() NOEXCEPT override;

    VkShaderModule vkShaderModule() const { return _vkShaderModule; }

    virtual FDataRef Data() const NOEXCEPT override final { return reinterpret_cast<FDataRef>(&_vkShaderModule); }
    virtual FConstChar EntryPoint() const NOEXCEPT override final { return _entryPoint.c_str(); }
    virtual FFingerprint Fingerprint() const NOEXCEPT override final { return _fingerprint; }

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT override final { return _debugName; }

    virtual bool ParseDebugOutput(TAppendable<FString> outp, EShaderDebugMode mode, FRawMemoryConst trace) override;
#endif

    void TearDown(
        VkDevice vkDevice,
        PFN_vkDestroyShaderModule vkDestroyShaderModule,
        const VkAllocationCallbacks* pAllocator );

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
