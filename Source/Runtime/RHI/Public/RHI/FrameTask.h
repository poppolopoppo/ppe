#pragma once

#include "RHI_fwd.h"
#include "SamplerEnums.h"

#include "RHI/Config.h"
#include "RHI/DrawTask.h"
#include "RHI/FrameDebug.h"
#include "RHI/ResourceEnums.h"

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

    self_type& SetName(FStringView value) { TaskName.Assign(value); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FRgba8u& value) { DebugColor = FColor(value).ToLinear(); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FColor& value) { DebugColor = value.ToLinear(); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(const FLinearColor& value) { DebugColor = value; return static_cast<self_type&>(*this); }
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

    explicit FSubmitRenderPass(FLogicalPassID&& renderPassId) NOEXCEPT :
#if USE_PPE_RHIDEBUG
        TFrameTaskDesc<FSubmitRenderPass>("SubmitRenderPass", FDebugColorScheme::Get().RenderPass),
#endif
        RenderPassId(std::move(renderPassId)) {
        Assert(renderPassId);
    }

    FSubmitRenderPass& AddImage(FRawImageID id, EResourceState state = EResourceState::ShaderSample) {
        Assert(id);
        Images.Push(id, state);
        return (*this);
    }

    FSubmitRenderPass& AddBuffer(FRawBufferID id, EResourceState state = EResourceState::UniformRead) {
        Assert(id);
        Buffers.Push(id, state);
        return (*this);
    }
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

#if USE_PPE_RHITASKNAME
    FDispatchCompute() NOEXCEPT : TFrameTaskDesc<FDispatchCompute>("DispatchCompute", FDebugColorScheme::Get().Compute) {}
#endif

    FDispatchCompute& SetPipeline(FRawCPipelineID value) { Assert(value); Pipeline = value; return (*this); }

    FDispatchCompute& SetLocalSize(const uint2& xy) { return SetLocalSize(uint3(xy, 1)); }
    FDispatchCompute& SetLocalSize(u32 x, u32 y, u32 z) { return SetLocalSize(uint3(x, y, z)); }
    FDispatchCompute& SetLocalSize(const uint3& count) { LocalGroupSize = count; return (*this); }

    template <typename T>
    FDispatchCompute& AddPushConstant(const FPushConstantID& id, const T& value) { return AddPushConstant(id, &value, sizeof(value)); }
    FDispatchCompute& AddPushConstant(const FPushConstantID& id, const void* p, size_t size) { PushConstants.Push(id, p, size); return (*this); }

    FDispatchCompute& AddResources(const FDescriptorSetID& id, const FPipelineResources* res) {
        Assert(id);
        Assert(res);
        Resources.Add(id) = res;
        return (*this);
    }

    FDispatchCompute& Dispatch(const uint2& count) { return Dispatch(uint3(0), uint3(count, 1)); }
    FDispatchCompute& Dispatch(const uint3& count) { return Dispatch(uint3(0), count); }
    FDispatchCompute& Dispatch(const uint2& off, const uint2& count) { return Dispatch(uint3(off, 0), uint3(count, 1)); }
    FDispatchCompute& Dispatch(const uint3& off, const uint3& count) { Emplace_Back(Commands, off, count); return (*this); }

#if USE_PPE_RHIDEBUG
    FDispatchCompute& EnableShaderDebugTrace() { return EnableShaderDebugTrace(uint3(~0u)); }
    FDispatchCompute& EnableShaderDebugTrace(const uint3& globalId);
#endif

#if USE_PPE_RHIDEBUG
    FDispatchCompute& EnableShaderProfiling() { return EnableShaderProfiling(uint3(~0u)); }
    FDispatchCompute& EnableShaderProfiling(const uint3& globalId);
#endif
};
//----------------------------------------------------------------------------
// FDispatchComputeIndirect
//----------------------------------------------------------------------------
struct FDispatchComputeIndirect final : details::TFrameTaskDesc<FDispatchComputeIndirect> {

    struct FComputeCommand {
        u32 IndirectBufferOffset{ 0 };
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

#if USE_PPE_RHITASKNAME
    FDispatchComputeIndirect() NOEXCEPT : TFrameTaskDesc<FDispatchComputeIndirect>("DispatchComputeIndirect", FDebugColorScheme::Get().Compute) {}
#endif

    FDispatchComputeIndirect& SetPipeline(FRawCPipelineID value) { Assert(value); Pipeline = value; return (*this); }

    FDispatchComputeIndirect& SetLocalSize(const uint2& xy) { return SetLocalSize(uint3(xy, 1)); }
    FDispatchComputeIndirect& SetLocalSize(u32 x, u32 y, u32 z) { return SetLocalSize(uint3(x, y, z)); }
    FDispatchComputeIndirect& SetLocalSize(const uint3& count) { LocalGroupSize = count; return (*this); }

    FDispatchComputeIndirect& SetIndirectBuffer(FRawBufferID buffer) { Assert(buffer); IndirectBuffer = buffer; return (*this); }

    FDispatchComputeIndirect& Dispatch(u32 offset) { Emplace_Back(Commands, offset); return (*this); }

    template <typename T>
    FDispatchComputeIndirect& AddPushConstant(const FPushConstantID& id, const T& value) { return AddPushConstant(id, &value, sizeof(value)); }
    FDispatchComputeIndirect& AddPushConstant(const FPushConstantID& id, const void* p, size_t size) { PushConstants.Push(id, p, size); return (*this); }

    FDispatchComputeIndirect& AddResources(const FDescriptorSetID& id, const FPipelineResources* res) {
        Assert(id);
        Assert(res);
        Resources.Add(id) = res;
        return (*this);
    }

#if USE_PPE_RHIDEBUG
    FDispatchComputeIndirect& EnableShaderDebugTrace() { return EnableShaderDebugTrace(uint3(~0u)); }
    FDispatchComputeIndirect& EnableShaderDebugTrace(const uint3& globalId);
#endif

#if USE_PPE_RHIDEBUG
    FDispatchComputeIndirect& EnableShaderProfiling() { return EnableShaderProfiling(uint3(~0u)); }
    FDispatchComputeIndirect& EnableShaderProfiling(const uint3& globalId);
#endif
};
//----------------------------------------------------------------------------
// FCopyBuffer
//----------------------------------------------------------------------------
struct FCopyBuffer final : details::TFrameTaskDesc<FCopyBuffer> {

    struct FRegion {
        u32 SrcOffset;
        u32 DstOffset;
        u32 Size;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawBufferID SrcBuffer;
    FRawBufferID DstBuffer;
    FRegions Regions;

#if USE_PPE_RHITASKNAME
    FCopyBuffer() NOEXCEPT : TFrameTaskDesc<FCopyBuffer>("CopyBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FCopyBuffer& From(FRawBufferID buffer) { Assert(buffer); SrcBuffer = buffer; return (*this); }
    FCopyBuffer& To(FRawBufferID buffer) { Assert(buffer); DstBuffer = buffer; return (*this); }

    FCopyBuffer& AddRegion(u32 srcOffset, u32 dstOffset, u32 size) {
        Assert(size);
        Emplace_Back(Regions, srcOffset, dstOffset, size);
        return (*this);
    }
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

#if USE_PPE_RHITASKNAME
    FCopyImage() NOEXCEPT : TFrameTaskDesc<FCopyImage>("CopyImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FCopyImage& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FCopyImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FCopyImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int2& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int2& dstOffset,
        const uint2& size ) {
        return AddRegion(srcSubresource, int3(srcOffset, 0), dstSubresource, int3(dstOffset, 0), uint3(size, 1));
    }

    FCopyImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int3& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int3& dstOffset,
        const uint3& size ) {
        Assert(All(bool3(size > uint3(0))));
        Emplace_Back(Regions, srcSubresource, srcOffset, dstSubresource, dstOffset, size);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FCopyBufferToImage
//----------------------------------------------------------------------------
struct FCopyBufferToImage final : details::TFrameTaskDesc<FCopyBufferToImage> {

    struct FRegion {
        u32 BufferOffset;
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

#if USE_PPE_RHITASKNAME
    FCopyBufferToImage() NOEXCEPT : TFrameTaskDesc<FCopyBufferToImage>("CopyBufferToImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FCopyBufferToImage& From(FRawBufferID buffer) { Assert(buffer); SrcBuffer = buffer; return (*this); }
    FCopyBufferToImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FCopyBufferToImage& AddRegion(
        u32 srcBufferOffset, u32 srcBufferRowLength, u32 srcBufferImageHeight,
        const FImageSubresourceRange& dstImageLayers, const int2& dstImageOffset, const uint2& dstImageSize ) {
        return AddRegion(srcBufferOffset, srcBufferRowLength, srcBufferImageHeight, dstImageLayers, int3(dstImageOffset, 0), uint3(dstImageSize, 1));
    }

    FCopyBufferToImage& AddRegion(
        u32 SrcBufferOffset, u32 srcBufferRowLength, u32 srcBufferImageHeight,
        const FImageSubresourceRange& dstImageLayers, const int3& dstImageOffset, const uint3& dstImageSize) {
        Assert(All(bool3(dstImageSize > uint3(0))));
        Emplace_Back(Regions, SrcBufferOffset, srcBufferRowLength, srcBufferImageHeight, dstImageLayers, dstImageOffset, dstImageSize);
        return (*this);
    }
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

#if USE_PPE_RHITASKNAME
    FCopyImageToBuffer() NOEXCEPT : TFrameTaskDesc<FCopyImageToBuffer>("CopyImageToBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FCopyImageToBuffer& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FCopyImageToBuffer& To(FRawBufferID buffer) { Assert(buffer); DstBuffer = buffer; return (*this); }

    FCopyImageToBuffer& AddRegion(
        const FImageSubresourceRange& srcImageLayers, const int2& srcImageOffset, const uint2& srcImageSize,
        u32 dstBufferOffset, u32 dstBufferRowLength, u32 dstBufferImageHeight ) {
        return AddRegion(srcImageLayers, int3(srcImageOffset, 0), uint3(srcImageSize, 1), dstBufferOffset, dstBufferRowLength, dstBufferImageHeight);
    }

    FCopyImageToBuffer& AddRegion(
        const FImageSubresourceRange& srcImageLayers, const int3& srcImageOffset, const uint3& srcImageSize,
        u32 dstBufferOffset, u32 dstBufferRowLength, u32 dstBufferImageHeight ) {
        Assert(All(bool3(srcImageSize > uint3(0))));
        Emplace_Back(Regions, dstBufferOffset, dstBufferRowLength, dstBufferImageHeight, srcImageLayers, srcImageOffset, srcImageSize);
        return (*this);
    }
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

#if USE_PPE_RHITASKNAME
    FBlitImage() NOEXCEPT : TFrameTaskDesc<FBlitImage>("BlitImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FBlitImage& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FBlitImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FBlitImage& SetFilter(ETextureFilter filter) { Filter = filter; return (*this); }

    FBlitImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int2& srcOffsetStart, const int2 srcOffsetEnd,
        const FImageSubresourceRange& dstSubresource, const int2& dstOffsetStart, const int2 dstOffsetEnd ) {
        return AddRegion(
            srcSubresource, int3(srcOffsetStart, 0), int3(srcOffsetEnd, 1),
            dstSubresource, int3(dstOffsetStart, 0), int3(dstOffsetEnd, 1) );
    }

    FBlitImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int3& srcOffsetStart, const int3& srcOffsetEnd,
        const FImageSubresourceRange& dstSubresource, const int3& dstOffsetStart, const int3& dstOffsetEnd ) {
        Assert(All(bool3(srcOffsetStart < srcOffsetEnd)));
        Assert(All(bool3(dstOffsetStart < dstOffsetEnd)));
        Emplace_Back(Regions, srcSubresource, srcOffsetStart, srcOffsetEnd, dstSubresource, dstOffsetStart, dstOffsetEnd);
        return (*this);
    }
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

#if USE_PPE_RHITASKNAME
    FGenerateMipmaps() NOEXCEPT : TFrameTaskDesc<FGenerateMipmaps>("GenerateMipmaps", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

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

#if USE_PPE_RHITASKNAME
    FResolveImage() NOEXCEPT : TFrameTaskDesc<FResolveImage>("ResolveImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FResolveImage& From(FRawImageID image) { Assert(image); SrcImage = image; return (*this); }
    FResolveImage& To(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FResolveImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int2& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int2& dstOffset,
        const uint2& extent ) {
        return AddRegion(srcSubresource, int3(srcOffset, 0), dstSubresource, int3(dstOffset, 0), uint3(extent, 1));
    }

    FResolveImage& AddRegion(
        const FImageSubresourceRange& srcSubresource, const int3& srcOffset,
        const FImageSubresourceRange& dstSubresource, const int3& dstOffset,
        const uint3& extent ) {
        Assert(All(bool3(extent > uint3(0))));
        Emplace_Back(Regions, srcSubresource, srcOffset, dstSubresource, dstOffset, extent);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FFillBuffer
//----------------------------------------------------------------------------
struct FFillBuffer final : details::TFrameTaskDesc<FFillBuffer> {
    FRawBufferID DstBuffer;
    u32 DstOffset{ 0 };
    u32 Size{ 0 };
    u32 Pattern{ 0 };

#if USE_PPE_RHITASKNAME
    FFillBuffer() NOEXCEPT : TFrameTaskDesc<FFillBuffer>("FillBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FFillBuffer& SetBuffer(FRawBufferID buffer) { return SetBuffer(buffer, 0, UMax); }
    FFillBuffer& SetBuffer(FRawBufferID buffer, u32 offset, u32 size) {
        Assert(buffer);
        DstBuffer = buffer;
        DstOffset = offset;
        Size = size;
        return (*this);
    }

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
        FRgba8u,
        FRgba32i,
        FRgba32u,
        FRgba32f >;

    FRawImageID DstImage;
    FClearColor ClearColor;
    FRanges Ranges;

#if USE_PPE_RHITASKNAME
    FClearColorImage() NOEXCEPT : TFrameTaskDesc<FClearColorImage>("ClearColorImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FClearColorImage& SetImage(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FClearColorImage& Clear(const FRgba8u& color) { ClearColor = color; return (*this); }
    FClearColorImage& Clear(const FRgba32u& color) { ClearColor = color; return (*this); }
    FClearColorImage& Clear(const FRgba32i& color) { ClearColor = color; return (*this); }

    FClearColorImage& AddRange(FMipmapLevel baseMipLevel, u32 levelCount, FImageLayer baseLayer, u32 layerCount) {
        Assert(levelCount > 0);
        Assert(layerCount > 0);
        Emplace_Back(Ranges, EImageAspect::Color, baseMipLevel, levelCount, baseLayer, layerCount);
        return (*this);
    }
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

#if USE_PPE_RHITASKNAME
    FClearDepthStencilImage() NOEXCEPT : TFrameTaskDesc<FClearDepthStencilImage>("ClearDepthStencilImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FClearDepthStencilImage& SetImage(FRawImageID image) { Assert(image); DstImage = image; return (*this); }

    FClearDepthStencilImage& Clear(FDepthValue depth, FStencilValue stencil) { ClearDepth = depth; ClearStencil = stencil; return (*this); }

    FClearDepthStencilImage& AddRange(FMipmapLevel baseMipLevel, u32 levelCount, FImageLayer baseLayer, u32 layerCount) {
        Assert(levelCount > 0);
        Assert(layerCount > 0);
        Emplace_Back(Ranges, EImageAspect::Color, baseMipLevel, levelCount, baseLayer, layerCount);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FUpdateBuffer
//----------------------------------------------------------------------------
struct FUpdateBuffer final : details::TFrameTaskDesc<FUpdateBuffer> {

    struct FRegion {
        u32 Offset{ 0 };
        FRawMemoryConst Data;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawBufferID DstBuffer;
    FRegions Regions;

#if USE_PPE_RHITASKNAME
    FUpdateBuffer() NOEXCEPT : TFrameTaskDesc<FUpdateBuffer>("UpdateBuffer", FDebugColorScheme::Get().HostToDeviceTransfer) {}
#endif

    FUpdateBuffer& SetBuffer(FRawBufferID buffer) { Assert(buffer); DstBuffer = buffer; return (*this); }

    template <typename T>
    FUpdateBuffer& AddData(const TMemoryView<const T>& data, u32 offset = 0) { return AddData(data.template Cast<const u8>(), offset); }
    FUpdateBuffer& AddData(const u8* p, const size_t size, u32 offset = 0) { return AddData(FRawMemoryConst(p, size), offset); }
    FUpdateBuffer& AddData(const FRawMemoryConst& data, u32 offset = 0) {
        Assert(not data.empty());
        Emplace_Back(Regions, offset, data);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FReadBuffer
//----------------------------------------------------------------------------
struct FReadBuffer final : details::TFrameTaskDesc<FReadBuffer> {

    using FCallback = TFunction<void(const FBufferView&)>;

    FRawBufferID SrcBuffer;
    u32 SrcOffset{ 0 };
    u32 SrcSize{ 0 };
    FCallback Callback;

#if USE_PPE_RHITASKNAME
    FReadBuffer() NOEXCEPT : TFrameTaskDesc<FReadBuffer>("ReadBuffer", FDebugColorScheme::Get().DeviceToHostTransfer) {}
#endif

    FReadBuffer& SetBuffer(FRawBufferID buffer, u32 offset, u32 size) {
        Assert(buffer);
        SrcBuffer = buffer;
        SrcOffset = offset;
        SrcSize = size;
        return (*this);
    }

    FReadBuffer& SetCallback(const FCallback& callback) { return SetCallback(FCallback(callback)); }
    FReadBuffer& SetCallback(FCallback&& rcallback) {
        Assert(rcallback);
        Callback = std::move(rcallback);
        return (*this);
    }
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
    u32 DataRowPitch{ 0 };
    u32 DataSlicePitch{ 0 };
    EImageAspect AspectMask{ EImageAspect::Color };
    FRawMemoryConst Data;

#if USE_PPE_RHITASKNAME
    FUpdateImage() NOEXCEPT : TFrameTaskDesc<FUpdateImage>("UpdateImage", FDebugColorScheme::Get().HostToDeviceTransfer) {}
#endif

    FUpdateImage& SetImage(FRawImageID image, const int2& offset, FMipmapLevel mipmap = Default) { return SetImage(image, int3(offset, 0), mipmap); }
    FUpdateImage& SetImage(FRawImageID image, const int3& offset = Default, FMipmapLevel mipmap = Default) {
        Assert(image);
        DstImage = image;
        ImageOffset = offset;
        MipmapLevel = mipmap;
        return (*this);
    }
    FUpdateImage& SetImage(FRawImageID image, const int2& offset, FImageLayer layer, FMipmapLevel mipmap) {
        Assert(image);
        DstImage = image;
        ImageOffset = int3(offset, 0);
        ArrayLayer = layer;
        MipmapLevel = mipmap;
        return (*this);
    }

    FUpdateImage& SetData(FRawMemoryConst data, const uint2& dimension, u32 rowPitch = 0) { return SetData(data, uint3(dimension, 0), rowPitch); }
    FUpdateImage& SetData(FRawMemoryConst data, const uint3& dimension, u32 rowPitch = 0, u32 slicePitch = 0) {
        Assert(not data.empty());
        Data = data;
        ImageSize = dimension;
        DataRowPitch = rowPitch;
        DataSlicePitch = slicePitch;
        return (*this);
    }

    template <typename T>
    FUpdateImage& SetData(TMemoryView<const T> data, const uint2& dimension, u32 rowPitch = 0) { return SetData(data.template Cast<const u8>(), uint3(dimension, 0), rowPitch); }
    template <typename T>
    FUpdateImage& SetData(TMemoryView<const T> data, const uint3& dimension, u32 rowPitch = 0, u32 slicePitch = 0) { return SetData(data.template Cast<const u8>(), dimension, rowPitch, slicePitch); }
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

#if USE_PPE_RHITASKNAME
    FReadImage() NOEXCEPT : TFrameTaskDesc<FReadImage>("ReadImage", FDebugColorScheme::Get().DeviceToHostTransfer) {}
#endif

    FReadImage& SetImage(FRawImageID image, const int2& offset, const uint2& size, FMipmapLevel mipmap = Default) { return SetImage(image, int3(offset, 0), uint3(size, 0), mipmap); }
    FReadImage& SetImage(FRawImageID image, const int3& offset, const uint3& size, FMipmapLevel mipmap = Default) {
        Assert(image);
        SrcImage = image;
        ImageOffset = offset;
        ImageSize = size;
        MipmapLevel = mipmap;
        return (*this);
    }
    FReadImage& SetImage(FRawImageID image, const int2& offset, const uint2& size, FImageLayer layer, FMipmapLevel mipmap = Default) {
        Assert(image);
        SrcImage = image;
        ImageOffset = int3(offset, 0);
        ImageSize = uint3(size, 0);
        ArrayLayer = layer;
        MipmapLevel = mipmap;
        return (*this);
    }

    FReadImage& SetCallback(const FCallback& callback) { return SetCallback(FCallback(callback)); }
    FReadImage& SetCallback(FCallback&& rcallback) {
        Assert(rcallback);
        Callback = std::move(rcallback);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FPresent
//----------------------------------------------------------------------------
struct FPresent final : details::TFrameTaskDesc<FPresent> {
    FRawSwapchainID Swapchain;
    FRawImageID SrcImage;
    FImageLayer Layer;
    FMipmapLevel Mipmap;

#if USE_PPE_RHITASKNAME
    FPresent() NOEXCEPT : TFrameTaskDesc<FPresent>("Present", FDebugColorScheme::Get().Present) {}
#else
    FPresent() = default;
#endif

    explicit FPresent(FRawSwapchainID swapchain, FRawImageID image) : FPresent() {
        SetSwapchain(swapchain).SetImage(image);
    }

    FPresent& SetSwapchain(FRawSwapchainID swapchain) {
        Assert(swapchain);
        Swapchain = swapchain;
        return (*this);
    }

    FPresent& SetImage(FRawImageID image, FImageLayer layer = Default, FMipmapLevel mipmap = Default) {
        Assert(image);
        SrcImage = image;
        Layer = layer;
        Mipmap = mipmap;
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FCustomTask
//----------------------------------------------------------------------------
struct FCustomTask final : details::TFrameTaskDesc<FCustomTask> {

    using FExternalContext = void*;
    using FCallback = TFunction<void(FExternalContext)>;
    using FImages = TFixedSizeStack<TPair<FRawImageID, EResourceState>, 8>;
    using FBuffers = TFixedSizeStack<TPair<FRawBufferID, EResourceState>, 8>;

    FCallback Callback;
    FImages Images;
    FBuffers Buffers;

#if USE_PPE_RHITASKNAME
    FCustomTask() NOEXCEPT : TFrameTaskDesc<FCustomTask>("CustomTask", FDebugColorScheme::Get().Debug) {}
#else
    FCustomTask() = default;
#endif

    explicit FCustomTask(FCallback&& rcallback) : FCustomTask() {
        Callback = std::move(rcallback);
    }

    FCustomTask& AddImage(FRawImageID image, EResourceState state) {
        Assert(image);
        Images.Push(image, state);
        return (*this);
    }

    FCustomTask& AddBuffer(FRawBufferID image, EResourceState state) {
        Assert(image);
        Buffers.Push(image, state);
        return (*this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
