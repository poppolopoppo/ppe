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

    template <EDescriptorType _TypeId, typename _Element, u32 _Capacity = 1>
    struct TResource {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, _TypeId);
        STATIC_CONST_INTEGRAL(u32, Capacity, _Capacity);

        using base_type = TResource;
        using FElement = _Element;

        FBindingIndex Index;
        //const u16 ElementCapacity;
        u16 ElementCount;
        FElement Elements[_Capacity];

        bool operator ==(const TResource& other) const {
            if (Index != other.Index || ElementCount != other.ElementCount)
                return false;
            return std::equal(std::begin(Elements), std::begin(Elements) + ElementCount, std::begin(other.Elements));
        }
        bool operator !=(const TResource& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const TResource& res) {
            return hash_tuple(res.Index, res.ElementCount, hash_range(res.Elements, res.ElementCount));
        }
    };
    template <EDescriptorType _TypeId, typename _Element, u32 _Capacity = 1>
    struct TResourceStateful : TResource<_TypeId, _Element, _Capacity> {
        using base_type = TResource<_TypeId, _Element, _Capacity>;

        EResourceState State;

        bool operator ==(const TResourceStateful& other) const { return (State == other.State && static_cast<const base_type&>(*this) == other); }
        bool operator !=(const TResourceStateful& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const TResourceStateful& res) {
            return hash_tuple(res.State, static_cast<const base_type&>(res));
        }
    };

    // FBuffer
    struct FBufferElement {
        FRawBufferID BufferId;
        size_t Offset;
        size_t Size;

        bool operator ==(const FBufferElement& other) const { return (BufferId == other.BufferId && Offset == other.Offset && Size == other.Size); }
        bool operator !=(const FBufferElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FBufferElement& res) { return hash_tuple(res.BufferId, res.Offset, res.Size); }
    };
    struct FBuffer : TResourceStateful<EDescriptorType::Buffer, FBufferElement> {
        u32 DynamicStateOffset;
        size_t StaticSize;
        size_t ArrayStride;

        bool operator ==(const FBuffer& other) const {
            if (DynamicStateOffset != other.DynamicStateOffset || StaticSize != other.StaticSize || ArrayStride != other.ArrayStride)
                return false;
            return (static_cast<const base_type&>(*this) == other);
        }
        bool operator !=(const FBuffer& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FBuffer& res) {
            return hash_tuple(res.DynamicStateOffset, res.StaticSize, res.ArrayStride, static_cast<const base_type&>(res));
        }
    };

    // FTexelBuffer
    struct FTexelBufferElement {
        FRawBufferID BufferId;
        FBufferViewDesc Desc;

        bool operator ==(const FTexelBufferElement& other) const { return (BufferId == other.BufferId && Desc == other.Desc); }
        bool operator !=(const FTexelBufferElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FTexelBufferElement& res) { return hash_tuple(res.BufferId, res.Desc); }
    };
    using FTexelBuffer = TResourceStateful<EDescriptorType::TexelBuffer, FTexelBufferElement>;

    // FImage
    struct FImageElement {
        FRawImageID ImageId;
        Meta::TOptional<FImageViewDesc> Desc;

        bool operator ==(const FImageElement& other) const { return (ImageId == other.ImageId && Desc == other.Desc); }
        bool operator !=(const FImageElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FImageElement& res) { return hash_tuple(res.ImageId, res.Desc); }
    };
    using FImage = TResourceStateful<EDescriptorType::Image, FImageElement>;

    // FTexture
    struct FTextureElement {
        FRawImageID ImageId;
        FRawSamplerID SamplerId;
        Meta::TOptional<FImageViewDesc> Desc;

        bool operator ==(const FTextureElement& other) const { return (ImageId == other.ImageId && SamplerId == other.SamplerId && Desc == other.Desc); }
        bool operator !=(const FTextureElement& other) const { return (not operator ==(other)); }

        friend hash_t hash_value(const FTextureElement& res) { return hash_tuple(res.ImageId, res.SamplerId, res.Desc); }
    };
    using FTexture = TResourceStateful<EDescriptorType::Texture, FTextureElement>;

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
            return *reinterpret_cast<T*>(reinterpret_cast<u8*>(p) + Offset);
        }
        template <typename T>
        const T& Get(const void* p) const {
            return Get<T>(const_cast<void*>(p));
        }

        bool operator ==(const FUniformID& id) const { return (id == Id); }
        bool operator !=(const FUniformID& id) const { return (id != Id); }

        bool operator < (const FUniformID& id) const { return (id <  Id); }
        bool operator >=(const FUniformID& id) const { return (id >= Id); }

        bool operator <=(const FUniformID& id) const { return (id <= Id); }
        bool operator > (const FUniformID& id) const { return (id >  Id); }
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

        FDynamicData(const FDynamicData&) = delete;
        FDynamicData& operator =(const FDynamicData&) = delete;

        FDynamicData(FDynamicData&&) = default;
        FDynamicData& operator =(FDynamicData&&) = default;

        TMemoryView<FUniform> Uniforms() { return Storage.SubRange(UniformsOffset, UniformsCount * sizeof(FUniform)).Cast<FUniform>(); }
        TMemoryView<const FUniform> Uniforms() const { return const_cast<FDynamicData*>(this)->Uniforms(); }

        template <typename _Each>
        void EachUniform(_Each&& each);
        template <typename _Each>
        void EachUniform(_Each&& each) const { const_cast<FDynamicData*>(this)->EachUniform(each); }

        TMemoryView<u32> DynamicOffsets() { return { reinterpret_cast<u32*>((u8*)this + DynamicOffsetsOffset), DynamicOffsetsCount }; }
        TMemoryView<const u32> DynamicOffsets() const { return { reinterpret_cast<const u32*>((const u8*)this + DynamicOffsetsOffset), DynamicOffsetsCount }; }

        bool Equals(const FDynamicData& other) const;
        hash_t HashValue() const;

        friend hash_t hash_value(const FDynamicData& data) { return data.HashValue(); }

        bool operator ==(const FDynamicData& other) const { return Equals(other); }
        bool operator !=(const FDynamicData& other) const { return (not operator ==(other)); }
    };

    FPipelineResources() { ResetCachedId_(); }
    ~FPipelineResources();

    FPipelineResources(const FPipelineResources& other);
    FPipelineResources& operator =(const FPipelineResources& other);

    FPipelineResources(FPipelineResources&& rvalue) NOEXCEPT : FPipelineResources() { operator =(std::move(rvalue)); }
    FPipelineResources& operator =(FPipelineResources&& rvalue) NOEXCEPT {
        _dynamicData = std::move(rvalue._dynamicData);
        _allowEmptyResources = rvalue._allowEmptyResources;
        SetCachedId_(rvalue.CachedId_());
        rvalue.SetCachedId_(FRawPipelineResourcesID{0});
        return (*this);
    }

    FRawDescriptorSetLayoutID Layout() const {
        READSCOPELOCK(_rwlock);
        return (_dynamicData.LayoutId);
    }

    void AllowEmptyResources(bool value) {
        WRITESCOPELOCK(_rwlock);
        _allowEmptyResources = value;
    }
    bool AllowEmptyResources() const {
        READSCOPELOCK(_rwlock);
        return _allowEmptyResources;
    }

    TMemoryView<const u32> DynamicOffsets() const {
        READSCOPELOCK(_rwlock);
        return _dynamicData.DynamicOffsets();
    }

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

    void Reset(FUniformID uniform);
    void ResetAll();

    static FDynamicData CreateDynamicData(const FPipelineDesc::FUniformMap& uniforms,
        u32 resourceCount, u32 arrayElemCount, u32 bufferDynamicOffsetCount );
    static FDynamicData CloneDynamicData(const FPipelineResources& res);
    static FDynamicData RemoveDynamicData(FPipelineResources* pres);

    static bool Initialize(FPipelineResources* pres, FRawDescriptorSetLayoutID layoutId, FDynamicData&& rdata);

    static FRawPipelineResourcesID GetCached(const FPipelineResources& res) { return res.CachedId_(); }
    static void SetCached(const FPipelineResources& res, FRawPipelineResourcesID id) { res.SetCachedId_(id); }

private:
    using FCachedID_ = std::atomic< FRawPipelineResourcesID::data_t >;

    FReadWriteLock _rwlock;
    mutable FCachedID_ _cachedId;
    FDynamicData _dynamicData;
    bool _allowEmptyResources{ false };

    FRawPipelineResourcesID CachedId_() const { return FRawPipelineResourcesID(_cachedId.load(std::memory_order_acquire)); }
    void SetCachedId_(FRawPipelineResourcesID id) const { _cachedId.store(id.Packed, std::memory_order_relaxed); }
    void ResetCachedId_() const { _cachedId.store(UMax, std::memory_order_relaxed); }

    u32& DynamicOffset_(u32 i) { return _dynamicData.DynamicOffsets()[i]; }

    template <typename T>
    T* Resource_(const FUniformID& id);
    template <typename T>
    bool HasResource_(const FUniformID& id) const;
};
//----------------------------------------------------------------------------
using FPipelineResourceSet = ASSOCIATIVE_VECTORINSITU(
    RHIPipeline,
    FDescriptorSetID, PCPipelineResources,
    MaxDescriptorSets );
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
