#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Descriptors/VulkanDescriptorSetLayout.h"

#include "RHI/PipelineResources.h"

#include <variant>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanPipelineResources final : Meta::FNonCopyable {
public:
    struct FUpdateDescriptors;

    using FInfoVariant = std::variant<
        VkDescriptorBufferInfo,
        VkDescriptorImageInfo,
        VkAccelerationStructureKHR >;

    using FDynamicData = FPipelineResources::FDynamicData;

    struct FInternalResources {
        hash_t HashValue;
        FRawDescriptorSetLayoutID LayoutId;
        FVulkanDescriptorSet DescriptorSet;
        FDynamicData SignatureData;
        FDynamicData DynamicData;
        bool AllowEmptyResources;

        explicit FInternalResources(FDynamicData&& staticData, FRawDescriptorSetLayoutID layoutId, bool allowEmptyResources) NOEXCEPT;
    };

    explicit FVulkanPipelineResources(FPipelineResources&& rdesc) NOEXCEPT;
    explicit FVulkanPipelineResources(const FPipelineResources& desc);
    ~FVulkanPipelineResources();

    FVulkanPipelineResources(FVulkanPipelineResources&& rvalue) NOEXCEPT;
    FVulkanPipelineResources& operator =(FVulkanPipelineResources&&) = delete;

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    auto Read() const { return _resources.LockShared(); }
    auto Write() { return _resources.LockExclusive(); }

    VkDescriptorSet Handle() const { return Read()->DescriptorSet.First; }

    hash_t HashValue() const { return Read()->HashValue; }

    bool AllResourcesAlive(const FVulkanResourceManager& manager) const;

    NODISCARD bool Construct(FVulkanResourceManager& manager);
    void TearDown(FVulkanResourceManager& manager);

    template <typename _Each>
    void EachUniform(_Each&& each) const {
        Read()->DynamicData.EachUniform(std::forward<_Each>(each));
    }

    bool operator ==(const FVulkanPipelineResources& other) const NOEXCEPT;
    bool operator !=(const FVulkanPipelineResources& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanPipelineResources& resources) NOEXCEPT { return resources.HashValue(); }

    static void CheckBufferUsage(const class FVulkanBuffer&, EResourceState state);
    static void CheckTexelBufferUsage(const class FVulkanBuffer&, EResourceState state);
    static void CheckImageUsage(const class FVulkanImage&, EResourceState state);

    static void CheckImageType(const FUniformID& id, u32 index, const FVulkanImage& img, const FImageViewDesc& desc, EImageSampler shaderType);
    static void CheckTextureType(const FUniformID& id, u32 index, const FVulkanImage& img, const FImageViewDesc& desc, EImageSampler shaderType);

private:
    TRHIThreadSafe<FInternalResources> _resources;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;

    static void ValidateEmptyUniform_(const FInternalResources& data, const FUniformID&, u32 idx);
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
