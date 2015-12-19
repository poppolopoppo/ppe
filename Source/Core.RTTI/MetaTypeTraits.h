#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeVirtualTraits.h"

#include "Core/Container/HashMap.h"
#include "Core/IO/FS/Basename.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Maths/Geometry/ScalarBoundingBox_fwd.h"
#include "Core/Time/DateTime.h"
#include "Core/Time/Timestamp.h"

#include <type_traits>

namespace Core {
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator>
class Token;

namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct MetaTypeTraits;
//----------------------------------------------------------------------------
template <typename T, typename _Enabled = void>
struct MetaTypeTraitsImpl {
    typedef T wrapped_type;
    typedef T wrapper_type;

    typedef MetaType<T> meta_type;
    static_assert(meta_type::TypeId, "T is not supported by RTTI");

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return MetaType< wrapper_type >::IsDefaultValue(value); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = std::move(src); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = std::move(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Arithmetic types :
//----------------------------------------------------------------------------
template <typename T>
struct MetaTypeTraitsImpl< T, typename std::enable_if< std::is_arithmetic<T>::value >::type > {
    typedef T wrapped_type;
    typedef T wrapper_type;

    typedef MetaType< wrapper_type > meta_type;
    static_assert(meta_type::TypeId, "T is not supported by RTTI");

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(wrapped_type value) { return MetaType< wrapper_type >::IsDefaultValue(value); }

    template <typename _Dst> static void WrapMove(_Dst& dst, wrapped_type src) { dst = src; }
    template <typename _Dst> static void WrapCopy(_Dst& dst, wrapped_type src) { dst = src; }

    template <typename _Dst> static void UnwrapMove(_Dst& dst, wrapper_type src) { dst = src; }
    template <typename _Dst> static void UnwrapCopy(_Dst& dst, wrapper_type src) { dst = src; }
};
//----------------------------------------------------------------------------
// Strongly typed numeric:
//----------------------------------------------------------------------------
template <typename T, typename _Tag, T _DefaultValue >
struct MetaTypeTraitsImpl< StronglyTyped::Numeric<T, _Tag, _DefaultValue> > {
    typedef StronglyTyped::Numeric<T, _Tag, _DefaultValue> wrapped_type;
    typedef T wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return meta_type::IsDefaultValue(value.Value); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.Value; }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.Value; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst.Value = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst.Value = src; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// MetaObjects :
//----------------------------------------------------------------------------
template <typename T>
struct MetaTypeTraitsImpl< Core::RefPtr<T>, typename std::enable_if< std::is_base_of<RTTI::MetaObject, T>::value >::type > {
    typedef Core::RefPtr<T> wrapped_type;
    typedef PMetaObject wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return (nullptr == value.get()); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Strings :
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator>
struct MetaTypeTraitsImpl< std::basic_string<_Char, _Traits, _Allocator> > {
    typedef std::basic_string<_Char, _Traits, _Allocator> wrapped_type;
    typedef typename DefaultString<_Char>::type wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator>
struct MetaTypeTraitsImpl< Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> > {
    typedef Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> wrapped_type;
    typedef typename DefaultString<_Char>::type wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Vectors :
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
struct MetaTypeTraitsImpl< Core::Vector<T, _Allocator> > {
    typedef Core::Vector<T, _Allocator> wrapped_type;

    typedef MetaTypeTraits<T> value_traits;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::Vector< typename value_traits::wrapper_type > wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeVectorTraits< value_wrapper_type > *VirtualTraits() {
        return MetaTypeVectorTraits< value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
struct MetaTypeTraitsImpl< Core::VectorInSitu<T, _InSituCount, _Allocator> > {
    typedef Core::VectorInSitu<T, _InSituCount, _Allocator> wrapped_type;

    typedef MetaTypeTraits<T> value_traits;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::Vector<value_wrapper_type> wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeVectorTraits< value_wrapper_type > *VirtualTraits() {
        return MetaTypeVectorTraits< value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Pairs :
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
struct MetaTypeTraitsImpl< Core::Pair<_First, _Second> > {
    typedef Core::Pair<_First, _Second> wrapped_type;

    typedef MetaTypeTraits< typename std::decay<_First>::type > first_traits;
    typedef MetaTypeTraits< typename std::decay<_Second>::type > second_traits;

    typedef typename first_traits::wrapper_type first_wrapper_type;
    typedef typename second_traits::wrapper_type second_wrapper_type;

    typedef RTTI::Pair<first_wrapper_type, second_wrapper_type> wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypePairTraits< first_wrapper_type, second_wrapper_type > *VirtualTraits() {
        return MetaTypePairTraits< first_wrapper_type, second_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) {
        return  first_traits::IsDefaultValue(value.first) &&
                second_traits::IsDefaultValue(value.second);
    }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Associative Containers :
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
struct MetaTypeTraitsImpl< Core::AssociativeVector<_Key, _Value, _EqualTo, _Vector> > {
    typedef Core::AssociativeVector<_Key, _Value, _EqualTo, _Vector> wrapped_type;

    typedef MetaTypeTraits< typename std::decay<_Key>::type > key_traits;
    typedef MetaTypeTraits< typename std::decay<_Value>::type > value_traits;

    typedef typename key_traits::wrapper_type key_wrapper_type;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::Dictionary<key_wrapper_type, value_wrapper_type> wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type > *VirtualTraits() {
        return MetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
struct MetaTypeTraitsImpl< Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> > {
    typedef Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> wrapped_type;

    typedef MetaTypeTraits< typename std::decay<_Key>::type > key_traits;
    typedef MetaTypeTraits< typename std::decay<_Value>::type > value_traits;
    typedef MetaTypeTraits< Pair<_Key, _Value> > pair_traits;

    typedef typename key_traits::wrapper_type key_wrapper_type;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::Dictionary<key_wrapper_type, value_wrapper_type> wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type > *VirtualTraits() {
        return MetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Maths :
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct MetaTypeTraitsImpl< Core::ScalarBoundingBox<T, _Dim> > {
    STATIC_ASSERT(_Dim > 1 && _Dim <= 4);
    typedef Core::ScalarBoundingBox<T, _Dim> wrapped_type;

    typedef Core::ScalarVector<T, _Dim> value_type;
    typedef MetaTypeTraits<value_type> value_traits;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::Pair<value_wrapper_type, value_wrapper_type> wrapper_type;
    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypePairTraits< value_wrapper_type, value_wrapper_type > *VirtualTraits() {
        return MetaTypePairTraits< value_wrapper_type, value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return (wrapped_type::DefaultValue() == value); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { Wrap_(dst, src); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { Wrap_(dst, src); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { Unwrap_(dst, src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { Unwrap_(dst, src); }

    static void Wrap_(wrapper_type& dst, const wrapped_type& src) {
        value_traits::WrapCopy(dst.first, src.Min());
        value_traits::WrapCopy(dst.second, src.Max());
    }

    static void Unwrap_(wrapped_type& dst, const wrapper_type& src) {
        value_type vmin, vmax;
        value_traits::UnwrapCopy(vmin, src.first);
        value_traits::UnwrapCopy(vmax, src.second);
        dst = wrapped_type(vmin, vmax);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct MetaTypeTraits : MetaTypeTraitsImpl< typename std::decay<T>::type > {
    typedef MetaTypeTraitsImpl<typename std::decay<T>::type> impl_type;

    using typename impl_type::wrapped_type;
    using typename impl_type::wrapper_type;

    using typename impl_type::meta_type;

    using impl_type::VirtualTraits;

    using impl_type::WrapMove;
    using impl_type::WrapCopy;

    using impl_type::UnwrapMove;
    using impl_type::UnwrapCopy;

    enum : bool { Wrapping = (false == std::is_same<wrapped_type, wrapper_type>::value) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// fake_bool :
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< fake_bool > {
    typedef fake_bool wrapped_type;
    typedef bool wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return fake_bool() == value; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src; }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
// u128 :
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< u128 > {
    typedef u128 wrapped_type;
    typedef Pair<u64, u64> wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypePairTraits< u64, u64 > *VirtualTraits() {
        return MetaTypePairTraits< u64, u64 >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return 0 == value.lo && 0 == value.hi; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst.first = src.lo; dst.second = src.hi; }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst.first = src.lo; dst.second = src.hi; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst.lo = src.first; dst.hi = src.second; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst.lo = src.first; dst.hi = src.second; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FileSystem :
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< Basename > {
    typedef Basename wrapped_type;
    typedef DefaultString<typename FileSystem::char_type>::type wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.ToWString(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.ToWString(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< Dirpath > {
    typedef Dirpath wrapped_type;
    typedef DefaultString<FileSystem::char_type>::type wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.ToWString(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.ToWString(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< Filename > {
    typedef Filename wrapped_type;
    typedef DefaultString<FileSystem::char_type>::type wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.ToWString(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.ToWString(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Time :
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< DateTime > {
    typedef DateTime wrapped_type;
    typedef u64 wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return 0 == value.Ord(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.Ord(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.Ord(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = DateTime(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = DateTime(src); }
};
//----------------------------------------------------------------------------
template <>
struct MetaTypeTraitsImpl< Timestamp > {
    typedef Timestamp wrapped_type;
    typedef i64 wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return 0 == value.Value(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.Value(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.Value(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst.SetValue(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst.SetValue(src); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaTypeTraits-inl.h"
