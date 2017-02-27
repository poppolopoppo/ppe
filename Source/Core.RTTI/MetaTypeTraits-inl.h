#pragma once

#include "Core.RTTI/MetaTypeTraits.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypeTraitsImpl< TRefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::FMetaObject, T>::value>::type >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    typedef TRefPtr< Meta::TRemoveConst<T> > refptr_type;
    dst = std::move(*reinterpret_cast<refptr_type*>(&src)); // const objects are considered non const internally
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypeTraitsImpl< TRefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::FMetaObject, T>::value>::type >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    typedef TRefPtr< Meta::TRemoveConst<T> > refptr_type;
    dst = *reinterpret_cast<const refptr_type*>(&src); // const objects are considered non const internally
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypeTraitsImpl< TRefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::FMetaObject, T>::value>::type >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    dst = std::move(src);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypeTraitsImpl< TRefPtr<T>, typename std::enable_if<std::is_base_of<RTTI::FMetaObject, T>::value>::type >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    dst = src;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    dst = src.c_str();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    dst = src.c_str();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    dst = src;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    dst = src;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TMetaTypeTraitsImpl< Core::TVector<T, _Allocator> >::DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
    if (lhs.size() != rhs.size())
        return false;

    const size_t k = lhs.size();
    for (size_t i = 0; i < k; ++i)
        if (false == value_traits::DeepEquals(lhs[i], rhs[i]))
            return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVector<T, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::WrapMove(dst.push_back_Default(), std::move(src[i]));

    Assert(dst.size() == src.size());
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVector<T, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::WrapCopy(dst.push_back_Default(), src[i]);

    Assert(dst.size() == src.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVector<T, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapMove(dst.push_back_Default(), std::move(src[i]));

    Assert(dst.size() == src.size());
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVector<T, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapCopy(dst.push_back_Default(), src[i]);

    Assert(dst.size() == src.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
bool TMetaTypeTraitsImpl< Core::TVectorInSitu<T, _InSituCount, _Allocator> >::DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
    if (lhs.size() != rhs.size())
        return false;

    const size_t k = lhs.size();
    for (size_t i = 0; i < k; ++i)
        if (false == value_traits::DeepEquals(lhs[i], rhs[i]))
            return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVectorInSitu<T, _InSituCount, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::WrapMove(dst.push_back_Default(), std::move(src[i]));

    Assert(dst.size() == src.size());
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVectorInSitu<T, _InSituCount, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::WrapCopy(dst.push_back_Default(), src[i]);

    Assert(dst.size() == src.size());
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVectorInSitu<T, _InSituCount, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapMove(dst.push_back_Default(), std::move(src[i]));

    Assert(dst.size() == src.size());
    src.clear();
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator>
void TMetaTypeTraitsImpl< Core::TVectorInSitu<T, _InSituCount, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    const size_t k = src.size();
    dst.reserve(k);

    for (size_t i = 0; i < k; ++i)
        value_traits::UnwrapCopy(dst.push_back_Default(), src[i]);

    Assert(dst.size() == src.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypeTraitsImpl< Core::TPair<_Key, _Value> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    first_traits::WrapMove(dst.first, std::move(src.first));
    second_traits::WrapMove(dst.second, std::move(src.second));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypeTraitsImpl< Core::TPair<_Key, _Value> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    first_traits::WrapCopy(dst.first, src.first);
    second_traits::WrapCopy(dst.second, src.second);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypeTraitsImpl< Core::TPair<_Key, _Value> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    first_traits::UnwrapMove(dst.first, std::move(src.first));
    second_traits::UnwrapMove(dst.second, std::move(src.second));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypeTraitsImpl< Core::TPair<_Key, _Value> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    first_traits::UnwrapCopy(dst.first, src.first);
    second_traits::UnwrapCopy(dst.second, src.second);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TMetaTypeTraitsImpl< Core::TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >::DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
    if (lhs.size() != rhs.size())
        return false;

    typedef typename wrapped_type::vector_type wrapped_vector;

    const size_t k = lhs.size();
    const wrapped_vector& lhs_vector = lhs.Vector();
    const wrapped_vector& rhs_vector = rhs.Vector();
    for (size_t i = 0; i < k; ++i)
        if (false == key_traits::DeepEquals(lhs_vector[i].first, rhs_vector[i].first) ||
            false == value_traits::DeepEquals(lhs_vector[i].second, rhs_vector[i].second) )
            return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TMetaTypeTraitsImpl< Core::TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    wrapped_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapper_vector dst_vector;
    dst_vector.reserve(k);

    for (size_t i = 0; i < k; ++i) {
        auto& dst_it = dst_vector.push_back_Default();
        key_traits::WrapMove(dst_it.first, std::move(src_vector[i].first));
        value_traits::WrapMove(dst_it.second, std::move(src_vector[i].second));
    }

    dst = std::move(dst_vector);

    Assert(dst.size() == src.size());
    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TMetaTypeTraitsImpl< Core::TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    const wrapped_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapper_vector dst_vector;
    dst_vector.reserve(k);

    for (size_t i = 0; i < k; ++i) {
        auto& dst_it = dst_vector.push_back_Default();
        key_traits::WrapCopy(dst_it.first, src_vector[i].first);
        value_traits::WrapCopy(dst_it.second, src_vector[i].second);
    }

    dst = std::move(dst_vector);
    Assert(dst.size() == src.size());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TMetaTypeTraitsImpl< Core::TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapped_vector dst_vector;
    Assert(dst_vector.empty());
    dst_vector.reserve(k);

    for (size_t i = 0; i < k; ++i) {
        TPair<_Key, _Value>& dst_it = dst_vector.push_back_Default();
        key_traits::UnwrapMove(dst_it.first, std::move(src_vector[i].first));
        value_traits::UnwrapMove(dst_it.second, std::move(src_vector[i].second));
    }

    dst = std::move(dst_vector);

    Assert(dst.size() == src.size());
    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TMetaTypeTraitsImpl< Core::TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;
    typedef typename wrapped_type::vector_type wrapped_vector;

    const wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    wrapped_vector dst_vector;
    dst_vector.reserve(k);

    for (size_t i = 0; i < k; ++i) {
        TPair<_Key, _Value>& dst_it = dst_vector.push_back_Default();
        key_traits::UnwrapCopy(dst_it.first, src_vector[i].first);
        value_traits::UnwrapCopy(dst_it.second, src_vector[i].second);
    }

    dst = std::move(dst_vector);
    Assert(dst.size() == src.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool TMetaTypeTraitsImpl< Core::THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
    if (lhs.size() != rhs.size())
        return false;

    for (const TPair<_Key, _Value>& plhs : lhs) {
        bool found = false;
        for (const TPair<_Key, _Value>& prhs : rhs) {
            if (key_traits::DeepEquals(plhs.first, prhs.first)) {
                found = true;
                if (false == value_traits::DeepEquals(plhs.second, prhs.second))
                    return false;
                break;
            }
        }
        if (false == found)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void TMetaTypeTraitsImpl< Core::THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::WrapMove(wrapper_type& dst, wrapped_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    const size_t k = src.size();

    wrapper_vector& dst_vector = dst.Vector();
    dst_vector.reserve(k);

    for (auto& src_pair : src)
        pair_traits::WrapMove(dst_vector.push_back_Default(), std::move(src_pair));

    Assert(dst.size() == src.size());
    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void TMetaTypeTraitsImpl< Core::THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::WrapCopy(wrapper_type& dst, const wrapped_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    const size_t k = src.size();

    wrapper_vector& dst_vector = dst.Vector();
    dst_vector.resize(k);

    for (const auto& src_pair : src)
        pair_traits::WrapCopy(dst_vector.push_back_Default(), src_pair);

    Assert(dst.size() == src.size());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void TMetaTypeTraitsImpl< Core::THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    dst.clear();
    dst.reserve(k);

    for (auto& src_pair : src) {
        TPair<_Key, _Value> dst_pair;
        pair_traits::UnwrapMove(dst_pair, std::move(src_pair));
        dst.emplace(std::move(dst_pair));
    }

    src.clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void TMetaTypeTraitsImpl< Core::THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >::UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
    typedef typename wrapper_type::vector_type wrapper_vector;

    const wrapper_vector& src_vector = src.Vector();
    const size_t k = src_vector.size();

    dst.clear();
    dst.reserve(k);

    for (const auto& src_pair : src) {
        TPair<_Key, _Value> dst_pair;
        pair_traits::UnwrapCopy(dst_pair, std::move(src_pair));
        dst.emplace(std::move(dst_pair));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
