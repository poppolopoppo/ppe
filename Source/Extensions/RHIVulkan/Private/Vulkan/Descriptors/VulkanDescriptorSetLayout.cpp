
#include "stdafx.h"

#include "Vulkan/Descriptors/VulkanDescriptorSetLayout.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDescriptorSetLayout::FVulkanDescriptorSetLayout() NOEXCEPT = default;
//----------------------------------------------------------------------------
FVulkanDescriptorSetLayout::FVulkanDescriptorSetLayout(
    FBindings* pBindings,
    FOptionalFlags* pOptionalFlags,
    const FVulkanDevice& device,
    FSharedUniformMap&& uniforms ) {
    Assert(pBindings);
    Assert(pOptionalFlags);
    Assert(uniforms);

    const auto exclusivePool = _pool.LockExclusive();
    exclusivePool->Uniforms = std::move(uniforms);

    const bool enableDescriptorIndexing = device.Enabled().DescriptorIndexing;

    // bind uniforms

    hash_t uniformsHash{ hash_value(exclusivePool->Uniforms->size()) };

    pBindings->clear();
    pBindings->reserve(exclusivePool->Uniforms->size());

    pOptionalFlags->clear();
    pOptionalFlags->reserve(exclusivePool->Uniforms->size());

    VkDescriptorSetLayoutCreateFlags allFlags = 0;

    for (const auto& it : *exclusivePool->Uniforms) {
        Assert(it.first.Valid());
        hash_combine(uniformsHash, it.first);

        AssertReleaseMessage(L"requires Vulkan 1.2 or VK_EXT_descriptor_indexing", enableDescriptorIndexing or it.second.ArraySize != 0);

        VkDescriptorSetLayoutCreateFlags flags = 0;
        if (it.second.ArraySize == 0) {
            // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html
            //  indicates that descriptors in this binding that are not dynamically used need not contain valid descriptors at the time the
            //  descriptors are consumed.A descriptor is dynamically used if any shader invocation executes an instruction that performs any
            //  memory access using the descriptor.If a descriptor is not dynamically used, any resource referenced by the descriptor is not
            //  considered to be referenced during command execution.
            allFlags |= flags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
        }

        AddUniform_(pBindings, *exclusivePool, it.second);
        pOptionalFlags->push_back(std::move(flags));
    }

    if (not allFlags)
        pOptionalFlags->clear_ReleaseMemory();

    exclusivePool->HashValue = uniformsHash;
}
//----------------------------------------------------------------------------
FVulkanDescriptorSetLayout::FVulkanDescriptorSetLayout(FVulkanDescriptorSetLayout&& rvalue) NOEXCEPT
:   _pool(std::move(*rvalue._pool.LockExclusive()))
,   _descriptorSetCache(std::move(*rvalue._descriptorSetCache.LockExclusive())) {

}
//----------------------------------------------------------------------------
FVulkanDescriptorSetLayout::~FVulkanDescriptorSetLayout() {
    Assert_NoAssume(VK_NULL_HANDLE == _pool.LockExclusive()->Layout);
}
//----------------------------------------------------------------------------
bool FVulkanDescriptorSetLayout::Construct(const FVulkanDevice& device,
    const TMemoryView<const VkDescriptorSetLayoutBinding>& bindings,
    const TMemoryView<const VkDescriptorSetLayoutCreateFlags>& optionalFlags
    ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    const auto exclusivePool = _pool.LockExclusive();
    Assert(exclusivePool->Uniforms.valid());
    Assert_NoAssume(VK_NULL_HANDLE == exclusivePool->Layout);

    ONLY_IF_RHIDEBUG( _debugName = debugName );

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = checked_cast<u32>(bindings.size());
    info.pBindings = bindings.data();

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags{};
    flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flags.bindingCount = checked_cast<u32>(optionalFlags.size());
    flags.pBindingFlags = optionalFlags.data();

    if (not optionalFlags.empty())
        info.pNext = &flags;

    VK_CHECK( device.vkCreateDescriptorSetLayout(
        device.vkDevice(),
        &info,
        device.vkAllocator(),
        &exclusivePool->Layout ));

    ONLY_IF_RHIDEBUG( device.SetObjectName(
        FVulkanExternalObject{ exclusivePool->Layout },
        _debugName,
        VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT ));

    FPipelineResources::CreateDynamicData(
        &exclusivePool->ResourcesTemplate,
        *exclusivePool->Uniforms,
        exclusivePool->MaxIndex + 1,
        exclusivePool->ElementCount,
        exclusivePool->DynamicOffsetCount );

    return true;
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::TearDown(FVulkanResourceManager& resources) {
    const auto exclusivePool = _pool.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE != exclusivePool->Layout);

    if (exclusivePool->Layout) {
        const FVulkanDevice& device = resources.Device();
        device.vkDestroyDescriptorSetLayout(
            device.vkDevice(),
            exclusivePool->Layout,
            device.vkAllocator() );
    }

    const auto exclusiveCache = _descriptorSetCache.LockExclusive();
    if (not exclusiveCache->empty())
        resources.DescriptorManager().DeallocateDescriptorSets(exclusiveCache->MakeConstView());

    exclusiveCache->clear();

    exclusivePool->DynamicOffsetCount = 0;
    exclusivePool->ElementCount = 0;
    exclusivePool->HashValue = Default;
    exclusivePool->MaxIndex = 0;
    exclusivePool->Layout = VK_NULL_HANDLE;
    exclusivePool->PoolSizes.clear();
    exclusivePool->ResourcesTemplate = FDynamicData{};
    exclusivePool->Uniforms.reset();
}
//----------------------------------------------------------------------------
bool FVulkanDescriptorSetLayout::AllocateDescriptorSet(FVulkanDescriptorSet* pDescriptors, FVulkanResourceManager& resources) const {
    Assert(pDescriptors);
    *pDescriptors = { VK_NULL_HANDLE, UMax };

    const auto sharedPool = _pool.LockShared();

    if (_descriptorSetCache.LockExclusive()->Pop(pDescriptors))
        return true; // cache hit

    return resources.DescriptorManager().AllocateDescriptorSet(pDescriptors, sharedPool->Layout ARGS_IF_RHIDEBUG(_debugName.c_str()));
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::DeallocateDescriptorSet(FVulkanResourceManager& resources, const FVulkanDescriptorSet& descriptors) const {
    Assert(VK_NULL_HANDLE != descriptors.First);

    const auto sharedPool = _pool.LockShared();
    Unused(sharedPool);

    const auto exclusiveCache = _descriptorSetCache.LockExclusive();

    if (exclusiveCache->full()) {
        resources.DescriptorManager().DeallocateDescriptorSets(exclusiveCache->MakeConstView());
        exclusiveCache->clear();
    }

    exclusiveCache->Push(descriptors);
}
//----------------------------------------------------------------------------
bool FVulkanDescriptorSetLayout::operator==(const FVulkanDescriptorSetLayout& other) const NOEXCEPT {
    const auto lhs = _pool.LockShared();
    const auto rhs = other._pool.LockShared();

    if (lhs->HashValue != rhs->HashValue or
        not (lhs->Uniforms and rhs->Uniforms) or
        lhs->Uniforms->size() != rhs->Uniforms->size() ) {
        return false;
    }

    for (const auto& un : *lhs->Uniforms) {
        auto it = rhs->Uniforms->find(un.first);
        if ((rhs->Uniforms->end() == it) or (un.second != it->second))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
// AddUniform
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::BindDescriptor_(FBindings* pBinding, FInternalPool& pool, VkDescriptorType type, u32 bindingIndex, u32& arraySize, EShaderStages stageFlags) {
    Assert(pBinding);

    arraySize = (arraySize ? arraySize : MaxElementsInUnsizedDesc);
    pool.ElementCount += arraySize;

    VkDescriptorSetLayoutBinding bind{};
    bind.descriptorType = type;
    bind.stageFlags = VkCast(stageFlags);
    bind.binding = bindingIndex;
    bind.descriptorCount = arraySize;

    pool.MaxIndex = Max(pool.MaxIndex, bind.binding);

    pBinding->push_back(std::move(bind));

    for (VkDescriptorPoolSize& size : pool.PoolSizes) {
        if (size.type == type) {
            ++size.descriptorCount;
            return;
        }
    }

    pool.PoolSizes.Push(type, 1u);
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FVariantUniform& var) {
    Assert(var.Index.VKBinding() != UMax);

    std::visit([&](const auto& uniform) {
        AddUniform_(pBinding, pool, uniform, var.Index.VKBinding(), var.ArraySize, var.StageFlags);
    },  var.Data);
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FImage& img, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags) {
    pool.HashValue = hash_tuple(
        img.Type,
        bindingIndex,
        stageFlags,
        img.State,
        arraySize );

    BindDescriptor_(pBinding, pool,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        bindingIndex, arraySize, stageFlags);
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FTexture& tex, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags) {
    pool.HashValue = hash_tuple(
        tex.Type,
        bindingIndex,
        stageFlags,
        tex.State,
        arraySize );

    BindDescriptor_(pBinding, pool,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        bindingIndex, arraySize, stageFlags);
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FSampler& , u32 bindingIndex, u32 arraySize, EShaderStages stageFlags) {
    pool.HashValue = hash_tuple(
        bindingIndex,
        stageFlags,
        arraySize );

    BindDescriptor_(pBinding, pool,
        VK_DESCRIPTOR_TYPE_SAMPLER,
        bindingIndex, arraySize, stageFlags);
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FSubpassInput& spi, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags) {
    pool.HashValue = hash_tuple(
        spi.AttachmentIndex,
        spi.IsMultiSample,
        bindingIndex,
        stageFlags,
        spi.State,
        arraySize );

    BindDescriptor_(pBinding, pool,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        bindingIndex, arraySize, stageFlags);
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FUniformBuffer& ub, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags) {
    pool.HashValue = hash_tuple(
        ub.Size,
        bindingIndex,
        stageFlags,
        ub.State,
        arraySize );

    const bool isDynamic = (ub.State & EResourceState::_BufferDynamicOffset);

    BindDescriptor_(pBinding, pool,
        (isDynamic
            ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
            : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ),
        bindingIndex, arraySize, stageFlags);

    if (isDynamic)
        pool.DynamicOffsetCount += arraySize;
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FStorageBuffer& sb, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags) {
    pool.HashValue = hash_tuple(
        sb.StaticSize,
        sb.ArrayStride,
        bindingIndex,
        stageFlags,
        sb.State,
        arraySize );

    const bool isDynamic = (sb.State & EResourceState::_BufferDynamicOffset);

    BindDescriptor_(pBinding, pool,
        (isDynamic
            ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
            : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ),
        bindingIndex, arraySize, stageFlags);

    if (isDynamic)
        pool.DynamicOffsetCount += arraySize;
}
//----------------------------------------------------------------------------
void FVulkanDescriptorSetLayout::AddUniform_(FBindings* pBinding, FInternalPool& pool, const FPipelineDesc::FRayTracingScene& rts, u32 bindingIndex, u32 arraySize, EShaderStages stageFlags) {
    pool.HashValue = hash_tuple(
        rts.State,
        arraySize );

    BindDescriptor_(pBinding, pool,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
        bindingIndex, arraySize, stageFlags);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
