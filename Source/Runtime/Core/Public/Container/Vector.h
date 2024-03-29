#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/InSituAllocator.h"
#include "Container/Hash.h"
#include "HAL/PlatformMemory.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryView.h"
#include "Memory/RValuePtr.h"
#include "Meta/Iterator.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>

PRAGMA_DISABLE_RUNTIMECHECKS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Uses the default allocator
#define VECTOR(_DOMAIN, T) \
    ::PPE::TVector<COMMA_PROTECT(T), ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
// Allocates minimum N elements, useful to avoid small allocations and fragmentation
#define VECTORMINSIZE(_DOMAIN, T, N) \
    ::PPE::TVector<COMMA_PROTECT(T), ALLOCATOR_MINSIZE(_DOMAIN, COMMA_PROTECT(T), N) >
//----------------------------------------------------------------------------
// Don't allocate for first N elements, use inline storage instead
#define VECTORINSITU(_DOMAIN, T, N) \
    ::PPE::TVector<COMMA_PROTECT(T), INLINE_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T), N) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container) >
class TVector : private _Allocator {
public:
    template <typename U, typename _OtherAllocator>
    friend class TVector;

    typedef _Allocator allocator_type;
    typedef TAllocatorTraits<allocator_type> allocator_traits;

    typedef T value_type;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    typedef TCheckedArrayIterator<value_type> iterator;
    typedef TCheckedArrayIterator<Meta::TAddConst<value_type>> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    TVector() = default;
    ~TVector() {
        Assert(CheckInvariants());
        clear_ReleaseMemory();
    }

    explicit TVector(Meta::FForceInit) NOEXCEPT
    :   allocator_type(Meta::MakeForceInit<allocator_type>()) // used for non default-constructible allocators
    {}

    explicit TVector(allocator_type&& alloc) : allocator_type(std::move(alloc)) {}
    explicit TVector(const allocator_type& alloc) : allocator_type(alloc) {}

    explicit TVector(size_type count) : TVector(ForceInit) { resize_AssumeEmpty(count); }
    TVector(size_type count, const_reference value) : TVector(ForceInit) { resize_AssumeEmpty(count, value); }
    TVector(size_type count, const allocator_type& alloc) : TVector(alloc) { resize_AssumeEmpty(count); }
    TVector(size_type count, const_reference value, const allocator_type& alloc) : TVector(alloc) { resize_AssumeEmpty(count, value); }

    TVector(const TVector& other) : TVector(allocator_traits::SelectOnCopy(other)) { assign_AssumeEmpty(other.begin(), other.end()); }
    TVector(const TVector& other, const allocator_type& alloc) : TVector(alloc) { assign_AssumeEmpty(other.begin(), other.end()); }
    TVector& operator=(const TVector& other);

    TVector(TVector&& rvalue) NOEXCEPT : TVector(Meta::MakeForceInit<allocator_type>()/* handled by MoveAllocatorBlock() */) { assign(std::move(rvalue)); }
    TVector(TVector&& rvalue, const allocator_type& alloc) NOEXCEPT : TVector(alloc) { assign(std::move(rvalue)); }
    TVector& operator=(TVector&& rvalue) NOEXCEPT;

    TVector(std::initializer_list<value_type> ilist) : TVector(ForceInit) { assign_AssumeEmpty(ilist.begin(), ilist.end()); }
    TVector(std::initializer_list<value_type> ilist, const allocator_type& alloc) : TVector(alloc) { assign_AssumeEmpty(ilist.begin(), ilist.end()); }
    TVector& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    TVector(Meta::TDontDeduce<TRValueInitializerList<value_type>> ilist) : TVector(ForceInit) { assign_AssumeEmpty(ilist.begin(), ilist.end()); }
    TVector(Meta::TDontDeduce<TRValueInitializerList<value_type>> ilist, const allocator_type& alloc) : TVector(alloc) { assign_AssumeEmpty(ilist.begin(), ilist.end()); }
    TVector& operator=(Meta::TDontDeduce<TRValueInitializerList<value_type>> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    TVector(const TMemoryView<const value_type>& view) : TVector(ForceInit) { assign_AssumeEmpty(view.begin(), view.end()); }
    TVector(const TMemoryView<const value_type>& view, const allocator_type& alloc) : TVector(alloc) { assign_AssumeEmpty(view.begin(), view.end()); }
    TVector& operator=(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); return *this; }

    template <typename _It>
    TVector(TIterable<_It> range) : TVector(ForceInit) { assign_AssumeEmpty(range.begin(), range.end()); }
    template <typename _It>
    TVector(TIterable<_It> range, allocator_type&& ralloc) : TVector(std::move(ralloc)) { assign_AssumeEmpty(range.begin(), range.end()); }
    template <typename _It>
    TVector(TIterable<_It> range, const allocator_type& alloc) : TVector(alloc) { assign_AssumeEmpty(range.begin(), range.end()); }
    template <typename _It>
    TVector& operator=(TIterable<_It> range) { assign(range); return *this; }

    template <typename _OtherAllocator>
    TVector(const TVector<T, _OtherAllocator>& other) : TVector(ForceInit) { operator =(other); }
    template <typename _OtherAllocator>
    TVector& operator =(const TVector<T, _OtherAllocator>& other) { assign(other.MakeView()); return (*this); }

    template <typename _OtherAllocator, typename = Meta::TEnableIf<has_stealallocatorblock_v<_Allocator, _OtherAllocator>> >
    TVector(TVector<T, _OtherAllocator>&& rvalue) : TVector(ForceInit) { operator =(std::move(rvalue)); }
    template <typename _OtherAllocator, typename = Meta::TEnableIf<has_stealallocatorblock_v<_Allocator, _OtherAllocator>> >
    TVector& operator =(TVector<T, _OtherAllocator>&& rvalue) {
        if (_data)
            clear_ReleaseMemory();

        Assert_NoAssume(_data == nullptr && _allocationSize == 0 && _numElements == 0);

        Verify(TAllocatorTraits<_OtherAllocator>::StealAndAcquire(
            &allocator_traits::Get(*this),
            TAllocatorTraits<_OtherAllocator>::Get(rvalue),
            rvalue.allocator_block_()));

        _allocationSize = rvalue._allocationSize;
        _numElements = rvalue._numElements;
        _data = rvalue._data;

        rvalue._allocationSize = rvalue._numElements = 0;
        rvalue._data = nullptr;

        return (*this);
    }

    template <typename U>
    explicit TVector(const TMemoryView<U>& view) : TVector(ForceInit) { assign(view); }
    template <typename U>
    TVector(const TMemoryView<U>& view, const allocator_type& alloc) : TVector(alloc) { assign(view); }
    template <typename U>
    TVector& operator=(const TMemoryView<U>& view) { assign(view); return *this; }

    template <typename _It>
    TVector(_It first, _It last) : TVector(ForceInit) { assign(first, last); }
    template <typename _It>
    TVector(_It first, _It last, const allocator_type& alloc) : TVector(alloc) { assign(first, last); }

    CONSTF size_type size() const { return _numElements; }
    CONSTF size_type capacity() const { return (_allocationSize / sizeof(T)); }
    CONSTF size_type max_size() const { return std::numeric_limits<size_type>::max() / sizeof(T); }
    CONSTF bool empty() const { return (0 == _numElements); }

    CONSTF pointer data() { return _data; }
    CONSTF const_pointer data() const { return _data; }

    iterator begin() { return MakeCheckedIterator(_data, _numElements, 0); }
    iterator end() { return MakeCheckedIterator(_data, _numElements, _numElements); }

    const_iterator begin() const { return MakeCheckedIterator(const_pointer(_data), _numElements, 0); }
    const_iterator end() const { return MakeCheckedIterator(const_pointer(_data), _numElements, _numElements); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    allocator_type& get_allocator() { return allocator_traits::Get(*this); }
    const allocator_type& get_allocator() const { return allocator_traits::Get(*this); }

    reference at(size_type pos) { Assert_NoAssume(pos < _numElements); return _data[pos]; }
    const_reference at(size_type pos) const { Assert_NoAssume(pos < _numElements); return _data[pos]; }

    reference operator[](size_type pos) { return at(pos); }
    const_reference operator[](size_type pos) const { return at(pos); }

    reference front() { return at(0); }
    const_reference front() const { return at(0); }

    reference back() { return at(_numElements - 1); }
    const_reference back() const { return at(_numElements - 1); }

    template <typename _It>
    Meta::TEnableIf<Meta::is_iterator_v<_It>> assign(_It first, _It last);
    template <typename _It>
    Meta::TEnableIf<Meta::is_iterator_v<_It>> assign_AssumeEmpty(_It first, _It last);

    void assign(size_type count, const T& value);
    void assign(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }
    void assign(TVector&& rvalue);

    template <typename _It>
    auto assign(TIterable<_It> range) { return assign(range.begin(), range.end()); }

    template <typename U>
    void assign(const TMemoryView<U>& view) { assign(view.begin(), view.end()); }
    void assign(const TMemoryView<value_type>& view) { assign(std::make_move_iterator(view.begin()), std::make_move_iterator(view.end())); }
    void assign(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); }

    template <typename _Result, class... _Args>
    using TEnableIfConstructible = Meta::TEnableIf<std::is_constructible_v<T, _Args...>, _Result>;

    template <class... _Args>
    TEnableIfConstructible<iterator, _Args&&...> emplace(const_iterator pos, _Args&&... args);

    template <class... _Args>
    TEnableIfConstructible<void, _Args&&...> emplace_back(_Args&&... args);
    void emplace_back(const T& value);
    void emplace_back(T&& rvalue);

    template <class... _Args>
    TEnableIfConstructible<void, _Args&&...> emplace_back_AssumeNoGrow(_Args&&... args);
    void emplace_back_AssumeNoGrow(const T& value);
    void emplace_back_AssumeNoGrow(T&& rvalue);

    iterator erase(const_iterator pos);
    reverse_iterator erase(const_reverse_iterator pos) { return std::make_reverse_iterator(erase(pos.base() - 1)); }
    iterator erase(const_iterator first, const_iterator last);
    void erase_DontPreserveOrder(const_iterator pos);

    template <typename _It>
    Meta::TEnableIf<Meta::is_iterator_v<_It>, iterator> insert(const_iterator pos, _It first, _It last);
    iterator insert(const_iterator pos, const T& value) { return emplace(pos, value); }
    iterator insert(const_iterator pos, T&& rvalue) { return emplace(pos, std::move(rvalue)); }
    iterator insert(const_iterator pos, size_type count, const T& value);
    iterator insert(const_iterator pos, std::initializer_list<T> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

    template <typename _It>
    auto insert(const_iterator pos, TIterable<_It> range) { return insert(pos, range.begin(), range.end()); }

    template <typename _It>
    auto append(TIterable<_It> range) { return insert(end(), range.begin(), range.end()); }
    auto append(std::initializer_list<T> ilist) { return insert(end(), ilist.begin(), ilist.end()); }

    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& rvalue) { emplace_back(std::move(rvalue)); }
    reference push_back_Default() { emplace_back(); return back(); }
    pointer push_back_Uninitialized() { reserve_Additional(1); return (_data + _numElements++); }
    void push_back_AssumeNoGrow(const T& value);
    void push_back_AssumeNoGrow(T&& rvalue);

    void pop_back();
    value_type pop_back_ReturnBack();

    void clear();
    void clear_ReleaseMemory();
    void reserve(size_type count);
    void reserve_AtLeast(size_type count);
    void reserve_Additional(size_type count) { reserve_AtLeast(_numElements + count); }
    void reserve_AssumeEmpty(size_type count);
    void reserve_Exactly(size_type count);
    void resize(size_type count);
    void resize(size_type count, const_reference value);
    void resize_Uninitialized(size_type count);
    void resize_AssumeEmpty(size_type count);
    void resize_AssumeEmpty(size_type count, const_reference value);
    void shrink_to_fit();

    void swap(TVector& other) NOEXCEPT;
    friend void swap(TVector& lhs, TVector& rhs) NOEXCEPT { lhs.swap(rhs); }

    operator TMemoryView<Meta::TAddConst<value_type>>() const { return MakeConstView(); }

    TMemoryView<value_type> MakeView() const { return TMemoryView<value_type>(_data, _numElements); }
    TMemoryView<Meta::TAddConst<value_type>> MakeConstView() const { return TMemoryView<Meta::TAddConst<value_type>>(_data, _numElements); }

    bool CheckInvariants() const;

    bool AliasesToContainer(const_reference v) const { return ((&v >= _data) && (&v < _data + _numElements)); }
    bool AliasesToContainer(const const_iterator& it) const { return (it >= begin() && it < end()); }
    bool AliasesToContainer(const const_reverse_iterator& it) const { return (it >= rbegin() && it < rend()); }

private:
    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }
    FAllocatorBlock allocator_block_() const { return { _data, _allocationSize }; }

    void allocator_copy_(const allocator_type& other, std::true_type );
    void allocator_copy_(const allocator_type& other, std::false_type ) { Unused(other); }

    template <typename _It>
    void assign_(_It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    void assign_(_It first, _It last, _ItCat );

    template <typename _It>
    iterator insert_(const_iterator pos, _It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    iterator insert_(const_iterator pos, _It first, _It last, _ItCat );

    void swap_(TVector& other, std::true_type  ) NOEXCEPT;
    void swap_(TVector& other, std::false_type ) NOEXCEPT;

    pointer _data{ nullptr };
    u32 _allocationSize{ 0 };
    u32 _numElements{ 0 };
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_DEBUGGING
// memory debugging disables insitu allocators (makes sanitizer's job easier)
template <typename T, u32 _NumInSitu, typename _Allocator>
using TVectorInSitu = TVector<T, _Allocator>;
#else
// alias for templates when macros can't be used
template <typename T, u32 _NumInSitu, typename _Allocator>
using TVectorInSitu = TVector<T, TSegregateAllocator<
    (sizeof(T) * _NumInSitu),
    TInSituAllocator<sizeof(T) * _NumInSitu>,
    _Allocator>>;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(TVector<T, _Allocator>& v, const TMemoryView<const T>& elts);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Append(TVector<T, _Allocator>& v, _It first, _It last);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Assign(TVector<T, _Allocator>& v, _It first, _It last);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
bool Contains(const TVector<T, _Allocator>& v, const U& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
size_t IndexOf(const TVector<T, _Allocator>& v, const U& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, const TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
bool FindPredicateIndexIFP(size_t *pIndex, const TVector<T, _Allocator>& v, const _Pred& pred);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, Meta::TDontDeduce<T>&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Add_Unique(TVector<T, _Allocator>& v, Meta::TDontDeduce<T>&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename... _Args>
auto Emplace_Back(TVector<T, _Allocator>& v, _Args&&... args) -> Meta::TEnableIf<std::is_constructible_v<T, _Args&&...>,
    typename TVector<T, _Allocator>::iterator>;
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
size_t Remove_If(TVector<T, _Allocator>& v, _Pred&& pred);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
size_t Remove_If_DontPreserveOrder(TVector<T, _Allocator>& v, _Pred&& pred);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(TVector<T, _Allocator>& v, const Meta::TDontDeduce<T>& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, size_t index);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, const typename TVector<T, _Allocator>::const_iterator& it);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear(TVector<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(TVector<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Reserve(TVector<T, _Allocator>& v, size_t capacity);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
NODISCARD size_t SizeOf(const TVector<T, _Allocator>& v) NOEXCEPT {
    return v.size();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Resize_DiscardData(TVector<T, _Allocator>& v, size_t size) {
    v.resize(size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const TVector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FTextWriter& operator <<(FTextWriter& oss, const TVector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FWTextWriter& operator <<(FWTextWriter& oss, const TVector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#if 0 // **BAD IDEA** : will invalidate allocated blocks upon growth => use a TSparseArray<> for this
//----------------------------------------------------------------------------
// Use TVector<T> as an in-place allocator :
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
inline void* operator new(size_t sizeInBytes, PPE::TVector<T, _Allocator>& vector) {
    Assert(sizeInBytes == sizeof(T));
    void* const p = vector.push_back_Uninitialized();
    Assert(Meta::IsAlignedPow2(std::alignment_of_v<T>, p));
    return p;
}
template <typename T, typename _Allocator>
inline void operator delete(void* ptr, PPE::TVector<T, _Allocator>& vector) {
    Assert_NoAssume(vector.AliasesToContainer(static_cast<T*>(ptr)));
    AssertNotImplemented(); // can't move elements around the vector
}
#endif

#include "Container/Vector-inl.h"

PRAGMA_RESTORE_RUNTIMECHECKS
