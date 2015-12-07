#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/InSituAllocator.h"
#include "Core/Container/Hash.h"

#include <utility>
#include <vector>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Allocator>
struct std_vector_no_bool_specialization { typedef std::vector<T, _Allocator> type; };
template <typename _Allocator>
struct std_vector_no_bool_specialization<bool, _Allocator> {
    typedef typename _Allocator::template rebind<fake_bool>::other allocator_type;
    typedef std::vector<fake_bool, allocator_type> type;
};
} //!namespace details
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T)>
class Vector : public details::std_vector_no_bool_specialization<T, _Allocator>::type {
    typedef typename details::std_vector_no_bool_specialization<T, _Allocator>::type parent_type;
public:
    using parent_type::parent_type;
    using parent_type::operator =;
};
//----------------------------------------------------------------------------
#define VECTOR(_DOMAIN, T) \
    ::Core::Vector<COMMA_PROTECT(T), ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTOR_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::Vector<COMMA_PROTECT(T), THREAD_LOCAL_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTORINSITU(_DOMAIN, T, _InSituCount) \
    ::Core::VectorInSitu<COMMA_PROTECT(T), _InSituCount, ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTORINSITU_THREAD_LOCAL(_DOMAIN, T, _InSituCount) \
    ::Core::VectorInSitu<COMMA_PROTECT(T), _InSituCount, THREAD_LOCAL_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(Vector<T, _Allocator>& v, const MemoryView<const T>& elts);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
typename Vector<T, _Allocator>::const_iterator FindFirstOf(const Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Contains(const Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, T&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, size_t index);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, const typename Vector<T, _Allocator>::const_iterator& it);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(Vector<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const Vector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator = ALLOCATOR(Container, T) >
class VectorInSitu :
    private InSituAllocator<T, sizeof(T) * _InSituCount, _Allocator>::storage_type
,   public Vector<T, InSituAllocator<T, sizeof(T) * _InSituCount, _Allocator> > {
public:
    STATIC_ASSERT(_InSituCount > 0);
    enum : size_t { InSituSizeInBytes = sizeof(T) * _InSituCount };
    typedef InSituAllocator<T, InSituSizeInBytes, _Allocator> allocator_type;
    typedef typename allocator_type::storage_type storage_type;
    typedef Vector<T, allocator_type> vector_type;
    typedef VectorInSitu<T, _InSituCount, allocator_type> self_type;

    using vector_type::operator [];
    typedef typename vector_type::value_type value_type;
    typedef typename vector_type::size_type size_type;

    VectorInSitu() : vector_type(allocator_type(static_cast<storage_type&>(*this))) { vector_type::reserve(_InSituCount); }

    VectorInSitu(size_type count) : VectorInSitu() { vector_type::resize(count); }
    VectorInSitu(size_type count, const value_type& value) : VectorInSitu() { vector_type::resize(count, value); }

    template<class _It>
    VectorInSitu(_It first, _It last) : VectorInSitu() { vector_type::insert(vector_type::end(), first, last); }

    VectorInSitu(const VectorInSitu& other) : VectorInSitu() { vector_type::operator =(other); }
    VectorInSitu& operator =(const VectorInSitu& other);

    VectorInSitu(VectorInSitu&& rvalue) : VectorInSitu() { operator =(std::move(rvalue)); }
    VectorInSitu& operator =(VectorInSitu&& rvalue);

    bool UseInSitu() const { return storage_type::Contains(vector_type::data()); }

    friend hash_t hash_value(const VectorInSitu& v) { return hash_value(static_cast<const vector_type&>(v)); }

    template <typename U, size_t N, typename A>
    friend void swap(VectorInSitu<U, N, A>& lhs, VectorInSitu<U, N, A>& rhs);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Char, typename _Traits >
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const std::vector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Vector-inl.h"
