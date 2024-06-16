#pragma once

#include "RHIVulkan_fwd.h"

#include "Meta/Utility.h"
#include "Thread/ReadWriteLock.h"
#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingShaderTable final : Meta::FNonCopyable {

    friend class FVulkanPipelineCache;

public:
    struct FShaderTable {
        VkPipeline Pipeline{ VK_NULL_HANDLE };
        FPipelineLayoutID LayoutId;
        u32 BufferOffset{ 0 };
        EShaderDebugMode Mode{ Default };
    };

    using FTables = TFixedSizeStack<FShaderTable, u32(EShaderDebugMode::_Count)>;

    struct FInternalData {
        FBufferID BufferId;
        FRTPipelineID PipelineId;
        FTables Tables;

        u32 RayGenOffset{ 0 };
        u32 RayMissOffset{ 0 };
        u32 RayHitOffset{ 0 };
        u32 CallableOffset{ 0 };
        u32 BlockSize{ 0 };

        u16 RayMissStride{ 0 };
        u16 RayHitStride{ 0 };
        u16 CallableStride{ 0 };

        Meta::TStaticBitset<3> AvailableShaders{}; // ray miss, ray hit, callable
    };

    FVulkanRayTracingShaderTable() = default;
#if USE_PPE_RHIDEBUG
    ~FVulkanRayTracingShaderTable();
#endif

    auto Read() const { return _data.LockShared(); }

    FRawRTPipelineID Pipeline() const { return *Read()->PipelineId; }
    FRawBufferID Buffer() const { return *Read()->BufferId; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebugName& DebugName() const { return _debugName; }
#endif

    NODISCARD bool Construct(ARG0_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    NODISCARD bool BindingsFor(
        FRawPipelineLayoutID* pLayout,
        VkPipeline* pPipeline,
        VkDeviceSize* pBlockSize,
        VkDeviceSize* pRayGenOffset, VkDeviceSize* pRayMissOffset, VkDeviceSize* pRayMissStride,
        VkDeviceSize* pRayHitOffset, VkDeviceSize* pRayHitStride,
        VkDeviceSize* pCallableOffset, VkDeviceSize* pCallableStride,
        Meta::TStaticBitset<3>* pAvailableShaders,
        EShaderDebugMode mode ) const;

private:
    TThreadSafe<FInternalData, EThreadBarrier::RWLock> _data;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
