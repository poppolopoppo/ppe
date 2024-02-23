#pragma once

#include "RayTracingTask.h"
#include "RHI_fwd.h"

#include "RHI/FrameTask.h"
#include "RHI/RayTracingDesc.h"
#include "RHI/RenderPassDesc.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ShaderEnums.h"

#include "Memory/RefPtr.h"
#include "IO/StringView.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FCommandBufferDesc {
    EQueueType      QueueType{ EQueueType::Graphics };
    EDebugFlags     DebugFlags{ Default };
    FStringLiteral  Name;

    FCommandBufferDesc() = default;
    explicit FCommandBufferDesc(EQueueType queueType) : QueueType(queueType) {}

    FCommandBufferDesc& SetQueueType(EQueueType value) { QueueType = value; return *this; }
    FCommandBufferDesc& SetDebugFlags(EDebugFlags value) { DebugFlags = value; return *this; }
    FCommandBufferDesc& SetName(FStringLiteral value) { Name = value; return *this; }
};
PPE_ASSUME_TYPE_AS_POD(FCommandBufferDesc)
//----------------------------------------------------------------------------
struct FStagingBlock {
    FRawBufferID RawBufferID;
    size_t Offset{ UMax };
    ubyte* Mapped{ nullptr };
};
PPE_ASSUME_TYPE_AS_POD(FStagingBlock)
//----------------------------------------------------------------------------
class PPE_RHI_API ICommandBuffer : public FRefCountable {
public: // interface
    virtual ~ICommandBuffer() = default;

    virtual SFrameGraph FrameGraph() const NOEXCEPT = 0;

    // Implement for refptr recycling, see RefPtr-inl.h
    virtual void OnStrongRefCountReachZero() NOEXCEPT = 0;

    // Acquire next swapchain image. This image will be presented after command buffer execution.
    // Do not use this image in any other command buffers.
    NODISCARD virtual FRawImageID SwapchainImage(FRawSwapchainID swapchainId) = 0;

    // Add input dependency.
    // Current command buffer will be executed on the GPU only when input dependencies finished execution.
    virtual bool DependsOn(const SCommandBatch& batch) = 0;

    // Allocate space in the staging buffer.
    NODISCARD virtual bool StagingAlloc(FStagingBlock* pStaging, size_t size, size_t align) = 0;

    // Starts tracking image state in current command buffer.
    // Image may be in immutable or mutable state, immutable state disables layout transitions and barrier placement.
    // If 'invalidate' sets to 'true' then previous content of the image may be invalidated, this may improve performance on GPU.
    virtual void AcquireImage(FRawImageID id, bool makeMutable, bool invalidate) = 0;

    // Starts tracking buffer state in current command buffer.
    // Buffer may be in immutable or mutable state, immutable state disables barrier placement that increases performance on CPU.
    virtual void AcquireBuffer(FRawBufferID id, bool makeMutable) = 0;

    // Tasks
    NODISCARD virtual PFrameTask Task(const FSubmitRenderPass& task) = 0;
    NODISCARD virtual PFrameTask Task(const FDispatchCompute& task) = 0;
    NODISCARD virtual PFrameTask Task(const FDispatchComputeIndirect& task) = 0;
    NODISCARD virtual PFrameTask Task(const FCopyBuffer& task) = 0;
    NODISCARD virtual PFrameTask Task(const FCopyImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FCopyBufferToImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FCopyImageToBuffer& task) = 0;
    NODISCARD virtual PFrameTask Task(const FBlitImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FResolveImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FGenerateMipmaps& task) = 0;
    NODISCARD virtual PFrameTask Task(const FFillBuffer& task) = 0;
    NODISCARD virtual PFrameTask Task(const FClearColorImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FClearDepthStencilImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FUpdateBuffer& task) = 0;
    NODISCARD virtual PFrameTask Task(const FUpdateImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FReadBuffer& task) = 0;
    NODISCARD virtual PFrameTask Task(const FReadImage& task) = 0;
    NODISCARD virtual PFrameTask Task(const FPresent& task) = 0;
    NODISCARD virtual PFrameTask Task(const FUpdateRayTracingShaderTable& task) = 0;
    NODISCARD virtual PFrameTask Task(const FBuildRayTracingGeometry& task) = 0;
    NODISCARD virtual PFrameTask Task(const FBuildRayTracingScene& task) = 0;
    NODISCARD virtual PFrameTask Task(const FTraceRays& task) = 0;
    NODISCARD virtual PFrameTask Task(const FCustomTask& task) = 0;

    // Create render pass.
    NODISCARD virtual FLogicalPassID CreateRenderPass(const FRenderPassDesc& desc) = 0;

    // Add task to the render pass.
    virtual void Task(FLogicalPassID renderPass, const FDrawVertices& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawIndexed& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawVerticesIndirect& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawIndexedIndirect& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawVerticesIndirectCount& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawIndexedIndirectCount& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawMeshes& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawMeshesIndirect& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FDrawMeshesIndirectCount& draw) = 0;
    virtual void Task(FLogicalPassID renderPass, const FCustomDraw& draw) = 0;

#if USE_PPE_RHIDEBUG
    // Begin shader time measurement for all subsequent tasks.
    // Draw tasks are not affected, but timemap enabled for render pass.
    // Dimension should be same as in 'dstImage' argument in 'EndShaderTimeMap()', otherwise result will be scaled.
    NODISCARD virtual bool BeginShaderTimeMap(const uint2& dim, EShaderStages stages = EShaderStages::All) = 0;

    // Stop shader time measurement, result will be copied into specified image.
    // Image must be RGBA UNorm/Float 2D image.
    NODISCARD virtual PFrameTask EndShaderTimeMap(FRawImageID dstImage, FImageLayer layer = Default, FMipmapLevel level = Default, TMemoryView<PFrameTask> dependsOn = Default) = 0;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
