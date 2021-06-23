#pragma once

#include "Container/FlatSet.h"
#include "Container/Tuple.h"
#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingScene final {
public:

    struct FInstance {
        FInstanceID InstanceId;
        FRTGeometryID GeometryId;
        u32 IndexOffset{ UMax };

        FInstance() = default;
        FInstance(const FInstanceID& instanceId, FRTGeometryID&& geometryId, u32 offset) NOEXCEPT
        :   InstanceId(instanceId), GeometryId(std::move(geometryId)), IndexOffset(offset)
        {}

        bool operator ==(const FInstance& other) const { return operator ==(other.InstanceId); }
        bool operator !=(const FInstance& other) const { return operator !=(other.InstanceId); }

        bool operator < (const FInstance& other) const { return operator < (other.InstanceId); }
        bool operator >=(const FInstance& other) const { return operator >=(other.InstanceId); }

        bool operator ==(const FInstanceID& other) const { return (InstanceId == other); }
        bool operator !=(const FInstanceID& other) const { return (not operator ==(other)); }

        bool operator < (const FInstanceID& other) const { return (InstanceId < other); }
        bool operator >=(const FInstanceID& other) const { return (not operator < (other)); }
    };

    struct FInstancesData {
        FLATSET(RHIRayTracing, FInstance) GeometryInstances;
        u32 HitShadersPerInstance{ 0 };
        u32 MaxHitShaderCount{ 0 };
    };

    struct FInternalData {
        VkAccelerationStructureKHR TopLevelAS{ VK_NULL_HANDLE };
        FMemoryID MemoryId;
        u32 MaxInstanceCount{ 0 };
        ERayTracingBuildFlags Flags{ Default };
    };

    FVulkanRayTracingScene() = default;
    ~FVulkanRayTracingScene();

    auto Read() const { return _data.LockShared(); }

    VkAccelerationStructureKHR Handle() const { return Read()->TopLevelAS; }
    u32 MaxInstanceCount() const { return Read()->MaxInstanceCount; }
    ERayTracingBuildFlags Flags() const { return Read()->Flags; }

    auto ExclusiveData() { return _currentData.LockExclusive(); }
    auto SharedData() const { return _currentData.LockShared(); }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(FVulkanResourceManager& resources, const FRayTracingSceneDesc& desc, FRawMemoryID memoryId, FVulkanMemoryObject& pobj ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    void SetGeometryInstances(
        FVulkanResourceManager& resources,
        TMemoryView<const TTuple<FInstanceID, FRTGeometryID, u32>> instances,
        u32 instanceCount,
        u32 hitShadersPerInstance,
        u32 maxHitShaders ) const;

private:
    TRHIThreadSafe<FInternalData> _data;

    TThreadSafe<FInstancesData, EThreadBarrier::RWLock> _currentData;

#if USE_PPE_RHITASKNAME
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
