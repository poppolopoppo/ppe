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
struct TFrameGraphTaskDesc {
    using FTasks = TFixedSizeStack<PFrameGraphTask, MaxTaskDependencies>;

    using self_type = _Self;

    FTasks Dependencies;

    TFrameGraphTaskDesc() = default;

    self_type& DependsOn(const PFrameGraphTask& task) {
        if (task) Add_AssertUnique(Dependencies, task);
        return static_cast<self_type&>(*this);
    }
    self_type& DependsOn(PFrameGraphTask&& rtask) {
        if (rtask) Add_AssertUnique(Dependencies, std::move(rtask));
        return static_cast<self_type&>(*this);
    }
    template <typename _Arg0, typename... _Args>
    self_type& DependsOn(_Arg0 task0, _Args&&... tasks) {
        DependsOn(std::forward<_Arg0>(task0));
        DependsOn(std::forward<_Args>(tasks)...);
        return static_cast<self_type&>(*this);
    }

#if USE_PPE_RHIDEBUG
    FTaskName Name;
    FRgba8u DebugColor;

    TFrameGraphTaskDesc(FStringView name, FRgba8u color) : Name(name), DebugColor(color) {}

    self_type& SetName(FStringView value) { Name.Assign(value); return static_cast<self_type&>(*this); }
    self_type& SetDebugColor(FStringView value) { Name.Assign(value); return static_cast<self_type&>(*this); }
#endif
};
} //!details
//----------------------------------------------------------------------------
// FSubmitRenderPass
//----------------------------------------------------------------------------
struct FSubmitRenderPass : details::TFrameGraphTaskDesc<FSubmitRenderPass> {
    using FImages = FCustomDraw::FImages;
    using FBuffers = FCustomDraw::FBuffers;

    FLogicalPassID RenderPassId;
    FImages Images;
    FBuffers Buffers;

    explicit FSubmitRenderPass(FLogicalPassID&& renderPassId) :
#if USE_PPE_RHIDEBUG
        TFrameGraphTaskDesc<FSubmitRenderPass>("SubmitRenderPass", FDebugColorScheme::Get().RenderPass),
#endif
        RenderPassId(std::move(renderPassId)) {
        Assert(renderPassId);
    }

    FSubmitRenderPass& AddImage(FRawImageID id, EResourceState state = EResourceState::ShaderSample) {
        Assert(id);
        Images.Add_Overwrite(id, state);
        return (*this);
    }

    FSubmitRenderPass& AddBuffer(FRawBufferID id, EResourceState state = EResourceState::UniformRead) {
        Assert(id);
        Buffers.Add_Overwrite(id, state);
        return (*this);
    }
};
//----------------------------------------------------------------------------
// FDispatchCompute
//----------------------------------------------------------------------------
struct FDispatchCompute : details::TFrameGraphTaskDesc<FDispatchCompute> {

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
    FDispatchCompute() : TFrameGraphTaskDesc<FDispatchCompute>("DispatchCompute", FDebugColorScheme::Get().Compute) {}
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

#if USE_PPE_RHIPROFILING
    FDispatchCompute& EnableShaderProfiling() { return EnableShaderProfiling(uint3(~0u)); }
    FDispatchCompute& EnableShaderProfiling(const uint3& globalId);
#endif
};
//----------------------------------------------------------------------------
// FDispatchComputeIndirect
//----------------------------------------------------------------------------
struct FDispatchComputeIndirect final : details::TFrameGraphTaskDesc<FDispatchComputeIndirect> {

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
    FDispatchComputeIndirect() : TFrameGraphTaskDesc<FDispatchComputeIndirect>("DispatchComputeIndirect", FDebugColorScheme::Get().Compute) {}
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

#if USE_PPE_RHIPROFILING
    FDispatchComputeIndirect& EnableShaderProfiling() { return EnableShaderProfiling(uint3(~0u)); }
    FDispatchComputeIndirect& EnableShaderProfiling(const uint3& globalId);
#endif
};
//----------------------------------------------------------------------------
// FCopyBuffer
//----------------------------------------------------------------------------
struct FCopyBuffer final : details::TFrameGraphTaskDesc<FCopyBuffer> {

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
    FCopyBuffer() : TFrameGraphTaskDesc<FCopyBuffer>("CopyBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
struct FCopyImage final : details::TFrameGraphTaskDesc<FCopyImage> {

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
    FCopyImage() : TFrameGraphTaskDesc<FCopyImage>("CopyImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
struct FCopyBufferToImage final : details::TFrameGraphTaskDesc<FCopyBufferToImage> {

    struct FRegion {
        u32 SrcBufferOffset;
        u32 SrcBufferRowLength;
        u32 SrcBufferImageHeight;
        FImageSubresourceRange DstImageLayers;
        int3 DstImageOffset;
        uint3 DstImageSize;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawBufferID SrcBuffer;
    FRawImageID DstImage;
    FRegions Regions;

#if USE_PPE_RHITASKNAME
    FCopyBufferToImage() : TFrameGraphTaskDesc<FCopyBufferToImage>("CopyBufferToImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
// FBlitImage
//----------------------------------------------------------------------------
struct FBlitImage final : details::TFrameGraphTaskDesc<FBlitImage> {

    struct FRegion {
        FImageSubresourceRange SrcSubresource;
        int3 SrcOffsetStart;
        int3 SrcOffsetEnd;
        FImageSubresourceRange DstSubresource;
        int3 DstOffsetStart;
        int3 DstOffsetEnd;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawImageID SrcImage;
    FRawImageID DstImage;
    ETextureFilter Filter{ ETextureFilter::Nearest };
    FRegions Regions;

#if USE_PPE_RHITASKNAME
    FBlitImage() : TFrameGraphTaskDesc<FBlitImage>("BlitImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
struct FGenerateMipmaps : details::TFrameGraphTaskDesc<FGenerateMipmaps> {
    FRawImageID Image;
    FMipmapLevel BaseLevel;
    u32 LevelCount{ 0 };

#if USE_PPE_RHITASKNAME
    FGenerateMipmaps() : TFrameGraphTaskDesc<FGenerateMipmaps>("GenerateMipmaps", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#endif

    FGenerateMipmaps& SetImage(FRawImageID image) { Assert(image); Image = image; return (*this); }
    FGenerateMipmaps& SetRange(FMipmapLevel base, u32 count) { BaseLevel = base; LevelCount = count; return (*this); }
};
//----------------------------------------------------------------------------
// FResolveImage
//----------------------------------------------------------------------------
struct FResolveImage final : details::TFrameGraphTaskDesc<FResolveImage> {

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
    FResolveImage() : TFrameGraphTaskDesc<FResolveImage>("ResolveImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
struct FFillBuffer final : details::TFrameGraphTaskDesc<FFillBuffer> {
    FRawBufferID DstBuffer;
    u32 DstOffset{ 0 };
    u32 Size{ 0 };
    u32 Pattern{ 0 };

#if USE_PPE_RHITASKNAME
    FFillBuffer() : TFrameGraphTaskDesc<FFillBuffer>("FillBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
struct FClearColorImage final : details::TFrameGraphTaskDesc<FClearColorImage> {

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
        FRgba32u >;

    FRawImageID DstImage;
    FClearColor ClearColor;
    FRanges Ranges;

#if USE_PPE_RHITASKNAME
    FClearColorImage() : TFrameGraphTaskDesc<FClearColorImage>("ClearColorImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
struct FClearDepthStencilImage final : details::TFrameGraphTaskDesc<FClearDepthStencilImage> {
    using FRange = FClearColorImage::FRange;
    using FRanges = FClearColorImage::FRanges;

    FRawImageID DstImage;
    FDepthValue ClearDepth{ 0.0f };
    FStencilValue ClearStencil{ 0 };
    FRanges Ranges;

#if USE_PPE_RHITASKNAME
    FClearDepthStencilImage() : TFrameGraphTaskDesc<FClearDepthStencilImage>("ClearDepthStencilImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
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
struct FUpdateBuffer final : details::TFrameGraphTaskDesc<FUpdateBuffer> {

    struct FRegion {
        u32 Offset{ 0 };
        FRawMemoryConst Data;
    };
    using FRegions = TFixedSizeStack<FRegion, MaxCopyRegions>;

    FRawBufferID DstBuffer;
    FRegions Regions;

#if USE_PPE_RHITASKNAME
    FUpdateBuffer() : TFrameGraphTaskDesc<FUpdateBuffer>("UpdateBuffer", FDebugColorScheme::Get().HostToDeviceTransfer) {}
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
struct FReadBuffer final : details::TFrameGraphTaskDesc<FReadBuffer> {

    using FCallback = TFunction<void(const FBufferView&)>;

    FRawBufferID SrcBuffer;
    u32 SrcOffset{ 0 };
    u32 SrcSize{ 0 };
    FCallback Callback;

#if USE_PPE_RHITASKNAME
    FReadBuffer() : TFrameGraphTaskDesc<FReadBuffer>("ReadBuffer", FDebugColorScheme::Get().DeviceToHostTransfer) {}
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
struct FUpdateImage final : details::TFrameGraphTaskDesc<FUpdateImage> {

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
    FUpdateImage() : TFrameGraphTaskDesc<FUpdateImage>("UpdateImage", FDebugColorScheme::Get().HostToDeviceTransfer) {}
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
struct FReadImage final : details::TFrameGraphTaskDesc<FReadImage> {

    using FCallback = TFunction<void(const FImageView&)>;

    FRawImageID SrcImage;
    int3 ImageOffset{ 0 };
    uint3 ImageSize{ 0 };
    FImageLayer ArrayLayer;
    FMipmapLevel MipmapLevel;
    EImageAspect AspectMask = EImageAspect::Color;
    FCallback Callback;

#if USE_PPE_RHITASKNAME
    FReadImage() : TFrameGraphTaskDesc<FReadImage>("ReadImage", FDebugColorScheme::Get().DeviceToHostTransfer) {}
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
struct FPresent final : details::TFrameGraphTaskDesc<FPresent> {
    FRawSwapchainID Swapchain;
    FRawImageID SrcImage;
    FImageLayer Layer;
    FMipmapLevel Mipmap;

#if USE_PPE_RHITASKNAME
    FPresent() : TFrameGraphTaskDesc<FPresent>("Present", FDebugColorScheme::Get().Present) {}
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
struct FCustomTask final : details::TFrameGraphTaskDesc<FCustomTask> {

    using FExternalContext = void*;
    using FCallback = TFunction<void(FExternalContext)>;
    using FImages = TFixedSizeHashMap<FRawImageID, EResourceState, 8>;
    using FBuffers = TFixedSizeHashMap<FRawBufferID, EResourceState, 8>;

    FCallback Callback;
    FImages Images;
    FBuffers Buffers;

#if USE_PPE_RHITASKNAME
    FCustomTask() : TFrameGraphTaskDesc<FCustomTask>("CustomTask", FDebugColorScheme::Get().Debug) {}
#else
    FCustomTask() = default;
#endif

    explicit FCustomTask(FCallback&& rcallback) : FCustomTask() {
        Callback = std::move(rcallback);
    }

    FCustomTask& AddImage(FRawImageID image, EResourceState state) {
        Assert(image);
        Images.Add_Overwrite(image, state);
        return (*this);
    }

    FCustomTask& AddBuffer(FRawBufferID image, EResourceState state) {
        Assert(image);
        Buffers.Add_Overwrite(image, state);
        return (*this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE