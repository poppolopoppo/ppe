
#include "stdafx.h"

#include "Vulkan/RayTracing/VulkanRayTracingScene.h"

#include "Vulkan/RayTracing/VulkanRayTracingGeometryInstance.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

#include "RHI/RayTracingDesc.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanRayTracingScene::~FVulkanRayTracingScene() {
    Assert_NoAssume(VK_NULL_HANDLE == _data.LockExclusive()->TopLevelAS);
}
#endif
//----------------------------------------------------------------------------
bool FVulkanRayTracingScene::Construct(
    FVulkanResourceManager& resources,
    const FRayTracingSceneDesc& desc,
    FRawMemoryID memoryId, FVulkanMemoryObject& memoryObj
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    const auto exclusiveData = _data.LockExclusive();
    LOG_CHECK(RHI, VK_NULL_HANDLE == exclusiveData->TopLevelAS);
    LOG_CHECK(RHI, not exclusiveData->MemoryId.Valid());
    LOG_CHECK(RHI, desc.MaxInstanceCount > 0);

    VkAccelerationStructureCreateInfoNV create{};
    create.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    create.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    create.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    create.info.instanceCount = desc.MaxInstanceCount;
    create.info.flags = VkCast(desc.Flags);

    const FVulkanDevice& device = resources.Device();
    VK_CHECK( device.vkCreateAccelerationStructureNV(
        device.vkDevice(),
        &create,
        device.vkAllocator(),
        &exclusiveData->TopLevelAS ));

    LOG_CHECK(RHI, memoryObj.AllocateAccelStruct(
        resources.MemoryManager(),
        exclusiveData->TopLevelAS ));

    exclusiveData->MaxInstanceCount = desc.MaxInstanceCount;
    exclusiveData->MemoryId = FMemoryID{ memoryId };
    exclusiveData->Flags = desc.Flags;

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->TopLevelAS, _debugName, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV);
    }
#endif

    return true;
}
//----------------------------------------------------------------------------
void FVulkanRayTracingScene::TearDown(FVulkanResourceManager& resources) {
    const auto exclusiveData = _data.LockExclusive();

    if (exclusiveData->TopLevelAS != VK_NULL_HANDLE) {
        const FVulkanDevice& device = resources.Device();
        device.vkDestroyAccelerationStructureNV(
            device.vkDevice(),
            exclusiveData->TopLevelAS,
            device.vkAllocator() );
    }

    if (exclusiveData->MemoryId.Valid()) {
        resources.ReleaseResource(exclusiveData->MemoryId.Release());
    }

    {
        const auto exclusiveInstances = _currentData.LockExclusive();

        for (auto& instance : exclusiveInstances->GeometryInstances) {
            resources.ReleaseResource(instance.GeometryId.Release());
        }

        exclusiveInstances->GeometryInstances.clear_ReleaseMemory();
    }

    exclusiveData->TopLevelAS = VK_NULL_HANDLE;
    exclusiveData->MemoryId = Default;
    exclusiveData->Flags = Default;
    exclusiveData->MaxInstanceCount = Zero;

    ONLY_IF_RHIDEBUG(_debugName.Clear());
}
//---------------------------------------------------------------------------
// 'instances' is sorted by instance ID and contains the strong references for geometries
void FVulkanRayTracingScene::SetGeometryInstances(
    FVulkanResourceManager& resources,
    TMemoryView<FVulkanRayTracingSceneInstance> instances, u32 instanceCount,
    u32 hitShadersPerInstance, u32 maxHitShaders ) const {
    const auto exclusiveData = _data.LockExclusive();

    Unused(exclusiveData);

    const auto exclusiveInstances = _currentData.LockExclusive();

    // release previous geometries
    for (auto& it : exclusiveInstances->GeometryInstances) {
        resources.ReleaseResource(it.GeometryId.Release());
    }

    exclusiveInstances->GeometryInstances.clear();
    exclusiveInstances->GeometryInstances.reserve(instanceCount);

    for (FVulkanRayTracingSceneInstance& it : instances.CutBefore(instanceCount)) {
        VerifyRelease( resources.AcquireResource(*it.GeometryId) );
        exclusiveInstances->GeometryInstances.emplace_back(std::move(it));
    }

    exclusiveInstances->HitShadersPerInstance = hitShadersPerInstance;
    exclusiveInstances->MaxHitShaderCount = maxHitShaders;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
