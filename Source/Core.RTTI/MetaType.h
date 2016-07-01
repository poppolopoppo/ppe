#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#include "Core/Memory/RefPtr.h"

#include "Core/Meta/OneTimeInitialize.h"

#include <iosfwd>

#define CORE_RTTI_METATYPE_NAME_CAPACITY 128

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
typedef u32 MetaTypeId;
//----------------------------------------------------------------------------
enum class MetaTypeFlags : u32 {
    Scalar      = 0,
    Vector      = 1,
    Pair        = 2,
    Dictionary  = 3
};
//----------------------------------------------------------------------------
template <typename T>
struct MetaType {
    enum : MetaTypeId { TypeId = 0 };
    static constexpr bool Enabled = false;
    static constexpr MetaTypeId Id() = delete;
    static constexpr MetaTypeFlags Flags() = delete;
    static StringSlice Name() = delete;
    static T DefaultValue() = delete;
    static bool IsDefaultValue(const T& value) = delete;
    static hash_t HashValue(const T& value) = delete;
    static bool DeepEquals(const T& lhs, const T& rhs) = delete;
};
//----------------------------------------------------------------------------
template <typename... _Args>
constexpr MetaTypeId hash_MetaTypeId_constexpr(_Args... args) {
    STATIC_ASSERT(sizeof(MetaTypeId) == sizeof(u32));
    return hash_u32_constexpr(MetaTypeId(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    template <> struct MetaType< T > { \
        enum : MetaTypeId { TypeId = _TypeId }; \
        static constexpr bool Enabled = true; \
        static constexpr MetaTypeId Id() { return _TypeId; } \
        static constexpr MetaTypeFlags Flags() { return MetaTypeFlags::Scalar; } \
        static StringSlice Name(); \
        static T DefaultValue(); \
        static bool IsDefaultValue(const T& value); \
        static hash_t HashValue(const T& value); \
        static bool DeepEquals(const T& lhs, const T& rhs); \
        static const MetaTypeScalarTraits< T >* VirtualTraits(); \
    };
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const AbstractMetaTypeScalarTraits* ScalarTraitsFromTypeId(MetaTypeId typeId);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_RTTI_METATYPE_NAMETYPE StaticFormat<char, CORE_RTTI_METATYPE_NAME_CAPACITY>
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
struct MetaType< RTTI::Pair<_First, _Second> > {
    typedef MetaType< typename std::decay< _First >::type >     first_meta_type;
    typedef MetaType< typename std::decay< _Second >::type >    second_meta_type;

    enum : MetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(MetaTypeFlags::Pair, first_meta_type::TypeId, second_meta_type::TypeId)
    };

    static constexpr bool Enabled = first_meta_type::Enabled && second_meta_type::Enabled;

    static constexpr MetaTypeId Id() { return TypeId; }
    static constexpr MetaTypeFlags Flags() { return MetaTypeFlags::Pair; }

    NO_INLINE static StringSlice Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Pair<{0}, {1}>", first_meta_type::Name(), second_meta_type::Name() );
        return gName.MakeView();
    }

    static RTTI::Pair<_First, _Second> DefaultValue() {
        return RTTI::Pair<_First, _Second>(first_meta_type::DefaultValue(), second_meta_type::DefaultValue());
    }

    static bool IsDefaultValue(const RTTI::Pair<_First, _Second>& pair) {
        return  first_meta_type::IsDefaultValue(pair.first) &&
                second_meta_type::IsDefaultValue(pair.second);
    }

    static hash_t HashValue(const RTTI::Pair<_First, _Second>& pair) {
        return hash_tuple(hash_t(TypeId), first_meta_type::HashValue(pair.first), first_meta_type::HashValue(pair.second));
    }

    static bool DeepEquals(const RTTI::Pair<_First, _Second>& lhs, const RTTI::Pair<_First, _Second>& rhs) {
        return  first_meta_type::DeepEquals(lhs.first, rhs.first) &&
                second_meta_type::DeepEquals(lhs.second, rhs.second);
    }

    static const MetaTypePairTraits< _First, _Second >* VirtualTraits() {
        return MetaTypePairTraits< _First, _Second >::Instance();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct MetaType< RTTI::Vector<T> > {
    typedef MetaType< typename std::decay< T >::type >  value_meta_type;

    enum : MetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(MetaTypeFlags::Vector, value_meta_type::TypeId)
    };

    static constexpr bool Enabled = value_meta_type::Enabled;

    static constexpr MetaTypeId Id() { return TypeId; }
    static constexpr MetaTypeFlags Flags() { return MetaTypeFlags::Vector; }

    NO_INLINE static StringSlice Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Vector<{0}>", value_meta_type::Name() );
        return gName.MakeView();
    }

    static RTTI::Vector<T> DefaultValue() {
        return RTTI::Vector<T>();
    }

    static bool IsDefaultValue(const RTTI::Vector<T>& vector)  {
        return vector.empty();
    }

    static hash_t HashValue(const RTTI::Vector<T>& vector) {
        hash_t result = hash_t(TypeId);
        for (const T& value : vector)
            hash_combine(result, value_meta_type::HashValue(value));
        return result;
    }

    static bool DeepEquals(const RTTI::Vector<T>& lhs, const RTTI::Vector<T>& rhs) {
        if (lhs.size() != rhs.size())
            return false;
        const size_t k = lhs.size();
        for (size_t i = 0; i < k; ++i)
            if (false == value_meta_type::DeepEquals(lhs[i], rhs[i]))
                return false;
        return true;
    }

    static const MetaTypeVectorTraits< T >* VirtualTraits() {
        return MetaTypeVectorTraits< T >::Instance();
    }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
struct MetaType< RTTI::Dictionary<_Key, _Value> > {
    typedef MetaType< typename std::decay< _Key >::type >   key_meta_type;
    typedef MetaType< typename std::decay< _Value >::type > value_meta_type;

    enum : MetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(MetaTypeFlags::Dictionary, key_meta_type::TypeId, value_meta_type::TypeId)
    };

    static constexpr bool Enabled = key_meta_type::Enabled && value_meta_type::Enabled;

    static constexpr MetaTypeId Id() { return TypeId; }
    static constexpr MetaTypeFlags Flags() { return MetaTypeFlags::Dictionary; }

    NO_INLINE static StringSlice Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Dictionary<{0}, {1}>", key_meta_type::Name(), value_meta_type::Name() );
        return gName.MakeView();
    }

    static RTTI::Dictionary<_Key, _Value> DefaultValue() {
        return RTTI::Dictionary<_Key, _Value>();
    }

    static bool IsDefaultValue(const RTTI::Dictionary<_Key, _Value>& dico)  {
        return dico.empty();
    }

    static hash_t HashValue(const RTTI::Dictionary<_Key, _Value>& dico) {
        hash_t result = hash_t(TypeId);
        for (const auto& it : dico)
            result = hash_tuple(result, key_meta_type::HashValue(it.first), key_meta_type::HashValue(it.second));
        return result;
    }

    static bool DeepEquals(const RTTI::Dictionary<_Key, _Value>& lhs, const RTTI::Dictionary<_Key, _Value>& rhs) {
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

    static const MetaTypeDictionaryTraits< _Key, _Value >* VirtualTraits() {
        return MetaTypeDictionaryTraits< _Key, _Value >::Instance();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MetaTypeInfo {
    MetaTypeId Id;
    MetaTypeFlags Flags;
    StringSlice Name;

    bool operator ==(const MetaTypeInfo& other) const { return Id == other.Id; }
    bool operator !=(const MetaTypeInfo& other) const { return Id != other.Id; }

    inline friend hash_t hash_value(const MetaTypeInfo& typeInfo) {
        return static_cast<size_t>(typeInfo.Id);
    }
};
//----------------------------------------------------------------------------
MetaTypeInfo ScalarTypeInfoFromTypeId(MetaTypeId typeId);
//----------------------------------------------------------------------------
template <typename T>
MetaTypeInfo TypeInfo() {
    typedef MetaType<T> meta_type;
    return MetaTypeInfo{ meta_type::Id(), meta_type::Flags(), meta_type::Name() };
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
String ToString(const RTTI::Name& name);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::Name& name) {
    return oss << ToString(name);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const RTTI::BinaryData& rawdata);
//----------------------------------------------------------------------------
String ToString(const RTTI::BinaryData& rawdata);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::BinaryData& rawdata) {
    return oss << ToString(rawdata);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
String ToString(const RTTI::OpaqueData& opaqueData);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::OpaqueData& opaqueData) {
    return oss << ToString(opaqueData);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::MetaTypeInfo& typeInfo) {
    return oss << typeInfo.Name;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#pragma warning( pop )
