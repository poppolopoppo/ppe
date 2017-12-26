#pragma once

#include "Core/Container/Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::operator=(const TVector& other) -> TVector& {
    if (this == &other)
        return *this;

    typedef typename allocator_traits::propagate_on_container_copy_assignment propagate_type;
    allocator_copy_(other, propagate_type());

    assign(other.begin(), other.end());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::operator=(TVector&& rvalue) noexcept -> TVector& {
    if (this == &rvalue)
        return *this;

    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    allocator_move_(std::move(rvalue), propagate_type());

    assign(std::move(rvalue));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::allocator_copy_(const allocator_type& other, std::true_type ) {
    if (allocator_() != other) {
        clear_ReleaseMemory();
        Assert(nullptr == _data);
        Assert(0 == _size);
        Assert(0 == _capacity);
        allocator_type::operator=(other);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::allocator_move_(allocator_type&& rvalue, std::true_type ) {
    if (allocator_() != rvalue) {
        clear_ReleaseMemory();
        Assert(nullptr == _data);
        Assert(0 == _size);
        Assert(0 == _capacity);
        allocator_type::operator=(std::move(rvalue));
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::assign(TVector&& rvalue) {
    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    assign_rvalue_(std::move(rvalue), propagate_type());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::assign_rvalue_(TVector&& rvalue, std::true_type ) {
    clear_ReleaseMemory();
    Assert(nullptr == _data);
    Assert(0 == _size);
    Assert(0 == _capacity);

    std::swap(_capacity, rvalue._capacity);
    std::swap(_size, rvalue._size);
    std::swap(_data, rvalue._data);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::assign_rvalue_(TVector&& rvalue, std::false_type ) {
    if (allocator_() == rvalue.allocator_())
        assign_rvalue_(std::move(rvalue), std::true_type());
    else
        assign(std::make_move_iterator(rvalue.begin()), std::make_move_iterator(rvalue.end()));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TVector<T, _Allocator>::assign(_It first, _It last)
    -> typename std::enable_if<Meta::is_iterator<_It>::value>::type {
    typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
    assign_(first, last, iterator_category());
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
void TVector<T, _Allocator>::assign_(_It first, _It last, std::input_iterator_tag ) {
    size_type count = 0;
    for (; count < _size && first != last; ++count, ++first)
        _data[count] = *first;

    Assert((first == last) || (count == _size));
    if (first != last) {
        do {
            Assert(_size == count);

            reserve_AtLeast(++count);
            allocator_traits::construct(allocator_(), _data + _size, *first);

            ++_size;
            ++first;
        } while (first != last);
    }
    else if (_size > count) {
        Assert(_size >= count);

        Destroy(allocator_(), TMemoryView<T>(_data + count, _size - count))

        _size = checked_cast<u32>(count);
    }
    Assert(_size == count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It, typename _ItCat>
void TVector<T, _Allocator>::assign_(_It first, _It last, _ItCat ) {
    const size_type count = checked_cast<size_type>(std::distance(first, last));
    if (_size >= count) {
        Assert(count <= _capacity);

        for (size_type i = _size; i < count; ++i)
            allocator_traits::destroy(*this, _data + i);

        if (0 < count)
            std::copy(first, last, MakeCheckedIterator(_data, _size, 0));
    }
    else {
        if (_capacity < count)
            reserve_Exactly(count);

        _It pivot = first;
        std::advance(pivot, _size);
        std::copy(first, pivot, MakeCheckedIterator(_data, count, 0));
        std::uninitialized_copy(pivot, last, MakeCheckedIterator(_data, count, _size));
    }
    _size = checked_cast<u32>(count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::assign(size_type count, const T& value) {
    if (_size >= count) {
        Assert(count <= _capacity);

        Destroy(allocator_(), TMemoryView<T>(_data + _size, count - _size));

        if (0 < count)
            std::fill_n(MakeCheckedIterator(_data, _size, 0), count, value);
    }
    else {
        if (_capacity < count)
            reserve_Exactly(count);

        std::fill_n(MakeCheckedIterator(_data, count, 0), _size, value);
        std::uninitialized_fill_n(MakeCheckedIterator(_data, count, _size), count - _size, value);
    }
    _size = checked_cast<u32>(count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
auto TVector<T, _Allocator>::emplace(const_iterator pos, _Args&&... args) -> iterator {
    Assert_NoAssume(AliasesToContainer(pos));

    const size_type i = std::distance<const_iterator>(begin(), pos);
    emplace_back(std::forward<_Args>(args)...);

    std::rotate(begin() + i, begin() + (_size - 1), end());

    return MakeCheckedIterator(_data, _size, i);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
void TVector<T, _Allocator>::emplace_back(_Args&&... args) {
    reserve_Additional(1);
    emplace_back_AssumeNoGrow(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back(const T& value) {
    reserve_Additional(1);
    emplace_back_AssumeNoGrow(value);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back(T&& rvalue) {
    reserve_Additional(1);
    emplace_back_AssumeNoGrow(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
void TVector<T, _Allocator>::emplace_back_AssumeNoGrow(_Args&&... args) {
    Assert(_size < _capacity);
    allocator_traits::construct(allocator_(), &_data[_size++], std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back_AssumeNoGrow(const T& value) {
    Assert(_size < _capacity);
    if (AliasesToContainer(&value)) {
        T tmp(value); // value points to something in this container
        allocator_traits::construct(allocator_(), &_data[_size++], std::move(tmp));
    }
    else {
        allocator_traits::construct(allocator_(), &_data[_size++], value);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::emplace_back_AssumeNoGrow(T&& rvalue) {
    Assert(_size < _capacity);
    if (AliasesToContainer(&rvalue)) {
        T tmp(std::move(rvalue)); // value points to something in this container
        allocator_traits::construct(allocator_(), &_data[_size++], std::move(tmp));
    }
    else {
        allocator_traits::construct(allocator_(), &_data[_size++], std::move(rvalue));
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::push_back_AssumeNoGrow(const T& value) {
    AssertRelease(_size < _capacity);
    allocator_traits::construct(allocator_(), &_data[_size++], value);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::push_back_AssumeNoGrow(T&& rvalue) {
    AssertRelease(_size < _capacity);
    allocator_traits::construct(allocator_(), &_data[_size++], std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::erase(const_iterator pos) -> iterator {
    Assert(AliasesToContainer(pos));

    const iterator& p = (const iterator&)(pos);
    std::rotate(p, p + 1, end());

    pop_back();
    return p;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::erase(const_iterator first, const_iterator last) -> iterator {
    Assert(AliasesToContainer(first));
    Assert(AliasesToContainer(last));
    Assert(first <= last);

    const size_type p = std::distance(cbegin(), first);
    const size_type n = std::distance(first, last);
    Assert(p + n <= _size);

    const iterator& f = (const iterator&)(first);
    const iterator& l = (const iterator&)(last);
    std::rotate(f, l, end());

    for (size_type i = _size - n; i < _size; ++i)
        allocator_traits::destroy(allocator_(), &_data[i]);

    _size -= n;
    return MakeCheckedIterator(_data, _capacity, p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::erase_DontPreserveOrder(const_iterator pos) {
    Assert(AliasesToContainer(pos));
    Assert(_size > 0);

    const iterator& p = (const iterator&)(pos);
    const iterator b = end() - 1;
    if (p != b)
        std::swap(*p, *b);

    pop_back();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TVector<T, _Allocator>::insert(const_iterator pos, _It first, _It last)
    -> typename std::enable_if<Meta::is_iterator<_It>::value, iterator>::type  {
    typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
    const iterator it = insert_(pos, first, last, iterator_category());
    Assert(CheckInvariants());
    return it;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto TVector<T, _Allocator>::insert_(const_iterator pos, _It first, _It last, std::input_iterator_tag ) -> iterator {
    Assert(AliasesToContainer(pos));

    const size_type p = std::distance<const_iterator>(begin(), pos);
    const size_type o = _size;

    size_type count = 0;
    for (; first != last; ++count, ++first) {
        Assert(_size == o + count);

        reserve_AtLeast(o + count + 1);

        allocator_traits::construct(allocator_(), &_data[o + count], *first);
        ++_size;
    }
    Assert(p <= count);
    Assert(o + count == _size);

    std::rotate(begin() + p, begin() + o, begin() + (o + count));
    return MakeCheckedIterator(_data, _capacity, p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It, typename _ItCat>
auto TVector<T, _Allocator>::insert_(const_iterator pos, _It first, _It last, _ItCat ) -> iterator {
    Assert(AliasesToContainer(pos));

    const size_type p = std::distance<const_iterator>(begin(), pos);
    const u32 count = checked_cast<u32>(std::distance(first, last));

    Assert(p <= count);
    if (0 < count) {
        reserve_Additional(count);

        std::uninitialized_copy(first, last, MakeCheckedIterator(_data, _capacity, _size));
        std::rotate(begin() + p, begin() + _size, begin() + (_size + count));

        _size += count;
        Assert(_capacity >= _size);
    }
    return MakeCheckedIterator(_data, _capacity, p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::insert(const_iterator pos, size_type count, const T& value) -> iterator {
    Assert(AliasesToContainer(pos));

    reserve_Additional(count);

    std::uninitialized_fill_n(MakeCheckedIterator(_data, _capacity, _size), count, value);

    const size_type p = std::distance<const_iterator>(begin(), pos);
    std::rotate(begin() + p, begin() + (p + count), begin() + (_size + count));

    _size += count;
    Assert(_capacity >= _size);

    return MakeCheckedIterator(_data, _capacity, p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::pop_back() {
    AssertRelease(_size > 0);
    allocator_traits::destroy(allocator_(), &_data[--_size]);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto TVector<T, _Allocator>::pop_back_ReturnBack() -> value_type {
    value_type result = std::move(back());
    pop_back();
    return result;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::clear() {
    Destroy(allocator_(), MakeView());
    _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::clear_ReleaseMemory() {
    clear();
    Assert(0 == _size);
    if (nullptr != _data) {
        Assert(0 < _capacity);
        allocator_traits::deallocate(allocator_(), _data, _capacity);
        _data = nullptr;
        _capacity = 0;
    }
    Assert(0 == _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve(size_type count) {
    if (_capacity < count)
        reserve_Exactly(count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve_AtLeast(size_type count) {
    if (_capacity < count) {
        size_type newCapacity = (0 == _capacity ? 1 : _capacity);

        while (newCapacity < count)
            newCapacity = newCapacity<<1;

        reserve_Exactly(newCapacity);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve_AssumeEmpty(size_type count) {
    Assert(0 == _size);
    if (_capacity < count) {
        if (_data) {
            Assert(0 < _capacity);
            allocator_traits::deallocate(allocator_(), _data, _capacity);
        }
        count = AllocatorSnapSize(allocator_(), count);
        _data = allocator_traits::allocate(allocator_(), count);
        _capacity = checked_cast<u32>(count);
    }
    Assert(nullptr != _data);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::reserve_Exactly(size_type count) {
    count = AllocatorSnapSize(allocator_(), count);
    if (_capacity == count)
        return;
    AssertRelease(count >= _size);
#if 1 //%TODO
    _data = Relocate(allocator_(), MakeView(), count, _capacity);
    _capacity = checked_cast<u32>(count);
#else
    T* newdata = nullptr;
    try {
        newdata = (count > 0)
            ? reinterpret_cast<T*>(allocator_traits::allocate(allocator_(), count))
            : nullptr;
    }
    catch(std::bad_alloc e) {
        return;
    }

    Assert((0 == count) || (nullptr != newdata));
    for (size_type i = 0; i < _size; ++i) {
        allocator_traits::construct(allocator_(), &newdata[i], std::move(_data[i]));
        allocator_traits::destroy(allocator_(), &_data[i]);
    }

    if (_data)
        allocator_traits::deallocate(allocator_(), _data, _capacity);
    else
        Assert(0 == _capacity);

    _data = newdata;
    _capacity = count;
#endif
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize(size_type count) {
    if (_size == count)
        return;

    reserve(count);

    if (count < _size)
        Destroy(allocator_(), TMemoryView<T>(_data + count, _size - count));
    else
        Construct(allocator_(), TMemoryView<T>(_data + _size, count - _size));

    _size = checked_cast<u32>(count);
    Assert(_size <= _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize(size_type count, const_reference value) {
    if (_size == count)
        return;

    reserve(count);

    if (count < _size)
        Destroy(allocator_(), TMemoryView<T>(_data + count, _size - count));
    else
        std::uninitialized_fill_n(MakeCheckedIterator(_data, _capacity, _size), _capacity - _size, value);

    _size = checked_cast<u32>(count);
    Assert(_size <= _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize_Uninitialized(size_type count) {
    if (_size == count)
        return;

    reserve(count);

    if (count < _size)
        Destroy(allocator_(), TMemoryView<T>(_data + count, _size - count));

    _size = checked_cast<u32>(count);
    Assert(_size <= _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize_AssumeEmpty(size_type count) {
    reserve_AssumeEmpty(count);
    Assert(0 == _size);
    Assert(count <= _capacity);

    Construct(allocator_(), TMemoryView<T>(_data, count));

    _size = checked_cast<u32>(count);
    Assert(_size <= _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::resize_AssumeEmpty(size_type count, const_reference value) {
    reserve_AssumeEmpty(count);

    Assert(0 == _size);
    Assert(count <= _capacity);

    std::uninitialized_fill_n(MakeCheckedIterator(_data, _capacity, 0), count, value);

    _size = checked_cast<u32>(count);
    Assert(_size <= _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::swap(TVector& other) {
    typedef typename allocator_traits::propagate_on_container_swap propagate_type;
    if (this != &other)
        swap_(other, propagate_type());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::swap_(TVector& other, std::true_type ) {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(other));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVector<T, _Allocator>::swap_(TVector& other, std::false_type ) {
    TVector* plhs = nullptr;
    TVector* prhs = nullptr;
    if (_size < other._size) {
        plhs = this;
        prhs = &other;
    }
    else {
        plhs = &other;
        prhs = this;
    }

    Assert(plhs->_size <= prhs->_size);
    Assert(plhs->_capacity <= prhs->_capacity);
    std::swap_ranges(plhs->begin(), plhs->end(), MakeCheckedIterator(prhs->_data, prhs->_capacity, 0));

    if (plhs->_size != prhs->_size) {
        Assert(plhs->_size < prhs->_size);
        plhs->reserve(prhs->_size);

        const size_type n = prhs->_size;
        for (size_type i = plhs->_size; i < n; ++i) {
            allocator_traits::construct(allocator_(), plhs->_data + i, std::move(prhs->_data[i]));
            allocator_traits::destroy(allocator_(), prhs->_data + i);
        }

        std::swap(plhs->_size, prhs->_size);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TVector<T, _Allocator>::CheckInvariants() const {
#ifndef NDEBUG
    if (nullptr == _data && (_size || _capacity))
        return false;
    if (nullptr != _data && 0 == _capacity)
        return false;
    if (_size > _capacity)
        return false;
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator==(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    typedef typename TVector<T, _Allocator>::const_pointer const_pointer;
    return (lhs.size() == rhs.size())
        ? std::equal(lhs.begin(), lhs.end(), rhs.begin())
        : false;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator!=(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    return false == operator==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator< (const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    const size_type k = std::min(lhs.size(), rhs.size());
    const T* plhs = lhs.data();
    const T* prhs = rhs.data();
    for (size_t i = 0; i < k; ++i)
        if (plhs[i] >= prhs[i])
            return false;
    return (lhs.size() <= rhs.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator>=(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    return false == operator< (lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator> (const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    const size_type k = std::min(lhs.size(), rhs.size());
    const T* plhs = lhs.data();
    const T* prhs = rhs.data();
    for (size_t i = 0; i < k; ++i)
        if (plhs[i] <= prhs[i])
            return false;
    return (lhs.size() >= rhs.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator<=(const TVector<T, _Allocator>& lhs, const TVector<T, _Allocator>& rhs) {
    return false == operator> (lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(TVector<T, _Allocator>& v, const TMemoryView<const T>& elts) {
    v.insert(v.end(), elts.begin(), elts.end());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
bool Contains(const TVector<T, _Allocator>& v, const U& elt) {
    return (v.end() != std::find(v.begin(), v.end(), elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
size_t IndexOf(const TVector<T, _Allocator>& v, const U& elt) {
    const auto it = std::find(v.begin(), v.end(), elt);
    return (v.end() == it ? std::distance(v.begin(), it) : INDEX_NONE);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, TVector<T, _Allocator>& v, const T& elt) {
    Assert(pIndex);
    const auto it = std::find(v.begin(), v.end(), elt);
    if (v.end() != it) {
        *pIndex = std::distance(v.begin(),it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
bool FindPredicateIndexIFP(size_t *pIndex, TVector<T, _Allocator>& v, const _Pred& pred) {
    Assert(pIndex);
    const auto it = std::find_first_of(v.begin(), v.end(), pred);
    if (v.end() != it) {
        *pIndex = std::distance(v.begin(), it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, const T& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, T&& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(std::move(elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_Unique(TVector<T, _Allocator>& v, T&& elt) {
    if (not Contains(v, elt))
        v.emplace_back(std::move(elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(TVector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(it != v.end());
    v.erase(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(TVector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    if (it == v.end())
        return false;

    v.erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(TVector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(v.end() != it);
    v.erase_DontPreserveOrder(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(TVector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    if (v.end() != it) {
        v.erase_DontPreserveOrder(it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, size_t index) {
    v.erase(v.begin() + index);
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, const typename TVector<T, _Allocator>::const_iterator& it) {
    v.erase(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void clear_ReleaseMemory(TVector<T, _Allocator>& v) {
    v.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const TVector<T, _Allocator>& vector) {
    hash_t h(CORE_HASH_VALUE_SEED);
    for (const T& it : vector)
        hash_combine(h, hash_value(it));
    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename T,
    typename _Allocator,
    typename _Char,
    typename _Traits
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Core::TVector<T, _Allocator>& vector) {
    if (vector.empty()) {
        return oss << "[]";
    }
    else {
        auto it = vector.begin();
        oss << '[' << *it;
        ++it;
        for (const auto end = vector.end(); it != end; ++it)
            oss << ", " << *it;
        return oss << ']';
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
