#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaObjectHelpers.h"
#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeVirtualTraits.h"

#include "Core/Container/HashMap.h"
#include "Core/IO/FS/Basename.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Maths/ScalarBoundingBox_fwd.h"
#include "Core/Maths/PackedVectors.h"
#include "Core/Maths/PackingHelpers.h"
#include "Core/Time/DateTime.h"
#include "Core/Time/Timestamp.h"

#include <type_traits>

namespace Core {
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator>
class EToken;

namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TMetaTypeTraits;
//----------------------------------------------------------------------------
template <typename T, typename _Enabled = void >
struct TMetaTypeTraitsImpl {
    typedef T wrapped_type;
    typedef T wrapper_type;

    typedef TMetaType<T> meta_type;
    static_assert(meta_type::Enabled, "T is not supported by RTTI");

    static const auto* VirtualTraits() = delete;

    static bool IsDefaultValue(const wrapped_type& value) = delete;

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) = delete;

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) = delete;
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) = delete;

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) = delete;
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) = delete;
};
//----------------------------------------------------------------------------
template <typename T>
struct TMetaTypeTraitsDefault {
    typedef T wrapped_type;
    typedef T wrapper_type;

    typedef TMetaType<T> meta_type;
    static_assert(meta_type::Enabled, "T is not supported by RTTI");

    static const auto* VirtualTraits() { return meta_type::VirtualTraits(); }

    static bool IsDefaultValue(const wrapped_type& value) { return meta_type::IsDefaultValue(value); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return meta_type::DeepEquals(lhs, rhs); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = std::move(src); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = std::move(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Enums:
//----------------------------------------------------------------------------
template <typename T>
struct TMetaTypeTraitsImpl< T, typename std::enable_if< std::is_enum<T>::value >::type > {
    typedef T wrapped_type;
    typedef typename std::conditional<sizeof(T) < sizeof(u64), u32, u64>::type wrapper_type;
    STATIC_ASSERT(sizeof(T) <= sizeof(u64));

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return (wrapped_type() == value); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = wrapper_type(src); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = wrapper_type(src); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = wrapped_type(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = wrapped_type(src); }
};
//----------------------------------------------------------------------------
// Strongly typed numeric:
//----------------------------------------------------------------------------
template <typename T, typename _Tag, T _DefaultValue >
struct TMetaTypeTraitsImpl< StronglyTyped::TNumeric<T, _Tag, _DefaultValue> > {
    typedef StronglyTyped::TNumeric<T, _Tag, _DefaultValue> wrapped_type;
    typedef T wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return meta_type::IsDefaultValue(value.Value); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.Value; }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.Value; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst.Value = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst.Value = src; }
};
//----------------------------------------------------------------------------
// Quantized float
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
struct TMetaTypeTraitsImpl< TBasicNorm<T, _Traits> > {
    typedef TBasicNorm<T, _Traits> wrapped_type;
    typedef T wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;
    STATIC_ASSERT(TMetaType< wrapper_type >::Enabled);

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return wrapped_type::DefaultValue() == value; }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src._data; }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src._data; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst._data = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst._data = src; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// MetaObjects :
//----------------------------------------------------------------------------
template <typename T>
struct TMetaTypeTraitsImpl< Core::TRefPtr<T>, typename std::enable_if< std::is_base_of<RTTI::FMetaObject, T>::value >::type > {
    typedef Core::TRefPtr<T> wrapped_type;
    typedef PMetaObject wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return (nullptr == value.get()); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
        return (lhs && rhs) ? RTTI::DeepEquals(*lhs, *rhs) : lhs == rhs;
    }

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
struct TMetaTypeTraitsImpl< std::basic_string<_Char, _Traits, _Allocator> > {
    typedef std::basic_string<_Char, _Traits, _Allocator> wrapped_type;
    typedef TBasicString<_Char> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator>
struct TMetaTypeTraitsImpl< Core::EToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator> > {
    typedef Core::EToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator> wrapped_type;
    typedef TBasicString<_Char> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type >*VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

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
struct TMetaTypeTraitsImpl< Core::TVector<T, _Allocator> > {
    typedef Core::TVector<T, _Allocator> wrapped_type;

    typedef TMetaTypeTraits<T> value_traits;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::TVector< typename value_traits::wrapper_type > wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeVectorTraits< value_wrapper_type > *VirtualTraits() {
        return TMetaTypeVectorTraits< value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs);

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
struct TMetaTypeTraitsImpl< Core::TVectorInSitu<T, _InSituCount, _Allocator> > {
    typedef Core::TVectorInSitu<T, _InSituCount, _Allocator> wrapped_type;

    typedef TMetaTypeTraits<T> value_traits;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::TVector<value_wrapper_type> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeVectorTraits< value_wrapper_type > *VirtualTraits() {
        return TMetaTypeVectorTraits< value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs);

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
struct TMetaTypeTraitsImpl< Core::TPair<_First, _Second> > {
    typedef Core::TPair<_First, _Second> wrapped_type;

    typedef TMetaTypeTraits< Meta::TDecay<_First> > first_traits;
    typedef TMetaTypeTraits< Meta::TDecay<_Second> > second_traits;

    typedef typename first_traits::wrapper_type first_wrapper_type;
    typedef typename second_traits::wrapper_type second_wrapper_type;

    typedef RTTI::TPair<first_wrapper_type, second_wrapper_type> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypePairTraits< first_wrapper_type, second_wrapper_type > *VirtualTraits() {
        return TMetaTypePairTraits< first_wrapper_type, second_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) {
        return  first_traits::IsDefaultValue(value.first) &&
                second_traits::IsDefaultValue(value.second);
    }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
        return  first_traits::DeepEquals(lhs.first, rhs.first) &&
                second_traits::DeepEquals(lhs.second, rhs.second);
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
struct TMetaTypeTraitsImpl< Core::TAssociativeVector<_Key, _Value, _EqualTo, _Vector> > {
    typedef Core::TAssociativeVector<_Key, _Value, _EqualTo, _Vector> wrapped_type;

    typedef TMetaTypeTraits< Meta::TDecay<_Key> > key_traits;
    typedef TMetaTypeTraits< Meta::TDecay<_Value> > value_traits;

    typedef typename key_traits::wrapper_type key_wrapper_type;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::TDictionary<key_wrapper_type, value_wrapper_type> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type > *VirtualTraits() {
        return TMetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs);

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
struct TMetaTypeTraitsImpl< Core::THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> > {
    typedef Core::THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> wrapped_type;

    typedef TMetaTypeTraits< Meta::TDecay<_Key> > key_traits;
    typedef TMetaTypeTraits< Meta::TDecay<_Value> > value_traits;
    typedef TMetaTypeTraits< TPair<_Key, _Value> > pair_traits;

    typedef typename key_traits::wrapper_type key_wrapper_type;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::TDictionary<key_wrapper_type, value_wrapper_type> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type > *VirtualTraits() {
        return TMetaTypeDictionaryTraits< key_wrapper_type, value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs);

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
struct TMetaTypeTraitsImpl< Core::TScalarVector<T, _Dim>, typename std::enable_if< not TMetaType<Core::TScalarVector<T, _Dim>>::Enabled >::type > {
    STATIC_ASSERT(_Dim > 1 && _Dim <= 4);
    typedef Core::TScalarVector<T, _Dim> wrapped_type;

    typedef T value_type;
    typedef TMetaTypeTraits<value_type> value_traits;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef TScalarVector<value_wrapper_type, _Dim> wrapper_type;
    typedef TMetaType< wrapper_type > meta_type;
    STATIC_ASSERT(meta_type::Enabled);

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return (TNumericLimits<wrapped_type>::DefaultValue() == value); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { Wrap_(dst, src); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { Wrap_(dst, src); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { Unwrap_(dst, src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { Unwrap_(dst, src); }

    static void Wrap_(wrapper_type& dst, const wrapped_type& src) {
        forrange(i, 0, _Dim)
            value_traits::WrapCopy(dst[i], src[i]);
    }

    static void Unwrap_(wrapped_type& dst, const wrapper_type& src) {
        forrange(i, 0, _Dim)
            value_traits::UnwrapCopy(dst[i], src[i]);
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TMetaTypeTraitsImpl< Core::TScalarBoundingBox<T, _Dim> > {
    STATIC_ASSERT(_Dim > 1 && _Dim <= 4);
    typedef Core::TScalarBoundingBox<T, _Dim> wrapped_type;

    typedef Core::TScalarVector<T, _Dim> value_type;
    typedef TMetaTypeTraits<value_type> value_traits;
    typedef typename value_traits::wrapper_type value_wrapper_type;

    typedef RTTI::TPair<value_wrapper_type, value_wrapper_type> wrapper_type;
    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypePairTraits< value_wrapper_type, value_wrapper_type > *VirtualTraits() {
        return TMetaTypePairTraits< value_wrapper_type, value_wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return (wrapped_type::DefaultValue() == value); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

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
namespace details {
template <typename T, typename _Enabled = void>
struct bind_traits_ { typedef TMetaTypeTraitsImpl< Meta::TDecay<T> > type; };
template <typename T >
struct bind_traits_<T, typename std::enable_if< TMetaType< Meta::TDecay<T> >::Enabled >::type> { typedef TMetaTypeTraitsDefault< Meta::TDecay<T> > type; };
} //!details
//----------------------------------------------------------------------------
template <typename T>
struct TMetaTypeTraits : details::bind_traits_<T>::type {
    typedef typename details::bind_traits_<T>::type impl_type;

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
struct TMetaTypeTraitsImpl< fake_bool > {
    typedef fake_bool wrapped_type;
    typedef bool wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return fake_bool() == value; }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src; }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
// u128 :
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< u128 > {
    typedef u128 wrapped_type;
    typedef TPair<u64, u64> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypePairTraits< u64, u64 > *VirtualTraits() {
        return TMetaTypePairTraits< u64, u64 >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return 0 == value.lo && 0 == value.hi; }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst.first = src.lo; dst.second = src.hi; }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst.first = src.lo; dst.second = src.hi; }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst.lo = src.first; dst.hi = src.second; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst.lo = src.first; dst.hi = src.second; }
};
//----------------------------------------------------------------------------
// FHalfFloat
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< FHalfFloat > {
    typedef FHalfFloat wrapped_type;
    typedef float wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return FHalfFloat::DefaultValue() == value; }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.Unpack(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.Unpack(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst.Pack(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst.Pack(src); }
};
//----------------------------------------------------------------------------
// UX10Y10Z10W2N
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< UX10Y10Z10W2N > {
    typedef UX10Y10Z10W2N wrapped_type;
    typedef float4 wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return UX10Y10Z10W2N::DefaultValue() == value; }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.Unpack(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.Unpack(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst.Pack(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst.Pack(src); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FileSystem :
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< FBasename > {
    typedef FBasename wrapped_type;
    typedef TBasicString<typename FileSystem::char_type> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.ToWString(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.ToWString(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< FDirpath > {
    typedef FDirpath wrapped_type;
    typedef TBasicString<FileSystem::char_type> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.ToWString(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.ToWString(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = src; }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = src; }
};
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< FFilename > {
    typedef FFilename wrapped_type;
    typedef TBasicString<FileSystem::char_type> wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return value.empty(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

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
struct TMetaTypeTraitsImpl< FDateTime > {
    typedef FDateTime wrapped_type;
    typedef u64 wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return 0 == value.Ord(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) { dst = src.Ord(); }
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) { dst = src.Ord(); }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) { dst = FDateTime(src); }
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) { dst = FDateTime(src); }
};
//----------------------------------------------------------------------------
template <>
struct TMetaTypeTraitsImpl< FTimestamp > {
    typedef FTimestamp wrapped_type;
    typedef i64 wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) { return 0 == value.Value(); }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) { return lhs == rhs; }

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
