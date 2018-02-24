#pragma once

#include "Core/IO/String.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char>::TBasicString(TBasicStringBuilder<_Char>&& sb) noexcept
    : TBasicString() {
    assign(std::move(sb));
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char>::~TBasicString() {
    Assert(CheckInvariants());

    if (is_large_()) {
        Assert(_large.Storage);
        allocator_traits_::deallocate(get_allocator_(), _large.Storage, _large.Capacity);
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(TBasicString&& rvalue) {
    clear_ReleaseMemory();

    if (rvalue.is_large_()) {
        size_t len;
        auto stolen = rvalue.StealDataUnsafe(get_allocator_(), &len);
        _large = { stolen.data() != nullptr, len, stolen.size(), stolen.data() };
    }
    else {
        assign(rvalue.MakeView());
        rvalue._large = { 0, 0, 0, nullptr };
    }

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(const stringview_type& str) {
    clear();

    if (not str.empty()) {
        auto dst = resizeWNullChar_(str.size());
        std::copy(str.begin(), str.end(), dst);
    }

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(size_t n, _Char fill) {
    Assert(n);

    clear();

    auto dst = resizeWNullChar_(n);
    std::fill_n(dst, n, fill);

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::append(size_t n, _Char fill) {
    Assert(n);

    const size_t oldSize = size();
    const size_t newSize = (oldSize + n);

    auto dst = resizeWNullChar_(newSize);
    std::fill_n(dst + oldSize, n, fill);

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::append(const stringview_type& str) {
    Assert(not str.empty());

    const size_t oldSize = size();
    const size_t newSize = (oldSize + str.size());

    auto dst = resizeWNullChar_(newSize);
    std::copy(str.begin(), str.end(), dst + oldSize);

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::insert(size_t pos, size_t n, _Char fill) {
    Assert(n);

    const size_t oldSize = size();
    if (pos == npos) pos = oldSize;
    const size_t newSize = (oldSize + n);

    auto dst = resizeWNullChar_(newSize);
    std::fill_n(dst + oldSize, n, fill);
    std::rotate(dst + pos, dst + oldSize, dst + newSize);

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::insert(size_t pos, const stringview_type& str) {
    Assert(not str.empty());

    const size_t oldSize = size();
    const size_t newSize = (oldSize + str.size());
    if (pos == npos) pos = oldSize;

    auto dst = resizeWNullChar_(newSize);
    std::copy(str.begin(), str.end(), dst + oldSize);
    std::rotate(dst + pos, dst + oldSize, dst + newSize);

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::erase(size_t pos/* = 0 */, size_t len/* = npos */) {
    const size_t oldSize = size();
    if (len == npos) len = (oldSize - pos);
    Assert(pos + len <= oldSize);

    const size_t newSize = (oldSize - len);
    auto dst = MakeCheckedIterator(data_(), oldSize, 0);
    std::rotate(dst + pos, dst + (pos + len), dst + oldSize);
    dst[newSize] = _Char();

    if (is_large_())
        _large.Size = newSize;
    else
        _small.Size = checked_cast<_Char>(newSize);
    Assert(size() == newSize);

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::replace(size_t pos, size_t len, const stringview_type& str) {
    const size_t oldSize = size();
    if (len == npos) len = (oldSize - pos);
    Assert(pos + len <= oldSize);

    const size_t newSize = (oldSize - len + str.size());
    auto dst = resizeNoNullChar_(newSize);

    if (newSize < oldSize)
        std::rotate(dst + (pos + str.size()), dst + (pos + len), dst + oldSize);
    else if (newSize > oldSize)
        std::rotate(dst + (pos + len), dst + oldSize, dst + newSize);

    std::copy(str.begin(), str.end(), dst + pos);
    dst[newSize] = _Char();

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::replace(size_t pos, size_t len, size_t n, _Char fill) {
    const size_t oldSize = size();
    if (len == npos) len = (oldSize - pos);
    Assert(pos + len <= oldSize);

    const size_t newSize = (oldSize - len + n);
    auto dst = resizeNoNullChar_(newSize);

    if (newSize < oldSize)
        std::rotate(dst + (pos + n), dst + (pos + len), dst + oldSize);
    else if (newSize > oldSize)
        std::rotate(dst + (pos + len), dst + oldSize, dst + newSize);

    std::fill_n(dst + pos, n, fill);
    dst[newSize] = _Char();

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
size_t TBasicString<_Char>::find(const stringview_type& str, size_t pos/* = 0 */) const noexcept {
    Assert(not str.empty());

    auto it = std::search(begin() + pos, end(), str.begin(), str.end());
    return (it == end() ? npos : it - begin());
}
//----------------------------------------------------------------------------
template <typename _Char>
size_t TBasicString<_Char>::rfind(const stringview_type& str, size_t pos/* = npos */) const noexcept {
    Assert(not str.empty());

    auto it = std::search(pos == npos ? rbegin() : const_reverse_iterator(begin() + pos), rend(), str.rbegin(), str.rend());
    return (it == rend() ? npos : rend() - it);
}
//----------------------------------------------------------------------------
template <typename _Char>
auto TBasicString<_Char>::substr(size_t pos/* = 0 */, size_t len/* = npos */) const -> TBasicString {
    return TBasicString(MakeView().SubRange(pos, len));
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::resize(size_t newSize, _Char fill) {
    const size_t oldSize = size();
    auto dst = resizeNoNullChar_(newSize);

    if (newSize > oldSize)
        std::fill_n(dst + oldSize, newSize - oldSize, fill);

    dst[newSize] = _Char();

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::shrink_to_fit() {
    if (is_large_()) {
        const size_t newCapacity = SafeAllocatorSnapSize(get_allocator_(), _large.Size + 1/* null char */);

        if (newCapacity < _large.Capacity) {
            if (USE_CORE_BASICSTRING_SBO && newCapacity <= FSmallString_::GCapacity) {
                _Char* const largeStorage = _large.Storage;
                const size_t largeCapacity = _large.Capacity;
                ::memcpy(_small.Buffer, _large.Storage, (_large.Size + 1/* null char */) * sizeof(_Char));
                _small.IsLarge = 0;
                _small.Size = checked_cast<_Char>(_large.Size);
                get_allocator_().deallocate(largeStorage, largeCapacity);
            }
            else {
                _large.Storage = Relocate_AssumePod(
                    get_allocator_(),
                    mutableview_type(_large.Storage, _large.Size + 1/* null char */),
                    newCapacity, _large.Capacity );
                _large.Capacity = newCapacity;
            }
        }
        else {
            Assert(newCapacity == _large.Capacity);
        }
    }

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::clear() {
    if (is_large_()) {
        _large.Size = 0;
        if (_large.Storage)
            _large.Storage[0] = _Char();
    }
    else {
        _small.Size = 0;
        _small.Buffer[0] = _Char();
    }

    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::clear_ReleaseMemory() {
    if (is_large_()) {
        Assert(_large.Storage);
        allocator_traits::deallocate(get_allocator_(), _large.Storage, _large.Capacity);
    }

    _large = { 0, 0, 0, nullptr };

    Assert(not is_large_());
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, const TBasicString<_Char>& rhs) {
    const size_t newSize = (lhs.size() + rhs.size());
    TBasicString<_Char> result;
    result.reserve(newSize);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, TBasicString<_Char>&& rhs) {
    if (capacity < rhs.capacity()) {
        rhs.insert(0, lhs);
        return std::move(rhs);
    }
    else {
        lhs.append(rhs);
        return std::move(lhs);
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, const TBasicString<_Char>& rhs) {
    lhs.append(rhs);
    return std::move(lhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, TBasicString<_Char>&& rhs) {
    rhs.insert(0, lhs);
    return std::move(rhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, const TBasicStringView<_Char>& rhs) {
    const size_t newSize = (lhs.size() + rhs.size());
    TBasicString<_Char> result;
    result.reserve(newSize);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, const TBasicStringView<_Char>& rhs) {
    lhs.append(rhs);
    return std::move(lhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicStringView<_Char>& lhs, const TBasicString<_Char>& rhs) {
    const size_t newSize = (lhs.size() + rhs.size());
    TBasicString<_Char> result;
    result.reserve(newSize);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicStringView<_Char>& lhs, TBasicString<_Char>&& rhs) {
    rhs.insert(0, lhs);
    return std::move(rhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, _Char rhs) {
    TBasicString<_Char> result;
    result.reserve(lhs.size() + 1);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, _Char rhs) {
    lhs.append(rhs);
    return std::move(lhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(_Char lhs, const TBasicString<_Char>& rhs) {
    TBasicString<_Char> result;
    result.reserve(rhs.size() + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(_Char lhs, TBasicString<_Char>&& rhs) {
    rhs.insert(0, lhs);
    return std::move(rhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicString<_Char>::gsub(_Char from, _Char to) {
    bool modified = false;
    for (_Char& ch : MutableView()) {
        if (ch == from) {
            ch = to;
            modified = true;
        }
    }
    return modified;
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicString<_Char>::gsub(_Char from, const stringview_type& to) {
    Assert(to.empty());

    bool modified = false;
    forrange(pos, 0, size()) {
        if (at(pos) == from) {
            replace(pos, 1, to);
            pos += (to.size() - 1);
            modified = true;
        }
    }

    return modified;
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicString<_Char>::gsub(const stringview_type& from, const stringview_type& to) {
    bool modified = false;
    for (size_t pos = 0;;) {
        pos = find(from, pos);
        if (npos == pos)
            break;

        replace(pos, from.size(), to);
        pos += to.size();
        modified = true;
    }
    return modified;
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(TBasicStringBuilder<_Char>&& sb) {
    clear_ReleaseMemory();

    const stringview_type written = sb.Written();
    Assert(written.back() == _Char());

    if (USE_CORE_BASICSTRING_SBO && written.size() <= FSmallString_::GCapacity) {
        _small.IsLarge = 0;
        _small.Size = checked_cast<_Char>(written.size() - 1);
        ::memcpy(_small.Buffer, written.data(), written.size() * sizeof(_Char)/* also copies the null char */);

        sb.clear();
    }
    else {
        size_t len;
        auto stolen = sb.StealDataUnsafe(get_allocator_(), &len);
        Assert(stolen[len - 1] == _Char()); // check that the buffer is null terminated

        _large = { stolen.data() != nullptr, len - 1/* null char */, stolen.size(), stolen.data() };
    }

    Assert(sb.empty());
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicString<_Char>::CheckInvariants() const {
#ifndef NDEBUG
    if ((!!_large.IsLarge) != (!!_small.IsLarge))
        return false;

    if (_large.IsLarge) {
        if (nullptr == _large.Storage)
            return false;
        if (_large.Size >= _large.Capacity)
            return false;
        if (_large.Storage[_large.Size] != _Char())
            return false;
        /* Not true when using clear() !
         if (USE_CORE_BASICSTRING_SBO && _large.Capacity <= FSmallString_::GCapacity)
            return false;
            */
    }
    else {
        if (!USE_CORE_BASICSTRING_SBO && _small.Size)
            return false;
        if (_small.Size >= FSmallString_::GCapacity)
            return false;
        if (_small.Buffer[size_t(_small.Size)] != _Char())
            return false;
    }
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::reserve(size_t count) {
    if (count > capacity()) {
        const size_t sz = size();
        auto dst = resizeNoNullChar_(count, false);
        Assert(size() == sz);
        dst[sz] = _Char();
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeNoNullChar_(size_t count, bool change_size/* = true */) {
    return ((USE_CORE_BASICSTRING_SBO && count < FSmallString_::GCapacity)
        ? resizeNoNullChar_Small_(count, change_size)
        : resizeNoNullChar_Large_(count, change_size) );
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeNoNullChar_Large_(size_t count, bool change_size) {
    Assert(CheckInvariants());

    const size_t newCapacity = SafeAllocatorSnapSize(get_allocator_(), count + 1/* null char */);
    Assert(!USE_CORE_BASICSTRING_SBO || newCapacity > FSmallString_::GCapacity);

    if (is_large_()) {
        _large.Storage = Relocate_AssumePod(
            get_allocator_(),
            mutableview_type(_large.Storage, _large.Size),
            newCapacity,
            _large.Capacity );
    }
    else {
        _large.IsLarge = 1;
        _Char* const newStorage = allocator_traits::allocate(get_allocator_(), newCapacity);
        ::memcpy(newStorage, _small.Buffer, Min(size_t(_small.Size), count) * sizeof(_Char));
        _large.Storage = newStorage; // assign after memcpy to don't overwrite insitu storage
    }

    _large.Capacity = newCapacity;
    if (change_size) {
        _large.Size = count;
        Assert(_large.Size == count);
    }

    return MakeCheckedIterator(_large.Storage, count + 1/* null char */, 0);
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeNoNullChar_Small_(size_t count, bool change_size) {
    Assert(CheckInvariants());
    Assert(count < FSmallString_::GCapacity);

    if (is_large_()) {
        Assert(_large.Storage);
        _small.IsLarge = 0;
        _Char* const prev_ptr = _large.Storage;
        const size_t prev_capacity = _large.Capacity;
        ::memcpy(_small.Buffer, prev_ptr, Min(_large.Size, count) * sizeof(_Char));
        allocator_traits::deallocate(get_allocator_(), prev_ptr, prev_capacity);
    }

    if (change_size) {
        _small.Size = checked_cast<_Char>(count);
        Assert(_small.Size == count);
    }

    return MakeCheckedIterator(_small.Buffer, count + 1/* null char */, 0);
}
//----------------------------------------------------------------------------
template <typename _Char>
TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeWNullChar_(size_t count) {
    auto dst = resizeNoNullChar_(count);
    dst[count] = _Char();

    Assert(CheckInvariants());

    return dst;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
