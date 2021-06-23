#pragma once

#include "RHI_fwd.h"

#include "RHI/BindingIndex.h"
#include "RHI/BufferDesc.h"
#include "RHI/ImageDesc.h"
#include "RHI/PipelineDesc.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceState.h"

#include "Container/AssociativeVector.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Meta/Optional.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FPipelineResources : public FRefCountable {
public:
    enum class EDescriptorType : u16 {
        Unknown = 0,
        Buffer,
        TexelBuffer,
        Image,
        Texture,
        SubpassInput,
        Sampler,
        RayTracingScene,
    };

    struct FBaseResource {
        FBindingIndex Index;
        const u16 ElementCapacity;
        u16 ElementCount;

        FBaseResource(FBindingIndex index, u16 elementCapacity, u16 elementCount) NOEXCEPT
        :   Index(index), ElementCapacity(elementCapacity), ElementCount(elementCount)
        {}

        bool operator ==(const FBaseResource& other) const NOEXCEPT {
            return (Index == other.Index && ElementCapacity == other.ElementCapacity && ElementCount == other.ElementCount);
        }
        bool operator !=(const FBaseResource& other) const NOEXCEPT { return (not operator ==(other)); }

        friend hash_t hash_value(const FBaseResource& res) {
            return hash_tuple(res.Index, res.ElementCount);
        }
    };
    struct FBaseResourceWithState : FBaseResource {
        EResourceState State;

        FBaseResourceWithState(FBindingIndex index, u16 elementCapacity, u16 elementCount, EResourceState state) NOEXCEPT
        :   FBaseResource(index, elementCapacity, elementCount), State(state)
        {}

        bool operator ==(const FBaseResourceWithState& other) const NOEXCEPT {
            return (static_cast<const FBaseResource&>(*this) == other && State == other.State);
        }
        bool operator !=(const FBaseResourceWithState& other) const NOEXCEPT { return (not operator ==(other)); }

        friend hash_t hash_value(const FBaseResourceWithState& res) {
            return hash_tuple(static_cast<const FBaseResource&>(res), res.State);
        }
    };
    struct FBaseResourceWithDynamicOffsets : FBaseResourceWithState {
        u32 DynamicOffsetIndex;
        u32 StaticSize;
        u32 ArrayStride;

        FBaseResourceWithDynamicOffsets(FBindingIndex index, u16 elementCapacity, u16 elementCount, EResourceState state,
            u32 dynamicOffsetIndex, u32 staticSize, u32 arrayStride ) NOEXCEPT
        :   FBaseResourceWithState(index, elementCapacity, elementCount, state)
        ,   DynamicOffsetIndex(dynamicOffsetIndex), StaticSize(staticSize), ArrayStride(arrayStride)
        {}

        bool operator ==(const FBaseResourceWithDynamicOffsets& other) const NOEXCEPT {
            return (static_cast<const FBaseResourceWithState&>(*this) == other &&
                DynamicOffsetIndex == other.DynamicOffsetIndex && StaticSize == other.StaticSize && ArrayStride == other.ArrayStride );
        }
        bool operator !=(const FBaseResourceWithDynamicOffsets& other) const NOEXCEPT { return (not operator ==(other)); }

        friend hash_t hash_value(const FBaseResourceWithDynamicOffsets& res) {
            return hash_tuple(static_cast<const FBaseResourceWithState&>(res), res.DynamicOffsetIndex, res.StaticSize, res.ArrayStride);
        }
    };

    template <EDescriptorType _TypeId, typename _Element, class _BaseResource = FBaseResource >
    struct TResource : _BaseResource {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, _TypeId);

        using base_type = _BaseResource;
        using base_type::base_type;
        using element_type = _Element;

        element_type Elements[1];

        TMemoryView<element_type> MakeView() { return { Elements, base_type::ElementCount }; }
        TMemoryView<const element_type> MakeView() const { return { Elements, base_type::ElementCount }; }

        bool operator ==(const TResource& other) const {
            if (static_cast<const base_type&>(*this) != other ||
                other.ElementCount != this->ElementCount )
                return false;
            return std::equal(
                Elements, Elements + this->ElementCount,
                other.Elements, other.Elements + other.ElementCount );
        }
        bool operator !=(const TResource& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const TResource& res) {
            hash_t hash{ hash_value(static_cast<const base_type&>(res)) };
            hash_range(hash, res.Elements, res.ElementCount);
            return hash;
        }
    };
    template <EDescriptorType _TypeId, typename _Element>
    using TResourceWithState = TResource<_TypeId, _Element, FBaseResourceWithState>;

    // FBuffer
    struct FBufferElement {
        FRawBufferID BufferId;
        u32 Offset;
        u32 Size;

        bool operator ==(const FBufferElement& other) const { return (BufferId == other.BufferId && Offset == other.Offset && Size == other.Size); }
        bool operator !=(const FBufferElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FBufferElement& res) { return hash_tuple(res.BufferId, res.Offset, res.Size); }
    };
    using FBuffer = TResource<EDescriptorType::TexelBuffer, FBufferElement, FBaseResourceWithDynamicOffsets>;

    // FTexelBuffer
    struct FTexelBufferElement {
        FRawBufferID BufferId;
        FBufferViewDesc Desc;

        bool operator ==(const FTexelBufferElement& other) const { return (BufferId == other.BufferId && Desc == other.Desc); }
        bool operator !=(const FTexelBufferElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FTexelBufferElement& res) { return hash_tuple(res.BufferId, res.Desc); }
    };
    using FTexelBuffer = TResourceWithState<EDescriptorType::TexelBuffer, FTexelBufferElement>;

    // FImage
    struct FImageElement {
        FRawImageID ImageId;
        Meta::TOptional<FImageViewDesc> Desc;

        bool operator ==(const FImageElement& other) const { return (ImageId == other.ImageId && Desc == other.Desc); }
        bool operator !=(const FImageElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FImageElement& res) { return hash_tuple(res.ImageId, res.Desc); }
    };
    using FImage = TResourceWithState<EDescriptorType::Image, FImageElement>;

    // FTexture
    struct FTextureElement {
        FRawImageID ImageId;
        FRawSamplerID SamplerId;
        Meta::TOptional<FImageViewDesc> Desc;

        bool operator ==(const FTextureElement& other) const { return (ImageId == other.ImageId && SamplerId == other.SamplerId && Desc == other.Desc); }
        bool operator !=(const FTextureElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FTextureElement& res) { return hash_tuple(res.ImageId, res.SamplerId, res.Desc); }
    };
    using FTexture = TResourceWithState<EDescriptorType::Texture, FTextureElement>;

    // FSampler
    struct FSamplerElement {
        FRawSamplerID SamplerId;

        bool operator ==(const FSamplerElement& other) const { return (SamplerId == other.SamplerId); }
        bool operator !=(const FSamplerElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FSamplerElement& res) { return hash_value(res.SamplerId); }
    };
    using FSampler = TResource<EDescriptorType::Sampler, FSamplerElement>;

    // FRayTracingScene
    struct FRayTracingSceneElement {
        FRawRTSceneID SceneId;

        bool operator ==(const FRayTracingSceneElement& other) const { return (SceneId == other.SceneId); }
        bool operator !=(const FRayTracingSceneElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FRayTracingSceneElement& res) { return hash_value(res.SceneId); }
    };
    using FRayTracingScene = TResource<EDescriptorType::RayTracingScene, FRayTracingSceneElement>;

    // FUniform
    struct FUniform {
        FUniformID Id;
        EDescriptorType Type{ Default };
        u16 Offset{ 0 };

        template <typename T>
        T& Get(void* p) const {
            Assert(T::TypeId == Type);
            return *reinterpret_cast<T*>(static_cast<u8*>(p) + Offset);
        }
        template <typename T>
        const T& Get(const void* p) const {
            return Get<T>(const_cast<void*>(p));
        }

        bool operator ==(const FUniformID& id) const { return (id == Id); }
        bool operator !=(const FUniformID& id) const { return (id != Id); }

        bool operator < (const FUniformID& id) const { return (id <  Id); }
        bool operator >=(const FUniformID& id) const { return (id >= Id); }
    };

    // FDynamicData
    using FDynamicDataStorage = RAWSTORAGE(RHIDynamicData, u8);
    struct FDynamicData {
        FRawDescriptorSetLayoutID LayoutId;
        u32 UniformsCount{ 0 };
        u32 UniformsOffset{ 0 };
        u32 DynamicOffsetsCount{ 0 };
        u32 DynamicOffsetsOffset{ 0 };
        FDynamicDataStorage Storage;

        FDynamicData() = default;

        TMemoryView<FUniform> Uniforms() { return Storage.SubRange(UniformsOffset, UniformsCount * sizeof(FUniform)).Cast<FUniform>(); }
        TMemoryView<const FUniform> Uniforms() const { return const_cast<FDynamicData*>(this)->Uniforms(); }

        template <typename _Each>
        void EachUniform(_Each&& each);
        template <typename _Each>
        void EachUniform(_Each&& each) const { const_cast<FDynamicData*>(this)->EachUniform(each); }

        TMemoryView<u32> DynamicOffsets() { return { reinterpret_cast<u32*>(reinterpret_cast<u8*>(this) + DynamicOffsetsOffset), DynamicOffsetsCount }; }
        TMemoryView<const u32> DynamicOffsets() const { return { reinterpret_cast<const u32*>(reinterpret_cast<const u8*>(this) + DynamicOffsetsOffset), DynamicOffsetsCount }; }

        bool operator ==(const FDynamicData& other) const { return CompareDynamicData(*this, other); }
        bool operator !=(const FDynamicData& other) const { return (not operator ==(other)); }
    };

    FPipelineResources() NOEXCEPT { ResetCachedId_(); }
    ~FPipelineResources();

    FPipelineResources(const FPipelineResources& other);
    FPipelineResources(FPipelineResources&& rvalue) NOEXCEPT
    :   _dynamicData(std::move(rvalue._dynamicData))
    ,   _allowEmptyResources(rvalue._allowEmptyResources) {
        SetCachedId_(rvalue.CachedId_());
        rvalue.SetCachedId_(FRawPipelineResourcesID{0});
    }

    FRawDescriptorSetLayoutID Layout() const { READSCOPELOCK(_rwlock); return (_dynamicData.LayoutId); }
    TMemoryView<const u32> DynamicOffsets() const { READSCOPELOCK(_rwlock); return _dynamicData.DynamicOffsets(); }

    bool AllowEmptyResources() const { READSCOPELOCK(_rwlock); return _allowEmptyResources; }
    void SetAllowEmptyResources(bool value) { WRITESCOPELOCK(_rwlock); _allowEmptyResources = value; }

    bool HasImage(const FUniformID& id) const;
    bool HasSampler(const FUniformID& id) const;
    bool HasTexture(const FUniformID& id) const;
    bool HasBuffer(const FUniformID& id) const;
    bool HasTexelBuffer(const FUniformID& id) const;
    bool HasRayTracingScene(const FUniformID& id) const;

    FPipelineResources& BindImage(const FUniformID& id, FRawImageID image, u32 elementIndex = 0);
    FPipelineResources& BindImage(const FUniformID& id, FRawImageID image, const FImageViewDesc& desc, u32 elementIndex = 0);
    FPipelineResources& BindImages(const FUniformID& id, TMemoryView<const FImageID> images);
    FPipelineResources& BindImages(const FUniformID& id, TMemoryView<const FRawImageID> images);

    FPipelineResources& BindTexture(const FUniformID& id, FRawImageID image, FRawSamplerID sampler, u32 elementIndex = 0);
    FPipelineResources& BindTexture(const FUniformID& id, FRawImageID image, FRawSamplerID sampler, const FImageViewDesc& desc, u32 elementIndex = 0);
    FPipelineResources& BindTextures(const FUniformID& id, TMemoryView<const FImageID> images, FRawSamplerID sampler);
    FPipelineResources& BindTextures(const FUniformID& id, TMemoryView<const FRawImageID> images, FRawSamplerID sampler);

    FPipelineResources& BindSampler(const FUniformID& id, FRawSamplerID sampler, u32 elementIndex = 0);
    FPipelineResources& BindSamplers(const FUniformID& id, TMemoryView<const FSamplerID> samplers);
    FPipelineResources& BindSamplers(const FUniformID& id, TMemoryView<const FRawSamplerID> samplers);

    FPipelineResources& BindBuffer(const FUniformID& id, FRawBufferID buffer, u32 elementIndex = 0);
    FPipelineResources& BindBuffer(const FUniformID& id, FRawBufferID buffer, u32 offset, u32 size, u32 elementIndex = 0);
    FPipelineResources& BindBuffers(const FUniformID& id, TMemoryView<const FBufferID> buffers);
    FPipelineResources& BindBuffers(const FUniformID& id, TMemoryView<const FRawBufferID> buffers);

    FPipelineResources& SetBufferBase(const FUniformID& id, u32 offset, u32 elementIndex = 0);

    FPipelineResources& BindTexelBuffer(const FUniformID& name, FRawBufferID buffer, const FBufferViewDesc& desc, u32 elementIndex = 0);
    FPipelineResources& BindRayTracingScene(const FUniformID& id, FRawRTSceneID scene, u32 elementIndex = 0);

    void Reset(const FUniformID& uniform);
    void ResetAll();

    static void Initialize(FPipelineResources* pResources, FRawDescriptorSetLayoutID layoutId, const FDynamicData& data);

    static void CreateDynamicData(
        FDynamicData* pDynamicData,
        const FPipelineDesc::FUniformMap& uniforms,
        u32 resourceCount, u32 arrayElemCount, u32 bufferDynamicOffsetCount );
    static hash_t ComputeDynamicDataHash(const FDynamicData& dynamicData) NOEXCEPT;
    static bool CompareDynamicData(const FDynamicData& lhs, const FDynamicData& rhs) NOEXCEPT;

    static FRawPipelineResourcesID Cached(const FPipelineResources& resource) { return resource.CachedId_(); }
    static void SetCached(const FPipelineResources& resource, FRawPipelineResourcesID id) { resource.SetCachedId_(id); }

private:
    using FCachedID_ = std::atomic< FRawPipelineResourcesID::FPackedData >;

    FReadWriteLock _rwlock;
    mutable FCachedID_ _cachedId;
    FDynamicData _dynamicData;
    bool _allowEmptyResources{ false };

    FRawPipelineResourcesID CachedId_() const { return FRawPipelineResourcesID(_cachedId.load(std::memory_order_acquire)); }
    void SetCachedId_(FRawPipelineResourcesID id) const { _cachedId.store(id.Packed, std::memory_order_relaxed); }
    void ResetCachedId_() const { _cachedId.store(UMax, std::memory_order_relaxed); }

    u32& DynamicOffset_(u32 i) { return _dynamicData.DynamicOffsets()[i]; }

    template <typename T>
    T& Resource_Unlocked_(const FUniformID& id);
    template <typename T>
    bool HasResource_Unlocked_(const FUniformID& id) const;
};
//----------------------------------------------------------------------------
using FPipelineResourceSet = TFixedSizeAssociativeVector<
    FDescriptorSetID, PCPipelineResources,
    MaxDescriptorSets >;
//----------------------------------------------------------------------------
template <typename _Each>
void FPipelineResources::FDynamicData::EachUniform(_Each&& each) {
    for (const FUniform& uni : Uniforms()) {
        FRawMemory rawData = Storage.MakeView().CutStartingAt(uni.Offset);

        switch (uni.Type) {
        case EDescriptorType::Unknown: break;
        case EDescriptorType::Buffer: each(uni.Id, *rawData.Peek<FBuffer>()); break;
        case EDescriptorType::TexelBuffer: each(uni.Id, *rawData.Peek<FTexelBuffer>()); break;
        case EDescriptorType::SubpassInput:
        case EDescriptorType::Image: each(uni.Id, *rawData.Peek<FImage>()); break;
        case EDescriptorType::Texture: each(uni.Id, *rawData.Peek<FTexture>()); break;
        case EDescriptorType::Sampler: each(uni.Id, *rawData.Peek<FSampler>()); break;
        case EDescriptorType::RayTracingScene: each(uni.Id, *rawData.Peek<FRayTracingScene>()); break;
        default: AssertNotImplemented();
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
