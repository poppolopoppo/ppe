﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI/PipelineResources.h"

#include "RHI/ResourceId.h"

#include "Diagnostic/Logger.h"
#include "Meta/Functor.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPipelineResources::FPipelineResources() NOEXCEPT {
    ResetCachedId_();
}
//----------------------------------------------------------------------------
FPipelineResources::~FPipelineResources() {
    ONLY_IF_RHIDEBUG( Unused(_dynamicData.LockExclusive()) );
}
//----------------------------------------------------------------------------
FPipelineResources::FPipelineResources(const FPipelineResources& other)
:   FRefCountable(other)
,   _dynamicData(*other._dynamicData.LockShared())
,   _allowEmptyResources(other._allowEmptyResources) {
    STATIC_ASSERT(FCachedID_::is_always_lock_free);
    SetCachedId_(other.CachedId_());
}
//----------------------------------------------------------------------------
FPipelineResources::FPipelineResources(FPipelineResources&& rvalue) NOEXCEPT
:   FRefCountable(std::move(rvalue))
,   _dynamicData(std::move(*rvalue._dynamicData.LockExclusive()))
,   _allowEmptyResources(rvalue._allowEmptyResources) {
    SetCachedId_(rvalue.CachedId_());
    rvalue.SetCachedId_(FRawPipelineResourcesID{0});
}
//----------------------------------------------------------------------------
template <typename T>
T& FPipelineResources::Resource_(const FUniformID& id) {
    Assert(id.Valid());

    const auto exclusiveData = _dynamicData.LockExclusive();

    const auto uniforms = exclusiveData->Uniforms();
    const auto it = Meta::LowerBound(uniforms.begin(), uniforms.end(), id);
    if (Likely(uniforms.end() != it && *it == id)) {
        Assert_NoAssume(it->Type == T::TypeId);
        return it->template Get<T>();
    }

    PPE_LOG(RHI, Error, "failed to find uniform in pipeline resource with id: {0}", id);
    AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T>
bool FPipelineResources::HasResource_(const FUniformID& id) const {
    Assert(id.Valid());

    const auto sharedDynamicData = _dynamicData.LockShared();
    const auto uniforms = sharedDynamicData->Uniforms();
    const auto it = Meta::LowerBound(uniforms.begin(), uniforms.end(), id);
    if (Likely(uniforms.end() != it && *it == id))
        return (it->Type == T::TypeId);

    return false;
}
//----------------------------------------------------------------------------
bool FPipelineResources::HasImage(const FUniformID& id) const {
    return HasResource_<FImage>(id);
}
bool FPipelineResources::HasSampler(const FUniformID& id) const {
    return HasResource_<FSampler>(id);
}
bool FPipelineResources::HasTexture(const FUniformID& id) const {
    return HasResource_<FTexture>(id);
}
bool FPipelineResources::HasBuffer(const FUniformID& id) const {
    return HasResource_<FBuffer>(id);
}
bool FPipelineResources::HasTexelBuffer(const FUniformID& id) const {
    return HasResource_<FTexelBuffer>(id);
}
bool FPipelineResources::HasRayTracingScene(const FUniformID& id) const {
    return HasResource_<FRayTracingScene>(id);
}
//----------------------------------------------------------------------------
void FPipelineResources::Reset(const FUniformID& uniform) {
    Assert(uniform.Valid());

    const auto exclusiveData = _dynamicData.LockExclusive();

    const auto uniforms = exclusiveData->Uniforms();
    const auto it = Meta::LowerBound(uniforms.begin(), uniforms.end(), uniform);
    Assert(uniforms.end() != it);
    Assert(*it == uniform);

    switch (it->Type) {
    case EDescriptorType::Unknown: break;
    case EDescriptorType::Buffer: it->Get<FBuffer>().Elements.Count = 0; break;
    case EDescriptorType::TexelBuffer: it->Get<FTexelBuffer>().Elements.Count = 0; break;
    case EDescriptorType::SubpassInput:
    case EDescriptorType::Image: it->Get<FImage>().Elements.Count = 0; break;
    case EDescriptorType::Texture: it->Get<FTexture>().Elements.Count = 0; break;
    case EDescriptorType::Sampler: it->Get<FSampler>().Elements.Count = 0; break;
    case EDescriptorType::RayTracingScene: it->Get<FRayTracingScene>().Elements.Count = 0; break;
    }

    ResetCachedId_();
}
//----------------------------------------------------------------------------
void FPipelineResources::ResetAll() {
    _dynamicData.LockShared()->EachUniform([](auto&, auto& resource) {
        resource.Elements.Count = 0;
    });

    ResetCachedId_();
}
//----------------------------------------------------------------------------
// Images
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImage(const FUniformID& id, FRawImageID image, u32 elementIndex) {
    Assert(id.Valid());
    Assert(image.Valid());

    auto& resource = Resource_<FImage>(id);
    Assert(elementIndex < resource.Elements.Capacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.Desc.has_value() || resource.Elements.Count <= elementIndex )
        ResetCachedId_();

    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);
    element.ImageId = image;
    element.Desc.reset();

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImage(const FUniformID& id, FRawImageID image, const FImageViewDesc& desc, u32 elementIndex) {
    Assert(id.Valid());
    Assert(image.Valid());

    auto& resource = Resource_<FImage>(id);
    Assert(elementIndex < resource.Elements.Capacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.Desc != desc || resource.Elements.Count <= elementIndex )
        ResetCachedId_();

    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);
    element.ImageId = image;
    element.Desc = desc;

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImages(const FUniformID& id, TMemoryView<const FImageID> images) {
    STATIC_ASSERT(sizeof(FRawImageID) == sizeof(FImageID));
    return BindImages(id, images.Cast<const FRawImageID>());
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImages(const FUniformID& id, TMemoryView<const FRawImageID> images) {
    Assert(id.Valid());

    auto& resource = Resource_<FImage>(id);

    Assert(images.size() <= resource.Elements.Capacity);
    bool needUpdate = (images.size() != resource.Elements.Count);
    resource.Elements.Count = checked_cast<u16>(images.size());

    forrange(i, 0, images.size()) {
        Assert(images[i].Valid());

        FImage::FElement& element = resource.Elements[i];
        needUpdate |= (element.ImageId != images[i] || element.Desc.has_value());

        element.ImageId = images[i];
        element.Desc.reset();
    }

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTexture(const FUniformID& id, FRawImageID image, FRawSamplerID sampler, u32 elementIndex) {
    Assert(id.Valid());
    Assert(image.Valid());
    Assert(sampler.Valid());

    auto& resource = Resource_<FTexture>(id);
    Assert(elementIndex < resource.Elements.Capacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.SamplerId != sampler ||
        element.Desc.has_value() ||
        resource.Elements.Count <= elementIndex )
        ResetCachedId_();

    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);
    element.ImageId = image;
    element.SamplerId = sampler;
    element.Desc.reset();

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTexture(const FUniformID& id, FRawImageID image, FRawSamplerID sampler, const FImageViewDesc& desc, u32 elementIndex) {
    Assert(id.Valid());
    Assert(image.Valid());
    Assert(sampler.Valid());

    auto& resource = Resource_<FTexture>(id);
    Assert(elementIndex < resource.Elements.Capacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.SamplerId != sampler ||
        not element.Desc.has_value() ||
        element.Desc.value() != desc ||
        resource.Elements.Count <= elementIndex )
        ResetCachedId_();

    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);
    element.ImageId = image;
    element.SamplerId = sampler;
    element.Desc = desc;

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTextures(const FUniformID& id, TMemoryView<const FImageID> images, FRawSamplerID sampler) {
    STATIC_ASSERT(sizeof(FImageID) == sizeof(FRawImageID));
    return BindTextures(id, images.Cast<const FRawImageID>(), sampler);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTextures(const FUniformID& id, TMemoryView<const FRawImageID> images, FRawSamplerID sampler) {
    Assert(id.Valid());
    Assert(sampler.Valid());

    auto& resource = Resource_<FTexture>(id);

    Assert(images.size() <= resource.Elements.Capacity);
    bool needUpdate = (images.size() != resource.Elements.Count);
    resource.Elements.Count = checked_cast<u16>(images.size());

    forrange(i, 0, images.size()) {
        Assert(images[i].Valid());

        auto& element = resource.Elements[i];
        needUpdate |= (element.ImageId != images[i] || element.SamplerId != sampler || element.Desc.has_value());

        element.ImageId = images[i];
        element.SamplerId = sampler;
        element.Desc.reset();
    }

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
// Samplers
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindSampler(const FUniformID& id, FRawSamplerID sampler, u32 elementIndex) {
    Assert(id.Valid());
    Assert(sampler.Valid());

    auto& resource = Resource_<FSampler>(id);
    Assert(elementIndex < resource.Elements.Capacity);
    auto& element = resource.Elements[elementIndex];

    if (element.SamplerId != sampler || resource.Elements.Count <= elementIndex )
        ResetCachedId_();

    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);
    element.SamplerId = sampler;

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindSamplers(const FUniformID& id, TMemoryView<const FSamplerID> samplers) {
    STATIC_ASSERT(sizeof(FSamplerID) == sizeof(FRawSamplerID));
    return BindSamplers(id, samplers.Cast<const FRawSamplerID>());
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindSamplers(const FUniformID& id, TMemoryView<const FRawSamplerID> samplers) {
    Assert(id.Valid());

    auto& resource = Resource_<FSampler>(id);

    Assert(samplers.size() <= resource.Elements.Capacity);
    bool needUpdate = (samplers.size() != resource.Elements.Count);
    resource.Elements.Count = checked_cast<u16>(samplers.size());

    forrange(i, 0, samplers.size()) {
        Assert(samplers[i].Valid());

        auto& element = resource.Elements[i];
        needUpdate |= (element.SamplerId != samplers[i]);

        element.SamplerId = samplers[i];
    }

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
// Buffers
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffer(const FUniformID& id, FRawBufferID buffer, u32 elementIndex) {
    return BindBuffer(id, buffer, 0, UMax, elementIndex);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffer(const FUniformID& id, FRawBufferID buffer, size_t offset, size_t size, u32 elementIndex) {
    Assert(id.Valid());
    Assert(buffer.Valid());

    auto& resource = Resource_<FBuffer>(id);
    Assert(elementIndex < resource.Elements.Capacity);
    auto& element = resource.Elements[elementIndex];

    Assert_NoAssume( UMax == size || ( (size >= resource.StaticSize) &&
        (resource.ArrayStride == 0 || (size - resource.StaticSize) % resource.ArrayStride == 0)) );

    bool needUpdate = (element.BufferId != buffer || element.Size != size || resource.Elements.Count <= elementIndex);

    if (resource.DynamicOffsetIndex == FPipelineDesc::StaticOffset) {
        needUpdate |= (element.Offset != offset);
        element.Offset = offset;
    }
    else {
        Assert_NoAssume( offset >= element.Offset && offset - element.Offset < UMax );
        _dynamicData.LockExclusive()->DynamicOffsets()[resource.DynamicOffsetIndex + elementIndex] = checked_cast<u32>(offset - element.Offset);
    }

    if (needUpdate)
        ResetCachedId_();

    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);
    element.BufferId = buffer;
    element.Size = size;

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffers(const FUniformID& id, TMemoryView<const FBufferID> buffers) {
    STATIC_ASSERT(sizeof(FBufferID) == sizeof(FRawBufferID));
    return BindBuffers(id, buffers.Cast<const FRawBufferID>());
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindBuffers(const FUniformID& id, TMemoryView<const FRawBufferID> buffers) {
    Assert(id.Valid());

    auto& resource = Resource_<FBuffer>(id);

    Assert(buffers.size() <= resource.Elements.Capacity);
    bool needUpdate = (buffers.size() != resource.Elements.Count);
    resource.Elements.Count = checked_cast<u16>(buffers.size());

    constexpr size_t offset = 0;
    constexpr size_t size = UMax;
    forrange(i, 0, checked_cast<u32>(buffers.size())) {
        Assert(buffers[i].Valid());

        auto& element = resource.Elements[i];
        needUpdate |= (element.BufferId != buffers[i] || element.Size != size);

        if (resource.DynamicOffsetIndex == FPipelineDesc::StaticOffset) {
            needUpdate |= (element.Offset != offset);
            element.Offset = offset;
        }
        else {
            Assert_NoAssume( offset >= element.Offset && offset - element.Offset < UMax );
            _dynamicData.LockExclusive()->DynamicOffsets()[resource.DynamicOffsetIndex + i] = checked_cast<u32>(offset - element.Offset);
        }

        element.BufferId = buffers[i];
        element.Size = size;
    }

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::SetBufferBase(const FUniformID& id, size_t offset, u32 elementIndex) {
    Assert(id.Valid());

    auto& resource = Resource_<FBuffer>(id);
    Assert(elementIndex < resource.Elements.Capacity);

    bool needUpdate = (resource.Elements.Count <= elementIndex);
    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);

    auto& element = resource.Elements[elementIndex];
    if (resource.DynamicOffsetIndex != FPipelineDesc::StaticOffset) {
        needUpdate |= (element.Offset != offset);
        const auto exclusiveDynamicData = _dynamicData.LockExclusive();
        u32& dynOffset = exclusiveDynamicData->DynamicOffsets()[resource.DynamicOffsetIndex + elementIndex];
        dynOffset = static_cast<u32>((dynOffset + element.Offset - offset) & 0xFFFFFFFF_size_t);
        element.Offset = offset;
    }

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTexelBuffer(const FUniformID& id, FRawBufferID buffer, const FBufferViewDesc& desc, u32 elementIndex) {
    Assert(id.Valid());
    Assert(buffer.Valid());

    auto& resource = Resource_<FTexelBuffer>(id);
    Assert(elementIndex < resource.Elements.Capacity);

    bool needUpdate = (resource.Elements.Count <= elementIndex);
    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);

    auto& element = resource.Elements[elementIndex];
    needUpdate |= (element.BufferId != buffer || element.Desc != desc);

    element.BufferId = buffer;
    element.Desc = desc;

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
// RayTracing
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindRayTracingScene(const FUniformID& id, FRawRTSceneID scene, u32 elementIndex) {
    Assert(id.Valid());
    Assert(scene.Valid());

    auto& resource = Resource_<FRayTracingScene>(id);
    Assert(elementIndex < resource.Elements.Capacity);

    bool needUpdate = (resource.Elements.Count <= elementIndex);
    resource.Elements.Count = Max(checked_cast<u16>(elementIndex + 1), resource.Elements.Count);

    auto& element = resource.Elements[elementIndex];
    needUpdate |= (element.SceneId != scene);

    element.SceneId = scene;

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FPipelineResources::Initialize(FPipelineResources* pResources, FRawDescriptorSetLayoutID layoutId, const FDynamicData& data) {
    Assert(pResources);
    Assert(layoutId.Valid());

    const auto exclusiveData = pResources->_dynamicData.LockExclusive();

    pResources->ResetCachedId_();

    *exclusiveData = data;
    exclusiveData->LayoutId = layoutId;
}
//----------------------------------------------------------------------------
hash_t FPipelineResources::ComputeDynamicDataHash(const FDynamicData& dynamicData) NOEXCEPT {
    hash_t h{ hash_value(dynamicData.UniformsCount) };

    dynamicData.EachUniform([&h](const FUniformID& id, auto& res) {
        hash_combine(h, id, res);
    });

    return h;
}
//----------------------------------------------------------------------------
void FPipelineResources::CreateDynamicData(
    FDynamicData* pDynamicData,
    const FPipelineDesc::FUniformMap& uniforms,
    u32 resourceCount, u32 arrayElemCount, u32 bufferDynamicOffsetCount ) {
    Assert(pDynamicData);

    using self = FPipelineResources;
    constexpr u32 sizeofResources = static_cast<u32>(std::max({
        sizeof(self::FBuffer), sizeof(self::FImage), sizeof(self::FTexture), sizeof(self::FSampler), sizeof(self::FRayTracingScene)
    }));
    constexpr u32 sizeofElements = static_cast<u32>(std::max({
        sizeof(self::FBuffer::FElement), sizeof(self::FImage::FElement), sizeof(self::FTexture::FElement), sizeof(self::FSampler::FElement), sizeof(self::FRayTracingScene::FElement)
    }));

    const size_t requestedSize{
        sizeof(FUniform) * uniforms.size() +
        sizeof(u32) * bufferDynamicOffsetCount +
        static_cast<size_t>(sizeofResources) * resourceCount +
        static_cast<size_t>(sizeofElements) * arrayElemCount };

    pDynamicData->Storage.Resize_DiscardData(requestedSize);
    FRawMemory rawData = pDynamicData->Storage.MakeView();

    pDynamicData->UniformsOffset = checked_cast<u32>(rawData.data() - pDynamicData->Storage.data());
    pDynamicData->UniformsCount = checked_cast<u32>(uniforms.size());
    TMemoryView<FUniform> uniformData = rawData.Eat(sizeof(FUniform) * uniforms.size()).Cast<FUniform>();

    pDynamicData->DynamicOffsetsOffset = checked_cast<u32>(rawData.data() - pDynamicData->Storage.data());
    pDynamicData->DynamicOffsetsCount = bufferDynamicOffsetCount;
    const TMemoryView<u32> dynamicOffsetData = rawData.Eat(sizeof(u32) * bufferDynamicOffsetCount).Cast<u32>();
    Unused(dynamicOffsetData);

    u32 numDBO = 0;
    for (auto& it : uniforms) {
        const u16 elementCapacity = checked_cast<u16>(it.second.ArraySize ? it.second.ArraySize : MaxElementsInUnsizedDesc);
        const u16 elementCount = checked_cast<u16>(it.second.ArraySize);
        Assert(elementCapacity);

        FUniform* const pUniform = INPLACE_NEW(uniformData.Eat(1).data(), FUniform) { .Id = it.first };
        pUniform->RelativeOffset = checked_cast<u16>(rawData.data() - bit_cast<u8*>(pUniform));

        Meta::Visit(it.second.Data,
            [&](const FPipelineDesc::FTexture& tex) {
                pUniform->Type = FTexture::TypeId;
                const auto block = rawData.Eat(sizeof(FTexture) + sizeof(FTexture::FElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FTexture) {
                    it.second.Index,
                    tex.State,
                    tex.Type,
                    { elementCapacity, elementCount }
                };
            },
            [&](const FPipelineDesc::FSampler& ) {
                pUniform->Type = FSampler::TypeId;
                const auto block = rawData.Eat(sizeof(FSampler) + sizeof(FSampler::FElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FSampler) {
                    it.second.Index,
                    { elementCapacity, elementCount }
                };
            },
            [&](const FPipelineDesc::FSubpassInput& spi) {
                pUniform->Type = FImage::TypeId;
                const auto block = rawData.Eat(sizeof(FImage) + sizeof(FImage::FElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FImage) {
                    it.second.Index,
                    spi.State,
                    Default,
                    { elementCapacity, elementCount }
                };
            },
            [&](const FPipelineDesc::FImage& img) {
                pUniform->Type = FImage::TypeId;
                const auto block = rawData.Eat(sizeof(FImage) + sizeof(FImage::FElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FImage) {
                    it.second.Index,
                    img.State,
                    img.Type,
                    { elementCapacity, elementCount }
                };
            },
            [&](const FPipelineDesc::FUniformBuffer& ubuf) {
                pUniform->Type = FBuffer::TypeId;
                const auto block = rawData.Eat(sizeof(FBuffer) + sizeof(FBuffer::FElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FBuffer) {
                    it.second.Index,
                    ubuf.State,
                    ubuf.DynamicOffsetIndex,
                    ubuf.Size,
                    0, // ArrayStride
                    { elementCapacity, elementCount }
                };
                if (ubuf.DynamicOffsetIndex != FPipelineDesc::StaticOffset)
                    numDBO += elementCapacity;
            },
            [&](const FPipelineDesc::FStorageBuffer& sbuf) {
                pUniform->Type = FBuffer::TypeId;
                const auto block = rawData.Eat(sizeof(FBuffer) + sizeof(FBuffer::FElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FBuffer) {
                    it.second.Index,
                    sbuf.State,
                    sbuf.DynamicOffsetIndex,
                    sbuf.StaticSize,
                    sbuf.ArrayStride,
                    { elementCapacity, elementCount }
                };
                if (sbuf.DynamicOffsetIndex != FPipelineDesc::StaticOffset)
                    numDBO += elementCapacity;
            },
            [&](const FPipelineDesc::FRayTracingScene& ) {
                pUniform->Type = FRayTracingScene::TypeId;
                const auto block = rawData.Eat(sizeof(FRayTracingScene) + sizeof(FRayTracingScene::FElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FRayTracingScene) {
                    it.second.Index,
                    { elementCapacity, elementCount }
                };
            });
    }
    Assert_NoAssume(uniformData.empty());
    AssertRelease(numDBO == bufferDynamicOffsetCount);
    Unused(numDBO);
}
//----------------------------------------------------------------------------
bool FPipelineResources::CompareDynamicData(const FDynamicData& lhs, const FDynamicData& rhs) NOEXCEPT {
    if (lhs.LayoutId != rhs.LayoutId || lhs.UniformsCount != rhs.UniformsCount)
        return false;

    const TMemoryView<const FUniform> lhsUniforms = lhs.Uniforms();
    const TMemoryView<const FUniform> rhsUniforms = rhs.Uniforms();

    forrange(i, 0, lhs.UniformsCount) {
        const FUniform& a = lhsUniforms[i];
        const FUniform& b = rhsUniforms[i];
        if (a.Id != b.Id || a.Type != b.Type)
            return false;

        bool equals = true;
        switch (a.Type) {
        case EDescriptorType::Unknown: break;
        case EDescriptorType::Buffer: equals = (a.Get<FBuffer>() == b.Get<FBuffer>()); break;
        case EDescriptorType::TexelBuffer: equals = (a.Get<FTexelBuffer>() == b.Get<FTexelBuffer>()); break;
        case EDescriptorType::SubpassInput:
        case EDescriptorType::Image: equals = (a.Get<FImage>() == b.Get<FImage>()); break;
        case EDescriptorType::Texture: equals = (a.Get<FTexture>() == b.Get<FTexture>()); break;
        case EDescriptorType::Sampler: equals = (a.Get<FSampler>() == b.Get<FSampler>()); break;
        case EDescriptorType::RayTracingScene: equals = (a.Get<FRayTracingScene>() == b.Get<FRayTracingScene>()); break;
        }

        if (not equals)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
auto FPipelineResources::CloneDynamicData(const FPipelineResources& desc) -> FDynamicData {
    return FDynamicData{ *desc._dynamicData.LockShared() };
}
//----------------------------------------------------------------------------
auto FPipelineResources::StealDynamicData(FPipelineResources& desc) NOEXCEPT -> FDynamicData&& {
    return std::move( *desc._dynamicData.LockExclusive() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
