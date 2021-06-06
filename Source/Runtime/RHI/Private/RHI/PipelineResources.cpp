#include "stdafx.h"

#include "RHI/PipelineResources.h"

#include "RHI/ResourceId.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPipelineResources::~FPipelineResources() {
    WRITESCOPELOCK(_rwlock);
}
//----------------------------------------------------------------------------
FPipelineResources::FPipelineResources(const FPipelineResources& other)
:   _dynamicData(other._dynamicData)
,   _allowEmptyResources(other._allowEmptyResources) {
    READSCOPELOCK(_rwlock);
    STATIC_ASSERT(FCachedID_::is_always_lock_free);
    SetCachedId_(other.CachedId_());
}
//----------------------------------------------------------------------------
template <typename T>
T& FPipelineResources::Resource_Unlocked_(const FUniformID& id) {
    Assert(id.Valid());

    const auto uniforms = _dynamicData.Uniforms();
    const auto it = std::lower_bound(uniforms.begin(), uniforms.end(), id);
    if (uniforms.end() != it && *it == id) {
        Assert_NoAssume(it->Type == T::TypeId);
        return *_dynamicData.Storage.MakeView().CutStartingAt(it->Offset).Peek<T>();
    }

    LOG(RHI, Fatal, L"failed to find uniform in pipeline resource with id: {0}", id);
    AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T>
bool FPipelineResources::HasResource_Unlocked_(const FUniformID& id) const {
    Assert(id.Valid());

    const auto uniforms = _dynamicData.Uniforms();
    const auto it = std::lower_bound(uniforms.begin(), uniforms.end(), id);
    if (uniforms.end() != it && *it == id)
        return (it->Type == T::TypeId);

    return false;
}
//----------------------------------------------------------------------------
bool FPipelineResources::HasImage(const FUniformID& id) const {
    READSCOPELOCK(_rwlock);
    return HasResource_Unlocked_<FImage>(id);
}
bool FPipelineResources::HasSampler(const FUniformID& id) const {
    READSCOPELOCK(_rwlock);
    return HasResource_Unlocked_<FSampler>(id);
}
bool FPipelineResources::HasTexture(const FUniformID& id) const {
    READSCOPELOCK(_rwlock);
    return HasResource_Unlocked_<FTexture>(id);
}
bool FPipelineResources::HasBuffer(const FUniformID& id) const {
    READSCOPELOCK(_rwlock);
    return HasResource_Unlocked_<FBuffer>(id);
}
bool FPipelineResources::HasTexelBuffer(const FUniformID& id) const {
    READSCOPELOCK(_rwlock);
    return HasResource_Unlocked_<FTexelBuffer>(id);
}
bool FPipelineResources::HasRayTracingScene(const FUniformID& id) const {
    READSCOPELOCK(_rwlock);
    return HasResource_Unlocked_<FRayTracingScene>(id);
}
//----------------------------------------------------------------------------
void FPipelineResources::Reset(const FUniformID& uniform) {
    Assert(uniform.Valid());
    WRITESCOPELOCK(_rwlock);

    const auto uniforms = _dynamicData.Uniforms();
    const auto it = std::lower_bound(uniforms.begin(), uniforms.end(), uniform);
    Assert(uniforms.end() != it);
    Assert(*it == uniform);

    const FRawMemory rawData{ _dynamicData.Storage.MakeView().CutStartingAt(it->Offset) };

    switch (it->Type) {
    case EDescriptorType::Unknown: break;
    case EDescriptorType::Buffer: rawData.Peek<FBuffer>()->ElementCount = 0; break;
    case EDescriptorType::TexelBuffer: rawData.Peek<FTexelBuffer>()->ElementCount = 0; break;
    case EDescriptorType::SubpassInput:
    case EDescriptorType::Image: rawData.Peek<FImage>()->ElementCount = 0; break;
    case EDescriptorType::Texture: rawData.Peek<FTexture>()->ElementCount = 0; break;
    case EDescriptorType::Sampler: rawData.Peek<FSampler>()->ElementCount = 0; break;
    case EDescriptorType::RayTracingScene: rawData.Peek<FRayTracingScene>()->ElementCount = 0; break;
    }

    ResetCachedId_();
}
//----------------------------------------------------------------------------
void FPipelineResources::ResetAll() {
    WRITESCOPELOCK(_rwlock);

    _dynamicData.EachUniform([](auto&, auto& resource) {
        resource.ElementCount = 0;
    });

    ResetCachedId_();
}
//----------------------------------------------------------------------------
// Images
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImage(const FUniformID& id, FRawImageID image, u32 elementIndex) {
    Assert(id.Valid());
    Assert(image.Valid());
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FImage>(id);
    Assert(elementIndex < resource.ElementCapacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.Desc.has_value() || resource.ElementCount <= elementIndex )
        ResetCachedId_();

    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);
    element.ImageId = image;
    element.Desc.reset();

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindImage(const FUniformID& id, FRawImageID image, const FImageViewDesc& desc, u32 elementIndex) {
    Assert(id.Valid());
    Assert(image.Valid());
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FImage>(id);
    Assert(elementIndex < resource.ElementCapacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.Desc != desc || resource.ElementCount <= elementIndex )
        ResetCachedId_();

    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);
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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FImage>(id);

    Assert(images.size() <= resource.ElementCapacity);
    bool needUpdate = (images.size() != resource.ElementCount);
    resource.ElementCount = checked_cast<u16>(images.size());

    forrange(i, 0, images.size()) {
        Assert(images[i].Valid());

        FImageElement& element = resource.Elements[i];
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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FTexture>(id);
    Assert(elementIndex < resource.ElementCapacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.SamplerId != sampler || element.Desc.has_value() || resource.ElementCount <= elementIndex )
        ResetCachedId_();

    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);
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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FTexture>(id);
    Assert(elementIndex < resource.ElementCapacity);
    auto& element = resource.Elements[elementIndex];

    if (element.ImageId != image || element.SamplerId != sampler || element.Desc != desc || resource.ElementCount <= elementIndex )
        ResetCachedId_();

    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);
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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FTexture>(id);

    Assert(images.size() <= resource.ElementCapacity);
    bool needUpdate = (images.size() != resource.ElementCount);
    resource.ElementCount = checked_cast<u16>(images.size());

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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FSampler>(id);
    Assert(elementIndex < resource.ElementCapacity);
    auto& element = resource.Elements[elementIndex];

    if (element.SamplerId != sampler || resource.ElementCount <= elementIndex )
        ResetCachedId_();

    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);
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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FSampler>(id);

    Assert(samplers.size() <= resource.ElementCapacity);
    bool needUpdate = (samplers.size() != resource.ElementCount);
    resource.ElementCount = checked_cast<u16>(samplers.size());

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
FPipelineResources& FPipelineResources::BindBuffer(const FUniformID& id, FRawBufferID buffer, u32 offset, u32 size, u32 elementIndex) {
    Assert(id.Valid());
    Assert(buffer.Valid());
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FBuffer>(id);
    Assert(elementIndex < resource.ElementCapacity);
    auto& element = resource.Elements[elementIndex];

    Assert_NoAssume( UMax == size || ( (size >= resource.StaticSize) &&
        (resource.ArrayStride == 0 || (size - resource.StaticSize) % resource.ArrayStride == 0)) );

    bool needUpdate = (element.BufferId != buffer || element.Size != size || resource.ElementCount <= elementIndex);

    if (resource.DynamicOffsetIndex == FPipelineDesc::StaticOffset) {
        needUpdate |= (element.Offset != offset);
        element.Offset = offset;
    }
    else {
        Assert_NoAssume( offset >= element.Offset && offset - element.Offset < UMax );
        DynamicOffset_(resource.DynamicOffsetIndex + elementIndex) = checked_cast<u32>(offset - element.Offset);
    }

    if (needUpdate)
        ResetCachedId_();

    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);
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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FBuffer>(id);

    Assert(buffers.size() <= resource.ElementCapacity);
    bool needUpdate = (buffers.size() != resource.ElementCount);
    resource.ElementCount = checked_cast<u16>(buffers.size());

    constexpr u32 offset = 0;
    constexpr u32 size = UMax;
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
            DynamicOffset_(resource.DynamicOffsetIndex + i) = checked_cast<u32>(offset - element.Offset);
        }

        element.BufferId = buffers[i];
        element.Size = size;
    }

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::SetBufferBase(const FUniformID& id, u32 offset, u32 elementIndex) {
    Assert(id.Valid());
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FBuffer>(id);
    Assert(elementIndex < resource.ElementCapacity);

    bool needUpdate = (resource.ElementCount <= elementIndex);
    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);

    auto& element = resource.Elements[elementIndex];
    if (resource.DynamicOffsetIndex != FPipelineDesc::StaticOffset) {
        needUpdate |= (element.Offset != offset);
        u32& dynOffset = DynamicOffset_(resource.DynamicOffsetIndex + elementIndex);
        dynOffset = static_cast<u32>((dynOffset + element.Offset - offset) & 0xFFFFFFFFull);
        element.Offset = offset;
    }

    if (needUpdate)
        ResetCachedId_();

    return (*this);
}
//----------------------------------------------------------------------------
FPipelineResources& FPipelineResources::BindTexelBuffer(const FUniformID& name, FRawBufferID buffer, const FBufferViewDesc& desc, u32 elementIndex) {
    Assert(name.Valid());
    Assert(buffer.Valid());
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FTexelBuffer>(name);
    Assert(elementIndex < resource.ElementCapacity);

    bool needUpdate = (resource.ElementCount <= elementIndex);
    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);

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
    WRITESCOPELOCK(_rwlock);

    auto& resource = Resource_Unlocked_<FRayTracingScene>(id);
    Assert(elementIndex < resource.ElementCapacity);

    bool needUpdate = (resource.ElementCount <= elementIndex);
    resource.ElementCount = Max(checked_cast<u16>(elementIndex + 1), resource.ElementCount);

    auto& element = resource.Elements[elementIndex];
    needUpdate |= (element.SceneId != scene);

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

    pResources->ResetCachedId_();
    pResources->_dynamicData = data;
    pResources->_dynamicData.LayoutId = layoutId;
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
namespace {
// https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts> struct overloaded_ : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded_(Ts...) -> overloaded_<Ts...>;
}
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
        sizeof(self::FBufferElement), sizeof(self::FImageElement), sizeof(self::FTextureElement), sizeof(self::FSamplerElement), sizeof(self::FRayTracingSceneElement)
    }));

    const size_t requestedSize{
        sizeof(FUniform) * uniforms.size() +
        sizeof(u32) * bufferDynamicOffsetCount +
        sizeofResources * resourceCount +
        sizeofElements * arrayElemCount };

    pDynamicData->Storage.Resize_DiscardData(requestedSize);
    FRawMemory rawData = pDynamicData->Storage.MakeView();

    pDynamicData->UniformsOffset = checked_cast<u32>(rawData.data() - pDynamicData->Storage.data());
    pDynamicData->UniformsCount = checked_cast<u32>(uniforms.size());
    TMemoryView<FUniform> uniformData = rawData.Eat(sizeof(FUniform) * uniforms.size()).Cast<FUniform>();

    pDynamicData->DynamicOffsetsOffset = checked_cast<u32>(rawData.data() - pDynamicData->Storage.data());
    pDynamicData->DynamicOffsetsCount = bufferDynamicOffsetCount;
    const TMemoryView<u32> dynamicOffsetData = rawData.Eat(sizeof(u32) * bufferDynamicOffsetCount).Cast<u32>();
    UNUSED(dynamicOffsetData);

    u32 numDBO = 0;
    for (auto& it : uniforms) {
        const u16 elementCapacity = checked_cast<u16>(it.second.ArraySize ? it.second.ArraySize : MaxElementsInUnsizedDesc);
        const u16 elementCount = checked_cast<u16>(it.second.ArraySize);
        Assert(elementCapacity);

        FUniform& current = uniformData.Eat(1).front();
        current.Id = it.first;

        std::visit(overloaded_{
            [&](const FPipelineDesc::FTexture& tex) {
                current.Type = FTexture::TypeId;
                current.Offset = checked_cast<u16>(rawData.data() - pDynamicData->Storage.data());
                const auto block = rawData.Eat(sizeof(FTexture) + sizeof(FTextureElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FTexture) {
                    it.second.Index,
                    elementCapacity,
                    elementCount,
                    tex.State
                };
            },
            [&](const FPipelineDesc::FSampler& ) {
                current.Type = FSampler::TypeId;
                current.Offset = checked_cast<u16>(rawData.data() - pDynamicData->Storage.data());
                const auto block = rawData.Eat(sizeof(FSampler) + sizeof(FSamplerElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FSampler) {
                    it.second.Index,
                    elementCapacity,
                    elementCount
                };
            },
            [&](const FPipelineDesc::FSubpassInput& spi) {
                current.Type = FImage::TypeId;
                current.Offset = checked_cast<u16>(rawData.data() - pDynamicData->Storage.data());
                const auto block = rawData.Eat(sizeof(FImage) + sizeof(FImageElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FImage) {
                    it.second.Index,
                    elementCapacity,
                    elementCount,
                    spi.State
                };
            },
            [&](const FPipelineDesc::FImage& img) {
                current.Type = FImage::TypeId;
                current.Offset = checked_cast<u16>(rawData.data() - pDynamicData->Storage.data());
                const auto block = rawData.Eat(sizeof(FImage) + sizeof(FImageElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FImage) {
                    it.second.Index,
                    elementCapacity,
                    elementCount,
                    img.State
                };
            },
            [&](const FPipelineDesc::FUniformBuffer& ubuf) {
                current.Type = FBuffer::TypeId;
                current.Offset = checked_cast<u16>(rawData.data() - pDynamicData->Storage.data());
                const auto block = rawData.Eat(sizeof(FBuffer) + sizeof(FBufferElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FBuffer) {
                    it.second.Index,
                    elementCapacity,
                    elementCount,
                    ubuf.State,
                    ubuf.DynamicOffsetIndex,
                    ubuf.Size,
                    0// ArrayStride
                };
                if (ubuf.DynamicOffsetIndex != FPipelineDesc::StaticOffset)
                    numDBO += elementCapacity;
            },
            [&](const FPipelineDesc::FStorageBuffer& sbuf) {
                current.Type = FBuffer::TypeId;
                current.Offset = checked_cast<u16>(rawData.data() - pDynamicData->Storage.data());
                const auto block = rawData.Eat(sizeof(FBuffer) + sizeof(FBufferElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FBuffer) {
                    it.second.Index,
                    elementCapacity,
                    elementCount,
                    sbuf.State,
                    sbuf.DynamicOffsetIndex,
                    sbuf.StaticSize,
                    sbuf.ArrayStride
                };
                if (sbuf.DynamicOffsetIndex != FPipelineDesc::StaticOffset)
                    numDBO += elementCapacity;
            },
            [&](const FPipelineDesc::FRayTracingScene& ) {
                current.Type = FRayTracingScene::TypeId;
                current.Offset = checked_cast<u16>(rawData.data() - pDynamicData->Storage.data());
                const auto block = rawData.Eat(sizeof(FRayTracingScene) + sizeof(FRayTracingSceneElement) * (elementCapacity - 1));
                INPLACE_NEW(block.data(), FRayTracingScene) {
                    it.second.Index,
                    elementCapacity,
                    elementCount
                };
            }
        },  it.second.Data );
    }
    Assert_NoAssume(uniformData.empty());
    AssertRelease(numDBO == bufferDynamicOffsetCount);
    UNUSED(numDBO);

    const auto writtenUniforms = pDynamicData->Storage.MakeView().SubRange(pDynamicData->UniformsOffset, pDynamicData->UniformsCount * sizeof(FUniform)).Cast<FUniform>();
    std::sort(writtenUniforms.begin(),  writtenUniforms.end(), [](const FUniform& lhs, const FUniform& rhs) {
        return (lhs.Id < rhs.Id); // for binary search later (std::lower_bound)
    });
}
//----------------------------------------------------------------------------
bool FPipelineResources::CompareDynamicData(const FDynamicData& lhs, const FDynamicData& rhs) NOEXCEPT {
    if (lhs.LayoutId != rhs.LayoutId || lhs.UniformsCount != rhs.UniformsCount)
        return false;

    forrange(i, 0, lhs.UniformsCount) {
        const auto& lhsUni = lhs.Uniforms()[i];
        const auto& rhsUni = rhs.Uniforms()[i];
        if (lhsUni.Id != rhsUni.Id || lhsUni.Type != rhsUni.Type)
            return false;

        const FRawMemoryConst lhsData{ lhs.Storage.MakeView().CutStartingAt(lhsUni.Offset) };
        const FRawMemoryConst rhsData{ rhs.Storage.MakeView().CutStartingAt(lhsUni.Offset) };

        bool equals = true;
        switch (lhsUni.Type) {
        case EDescriptorType::Unknown: break;
        case EDescriptorType::Buffer: equals = (*lhsData.Peek<FBuffer>() == *rhsData.Peek<FBuffer>()); break;
        case EDescriptorType::TexelBuffer: equals = (*lhsData.Peek<FTexelBuffer>() == *rhsData.Peek<FTexelBuffer>()); break;
        case EDescriptorType::SubpassInput:
        case EDescriptorType::Image: equals = (*lhsData.Peek<FImage>() == *rhsData.Peek<FImage>()); break;
        case EDescriptorType::Texture: equals = (*lhsData.Peek<FTexture>() == *rhsData.Peek<FTexture>()); break;
        case EDescriptorType::Sampler: equals = (*lhsData.Peek<FSampler>() == *rhsData.Peek<FSampler>()); break;
        case EDescriptorType::RayTracingScene: equals = (*lhsData.Peek<FRayTracingScene>() == *rhsData.Peek<FRayTracingScene>()); break;
        }

        if (not equals)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
