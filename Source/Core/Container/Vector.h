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
template <typename T, typename _Allocator = ALLOCATOR(Container, T)>
using Vector = std::vector<T, _Allocator>;
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
size_t hash_value(const Vector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSituCount, typename _Allocator = ALLOCATOR(Container, T) >
class VectorInSitu : public std::vector<T, InSituAllocator<T, sizeof(T) * _InSituCount, _Allocator> > {
public:
    enum : size_t { InSituSizeInBytes = sizeof(T) * _InSituCount };
    typedef InSituAllocator<T, InSituSizeInBytes, _Allocator> allocator_type;
    typedef typename allocator_type::storage_type storage_type;
    typedef std::vector<T, allocator_type> vector_type;

    using vector_type::operator [];
    typedef typename vector_type::value_type value_type;
    typedef typename vector_type::size_type size_type;

    VectorInSitu() : vector_type(allocator_type(_inSituData)) { vector_type::reserve(_InSituCount); }

    VectorInSitu(const size_type count) : VectorInSitu() { vector_type::resize(count); }
    VectorInSitu(const size_type count, const value_type& value) : VectorInSitu() { vector_type::resize(count, value); }

    template<class _It>
	VectorInSitu(_It first, _It last) : VectorInSitu() { vector_type::insert(vector_type::end(), first, last); }

    VectorInSitu(const VectorInSitu& other) : VectorInSitu() { vector_type::operator =(other); }
    VectorInSitu& operator =(const VectorInSitu& other) = default;

    VectorInSitu(VectorInSitu&& rvalue) : VectorInSitu() { operator =(std::move(rvalue)); }
    VectorInSitu& operator =(VectorInSitu&& rvalue);

private:
    storage_type _inSituData;
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
