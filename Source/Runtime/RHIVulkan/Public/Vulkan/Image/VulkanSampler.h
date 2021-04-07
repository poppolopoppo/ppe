#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/SamplerDesc.h"

#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanSampler final : Meta::FNonCopyable {
public:
    struct FInternalData {
        hash_t HashValue;
        VkSampler Sampler{ VK_NULL_HANDLE };
        VkSamplerCreateInfo CreateInfo{};
    };

    FVulkanSampler() = default;
    FVulkanSampler(const FVulkanDevice& device, const FSamplerDesc& desc);
    ~FVulkanSampler();

    FVulkanSampler(FVulkanSampler&&) = default;
    FVulkanSampler& operator =(FVulkanSampler&&) = delete;

    auto Read() const { return _data.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool Create(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FStringView));
    void TearDown(FVulkanResourceManager& resources);

    bool operator ==(const FVulkanSampler& other) const;
    bool operator !=(const FVulkanSampler& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanSampler& sampler) { return sampler.HashValue(); }

private:
    TRHIThreadSafe<FInternalData> _data;

#ifdef USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
