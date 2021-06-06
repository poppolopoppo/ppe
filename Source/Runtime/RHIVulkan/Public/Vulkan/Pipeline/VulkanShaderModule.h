#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/PipelineDesc.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanShaderModule final : public IShaderData<FShaderModule> {
public:
    using IShaderData<FShaderModule>::FDataRef;

    FVulkanShaderModule(
        VkShaderModule vkShaderModule,
        hash_t sourceFingerprint,
        FStringView entryPoint
        ARGS_IF_RHIDEBUG(FConstChar debugName)) NOEXCEPT;

    virtual ~FVulkanShaderModule() NOEXCEPT override;

    VkShaderModule vkShaderModule() const { return _vkShaderModule; }

    virtual FDataRef Data() const NOEXCEPT override { return reinterpret_cast<FDataRef>(&_vkShaderModule); }
    virtual FStringView EntryPoint() const NOEXCEPT override { return _entryPoint.Str(); }
    virtual hash_t HashValue() const NOEXCEPT override { return _hashValue; }

#if USE_PPE_RHIDEBUG
    virtual FConstChar DebugName() const NOEXCEPT override { return _debugName; }

    virtual bool ParseDebugOutput(TAppendable<FString> outp, EShaderDebugMode mode, FRawMemoryConst trace) override;
#endif

    void TearDown(const FVulkanDevice& device);

private:
    VkShaderModule _vkShaderModule;
    hash_t _hashValue;
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
