#pragma once

#include "Core.h"

#include "MetaType.h"
#include "MetaTypeVirtualTraits.h"

#include "HashMap.h"

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, class _Enabled = void>
struct MetaTypeTraitsImpl {
    typedef T wrapped_type;
    typedef T wrapper_type;

    typedef MetaType<T> meta_type;
    static_assert(meta_type::TypeId, "T is not supported by RTTI");

    static const MetaTypeScalarTraits *VirtualTraits() { return MetaTypeScalarTraits::Instance(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct MetaTypeTraitsImpl< Core::RefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::MetaObject, T>::value>::type > {
    typedef Core::RefPtr<T> wrapped_type;
    typedef PMetaObject wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits *VirtualTraits() { return MetaTypeScalarTraits::Instance(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
struct MetaTypeTraitsImpl< Core::Vector<T, _Allocator> > {
    typedef Core::Vector<T, _Allocator> wrapped_type;

    typedef MetaTypeTraits<T> value_traits;

    typedef RTTI::Vector< typename value_traits::wrapper_type > wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeVectorTraits *VirtualTraits() { return MetaTypeVectorTraits::Instance(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
struct MetaTypeTraitsImpl< Core::Pair<_Key, _Value> > {
    typedef Core::Pair<_Key, _Value> wrapped_type;

    typedef MetaTypeTraits< typename std::decay<_Key>::type > key_traits;
    typedef MetaTypeTraits< typename std::decay<_Value>::type > value_traits;

    typedef RTTI::Pair<
        typename key_traits::wrapper_type
    ,   typename value_traits::wrapper_type
    >   wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypePairTraits *VirtualTraits() { return MetaTypePairTraits::Instance(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Allocator>
struct MetaTypeTraitsImpl< Core::AssociativeVector<_Key, _Value, _EqualTo, _Allocator> > {
    typedef Core::AssociativeVector<_Key, _Value, _EqualTo, _Allocator> wrapped_type;

    typedef MetaTypeTraits< typename std::decay<_Key>::type > key_traits;
    typedef MetaTypeTraits< typename std::decay<_Value>::type > value_traits;

    typedef RTTI::Dictionary<
        typename key_traits::wrapper_type
    ,   typename value_traits::wrapper_type
    >   wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeDictionaryTraits *VirtualTraits() { return MetaTypeDictionaryTraits::Instance(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
struct MetaTypeTraitsImpl< Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> > {
    typedef Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> wrapped_type;

    typedef MetaTypeTraits< typename std::decay<_Key>::type > key_traits;
    typedef MetaTypeTraits< typename std::decay<_Value>::type > value_traits;
    typedef MetaTypeTraits< Pair<_Key, _Value> > pair_traits;

    typedef RTTI::Dictionary<
        typename key_traits::wrapper_type
    ,   typename value_traits::wrapper_type
    >   wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeDictionaryTraits *VirtualTraits() { return MetaTypeDictionaryTraits::Instance(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator>
struct MetaTypeTraitsImpl< Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> > {
    typedef Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> wrapped_type;
    typedef BasicString<_Char> wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits *VirtualTraits() { return MetaTypeScalarTraits::Instance(); }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src);
    static void WrapCopy(wrapper_type& dst, const wrapped_type& src);

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src);
    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src);
};
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

    enum : bool { Wrapping = !std::is_same<wrapped_type, wrapper_type>::value };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "MetaTypeTraits-inl.h"
