#pragma once

#include "Core_fwd.h"

#ifdef EXPORT_PPE_RUNTIME_RHI
#   define PPE_RHI_API DLL_EXPORT
#else
#   define PPE_RHI_API DLL_IMPORT
#endif

#define USE_PPE_RHIMOBILE (0)
#define USE_PPE_RHIDEBUG (USE_PPE_ASSERT || USE_PPE_MEMORY_DEBUGGING)
#define USE_PPE_RHIOPTIMIZEIDS (!USE_PPE_RHIDEBUG)
#define USE_PPE_RHIPROFILING (!USE_PPE_FINAL_RELEASE)
#define USE_PPE_RHITASKNAME (USE_PPE_RHIPROFILING)

#if USE_PPE_RHIDEBUG
#   define ARG0_IF_RHIDEBUG(...) __VA_ARGS__
#   define ARGS_IF_RHIDEBUG(...) COMMA __VA_ARGS__
#   define ONLY_IF_RHIDEBUG(...) __VA_ARGS__
#else
#   define ARG0_IF_RHIDEBUG(...)
#   define ARGS_IF_RHIDEBUG(...)
#   define ONLY_IF_RHIDEBUG(...) NOOP()
#endif

#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger_fwd.h"
#include "Maths/ScalarVector.h"
#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"
#include "Meta/StronglyTyped.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRHIException;
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FWindowHandle);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FWindowSurface);
using FRawData = RAWSTORAGE(RHIRawData, u8);
template <typename T>
using TArray = VECTORINSITU(RHIMisc, T, 5);
template <typename T>
using TRHIThreadSafe = TThreadSafe<T, (!!USE_PPE_RHIDEBUG)>;
//----------------------------------------------------------------------------
enum class EQueueType : u32;
enum class EQueueUsage : u32;
enum class EMemoryType : u32;
enum class EBufferUsage : u32;
enum class EImageType : u32;
enum class EImageUsage : u32;
enum class EImageAspect : u32;
enum class ESwapchainImage : u32;
enum class EAttachmentStoreOp : u32;
enum class EShadingRatePalette : u8;
enum class EPixelFormat : u32;
enum class EColorSpace : u32;
enum class EFragmentOutput : u32;
enum class EDebugFlags : u32;
//----------------------------------------------------------------------------
enum class EBlendFactor : u32;
enum class EBlendOp : u32;
enum class ELogicOp : u32;
enum class EColorMask : u8;
enum class ECompareOp : u8;
enum class EStencilOp : u8;
enum class EPolygonMode : u32;
enum class EPrimitiveTopology : u32;
enum class ECullMode : u8;
enum class EPipelineDynamicState : u8;
//----------------------------------------------------------------------------
enum class ERayTracingGeometryFlags : u32;
enum class ERayTracingInstanceFlags : u32;
enum class ERayTracingBuildFlags : u32;
//----------------------------------------------------------------------------
enum class EShaderType : u32;
enum class EShaderStages : u32;
enum class EShaderAccess : u32;
enum class EShaderLangFormat : u32;
enum class EShaderDebugMode : u32;
//----------------------------------------------------------------------------
enum class ETextureFilter : u32;
enum class EMipmapFilter : u32;
enum class EAddressMode : u32;
enum class EBorderColor : u32;
//----------------------------------------------------------------------------
enum class EIndexFormat : u32;
enum class EVertexFormat : u32;
//----------------------------------------------------------------------------
enum class EPixelValueType : u32;
//----------------------------------------------------------------------------
enum class EPresentMode : u32;
enum class ESurfaceTransform : u32;
//----------------------------------------------------------------------------
struct FBindingIndex;
struct FBufferDesc;
struct FBufferViewDesc;
struct FBufferView;
//----------------------------------------------------------------------------
struct FImageDesc;
struct FImageViewDesc;
struct FImageView;
PPE_STRONGLYTYPED_NUMERIC_DEF(u32, FImageLayer);
PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(u16, FImageSwizzle, 0x4321); // 0 - unknown, 1 - R, 2 - G, 3 - B, 4 - A, 5 - O, 6 - 1, example: RGB0 - 0x1235
PPE_STRONGLYTYPED_NUMERIC_DEF(u32, FMipmapLevel);
PPE_STRONGLYTYPED_NUMERIC_DEF(u32, FMultiSamples);
//----------------------------------------------------------------------------
struct FMemoryDesc;
//----------------------------------------------------------------------------
struct FSamplerDesc;
//----------------------------------------------------------------------------
struct FSwapchainDesc;
//----------------------------------------------------------------------------
struct FTaskHandle;
struct FSubmitRenderPass;
struct FDispatchCompute;
struct FDispatchComputeIndirect;
struct FCopyBuffer;
struct FCopyImage;
struct FCopyBufferToImage;
struct FCopyImageToBuffer;
struct FBlitImage;
struct FResolveImage;
struct FGenerateMipmaps;
struct FFillBuffer;
struct FClearColorImage;
struct FClearDepthStencilImage;
struct FUpdateBuffer;
struct FUpdateImage;
struct FReadBuffer;
struct FReadImage;
struct FPresent;
struct FCustomTask;
//----------------------------------------------------------------------------
struct FUpdateRayTracingShaderTable;
struct FBuildRayTracingGeometry;
struct FBuildRayTracingScene;
struct FTraceRays;
//----------------------------------------------------------------------------
struct FDrawVertices;
struct FDrawIndexed;
struct FDrawVerticesIndirect;
struct FDrawIndexedIndirect;
struct FDrawMeshes;
struct FDrawMeshesIndirect;
struct FCustomDraw;
//----------------------------------------------------------------------------
struct FCommandBufferDesc;
struct FCommandBufferBatch;
FWD_INTERFACE_REFPTR(CommandBatch);
FWD_INTERFACE_REFPTR(CommandBuffer);
struct FStagingBlock;
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(DrawContext);
//----------------------------------------------------------------------------
FWD_REFPTR(PipelineResources);
//----------------------------------------------------------------------------
struct FFrameStatistics;
FWD_INTERFACE_REFPTR(FrameGraph);
class IFrameTask;
using PFrameTask = TPtrRef<IFrameTask>;
//----------------------------------------------------------------------------
struct FPipelineBaseUniform;
struct FPipelineDesc;
struct FGraphicsPipelineDesc;
struct FComputePipelineDesc;
struct FMeshPipelineDesc;
struct FRayTracingPipelineDesc;
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(PipelineCompiler);
//----------------------------------------------------------------------------
struct FRayTracingGeometryDesc;
struct FRayTracingSceneDesc;
//----------------------------------------------------------------------------
struct FRenderTarget;
struct FRenderViewport;
struct FRenderPassDesc;
//----------------------------------------------------------------------------
struct FBlendState;
struct FColorBufferState;
struct FDepthBufferState;
struct FInputAssemblyState;
struct FMultisampleState;
struct FStencilBufferState;
struct FRasterizationState;
struct FRenderState;
//----------------------------------------------------------------------------
enum class EDrawDynamicState : u32;
struct FDrawDynamicStates;
struct FImageDataRange;
struct FImageSubresourceRange;
struct FPushConstantData;
struct FVertexBuffer;
PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(float, FDepthValue, 0.0f);
PPE_STRONGLYTYPED_NUMERIC_DEFAULTVALUE_DEF(u8, FStencilValue, 0xFF);
struct FDepthStencilValue {
    FDepthValue Depth;
    FStencilValue Stencil;
};
//----------------------------------------------------------------------------
template <typename T>
class IShaderData;
template <typename T>
using PShaderData = TRefPtr< IShaderData<T> >;
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FShaderHandle);
using PShaderModule = PShaderData< FShaderHandle >;
struct FShaderSource;
using PShaderSource = PShaderData< FShaderSource >;
//----------------------------------------------------------------------------
template <typename T>
class TResourceProxy;
//----------------------------------------------------------------------------
struct FVertexAttribute;
struct FVertexInput;
struct FVertexBufferBinding;
struct FVertexInputState;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// See ResourceId.h
//----------------------------------------------------------------------------
namespace details {
template <u32 _Uid, bool _KeepName>
struct TNamedId;
template <u32 _Uid>
struct TResourceId;
template <typename T>
struct TResourceWrappedId;
} //!details
//----------------------------------------------------------------------------
// Resources
//----------------------------------------------------------------------------
using FUniformID                = details::TNamedId<   1, USE_PPE_RHIOPTIMIZEIDS >;
using FPushConstantID           = details::TNamedId<   2, USE_PPE_RHIOPTIMIZEIDS >;
using FDescriptorSetID          = details::TNamedId<   3, USE_PPE_RHIOPTIMIZEIDS >;
using FSpecializationID         = details::TNamedId<   4, USE_PPE_RHIOPTIMIZEIDS >;
using FVertexID                 = details::TNamedId<   5, USE_PPE_RHIOPTIMIZEIDS >;
using FVertexBufferID           = details::TNamedId<   6, USE_PPE_RHIOPTIMIZEIDS >;
using FMemPoolID                = details::TNamedId<   7, USE_PPE_RHIOPTIMIZEIDS >;
using FRTShaderID               = details::TNamedId<   8, USE_PPE_RHIOPTIMIZEIDS >;
using FGeometryID               = details::TNamedId<   9, USE_PPE_RHIOPTIMIZEIDS >;
using FInstanceID               = details::TNamedId<  10, USE_PPE_RHIOPTIMIZEIDS >;

enum class ERenderTargetID : u32;
PPE_STRONGLYTYPED_NUMERIC_DEF(u32, FStagingBufferIndex);

//----------------------------------------------------------------------------
// Weak references
//----------------------------------------------------------------------------
using FRawBufferID              = details::TResourceId<  1 >;
using FRawImageID               = details::TResourceId<  2 >;
using FRawGPipelineID           = details::TResourceId<  3 >;
using FRawMPipelineID           = details::TResourceId<  4 >;
using FRawCPipelineID           = details::TResourceId<  5 >;
using FRawRTPipelineID          = details::TResourceId<  6 >;
using FRawSamplerID             = details::TResourceId<  7 >;
using FRawDescriptorSetLayoutID = details::TResourceId<  8 >;
using FRawPipelineResourcesID   = details::TResourceId<  9 >;
using FRawRTSceneID             = details::TResourceId< 10 >;
using FRawRTGeometryID          = details::TResourceId< 11 >;
using FRawRTShaderTableID       = details::TResourceId< 12 >;
using FRawSwapchainID           = details::TResourceId< 13 >;
using FRawLogicalPassID         = details::TResourceId< 14 >;

using FRawMemoryID              = details::TResourceId< 15 >;
using FRawPipelineLayoutID      = details::TResourceId< 16 >;
using FRawRenderPassID          = details::TResourceId< 17 >;
using FRawFramebufferID         = details::TResourceId< 18 >;

//----------------------------------------------------------------------------
// Strong references
//----------------------------------------------------------------------------
using FBufferID                 = details::TResourceWrappedId< FRawBufferID >;
using FImageID                  = details::TResourceWrappedId< FRawImageID >;
using FGPipelineID              = details::TResourceWrappedId< FRawGPipelineID >;
using FMPipelineID              = details::TResourceWrappedId< FRawMPipelineID >;
using FCPipelineID              = details::TResourceWrappedId< FRawCPipelineID >;
using FRTPipelineID             = details::TResourceWrappedId< FRawRTPipelineID >;
using FSamplerID                = details::TResourceWrappedId< FRawSamplerID >;
using FRTSceneID                = details::TResourceWrappedId< FRawRTSceneID >;
using FRTGeometryID             = details::TResourceWrappedId< FRawRTGeometryID >;
using FRTShaderTableID          = details::TResourceWrappedId< FRawRTShaderTableID >;
using FSwapchainID              = details::TResourceWrappedId< FRawSwapchainID >;
using FLogicalPassID            = details::TResourceWrappedId< FRawLogicalPassID >;

using FMemoryID                 = details::TResourceWrappedId< FRawMemoryID >;
using FPipelineLayoutID         = details::TResourceWrappedId< FRawPipelineLayoutID >;
using FRenderPassID             = details::TResourceWrappedId< FRawRenderPassID >;
using FFramebufferID            = details::TResourceWrappedId< FRawFramebufferID >;

using FPipelineResourcesID      = details::TResourceWrappedId< FRawPipelineResourcesID >;
using FDescriptorSetLayoutID    = details::TResourceWrappedId< FRawDescriptorSetLayoutID >;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
