#pragma once

#include "Core/Core.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"

#include "Core/IO/Format.h"
#include "Core/IO/String.h"

#include "Core/Memory/RefPtr.h"

#include "Core/Meta/OneTimeInitialize.h"

#include <iosfwd>

#define CORE_RTTI_METATYPE_NAME_CAPACITY 128

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
typedef uint64_t MetaTypeId;
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
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using Vector = Core::Vector<T, ALLOCATOR(RTTI, T)>;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using Pair = Core::Pair<_Key, _Value>;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using Dictionary = Core::AssociativeVector<
    _Key,
    _Value,
    EqualTo<_Key>,
    ALLOCATOR(RTTI, RTTI::Pair<_Key COMMA _Value>)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_METATYPE_SCALAR(T, _TypeId) \
    template <> struct MetaType< T > { \
        enum : MetaTypeId { TypeId = MetaTypeId(_TypeId) }; \
        static MetaTypeId Id(); \
        static MetaTypeFlags Flags() { return MetaTypeFlags::Scalar; } \
        static const char *Name(); \
        static T DefaultValue(); \
    };
//----------------------------------------------------------------------------
#include "Core/RTTI/Type/MetaType.Definitions-inl.h"
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

    enum : MetaTypeId { TypeId = MetaTypeId(
        ((first_meta_type::TypeId ^ (second_meta_type::TypeId << 8)) << 4) |
        MetaTypeId(MetaTypeFlags::Pair)
        ) };

    static MetaTypeId Id() { return TypeId; }
    static MetaTypeFlags Flags() { return MetaTypeFlags::Pair; }

    static const char *Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Pair<{0}, {1}>", first_meta_type::Name(), second_meta_type::Name() );
        return gName.c_str();
    }

    static RTTI::Pair<_First, _Second> DefaultValue() {
        return RTTI::Pair<_First, _Second>(first_meta_type::DefaultValue(), second_meta_type::DefaultValue());
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct MetaType< RTTI::Vector<T> > {
    typedef MetaType< typename std::decay< T >::type >  value_meta_type;

    enum : MetaTypeId { TypeId = MetaTypeId(
        ((value_meta_type::TypeId) << 4) |
        MetaTypeId(MetaTypeFlags::Vector)
        )
    };

    static MetaTypeId Id() { return TypeId; }
    static MetaTypeFlags Flags() { return MetaTypeFlags::Vector; }

    static const char *Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Vector<{0}>", value_meta_type::Name() );
        return gName.c_str();
    }

    static RTTI::Vector<T> DefaultValue() {
        return RTTI::Vector<T>();
    }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
struct MetaType< RTTI::Dictionary<_Key, _Value> > {
    typedef MetaType< typename std::decay< _Key >::type >   key_meta_type;
    typedef MetaType< typename std::decay< _Value >::type > value_meta_type;

    enum : MetaTypeId { TypeId = MetaTypeId(
        ((key_meta_type::TypeId ^ (value_meta_type::TypeId << 8)) << 4) |
        MetaTypeId(MetaTypeFlags::Dictionary)
        ) };

    static MetaTypeId Id() { return TypeId; }
    static MetaTypeFlags Flags() { return MetaTypeFlags::Dictionary; }

    static const char *Name() {
        ONE_TIME_INITIALIZE(const CORE_RTTI_METATYPE_NAMETYPE, gName,
            "Dictionary<{0}, {1}>", key_meta_type::Name(), value_meta_type::Name() );
        return gName.c_str();
    }

    static RTTI::Dictionary<_Key, _Value> DefaultValue() {
        return RTTI::Dictionary<_Value, _Value>();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MetaTypeInfo {
    MetaTypeId      Id;
    MetaTypeFlags   Flags;
    const char *    Name;
};
//----------------------------------------------------------------------------
template <typename T>
MetaTypeInfo TypeInfo() {
    typedef MetaType<T> meta_type;
    return MetaTypeInfo{ meta_type::Id(), meta_type::Flags(), meta_type::Name() };
}
//----------------------------------------------------------------------------
inline size_t hash_value(const MetaTypeInfo& typeInfo) {
    return static_cast<size_t>(typeInfo.Id);
}
//----------------------------------------------------------------------------
inline bool operator ==(const MetaTypeInfo& lhs, const MetaTypeInfo& rhs) {
    return (lhs.Id == rhs.Id);
}
//----------------------------------------------------------------------------
inline bool operator !=(const MetaTypeInfo& lhs, const MetaTypeInfo& rhs) {
    return !operator ==(lhs, rhs);
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
