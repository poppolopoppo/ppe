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
        SLAB_ALLOCATOR(RHIDescriptor) Allocator;
        TMemoryView<VkWriteDescriptorSet> Descriptors;
        u32 DescriptorIndex{ 0 };

        template <typename T>
        TMemoryView<T> AllocateT(size_t n) NOEXCEPT {
            return TAllocatorTraits<Meta::TDecay<decltype(Allocator)>>::template AllocateT<T>(Allocator, n);
        }

        VkWriteDescriptorSet& NextWriteDescriptorSet() NOEXCEPT {
            VkWriteDescriptorSet& wds = Descriptors[DescriptorIndex++];
            wds = {};
            wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            return wds;
        }
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
        bool AllowEmptyResources;

        explicit FInternalResources(FDynamicData&& rdynamicData, FRawDescriptorSetLayoutID layoutId, bool allowEmptyResources) NOEXCEPT;
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
    static bool AddResource_(FUpdateDescriptors* pList, FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FBuffer& value);
    static bool AddResource_(FUpdateDescriptors* pList, FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FTexelBuffer& value);
    static bool AddResource_(FUpdateDescriptors* pList, FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FImage& value);
    static bool AddResource_(FUpdateDescriptors* pList, FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FTexture& value);
    static bool AddResource_(FUpdateDescriptors* pList, FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, const FPipelineResources::FSampler& value);
    static bool AddResource_(FUpdateDescriptors* pList, FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, const FPipelineResources::FRayTracingScene& value);

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
