#pragma once

#include "Thread/ReadWriteLock.h"
#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanRayTracingShaderTable final : Meta::FNonCopyable {
public:
    struct FShaderTable {
        VkPipeline Pipeline{ VK_NULL_HANDLE };
        FPipelineLayoutID LayoutId;
        u32 BufferOffset{ 0 };
        EShaderDebugMode Mode{ Default };
    };

    using FTables = TFixedSizeStack<FShaderTable, u32(EShaderDebugMode::_Count)>;

    FVulkanRayTracingShaderTable() = default;
    ~FVulkanRayTracingShaderTable();

    FRawRTPipelineID Pipeline() const { return _pipelineId.Get(); }
    FRawBufferID Buffer() const { return _bufferId.Get(); }

#ifdef USE_PPE_RHIDEBUG
    FStringView DebugName() const { return _debugName; }
#endif

    bool Create(
#if USE_PPE_RHIDEBUG
        FStringView debugName
#endif
        );
    void TearDown(FVulkanResourceManager& resources);

    bool BindingsFor(
        FRawPipelineLayoutID* playout, VkPipeline* ppipeline,
        VkDeviceSize* pblockSize, VkDeviceSize* prayGenOffset,
        VkDeviceSize* prayMissOffset, VkDeviceSize*  prayMissStride,
        VkDeviceSize* prayHitOffset, VkDeviceSize* prayHitStride,
        VkDeviceSize* pcallableOffset, VkDeviceSize* pcallableStride,
        EShaderDebugMode mode ) const;

private:
    mutable FReadWriteLock _rwLock;

    FBufferID _bufferId;
    FRTPipelineID _pipelineId;
    FTables _tables;

    u32 _rayGenOffset{ 0 };
    u32 _rayMissOffset{ 0 };
    u32 _rayHitOffset{ 0 };
    u32 _callableOffset{ 0 };
    u32 _blockSIze{ 0 };

    u16 _rayMissStride{ 0 };
    u16 _rayHitStride{ 0 };
    u16 _callableStride{ 0 };

#ifdef USE_PPE_RHIDEBUG
    FVulkanDebugName _debugName;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
