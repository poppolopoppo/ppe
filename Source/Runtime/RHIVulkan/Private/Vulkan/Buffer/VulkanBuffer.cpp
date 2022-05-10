
#include "stdafx.h"

#include "Vulkan/Buffer/VulkanBuffer.h"

#include "Vulkan/Common/VulkanEnums.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/Memory/VulkanMemoryObject.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static VkAccessFlagBits AllBufferReadAccessMasks_(VkBufferUsageFlags usage) {
    VkAccessFlagBits result = static_cast<VkAccessFlagBits>(0);

    for (VkBufferUsageFlags t = 1; t <= usage; t <<= 1) {
        if (not (usage & t))
            continue;

        switch (static_cast<VkBufferUsageFlagBits>(t)) {
        case VK_BUFFER_USAGE_TRANSFER_SRC_BIT: result |= VK_ACCESS_TRANSFER_READ_BIT; break;
        case VK_BUFFER_USAGE_TRANSFER_DST_BIT: break;
        case VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT: result |= VK_ACCESS_SHADER_READ_BIT; break;
        case VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT: result |= VK_ACCESS_SHADER_READ_BIT; break;
        case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT: result |= VK_ACCESS_UNIFORM_READ_BIT; break;
        case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT: result |= VK_ACCESS_SHADER_READ_BIT; break;
        case VK_BUFFER_USAGE_INDEX_BUFFER_BIT: result |= VK_ACCESS_INDEX_READ_BIT; break;
        case VK_BUFFER_USAGE_VERTEX_BUFFER_BIT: result |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT; break;
        case VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT: result |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT; break;
        case VK_BUFFER_USAGE_RAY_TRACING_BIT_NV: break;
        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR: break;
        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR: break;

        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT:
        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT:
        case VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT:
        case VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT:
        case VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM: break;	// to shutup compiler warnings
        }
    }

    return result;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FVulkanBuffer::FInternalData::IsReadOnly() const NOEXCEPT {
    return not (Desc.Usage ^ (
        EBufferUsage::TransferDst |
        EBufferUsage::StorageTexel |
        EBufferUsage::Storage |
        EBufferUsage::RayTracing));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanBuffer::~FVulkanBuffer() {
#   if USE_PPE_ASSERT
    const auto& sharedData = _data.LockShared();
    Assert_NoAssume(VK_NULL_HANDLE == sharedData->vkBuffer);
    Assert_NoAssume(not sharedData->MemoryId);
#   endif
}
#endif
//----------------------------------------------------------------------------
FVulkanBuffer::FVulkanBuffer(FVulkanBuffer&& rvalue) NOEXCEPT
:   _data(std::move(*rvalue._data.LockExclusive()))
,   _viewMap(std::move(*rvalue._viewMap.LockExclusive()))
{}
//----------------------------------------------------------------------------
bool FVulkanBuffer::Construct(
    FVulkanResourceManager& resources,
    const FBufferDesc& desc,
    FRawMemoryID memoryId, FVulkanMemoryObject& memoryObject,
    EVulkanQueueFamilyMask queueFamilyMask
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(memoryId.Valid());
    Assert(desc.SizeInBytes > 0);
    Assert_NoAssume(desc.Usage != Default);

    const FVulkanDevice& device = resources.Device();
    AssertRelease( IsSupported(device, desc, static_cast<EMemoryType>(memoryObject.MemoryType())) );

    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusiveData->vkBuffer);
    Assert_NoAssume(not exclusiveData->MemoryId.Valid());
    Assert_NoAssume(not exclusiveData->OnRelease.Valid());

    exclusiveData->Desc = desc;
    exclusiveData->MemoryId = FMemoryID{ memoryId };
    exclusiveData->IsExternal = false;

    // create buffer
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.usage = VkCast(exclusiveData->Desc.Usage);
    info.size = static_cast<VkDeviceSize>(exclusiveData->Desc.SizeInBytes);

    TStaticArray<u32, 8> queueFamilyIndices;

    // setup sharing mode
    if (queueFamilyMask != Default) {
        info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = queueFamilyIndices.data();

        for (u32 i = 0, mask = 1u << 1;
            mask <= static_cast<u32>(queueFamilyMask) &&
            info.queueFamilyIndexCount < queueFamilyIndices.size();
            ++i, mask = 1u << i ) {
            if (Meta::EnumHas(queueFamilyMask, mask))
                queueFamilyIndices[info.queueFamilyIndexCount++] = i;
        }
    }

    // reset exclusive mode when less than 2 queues involved
    if (info.queueFamilyIndexCount < 2) {
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.pQueueFamilyIndices = nullptr;
        info.queueFamilyIndexCount = 0;
    }

    VK_CHECK( device.vkCreateBuffer(device.vkDevice(), &info, device.vkAllocator(), &exclusiveData->vkBuffer) );

    if (not memoryObject.AllocateBuffer(resources.MemoryManager(), exclusiveData->vkBuffer)) {
        RHI_LOG(Error, L"failed to allocate memory for buffer '{0}'", debugName);
        device.vkDestroyBuffer(device.vkDevice(), exclusiveData->vkBuffer, device.vkAllocator());
        return false;
    }

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->vkBuffer, _debugName, VK_OBJECT_TYPE_BUFFER);
    }
#endif

    exclusiveData->ReadAccessMask = AllBufferReadAccessMasks_(info.usage);
    exclusiveData->QueueFamilyMask = queueFamilyMask;

    Assert_NoAssume(not exclusiveData->IsExternal);
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanBuffer::Construct(
    const FVulkanDevice& device,
    const FVulkanExternalBufferDesc& desc,
    FOnReleaseExternalBuffer&& onRelease
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(desc.SizeInBytes > 0);
    Assert(desc.Buffer);

    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusiveData->vkBuffer);
    Assert_NoAssume(not exclusiveData->MemoryId);

    exclusiveData->vkBuffer = desc.Buffer;
    exclusiveData->Desc.SizeInBytes = desc.SizeInBytes;
    exclusiveData->Desc.Usage = RHICast(desc.Usage);
    exclusiveData->IsExternal = true;

    LOG_CHECK(RHI, IsSupported(device, exclusiveData->Desc, EMemoryType::Default));

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->vkBuffer, _debugName, VK_OBJECT_TYPE_BUFFER);
    }
#endif

    LOG_CHECK(RHI, VK_QUEUE_FAMILY_IGNORED == desc.QueueFamily); // not supported yet
    LOG_CHECK(RHI, desc.ConcurrentQueueFamilyIndices.empty() || desc.ConcurrentQueueFamilyIndices.size() >= 2);

    exclusiveData->QueueFamilyMask = Zero;
    for (u32 index : desc.ConcurrentQueueFamilyIndices)
        exclusiveData->QueueFamilyMask |= bit_cast<EVulkanQueueFamily>(index);

    exclusiveData->OnRelease = std::move(onRelease);

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanBuffer::Construct(
    const FVulkanDevice& device,
    const FBufferDesc& desc,
    FExternalBuffer externalBuffer,
    FOnReleaseExternalBuffer&& onRelease,
    TMemoryView<const u32> queueFamilyIndices
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(desc.SizeInBytes > 0);
    Assert(externalBuffer);
    Assert_NoAssume(queueFamilyIndices.empty() || queueFamilyIndices.size() >= 2);
    AssertRelease(IsSupported(device, desc, EMemoryType::Default));

    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusiveData->vkBuffer);
    Assert_NoAssume(not exclusiveData->MemoryId);

    exclusiveData->vkBuffer = FVulkanExternalObject(externalBuffer).Cast<VkBuffer>();
    exclusiveData->Desc = desc;
    exclusiveData->OnRelease = std::move(onRelease);
    exclusiveData->IsExternal = true;
    exclusiveData->ReadAccessMask = AllBufferReadAccessMasks_(static_cast<VkBufferUsageFlags>(desc.Usage));

    exclusiveData->QueueFamilyMask = Default;
    for (u32 id : queueFamilyIndices)
        exclusiveData->QueueFamilyMask |= static_cast<EVulkanQueueFamily>(id);

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->vkBuffer, _debugName, VK_OBJECT_TYPE_BUFFER);
    }
#endif

    Assert_NoAssume(exclusiveData->IsExternal);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanBuffer::TearDown(FVulkanResourceManager& resources) {
    const FVulkanDevice& device = resources.Device();

    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE != exclusiveData->vkBuffer);

    { // release buffer views first
        const auto exclusiveViewMap = _viewMap.LockExclusive();
        for (auto& view : *exclusiveViewMap)
            device.vkDestroyBufferView(device.vkDevice(), view.second, device.vkAllocator());
        exclusiveViewMap->clear_ReleaseMemory();
    }

    if (exclusiveData->IsExternal) {
        if (exclusiveData->OnRelease)
            exclusiveData->OnRelease(FVulkanExternalObject(exclusiveData->vkBuffer).ExternalBuffer());
    }
    else
        device.vkDestroyBuffer(device.vkDevice(), exclusiveData->vkBuffer, device.vkAllocator());

    if (exclusiveData->MemoryId)
        resources.ReleaseResource(exclusiveData->MemoryId.Release());

    exclusiveData->vkBuffer = VK_NULL_HANDLE;
    exclusiveData->Desc = FBufferDesc{};
    exclusiveData->QueueFamilyMask = Default;
    exclusiveData->OnRelease.Reset();

    ONLY_IF_RHIDEBUG(_debugName.Clear());
    Assert_NoAssume(not exclusiveData->MemoryId.Valid());
}
//----------------------------------------------------------------------------
VkBufferView FVulkanBuffer::MakeView(const FVulkanDevice& device, const FBufferViewDesc& desc) const {
    const auto sharedData = _data.LockShared();

    { // any created image view already created ?
        const auto sharedViewMap = _viewMap.LockShared();
        const auto it = sharedViewMap->find(desc);
        if (sharedViewMap->end() != it) {
            Assert_NoAssume(VK_NULL_HANDLE != it->second);
            return it->second; // cache hit
        }
    }

    // create a new view and register in the cache
    const auto exclusiveViewMap = _viewMap.LockExclusive();

    const auto[it, inserted] = exclusiveViewMap->insert({ desc, VK_NULL_HANDLE });
    if (inserted) {
        VkBufferViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        info.flags = 0;
        info.buffer = sharedData->vkBuffer;
        info.format = VkCast(desc.Format);
        info.offset = checked_cast<VkDeviceSize>(desc.Offset);
        info.range = checked_cast<VkDeviceSize>(desc.SizeInBytes);

        VK_CALL( device.vkCreateBufferView(device.vkDevice(), &info, device.vkAllocator(), &it->second) );
    }

    Assert_NoAssume(VK_NULL_HANDLE != it->second);
    return it->second;
}
//----------------------------------------------------------------------------
bool FVulkanBuffer::IsSupported(const FVulkanDevice& device, const FBufferDesc& desc, EMemoryType memoryType) NOEXCEPT {
    Unused(memoryType);

    for (u32 t = 1; t <= static_cast<u32>(desc.Usage); t <<= 1) {
        if (not Meta::EnumHas(desc.Usage, t))
            continue;

        switch (static_cast<EBufferUsage>(t)) {
        case EBufferUsage::TransferSrc: break;
        case EBufferUsage::TransferDst: break;
        case EBufferUsage::UniformTexel: break;
        case EBufferUsage::StorageTexel: break;
        case EBufferUsage::StorageTexelAtomic: break;
        case EBufferUsage::Uniform: break;
        case EBufferUsage::Storage: break;
        case EBufferUsage::Index: break;
        case EBufferUsage::Vertex: break;
        case EBufferUsage::Indirect: break;

        case EBufferUsage::RayTracing:
            if (not (device.Enabled().RayTracingKHR || device.Enabled().RayTracingNV)) return false;
            break;
        case EBufferUsage::VertexPplnStore:
            if (not device.Features().vertexPipelineStoresAndAtomics) return false;
            break;
        case EBufferUsage::FragmentPplnStore:
            if (not device.Features().fragmentStoresAndAtomics) return false;
            break;

        default: AssertNotImplemented();
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanBuffer::IsSupported(const FVulkanDevice& device, const FBufferViewDesc& desc) const NOEXCEPT {
    const auto sharedData = _data.LockShared();

    VkFormatProperties props{};
    device.vkGetPhysicalDeviceFormatProperties(device.vkPhysicalDevice(), VkCast(desc.Format), &props);

    const VkFormatFeatureFlags deviceFlags = props.bufferFeatures;
    VkFormatFeatureFlags bufferFlags = 0;

    for (u32 t = 0; t <= static_cast<u32>(sharedData->Desc.Usage); t <<= 1) {
        if (not Meta::EnumHas(sharedData->Desc.Usage, t))
            continue;

        switch (static_cast<EBufferUsage>(t)) {
        case EBufferUsage::UniformTexel: bufferFlags |= VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT; break;
        case EBufferUsage::StorageTexel: bufferFlags |= VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT; break;
        case EBufferUsage::StorageTexelAtomic:  bufferFlags |= VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT; break;

        case EBufferUsage::TransferSrc: break;
        case EBufferUsage::TransferDst: break;
        case EBufferUsage::Uniform: break;
        case EBufferUsage::Storage: break;
        case EBufferUsage::Index: break;
        case EBufferUsage::Vertex: break;
        case EBufferUsage::Indirect: break;
        case EBufferUsage::RayTracing: break;
        case EBufferUsage::VertexPplnStore: break;
        case EBufferUsage::FragmentPplnStore: break;
        case EBufferUsage::_Last: break;
        case EBufferUsage::All: break;
        case EBufferUsage::Transfer: break;
        case EBufferUsage::Unknown: break;

        default: AssertNotImplemented();
        }
    }

    return !!(bufferFlags & deviceFlags);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
