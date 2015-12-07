#pragma once

#include "Core.RTTI/MetaTypeTraits.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void MetaTypeTraitsImpl< RefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::MetaObject, T>::value>::type >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    typedef RefPtr< typename std::remove_const<T>::type > refptr_type;
    dst = std::move(*reinterpret_cast<refptr_type*>(&src)); // const objects are considered non const internally
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypeTraitsImpl< RefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::MetaObject, T>::value>::type >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    typedef RefPtr< typename std::remove_const<T>::type > refptr_type;
    dst = *reinterpret_cast<const refptr_type*>(&src); // const objects are considered non const internally
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypeTraitsImpl< RefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::MetaObject, T>::value>::type >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    dst = std::move(src);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypeTraitsImpl< RefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::MetaObject, T>::value>::type >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    dst = src;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator>
void MetaTypeTraitsImpl< Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    dst = src.c_str();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator>
void MetaTypeTraitsImpl< Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    dst = src.c_str();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator>
void MetaTypeTraitsImpl< Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    dst = src;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator>
void MetaTypeTraitsImpl< Core::Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    dst = src;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MetaTypeTraitsImpl< Core::Vector<T, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::WrapMove(dst[i], std::move(src[i]));
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MetaTypeTraitsImpl< Core::Vector<T, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::WrapCopy(dst[i], src[i]);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MetaTypeTraitsImpl< Core::Vector<T, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapMove(dst[i], std::move(src[i]));
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void MetaTypeTraitsImpl< Core::Vector<T, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapCopy(dst[i], src[i]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void MetaTypeTraitsImpl< Core::VectorInSitu<T, _InSituCount, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::WrapMove(dst[i], std::move(src[i]));
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void MetaTypeTraitsImpl< Core::VectorInSitu<T, _InSituCount, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::WrapCopy(dst[i], src[i]);
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void MetaTypeTraitsImpl< Core::VectorInSitu<T, _InSituCount, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapMove(dst[i], std::move(src[i]));
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void MetaTypeTraitsImpl< Core::VectorInSitu<T, _InSituCount, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    const size_t k = src.size();
    dst.resize(k);
    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapCopy(dst[i], src[i]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypeTraitsImpl< Core::Pair<_Key, _Value> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    first_traits::WrapMove(dst.first, std::move(src.first));
    second_traits::WrapMove(dst.second, std::move(src.second));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypeTraitsImpl< Core::Pair<_Key, _Value> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    first_traits::WrapCopy(dst.first, src.first);
    second_traits::WrapCopy(dst.second, src.second);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypeTraitsImpl< Core::Pair<_Key, _Value> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    first_traits::UnwrapMove(dst.first, std::move(src.first));
    second_traits::UnwrapMove(dst.second, std::move(src.second));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypeTraitsImpl< Core::Pair<_Key, _Value> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    first_traits::UnwrapCopy(dst.first, src.first);
    second_traits::UnwrapCopy(dst.second, src.second);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void MetaTypeTraitsImpl< Core::AssociativeVector<_Key, _Value, _EqualTo, _Vector> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    wrapped_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapper_vector dst_vector;
    dst_vector.resize(k);

    for (size_t i = 0; i < k; ++i) {
        key_traits::WrapMove(dst_vector[i].first, std::move(src_vector[i].first));
        value_traits::WrapMove(dst_vector[i].second, std::move(src_vector[i].second));
    }

    dst = std::move(dst_vector);
    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void MetaTypeTraitsImpl< Core::AssociativeVector<_Key, _Value, _EqualTo, _Vector> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    const wrapped_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapper_vector dst_vector;
    dst_vector.resize(k);

    for (size_t i = 0; i < k; ++i) {
        key_traits::WrapCopy(dst_vector[i].first, src_vector[i].first);
        value_traits::WrapCopy(dst_vector[i].second, src_vector[i].second);
    }

    dst = std::move(dst_vector);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void MetaTypeTraitsImpl< Core::AssociativeVector<_Key, _Value, _EqualTo, _Vector> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapped_vector dst_vector;
    Assert(dst_vector.empty());
    dst_vector.resize(k);

    for (size_t i = 0; i < k; ++i) {
        key_traits::UnwrapMove(dst_vector[i].first, std::move(src_vector[i].first));
        value_traits::UnwrapMove(dst_vector[i].second, std::move(src_vector[i].second));
    }

    dst = std::move(dst_vector);
    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void MetaTypeTraitsImpl< Core::AssociativeVector<_Key, _Value, _EqualTo, _Vector> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    const wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapped_vector dst_vector;
    dst_vector.resize(k);

    for (size_t i = 0; i < k; ++i) {
        key_traits::UnwrapCopy(dst_vector[i].first, src_vector[i].first);
        value_traits::UnwrapCopy(dst_vector[i].second, src_vector[i].second);
    }

    dst = std::move(dst_vector);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void MetaTypeTraitsImpl< Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    const size_t k = src.size();

    wrapper_vector& dst_vector = dst.Vector();
    dst_vector.resize(k);

    size_t i = 0;
    for (auto& src_pair : src)
        pair_traits::WrapMove(dst_vector[i++], std::move(src_pair));
    Assert(k == i);

    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void MetaTypeTraitsImpl< Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    const size_t k = src.size();

    wrapper_vector& dst_vector = dst.Vector();
    dst_vector.resize(k);

    size_t i = 0;
    for (const auto& src_pair : src)
        pair_traits::WrapCopy(dst_vector[i++], src_pair);
    Assert(k == i);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void MetaTypeTraitsImpl< Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    dst.clear();
    dst.reserve(k);

    for (auto& src_pair : src) {
        Pair<_Key, _Value> dst_pair;
        pair_traits::UnwrapMove(dst_pair, std::move(src_pair));
        dst.insert(MakePair(std::move(dst_pair.first), std::move(dst_pair.second)));
    }

    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void MetaTypeTraitsImpl< Core::HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    const wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    dst.clear();
    dst.reserve(k);

    for (const auto& src_pair : src) {
        Pair<_Key, _Value> dst_pair;
        pair_traits::UnwrapCopy(dst_pair, std::move(src_pair));
        dst.insert(MakePair(std::move(dst_pair.first), std::move(dst_pair.second)));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
