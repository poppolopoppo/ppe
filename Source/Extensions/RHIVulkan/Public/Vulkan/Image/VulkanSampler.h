#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/SamplerDesc.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanSampler final : Meta::FNonCopyable {
public:
    struct FInternalData {
        hash_t HashValue{ Meta::ForceInit };
        VkSampler Sampler{ VK_NULL_HANDLE };
        VkSamplerCreateInfo CreateInfo{};
    };

    FVulkanSampler() = default;
    FVulkanSampler(const FVulkanDevice& device, const FSamplerDesc& desc);
#if USE_PPE_RHIDEBUG
    ~FVulkanSampler();
#endif

    FVulkanSampler(FVulkanSampler&& rvalue) NOEXCEPT;
    FVulkanSampler& operator =(FVulkanSampler&&) = delete;

    auto Read() const { return _data.LockShared(); }

    VkSampler Handle() const { return Read()->Sampler; }
    hash_t HashValue() const { return Read()->HashValue; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    bool operator ==(const FVulkanSampler& other) const NOEXCEPT;
    bool operator !=(const FVulkanSampler& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanSampler& sampler) NOEXCEPT { return sampler.HashValue(); }

private:
    TRHIThreadSafe<FInternalData> _data;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
