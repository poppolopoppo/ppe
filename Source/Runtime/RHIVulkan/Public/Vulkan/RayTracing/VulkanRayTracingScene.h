#pragma once

#include "Thread/ReadWriteLock.h"
#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingScene final : public FRefCountable {
public:

    struct FInstance {
        FInstanceID InstanceId;
        FRTGeometryID GeometryId;
        u32 IndexOffset{ UMax };

        FInstance() = default;
        FInstance(const FInstanceID& instanceId, FRTGeometryID&& geometryId, u32 offset)
        :   InstanceId(instanceId), GeometryId(std::move(geometryId)), IndexOffset(offset)
        {}

        bool operator ==(const FInstanceID& other) const { return (InstanceId == other); }
        bool operator !=(const FInstanceID& other) const { return (not operator ==(other)); }

        bool operator < (const FInstanceID& other) const { return (InstanceId < other); }
        bool operator >=(const FInstanceID& other) const { return (not operator < (other)); }
    };

    struct FInstancesData {
        FReadWriteLock RWLock;
        VECTOR(RHIRayTracing, FInstance) GeometryInstances;
        u32 HitShadersPerInstance{ 0 };
        u32 MaxHitShaderCount{ 0 };
    };

    struct FInternalData {
        VkAccelerationStructureKHR TopLevelAS{ VK_NULL_HANDLE };
        FMemoryID MemoryId;
        u32 MaxInstanceCount{ 0 };
        ERayTracingBuildFlags Flags{ Default };
        mutable FInstancesData InstancesData;
    };

    FVulkanRayTracingScene() = default;
    ~FVulkanRayTracingScene();

    auto Read() const { return _data.LockShared(); }

    VkAccelerationStructureKHR Handle() const { return Read()->TopLevelAS; }
    u32 MaxInstancCount() const { return Read()->MaxInstanceCount; }
    FInstancesData& CurrentData() const { return Read()->InstancesData; }
    ERayTracingBuildFlags Flags() const { return Read()->Flags; }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool Create(FVulkanMemoryObject* pobj, FVulkanResourceManager& resources, const FRayTracingGeometryDesc& desc, FRawMemoryID memoryId ARGS_IF_RHIDEBUG(FStringView debugName));
    void TearDown(FVulkanResourceManager& resources);

    void SetGeometryInstances(
        FVulkanResourceManager& resources,
        TMemoryView<const TTuple<FInstanceID, FRTGeometryID, u32>> instances,
        u32 instanceCount,
        u32 hitShadersPerInstance,
        u32 maxHitShaders ) const;

private:
    TRHIThreadSafe<FInternalData> _data;

#if USE_PPE_RHITASKNAME
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
