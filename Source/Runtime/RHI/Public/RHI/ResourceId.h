#pragma once

#include "RHI_fwd.h"

#include "RHI/Config.h"

#include "IO/StaticString.h"
#include "Meta/Hash_fwd.h"

namespace PPE {
namespace RHI {
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
    hash_t HashValue{ 0 };

    TNamedId() = default;

    CONSTEXPR TNamedId(const FStringView& name)
    :   Name(name)
    ,   HashValue(hash_fwdit_constexpr(name.begin(), name.end()))
    {}

    CONSTEXPR bool Valid() const { return !!HashValue; }
    CONSTEXPR FStringView MakeView() const { return Name.Str(); }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (Valid() ? this : nullptr); }

    CONSTEXPR friend hash_t hash_value(const TNamedId& id) { return id.HashValue; }

    CONSTEXPR bool operator ==(const TNamedId& other) const { return (HashValue == other.HashValue && Name == other.Name); }
    CONSTEXPR bool operator !=(const TNamedId& other) const { return (not operator ==(other)); }

    CONSTEXPR bool operator < (const TNamedId& other) const { return (HashValue != other.HashValue ? HashValue < other.HashValue : Name < other.Name); }
    CONSTEXPR bool operator >=(const TNamedId& other) const { return (not operator < (other)); }

    CONSTEXPR bool operator > (const TNamedId& other) const { return (other < *this); }
    CONSTEXPR bool operator <=(const TNamedId& other) const { return (not operator > (other)); }

};
//----------------------------------------------------------------------------
template <u32 _Uid>
struct TNamedId<_Uid, false> {
    hash_t HashValue{ 0 };

    TNamedId() = default;

    CONSTEXPR TNamedId(const FStringView& name)
    :   HashValue(hash_fwdit_constexpr(name.begin(), name.end()))
    {}

    CONSTEXPR bool Valid() const { return !!HashValue; }
    CONSTEXPR FStringView MakeView() const { return FStringView{}; }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (Valid() ? this : nullptr); }

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

    using index_t = u16;
    using instance_t = u16;
    using data_t = u32;
    STATIC_ASSERT(sizeof(index_t) + sizeof(instance_t) == sizeof(data_t));

    union {
        struct {
            index_t Index;
            instance_t Instance;
        };
        data_t Packed;
    };

    CONSTEXPR TResourceId() : Packed(UMax) {}

    TResourceId(const TResourceId&) = default;
    TResourceId& operator =(const TResourceId&) = default;

    explicit CONSTEXPR TResourceId(data_t data) : Packed(data) {}
    explicit CONSTEXPR TResourceId(index_t index, instance_t instance) : Index(index), Instance(instance) {}

    CONSTEXPR bool Valid() const { return (Packed != UMax); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return (Valid() ? this : nullptr); }

    CONSTEXPR bool operator ==(const TResourceId& other) const { return (Packed == other.Packed); }
    CONSTEXPR bool operator !=(const TResourceId& other) const { return (Packed != other.Packed); }

    CONSTEXPR friend hash_t hash_value(const TResourceId& id) { return hash_size_t_constexpr(id.Packed); }

};
PPE_ASSUME_TEMPLATE_AS_POD(TResourceId<_Uid>, u32 _Uid);
//----------------------------------------------------------------------------
template <typename T>
struct TResourceWrappedId;
template <u32 _Uid>
struct TResourceWrappedId< TResourceId<_Uid> > {
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

    CONSTEXPR id_t Release() {
        const id_t result{ Id };
        Id = Default;
        return result;
    }

    CONSTEXPR bool Valid() const { return Id.Valid(); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return (Valid() ? this : nullptr); }

    CONSTEXPR bool operator ==(const TResourceWrappedId& other) const { return (Id == other.Id); }
    CONSTEXPR bool operator !=(const TResourceWrappedId& other) const { return (Id != other.Id); }

    CONSTEXPR friend hash_t hash_value(const TResourceWrappedId& wrapped) { return hash_value(wrapped.Id); }

};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
