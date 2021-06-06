#pragma once

#include "RHI_fwd.h"

#include "RHI/Config.h"

#include "IO/StaticString.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryView.h"
#include "Meta/Hash_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FResourceHandle {
    union {
        struct {
            u32 Uid;
            u16 Index;
            u16 InstanceID;
        };

        u64 Packed;
    };

    FResourceHandle() = default;

    CONSTEXPR FResourceHandle(u32 uid, u16 index, u16 instanceID) NOEXCEPT
        : Uid(uid), Index(index), InstanceID(instanceID) {}

    CONSTEXPR bool operator ==(const FResourceHandle& other) const { return (Packed == other.Packed); }
    CONSTEXPR bool operator !=(const FResourceHandle& other) const { return (not operator ==(other)); }

    template <typename _Visitor>
    CONSTEXPR auto Visit(_Visitor&& visitor) const NOEXCEPT;

    friend hash_t hash_value(const FResourceHandle& handle) NOEXCEPT {
        return hash_value(handle.Packed);
    }

    friend void swap(FResourceHandle& lhs, FResourceHandle& rhs) NOEXCEPT {
        std::swap(lhs.Packed, rhs.Packed);
    }
};

PPE_ASSUME_TYPE_AS_POD(FResourceHandle);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <u32 _Uid, bool _KeepName>
struct TNamedId {
    STATIC_CONST_INTEGRAL(u32, StringCapacity, 32);
    using string_t = TStaticString<StringCapacity>;

    string_t Name;
    hash_t HashValue{0};

    TNamedId() = default;

    CONSTEXPR TNamedId(const FStringView& name)
        : Name(name)
          , HashValue(hash_mem_constexpr(name.data(), name.size())) {}

    CONSTEXPR bool Valid() const { return !!HashValue; }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR FStringView MakeView() const { return Name.Str(); }

    CONSTEXPR friend hash_t hash_value(const TNamedId& id) { return id.HashValue; }

    CONSTEXPR bool operator ==(const TNamedId& other) const {
        return (HashValue == other.HashValue && Name == other.Name);
    }

    CONSTEXPR bool operator !=(const TNamedId& other) const { return (not operator ==(other)); }

    CONSTEXPR bool operator <(const TNamedId& other) const {
        return (HashValue != other.HashValue ? HashValue < other.HashValue : Name < other.Name);
    }

    CONSTEXPR bool operator >=(const TNamedId& other) const { return (not operator <(other)); }

    CONSTEXPR bool operator >(const TNamedId& other) const { return (other < *this); }
    CONSTEXPR bool operator <=(const TNamedId& other) const { return (not operator >(other)); }

};

//----------------------------------------------------------------------------
template <u32 _Uid>
struct TNamedId<_Uid, false> {
    hash_t HashValue{0};

    TNamedId() = default;

    CONSTEXPR TNamedId(const FStringView& name)
        : HashValue(hash_mem_constexpr(name.data(), name.size())) {}

    CONSTEXPR bool Valid() const { return !!HashValue; }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR FStringView MakeView() const { return FStringView{}; }

    CONSTEXPR friend hash_t hash_value(const TNamedId& id) { return id.HashValue; }

    CONSTEXPR bool operator ==(const TNamedId& other) const { return (HashValue == other.HashValue); }
    CONSTEXPR bool operator !=(const TNamedId& other) const { return (HashValue != other.HashValue); }

    CONSTEXPR bool operator <(const TNamedId& other) const { return (HashValue < other.HashValue); }
    CONSTEXPR bool operator >=(const TNamedId& other) const { return (HashValue >= other.HashValue); }

    CONSTEXPR bool operator >(const TNamedId& other) const { return (HashValue > other.HashValue); }
    CONSTEXPR bool operator <=(const TNamedId& other) const { return (HashValue <= other.HashValue); }

};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TNamedId<_Uid COMMA _KeepName>, u32 _Uid, bool _KeepName);
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <u32 _Uid>
struct TResourceId {
    STATIC_CONST_INTEGRAL(u32, Uid, _Uid);

    using FIndex = u16;
    using FInstanceID = u16;
    using FPackedData = u32;
    STATIC_ASSERT(sizeof(FIndex) + sizeof(FInstanceID) == sizeof(FPackedData));

    union {
        struct {
            FIndex Index;
            FInstanceID InstanceID;
        };

        FPackedData Packed;
    };

    CONSTEXPR TResourceId() : Packed(UMax) {}

    TResourceId(const TResourceId&) = default;
    TResourceId& operator =(const TResourceId&) = default;

    explicit CONSTEXPR TResourceId(FPackedData data) : Packed(data) {}
    explicit CONSTEXPR TResourceId(FIndex index, FInstanceID instanceID) : Index(index), InstanceID(instanceID) {}

    CONSTEXPR bool Valid() const { return (Packed != UMax); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR FResourceHandle Pack() const { return {_Uid, Index, InstanceID}; }

    static CONSTEXPR TResourceId Unpack(FResourceHandle packed) {
        Assert_NoAssume(packed.Uid == _Uid);
        return TResourceId<_Uid>(packed.Index, packed.InstanceID);
    }

    CONSTEXPR bool operator ==(const TResourceId& other) const { return (Packed == other.Packed); }
    CONSTEXPR bool operator !=(const TResourceId& other) const { return (Packed != other.Packed); }

    CONSTEXPR friend hash_t hash_value(const TResourceId& id) { return hash_size_t_constexpr(id.Packed); }

};
PPE_ASSUME_TEMPLATE_AS_POD(TResourceId<_Uid>, u32 _Uid);
//----------------------------------------------------------------------------
template <typename T>
struct TResourceWrappedId;

template <u32 _Uid>
struct TResourceWrappedId<TResourceId<_Uid>> {
    using id_t = TResourceId<_Uid>;
    id_t Id;

    TResourceWrappedId() = default;

    TResourceWrappedId(const TResourceWrappedId&) = delete;
    TResourceWrappedId& operator =(const TResourceWrappedId&) = delete;

    CONSTEXPR TResourceWrappedId(TResourceWrappedId&& rvalue) NOEXCEPT : Id(rvalue.Release()) {}
    CONSTEXPR TResourceWrappedId& operator =(TResourceWrappedId&& rvalue) NOEXCEPT {
        Id = rvalue.Release();
        return (*this);
    }

    explicit CONSTEXPR TResourceWrappedId(id_t id) : Id(id) {}

    CONSTEXPR bool Valid() const { return Id.Valid(); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR id_t Get() const { return Id; }
    CONSTEXPR id_t operator *() const { return Id; }

    CONSTEXPR FResourceHandle Pack() const { return Id.Pack(); }

    static CONSTEXPR TResourceWrappedId Unpack(FResourceHandle packed) {
        Assert_NoAssume(packed.Uid == _Uid);
        return {TResourceId<_Uid>(packed.Index, packed.InstanceID)};
    }

    CONSTEXPR id_t Release() {
        const id_t result{Id};
        Id = Default;
        return result;
    }

    CONSTEXPR bool operator ==(const TResourceWrappedId& other) const { return (Id == other.Id); }
    CONSTEXPR bool operator !=(const TResourceWrappedId& other) const { return (Id != other.Id); }

    CONSTEXPR friend hash_t hash_value(const TResourceWrappedId& wrapped) { return hash_value(wrapped.Id); }
};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Visitor>
CONSTEXPR auto FResourceHandle::Visit(_Visitor&& visitor) const NOEXCEPT {
    switch (Uid) {
    case FRawBufferID::Uid: return visitor(FRawBufferID{Index, InstanceID});
    case FRawImageID::Uid: return visitor(FRawImageID{Index, InstanceID});
    case FRawGPipelineID::Uid: return visitor(FRawGPipelineID{Index, InstanceID});
    case FRawMPipelineID::Uid: return visitor(FRawMPipelineID{Index, InstanceID});
    case FRawCPipelineID::Uid: return visitor(FRawCPipelineID{Index, InstanceID});
    case FRawRTPipelineID::Uid: return visitor(FRawRTPipelineID{Index, InstanceID});
    case FRawSamplerID::Uid: return visitor(FRawSamplerID{Index, InstanceID});
    case FRawDescriptorSetLayoutID::Uid: return visitor(FRawDescriptorSetLayoutID{Index, InstanceID});
    case FRawPipelineResourcesID::Uid: return visitor(FRawPipelineResourcesID{Index, InstanceID});
    case FRawRTSceneID::Uid: return visitor(FRawRTSceneID{Index, InstanceID});
    case FRawRTGeometryID::Uid: return visitor(FRawRTGeometryID{Index, InstanceID});
    case FRawRTShaderTableID::Uid: return visitor(FRawRTShaderTableID{Index, InstanceID});
    case FRawSwapchainID::Uid: return visitor(FRawSwapchainID{Index, InstanceID});
    case FRawMemoryID::Uid: return visitor(FRawMemoryID{Index, InstanceID});
    case FRawPipelineLayoutID::Uid: return visitor(FRawPipelineLayoutID{Index, InstanceID});
    case FRawRenderPassID::Uid: return visitor(FRawRenderPassID{Index, InstanceID});
    case FRawFramebufferID::Uid: return visitor(FRawFramebufferID{Index, InstanceID});
    default: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
#define PPE_RHI_TEXTWRITEROPERATOR_DECL(_TYPE) \
PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, const _TYPE& value); \
PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, const _TYPE& value);
//----------------------------------------------------------------------------
PPE_RHI_TEXTWRITEROPERATOR_DECL(FUniformID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FPushConstantID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FDescriptorSetID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FSpecializationID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FVertexID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FVertexBufferID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FMemPoolID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRTShaderID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FGeometryID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FInstanceID)
//----------------------------------------------------------------------------
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawBufferID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawImageID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawGPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawMPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawCPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawRTPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawSamplerID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawDescriptorSetLayoutID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawPipelineResourcesID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawRTSceneID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawRTGeometryID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawRTShaderTableID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawSwapchainID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FLogicalPassID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawMemoryID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawPipelineLayoutID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawRenderPassID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRawFramebufferID)
//----------------------------------------------------------------------------
PPE_RHI_TEXTWRITEROPERATOR_DECL(FBufferID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FImageID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FGPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FMPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FCPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRTPipelineID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FSamplerID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRTSceneID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRTGeometryID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRTShaderTableID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FSwapchainID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FLogicalPassID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FMemoryID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FPipelineLayoutID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FRenderPassID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FFramebufferID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FPipelineResourcesID)
PPE_RHI_TEXTWRITEROPERATOR_DECL(FDescriptorSetLayoutID)
//----------------------------------------------------------------------------
#undef PPE_RHI_TEXTWRITEROPERATOR_DECL
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
