#pragma once

#include "RHI_fwd.h"

#include "RHI/BindingIndex.h"
#include "RHI/BufferDesc.h"
#include "RHI/ImageDesc.h"
#include "RHI/PipelineDesc.h"
#include "RHI/ResourceId.h"
#include "RHI/ResourceState.h"

#include "Container/AssociativeVector.h"
#include "Container/TupleTie.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Meta/Optional.h"

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

    template <typename T>
    struct TElementArray {
        const u16 Capacity;
        u16 Count;
        T Data[1];

        TMemoryView<T> MakeView() {
            return { Data, Count };
        }
        TMemoryView<const T> MakeView() const {
            return { Data, Count };
        }

        T& operator [](size_t index) {
            return MakeView()[index];
        }
        const T& operator [](size_t index) const {
            return MakeView()[index];
        }

        bool operator ==(const TElementArray& other) const {
            const auto lhs = MakeView();
            const auto rhs = other.MakeView();
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }
        bool operator !=(const TElementArray& other) const {
            return (not operator ==(other));
        }

        friend hash_t hash_value(const TElementArray& arr) NOEXCEPT {
            return hash_range(arr.Data, arr.Count);
        }
    };

    struct FBuffer {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Buffer);

        struct FElement {
            FRawBufferID BufferId;
            u32 Offset;
            u32 Size;

            TIE_AS_TUPLE_STRUCT(FElement)
            PPE_ASSUME_FRIEND_AS_POD(FElement)
        };

        FBindingIndex Index;
        EResourceState State;
        u32 DynamicOffsetIndex;
        u32 StaticSize;
        u32 ArrayStride;
        TElementArray<FElement> Elements;

        TIE_AS_TUPLE_STRUCT(FBuffer)
        PPE_ASSUME_FRIEND_AS_POD(FBuffer)
    };

    struct FTexelBuffer {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::TexelBuffer);

        struct FElement {
            FRawBufferID BufferId;
            FBufferViewDesc Desc;

            TIE_AS_TUPLE_STRUCT(FElement)
            PPE_ASSUME_FRIEND_AS_POD(FElement)
        };

        FBindingIndex Index;
        EResourceState State;
        TElementArray<FElement> Elements;

        TIE_AS_TUPLE_STRUCT(FTexelBuffer)
        PPE_ASSUME_FRIEND_AS_POD(FTexelBuffer)
    };

    struct FImage {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Image);

        struct FElement {
            FRawImageID ImageId;
            Meta::TOptional<FImageViewDesc> Desc;

            TIE_AS_TUPLE_STRUCT(FElement)
            PPE_ASSUME_FRIEND_AS_POD(FElement)
        };

        FBindingIndex Index;
        EResourceState State;
        EImageSampler ImageType;
        TElementArray<FElement> Elements;

        TIE_AS_TUPLE_STRUCT(FImage)
        PPE_ASSUME_FRIEND_AS_POD(FImage)
    };

    struct FTexture {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Texture);

        struct FElement {
            FRawImageID ImageId;
            FRawSamplerID SamplerId;
            Meta::TOptional<FImageViewDesc> Desc;

            TIE_AS_TUPLE_STRUCT(FElement)
            PPE_ASSUME_FRIEND_AS_POD(FElement)
        };

        FBindingIndex Index;
        EResourceState State;
        EImageSampler SamplerType;
        TElementArray<FElement> Elements;

        TIE_AS_TUPLE_STRUCT(FTexture)
        PPE_ASSUME_FRIEND_AS_POD(FTexture)
    };

    struct FSampler {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Sampler);

        struct FElement {
            FRawSamplerID SamplerId;

            TIE_AS_TUPLE_STRUCT(FElement)
            PPE_ASSUME_FRIEND_AS_POD(FElement)
        };

        FBindingIndex Index;
        TElementArray<FElement> Elements;

        TIE_AS_TUPLE_STRUCT(FSampler)
        PPE_ASSUME_FRIEND_AS_POD(FSampler)
    };

    struct FRayTracingScene {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::RayTracingScene);

        struct FElement {
            FRawRTSceneID SceneId;

            TIE_AS_TUPLE_STRUCT(FElement)
            PPE_ASSUME_FRIEND_AS_POD(FElement)
        };

        FBindingIndex Index;
        TElementArray<FElement> Elements;

        TIE_AS_TUPLE_STRUCT(FRayTracingScene)
        PPE_ASSUME_FRIEND_AS_POD(FRayTracingScene)
    };

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

        PPE_ASSUME_FRIEND_AS_POD(FUniform)
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
        void EachUniform(_Each&& each) const { const_cast<FDynamicData*>(this)->EachUniform(std::forward<_Each>(each)); }

        template <typename _Pred>
        bool UniformByPred(_Pred&& pred);
        template <typename _Pred>
        bool UniformByPred(_Pred&& pred) const { return const_cast<FDynamicData*>(this)->UniformByPred(std::forward<_Pred>(pred)); }

        TMemoryView<u32> DynamicOffsets() { return { reinterpret_cast<u32*>(reinterpret_cast<u8*>(this) + DynamicOffsetsOffset), DynamicOffsetsCount }; }
        TMemoryView<const u32> DynamicOffsets() const { return { reinterpret_cast<const u32*>(reinterpret_cast<const u8*>(this) + DynamicOffsetsOffset), DynamicOffsetsCount }; }

        bool operator ==(const FDynamicData& other) const { return CompareDynamicData(*this, other); }
        bool operator !=(const FDynamicData& other) const { return (not operator ==(other)); }
    };

    FPipelineResources() NOEXCEPT { ResetCachedId_(); }
    ~FPipelineResources();

    FPipelineResources(const FPipelineResources& other);
    FPipelineResources(FPipelineResources&& rvalue) NOEXCEPT;

    FRawDescriptorSetLayoutID Layout() const { return _dynamicData.LockShared()->LayoutId; }
    TMemoryView<const u32> DynamicOffsets() const { return _dynamicData.LockShared()->DynamicOffsets(); }

    bool AllowEmptyResources() const { return _allowEmptyResources; }
    void SetAllowEmptyResources(bool value) { _allowEmptyResources = value; }

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

    FPipelineResources& BindTexelBuffer(const FUniformID& id, FRawBufferID buffer, const FBufferViewDesc& desc, u32 elementIndex = 0);
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

    static FDynamicData CloneDynamicData(const FPipelineResources& desc);
    static FDynamicData&& StealDynamicData(FPipelineResources& desc) NOEXCEPT;

    static FRawPipelineResourcesID Cached(const FPipelineResources& resource) { return resource.CachedId_(); }
    static void SetCached(const FPipelineResources& resource, FRawPipelineResourcesID id) { resource.SetCachedId_(id); }

private:
    using FCachedID_ = std::atomic< FRawPipelineResourcesID::FPackedData >;


    mutable FCachedID_ _cachedId;
    TRHIThreadSafe<FDynamicData> _dynamicData;
    bool _allowEmptyResources{ false };

    FRawPipelineResourcesID CachedId_() const { return FRawPipelineResourcesID(_cachedId.load(std::memory_order_acquire)); }
    void SetCachedId_(FRawPipelineResourcesID id) const { _cachedId.store(id.Packed, std::memory_order_relaxed); }
    void ResetCachedId_() const { _cachedId.store(UMax, std::memory_order_relaxed); }

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
template <typename _Pred>
bool FPipelineResources::FDynamicData::UniformByPred(_Pred&& pred) {
    for (const FUniform& uni : Uniforms()) {
        FRawMemory rawData = Storage.MakeView().CutStartingAt(uni.Offset);

        switch (uni.Type) {
        case EDescriptorType::Unknown: break;
        case EDescriptorType::Buffer: if (pred(uni.Id, *rawData.Peek<FBuffer>())) return true; break;
        case EDescriptorType::TexelBuffer: if (pred(uni.Id, *rawData.Peek<FTexelBuffer>())) return true; break;
        case EDescriptorType::SubpassInput:
        case EDescriptorType::Image: if (pred(uni.Id, *rawData.Peek<FImage>())) return true; break;
        case EDescriptorType::Texture: if (pred(uni.Id, *rawData.Peek<FTexture>())) return true; break;
        case EDescriptorType::Sampler: if (pred(uni.Id, *rawData.Peek<FSampler>())) return true; break;
        case EDescriptorType::RayTracingScene: if (pred(uni.Id, *rawData.Peek<FRayTracingScene>())) return true; break;
        default: AssertNotImplemented();
        }
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
