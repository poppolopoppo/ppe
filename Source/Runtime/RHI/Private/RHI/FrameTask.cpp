#include "stdafx.h"

#include "RHI/FrameTask.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FSubmitRenderPass
//----------------------------------------------------------------------------
FSubmitRenderPass::FSubmitRenderPass(FLogicalPassID renderPassId) NOEXCEPT :
#if USE_PPE_RHIDEBUG
    TFrameTaskDesc<FSubmitRenderPass>("SubmitRenderPass", FDebugColorScheme::Get().RenderPass),
#endif
    RenderPassId(std::move(renderPassId)) {
    Assert(renderPassId);
}
//----------------------------------------------------------------------------
FSubmitRenderPass::~FSubmitRenderPass() = default;
//----------------------------------------------------------------------------
FSubmitRenderPass& FSubmitRenderPass::AddImage(FRawImageID id, EResourceState state/* = EResourceState::ShaderSample */) {
    Assert(id);
    Images.Push(id, state);
    return (*this);
}
//----------------------------------------------------------------------------
FSubmitRenderPass& FSubmitRenderPass::AddBuffer(FRawBufferID id, EResourceState state/* = EResourceState::UniformRead */) {
    Assert(id);
    Buffers.Push(id, state);
    return (*this);
}
//----------------------------------------------------------------------------
// FDispatchCompute
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDispatchCompute::FDispatchCompute() NOEXCEPT : TFrameTaskDesc<FDispatchCompute>("DispatchCompute", FDebugColorScheme::Get().Compute) {}
#else
FDispatchCompute::FDispatchCompute() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDispatchCompute::~FDispatchCompute() = default;
//----------------------------------------------------------------------------
FDispatchCompute& FDispatchCompute::AddPushConstant(const FPushConstantID& id, const void* p, size_t size) {
    PushConstants.Push(id, p, size); return (*this);
}
//----------------------------------------------------------------------------
FDispatchCompute& FDispatchCompute::AddResources(const FDescriptorSetID& id, const PCPipelineResources& res) {
    Assert(id);
    Assert(res);
    Resources.Add(id) = res;
    return (*this);
}
//----------------------------------------------------------------------------
FDispatchCompute& FDispatchCompute::Dispatch(const uint3& off, const uint3& count) {
    Emplace_Back(Commands, off, count); return (*this);
}
//----------------------------------------------------------------------------
// FDispatchComputeIndirect
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FDispatchComputeIndirect::FDispatchComputeIndirect() NOEXCEPT : TFrameTaskDesc<FDispatchComputeIndirect>("DispatchComputeIndirect", FDebugColorScheme::Get().Compute) {}
#else
FDispatchComputeIndirect::FDispatchComputeIndirect() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FDispatchComputeIndirect::~FDispatchComputeIndirect() = default;
//----------------------------------------------------------------------------
FDispatchComputeIndirect& FDispatchComputeIndirect::Dispatch(size_t offset) {
    Emplace_Back(Commands, offset); return (*this);
}
//----------------------------------------------------------------------------
FDispatchComputeIndirect& FDispatchComputeIndirect::AddPushConstant(const FPushConstantID& id, const void* p, size_t size) {
    PushConstants.Push(id, p, size); return (*this);
}
//----------------------------------------------------------------------------
FDispatchComputeIndirect& FDispatchComputeIndirect::AddResources(const FDescriptorSetID& id, const PCPipelineResources& res) {
    Assert(id);
    Assert(res);
    Resources.Add(id) = res;
    return (*this);
}
//----------------------------------------------------------------------------
// FCopyBuffer
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FCopyBuffer::FCopyBuffer() NOEXCEPT : TFrameTaskDesc<FCopyBuffer>("CopyBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FCopyBuffer::FCopyBuffer() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FCopyBuffer::~FCopyBuffer() = default;
//----------------------------------------------------------------------------
FCopyBuffer& FCopyBuffer::AddRegion(size_t srcOffset, size_t dstOffset, size_t size) {
    Assert(size);
    Emplace_Back(Regions, srcOffset, dstOffset, size);
    return (*this);
}
//----------------------------------------------------------------------------
// FCopyImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FCopyImage::FCopyImage() NOEXCEPT : TFrameTaskDesc<FCopyImage>("CopyImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FCopyImage::FCopyImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FCopyImage::~FCopyImage() = default;
//----------------------------------------------------------------------------
FCopyImage& FCopyImage::AddRegion(
    const FImageSubresourceRange& srcSubresource, const int2& srcOffset,
    const FImageSubresourceRange& dstSubresource, const int2& dstOffset,
    const uint2& size ) {
    return AddRegion(srcSubresource, int3(srcOffset, 0), dstSubresource, int3(dstOffset, 0), uint3(size, 1));
}
//----------------------------------------------------------------------------
FCopyImage& FCopyImage::AddRegion(
    const FImageSubresourceRange& srcSubresource, const int3& srcOffset,
    const FImageSubresourceRange& dstSubresource, const int3& dstOffset,
    const uint3& size ) {
    Assert(AllGreater(size, uint3(0)));
    Emplace_Back(Regions, srcSubresource, srcOffset, dstSubresource, dstOffset, size);
    return (*this);
}
//----------------------------------------------------------------------------
// FCopyBufferToImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FCopyBufferToImage::FCopyBufferToImage() NOEXCEPT : TFrameTaskDesc<FCopyBufferToImage>("CopyBufferToImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FCopyBufferToImage::FCopyBufferToImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FCopyBufferToImage::~FCopyBufferToImage() = default;
//----------------------------------------------------------------------------
FCopyBufferToImage& FCopyBufferToImage::AddRegion(
    size_t srcBufferOffset, u32 srcBufferRowLength, u32 srcBufferImageHeight,
    const FImageSubresourceRange& dstImageLayers, const int2& dstImageOffset, const uint2& dstImageSize ) {
    return AddRegion(srcBufferOffset, srcBufferRowLength, srcBufferImageHeight, dstImageLayers, int3(dstImageOffset, 0), uint3(dstImageSize, 1));
}
//----------------------------------------------------------------------------
FCopyBufferToImage& FCopyBufferToImage::AddRegion(
    size_t SrcBufferOffset, u32 srcBufferRowLength, u32 srcBufferImageHeight,
    const FImageSubresourceRange& dstImageLayers, const int3& dstImageOffset, const uint3& dstImageSize) {
    Assert(AllGreater(dstImageSize, uint3(0)));
    Emplace_Back(Regions, SrcBufferOffset, srcBufferRowLength, srcBufferImageHeight, dstImageLayers, dstImageOffset, dstImageSize);
    return (*this);
}
//----------------------------------------------------------------------------
// FCopyImageToBuffer
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FCopyImageToBuffer::FCopyImageToBuffer() NOEXCEPT : TFrameTaskDesc<FCopyImageToBuffer>("CopyImageToBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FCopyImageToBuffer::FCopyImageToBuffer() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FCopyImageToBuffer::~FCopyImageToBuffer() = default;
//----------------------------------------------------------------------------
FCopyImageToBuffer& FCopyImageToBuffer::AddRegion(
    const FImageSubresourceRange& srcImageLayers, const int2& srcImageOffset, const uint2& srcImageSize,
    size_t dstBufferOffset, u32 dstBufferRowLength, u32 dstBufferImageHeight ) {
    return AddRegion(srcImageLayers, int3(srcImageOffset, 0), uint3(srcImageSize, 1), dstBufferOffset, dstBufferRowLength, dstBufferImageHeight);
}
//----------------------------------------------------------------------------
FCopyImageToBuffer& FCopyImageToBuffer::AddRegion(
    const FImageSubresourceRange& srcImageLayers, const int3& srcImageOffset, const uint3& srcImageSize,
    size_t dstBufferOffset, u32 dstBufferRowLength, u32 dstBufferImageHeight ) {
    Assert(AllGreater(srcImageSize, uint3(0)));
    Emplace_Back(Regions, dstBufferOffset, dstBufferRowLength, dstBufferImageHeight, srcImageLayers, srcImageOffset, srcImageSize);
    return (*this);
}
//----------------------------------------------------------------------------
// FBlitImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FBlitImage::FBlitImage() NOEXCEPT : TFrameTaskDesc<FBlitImage>("BlitImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FBlitImage::FBlitImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FBlitImage::~FBlitImage() = default;
//----------------------------------------------------------------------------
FBlitImage& FBlitImage::AddRegion(
    const FImageSubresourceRange& srcSubresource, const int2& srcOffsetStart, const int2 srcOffsetEnd,
    const FImageSubresourceRange& dstSubresource, const int2& dstOffsetStart, const int2 dstOffsetEnd ) {
    return AddRegion(
        srcSubresource, int3(srcOffsetStart, 0), int3(srcOffsetEnd, 1),
        dstSubresource, int3(dstOffsetStart, 0), int3(dstOffsetEnd, 1) );
}
//----------------------------------------------------------------------------
FBlitImage& FBlitImage::AddRegion(
    const FImageSubresourceRange& srcSubresource, const int3& srcOffsetStart, const int3& srcOffsetEnd,
    const FImageSubresourceRange& dstSubresource, const int3& dstOffsetStart, const int3& dstOffsetEnd ) {
    Assert(AllLess(srcOffsetStart, srcOffsetEnd));
    Assert(AllLess(dstOffsetStart, dstOffsetEnd));
    Emplace_Back(Regions, srcSubresource, srcOffsetStart, srcOffsetEnd, dstSubresource, dstOffsetStart, dstOffsetEnd);
    return (*this);
}
//----------------------------------------------------------------------------
// FGenerateMipmaps
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FGenerateMipmaps::FGenerateMipmaps() NOEXCEPT : TFrameTaskDesc<FGenerateMipmaps>("GenerateMipmaps", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FGenerateMipmaps::FGenerateMipmaps() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FGenerateMipmaps::~FGenerateMipmaps() = default;
//----------------------------------------------------------------------------
// FResolveImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FResolveImage::FResolveImage() NOEXCEPT : TFrameTaskDesc<FResolveImage>("ResolveImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FResolveImage::FResolveImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FResolveImage::~FResolveImage() = default;
//----------------------------------------------------------------------------
FResolveImage& FResolveImage::AddRegion(
    const FImageSubresourceRange& srcSubresource, const int2& srcOffset,
    const FImageSubresourceRange& dstSubresource, const int2& dstOffset,
    const uint2& extent ) {
    return AddRegion(srcSubresource, int3(srcOffset, 0), dstSubresource, int3(dstOffset, 0), uint3(extent, 1));
}
//----------------------------------------------------------------------------
FResolveImage& FResolveImage::AddRegion(
    const FImageSubresourceRange& srcSubresource, const int3& srcOffset,
    const FImageSubresourceRange& dstSubresource, const int3& dstOffset,
    const uint3& extent ) {
    Assert(AllGreater(extent, uint3(0)));
    Emplace_Back(Regions, srcSubresource, srcOffset, dstSubresource, dstOffset, extent);
    return (*this);
}
//----------------------------------------------------------------------------
// FFillBuffer
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FFillBuffer::FFillBuffer() NOEXCEPT : TFrameTaskDesc<FFillBuffer>("FillBuffer", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FFillBuffer::FFillBuffer() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FFillBuffer::~FFillBuffer() = default;
//----------------------------------------------------------------------------
FFillBuffer& FFillBuffer::SetBuffer(FRawBufferID buffer, size_t offset, size_t size) {
    Assert(buffer);
    DstBuffer = buffer;
    DstOffset = offset;
    Size = size;
    return (*this);
}
//----------------------------------------------------------------------------
// FClearColorImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FClearColorImage::FClearColorImage() NOEXCEPT : TFrameTaskDesc<FClearColorImage>("ClearColorImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FClearColorImage::FClearColorImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FClearColorImage::~FClearColorImage() = default;
//----------------------------------------------------------------------------
FClearColorImage& FClearColorImage::Clear(const FRgba32u& color) {
    ClearColor.emplace<FRgba32u>(color);
    return (*this);
}
//----------------------------------------------------------------------------
FClearColorImage& FClearColorImage::Clear(const FRgba32i& color) {
    ClearColor.emplace<FRgba32i>(color);
    return (*this);
}
//----------------------------------------------------------------------------
FClearColorImage& FClearColorImage::Clear(const FLinearColor& color) {
    ClearColor.emplace<FLinearColor>(color);
    return (*this);
}
//----------------------------------------------------------------------------
FClearColorImage& FClearColorImage::AddRange(FMipmapLevel baseMipLevel, u32 levelCount, FImageLayer baseLayer, u32 layerCount) {
    Assert(levelCount > 0);
    Assert(layerCount > 0);
    Emplace_Back(Ranges, EImageAspect::Color, baseMipLevel, levelCount, baseLayer, layerCount);
    return (*this);
}
//----------------------------------------------------------------------------
// FClearDepthStencilImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FClearDepthStencilImage::FClearDepthStencilImage() NOEXCEPT : TFrameTaskDesc<FClearDepthStencilImage>("ClearDepthStencilImage", FDebugColorScheme::Get().DeviceLocalTransfer) {}
#else
FClearDepthStencilImage::FClearDepthStencilImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FClearDepthStencilImage::~FClearDepthStencilImage() = default;
//----------------------------------------------------------------------------
FClearDepthStencilImage& FClearDepthStencilImage::AddRange(FMipmapLevel baseMipLevel, u32 levelCount, FImageLayer baseLayer, u32 layerCount) {
    Assert(levelCount > 0);
    Assert(layerCount > 0);
    Emplace_Back(Ranges, EImageAspect::DepthStencil, baseMipLevel, levelCount, baseLayer, layerCount);
    return (*this);
}
//----------------------------------------------------------------------------
// FUpdateBuffer
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FUpdateBuffer::FUpdateBuffer() NOEXCEPT : TFrameTaskDesc<FUpdateBuffer>("UpdateBuffer", FDebugColorScheme::Get().HostToDeviceTransfer) {}
#else
FUpdateBuffer::FUpdateBuffer() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FUpdateBuffer::FUpdateBuffer(FRawBufferID buffer, size_t offset, const FRawMemoryConst& data) NOEXCEPT
:   FUpdateBuffer() {
    SetBuffer(buffer).AddData(data, offset);
}
//----------------------------------------------------------------------------
FUpdateBuffer::~FUpdateBuffer() = default;
//----------------------------------------------------------------------------
FUpdateBuffer& FUpdateBuffer::SetBuffer(FRawBufferID buffer) {
    Assert(buffer);
    DstBuffer = buffer;
    return (*this);
}
//----------------------------------------------------------------------------
FUpdateBuffer& FUpdateBuffer::AddData(const FRawMemoryConst& data, size_t offset/* = 0 */) {
    Assert(not data.empty());
    Emplace_Back(Regions, offset, data);
    return (*this);
}
//----------------------------------------------------------------------------
// FReadBuffer
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FReadBuffer::FReadBuffer() NOEXCEPT : TFrameTaskDesc<FReadBuffer>("ReadBuffer", FDebugColorScheme::Get().DeviceToHostTransfer) {}
#else
FReadBuffer::FReadBuffer() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FReadBuffer::~FReadBuffer() = default;
//----------------------------------------------------------------------------
FReadBuffer& FReadBuffer::SetBuffer(FRawBufferID buffer, size_t offset, size_t size) {
    Assert(buffer);
    SrcBuffer = buffer;
    SrcOffset = offset;
    SrcSize = size;
    return (*this);
}
//----------------------------------------------------------------------------
FReadBuffer& FReadBuffer::SetCallback(FCallback&& rcallback) {
    Assert(rcallback);
    Callback = std::move(rcallback);
    return (*this);
}
//----------------------------------------------------------------------------
// FUpdateImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FUpdateImage::FUpdateImage() NOEXCEPT : TFrameTaskDesc<FUpdateImage>("UpdateImage", FDebugColorScheme::Get().HostToDeviceTransfer) {}
#else
FUpdateImage::FUpdateImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FUpdateImage::~FUpdateImage() = default;
//----------------------------------------------------------------------------
FUpdateImage& FUpdateImage::SetImage(FRawImageID image, const int3& offset/* = Default */, FMipmapLevel mipmap/* = Default */) {
    Assert(image);
    DstImage = image;
    ImageOffset = offset;
    MipmapLevel = mipmap;
    return (*this);
}
//----------------------------------------------------------------------------
FUpdateImage& FUpdateImage::SetImage(FRawImageID image, const int2& offset, FImageLayer layer, FMipmapLevel mipmap) {
    Assert(image);
    DstImage = image;
    ImageOffset = int3(offset, 0);
    ArrayLayer = layer;
    MipmapLevel = mipmap;
    return (*this);
}
//----------------------------------------------------------------------------
FUpdateImage& FUpdateImage::SetData(FRawMemoryConst data, const uint3& dimension, size_t rowPitch/* = 0 */, size_t slicePitch/* = 0 */) {
    Assert(not data.empty());
    Data = data;
    ImageSize = dimension;
    DataRowPitch = rowPitch;
    DataSlicePitch = slicePitch;
    return (*this);
}
//----------------------------------------------------------------------------
// FReadImage
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FReadImage::FReadImage() NOEXCEPT : TFrameTaskDesc<FReadImage>("ReadImage", FDebugColorScheme::Get().DeviceToHostTransfer) {}
#else
FReadImage::FReadImage() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FReadImage::~FReadImage() = default;
//----------------------------------------------------------------------------
FReadImage& FReadImage::SetImage(FRawImageID image, const int3& offset, const uint3& size, FMipmapLevel mipmap/* = Default */) {
    Assert(image);
    SrcImage = image;
    ImageOffset = offset;
    ImageSize = size;
    MipmapLevel = mipmap;
    return (*this);
}
//----------------------------------------------------------------------------
FReadImage& FReadImage::SetImage(FRawImageID image, const int2& offset, const uint2& size, FImageLayer layer, FMipmapLevel mipmap/* = Default */) {
    Assert(image);
    SrcImage = image;
    ImageOffset = int3(offset, 0);
    ImageSize = uint3(size, 0);
    ArrayLayer = layer;
    MipmapLevel = mipmap;
    return (*this);
}
//----------------------------------------------------------------------------
FReadImage& FReadImage::SetCallback(FCallback&& rcallback) {
    Assert(rcallback);
    Callback = std::move(rcallback);
    return (*this);
}
//----------------------------------------------------------------------------
// FPresent
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FPresent::FPresent() NOEXCEPT : TFrameTaskDesc<FPresent>("Present", FDebugColorScheme::Get().Present) {}
#else
FPresent::FPresent() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FPresent::~FPresent() = default;
//----------------------------------------------------------------------------
FPresent::FPresent(FRawSwapchainID swapchain, FRawImageID image) NOEXCEPT : FPresent() {
    SetSwapchain(swapchain).SetImage(image);
}
//----------------------------------------------------------------------------
FPresent& FPresent::SetSwapchain(FRawSwapchainID swapchain) {
    Assert(swapchain);
    Swapchain = swapchain;
    return (*this);
}
//----------------------------------------------------------------------------
FPresent& FPresent::SetImage(FRawImageID image, FImageLayer layer/* = Default */, FMipmapLevel mipmap/* = Default */) {
    Assert(image);
    SrcImage = image;
    Layer = layer;
    Mipmap = mipmap;
    return (*this);
}
//----------------------------------------------------------------------------
// FCustomTask
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
FCustomTask::FCustomTask() NOEXCEPT : TFrameTaskDesc<FCustomTask>("CustomTask", FDebugColorScheme::Get().Debug) {}
#else
FCustomTask::FCustomTask() NOEXCEPT = default;
#endif
//----------------------------------------------------------------------------
FCustomTask::~FCustomTask() = default;
//----------------------------------------------------------------------------
FCustomTask::FCustomTask(FCallback&& rcallback) NOEXCEPT : FCustomTask() {
    Callback = std::move(rcallback);
}
//----------------------------------------------------------------------------
FCustomTask& FCustomTask::AddImage(FRawImageID image, EResourceState state) {
    Assert(image);
    Images.Push(image, state);
    return (*this);
}
//----------------------------------------------------------------------------
FCustomTask& FCustomTask::AddBuffer(FRawBufferID image, EResourceState state) {
    Assert(image);
    Buffers.Push(image, state);
    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
