#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Descriptors/VulkanDescriptorSetLayout.h"

#include "RHI/PipelineResources.h"

#include "Allocator/SlabAllocator.h"

#include <variant>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanPipelineResources final : Meta::FNonCopyable {
public:
    struct FUpdateDescriptors {
        FSlabAllocator Allocator;
        VkWriteDescriptorSet* Descriptors{ nullptr };
        u32 DescriptorIndex{ 0 };
    };

    using FInfoVariant = std::variant<
        VkDescriptorBufferInfo,
        VkDescriptorImageInfo,
        VkAccelerationStructureKHR >;

    using FDynamicData = FPipelineResources::FDynamicData;

    struct FInternalResources {
        hash_t HashValue;
        FRawDescriptorSetLayoutID LayoutId;
        FVulkanDescriptorSet DescriptorSet;
        FDynamicData DynamicData;
        const bool AllowEmptyResources{ false };
    };

    FVulkanPipelineResources() = default;
    explicit FVulkanPipelineResources(FPipelineResources& resources);
    explicit FVulkanPipelineResources(const FPipelineResources& resources);
    ~FVulkanPipelineResources();

    FVulkanPipelineResources(FVulkanPipelineResources&& rvalue) NOEXCEPT;
    FVulkanPipelineResources& operator =(FVulkanPipelineResources&&) = delete;

    auto Read() const { return _resources.LockShared(); }

    hash_t HashValue() const { return Read()->HashValue; }

#ifdef USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    bool AllResourcesAlive(const FVulkanResourceManager& manager) const;

    NODISCARD bool Construct(FVulkanResourceManager& manager);
    void TearDown(FVulkanResourceManager& manager);

    template <typename _Each>
    void EachUniform(_Each&& each) const {
        Read()->DynamicData.EachUniform(std::forward<_Each>(each));
    }

    bool operator ==(const FVulkanPipelineResources& other) const;
    bool operator !=(const FVulkanPipelineResources& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVulkanPipelineResources& resources) { return resources.HashValue(); }

    static void CheckBufferUsage(const class FVulkanBuffer&, EResourceState state);
    static void CheckTexelBufferUsage(const class FVulkanBuffer&, EResourceState state);
    static void CheckImageUsage(const class FVulkanImage&, EResourceState state);

private:
    bool AddResource_(FUpdateDescriptors* pupdate, FVulkanResourceManager&, const FUniformID&, FPipelineResources::FBuffer&);
    bool AddResource_(FUpdateDescriptors* pupdate, FVulkanResourceManager&, const FUniformID&, FPipelineResources::FTexelBuffer&);
    bool AddResource_(FUpdateDescriptors* pupdate, FVulkanResourceManager&, const FUniformID&, FPipelineResources::FImage&);
    bool AddResource_(FUpdateDescriptors* pupdate, FVulkanResourceManager&, const FUniformID&, FPipelineResources::FTexture&);
    bool AddResource_(FUpdateDescriptors* pupdate, FVulkanResourceManager&, const FUniformID&, const FPipelineResources::FSampler&);
    bool AddResource_(FUpdateDescriptors* pupdate, FVulkanResourceManager&, const FUniformID&, const FPipelineResources::FRayTracingScene&);

    void LogUniform_(const FUniformID&, u32 idx) const;

    TRHIThreadSafe<FInternalResources> _resources;

#ifdef USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
