#pragma once

#include "RHI_fwd.h"
#include "SamplerEnums.h"

#include "RHI/Config.h"
#include "RHI/DrawTask.h"
#include "RHI/FrameDebug.h"
#include "RHI/ResourceEnums.h"

#include "Color/Color.h"
#include "Container/Stack.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Meta/Optional.h"
#include "Misc/Function.h"

#include <variant>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
struct FComputeShaderDebugMode {
    EShaderDebugMode Mode{ Default };
    uint3 GlobalId{ ~0u };
};
#endif
//----------------------------------------------------------------------------
namespace details {
template <typename _Self>
struct TFrameTaskDesc {
    using FTasks = TFixedSizeStack<PFrameTask, MaxTaskDependencies>;

    using self_type = _Self;

    FTasks Dependencies;

    TFrameTaskDesc() = default;

    self_type& DependsOn(const PFrameTask& task) {
        if (task) Add_AssertUnique(Dependencies, task);
        return static_cast<self_type&>(*this);
    }
    template <typename _Arg0, typename... _Args>
    self_type& DependsOn(_Arg0 task0, _Args&&... tasks) {
        DependsOn(std::forward<_Arg0>(task0));
        DependsOn(std::forward<_Args>(tasks)...);
        return static_cast<self_type&>(*this);
    }

#if USE_PPE_RHITASKNAME
    FTaskName TaskName;
    FLinearColor DebugColor;

    TFrameTaskDesc(FStringView name, const FLinearColor& color) NOEXCEPT
    :   TaskName(name)
    ,   DebugColor(color) {
    }

    self_type& SetName(const FStringView& value) { TaskName.Assign(value); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FRgba8u& value) { DebugColor = FColor(value).ToLinear(); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FColor& value) { DebugColor = value.ToLinear(); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FLinearColor& value) { DebugColor = value; return static_cast<self_type&>(*this); }
#else
    self_type& SetName(const FStringView&) { return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FRgba8u&) { return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FColor&) { return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FLinearColor&) { return static_cast<self_type&>(*this); }
#endif
};
} //!details
//----------------------------------------------------------------------------
// FSubmitRenderPass
//----------------------------------------------------------------------------
struct FSubmitRenderPass : details::TFrameTaskDesc<FSubmitRenderPass> {
    using FImages = FCustomDraw::FImages;
    using FBuffers = FCustomDraw::FBuffers;

    FLogicalPassID RenderPassId;
    FImages Images;
    FBuffers Buffers;

    PPE_RHI_API explicit FSubmitRenderPass(FLogicalPassID renderPassId) NOEXCEPT;
    PPE_RHI_API ~FSubmitRenderPass();

    PPE_RHI_API FSubmitRenderPass& AddImage(FRawImageID id, EResourceState state = EResourceState::ShaderSample);
    PPE_RHI_API FSubmitRenderPass& AddBuffer(FRawBufferID id, EResourceState state = EResourceState::UniformRead);
};
//----------------------------------------------------------------------------
// FDispatchCompute
//----------------------------------------------------------------------------
struct FDispatchCompute : details::TFrameTaskDesc<FDispatchCompute> {

    struct FComputeCommand {
        uint3 BaseGroup{ 0 };
        uint3 GroupCount{ 0 };
    };
    using FComputeCommands = TFixedSizeStack<FComputeCommand, MaxComputeCommands>;

    FRawCPipelineID Pipeline;
    FPipelineResourceSet Resources;
    FComputeCommands Commands;
    FPushConstantDatas PushConstants;
    Meta::TOptional<uint3> LocalGroupSize;

#if USE_PPE_RHIDEBUG
    using FDebugMode = FComputeShaderDebugMode;
    FDebugMode DebugMode;
#endif

    PPE_RHI_API FDispatchCompute() NOEXCEPT;
    PPE_RHI_API ~FDispatchCompute();

    FDispatchCompute& SetPipeline(FRawCPipelineID value) { Assert(value); Pipeline = value; return (*this); }

    FDispatchCompute& SetLocalSize(const uint2& xy) { return SetLocalSize(uint3(xy, 1)); }
    FDispatchCompute& SetLocalSize(u32 x, u32 y, u32 z) { return SetLocalSize(uint3(x, y, z)); }
    FDispatchCompute& SetLocalSize(const uint3& count) { LocalGroupSize = count; return (*this); }

    PPE_RHI_API FDispatchCompute& AddPushConstant(const FPushConstantID& id, const void* p, size_t size);
    FDispatchCompute& AddPushConstant(const FPushConstantID& id, const FRawMemoryConst& raw) { return AddPushConstant(id, raw.data(), raw.SizeInBytes()); }

    PPE_RHI_API FDispatchCompute& AddResources(const FDescriptorSetID& id, const PCPipelineResources& res);

    FDispatchCompute& Dispatch(const uint2& count) { return Dispatch(uint3(0), uint3(count, 1)); }
    FDispatchCompute& Dispatch(const uint3& count) { return Dispatch(uint3(0), count); }
    FDispatchCompute& Dispatch(const uint2& off, const uint2& count) { return Dispatch(uint3(off, 0), uint3(count, 1)); }

    PPE_RHI_API FDispatchCompute& Dispatch(const uint3& off, const uint3& count);

#if USE_PPE_RHIDEBUG
    FDispatchCompute& EnableShaderDebugTrace() { return EnableShaderDebugTrace(uint3(~0u)); }
    FDispatchCompute& EnableShaderDebugTrace(const uint3& globalId) {
        DebugMode.Mode = EShaderDebugMode::Trace;
        DebugMode.GlobalId = globalId;
        return (*this);
    }
#endif

#if USE_PPE_RHIDEBUG
    FDispatchCompute& EnableShaderProfiling() { return EnableShaderProfiling(uint3(~0u)); }
    FDispatchCompute& EnableShaderProfiling(const uint3& globalId) {
        DebugMode.Mode = EShaderDebugMode::Profiling;
        DebugMode.GlobalId = globalId;
        return (*this);
    }
#endif
};
//----------------------------------------------------------------------------
// FDispatchComputeIndirect
//----------------------------------------------------------------------------
struct FDispatchComputeIndirect final : details::TFrameTaskDesc<FDispatchComputeIndirect> {

    struct FComputeCommand {
        size_t IndirectBufferOffset{ 0 };
    };
    using FComputeCommands = TFixedSizeStack<FComputeCommand, MaxComputeCommands>;

    struct FIndirectCommand {
        uint3 GroupCount;
    };
    STATIC_ASSERT(sizeof(FIndirectCommand) == 12);

    FRawCPipelineID Pipeline;
    FPipelineResourceSet Resources;
    FComputeCommands Commands;

    FRawBufferID IndirectBuffer;
    FPushConstantDatas PushConstants;
    Meta::TOptional<uint3> LocalGroupSize;

#if USE_PPE_RHIDEBUG
    using FDebugMode = FComputeShaderDebugMode;
    FDebugMode DebugMode;
#endif

    PPE_RHI_API FDispatchComputeIndirect() NOEXCEPT;
    PPE_RHI_API ~FDispatchComputeIndirect();

    FDispatchComputeIndirect& SetPipeline(FRawCPipelineID value) { Assert(value); Pipeline = value; return (*this); }

    FDispatchComputeIndirect& SetLocalSize(const uint2& xy) { return SetLocalSize(uint3(xy, 1)); }
    FDispatchComputeIndirect& SetLocalSize(u32 x, u32 y, u32 z) { return SetLocalSize(uint3(x, y, z)); }
    FDispatchComputeIndirect& SetLocalSize(const uint3& count) { LocalGroupSize = count; return (*this); }

    FDispatchComputeIndirect& SetIndirectBuffer(FRawBufferID buffer) { Assert(buffer); IndirectBuffer = buffer; return (*this); }

    PPE_RHI_API FDispatchComputeIndirect& Dispatch(size_t offset);

    PPE_RHI_API FDispatchComputeIndirect& AddPushConstant(const FPushConstantID& id, const void* p, size_t size);
    FDispatchComputeIndirect& AddPushConstant(const FPushConstantID& id, const FRawMemoryConst& raw) { return AddPushConstant(id, raw.data(), raw.SizeInBytes()); }

    PPE_RHI_API FDispatchComputeIndirect& AddResources(const FDescriptorSetID& id, const PCPipelineResources& res);

#if USE_PPE_RHIDEBUG
    FDispatchComputeIndirect& EnableShaderDebugTrace() { return EnableShaderDebugTrace(uint3(~0u)); }
    FDispatchComputeIndirect& EnableShaderDebugTrace(const uint3& globalId) {
        DebugMode.Mode = EShaderDebugMode::Trace;
        DebugMode.GlobalId = globalId;
        return (*this);
    }
#endif

#if USE_PPE_RHIDEBUG
    FDispatchComputeIndirect& EnableShaderProfiling() { return EnableShaderProfiling(uint3(~0u)); }
    FDispatchComputeIndirect& EnableShaderProfiling(const uint3& globalId) {
        DebugMode.Mode = EShaderDebugMode::Profiling;
        DebugMode.GlobalId = globalId;
        return (*this);
    }
#endif
};
//----------------------------------------------------------------------------
// FCopyBuffer
//----------------------------------------------------------------------------
struct FCopyBuffer final : details::TFrameTaskDesc<FCopyBuffer> {

    struct FRegion {
        size_t SrcOffset;
        size_t DstOffset;
        size_t Size;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawBufferID SrcBuffer;
    FRawBufferID DstBuffer;
    FRegions Regions;

    PPE_RHI_API FCopyBuffer() NOEXCEPT;
    PPE_RHI_API ~FCopyBuffer();

    FCopyBuffer& From(FRawBufferID buffer) { Assert(buffer); SrcBuffer = buffer; return (*this); }
    FCopyBuffer& To(FRawBufferID buffer) { Assert(buffer); DstBuffer = buffer; return (*this); }

    PPE_RHI_API FCopyBuffer& AddRegion(size_t srcOffset, size_t dstOffset, size_t size);
};
//----------------------------------------------------------------------------
// FCopyImage
//----------------------------------------------------------------------------
struct FCopyImage final : details::TFrameTaskDesc<FCopyImage> {

    struct FRegion {
        FImageSubresourceRange SrcSubresource;
        int3 SrcOffset;
        FImageSubresourceRange DstSubresource;
        int3 DstOffset;
        uint3 Size;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawImageID SrcImage;
    FRawImageID DstImage;
    FRegions Regions;

    PPE_RHI_API FCopyImage() NOEXCEPT;
    PPE_RHI_API ~FCopyImage();

    FCopyImage& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FCopyImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    PPE_RHI_API FCopyImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int2& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int2& dstOffset,
        const uint2& size );

    PPE_RHI_API FCopyImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int3& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int3& dstOffset,
        const uint3& size );
};
//----------------------------------------------------------------------------
// FCopyBufferToImage
//----------------------------------------------------------------------------
struct FCopyBufferToImage final : details::TFrameTaskDesc<FCopyBufferToImage> {

    struct FRegion {
        size_t BufferOffset;
        u32 BufferRowLength;
        u32 BufferImageHeight;
        FImageSubresourceRange ImageLayers;
        int3 ImageOffset;
        uint3 ImageSize;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawBufferID SrcBuffer;
    FRawImageID DstImage;
    FRegions Regions;

    PPE_RHI_API FCopyBufferToImage() NOEXCEPT;
    PPE_RHI_API ~FCopyBufferToImage();

    FCopyBufferToImage& From(FRawBufferID buffer) { Assert(buffer); SrcBuffer = buffer; return (*this); }
    FCopyBufferToImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    PPE_RHI_API FCopyBufferToImage& AddRegion(
        size_t srcBufferOffset, u32 srcBufferRowLength, u32 srcBufferImageHeight,
        const FImageSubresourceRange& dstImageLayers, const int2& dstImageOffset, const uint2& dstImageSize );
    PPE_RHI_API FCopyBufferToImage& AddRegion(
        size_t SrcBufferOffset, u32 srcBufferRowLength, u32 srcBufferImageHeight,
        const FImageSubresourceRange& dstImageLayers, const int3& dstImageOffset, const uint3& dstImageSize );
};

//----------------------------------------------------------------------------
// FCopyImageToBuffer
//----------------------------------------------------------------------------
struct FCopyImageToBuffer final : details::TFrameTaskDesc<FCopyImageToBuffer> {

    using FRegion = FCopyBufferToImage::FRegion;
    using FRegions = FCopyBufferToImage::FRegions;

    FRawImageID SrcImage;
    FRawBufferID DstBuffer;
    FRegions Regions;

    PPE_RHI_API FCopyImageToBuffer() NOEXCEPT;
    PPE_RHI_API ~FCopyImageToBuffer();

    FCopyImageToBuffer& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FCopyImageToBuffer& To(FRawBufferID buffer) { Assert(buffer); DstBuffer = buffer; return (*this); }

    PPE_RHI_API FCopyImageToBuffer& AddRegion(
        const FImageSubresourceRange& srcImageLayers, const int2& srcImageOffset, const uint2& srcImageSize,
        size_t dstBufferOffset, u32 dstBufferRowLength, u32 dstBufferImageHeight );
    PPE_RHI_API FCopyImageToBuffer& AddRegion(
        const FImageSubresourceRange& srcImageLayers, const int3& srcImageOffset, const uint3& srcImageSize,
        size_t dstBufferOffset, u32 dstBufferRowLength, u32 dstBufferImageHeight );
};
//----------------------------------------------------------------------------
// FBlitImage
//----------------------------------------------------------------------------
struct FBlitImage final : details::TFrameTaskDesc<FBlitImage> {

    struct FRegion {
        FImageSubresourceRange SrcSubresource;
        int3 SrcOffset0;
        int3 SrcOffset1;
        FImageSubresourceRange DstSubresource;
        int3 DstOffset0;
        int3 DstOffset1;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawImageID SrcImage;
    FRawImageID DstImage;
    ETextureFilter Filter{ ETextureFilter::Nearest };
    FRegions Regions;

    PPE_RHI_API FBlitImage() NOEXCEPT;
    PPE_RHI_API ~FBlitImage();

    FBlitImage& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FBlitImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FBlitImage& SetFilter(ETextureFilter filter) { Filter = filter; return (*this); }

    PPE_RHI_API FBlitImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int2& srcOffsetStart, const int2 srcOffsetEnd,
        const FImageSubresourceRange& dstSubresource, const int2& dstOffsetStart, const int2 dstOffsetEnd );
    PPE_RHI_API FBlitImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int3& srcOffsetStart, const int3& srcOffsetEnd,
        const FImageSubresourceRange& dstSubresource, const int3& dstOffsetStart, const int3& dstOffsetEnd );
};
//----------------------------------------------------------------------------
// FGenerateMipmaps
//----------------------------------------------------------------------------
struct FGenerateMipmaps : details::TFrameTaskDesc<FGenerateMipmaps> {
    FRawImageID Image;
    FMipmapLevel BaseLevel; // 0 - src level, mipmap generation will start from baseMipLevel + 1
    u32 LevelCount{ 0 };
    FImageLayer BaseLayer; // specify array layers that will be used
    u32 LayerCount{ 0 };

    PPE_RHI_API FGenerateMipmaps() NOEXCEPT;
    PPE_RHI_API ~FGenerateMipmaps();

    FGenerateMipmaps& SetImage(FRawImageID image) { Assert(image); Image = image; return (*this); }
    FGenerateMipmaps& SetMipmaps(u32 base, u32 count) { BaseLevel = FMipmapLevel{ base }; LevelCount = count; return (*this); }
    FGenerateMipmaps& SetArrayLayers(u32 base, u32 count) { BaseLayer = FImageLayer{ base }; LayerCount = count; return (*this); }
};
//----------------------------------------------------------------------------
// FResolveImage
//----------------------------------------------------------------------------
struct FResolveImage final : details::TFrameTaskDesc<FResolveImage> {

    struct FRegion {
        FImageSubresourceRange SrcSubresource;
        int3 SrcOffset;
        FImageSubresourceRange DstSubresource;
        int3 DstOffset;
        uint3 Extent;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxResolveRegions>;

    FRawImageID SrcImage;
    FRawImageID DstImage;
    FRegions Regions;

    PPE_RHI_API FResolveImage() NOEXCEPT;
    PPE_RHI_API ~FResolveImage();

    FResolveImage& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FResolveImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    PPE_RHI_API FResolveImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int2& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int2& dstOffset,
        const uint2& extent );
    PPE_RHI_API FResolveImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int3& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int3& dstOffset,
        const uint3& extent );
};
//----------------------------------------------------------------------------
// FFillBuffer
//----------------------------------------------------------------------------
struct FFillBuffer final : details::TFrameTaskDesc<FFillBuffer> {
    FRawBufferID DstBuffer;
    size_t DstOffset{ 0 };
    size_t Size{ 0 };
    u32 Pattern{ 0 };

    PPE_RHI_API FFillBuffer() NOEXCEPT;
    PPE_RHI_API ~FFillBuffer();

    FFillBuffer& SetBuffer(FRawBufferID buffer) { return SetBuffer(buffer, 0, UMax); }
    PPE_RHI_API FFillBuffer& SetBuffer(FRawBufferID buffer, size_t offset, size_t size);

    FFillBuffer& SetPattern(u32 pattern) { Pattern = pattern; return (*this); }
};
//----------------------------------------------------------------------------
// FClearColorImage
//----------------------------------------------------------------------------
struct FClearColorImage final : details::TFrameTaskDesc<FClearColorImage> {

    struct FRange {
        EImageAspect AspectMask;
        FMipmapLevel BaseMipLevel;
        u32 LevelCount;
        FImageLayer BaseLayer;
        u32 LayerCount;
    };
    using FRanges = TFixedSizeStack<FRange, MaxClearRanges>;

    using FClearColor = std::variant<
        FRgba32i,
        FRgba32u,
        FLinearColor >;

    FRawImageID DstImage;
    FClearColor ClearColor;
    FRanges Ranges;

    PPE_RHI_API FClearColorImage() NOEXCEPT;
    PPE_RHI_API ~FClearColorImage();

    FClearColorImage& SetImage(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    PPE_RHI_API FClearColorImage& Clear(const FRgba32u& color);
    PPE_RHI_API FClearColorImage& Clear(const FRgba32i& color);
    PPE_RHI_API FClearColorImage& Clear(const FLinearColor& color);

    PPE_RHI_API FClearColorImage& AddRange(FMipmapLevel baseMipLevel, u32 levelCount, FImageLayer baseLayer, u32 layerCount);
};
//----------------------------------------------------------------------------
// FClearDepthStencilImage
//----------------------------------------------------------------------------
struct FClearDepthStencilImage final : details::TFrameTaskDesc<FClearDepthStencilImage> {
    using FRange = FClearColorImage::FRange;
    using FRanges = FClearColorImage::FRanges;

    FRawImageID DstImage;
    FDepthValue ClearDepth{ 0.0f };
    FStencilValue ClearStencil{ 0 };
    FRanges Ranges;

    PPE_RHI_API FClearDepthStencilImage() NOEXCEPT;
    PPE_RHI_API ~FClearDepthStencilImage();

    FClearDepthStencilImage& SetImage(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FClearDepthStencilImage& Clear(FDepthValue depth, FStencilValue stencil = FStencilValue(0)) { ClearDepth = depth; ClearStencil = stencil; return (*this); }

    PPE_RHI_API FClearDepthStencilImage& AddRange(FMipmapLevel baseMipLevel, u32 levelCount, FImageLayer baseLayer, u32 layerCount);
};
//----------------------------------------------------------------------------
// FUpdateBuffer
//----------------------------------------------------------------------------
struct FUpdateBuffer final : details::TFrameTaskDesc<FUpdateBuffer> {

    struct FRegion {
        size_t Offset{ 0 };
        FRawMemoryConst Data;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawBufferID DstBuffer;
    FRegions Regions;

    PPE_RHI_API FUpdateBuffer() NOEXCEPT;
    PPE_RHI_API ~FUpdateBuffer();

    PPE_RHI_API FUpdateBuffer(FRawBufferID buffer, size_t offset, const FRawMemoryConst& data) NOEXCEPT;

    PPE_RHI_API FUpdateBuffer& SetBuffer(FRawBufferID buffer);

    template <typename T>
    FUpdateBuffer& AddData(const TMemoryView<const T>& data, size_t offset = 0) { return AddData(data.template Cast<const u8>(), offset); }
    FUpdateBuffer& AddData(const void* p, const size_t size, size_t offset = 0) { return AddData(FRawMemoryConst(static_cast<const u8*>(p), size), offset); }

    PPE_RHI_API FUpdateBuffer& AddData(const FRawMemoryConst& data, size_t offset = 0);
};
//----------------------------------------------------------------------------
// FReadBuffer
//----------------------------------------------------------------------------
struct FReadBuffer final : details::TFrameTaskDesc<FReadBuffer> {

    using FCallback = TFunction<void(const FBufferView&)>;

    FRawBufferID SrcBuffer;
    size_t SrcOffset{ 0 };
    size_t SrcSize{ 0 };
    FCallback Callback;

    PPE_RHI_API FReadBuffer() NOEXCEPT;
    PPE_RHI_API ~FReadBuffer();

    PPE_RHI_API FReadBuffer& SetBuffer(FRawBufferID buffer, size_t offset, size_t size);

    FReadBuffer& SetCallback(const FCallback& callback) { return SetCallback(FCallback(callback)); }
    PPE_RHI_API FReadBuffer& SetCallback(FCallback&& rcallback);
};
//----------------------------------------------------------------------------
// FUpdateImage
//----------------------------------------------------------------------------
struct FUpdateImage final : details::TFrameTaskDesc<FUpdateImage> {

    FRawImageID DstImage;
    int3 ImageOffset{ 0 };
    uint3 ImageSize{ 0 };
    FImageLayer ArrayLayer;
    FMipmapLevel MipmapLevel;
    size_t DataRowPitch{ 0 };
    size_t DataSlicePitch{ 0 };
    EImageAspect AspectMask{ EImageAspect::Color };
    FRawMemoryConst Data;

    PPE_RHI_API FUpdateImage() NOEXCEPT;
    PPE_RHI_API ~FUpdateImage();

    FUpdateImage& SetImage(FRawImageID image, const int2& offset, FMipmapLevel mipmap = Default) { return SetImage(image, int3(offset, 0), mipmap); }
    PPE_RHI_API FUpdateImage& SetImage(FRawImageID image, const int3& offset = Default, FMipmapLevel mipmap = Default);
    PPE_RHI_API FUpdateImage& SetImage(FRawImageID image, const int2& offset, FImageLayer layer, FMipmapLevel mipmap);

    FUpdateImage& SetData(FRawMemoryConst data, const uint2& dimension, size_t rowPitch = 0) { return SetData(data, uint3(dimension, 0), rowPitch); }
    PPE_RHI_API FUpdateImage& SetData(FRawMemoryConst data, const uint3& dimension, size_t rowPitch = 0, size_t slicePitch = 0);

    template <typename T>
    FUpdateImage& SetData(TMemoryView<const T> data, const uint2& dimension, size_t rowPitch = 0) { return SetData(data.template Cast<const u8>(), uint3(dimension, 0), rowPitch); }
    template <typename T>
    FUpdateImage& SetData(TMemoryView<const T> data, const uint3& dimension, size_t rowPitch = 0, size_t slicePitch = 0) { return SetData(data.template Cast<const u8>(), dimension, rowPitch, slicePitch); }
};
//----------------------------------------------------------------------------
// FReadImage
//----------------------------------------------------------------------------
struct FReadImage final : details::TFrameTaskDesc<FReadImage> {

    using FCallback = TFunction<void(const FImageView&)>;

    FRawImageID SrcImage;
    int3 ImageOffset{ 0 };
    uint3 ImageSize{ 0 };
    FImageLayer ArrayLayer;
    FMipmapLevel MipmapLevel;
    EImageAspect AspectMask = EImageAspect::Color;
    FCallback Callback;

    PPE_RHI_API FReadImage() NOEXCEPT;
    PPE_RHI_API ~FReadImage();

    FReadImage& SetImage(FRawImageID image, const int2& offset, const uint2& size, FMipmapLevel mipmap = Default) { return SetImage(image, int3(offset, 0), uint3(size, 0), mipmap); }
    PPE_RHI_API FReadImage& SetImage(FRawImageID image, const int3& offset, const uint3& size, FMipmapLevel mipmap = Default);
    PPE_RHI_API FReadImage& SetImage(FRawImageID image, const int2& offset, const uint2& size, FImageLayer layer, FMipmapLevel mipmap = Default);

    FReadImage& SetCallback(const FCallback& callback) { return SetCallback(FCallback(callback)); }
    PPE_RHI_API FReadImage& SetCallback(FCallback&& rcallback);
};
//----------------------------------------------------------------------------
// FPresent
//----------------------------------------------------------------------------
struct FPresent final : details::TFrameTaskDesc<FPresent> {
    FRawSwapchainID Swapchain;
    FRawImageID SrcImage;
    FImageLayer Layer;
    FMipmapLevel Mipmap;

    PPE_RHI_API FPresent() NOEXCEPT;
    PPE_RHI_API ~FPresent();

    PPE_RHI_API explicit FPresent(FRawSwapchainID swapchain, FRawImageID image) NOEXCEPT;

    PPE_RHI_API FPresent& SetSwapchain(FRawSwapchainID swapchain);
    PPE_RHI_API FPresent& SetImage(FRawImageID image, FImageLayer layer = Default, FMipmapLevel mipmap = Default);
};
//----------------------------------------------------------------------------
// FCustomTask
//----------------------------------------------------------------------------
struct FCustomTask final : details::TFrameTaskDesc<FCustomTask> {

    using FExternalContext = void*;
    using FCallback = TFunction<void(ICommandBuffer* cmd, FExternalContext)>;
    using FImages = TFixedSizeStack<TPair<FRawImageID, EResourceState>, 8>;
    using FBuffers = TFixedSizeStack<TPair<FRawBufferID, EResourceState>, 8>;

    FCallback Callback;
    FImages Images;
    FBuffers Buffers;

    PPE_RHI_API FCustomTask() NOEXCEPT;
    PPE_RHI_API ~FCustomTask();

    PPE_RHI_API explicit FCustomTask(FCallback&& rcallback) NOEXCEPT;

    PPE_RHI_API FCustomTask& AddImage(FRawImageID image, EResourceState state);
    PPE_RHI_API FCustomTask& AddBuffer(FRawBufferID image, EResourceState state);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
