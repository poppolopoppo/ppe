#pragma once

#include "RHI_fwd.h"

#include "RHI/Config.h"

#include "Container/Hash.h"
#include "Container/HashHelpers.h"
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
        u64 Packed{ 0 };

        struct {
            u32 Uid;
            u16 Index;
            u16 InstanceID;
        };
    };

    FResourceHandle() = default;

    CONSTEXPR explicit FResourceHandle(u64 packed) NOEXCEPT
    :   Packed(packed)
    {}

    CONSTEXPR FResourceHandle(u32 uid, u16 index, u16 instanceID) NOEXCEPT {
        Uid = uid;
        Index = index;
        InstanceID = instanceID;
    }

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
// TNamedId<> computes a hashed id based on an input string and an index, which can be optimized out
template <u32 _Uid, bool _KeepName>
struct TNamedId {
    STATIC_CONST_INTEGRAL(u32, StringCapacity, 32);
    using string_t = TStaticString<StringCapacity>;

    hash_t HashValue{ 0 };
    size_t Index{ 0 };
    string_t Name;

    CONSTEXPR TNamedId() NOEXCEPT : TNamedId("") {}

    CONSTEXPR explicit TNamedId(Meta::FEmptyKey) NOEXCEPT // for Meta::TEmptyKey<> traits
        : HashValue(UMax)
#if USE_PPE_ASSERT
        , Index(UMax)
        , Name(UMax, 0)
#endif
    {}

    CONSTEXPR explicit TNamedId(const FStringView& name, size_t index = 0) NOEXCEPT
        : HashValue(hash_mem_constexpr(name.data(), name.size()))
        , Index(index)
        , Name(name) {
        if (not Name.empty())
            HashValue = hash_size_t_constexpr(index, HashValue);
    }

    CONSTEXPR CONSTF bool Valid() const { return !!HashValue; }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR FStringView MakeView() const { return Name.Str(); }

    CONSTEXPR friend hash_t hash_value(const TNamedId& id) { return id.HashValue; }

    CONSTEXPR bool operator !=(const TNamedId& other) const { return (not operator ==(other)); }
    CONSTEXPR bool operator ==(const TNamedId& other) const {
        if (HashValue == other.HashValue) {
            Assert_NoAssume(Name == other.Name);
            Assert_NoAssume(Name.empty() || Index == other.Index);
            return true;
        }
        else {
            Assert_NoAssume(Name != other.Name || Index != other.Index);
            return false;
        }
    }

    CONSTEXPR bool operator < (const TNamedId& other) const { return (HashValue <  other.HashValue); }
    CONSTEXPR bool operator >=(const TNamedId& other) const { return (HashValue >= other.HashValue); }

    CONSTEXPR bool operator > (const TNamedId& other) const { return (HashValue >  other.HashValue); }
    CONSTEXPR bool operator <=(const TNamedId& other) const { return (HashValue <= other.HashValue); }

};
//----------------------------------------------------------------------------
template <u32 _Uid>
struct TNamedId<_Uid, false> {
    hash_t HashValue{0};

    TNamedId() = default;

    CONSTEXPR TNamedId(const TNamedId<_Uid, true>& keepName) NOEXCEPT : HashValue(keepName.HashValue) {}

    CONSTEXPR explicit TNamedId(Meta::FEmptyKey) NOEXCEPT : HashValue(UMax) {} // for Meta::TEmptyKey<> traits

    CONSTEXPR explicit TNamedId(const FStringView& name, size_t index = 0) NOEXCEPT
        : HashValue(hash_size_t_constexpr(index, hash_mem_constexpr(name.data(), name.size())))
    {}

    CONSTEXPR CONSTF bool Valid() const { return !!HashValue; }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR FStringView MakeView() const { return FStringView{}; }

    CONSTEXPR friend hash_t hash_value(const TNamedId& id) { return id.HashValue; }

    CONSTEXPR bool operator ==(const TNamedId& other) const { return (HashValue == other.HashValue); }
    CONSTEXPR bool operator !=(const TNamedId& other) const { return (HashValue != other.HashValue); }

    CONSTEXPR bool operator < (const TNamedId& other) const { return (HashValue <  other.HashValue); }
    CONSTEXPR bool operator >=(const TNamedId& other) const { return (HashValue >= other.HashValue); }

    CONSTEXPR bool operator > (const TNamedId& other) const { return (HashValue >  other.HashValue); }
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
    CONSTEXPR TResourceId(FIndex index, FInstanceID instanceID) : Index(index), InstanceID(instanceID) {}

    CONSTEXPR CONSTF bool Valid() const { return (Packed != UMax); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR FResourceHandle Pack() const { return {_Uid, Index, InstanceID}; }

    static CONSTEXPR TResourceId Unpack(FResourceHandle packed) {
        Assert_NoAssume(packed.Uid == _Uid);
        return { packed.Index, packed.InstanceID };
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

    CONSTEXPR explicit TResourceWrappedId(id_t id) : Id(id) {}

    CONSTEXPR CONSTF bool Valid() const { return Id.Valid(); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTEXPR id_t Get() const { return Id; }
    CONSTEXPR operator id_t () const { return Id; }
    CONSTEXPR id_t operator *() const { return Id; }

    CONSTEXPR FResourceHandle Pack() const { return Id.Pack(); }

    static CONSTEXPR TResourceWrappedId Unpack(FResourceHandle packed) {
        Assert_NoAssume(packed.Uid == _Uid);
        return TResourceWrappedId{TResourceId<_Uid>(packed.Index, packed.InstanceID)};
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
// FResourceHandle can be used as a polymorphic resource id
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
#define PPE_RHI_RESOURCESID_TEXTWRITER_DECL(_TYPE) \
    PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, const _TYPE& value); \
    PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, const _TYPE& value);
//----------------------------------------------------------------------------
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FUniformID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FPushConstantID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FDescriptorSetID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FSpecializationID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FVertexID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FVertexBufferID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FMemPoolID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRTShaderID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FGeometryID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FInstanceID)
//----------------------------------------------------------------------------
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawBufferID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawImageID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawGPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawMPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawCPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawRTPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawSamplerID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawDescriptorSetLayoutID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawPipelineResourcesID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawRTSceneID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawRTGeometryID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawRTShaderTableID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawSwapchainID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FLogicalPassID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawMemoryID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawPipelineLayoutID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawRenderPassID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRawFramebufferID)
//----------------------------------------------------------------------------
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FBufferID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FImageID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FGPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FMPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FCPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRTPipelineID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FSamplerID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRTSceneID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRTGeometryID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRTShaderTableID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FSwapchainID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FLogicalPassID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FMemoryID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FPipelineLayoutID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FRenderPassID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FFramebufferID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FPipelineResourcesID)
PPE_RHI_RESOURCESID_TEXTWRITER_DECL(FDescriptorSetLayoutID)
//----------------------------------------------------------------------------
#undef PPE_RHI_RESOURCESID_TEXTWRITER_DECL
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   1, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   2, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   3, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   4, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   5, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   6, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   7, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   8, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<   9, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TNamedId<  10, !USE_PPE_RHIOPTIMIZEIDS >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  1 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  2 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  3 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  4 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  5 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  6 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  7 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  8 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId<  9 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 10 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 11 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 12 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 13 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 14 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 15 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 16 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 17 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceId< 18 >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawBufferID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawImageID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawGPipelineID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawMPipelineID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawCPipelineID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawRTPipelineID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawSamplerID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawRTSceneID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawRTGeometryID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawRTShaderTableID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawSwapchainID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawMemoryID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawPipelineLayoutID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawRenderPassID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawFramebufferID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawPipelineResourcesID >;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_RHI_API) TResourceWrappedId< FRawDescriptorSetLayoutID >;
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// String literal user operator for declaring TNamedId<>
// Defined in PPE namespace for easier access
#define PPE_RHI_RESOURCEID_USERLITERAL_DECL(_TYPE, _ALIAS) \
    CONSTEXPR _TYPE operator "" CONCAT(_, _ALIAS)(const char* str, size_t len) { \
        return _TYPE{ FStringView(str, len) }; \
    }
//----------------------------------------------------------------------------
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FUniformID, uniform)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FPushConstantID, pushconstant)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FDescriptorSetID, descriptorset)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FSpecializationID, specialization)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FVertexID, vertex)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FVertexBufferID, vertexbuffer)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FMemPoolID, mempool)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FRTShaderID, rtshader)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FGeometryID, geometry)
PPE_RHI_RESOURCEID_USERLITERAL_DECL(RHI::FInstanceID, instance)
//----------------------------------------------------------------------------
#undef PPE_RHI_RESOURCEID_USERLITERAL_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
