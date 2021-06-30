
#include "stdafx.h"

#include "Vulkan/Image/VulkanSampler.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static hash_t hash_value(const VkSamplerCreateInfo& info) {
    hash_t h = hash_tuple(
        info.flags,
        info.magFilter,
        info.minFilter,
        info.mipmapMode,
        info.addressModeU,
        info.addressModeV,
        info.addressModeW,
        info.mipLodBias,
        info.anisotropyEnable,
        info.compareEnable,
        info.minLod,
        info.maxLod,
        info.borderColor,
        info.unnormalizedCoordinates );

    if (info.anisotropyEnable)
        hash_combine(h, info.maxAnisotropy);

    if (info.compareEnable)
        hash_combine(h, info.compareOp);

    return h;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanSampler::~FVulkanSampler() {
    Assert_NoAssume(VK_NULL_HANDLE == _data.LockExclusive()->Sampler);
}
#endif
//----------------------------------------------------------------------------
FVulkanSampler::FVulkanSampler(const FVulkanDevice& device, const FSamplerDesc& desc) {
    const auto exclusiveData = _data.LockExclusive();

    exclusiveData->CreateInfo = {};
    exclusiveData->CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    exclusiveData->CreateInfo.flags = 0;
    exclusiveData->CreateInfo.magFilter = VkCast( desc.MagFilter );
    exclusiveData->CreateInfo.minFilter = VkCast( desc.MinFilter );
    exclusiveData->CreateInfo.mipmapMode = VkCast( desc.MipmapFilter );
    exclusiveData->CreateInfo.addressModeU = VkCast( desc.AddressModes[0] );
    exclusiveData->CreateInfo.addressModeV = VkCast( desc.AddressModes[1] );
    exclusiveData->CreateInfo.addressModeW = VkCast( desc.AddressModes[2] );
    exclusiveData->CreateInfo.mipLodBias = desc.MipLodBias;
    exclusiveData->CreateInfo.anisotropyEnable = desc.MaxAnisotropy.has_value() ? VK_TRUE : VK_FALSE;
    exclusiveData->CreateInfo.maxAnisotropy = desc.MaxAnisotropy.value_or( 0.0f );
    exclusiveData->CreateInfo.compareEnable = desc.CompareOp.has_value() ? VK_TRUE : VK_FALSE;
    exclusiveData->CreateInfo.compareOp = VkCast( desc.CompareOp.value_or( ECompareOp::Always ));
    exclusiveData->CreateInfo.minLod = desc.MinLod;
    exclusiveData->CreateInfo.maxLod = desc.MaxLod;
    exclusiveData->CreateInfo.borderColor = VkCast( desc.BorderColor );
    exclusiveData->CreateInfo.unnormalizedCoordinates = desc.EnableUnnormalizedCoords ? VK_TRUE : VK_FALSE;

    // validate

    const VkPhysicalDeviceLimits& limits = device.Limits();
    const VkPhysicalDeviceFeatures& features = device.Capabilities().Features;

    if (exclusiveData->CreateInfo.unnormalizedCoordinates) {
        Assert(exclusiveData->CreateInfo.minFilter == exclusiveData->CreateInfo.magFilter);
        Assert(exclusiveData->CreateInfo.mipmapMode == VK_SAMPLER_MIPMAP_MODE_NEAREST);
        Assert(exclusiveData->CreateInfo.minLod == 0.0f and exclusiveData->CreateInfo.maxLod == 0.0f);
        Assert(exclusiveData->CreateInfo.addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or
               exclusiveData->CreateInfo.addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER );
        Assert(exclusiveData->CreateInfo.addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or
               exclusiveData->CreateInfo.addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER );

        exclusiveData->CreateInfo.magFilter = exclusiveData->CreateInfo.minFilter;
        exclusiveData->CreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        exclusiveData->CreateInfo.minLod = exclusiveData->CreateInfo.maxLod = 0.0f;
        exclusiveData->CreateInfo.anisotropyEnable = VK_FALSE;
        exclusiveData->CreateInfo.compareEnable = VK_FALSE;

        if (exclusiveData->CreateInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE and
            exclusiveData->CreateInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ) {
            Assert(false);
            exclusiveData->CreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        if (exclusiveData->CreateInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE and
            exclusiveData->CreateInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ) {
            Assert(false);
            exclusiveData->CreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }

    if (exclusiveData->CreateInfo.mipLodBias > limits.maxSamplerLodBias) {
        Assert(exclusiveData->CreateInfo.mipLodBias <= limits.maxSamplerLodBias);
        exclusiveData->CreateInfo.mipLodBias = limits.maxSamplerLodBias;
    }

    if (exclusiveData->CreateInfo.maxLod < exclusiveData->CreateInfo.minLod) {
        Assert(exclusiveData->CreateInfo.maxLod >= exclusiveData->CreateInfo.minLod);
        exclusiveData->CreateInfo.maxLod = exclusiveData->CreateInfo.minLod;
    }

    if (not features.samplerAnisotropy) {
        Assert(not exclusiveData->CreateInfo.anisotropyEnable);
        exclusiveData->CreateInfo.anisotropyEnable = VK_FALSE;
    }

    if (exclusiveData->CreateInfo.minFilter == VK_FILTER_CUBIC_IMG or
        exclusiveData->CreateInfo.magFilter == VK_FILTER_CUBIC_IMG ) {
        Assert(not exclusiveData->CreateInfo.anisotropyEnable);
        exclusiveData->CreateInfo.anisotropyEnable = VK_FALSE;
    }

    if (not exclusiveData->CreateInfo.anisotropyEnable)
        exclusiveData->CreateInfo.maxAnisotropy = 0.0f;
    else
        exclusiveData->CreateInfo.maxAnisotropy = Clamp(exclusiveData->CreateInfo.maxAnisotropy, 1.0f, limits.maxSamplerAnisotropy);

    if (not exclusiveData->CreateInfo.compareEnable) {
        exclusiveData->CreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    }

    if (exclusiveData->CreateInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER and
        exclusiveData->CreateInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER and
        exclusiveData->CreateInfo.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ) {
        // reset border color, because it is unused
        exclusiveData->CreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    }

    if (not device.Enabled().SamplerMirrorClamp) {
        if (exclusiveData->CreateInfo.addressModeU == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            Assert(false);
            exclusiveData->CreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
        if (exclusiveData->CreateInfo.addressModeV == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            Assert(false);
            exclusiveData->CreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
        if (exclusiveData->CreateInfo.addressModeW == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            Assert(false);
            exclusiveData->CreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }

    // calculate hash

    exclusiveData->HashValue = hash_value(exclusiveData->CreateInfo);
}
//----------------------------------------------------------------------------
FVulkanSampler::FVulkanSampler(FVulkanSampler&& rvalue) NOEXCEPT
:   _data(std::move(*rvalue._data.LockExclusive()))
ARGS_IF_RHIDEBUG(_debugName(rvalue.DebugName()))
{

}
//----------------------------------------------------------------------------
bool FVulkanSampler::Construct(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(VK_NULL_HANDLE == exclusiveData->Sampler);

    VK_CHECK(device.vkCreateSampler(
        device.vkDevice(),
        &exclusiveData->CreateInfo,
        device.vkAllocator(),
        &exclusiveData->Sampler ));

    ONLY_IF_RHIDEBUG(_debugName = debugName);

    return true;
}
//----------------------------------------------------------------------------
void FVulkanSampler::TearDown(FVulkanResourceManager& resources) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(VK_NULL_HANDLE != exclusiveData->Sampler);

    if (exclusiveData->Sampler) {
        const FVulkanDevice& device = resources.Device();
        device.vkDestroySampler(
            device.vkDevice(),
            exclusiveData->Sampler,
            device.vkAllocator() );
    }

    exclusiveData->Sampler = VK_NULL_HANDLE;
    exclusiveData->HashValue = Default;
    exclusiveData->CreateInfo = Default;

    ONLY_IF_RHIDEBUG(_debugName.Clear());
}
//----------------------------------------------------------------------------
bool FVulkanSampler::operator==(const FVulkanSampler& other) const NOEXCEPT {
    const auto lhs = _data.LockShared();
    const auto rhs = other._data.LockShared();

    if (lhs->HashValue != rhs->HashValue)
        return false;

    const VkSamplerCreateInfo& lci = lhs->CreateInfo;
    const VkSamplerCreateInfo& rci = rhs->CreateInfo;

    return (
        lci.flags == rci.flags and
        lci.magFilter == rci.magFilter and
        lci.minFilter == rci.minFilter and
        lci.mipmapMode == rci.mipmapMode and
        lci.addressModeU == rci.addressModeU and
        lci.addressModeV == rci.addressModeV and
        lci.addressModeW == rci.addressModeW and
        lci.mipLodBias == rci.mipLodBias and

        lci.anisotropyEnable == rci.anisotropyEnable and
        (not lci.anisotropyEnable or
        lci.maxAnisotropy == rci.maxAnisotropy) and

        lci.compareEnable == rci.compareEnable and
        (not lci.compareEnable or
        lci.compareOp == rci.compareOp) and

        lci.minLod == rci.minLod and
        lci.maxLod == rci.maxLod and
        lci.borderColor == rci.borderColor and
        lci.unnormalizedCoordinates	== rci.unnormalizedCoordinates );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
