#pragma once

#include "RHI_fwd.h"

#include "RHI/FrameGraphTask.h"
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
    FStringView     Name;

    FCommandBufferDesc() = default;
    explicit FCommandBufferDesc(EQueueType queueType) : QueueType(queueType) {}
};
//----------------------------------------------------------------------------
struct FStagingBlock {
    FRawBufferID RawBufferID;
    size_t Offset;
    void* Mapped;
};
//----------------------------------------------------------------------------
class ICommandBuffer : public FRefCountable {
public: // interface
    virtual ~ICommandBuffer() = default;

    virtual SFrameGraph FrameGraph() = 0;

    // Acquire next swapchain image. This image will be presented after command buffer execution.
    // Do not use this image in any other command buffers.
    virtual FRawImageID GetSwapchainImage(FRawSwapchainID swapchainId, ESwapchainImage type = ESwapchainImage::Primary) = 0;

    // Add input dependency.
    // Current command buffer will be executed on the GPU only when input dependencies finished execution.
    virtual bool DependsOn(const FCommandBufferRef&) = 0;

    // Allocate space in the staging buffer.
    virtual bool StagingAlloc(FStagingBlock* pstaging, size_t size, size_t align) = 0;

    // Starts tracking image state in current command buffer.
    // Image may be in immutable or mutable state, immutable state disables layout transitions and barrier placement.
    // If 'invalidate' sets to 'true' then previous content of the image may be invalidated, this may improve performance on GPU.
    virtual void AcquireImage(FRawImageID id, bool makeMutable, bool invalidate) = 0;

    // Starts tracking buffer state in current command buffer.
    // Buffer may be in immutable or mutable state, immutable state disables barrier placement that increases performance on CPU.
    virtual void AcquireBuffer(FRawBufferID id, bool makeMutable) = 0;

    // Tasks
    virtual FTaskHandle Task(const FSubmitRenderPass&) = 0;
    virtual FTaskHandle Task(const FDispatchCompute&) = 0;
    virtual FTaskHandle Task(const FDispatchComputeIndirect&) = 0;
    virtual FTaskHandle Task(const FCopyBuffer&) = 0;
    virtual FTaskHandle Task(const FCopyImage&) = 0;
    virtual FTaskHandle Task(const FCopyBufferToImage&) = 0;
    virtual FTaskHandle Task(const FCopyImageToBuffer&) = 0;
    virtual FTaskHandle Task(const FBlitImage&) = 0;
    virtual FTaskHandle Task(const FResolveImage&) = 0;
    virtual FTaskHandle Task(const FGenerateMipmaps&) = 0;
    virtual FTaskHandle Task(const FFillBuffer&) = 0;
    virtual FTaskHandle Task(const FClearColorImage&) = 0;
    virtual FTaskHandle Task(const FClearDepthStencilImage&) = 0;
    virtual FTaskHandle Task(const FUpdateBuffer&) = 0;
    virtual FTaskHandle Task(const FUpdateImage&) = 0;
    virtual FTaskHandle Task(const FReadBuffer&) = 0;
    virtual FTaskHandle Task(const FReadImage&) = 0;
    virtual FTaskHandle Task(const FPresent&) = 0;
    virtual FTaskHandle Task(const FUpdateRayTracingShaderTable&) = 0;
    virtual FTaskHandle Task(const FBuildRayTracingGeometry&) = 0;
    virtual FTaskHandle Task(const FBuildRayTracingScene&) = 0;
    virtual FTaskHandle Task(const FTraceRays&) = 0;
    virtual FTaskHandle Task(const FCustomTask&) = 0;

    // Begin shader time measurement for all subsequent tasks.
    // Draw tasks are not affected, but timemap enabled for render pass.
    // Dimension should be same as in 'dstImage' argument in 'EndShaderTimeMap()', otherwise result will be scaled.
    virtual bool BeginShaderTimeMap(const uint2& dim, EShaderStages stages = EShaderStages::All) = 0;

    // Stop shader time measurement, result will be copied into specified image.
    // Image must be RGBA UNorm/Float 2D image.
    virtual FTaskHandle EndShaderTimeMap(FRawImageID dstImage, FImageLayer layer = Default, FMipmapLevel level = Default, TMemoryView<FTaskHandle> dependsOn = Default) = 0;

    // Create render pass.
    virtual FLogicalPassID CreateRenderPass(const FRenderPassDesc&) = 0;

    // Add task to the render pass.
    virtual void Task(FLogicalPassID, const FDrawVertices&) = 0;
    virtual void Task(FLogicalPassID, const FDrawIndexed&) = 0;
    virtual void Task(FLogicalPassID, const FDrawVerticesIndirect&) = 0;
    virtual void Task(FLogicalPassID, const FDrawIndexedIndirect&) = 0;
    virtual void Task(FLogicalPassID, const FDrawMeshes&) = 0;
    virtual void Task(FLogicalPassID, const FDrawMeshesIndirect&) = 0;
    virtual void Task(FLogicalPassID, const FCustomDraw&) = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
