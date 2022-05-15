#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/RayTracing/VulkanRayTracingGeometryInstance.h"

#include "Container/FlatSet.h"
#include "Container/Tuple.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingScene final {
public:
    using FInstance = FVulkanRayTracingSceneInstance;

    struct FInstancesData {
        VECTOR(RHIRayTracing, FInstance) GeometryInstances;
        u32 HitShadersPerInstance{ 0 };
        u32 MaxHitShaderCount{ 0 };
    };

    struct FInternalData {
        VkAccelerationStructureNV TopLevelAS{ VK_NULL_HANDLE };
        FMemoryID MemoryId;
        u32 MaxInstanceCount{ 0 };
        ERayTracingBuildFlags Flags{ Default };
    };

    FVulkanRayTracingScene() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanRayTracingScene();
#endif

    auto Read() const { return _data.LockShared(); }

    VkAccelerationStructureNV Handle() const { return Read()->TopLevelAS; }
    u32 MaxInstanceCount() const { return Read()->MaxInstanceCount; }
    ERayTracingBuildFlags Flags() const { return Read()->Flags; }

    auto ExclusiveData() { return _currentData.LockExclusive(); }
    auto SharedData() const { return _currentData.LockShared(); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(FVulkanResourceManager& resources, const FRayTracingSceneDesc& desc, FRawMemoryID memoryId, FVulkanMemoryObject& memoryObj ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    void SetGeometryInstances(
        FVulkanResourceManager& resources,
        TMemoryView<FVulkanRayTracingSceneInstance> instances,
        u32 instanceCount,
        u32 hitShadersPerInstance,
        u32 maxHitShaders ) const;

private:
    mutable TRHIThreadSafe<FInternalData> _data;

    mutable TThreadSafe<FInstancesData, EThreadBarrier::RWLock> _currentData;

#if USE_PPE_RHITASKNAME
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
