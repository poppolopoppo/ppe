#pragma once

#include "Core/Container/Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto Vector<T, _Allocator>::operator=(const Vector& other) -> Vector& {
    if (this == &other)
        return *this;

    typedef typename allocator_traits::propagate_on_container_copy_assignment propagate_type;
    allocator_copy_(other, propagate_type());

    assign(other.begin(), other.end());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto Vector<T, _Allocator>::operator=(Vector&& rvalue) noexcept -> Vector& {
    if (this == &rvalue)
        return *this;

    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    allocator_move_(std::move(rvalue), propagate_type());

    assign(std::move(rvalue));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::allocator_copy_(const allocator_type& other, std::true_type ) {
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
void Vector<T, _Allocator>::allocator_move_(allocator_type&& rvalue, std::true_type ) {
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
void Vector<T, _Allocator>::assign(Vector&& rvalue) {
    typedef typename allocator_traits::propagate_on_container_move_assignment propagate_type;
    assign_rvalue_(std::move(rvalue), propagate_type());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::assign_rvalue_(Vector&& rvalue, std::true_type ) {
    clear_ReleaseMemory();
    Assert(nullptr == _data);
    Assert(0 == _size);
    Assert(0 == _capacity);

    std::swap(_data, rvalue._data);
    std::swap(_size, rvalue._size);
    std::swap(_capacity, rvalue._capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::assign_rvalue_(Vector&& rvalue, std::false_type ) {
    if (allocator_() == rvalue.allocator_())
        assign_rvalue_(std::move(rvalue), std::true_type());
    else
        assign(std::make_move_iterator(rvalue.begin()), std::make_move_iterator(rvalue.end()));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto Vector<T, _Allocator>::assign(_It first, _It last)
    -> typename std::enable_if<Meta::is_iterator<_It>::value>::type {
    typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
    assign_(first, last, iterator_category());
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
void Vector<T, _Allocator>::assign_(_It first, _It last, std::input_iterator_tag ) {
    size_type count = 0;
    for (; count < _size && first != last; ++count, ++first)
        _data[count] = *first;

    Assert((first == last) || (count == _size));
    if (first != last) {
        do {
            Assert(_size == count);
            reserve_AtLeast(++count);
            allocator_traits::construct(*this, _data + _size, *first);
            ++_size;
            ++first;
        } while (first != last);
    }
    else if (_size > count) {
        Assert(_size >= count);
        for (size_type i = count; i < _size; ++i)
            allocator_traits::destroy(*this, _data + i);
        _size = count;
    }
    Assert(_size == count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It, typename _ItCat>
void Vector<T, _Allocator>::assign_(_It first, _It last, _ItCat ) {
    const size_type count = checked_cast<size_type>(std::distance(first, last));
    if (_size >= count) {
        Assert(count <= _capacity);

        for (size_type i = _size; i < count; ++i)
            allocator_traits::destroy(*this, _data + i);

        if (0 < count)
            std::copy(first, last, CORE_CHECKED_ARRAY_ITERATOR(pointer, _data, count));
    }
    else {
        if (_capacity < count)
            reserve_Exactly(count);

        _It pivot = first;
        std::advance(pivot, _size);
        std::copy(first, pivot, CORE_CHECKED_ARRAY_ITERATOR(pointer, _data, _size));
        std::uninitialized_copy(pivot, last, CORE_CHECKED_ARRAY_ITERATOR(pointer, _data + _size, count - _size));
    }
    _size = count;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::assign(size_type count, const T& value) {
    if (_size >= count) {
        Assert(count <= _capacity);
        for (size_type i = _size; i < count; ++i)
            allocator_traits::destroy(*this, _data + i);
        if (0 < count)
            std::fill_n(CORE_CHECKED_ARRAY_ITERATOR(pointer, _data, count), count, value);
    }
    else {
        if (_capacity < count)
            reserve_Exactly(count);
        std::fill_n(CORE_CHECKED_ARRAY_ITERATOR(pointer, _data, _size), _size, value);
        std::uninitialized_fill_n(CORE_CHECKED_ARRAY_ITERATOR(pointer, _data + _size, count - _size), count - _size, value);
    }
    _size = count;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
auto Vector<T, _Allocator>::emplace(const_iterator pos, _Args&&... args) -> iterator {
    Assert(pos.AliasesToContainer(*this));
    const size_type i = std::distance<const_iterator>(begin(), pos);
    emplace_back(std::forward<_Args>(args)...);
    std::rotate(begin() + i, begin() + (_size - 1), end());
    return iterator(_data + i);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <class... _Args>
void Vector<T, _Allocator>::emplace_back(_Args&&... args) {
    reserve_Additional(1);
    Assert(_size < _capacity);
    allocator_traits::construct(*this, &_data[_size++], std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::emplace_back(const T& value) {
    if (AliasesToContainer(&value)) {
        T tmp(value); // value points to something in this container
        reserve_Additional(1);
        Assert(_size < _capacity);
        allocator_traits::construct(*this, &_data[_size++], std::move(tmp));
    }
    else {
        reserve_Additional(1);
        Assert(_size < _capacity);
        allocator_traits::construct(*this, &_data[_size++], value);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::emplace_back(T&& rvalue) {
    if (AliasesToContainer(&rvalue)) {
        T tmp(std::move(rvalue)); // value points to something in this container
        reserve_Additional(1);
        Assert(_size < _capacity);
        allocator_traits::construct(*this, &_data[_size++], std::move(tmp));
    }
    else {
        reserve_Additional(1);
        Assert(_size < _capacity);
        allocator_traits::construct(*this, &_data[_size++], std::move(rvalue));
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto Vector<T, _Allocator>::erase(const_iterator pos) -> iterator {
    Assert(pos.AliasesToContainer(*this));
    iterator p(const_cast<pointer>(pos.data()));
    std::rotate(p, p + 1, end());
    pop_back();
    return p;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto Vector<T, _Allocator>::erase(const_iterator first, const_iterator last) -> iterator {
    Assert(first.AliasesToContainer(*this));
    Assert(last.AliasesToContainer(*this));
    Assert(first <= last);
    const size_type p = std::distance<const_iterator>(begin(), first);
    const size_type n = std::distance<const_iterator>(first, last);
    Assert(p + n <= _size);
    iterator f(const_cast<pointer>(first.data()));
    iterator l(const_cast<pointer>(last.data()));
    std::rotate(f, l, end());
    for (size_type i = _size - n; i < _size; ++i)
        allocator_traits::destroy(*this, &_data[i]);
    _size -= n;
    return iterator(_data + p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::erase_DontPreserveOrder(const_iterator pos) {
    Assert(pos.AliasesToContainer(*this));
    Assert(_size > 0);
    iterator p(const_cast<pointer>(pos.data()));
    iterator b = end() - 1;
    if (p != b)
        std::swap(*p, *b);
    pop_back();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto Vector<T, _Allocator>::insert(const_iterator pos, _It first, _It last)
    -> typename std::enable_if<Meta::is_iterator<_It>::value, iterator>::type  {
    typedef typename std::iterator_traits<_It>::iterator_category iterator_category;
    const iterator it = insert_(pos, first, last, iterator_category());
    Assert(CheckInvariants());
    return it;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It>
auto Vector<T, _Allocator>::insert_(const_iterator pos, _It first, _It last, std::input_iterator_tag ) -> iterator {
    Assert(pos.AliasesToContainer(*this));
    const size_type p = std::distance<const_iterator>(begin(), pos);
    const size_type o = _size;

    size_type count = 0;
    for (; first != last; ++count, ++first) {
        Assert(_size == o + count);
        reserve_AtLeast(o + count + 1);
        allocator_traits::construct(*this, &_data[o + count], *first);
        ++_size;
    }
    Assert(p <= count);
    Assert(o + count == _size);

    std::rotate(begin() + p, begin() + o, begin() + (o + count));
    return iterator(begin() + p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
template <typename _It, typename _ItCat>
auto Vector<T, _Allocator>::insert_(const_iterator pos, _It first, _It last, _ItCat ) -> iterator {
    Assert(pos.AliasesToContainer(*this));
    const size_type p = std::distance<const_iterator>(begin(), pos);
    const size_type count = std::distance(first, last);
    Assert(p <= count);
    if (0 < count) {
        reserve_Additional(count);
        std::uninitialized_copy(first, last, CORE_CHECKED_ARRAY_ITERATOR(pointer, _data + _size, _capacity - _size));
        std::rotate(begin() + p, begin() + _size, begin() + (_size + count));
        _size += count;
        Assert(_capacity >= _size);
    }
    return iterator(_data + p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
auto Vector<T, _Allocator>::insert(const_iterator pos, size_type count, const T& value) -> iterator {
    Assert(pos.AliasesToContainer(*this));
    reserve_Additional(count);
    const size_type p = std::distance<const_iterator>(begin(), pos);
    std::uninitialized_fill_n(CORE_CHECKED_ARRAY_ITERATOR(pointer, _data + _size, _capacity - _size), count, value);
    std::rotate(begin() + p, begin() + (p + count), begin() + (_size + count));
    _size += count;
    Assert(_capacity >= _size);
    return iterator(_data + p);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::pop_back() {
    AssertRelease(_size > 0);
    allocator_traits::destroy(*this, &_data[--_size]);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::clear() {
    for (size_type i = 0; i < _size; ++i)
        allocator_traits::destroy(*this, &_data[i]);
    _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::clear_ReleaseMemory() {
    clear();
    Assert(0 == _size);
    if (nullptr != _data) {
        Assert(0 < _capacity);
        allocator_traits::deallocate(*this, _data, _capacity);
        _data = nullptr;
        _capacity = 0;
    }
    Assert(0 == _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::reserve(size_type count) {
    if (_capacity < count)
        reserve_Exactly(count);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::reserve_AtLeast(size_type count) {
    if (_capacity < count) {
        size_type newCapacity = (0 == _capacity ? 1  : _capacity);
        while (newCapacity < count) { newCapacity = newCapacity<<1; }
        reserve_Exactly(newCapacity);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::reserve_AssumeEmpty(size_type count) {
    Assert(0 == _size);
    if (_capacity < count) {
        if (_data) {
            Assert(0 < _capacity);
            allocator_traits::deallocate(*this, _data, _capacity);
        }
        _capacity = count;
        _data = allocator_traits::allocate(*this, count);
    }
    Assert(nullptr != _data);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::reserve_Exactly(size_type count) {
    if (_capacity == count)
        return;
    AssertRelease(count >= _size);
#if 1 //%TODO
    _data = Relocate(allocator_(), MemoryView<value_type>(_data, _size), count, _capacity);
    _capacity = count;
#else
    T* newdata = nullptr;
    try {
        newdata = (count > 0)
            ? reinterpret_cast<T*>(allocator_traits::allocate(*this, count))
            : nullptr;
    }
    catch(std::bad_alloc e) {
        return;
    }

    Assert((0 == count) || (nullptr != newdata));
    for (size_type i = 0; i < _size; ++i) {
        allocator_traits::construct(*this, &newdata[i], std::move(_data[i]));
        allocator_traits::destroy(*this, &_data[i]);
    }

    if (_data)
        allocator_traits::deallocate(*this, _data, _capacity);
    else
        Assert(0 == _capacity);

    _data = newdata;
    _capacity = count;
#endif
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::resize(size_type count) {
    if (_size == count)
        return;
    reserve(count);
    if (count < _size) {
        for (size_type i = count; i < _size; ++i)
            allocator_traits::destroy(*this, &_data[i]);
    }
    else {
        for (size_type i = _size; i < count; ++i)
            allocator_traits::construct(*this, &_data[i]);
    }
    _size = count;
    Assert(_size <= _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::resize(size_type count, const_reference value) {
    if (_size == count)
        return;
    reserve(count);
    if (count < _size) {
        for (size_type i = count; i < _size; ++i)
            allocator_traits::destroy(*this, &_data[i]);
    }
    else {
        std::uninitialized_fill_n(CORE_CHECKED_ARRAY_ITERATOR(pointer, _data + _size, _capacity - _size), _capacity - _size, value);
    }
    _size = count;
    Assert(_size <= _capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::resize_AssumeEmpty(size_type count) {
    reserve_AssumeEmpty(count);
    Assert(0 == _size);
    Assert(count <= _capacity);
    for (size_type i = 0; i < count; ++i)
        allocator_traits::construct(*this, &_data[i]);
    _size = count;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::resize_AssumeEmpty(size_type count, const_reference value) {
    reserve_AssumeEmpty(count);
    Assert(0 == _size);
    Assert(count <= _capacity);
    std::uninitialized_fill_n(CORE_CHECKED_ARRAY_ITERATOR(pointer, _data, _capacity), count, value);
    _size = count;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::swap(Vector& other) {
    typedef typename allocator_traits::propagate_on_container_swap propagate_type;
    swap_(other, propagate_type());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::swap_(Vector& other, std::true_type ) {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(other));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Vector<T, _Allocator>::swap_(Vector& other, std::false_type ) {
    reserve(other._size);
    other.reserve(_size);

    Vector* plhs = nullptr;
    Vector* prhs = nullptr;
    if (_size < other._size) {
        plhs = this;
        prhs = &other;
    }
    else {
        plhs = &other;
        prhs = this;
    }

    std::swap_ranges(plhs->begin(), plhs->end(), CORE_CHECKED_ARRAY_ITERATOR(pointer, prhs->_data, prhs->_capacity));

    const size_type n = prhs->_size;
    for (size_type i = plhs->_size; i < n; ++i) {
        allocator_traits::construct(*this, plhs->_data + i);
        std::swap(plhs->_data[i], prhs->_data[i]);
        allocator_traits::destroy(*this, prhs->_data + i);
    }

    std::swap(plhs->_size, prhs->_size);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Vector<T, _Allocator>::CheckInvariants() const {
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
bool operator==(const Vector<T, _Allocator>& lhs, const Vector<T, _Allocator>& rhs) {
    typedef typename Vector<T, _Allocator>::const_pointer const_pointer;
    return (lhs.size() == rhs.size())
        ? std::equal(lhs.begin(), lhs.end(), CORE_CHECKED_ARRAY_ITERATOR(const_pointer, rhs.data(), rhs.size()))
        : false;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator!=(const Vector<T, _Allocator>& lhs, const Vector<T, _Allocator>& rhs) {
    return false == operator==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator< (const Vector<T, _Allocator>& lhs, const Vector<T, _Allocator>& rhs) {
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
bool operator>=(const Vector<T, _Allocator>& lhs, const Vector<T, _Allocator>& rhs) {
    return false == operator< (lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool operator> (const Vector<T, _Allocator>& lhs, const Vector<T, _Allocator>& rhs) {
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
bool operator<=(const Vector<T, _Allocator>& lhs, const Vector<T, _Allocator>& rhs) {
    return false == operator> (lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(Vector<T, _Allocator>& v, const MemoryView<const T>& elts) {
    v.insert(v.end(), elts.begin(), elts.end());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
typename Vector<T, _Allocator>::const_iterator FindFirstOf(const Vector<T, _Allocator>& v, const T& elt) {
    return std::find(v.begin(), v.end(), elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Contains(const Vector<T, _Allocator>& v, const T& elt) {
    return v.end() != FindFirstOf(v, elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, Vector<T, _Allocator>& v, const T& elt) {
    Assert(pIndex);
    const auto it = std::find(v.begin(), v.end(), elt);
    if (v.end() != it) {
        *pIndex = std::distance(v.begin(), v.end());
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, const T& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(elt);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, T&& elt) {
    Assert(!Contains(v, elt));
    v.emplace_back(std::move(elt));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(Vector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(it != v.end());
    v.erase(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(Vector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    if (it == v.end())
        return false;

    v.erase(it);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt) {
    auto it = std::find(v.begin(), v.end(), elt);
    Assert(v.end() != it);
    v.erase_DontPreserveOrder(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt) {
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
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, size_t index) {
    v.erase(v.begin() + index);
}
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, const typename Vector<T, _Allocator>::const_iterator& it) {
    v.erase(it);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(Vector<T, _Allocator>& v) {
    v.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const Vector<T, _Allocator>& vector) {
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
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Core::Vector<T, _Allocator>& vector) {
    oss << "[ ";
    for (const auto& it : vector)
        oss << it << ", ";
    return oss << ']';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
