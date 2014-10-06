#pragma once

#include "Core.h"
#include "Allocation.h"
#include "Hash.h"

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
    ::Core::Vector<T, ALLOCATOR(_DOMAIN, T)>
//----------------------------------------------------------------------------
#define VECTOR_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::Vector<T, THREAD_LOCAL_ALLOCATOR(_DOMAIN, T)>
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
void Insert_AssertUnique(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, const typename Vector<T, _Allocator>::const_iterator& it);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
size_t hash_value(const Vector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Char, typename _Traits >
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const std::vector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Vector-inl.h"
