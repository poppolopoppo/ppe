// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/RayTracing/VulkanRayTracingShaderTable.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanRayTracingShaderTable::~FVulkanRayTracingShaderTable() {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(exclusiveData->Tables.empty());
    Assert_NoAssume(not exclusiveData->BufferId.Valid());
    Assert_NoAssume(not exclusiveData->PipelineId.Valid());
}
#endif
//----------------------------------------------------------------------------
bool FVulkanRayTracingShaderTable::Construct(ARG0_IF_RHIDEBUG(FConstChar debugName)) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(not exclusiveData->BufferId);
    Assert(not exclusiveData->PipelineId);

    exclusiveData->AvailableShaders.clear();

    ONLY_IF_RHIDEBUG(_debugName = debugName);
    return true;
}
//----------------------------------------------------------------------------
void FVulkanRayTracingShaderTable::TearDown(FVulkanResourceManager& resources) {
    const auto exclusiveData = _data.LockExclusive();

    const FVulkanDevice& device = resources.Device();

    for (auto& table : exclusiveData->Tables) {
        device.vkDestroyPipeline(
            device.vkDevice(),
            table.Pipeline,
            device.vkAllocator() );
        resources.ReleaseResource(table.LayoutId.Release());
    }

    if (exclusiveData->BufferId.Valid()) {
        resources.ReleaseResource(exclusiveData->BufferId.Release());
    }

    if (exclusiveData->PipelineId.Valid()) {
        resources.ReleaseResource(exclusiveData->PipelineId.Release());
    }

    exclusiveData->AvailableShaders.clear();
    exclusiveData->Tables.clear();

    ONLY_IF_RHIDEBUG(_debugName.Clear());
}
//----------------------------------------------------------------------------
bool FVulkanRayTracingShaderTable::BindingsFor(
    FRawPipelineLayoutID* pLayout,
    VkPipeline* pPipeline,
    VkDeviceSize* pBlockSize,
    VkDeviceSize* pRayGenOffset, VkDeviceSize* pRayMissOffset, VkDeviceSize* pRayMissStride,
    VkDeviceSize* pRayHitOffset, VkDeviceSize* pRayHitStride,
    VkDeviceSize* pCallableOffset, VkDeviceSize* pCallableStride,
    Meta::TStaticBitset<3>* pAvailableShaders,
    EShaderDebugMode mode ) const {
    const auto sharedData = _data.LockShared();

    for (const auto& table : sharedData->Tables) {
        if (table.Mode == mode) {
            *pLayout = *table.LayoutId;
            *pPipeline = table.Pipeline;

            *pBlockSize = checked_cast<VkDeviceSize>(sharedData->BlockSize + table.BufferOffset);
            *pRayGenOffset = checked_cast<VkDeviceSize>(sharedData->RayGenOffset + table.BufferOffset);
            *pRayMissOffset = checked_cast<VkDeviceSize>(sharedData->RayMissOffset + table.BufferOffset);
            *pRayMissStride = checked_cast<VkDeviceSize>(sharedData->RayMissStride);
            *pRayHitOffset = checked_cast<VkDeviceSize>(sharedData->RayHitOffset + table.BufferOffset);
            *pRayHitStride = checked_cast<VkDeviceSize>(sharedData->RayHitStride);
            *pCallableOffset = checked_cast<VkDeviceSize>(sharedData->CallableOffset + table.BufferOffset);
            *pCallableStride = checked_cast<VkDeviceSize>(sharedData->CallableStride);

            *pAvailableShaders = sharedData->AvailableShaders;

            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
