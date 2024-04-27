#pragma once

#include "Core_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class FSparseDataId : size_t {
    Unknown = 0,
};
//----------------------------------------------------------------------------
template <typename T>
struct TSparseArrayItem {
    T Data;
    FSparseDataId Id;
};
//----------------------------------------------------------------------------
template <typename T>
class TSparseArrayIterator;
//----------------------------------------------------------------------------
template <typename T>
class TBasicSparseArray;
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TSparseArray;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Same API than TVector<>
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(TSparseArray<T, _Allocator>& v, const TMemoryView<const T>& elts);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Append(TSparseArray<T, _Allocator>& v, _It first, _It last);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Assign(TSparseArray<T, _Allocator>& v, _It first, _It last);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
bool Contains(const TSparseArray<T, _Allocator>& v, const U& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TSparseArray<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TSparseArray<T, _Allocator>& v, T&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Add_Unique(TSparseArray<T, _Allocator>& v, T&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename... _Args>
auto Emplace_Back(TSparseArray<T, _Allocator>& v, _Args&&... args) -> typename TSparseArray<T, _Allocator>::iterator;
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TSparseArray<T, _Allocator>& v, const typename TSparseArray<T, _Allocator>::const_iterator& it);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(TSparseArray<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(TSparseArray<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear(TSparseArray<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(TSparseArray<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Reserve(TSparseArray<T, _Allocator>& v, size_t capacity);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const TSparseArray<T, _Allocator>& v) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
