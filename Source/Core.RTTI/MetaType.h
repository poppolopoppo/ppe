#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"

#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Transform/ScalarMatrix_fwd.h"

#include "Core/Memory/RefPtr.h"

#include "Core/Meta/OneTimeInitialize.h"

#include <iosfwd>

#define CORE_RTTI_METATYPE_NAME_CAPACITY 128

#include "Core.RTTI/MetaType.Definitions-inl.h"

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
    static MetaTypeId Id() = delete;
    static MetaTypeFlags Flags() = delete;
    static const char *Name() = delete;
    static T DefaultValue() = delete;
    static bool IsDefaultValue(const T& value) = delete;
    static hash_t HashValue(const T& value) = delete;
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
template <typename T>
using Vector = VECTORINSITU(Container, T, 4);
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using Pair = Core::Pair<_Key, _Value>;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using Dictionary = Core::AssociativeVector<
    _Key,
    _Value,
    EqualTo<_Key>,
    RTTI::Vector<RTTI::Pair<_Key COMMA _Value> >
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    template <> struct MetaType< T > { \
        enum : MetaTypeId { TypeId = _TypeId }; \
        static MetaTypeId Id(); \
        static MetaTypeFlags Flags() { return MetaTypeFlags::Scalar; } \
        static const char *Name(); \
        static T DefaultValue(); \
        static bool IsDefaultValue(const T& value); \
        static hash_t HashValue(const T& value); \
    };
//----------------------------------------------------------------------------
FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
//----------------------------------------------------------------------------
#undef DEF_METATYPE_SCALAR
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

    static MetaTypeId Id() { return TypeId; }
    static MetaTypeFlags Flags() { return MetaTypeFlags::Pair; }

    NO_INLINE static const char *Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Pair<{0}, {1}>", first_meta_type::Name(), second_meta_type::Name() );
        return gName.c_str();
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
};
//----------------------------------------------------------------------------
template <typename T>
struct MetaType< RTTI::Vector<T> > {
    typedef MetaType< typename std::decay< T >::type >  value_meta_type;

    enum : MetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(MetaTypeFlags::Vector, value_meta_type::TypeId)
    };

    static MetaTypeId Id() { return TypeId; }
    static MetaTypeFlags Flags() { return MetaTypeFlags::Vector; }

    NO_INLINE static const char *Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Vector<{0}>", value_meta_type::Name() );
        return gName.c_str();
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
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
struct MetaType< RTTI::Dictionary<_Key, _Value> > {
    typedef MetaType< typename std::decay< _Key >::type >   key_meta_type;
    typedef MetaType< typename std::decay< _Value >::type > value_meta_type;

    enum : MetaTypeId {
        TypeId = hash_MetaTypeId_constexpr(MetaTypeFlags::Dictionary, key_meta_type::TypeId, value_meta_type::TypeId)
    };

    static MetaTypeId Id() { return TypeId; }
    static MetaTypeFlags Flags() { return MetaTypeFlags::Dictionary; }

    NO_INLINE static const char *Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Dictionary<{0}, {1}>", key_meta_type::Name(), value_meta_type::Name() );
        return gName.c_str();
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
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MetaTypeInfo {
    MetaTypeId      Id;
    MetaTypeFlags   Flags;
    const char *    Name;

    bool operator ==(const MetaTypeInfo& other) const { return Id == other.Id; }
    bool operator !=(const MetaTypeInfo& other) const { return Id != other.Id; }

    inline friend hash_t hash_value(const MetaTypeInfo& typeInfo) {
        return static_cast<size_t>(typeInfo.Id);
    }
};
//----------------------------------------------------------------------------
template <typename T>
MetaTypeInfo TypeInfo() {
    typedef MetaType<T> meta_type;
    return MetaTypeInfo{ meta_type::Id(), meta_type::Flags(), meta_type::Name() };
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const MetaTypeInfo& typeInfo) {
    return oss << typeInfo.Name;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#pragma warning( pop )
