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
#include "Meta/AutoStruct.h"
#include "Meta/Optional.h"

#if USE_PPE_RHIDEBUG
#   include "IO/StaticString.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TPipelineElementArray {
    const u16 Capacity{};
    u16 Count{};
    T Data[1];

    TPipelineElementArray() = default;
    TPipelineElementArray(u16 capacity, u16 count) NOEXCEPT
    :   Capacity(capacity)
    ,   Count(count) {
        FPlatformMemory::Memzero(Data, Count * sizeof(T));
    }

    NODISCARD TMemoryView<T> MakeView() {
        return { Data, Count };
    }
    NODISCARD TMemoryView<const T> MakeView() const {
        return { Data, Count };
    }

    NODISCARD T& operator [](size_t index) {
        return MakeView()[index];
    }
    NODISCARD const T& operator [](size_t index) const {
        return MakeView()[index];
    }

    bool operator ==(const TPipelineElementArray& other) const NOEXCEPT {
        const auto lhs = MakeView();
        const auto rhs = other.MakeView();
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    bool operator !=(const TPipelineElementArray& other) const NOEXCEPT {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const TPipelineElementArray& arr) NOEXCEPT {
        return hash_range(arr.Data, arr.Count);
    }
};
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
    using TElementArray = TPipelineElementArray<T>;

    struct FBuffer {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Buffer);

        PPE_DEFINE_AUTOPOD(FElement,
            (FRawBufferID, BufferId),
            (size_t, Offset, {}),
            (size_t, Size, {}))

        PPE_AUTOSTRUCT_MEMBERS(FBuffer,
            (FBindingIndex, Index),
            (EResourceState, State),
            (u32, DynamicOffsetIndex, {}),
            (size_t, StaticSize, {}),
            (size_t, ArrayStride, {}),
            (TElementArray<FElement>, Elements))

        PPE_ASSUME_FRIEND_AS_POD(FBuffer)
    };

    struct FTexelBuffer {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::TexelBuffer);

        PPE_DEFINE_AUTOPOD(FElement,
            (FRawBufferID, BufferId),
            (FBufferViewDesc, Desc))

        PPE_AUTOSTRUCT_MEMBERS(FTexelBuffer,
            (FBindingIndex, Index),
            (EResourceState, State),
            (TElementArray<FElement>, Elements))

        PPE_ASSUME_FRIEND_AS_POD(FTexelBuffer)
    };

    struct FImage {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Image);

        PPE_DEFINE_AUTOPOD(FElement,
            (FRawImageID, ImageId),
            (Meta::TOptional<FImageViewDesc>, Desc))

        PPE_AUTOSTRUCT_MEMBERS(FImage,
            (FBindingIndex, Index),
            (EResourceState, State),
            (EImageSampler, ImageType),
            (TElementArray<FElement>, Elements))

        PPE_ASSUME_FRIEND_AS_POD(FImage)
    };

    struct FTexture {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Texture);

        PPE_DEFINE_AUTOPOD(FElement,
            (FRawImageID, ImageId),
            (FRawSamplerID, SamplerId),
            (Meta::TOptional<FImageViewDesc>, Desc))

        PPE_AUTOSTRUCT_MEMBERS(FTexture,
            (FBindingIndex, Index),
            (EResourceState, State),
            (EImageSampler, SamplerType),
            (TElementArray<FElement>, Elements))

        PPE_ASSUME_FRIEND_AS_POD(FTexture)
    };

    struct FSampler {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::Sampler);

        PPE_DEFINE_AUTOPOD(FElement,
            (FRawSamplerID, SamplerId))

        PPE_AUTOSTRUCT_MEMBERS(FSampler,
            (FBindingIndex, Index),
            (TElementArray<FElement>, Elements))

        PPE_ASSUME_FRIEND_AS_POD(FSampler)
    };

    struct FRayTracingScene {
        STATIC_CONST_INTEGRAL(EDescriptorType, TypeId, EDescriptorType::RayTracingScene);

        PPE_DEFINE_AUTOPOD(FElement,
            (FRawRTSceneID, SceneId))

        PPE_AUTOSTRUCT_MEMBERS(FRayTracingScene,
            (FBindingIndex, Index),
            (TElementArray<FElement>, Elements))

        PPE_ASSUME_FRIEND_AS_POD(FRayTracingScene)
    };

    // FUniform
    struct FUniform {
        FUniformID Id;
        EDescriptorType Type{ Default };
        u16 RelativeOffset{ 0 };

        template <typename T>
        NODISCARD T& Get() NOEXCEPT {
            Assert(T::TypeId == Type);
            return *bit_cast<T*>(bit_cast<intptr_t>(this) + RelativeOffset);
        }
        template <typename T>
        NODISCARD const T& Get() const NOEXCEPT {
            return const_cast<FUniform*>(this)->Get<T>();
        }

        bool operator ==(const FUniformID& id) const NOEXCEPT { return (id == Id); }
        bool operator !=(const FUniformID& id) const NOEXCEPT { return (id != Id); }

        bool operator < (const FUniformID& id) const NOEXCEPT { return (Id <  id); }
        bool operator >=(const FUniformID& id) const NOEXCEPT { return (Id >= id); }

        PPE_ASSUME_FRIEND_AS_POD(FUniform)
    };

    // FDynamicData
    using FDynamicDataStorage = RAWSTORAGE(RHIDynamicData, u8);
    struct FDynamicData {
        FDynamicDataStorage Storage;
        FRawDescriptorSetLayoutID LayoutId;
        u32 UniformsCount{ 0 };
        u32 UniformsOffset{ 0 };
        u32 DynamicOffsetsCount{ 0 };
        u32 DynamicOffsetsOffset{ 0 };

        NODISCARD TMemoryView<FUniform> Uniforms() { return Storage.SubRange(UniformsOffset, UniformsCount * sizeof(FUniform)).Cast<FUniform>(); }
        NODISCARD TMemoryView<const FUniform> Uniforms() const { return const_cast<FDynamicData*>(this)->Uniforms(); }

        template <typename _Each>
        void EachUniform(_Each&& each);
        template <typename _Each>
        void EachUniform(_Each&& each) const { const_cast<FDynamicData*>(this)->EachUniform(std::forward<_Each>(each)); }

        template <typename _Pred>
        NODISCARD bool UniformByPred(_Pred&& pred);
        template <typename _Pred>
        NODISCARD bool UniformByPred(_Pred&& pred) const { return const_cast<FDynamicData*>(this)->UniformByPred(std::forward<_Pred>(pred)); }

        NODISCARD TMemoryView<u32> DynamicOffsets() { return Storage.SubRange(DynamicOffsetsOffset, DynamicOffsetsCount * sizeof(u32)).Cast<u32>(); }
        NODISCARD TMemoryView<const u32> DynamicOffsets() const { return const_cast<FDynamicData*>(this)->DynamicOffsets(); }

        bool operator ==(const FDynamicData& other) const NOEXCEPT { return CompareDynamicData(*this, other); }
        bool operator !=(const FDynamicData& other) const NOEXCEPT { return (not operator ==(other)); }
    };

    FPipelineResources() NOEXCEPT;
    ~FPipelineResources();

    FPipelineResources(const FPipelineResources& other);
    FPipelineResources(FPipelineResources&& rvalue) NOEXCEPT;

#if USE_PPE_RHIDEBUG
    FConstChar DebugName() const { return _debugName; }
    void SetDebugName(FConstChar name) { _debugName = name; }
#endif

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
    FPipelineResources& BindBuffer(const FUniformID& id, FRawBufferID buffer, size_t offset, size_t size, u32 elementIndex = 0);
    FPipelineResources& BindBuffers(const FUniformID& id, TMemoryView<const FBufferID> buffers);
    FPipelineResources& BindBuffers(const FUniformID& id, TMemoryView<const FRawBufferID> buffers);

    FPipelineResources& SetBufferBase(const FUniformID& id, size_t offset, u32 elementIndex = 0);

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

#if USE_PPE_RHIDEBUG
    TStaticString<64> _debugName;
#endif

    FRawPipelineResourcesID CachedId_() const { return FRawPipelineResourcesID(_cachedId.load(std::memory_order_acquire)); }
    void SetCachedId_(FRawPipelineResourcesID id) const { _cachedId.store(id.Packed, std::memory_order_relaxed); }
    void ResetCachedId_() const { _cachedId.store(UMax, std::memory_order_relaxed); }

    template <typename T>
    T& Resource_(const FUniformID& id);
    template <typename T>
    bool HasResource_(const FUniformID& id) const;

};
//----------------------------------------------------------------------------
using FPipelineResourceSet = TFixedSizeAssociativeVector<
    FDescriptorSetID, PCPipelineResources,
    MaxDescriptorSets >;
//----------------------------------------------------------------------------
template <typename _Each>
void FPipelineResources::FDynamicData::EachUniform(_Each&& each) {
    for (FUniform& uni : Uniforms()) {
        switch (uni.Type) {
        case EDescriptorType::Unknown: break;
        case EDescriptorType::Buffer: each(uni.Id, uni.Get<FBuffer>()); break;
        case EDescriptorType::TexelBuffer: each(uni.Id, uni.Get<FTexelBuffer>()); break;
        case EDescriptorType::SubpassInput:
        case EDescriptorType::Image: each(uni.Id, uni.Get<FImage>()); break;
        case EDescriptorType::Texture: each(uni.Id, uni.Get<FTexture>()); break;
        case EDescriptorType::Sampler: each(uni.Id, uni.Get<FSampler>()); break;
        case EDescriptorType::RayTracingScene: each(uni.Id, uni.Get<FRayTracingScene>()); break;
        default: AssertNotImplemented();
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Pred>
bool FPipelineResources::FDynamicData::UniformByPred(_Pred&& pred) {
    for (FUniform& uni : Uniforms()) {
        switch (uni.Type) {
        case EDescriptorType::Unknown: break;
        case EDescriptorType::Buffer: if (pred(uni.Id, uni.Get<FBuffer>())) return true; break;
        case EDescriptorType::TexelBuffer: if (pred(uni.Id, uni.Get<FTexelBuffer>())) return true; break;
        case EDescriptorType::SubpassInput:
        case EDescriptorType::Image: if (pred(uni.Id, uni.Get<FImage>())) return true; break;
        case EDescriptorType::Texture: if (pred(uni.Id, uni.Get<FTexture>())) return true; break;
        case EDescriptorType::Sampler: if (pred(uni.Id, uni.Get<FSampler>())) return true; break;
        case EDescriptorType::RayTracingScene: if (pred(uni.Id, uni.Get<FRayTracingScene>())) return true; break;
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
