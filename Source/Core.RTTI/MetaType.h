#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#include "Core/Memory/RefPtr.h"

#include "Core/Meta/OneTimeInitialize.h"

#include <iosfwd>

#define CORE_RTTI_METATYPE_NAME_CAPACITY 48

#include "Core.RTTI/MetaType.Definitions-inl.h"
#include "Core.RTTI/MetaTypeVirtualTraits.h"
#include "Core.RTTI/Typedefs.h"

#pragma warning( push )
// TODO: see constexpr hash_MetaTypeId_constexpr(), is it dangerous ?
#pragma warning( disable : 4307 ) // C4100 'XXX': dépassement de constante intégrale

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
typedef u32 FMetaTypeId;
//----------------------------------------------------------------------------
enum class EMetaTypeFlags : u32 {
    Scalar      = 0,
    Vector      = 1,
    Pair        = 2,
    Dictionary  = 3
};
//----------------------------------------------------------------------------
template <typename T>
struct TMetaType {
    enum : FMetaTypeId { TypeId = 0 };
    static constexpr bool Enabled = false;
    static constexpr FMetaTypeId Id() = delete;
    static constexpr EMetaTypeFlags Flags() = delete;
    static FStringView Name() = delete;
    static T DefaultValue() = delete;
    static bool IsDefaultValue(const T& value) = delete;
    static hash_t HashValue(const T& value) = delete;
    static bool DeepEquals(const T& lhs, const T& rhs) = delete;
};
//----------------------------------------------------------------------------
template <typename... _Args>
constexpr FMetaTypeId hash_MetaTypeId_constexpr(_Args... args) {
    STATIC_ASSERT(sizeof(FMetaTypeId) == sizeof(u32));
    return hash_u32_constexpr(FMetaTypeId(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    template <> struct TMetaType< T > { \
        enum : FMetaTypeId { TypeId = _TypeId }; \
        static constexpr bool Enabled = true; \
        static constexpr FMetaTypeId Id() { return _TypeId; } \
        static constexpr EMetaTypeFlags Flags() { return EMetaTypeFlags::Scalar; } \
        static FStringView Name(); \
        static T DefaultValue(); \
        static bool IsDefaultValue(const T& value); \
        static hash_t HashValue(const T& value); \
        static bool DeepEquals(const T& lhs, const T& rhs); \
        static const TMetaTypeScalarTraits< T >* VirtualTraits(); \
    };
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FAbstractMetaTypeScalarTraits* ScalarTraitsFromTypeId(FMetaTypeId typeId);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_RTTI_METATYPE_NAMETYPE FStaticFormat<char, CORE_RTTI_METATYPE_NAME_CAPACITY>
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
struct TMetaType< RTTI::TPair<_First, _Second> > {
    typedef TMetaType< Meta::TDecay< _First > >     first_meta_type;
    typedef TMetaType< Meta::TDecay< _Second > >    second_meta_type;

    enum : FMetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(EMetaTypeFlags::Pair, first_meta_type::TypeId, second_meta_type::TypeId)
    };

    static constexpr bool Enabled = first_meta_type::Enabled && second_meta_type::Enabled;

    static constexpr FMetaTypeId Id() { return TypeId; }
    static constexpr EMetaTypeFlags Flags() { return EMetaTypeFlags::Pair; }

    NO_INLINE static FStringView Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "TPair<{0}, {1}>", first_meta_type::Name(), second_meta_type::Name() );
        return gName.MakeView();
    }

    static RTTI::TPair<_First, _Second> DefaultValue() {
        return RTTI::TPair<_First, _Second>(first_meta_type::DefaultValue(), second_meta_type::DefaultValue());
    }

    static bool IsDefaultValue(const RTTI::TPair<_First, _Second>& pair) {
        return  first_meta_type::IsDefaultValue(pair.first) &&
                second_meta_type::IsDefaultValue(pair.second);
    }

    static hash_t HashValue(const RTTI::TPair<_First, _Second>& pair) {
        return hash_tuple(hash_t(TypeId), first_meta_type::HashValue(pair.first), first_meta_type::HashValue(pair.second));
    }

    static bool DeepEquals(const RTTI::TPair<_First, _Second>& lhs, const RTTI::TPair<_First, _Second>& rhs) {
        return  first_meta_type::DeepEquals(lhs.first, rhs.first) &&
                second_meta_type::DeepEquals(lhs.second, rhs.second);
    }

    static const TMetaTypePairTraits< _First, _Second >* VirtualTraits() {
        return TMetaTypePairTraits< _First, _Second >::Instance();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TMetaType< RTTI::TVector<T> > {
    typedef TMetaType< Meta::TDecay< T > >  value_meta_type;

    enum : FMetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(EMetaTypeFlags::Vector, value_meta_type::TypeId)
    };

    static constexpr bool Enabled = value_meta_type::Enabled;

    static constexpr FMetaTypeId Id() { return TypeId; }
    static constexpr EMetaTypeFlags Flags() { return EMetaTypeFlags::Vector; }

    NO_INLINE static FStringView Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "TVector<{0}>", value_meta_type::Name() );
        return gName.MakeView();
    }

    static RTTI::TVector<T> DefaultValue() {
        return RTTI::TVector<T>();
    }

    static bool IsDefaultValue(const RTTI::TVector<T>& vector)  {
        return vector.empty();
    }

    static hash_t HashValue(const RTTI::TVector<T>& vector) {
        hash_t result = hash_t(TypeId);
        for (const T& value : vector)
            hash_combine(result, value_meta_type::HashValue(value));
        return result;
    }

    static bool DeepEquals(const RTTI::TVector<T>& lhs, const RTTI::TVector<T>& rhs) {
        if (lhs.size() != rhs.size())
            return false;
        const size_t k = lhs.size();
        for (size_t i = 0; i < k; ++i)
            if (false == value_meta_type::DeepEquals(lhs[i], rhs[i]))
                return false;
        return true;
    }

    static const TMetaTypeVectorTraits< T >* VirtualTraits() {
        return TMetaTypeVectorTraits< T >::Instance();
    }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
struct TMetaType< RTTI::TDictionary<_Key, _Value> > {
    typedef TMetaType< Meta::TDecay< _Key > >   key_meta_type;
    typedef TMetaType< Meta::TDecay< _Value > > value_meta_type;

    enum : FMetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(EMetaTypeFlags::Dictionary, key_meta_type::TypeId, value_meta_type::TypeId)
    };

    static constexpr bool Enabled = key_meta_type::Enabled && value_meta_type::Enabled;

    static constexpr FMetaTypeId Id() { return TypeId; }
    static constexpr EMetaTypeFlags Flags() { return EMetaTypeFlags::Dictionary; }

    NO_INLINE static FStringView Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "TDictionary<{0}, {1}>", key_meta_type::Name(), value_meta_type::Name() );
        return gName.MakeView();
    }

    static RTTI::TDictionary<_Key, _Value> DefaultValue() {
        return RTTI::TDictionary<_Key, _Value>();
    }

    static bool IsDefaultValue(const RTTI::TDictionary<_Key, _Value>& dico)  {
        return dico.empty();
    }

    static hash_t HashValue(const RTTI::TDictionary<_Key, _Value>& dico) {
        hash_t result = hash_t(TypeId);
        for (const auto& it : dico)
            result = hash_tuple(result, key_meta_type::HashValue(it.first), key_meta_type::HashValue(it.second));
        return result;
    }

    static bool DeepEquals(const RTTI::TDictionary<_Key, _Value>& lhs, const RTTI::TDictionary<_Key, _Value>& rhs) {
        if (lhs.size() != rhs.size())
            return false;
        const size_t k = lhs.size();
        const auto& lhs_vector = lhs.Vector();
        const auto& rhs_vector = rhs.Vector();
        for (size_t i = 0; i < k; ++i)
            if (false == key_meta_type::DeepEquals(lhs_vector[i].first, rhs_vector[i].first) ||
                false == value_meta_type::DeepEquals(lhs_vector[i].second, rhs_vector[i].second) )
                return false;
        return true;
    }

    static const TMetaTypeDictionaryTraits< _Key, _Value >* VirtualTraits() {
        return TMetaTypeDictionaryTraits< _Key, _Value >::Instance();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMetaTypeInfo {
    FMetaTypeId Id;
    EMetaTypeFlags Flags;
    FStringView Name;

    bool operator ==(const FMetaTypeInfo& other) const { return Id == other.Id; }
    bool operator !=(const FMetaTypeInfo& other) const { return Id != other.Id; }

    inline friend hash_t hash_value(const FMetaTypeInfo& typeInfo) {
        return static_cast<size_t>(typeInfo.Id);
    }
};
//----------------------------------------------------------------------------
FMetaTypeInfo ScalarTypeInfoFromTypeId(FMetaTypeId typeId);
//----------------------------------------------------------------------------
template <typename T>
FMetaTypeInfo TypeInfo() {
    typedef TMetaType<T> meta_type;
    return FMetaTypeInfo{ meta_type::Id(), meta_type::Flags(), meta_type::Name() };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const RTTI::FName& name);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::FName& name) {
    return oss << ToString(name);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const RTTI::FBinaryData& rawdata);
//----------------------------------------------------------------------------
FString ToString(const RTTI::FBinaryData& rawdata);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::FBinaryData& rawdata) {
    return oss << ToString(rawdata);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const RTTI::FOpaqueData& opaqueData);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::FOpaqueData& opaqueData) {
    return oss << ToString(opaqueData);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::FMetaTypeInfo& typeInfo) {
    return oss << typeInfo.Name;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#pragma warning( pop )
